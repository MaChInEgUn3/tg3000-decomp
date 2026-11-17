#!/bin/sh
# Re-run the differential test for every registered routine over the full autopilot movie
# (which starts with race1.txt) and over the explored shop movie, and rewrite
# map/verified.txt from scratch (tools/merge_verified.py joins the two). Needs the ROM.
# Run it whenever src/, tools/bench/routines.c or tools/bench/difftest.c changes; CI
# (progress.py --check) rejects verified rows whose inputs changed.
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root"
make -s -C tools/bench >/dev/null
rm -f map/verified.txt
out=$(mktemp)
neg=$(mktemp)
shop=$(mktemp)
shopres=$(mktemp)
status=0
shopstatus=0
shopframes=$(sed -n 's/.*Run it for \([0-9]*\) steps.*/\1/p' tools/bench/movies/explore_shop.txt)
# every deliberately wrong C must fail over the same movie (in parallel): the selftest's
# short movie does not reach them all
build/bench/bench "${TG3000_ROM:-TG3000.sfc}" 60000 --movie tools/bench/movies/autopilot_60k.txt \
  --difftest-negative --fuzz 50 > "$neg" 2>/dev/null &
negpid=$!
build/bench/bench "${TG3000_ROM:-TG3000.sfc}" "$shopframes" --movie tools/bench/movies/explore_shop.txt \
  --difftest "$shopres" --fuzz 500 > "$shop" 2>&1 &
shoppid=$!
build/bench/bench "${TG3000_ROM:-TG3000.sfc}" 60000 --movie tools/bench/movies/autopilot_60k.txt \
  --difftest map/verified.txt --fuzz 500 > "$out" 2>&1 || status=$?   # the bench's own status (4 = a routine failed)
wait $shoppid || shopstatus=$?
wait $negpid || true
if [ $shopstatus -ne 0 ]; then
  echo "verify_all: the shop movie failed (bench status $shopstatus):" >&2
  grep -A6 "^difftest.*: FAIL" "$shop" >&2 || true
  status=$shopstatus
fi
python3 -I tools/merge_verified.py map/verified.txt map/verified.txt "$shopres"
rm -f "$shop" "$shopres"
grep -A6 "^difftest" "$out" || true
rm -f "$out"
if grep -q "^difftest.*: \(PASS\|UNREACHED\)" "$neg" || ! grep -q "^difftest.*: FAIL" "$neg"; then
  echo "verify_all: a deliberately wrong C passed or was not reached:" >&2
  grep "^difftest.*: \(PASS\|UNREACHED\)" "$neg" >&2 || true
  status=5
else
  echo "negative rows: every deliberately wrong C fails over the full movie ($(grep -c ': FAIL' "$neg"))"
fi
rm -f "$neg"
python3 -I tools/progress.py
if [ $status -ne 0 ]; then echo "verify_all: FAILED (bench status $status)" >&2; fi
exit $status
