#!/bin/sh
# Bench self-test (needs the ROM, so it runs locally, not in CI). Exits non-zero on failure.
#  1. determinism: the same movie twice gives the same WRAM hash every frame
#  2. save states: a state saved mid-race and loaded in a *fresh process* continues exactly
#     like the uninterrupted run (catches state that a save file does not carry)
#  3. the race really runs: DSP-4 commands are written
#  4. differential test: the ported C passes, and a deliberately wrong C fails
# Steps 1-3 run side by side; step 4's rows run in SELFTEST_SHARDS parallel slices (default
# 6). SELFTEST_DIFFTEST=0 skips step 4: the full gate does, after tools/verify_all.sh has
# run every row (fuzz 500) and every negative row over the movies that reach them all.
set -eu
root=$(cd "$(dirname "$0")/../.." && pwd)
rom=${TG3000_ROM:-$root/TG3000.sfc}
bench=$root/build/bench/bench
movie=$root/tools/bench/movies/race1.txt
shards=${SELFTEST_SHARDS:-6}
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

split=7650   # menu/shop, just before the race
total=9450   # ~25 s into the race

"$bench" "$rom" $total --movie "$movie" --hashes "$tmp/a" > "$tmp/out" 2>/dev/null &
pa=$!
"$bench" "$rom" $total --movie "$movie" --hashes "$tmp/b" > /dev/null 2>&1 &
pb=$!
"$bench" "$rom" $split --movie "$movie" --save-state "$tmp/st" > /dev/null 2>&1 &
ps=$!
wait $pa; wait $pb; wait $ps
cmp -s "$tmp/a" "$tmp/b" || { echo "FAIL determinism"; exit 1; }
echo "ok   determinism ($total frames)"

grep -q "dsp4 data: [0-9]* reads [1-9]" "$tmp/out" || { echo "FAIL no DSP-4 writes: race not reached"; exit 1; }
echo "ok   race reached, DSP-4 in use"

awk -v s=$split '$1 !~ /^#/ && $1 >= s {print $1 - s, $2}' "$movie" > "$tmp/tail"
"$bench" "$rom" $((total - split)) --load-state "$tmp/st" --movie "$tmp/tail" --hashes "$tmp/c" > /dev/null 2>&1
awk -v s=$split 'NR > s {print $3}' "$tmp/a" > "$tmp/a3"
awk '{print $3}' "$tmp/c" > "$tmp/c3"
cmp -s "$tmp/a3" "$tmp/c3" || { echo "FAIL fresh-process save/load diverges"; exit 1; }
echo "ok   save state at $split reloaded in a new process matches ($((total - split)) frames)"

if [ "${SELFTEST_DIFFTEST:-1}" = 0 ]; then
  echo "ok   differential test: skipped (SELFTEST_DIFFTEST=0; verify_all ran every row)"
  exit 0
fi

pids=""
k=0
while [ $k -lt "$shards" ]; do
  "$bench" "$rom" $total --movie "$movie" --difftest "$tmp/v$k" --difftest-shard "$k/$shards" --fuzz 200 \
    > "$tmp/d$k" 2>/dev/null &
  pids="$pids $!"
  k=$((k + 1))
done
"$bench" "$rom" $total --movie "$movie" --difftest-negative --fuzz 50 > "$tmp/n" 2>/dev/null &
pn=$!
failed=0
for p in $pids; do s=0; wait "$p" || s=$?; [ $s -eq 0 ] || [ $s -eq 4 ] || failed=1; done
wait $pn || true
# a row fails here on a hard failure (a compared mismatch, a lost case, a broken contract:
# hard:1 in its evidence). The path and flag rules count over all the movies together
# (harness-8, tools/merge_verified.py): this short movie may reach a row on one path only.
hard=$(cat "$tmp"/v* | awk '!/^#/ && / hard:1 / {print $2}')
if [ $failed -ne 0 ] || [ -n "$hard" ]; then
  echo "FAIL differential test${hard:+ ($(echo $hard | tr ' ' ','))}"
  for name in $hard; do cat "$tmp"/d* | grep -A3 "^difftest $name .*: FAIL"; done
  exit 1
fi
echo "ok   differential test passes ($(cat "$tmp"/d* | grep -c PASS) routine(s))"
if grep -q "^difftest.*: PASS" "$tmp/n" || ! grep -q "^difftest.*: FAIL" "$tmp/n"; then
  echo "FAIL negative test: a deliberately wrong C passed"; grep "^difftest" "$tmp/n"; exit 1
fi
echo "ok   negative test: every deliberately wrong C fails ($(grep -c ': FAIL' "$tmp/n"))"
