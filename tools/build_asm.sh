#!/bin/sh
# Assemble asm/ with asar and check that it rebuilds the original ROM byte for byte.
# Needs your ROM (TG3000_ROM, default ./TG3000.sfc) for the incbin'd data, and asar
# (tools/bench/fetch.sh builds it into ext/asar).
#
# Two builds:
#  1. normal: must give the original SHA-1;
#  2. proof: the incbin source is a copy of the ROM with every code byte zeroed, and the
#     result must still be identical - so all code comes from the asm text, not the ROM.
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
rom_in=${TG3000_ROM:-$root/TG3000.sfc}
rom=$(cd "$(dirname "$rom_in")" && pwd)/$(basename "$rom_in")
asar=${ASAR:-$root/ext/asar-build/asar/bin/asar}
want=058aadf1ee719cb9e0c7333e665797f990a6381d
mkdir -p "$root/build"

build() { # baserom output
  head -c 1048576 /dev/zero > "$2"
  "$asar" --no-title-check --fix-checksum=off -Dbaserom="\"$1\"" "$root/asm/main.asm" "$2"
}

build "$rom" "$root/build/tg3000.sfc"
got=$(sha1sum "$root/build/tg3000.sfc" | cut -d' ' -f1)
if [ "$got" != "$want" ]; then
  echo "MISMATCH: built $got, want $want" >&2
  cmp -l "$rom" "$root/build/tg3000.sfc" | head -20 >&2 || true
  exit 1
fi

python3 -I - "$rom" "$root/map/rom_map.txt" "$root/build/nocode.sfc" <<'EOF'
import sys
rom = bytearray(open(sys.argv[1], "rb").read())
for line in open(sys.argv[2]):
    if not line.startswith("#"):
        a, b, kind = line.split()[:3]
        if kind == "code":
            rom[int(a, 16):int(b, 16)] = bytes(int(b, 16) - int(a, 16))
open(sys.argv[3], "wb").write(rom)
EOF
build "$root/build/nocode.sfc" "$root/build/proof.sfc"
proof=$(sha1sum "$root/build/proof.sfc" | cut -d' ' -f1)
rm -f "$root/build/nocode.sfc"
if [ "$proof" != "$want" ]; then
  echo "PROOF FAILED: some code bytes come from the incbin'd ROM, not from asm/" >&2
  exit 1
fi
echo "asm/ rebuilds the ROM byte for byte (SHA-1 $got), all code from the asm text"
