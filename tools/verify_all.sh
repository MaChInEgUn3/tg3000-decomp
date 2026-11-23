#!/bin/sh
# Re-run the differential test for every registered routine over the full autopilot movie
# (which starts with race1.txt) and over the three explored movies, and rewrite
# map/verified.txt from scratch (tools/merge_verified.py joins them). Needs the ROM.
# Run it whenever src/, tools/bench/routines.c or tools/bench/difftest.c changes; CI
# (progress.py --check) rejects verified rows whose inputs changed.
#
# Each movie's rows run as parallel slices (bench --difftest-shard K/N, contiguous, so
# the joined results keep the registry's order): TG3000_SHARDS slices of the 60k movie
# (default 5) and two each of the shop, shop-buy and password movies, beside the negative rows. A slice still runs its
# whole movie; a waiting row nested in another one's sample is sampled when the outer row
# is in another slice, so live counts can differ from a single run.
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root"
make -s -C tools/bench >/dev/null
rom="${TG3000_ROM:-TG3000.sfc}"
shards="${TG3000_SHARDS:-5}"
shopshards=2
buyshards=2
pwshards=2
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
status=0
shopframes=$(sed -n 's/.*Run it for \([0-9]*\) steps.*/\1/p' tools/bench/movies/explore_shop.txt)
buyframes=$(sed -n 's/.*Run it for \([0-9]*\) steps.*/\1/p' tools/bench/movies/explore_buy.txt)
pwframes=$(sed -n 's/.*Run it for \([0-9]*\) steps.*/\1/p' tools/bench/movies/explore_pw.txt)
# every deliberately wrong C must fail over the same movie (in parallel): the selftest's
# short movie does not reach them all
build/bench/bench "$rom" 60000 --movie tools/bench/movies/autopilot_60k.txt \
  --difftest-negative --fuzz 50 > "$tmp/neg" 2>/dev/null &
negpid=$!
pids=""
k=0
while [ $k -lt "$shards" ]; do
  build/bench/bench "$rom" 60000 --movie tools/bench/movies/autopilot_60k.txt --difftest-shard "$k/$shards" \
    --difftest "$tmp/main$k.txt" --fuzz 500 > "$tmp/main$k.log" 2>&1 &
  pids="$pids $!:main$k"
  k=$((k + 1))
done
k=0
while [ $k -lt $shopshards ]; do
  build/bench/bench "$rom" "$shopframes" --movie tools/bench/movies/explore_shop.txt --difftest-shard "$k/$shopshards" \
    --difftest "$tmp/shop$k.txt" --fuzz 500 > "$tmp/shop$k.log" 2>&1 &
  pids="$pids $!:shop$k"
  k=$((k + 1))
done
k=0
while [ $k -lt $buyshards ]; do
  build/bench/bench "$rom" "$buyframes" --movie tools/bench/movies/explore_buy.txt --difftest-shard "$k/$buyshards" \
    --difftest "$tmp/buy$k.txt" --fuzz 500 > "$tmp/buy$k.log" 2>&1 &
  pids="$pids $!:buy$k"
  k=$((k + 1))
done
k=0
while [ $k -lt $pwshards ]; do
  build/bench/bench "$rom" "$pwframes" --movie tools/bench/movies/explore_pw.txt --difftest-shard "$k/$pwshards" \
    --difftest "$tmp/pw$k.txt" --fuzz 500 > "$tmp/pw$k.log" 2>&1 &
  pids="$pids $!:pw$k"
  k=$((k + 1))
done
for p in $pids; do
  pid=${p%%:*}
  name=${p#*:}
  s=0
  wait "$pid" || s=$?   # the bench's own status (4 = a routine failed on that movie)
  # harness-8: a routine that fails on one movie by the flag or path rule alone may pass on
  # the evidence pooled over the movies; the merge below decides (other statuses fail)
  if [ $s -ne 0 ] && [ $s -ne 4 ]; then
    echo "verify_all: slice $name failed (bench status $s)" >&2
    status=$s
  fi
done
wait $negpid || true
# the 60k slices in order, then the shop slices: one row each per movie
mains=""
k=0
while [ $k -lt "$shards" ]; do mains="$mains $tmp/main$k.txt"; k=$((k + 1)); done
head -n 1 "$tmp/main0.txt" > "$tmp/main.txt"
for f in $mains; do tail -n +2 "$f" >> "$tmp/main.txt"; done
head -n 1 "$tmp/shop0.txt" > "$tmp/shop.txt"
k=0
while [ $k -lt $shopshards ]; do tail -n +2 "$tmp/shop$k.txt" >> "$tmp/shop.txt"; k=$((k + 1)); done
head -n 1 "$tmp/buy0.txt" > "$tmp/buy.txt"
k=0
while [ $k -lt $buyshards ]; do tail -n +2 "$tmp/buy$k.txt" >> "$tmp/buy.txt"; k=$((k + 1)); done
head -n 1 "$tmp/pw0.txt" > "$tmp/pw.txt"
k=0
while [ $k -lt $pwshards ]; do tail -n +2 "$tmp/pw$k.txt" >> "$tmp/pw.txt"; k=$((k + 1)); done
python3 -I tools/merge_verified.py map/verified.txt "$tmp/main.txt" "$tmp/shop.txt" "$tmp/buy.txt" "$tmp/pw.txt"
# the verdict: a routine the merge failed (its per-movie lines show why)
failed=$(awk '!/^#/ && $8 == "failed" {print $2}' map/verified.txt)
if [ -n "$failed" ]; then
  for name in $failed; do grep -h -A6 "^difftest $name .*: FAIL" "$tmp"/main*.log "$tmp"/shop*.log "$tmp"/buy*.log "$tmp"/pw*.log >&2 || true; done
  echo "verify_all: failed after pooling the movies: $(echo $failed | tr ' ' ',')" >&2
  status=4
fi
k=0
while [ $k -lt "$shards" ]; do grep -A6 "^difftest" "$tmp/main$k.log" || true; k=$((k + 1)); done
if grep -q "^difftest.*: \(PASS\|UNREACHED\)" "$tmp/neg" || ! grep -q "^difftest.*: FAIL" "$tmp/neg"; then
  echo "verify_all: a deliberately wrong C passed or was not reached:" >&2
  grep "^difftest.*: \(PASS\|UNREACHED\)" "$tmp/neg" >&2 || true
  status=5
else
  echo "negative rows: every deliberately wrong C fails over the full movie ($(grep -c ': FAIL' "$tmp/neg"))"
fi
# the mark of a full run (progress.py finds its commit with git blame), only when it passed
if [ $status -eq 0 ]; then
  sed -i "1a # full run: $(date -u +%Y-%m-%dT%H:%M:%SZ) (tools/verify_all.sh)" map/verified.txt
fi
python3 -I tools/progress.py
if [ $status -ne 0 ]; then echo "verify_all: FAILED (bench status $status)" >&2; fi
exit $status
