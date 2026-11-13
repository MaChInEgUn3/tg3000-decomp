#!/bin/sh
# Fetch the bench's third-party code into ext/ (ignored by git) at pinned commits,
# and apply our DSP-4 mapper patch to LakeSnes. Safe to re-run.
set -eu
root=$(cd "$(dirname "$0")/../.." && pwd)
mkdir -p "$root/ext"

fetch() { # dir url commit
  dir="$root/ext/$1"
  if [ ! -d "$dir/.git" ]; then
    git clone -q "$2" "$dir"
  fi
  git -C "$dir" fetch -q origin "$3" 2>/dev/null || true
  git -C "$dir" checkout -q -f "$3"
  git -C "$dir" clean -q -fd
}

fetch lakesnes-dinkc64 https://github.com/dinkc64/LakeSnes.git 048a0d7
fetch snes-dsp4 https://github.com/StackBlender/snes-dsp4.git 3155961

git -C "$root/ext/lakesnes-dinkc64" apply "$root/tools/bench/patches/lakesnes-dsp4.patch"
echo "ext/ ready"
