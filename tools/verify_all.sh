#!/bin/sh
# Re-run the differential test for every registered routine over the full autopilot movie
# (which starts with race1.txt), over the explored movies (shop, buy, password, menus) and the
# INFODATA movie, and
# rewrite map/verified.txt from scratch (tools/merge_verified.py joins them). Needs the ROM.
# Run it whenever src/, tools/bench/routines.c or tools/bench/difftest.c changes; CI
# (progress.py --check) rejects verified rows whose inputs changed.
#
# Each movie's rows run as parallel slices (bench --difftest-shard K/N, contiguous, so
# the joined results keep the registry's order): TG3000_SHARDS slices of the 60k movie
# (default 5), two each of the shop, shop-buy and password movies and one of the INFODATA
# movie, beside the negative rows. A slice still runs its
# whole movie; a waiting row nested in another one's sample is sampled when the outer row
# is in another slice, so live counts can differ from a single run. A row that fails after
# the merge because a waiting row in its slice hid entries from it runs again alone over
# that movie (tools/rerun_hidden.py, harness-24).
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
infoframes=$(sed -n 's/.*Run it for \([0-9]*\) steps.*/\1/p' tools/bench/movies/code_infodata.txt)
menuframes=$(sed -n 's/.*Run it for \([0-9]*\) steps.*/\1/p' tools/bench/movies/explore_menu.txt)
optframes=$(sed -n 's/.*Run it for \([0-9]*\) steps.*/\1/p' tools/bench/movies/explore_options.txt)
vsframes=$(sed -n 's/.*Run it for \([0-9]*\) steps.*/\1/p' tools/bench/movies/vs_race.txt)
# the VS race runs only with the DSP-4 run from its firmware (harness-27, the owner's file)
firmware="${TG3000_DSP4_FIRMWARE:-ext/firmware/dsp4.rom}"
vsfuzzskip=vs_screen_frame,player_car_frame,car_sprite_head,vs_cars_frame
pwfuzzskip=car_sprite_head  # (dsp4-11a: the password movie's last RAM)
# The slices run from a queue, at most TG3000_JOBS at once (default: the cores), the longest first
# (by their measured CPU: verify-speed-1), instead of all at once: the same slices and checks, less
# time lost to more processes than cores. A slice that crashes (a status other than 0 and 4)
# skips the slices not yet started.
jobs="${TG3000_JOBS:-$(nproc 2>/dev/null || echo 8)}"
: > "$tmp/queue"
job() {  # name, then the bench's arguments (its output to $tmp/name.log, its status to $tmp/name.st)
  jn=$1  # (not `name`: sh has no local variables, and the composed rows' loop uses it)
  shift
  {
    echo "if [ -e '$tmp/crashed' ]; then echo 99 > '$tmp/$jn.st'; exit 0; fi"
    echo "t0=\$(date +%s)"
    printf 'build/bench/bench'
    for a in "$@"; do printf " '%s'" "$a"; done
    echo " > '$tmp/$jn.log' 2>&1"
    echo "s=\$?; echo \$s > '$tmp/$jn.st'; echo \$((\$(date +%s) - t0)) > '$tmp/$jn.time'"
    echo "if [ \$s -ne 0 ] && [ \$s -ne 4 ] && [ '$jn' != neg ]; then : > '$tmp/crashed'; fi"
  } > "$tmp/$jn.sh"
  echo "$tmp/$jn.sh" >> "$tmp/queue"
}
main() { job "main$1" "$rom" 60000 --movie tools/bench/movies/autopilot_60k.txt --difftest-shard "$1/$shards" \
  --difftest "$tmp/main$1.txt" --fuzz 500; }
# (the queue's order is set below, the longest first)
k=$((shards - 1))
while [ $k -ge $((shards - 2)) ] && [ $k -ge 0 ]; do main $k; k=$((k - 1)); done
job buy1 "$rom" "$buyframes" --movie tools/bench/movies/explore_buy.txt --difftest-shard "1/$buyshards" \
  --difftest "$tmp/buy1.txt" --fuzz 500
