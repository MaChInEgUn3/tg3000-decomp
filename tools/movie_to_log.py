#!/usr/bin/env python3
"""Turn a bench movie into a BizHawk-style input log for the reference runner.

    python3 -I tools/movie_to_log.py MOVIE STEPS > log.txt
    build/refbench/refbench TG3000.sfc log.txt --firmware ext/firmware/dsp4.rom ...

MOVIE: "<step> <buttons>" lines (tools/bench/bench.c: the buttons held from that step on,
letters from "BYsSUDLRAXlr", "." for none). STEPS: how many log lines to write. Each line is
one frame of pad 1, "|..|" then the twelve buttons in BizHawk's order (Up Down Left Right
Select Start Y B X A L R), "." where not held; no reset or power. The menus line up with
the bench's steps (docs/DECISIONS.md explore-9).
"""
import sys

ORDER = "UDLRsSYBXAlr"


def main():
    path, steps = sys.argv[1], int(sys.argv[2])
    events = []
    for line in open(path):
        if line.startswith("#") or not line.strip():
            continue
        parts = line.split()
        events.append((int(parts[0]), parts[1] if len(parts) > 1 else "."))
    events.sort()
    held, k = ".", 0
    out = ["[Input]"]
    for step in range(steps):
        while k < len(events) and events[k][0] == step:
            held = events[k][1]
            k += 1
        out.append("|..|" + "".join(c if c in held else "." for c in ORDER) + "|")
    sys.stdout.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
