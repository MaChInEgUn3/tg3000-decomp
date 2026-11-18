#!/bin/sh
# Verify only the named routines (new rows, added without changing any existing line of
# src/ or tools/bench/) and merge their results into map/verified.txt; every other row
# keeps the result of the last full run (tools/verify_all.sh). Needs the ROM.
#
#     tools/verify_new.sh name1,name2,...
#
# The rows run over the full autopilot movie and the explored shop movie, as in
# verify_all.sh. progress.py --check (and CI) then accepts the older rows only while
# src/ and tools/bench/ changed by additions alone since the commit they were verified
# at, and that commit is at most MAX_BEHIND commits back (tools/progress.py); otherwise
# run tools/verify_all.sh. tools/bench/selftest.sh still runs every routine the short
# movie reaches.
set -eu
if [ $# -ne 1 ] || [ -z "$1" ]; then
  echo "usage: tools/verify_new.sh name1,name2,..." >&2
  exit 2
fi
rows=$1
root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root"
make -s -C tools/bench >/dev/null
rom="${TG3000_ROM:-TG3000.sfc}"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
shopframes=$(sed -n 's/.*Run it for \([0-9]*\) steps.*/\1/p' tools/bench/movies/explore_shop.txt)
build/bench/bench "$rom" 60000 --movie tools/bench/movies/autopilot_60k.txt --difftest "$tmp/main.txt" \
  --difftest-only "$rows" --fuzz 500 > "$tmp/main.log" 2>&1 &
mainpid=$!
build/bench/bench "$rom" "$shopframes" --movie tools/bench/movies/explore_shop.txt --difftest "$tmp/shop.txt" \
  --difftest-only "$rows" --fuzz 500 > "$tmp/shop.log" 2>&1 &
shoppid=$!
status=0
for p in "$mainpid:main" "$shoppid:shop"; do
  s=0
  wait "${p%%:*}" || s=$?   # the bench's own status (4 = a routine failed)
  if [ $s -ne 0 ]; then
    echo "verify_new: ${p#*:} movie failed (bench status $s):" >&2
    grep -A6 "^difftest.*: FAIL" "$tmp/${p#*:}.log" >&2 || true
    status=$s
  fi
done
grep -A6 "^difftest" "$tmp/main.log" || true
# every named row must be in the results: a misspelt name runs nothing
for name in $(echo "$rows" | tr ',' ' '); do
  if ! grep -q "^[0-9A-F]* $name " "$tmp/main.txt"; then
    echo "verify_new: no routine named $name" >&2
    status=6
  fi
done
if [ $status -ne 0 ]; then echo "verify_new: FAILED" >&2; exit $status; fi
python3 -I tools/merge_verified.py "$tmp/new.txt" "$tmp/main.txt" "$tmp/shop.txt"
python3 -I tools/merge_verified.py --update map/verified.txt "$tmp/new.txt"
python3 -I tools/progress.py