job pw1 "$rom" "$pwframes" --movie tools/bench/movies/explore_pw.txt --difftest-shard "1/$pwshards" \
  --fuzz-skip "$pwfuzzskip" --difftest "$tmp/pw1.txt" --fuzz 500
# the INFODATA movie (explore-4): short, one slice
job info "$rom" "$infoframes" --movie tools/bench/movies/code_infodata.txt --difftest "$tmp/info.txt" --fuzz 500
# the VS race's movie (verify-vs): the firmware DSP-4; its last state is a race, which is no
# base for the VS screen's frame's fuzz (harness-28: skipped there)
job vs "$rom" "$vsframes" --movie tools/bench/movies/vs_race.txt --dsp4-firmware "$firmware" \
  --fuzz-skip "$vsfuzzskip" --difftest "$tmp/vs.txt" --fuzz 500
while [ $k -ge 0 ]; do main $k; k=$((k - 1)); done
# every deliberately wrong C must fail over the same movie: the selftest's short movie does not
# reach them all
job neg "$rom" 60000 --movie tools/bench/movies/autopilot_60k.txt --difftest-negative --fuzz 50
k=0
while [ $k -lt $shopshards ]; do
  job "shop$k" "$rom" "$shopframes" --movie tools/bench/movies/explore_shop.txt --difftest-shard "$k/$shopshards" \
    --difftest "$tmp/shop$k.txt" --fuzz 500
  k=$((k + 1))
done
k=0
while [ $k -lt $buyshards ]; do
  [ $k -eq 1 ] || job "buy$k" "$rom" "$buyframes" --movie tools/bench/movies/explore_buy.txt \
    --difftest-shard "$k/$buyshards" --difftest "$tmp/buy$k.txt" --fuzz 500
  k=$((k + 1))
done
k=0
while [ $k -lt $pwshards ]; do
  [ $k -eq 1 ] || job "pw$k" "$rom" "$pwframes" --movie tools/bench/movies/explore_pw.txt \
    --difftest-shard "$k/$pwshards" --fuzz-skip "$pwfuzzskip" --difftest "$tmp/pw$k.txt" --fuzz 500
  k=$((k + 1))
done
# the menus' movie (explore-5) and the options submenu's (explore-7): short, one slice each
job menu "$rom" "$menuframes" --movie tools/bench/movies/explore_menu.txt --difftest "$tmp/menu.txt" --fuzz 500
job opt "$rom" "$optframes" --movie tools/bench/movies/explore_options.txt --difftest "$tmp/opt.txt" --fuzz 500
# glue-1: the composed rows (a screen whole, its parts' rows inside it), one per process and
# movie, live samples only (each encloses waiting rows, so apart from the slices), over the
# movies its "// movies:" comment in tools/bench/routines.c names
composed=$(awk '/^const Routine composedRoutines\[\]/ {p = 1} p && /\/\/ movies:/ {m = $0; sub(/.*movies: */, "", m)} p && /\.name = "/ {match($0, /\.name = "[^"]+"/); print substr($0, RSTART + 9, RLENGTH - 10) ":" m; gsub(/ /, "", m)} p && /^};/ {exit}' tools/bench/routines.c | tr ' ' ',')
compfiles=""
compnames=""
for row in $composed; do
  name=${row%%:*}
  for movie in $(echo "${row#*:}" | tr ',' ' '); do
    frames=$(sed -n 's/.*Run it for \([0-9]*\) steps.*/\1/p' "tools/bench/movies/$movie.txt")
    [ -n "$frames" ] || frames=60000  # (the 60k movie has no such line)
    if [ "$movie" = vs_race ]; then
      job "comp_${name}_$movie" "$rom" "$frames" --movie "tools/bench/movies/$movie.txt" --dsp4-firmware "$firmware" \
        --difftest-composed --difftest-only "$name" --difftest "$tmp/comp_${name}_$movie.txt" --fuzz 0
    else
      job "comp_${name}_$movie" "$rom" "$frames" --movie "tools/bench/movies/$movie.txt" \
        --difftest-composed --difftest-only "$name" --difftest "$tmp/comp_${name}_$movie.txt" --fuzz 0
    fi
    compfiles="$compfiles $tmp/comp_${name}_$movie.txt"
    compnames="$compnames comp_${name}_$movie"
  done
