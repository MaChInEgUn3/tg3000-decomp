#!/bin/sh
# Save every edit made in ext/lakesnes-dinkc64 (new files included) into
# tools/bench/patches/lakesnes-dsp4.patch, so fetch.sh can re-create it.
set -eu
root=$(cd "$(dirname "$0")/../.." && pwd)
ls_dir="$root/ext/lakesnes-dinkc64"
git -C "$ls_dir" add -N .
git -C "$ls_dir" diff > "$root/tools/bench/patches/lakesnes-dsp4.patch"
git -C "$ls_dir" diff --stat
