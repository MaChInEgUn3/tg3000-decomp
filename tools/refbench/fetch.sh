#!/bin/sh
# Fetch the reference bench's emulator into ext/ (ignored by git): bsnes v087 as shipped in
# BizHawk 1.9.4 (libsnes, the core community TAS movies of this game were recorded on),
# sparse-checked out at the 1.9.4 tag, plus our patch (Linux build, no Windows IPC wrapper,
# DSP-4 high-level hooks, a DSP-4 write counter). Needs tools/bench/fetch.sh for snes-dsp4.
#
# The DSP-4 firmware is not fetched: it is the owner's own dump, kept at
# ext/firmware/dsp4.rom (8192 bytes, SHA-1 af6478aecb6f1b67177e79c82ca04c56250a8c72) and
# never committed. Without it the reference bench uses the clean-room DSP-4.
#
# If ext/bizhawk-1.9.4 has edits that are not in the patch, it refuses to run (they would
# be wiped); save them with
#   git -C ext/bizhawk-1.9.4 diff -- libsnes/bsnes > tools/refbench/patches/bsnes087-refbench.patch
# or run with FORCE=1 to throw them away.
set -eu
root=$(cd "$(dirname "$0")/../.." && pwd)
patch="$root/tools/refbench/patches/bsnes087-refbench.patch"
dir="$root/ext/bizhawk-1.9.4"
tag=6e535d50b66e3dcd11d7ad46fc3fa13d23876a17  # BizHawk 1.9.4
mkdir -p "$root/ext"

if [ -d "$dir/.git" ] && [ "${FORCE:-0}" != 1 ]; then
  if ! git -C "$dir" diff -- libsnes/bsnes | cmp -s - "$patch"; then
    if [ -n "$(git -C "$dir" status --porcelain --untracked-files=no)" ]; then
      echo "fetch.sh: ext/bizhawk-1.9.4 has edits that differ from $patch" >&2
      exit 1
    fi
  fi
fi

if [ ! -d "$dir/.git" ]; then
  git clone -q --filter=blob:none --no-checkout https://github.com/TASEmulators/BizHawk "$dir"
  git -C "$dir" sparse-checkout init --cone
  git -C "$dir" sparse-checkout set libsnes
fi
git -C "$dir" checkout -q -f "$tag"
git -C "$dir" apply "$patch"
echo "ext/bizhawk-1.9.4 at BizHawk 1.9.4 with the refbench patch"
