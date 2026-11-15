#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Turn the bench's coverage file into the committed code/data map.

    python3 -I tools/map.py [traces/coverage.bin]

Writes:
  map/rom_map.txt      "<start> <end exclusive> <code|data|unknown> <bytes>" ranges
                       over the 1 MB ROM (file offsets, hex)
  map/code_starts.txt  every executed instruction start: "<offset> <BB:AAAA> <widths>"
                       widths = the M/X states seen there, e.g. "m8x8,m16x8" (+",emu")
  map/ram_exec.txt     PB:PC addresses executed outside ROM (code copied to RAM)

Ranges in map/static_code.txt (code no trusted run executed, known to be code from other
evidence) are decoded linearly and marked "static" in rom_map.txt; their instruction
starts go into code_starts.txt like executed ones. They must lie on unknown bytes.

Checks (exit 1 on failure): no instruction start lies inside the operand of another
start under the same widths, and the file is exactly 1 MB.
"""
import os
import shutil
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROM_SIZE = 0x100000
START, CODE, READ, M8X8, M8X16, M16X8, M16X16, EMU = 1, 2, 4, 8, 16, 32, 64, 128
WIDTHS = ((M8X8, "m8x8", True, True), (M8X16, "m8x16", True, False),
          (M16X8, "m16x8", False, True), (M16X16, "m16x16", False, False))

# 65816 lengths; -1 = accumulator-sized immediate, -2 = index-sized (same table as coverage.c)
OPLEN = [
    2, 2, 2, 2, 2, 2, 2, 2, 1, -1, 1, 1, 3, 3, 3, 4,
    2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
    3, 2, 4, 2, 2, 2, 2, 2, 1, -1, 1, 1, 3, 3, 3, 4,
    2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
    1, 2, 2, 2, 3, 2, 2, 2, 1, -1, 1, 1, 3, 3, 3, 4,
    2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4,
    1, 2, 3, 2, 2, 2, 2, 2, 1, -1, 1, 1, 3, 3, 3, 4,
    2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
    2, 2, 3, 2, 2, 2, 2, 2, 1, -1, 1, 1, 3, 3, 3, 4,
    2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
    -2, 2, -2, 2, 2, 2, 2, 2, 1, -1, 1, 1, 3, 3, 3, 4,
    2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
    -2, 2, 2, 2, 2, 2, 2, 2, 1, -1, 1, 1, 3, 3, 3, 4,
    2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
    -2, 2, 2, 2, 2, 2, 2, 2, 1, -1, 1, 1, 3, 3, 3, 4,
    2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
]


def length(op, m8, x8):
    n = OPLEN[op]
    return (2 if m8 else 3) if n == -1 else (2 if x8 else 3) if n == -2 else n


def cpu_addr(off):
    """FastROM mirror ($80-$FF) for a LoROM file offset, as the game runs from it."""
    return f"{0x80 | (off >> 15):02X}:{0x8000 | (off & 0x7FFF):04X}"


def main():
    cov_path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, "traces", "coverage.bin")
    rom_path = os.environ.get("TG3000_ROM", os.path.join(ROOT, "TG3000.sfc"))
    cov = open(cov_path, "rb").read()
    rom = open(rom_path, "rb").read()
    if len(cov) != ROM_SIZE or len(rom) != ROM_SIZE:
        print("coverage file and ROM must both be 1 MB", file=sys.stderr)
        return 1

    # consistency: a start must not sit inside another start's operand (same widths)
    bad = []
    for off in range(ROM_SIZE):
        f = cov[off]
        if not f & START:
            continue
        for bit, _, m8, x8 in WIDTHS:
            if f & bit:
                n = length(rom[off], m8, x8)
                for k in range(1, n):
                    o = off + k
                    if o < ROM_SIZE and (o >> 15) == (off >> 15) and cov[o] & START and cov[o] & bit:
                        bad.append((off, o))
    if bad:
        for a, b in bad[:20]:
            print(f"overlap: start {a:06X} ({cpu_addr(a)}) covers start {b:06X}", file=sys.stderr)
        print(f"{len(bad)} overlapping starts", file=sys.stderr)
        return 1

    out = os.path.join(ROOT, "map")
    os.makedirs(out, exist_ok=True)

    # statically found code: decode each range with its widths, following REP/SEP
    static = bytearray(ROM_SIZE)          # 1 = byte of a static range
    static_starts = {}                    # offset -> width bit
    static_path = os.path.join(out, "static_code.txt")
    if os.path.exists(static_path):
        for line in open(static_path):
            line = line.split(";")[0].split("#")[0].split()
            if not line:
                continue
            a, b = (((int(x[:2], 16) & 0x7F) << 15) | (int(x[3:], 16) & 0x7FFF) for x in line[:2])
            m8, x8 = line[2].startswith("m8"), line[2].endswith("x8")
            if any(cov[o] & (CODE | READ) for o in range(a, b)):
                print(f"static range {line[0]}..{line[1]} overlaps executed code or read data", file=sys.stderr)
                return 1
            o = a
            while o < b:
                bit = next(bt for bt, _, wm, wx in WIDTHS if wm == m8 and wx == x8)
                static_starts[o] = bit
                op, arg = rom[o], rom[o + 1]
                n = length(op, m8, x8)
                if op == 0xC2:
                    m8, x8 = m8 and not arg & 0x20, x8 and not arg & 0x10
                if op == 0xE2:
                    m8, x8 = m8 or bool(arg & 0x20), x8 or bool(arg & 0x10)
                o += n
            if o != b:
                print(f"static range {line[0]}..{line[1]}: the last instruction runs past its end", file=sys.stderr)
                return 1
            for o in range(a, b):
                static[o] = 1

    def kind(off):
        f = cov[off]
        return "code" if f & CODE else "data" if f & READ else "static" if static[off] else "unknown"

    counts = {"code": 0, "data": 0, "static": 0, "unknown": 0}
    with open(os.path.join(out, "rom_map.txt"), "w") as fh:
        fh.write("# start end(excl) kind bytes  -- ROM file offsets; generated by tools/map.py\n")
        start = 0
        cur = kind(0)
        for off in range(1, ROM_SIZE + 1):
            k = kind(off) if off < ROM_SIZE else None
            if k != cur:
                fh.write(f"{start:06X} {off:06X} {cur} {off - start}\n")
                counts[cur] += off - start
                start, cur = off, k

    with open(os.path.join(out, "code_starts.txt"), "w") as fh:
        fh.write("# offset cpu widths  -- executed instruction starts; generated by tools/map.py\n")
        for off in range(ROM_SIZE):
            f = cov[off] | (START | static_starts[off] if off in static_starts else 0)
            if f & START:
                w = [name for bit, name, _, _ in WIDTHS if f & bit] + (["emu"] if f & EMU else [])
                fh.write(f"{off:06X} {cpu_addr(off)} {','.join(w)}\n")

    ram_src = cov_path + ".ram"
    if os.path.exists(ram_src):
        shutil.copyfile(ram_src, os.path.join(out, "ram_exec.txt"))

    print(f"code {counts['code']:,}  static {counts['static']:,}  data {counts['data']:,}  unknown {counts['unknown']:,}  "
          f"({100 * (counts['code'] + counts['data']) / ROM_SIZE:.2f}% classified)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
