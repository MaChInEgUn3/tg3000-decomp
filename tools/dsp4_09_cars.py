#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""dsp4-09f: the sprite stream $09's cars, from a tape recorded with the firmware chip (bench
--dsp4-firmware --dsp4-tape-out), against the screen x rule. Outputs only.

    python3 -I tools/dsp4_09_cars.py TAPE

A car object (the player's, car_sprite_head $82:D267, and the rivals', rival_car_sprites)
sends 9 words: a row's line, $9000, its distance, two rows' lines, two rows' x, the height h
and its sideways place; the chip answers one word; then the car's height and attribute; then
two lists of parts from the ROM (w, a, b), as the scenery's (tools/dsp4_09_scenery.py). The
rule tested on each entry that is not $00EE:
    x = $80 + (answer * h >> 15) + b,   the entry's second word: attribute + w.
So the answer is the car's place across the screen before scaling; how the chip gets it from
the 9 words, and the entry's line, are not known yet. Exit 0 when every entry follows the rule.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsp4_09_scenery import frames_of, load, s16, words  # noqa: E402

HEADS = {0x82D26E: 'player', 0x82D08F: 'rival'}  # (the bench logs the instruction after the write)


def parts_rom(ws, i):
    parts = []
    for lst in range(2):
        w = ws[i][1]
        i += 1
        while w != 0:
            a, b = ws[i][1], ws[i + 1][1]
            i += 2
            out, f = [], ws[i][1]
            i += 1
            while f != 0:
                out.append((ws[i][1], ws[i + 1][1]))
                i += 2
                f = ws[i][1]
                i += 1
            parts.append((lst, w, a, b, out))
            w = ws[i][1]
            i += 1
    return parts, i


def cars(r, frame):
    """the frame's cars: (kind, 9 words, answer, (height, attribute), parts)"""
    a0, k1, k7, k8, k9, e = frame
    ws, i, out = words(r, k9, e), 0, []
    while i < len(ws):
        pc, v, rd = ws[i]
        if pc in HEADS and not rd:
            head = [ws[i + q][1] for q in range(9)]
            try:
                parts, j = parts_rom(ws, i + 12)
            except IndexError:  # a stream cut off by the tape's end
                break
            out.append((HEADS[pc], head, ws[i + 9][1], (ws[i + 10][1], ws[i + 11][1]), parts))
            i = j
        else:
            i += 1
    return out


def main():
    r = load(sys.argv[1])
    n = {'player': [0, 0, 0], 'rival': [0, 0, 0]}  # entries, x exact, attribute exact
    for frame in frames_of(r):
        for kind, head, ans, (hgt, attr), parts in cars(r, frame):
            h = head[7]
            for lst, w, a, b, out in parts:
                for o in out:
                    if o[1] == 0x00EE:
                        continue
                    n[kind][0] += 1
                    n[kind][1] += (o[0] & 0xFF) == ((0x80 + (s16(ans) * h >> 15) + s16(b)) & 0xFF)
                    n[kind][2] += o[1] == ((attr + w) & 0xFFFF)
    for kind, (e, x, at) in n.items():
        print('%s: %d entries, x exact %d, attribute exact %d' % (kind, e, x, at))
    return 0 if all(e and e == x == at for e, x, at in n.values()) else 1


if __name__ == '__main__':
    sys.exit(main())
