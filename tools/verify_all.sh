#!/bin/sh
# Re-run the differential test for every registered routine over the full autopilot movie
# (which starts with race1.txt) and rewrite map/verified.txt from scratch. Needs the ROM.
# Run it whenever src/, tools/bench/routines.c or tools/bench/difftest.c changes; CI
# (progress.py --check) rejects verified rows whose inputs changed.
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root"
make -s -C tools/bench >/dev/null
rm -f map/verified.txt
status=0
build/bench/bench "${TG3000_ROM:-TG3000.sfc}" 60000 --movie tools/bench/movies/autopilot_60k.txt \
  --difftest map/verified.txt --fuzz 500 | grep -A6 "^difftest" || status=$?
python3 -I tools/progress.py
exit $status
