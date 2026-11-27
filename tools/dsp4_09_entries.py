#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""dsp4-09h: every scenery and car part of the sprite stream $09 against the whole entry rule,
from a tape recorded with the firmware chip (bench --dsp4-firmware --dsp4-tape-out). Outputs only.

    python3 -I tools/dsp4_09_entries.py TAPE

A part at screen (x, y) (tools/dsp4_09_scenery.py, tools/dsp4_09_cars.py; a car's line is its
two rows' lines interpolated as its x, L = lB + ((lA - lB) * (distance >> 8) >> 8), then
y = $A9 + (L * h >> 15) + a + (height * h >> 15)), with the crest c (dsp4-09k: the lowest $A9 +
first word of every object and empty slot since the stream's $0009), and a size of 16 lines in its first list of parts, 8 in its second:
    x outside -8..255, or c above y: no entry (the part hidden);
    c within the size (c < y + size): first ((x | c << 8), $00EE), then the part's entry;
    otherwise the part's entry alone.
Prints, per kind, the parts the rule gets right, the parts hidden where it shows them (the
per-line budget, not modelled yet) and the others. Exit 0 when there are no others.
"""
import collections
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsp4_09_scenery import frames_of, load, s16, words  # noqa: E402


def predict(x, y, c, attr, size):
    if not -8 <= x <= 255 or c < y:
        return []
    out = [((x & 0xFF) | (c & 0xFF) << 8, 0x00EE)] if c < y + size else []
    return out + [((x & 0xFF) | (y & 0xFF) << 8, attr)]


def parts(r, frame):
    """every part in the stream's order: (kind, list, x, y, crest, attribute, the chip's entries);
    a bar's part (sprite_part_put, $81:8527) as ('bar', 1, None, None, None, None, entries).
    dsp4-09k: the crest is the lowest $A9 + first word of every object and empty slot sent since
    the stream's $0009 (the slots: a line and a 0, $82:CEF4); it starts again at each stream."""
    a0, k1, k7, k8, k9, e = frame
    ws, i, cur, lo = words(r, k9, e), 0, None, None

    def answers(i):
        out, f = [], ws[i][1]
        i += 1
        while f != 0:
            out.append((ws[i][1], ws[i + 1][1]))
            i += 2
            f = ws[i][1]
            i += 1
        return out, i

    def seen(v):
        nonlocal lo
        c = 0xA9 + s16(v)
        lo = c if lo is None else min(lo, c)
    lists = {0x82DDD1: 0, 0x82DE21: 1, 0x82DEBA: 0, 0x82DF0A: 1}  # a part's a (the bench's PCs)
    try:
        while i < len(ws):
            pc, v, rd = ws[i]
            if rd:
                i += 1
            elif (pc == 0x82D1F0 and v == 0x8000) or (v == 9 and ws[i + 1][1] == 0 and i and ws[i - 1][2]):
                lo, cur = None, None  # a stream's end, or another $0009 (the 60k movie's VS views)
                i += 1
            elif pc == 0x82CEF7:  # an empty slot: its line
                seen(v)
                i += 1
            elif pc == 0x82D93D:  # a scenery object
                cur = ('scenery', [ws[i + q][1] for q in range(7)])
                seen(v)
                i += 7
            elif pc in (0x82D26E, 0x82D08F):  # a car
                cur = ('car', [ws[i + q][1] for q in range(9)], ws[i + 9][1], ws[i + 10][1], ws[i + 11][1])
                seen(v)
                i += 12
            elif pc == 0x81852A:  # a bar's part: x, y, attribute, then its answers
                out, i = answers(i + 3)
                yield ('bar', 1, None, None, None, None, out)
            elif pc in lists and cur is not None:
                w, a, b = ws[i - 1][1], s16(ws[i][1]), s16(ws[i + 1][1])
                out, i = answers(i + 2)
                if cur[0] == 'scenery':
                    l2, h, P, l1, d20, d22, att = cur[1]
                    x = 0x80 - s16(P) + (s16(d20) * h >> 15) + b
                    y = 0xA9 + s16(l1) + a + (s16(d22) * h >> 15)
                    yield ('scenery', lists[pc], x, y, lo, (att + w) & 0xFFFF, out)
                else:
                    line, _, d, lA, lB, _, _, h, _ = cur[1]
                    ans, hgt, attr = cur[2], cur[3], cur[4]
                    L = s16(lB) + ((s16(lA) - s16(lB)) * (d >> 8) >> 8)
                    x = 0x80 + (s16(ans) * h >> 15) + b
                    y = 0xA9 + (L * h >> 15) + a + (s16(hgt) * h >> 15)
                    yield ('car', lists[pc], x, y, lo, (attr + w) & 0xFFFF, out)
            else:
                i += 1
    except IndexError:  # a stream cut off by the tape's end
        return


def main():
    r = load(sys.argv[1])
    n = collections.Counter()
    for frame in frames_of(r):
        for kind, lst, x, y, c, attr, out in parts(r, frame):
            if kind == 'bar':
                continue
            p = predict(x, y, c, attr, 16 if lst == 0 else 8)
            n[(kind, 'right' if p == out else 'hidden (budget)' if p and not out else 'other')] += 1
    for kind in ('car', 'scenery'):
        print('%-8s right %d, hidden where shown %d, other %d' % (
            kind, n[(kind, 'right')], n[(kind, 'hidden (budget)')], n[(kind, 'other')]))
    return 0 if sum(v for (k, cls), v in n.items() if cls == 'other') == 0 else 1


if __name__ == '__main__':
    sys.exit(main())
