#!/bin/sh
# Fetch the bench's third-party code into ext/ (ignored by git) at pinned commits,
# and apply our patch to LakeSnes. Safe to re-run.
#
# If ext/lakesnes-dinkc64 has edits that are not in tools/bench/patches/, it refuses
# to run (they would be wiped): save them with tools/bench/export-patch.sh first, or
# run with FORCE=1 to throw them away.
set -eu
root=$(cd "$(dirname "$0")/../.." && pwd)
patch="$root/tools/bench/patches/lakesnes-dsp4.patch"
ls_dir="$root/ext/lakesnes-dinkc64"
mkdir -p "$root/ext"

if [ -d "$ls_dir/.git" ] && [ "${FORCE:-0}" != 1 ]; then
  git -C "$ls_dir" add -N . 2>/dev/null || true
  if ! git -C "$ls_dir" diff | cmp -s - "$patch"; then
    if [ -n "$(git -C "$ls_dir" status --porcelain)" ]; then
      echo "fetch.sh: ext/lakesnes-dinkc64 has edits that differ from $patch" >&2
      echo "  save them:    tools/bench/export-patch.sh" >&2
      echo "  discard them: FORCE=1 tools/bench/fetch.sh" >&2
      exit 1
    fi
  fi
fi

fetch() { # dir url commit
  dir="$root/ext/$1"
  if [ ! -d "$dir/.git" ]; then
    git clone -q "$2" "$dir"
  fi
  git -C "$dir" fetch -q origin "$3" 2>/dev/null || true
  git -C "$dir" reset -q
  git -C "$dir" checkout -q -f "$3"
  git -C "$dir" clean -q -fd
}

fetch lakesnes-dinkc64 https://github.com/dinkc64/LakeSnes.git 048a0d7
fetch snes-dsp4 https://github.com/StackBlender/snes-dsp4.git 3155961

git -C "$ls_dir" apply "$patch"
echo "ext/ ready"
