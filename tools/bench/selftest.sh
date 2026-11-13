#!/bin/sh
# Bench self-test (needs the ROM, so it runs locally, not in CI). Exits non-zero on failure.
#  1. determinism: the same movie twice gives the same WRAM hash every frame
#  2. save states: a state saved mid-race and loaded in a *fresh process* continues exactly
#     like the uninterrupted run (catches state that a save file does not carry)
#  3. the race really runs: DSP-4 commands are written
set -eu
root=$(cd "$(dirname "$0")/../.." && pwd)
rom=${TG3000_ROM:-$root/TG3000.sfc}
bench=$root/build/bench/bench
movie=$root/tools/bench/movies/race1.txt
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

split=7650   # menu/shop, just before the race
total=9450   # ~25 s into the race

"$bench" "$rom" $total --movie "$movie" --hashes "$tmp/a" > "$tmp/out" 2>/dev/null
"$bench" "$rom" $total --movie "$movie" --hashes "$tmp/b" > /dev/null 2>&1
cmp -s "$tmp/a" "$tmp/b" || { echo "FAIL determinism"; exit 1; }
echo "ok   determinism ($total frames)"

grep -q "dsp4 data: [0-9]* reads [1-9]" "$tmp/out" || { echo "FAIL no DSP-4 writes: race not reached"; exit 1; }
echo "ok   race reached, DSP-4 in use"

"$bench" "$rom" $split --movie "$movie" --save-state "$tmp/st" > /dev/null 2>&1
awk -v s=$split '$1 !~ /^#/ && $1 >= s {print $1 - s, $2}' "$movie" > "$tmp/tail"
"$bench" "$rom" $((total - split)) --load-state "$tmp/st" --movie "$tmp/tail" --hashes "$tmp/c" > /dev/null 2>&1
awk -v s=$split 'NR > s {print $3}' "$tmp/a" > "$tmp/a3"
awk '{print $3}' "$tmp/c" > "$tmp/c3"
cmp -s "$tmp/a3" "$tmp/c3" || { echo "FAIL fresh-process save/load diverges"; exit 1; }
echo "ok   save state at $split reloaded in a new process matches ($((total - split)) frames)"