done
# the queue longest first, by the slices' wall times measured at 8 at once (verify-speed-1); a
# slice not named here (a new composed row) runs after them
longest="buy1 main4 pw1 info main3 vs main2 main0 main1 neg shop1 comp_password_show_autopilot_60k pw0 buy0 shop0 menu opt"
: > "$tmp/queue2"
for n in $longest; do grep "/$n\.sh\$" "$tmp/queue" >> "$tmp/queue2" || true; done
grep -v -x -F -f "$tmp/queue2" "$tmp/queue" >> "$tmp/queue2" || true
started=$(date +%s)
xargs -P "$jobs" -n 1 sh < "$tmp/queue2"
echo "verify_all: $(wc -l < "$tmp/queue") slices, $jobs at a time, $(( $(date +%s) - started )) s:$(for f in "$tmp"/*.time; do printf ' %s %ss' "$(basename "$f" .time)" "$(cat "$f")"; done)"
names="neg"
k=0
while [ $k -lt "$shards" ]; do names="$names main$k"; k=$((k + 1)); done
for n in shop buy pw; do
  k=0
  while [ $k -lt 2 ]; do names="$names $n$k"; k=$((k + 1)); done
done
for name in $names info menu opt vs $compnames; do
  s=$(cat "$tmp/$name.st" 2>/dev/null || echo 98)
  [ "$name" = neg ] && continue
  # harness-8: a routine that fails on one movie by the flag or path rule alone may pass on
  # the evidence pooled over the movies; the merge below decides (other statuses fail)
  if [ "$s" -ne 0 ] && [ "$s" -ne 4 ]; then
    echo "verify_all: slice $name failed (bench status $s)" >&2
    status=$s
  fi
done
cp "$tmp/neg.log" "$tmp/neg"
if [ $status -ne 0 ]; then echo "verify_all: FAILED (a slice crashed; the merge is skipped)" >&2; exit $status; fi
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
python3 -I tools/merge_verified.py map/verified.txt "$tmp/main.txt" "$tmp/shop.txt" "$tmp/buy.txt" "$tmp/pw.txt" "$tmp/info.txt" "$tmp/menu.txt" "$tmp/opt.txt" "$tmp/vs.txt" $compfiles
# harness-24: a row a slice's waiting row hid entries from (and with no hard failure) runs
# again alone over that movie, its line replacing the slice's; then the movies merge again
python3 -I tools/rerun_hidden.py "$tmp" "$rom"
python3 -I tools/merge_verified.py map/verified.txt "$tmp/main.txt" "$tmp/shop.txt" "$tmp/buy.txt" "$tmp/pw.txt" "$tmp/info.txt" "$tmp/menu.txt" "$tmp/opt.txt" "$tmp/vs.txt" $compfiles
# the verdict: a routine the merge failed (its per-movie lines show why)
failed=$(awk '!/^#/ && $8 == "failed" {print $2}' map/verified.txt)
if [ -n "$failed" ]; then
  for name in $failed; do grep -h -A6 "^difftest $name .*: FAIL" "$tmp"/main*.log "$tmp"/shop*.log "$tmp"/buy*.log "$tmp"/pw*.log "$tmp"/info.log "$tmp"/menu.log "$tmp"/opt.log "$tmp"/vs.log "$tmp"/comp_*.log >&2 || true; done
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
