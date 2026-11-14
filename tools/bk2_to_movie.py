#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Convert a BizHawk SNES movie (.bk2) into the bench's movie format (player 1 only).

    python3 -I tools/bk2_to_movie.py MOVIE.bk2 OUT.txt [--offset N]

Community TAS movies are not stored in this repository (their licence is their authors');
download them yourself (see docs/DECISIONS.md, "Community TAS movies"). The bench movie
has one line per change of held buttons: "<frame> <buttons>", buttons B Y s S U D L R A X l r.
--offset shifts every frame number (BizHawk and the bench may count the first frame
differently).
"""
import gzip
import io
import sys
import zipfile

# BizHawk's SNES log key: Up Down Left Right Select Start Y B X A L R
BK2_ORDER = "UDLRsSYBXAlr"


def open_bk2(path):
    data = open(path, "rb").read()
    if data[:2] == b"\x1f\x8b":  # TASVideos serves some user files gzip-wrapped
        data = gzip.decompress(data)
    return zipfile.ZipFile(io.BytesIO(data))


def main():
    if len(sys.argv) < 3:
        print(__doc__, file=sys.stderr)
        return 2
    src, dst = sys.argv[1], sys.argv[2]
    offset = int(sys.argv[sys.argv.index("--offset") + 1]) if "--offset" in sys.argv else 0
    z = open_bk2(src)
    header = dict(line.split(" ", 1) for line in z.read("Header.txt").decode().splitlines() if " " in line)
    lines = z.read("Input Log.txt").decode().splitlines()
    out = [f"# Converted from {header.get('GameName', '?')} by {header.get('Author', '?')} "
           f"(BizHawk {header.get('Core', '?')} core), {len(lines)} log lines; "
           f"ROM SHA-1 {header.get('SHA1', '?')}"]
    last = None
    frame = 0
    for line in lines:
        if not line.startswith("|") or line.startswith("|LogKey"):
            continue
        fields = line.strip("|").split("|")
        if len(fields) < 2:
            continue
        p1 = fields[1]
        held = "".join(ch for ch, mark in zip(BK2_ORDER, p1) if mark != ".")
        buttons = held or "."
        if buttons != last:
            out.append(f"{frame + offset} {buttons}")
            last = buttons
        frame += 1
    with open(dst, "w") as f:
        f.write("\n".join(out) + "\n")
    print(f"{frame} frames, {len(out) - 1} changes -> {dst}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
