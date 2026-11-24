#!/bin/sh
# Fetch the reference TAS ("Light Spike" by Jmosx36, TASVideos user file 34264942243851572:
# BizHawk 1.9.4, bsnes, the real DSP-4 firmware, 450,994 frames) into traces/tas/ (ignored by
# git: its author's work is not committed). Checks that its header names our ROM, then
# unpacks traces/tas/unpacked/Input Log.txt for tools/refbench (docs/WORKFLOW.md).
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
dir="$root/traces/tas"
mkdir -p "$dir"
if [ ! -f "$dir/lightspike.bk2" ]; then
  curl -sSL -A "Mozilla/5.0" -o "$dir/lightspike.bk2.dl" "https://tasvideos.org/UserFiles/Info/34264942243851572?handler=Download"
  if gzip -t "$dir/lightspike.bk2.dl" 2>/dev/null; then gunzip -c "$dir/lightspike.bk2.dl" > "$dir/lightspike.bk2"
  else mv "$dir/lightspike.bk2.dl" "$dir/lightspike.bk2"; fi
  rm -f "$dir/lightspike.bk2.dl"
fi
python3 -I - "$dir" <<'PY'
import os, sys, zipfile
d = sys.argv[1]
z = zipfile.ZipFile(os.path.join(d, "lightspike.bk2"))
header = z.read("Header.txt").decode(errors="replace")
if "058AADF1EE719CB9E0C7333E665797F990A6381D" not in header.upper():
    sys.exit("fetch_tas: the movie's header does not name our ROM")
os.makedirs(os.path.join(d, "unpacked"), exist_ok=True)
for name in ("Header.txt", "Input Log.txt", "SyncSettings.json"):
    z.extract(name, os.path.join(d, "unpacked"))
lines = z.read("Input Log.txt").decode(errors="replace").count("\n")
print("fetch_tas: traces/tas/unpacked/Input Log.txt (%d lines)" % lines)
PY
