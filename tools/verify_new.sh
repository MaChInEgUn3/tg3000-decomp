#!/bin/sh
# Verify only the named routines (new rows, added without changing any existing line of
# src/ or tools/bench/) and merge their results into map/verified.txt; every other row
# keeps the result of the last full run (tools/verify_all.sh). Needs the ROM.
#
#     tools/verify_new.sh name1,name2,...
#
# The rows run over the movies verify_all.sh uses, each skipped when it never executes a named
# row's entry (harness-12; the coverage of each movie is cached in build/cache/).
# progress.py --check (and CI) then accepts the older rows only while
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
# additions only: no line of src/ or tools/bench/ removed or changed, the harness untouched
if ! git diff --quiet HEAD -- tools/bench/difftest.c tools/bench/platform_bench.c \
   || git diff -U0 --no-color HEAD -- src tools/bench | grep -v '^--- ' | grep -q '^-'; then
  echo "verify_new: src/ or tools/bench/ changed by more than additions: run tools/verify_all.sh" >&2
  exit 7
fi
# a new waiting row can enclose older rows' samples and change their evidence (shop-21): it
# goes through the full run
for name in $(echo "$rows" | tr ',' ' '); do
  if awk -v n="$name" 'index($0, "{.name = \"" n "\"") > 0 { f = 1; print; next }
       f && (/^  \{\.name/ || /^  \/\// || /^};/) { exit } f { print }' tools/bench/routines.c \
     | grep -q waitsForInterrupts; then
    echo "verify_new: $name waits for interrupts (a waiting row): run tools/verify_all.sh" >&2
    exit 7
  fi
done
# every named row must exist: a misspelt name runs nothing
status=0
for name in $(echo "$rows" | tr ',' ' '); do
  if ! grep -q "{\.name = \"$name\"" tools/bench/routines.c; then
    echo "verify_new: no routine named $name" >&2
    status=6
  fi
done
if [ $status -ne 0 ]; then echo "verify_new: FAILED" >&2; exit $status; fi
make -s -C tools/bench >/dev/null
rom="${TG3000_ROM:-TG3000.sfc}"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
shopframes=$(sed -n 's/.*Run it for \([0-9]*\) steps.*/\1/p' tools/bench/movies/explore_shop.txt)
buyframes=$(sed -n 's/.*Run it for \([0-9]*\) steps.*/\1/p' tools/bench/movies/explore_buy.txt)
pwframes=$(sed -n 's/.*Run it for \([0-9]*\) steps.*/\1/p' tools/bench/movies/explore_pw.txt)
infoframes=$(sed -n 's/.*Run it for \([0-9]*\) steps.*/\1/p' tools/bench/movies/code_infodata.txt)
menuframes=$(sed -n 's/.*Run it for \([0-9]*\) steps.*/\1/p' tools/bench/movies/explore_menu.txt)
# harness-12: a movie that never executes any named row's entry gives the merge no evidence;
# it is skipped. Each movie's coverage is cached in build/cache/, keyed by the movie, the
# emulator patch and bench.c (a stale or missing file is made again: one plain run).
entries=$(python3 -I - "$rows" <<'PY'
import re, sys
text = open("tools/bench/routines.c").read()
for name in sys.argv[1].split(","):
    m = re.search(r'\{\.name = "%s", \.entry = 0x([0-9A-Fa-f]+)' % re.escape(name), text)
    print(m.group(1) if m else "")
PY
)
mkdir -p build/cache
covfile() { echo "build/cache/$1-$(cat "tools/bench/movies/$1.txt" tools/bench/patches/*.patch tools/bench/bench.c | sha1sum | cut -c1-16).cov"; }
covpids=""
for m in "autopilot_60k:60000" "explore_shop:$shopframes" "explore_buy:$buyframes" "explore_pw:$pwframes" "code_infodata:$infoframes" "explore_menu:$menuframes"; do
  movie=${m%%:*}; cov=$(covfile "$movie")
  if [ ! -f "$cov" ]; then  # made again, side by side
    rm -f build/cache/"$movie"-*.cov
    ( build/bench/bench "$rom" "${m#*:}" --movie "tools/bench/movies/$movie.txt" --coverage "$cov.tmp" > /dev/null 2>&1 \
      && mv "$cov.tmp" "$cov" || rm -f "$cov.tmp"; rm -f "$cov.tmp.ram" ) &  # (the RAM-executed list: unused)
    covpids="$covpids $!"
  fi
done
for p in $covpids; do wait "$p" || true; done
reaches() {  # movie: 0 when it executes one of the named rows' entries (or has no coverage)
  cov=$(covfile "$1")
  [ -f "$cov" ] || return 0
  python3 -I - "$cov" $entries <<'PY'
import sys
cov = open(sys.argv[1], "rb").read()
for e in sys.argv[2:]:
    a = int(e, 16)
    if cov[((a >> 16) & 0x7F) * 0x8000 + (a & 0x7FFF)]:
        sys.exit(0)
sys.exit(1)
PY
}
runs=""
for m in "main:autopilot_60k:60000" "shop:explore_shop:$shopframes" "buy:explore_buy:$buyframes" "pw:explore_pw:$pwframes" "info:code_infodata:$infoframes" "menu:explore_menu:$menuframes"; do
  k=${m%%:*}; rest=${m#*:}; movie=${rest%%:*}; frames=${rest#*:}
  if reaches "$movie"; then runs="$runs $k:$movie:$frames"; else echo "verify_new: $movie skipped (no named row's entry runs there)"; fi
done
[ -n "$runs" ] || runs=" main:autopilot_60k:60000"  # reached nowhere: one run records them unreached
pids=""
for r in $runs; do
  k=${r%%:*}; rest=${r#*:}; movie=${rest%%:*}; frames=${rest#*:}
  build/bench/bench "$rom" "$frames" --movie "tools/bench/movies/$movie.txt" --difftest "$tmp/$k.txt" \
    --difftest-only "$rows" --fuzz 500 > "$tmp/$k.log" 2>&1 &
  pids="$pids $!:$k"
done
for p in $pids; do
  s=0
  wait "${p%%:*}" || s=$?   # the bench's own status (4 = a routine failed on that movie)
  # harness-8: a routine that fails on one movie by the flag or path rule alone may pass on
  # the evidence pooled over the movies; the merge below decides (other statuses fail)
  if [ $s -ne 0 ] && [ $s -ne 4 ]; then
    echo "verify_new: ${p#*:} movie failed (bench status $s)" >&2
    status=$s
  fi
done
cat "$tmp"/*.log | grep -A6 "^difftest" || true
if [ $status -ne 0 ]; then echo "verify_new: FAILED" >&2; exit $status; fi
results=""
for r in $runs; do results="$results $tmp/${r%%:*}.txt"; done
python3 -I tools/merge_verified.py "$tmp/new.txt" $results
# the verdict: a named row the merge failed (its per-movie lines show why)
failed=$(awk '!/^#/ && $8 == "failed" {print $2}' "$tmp/new.txt")
if [ -n "$failed" ]; then
  for name in $failed; do cat "$tmp"/*.log | grep -A6 "^difftest $name .*: FAIL" >&2 || true; done
  echo "verify_new: FAILED ($(echo $failed | tr ' ' ','))" >&2
  exit 4
fi
python3 -I tools/merge_verified.py --update map/verified.txt "$tmp/new.txt"
python3 -I tools/progress.py
