#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""dsp4-09h: every scenery and car part of the sprite stream $09 against the whole entry rule,
from a tape recorded with the firmware chip (bench --dsp4-firmware --dsp4-tape-out). Outputs only.

    python3 -I tools/dsp4_09_entries.py TAPE

A part at screen (x, y) (tools/dsp4_09_scenery.py, tools/dsp4_09_cars.py; a car's line is its
two rows' lines interpolated as its x, L = lB + ((lA - lB) * (distance >> 8) >> 8), then
y = $A9 + (L * h >> 15) + a + (height * h >> 15)), with the crest c = $A9 + the object's first
word, and a size of 16 lines in its first list of parts, 8 in its second:
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
from dsp4_09_cars import cars  # noqa: E402
from dsp4_09_scenery import frames_of, load, s16, scenery  # noqa: E402


def predict(x, y, c, attr, size):
    if not -8 <= x <= 255 or c < y:
        return []
    out = [((x & 0xFF) | (c & 0xFF) << 8, 0x00EE)] if c < y + size else []
    return out + [((x & 0xFF) | (y & 0xFF) << 8, attr)]


def parts(r, frame):
    """(kind, list, x, y, crest, attribute, the chip's entries) of every part"""
    for kind, head, ans, (hgt, attr), ps in cars(r, frame):
        line, _, d, lA, lB, _, _, h, _ = head
        L = s16(lB) + ((s16(lA) - s16(lB)) * (d >> 8) >> 8)
        for lst, w, a, b, out in ps:
            yield (kind, lst, 0x80 + (s16(ans) * h >> 15) + s16(b),
                   0xA9 + (L * h >> 15) + s16(a) + (s16(hgt) * h >> 15), 0xA9 + s16(line), (attr + w) & 0xFFFF, out)
    for (l2, h, P, l1, d20, d22, att), ps in scenery(r, frame):
        for lst, w, a, b, out in ps:
            yield ('scenery', lst, 0x80 - s16(P) + (s16(d20) * h >> 15) + s16(b),
                   0xA9 + s16(l1) + s16(a) + (s16(d22) * h >> 15), 0xA9 + s16(l2), (att + w) & 0xFFFF, out)


def main():
    r = load(sys.argv[1])
    n = collections.Counter()
    for frame in frames_of(r):
        for kind, lst, x, y, c, attr, out in parts(r, frame):
            p = predict(x, y, c, attr, 16 if lst == 0 else 8)
            n[(kind, 'right' if p == out else 'hidden (budget)' if p and not out else 'other')] += 1
    for kind in ('player', 'rival', 'scenery'):
        print('%-8s right %d, hidden where shown %d, other %d' % (
            kind, n[(kind, 'right')], n[(kind, 'hidden (budget)')], n[(kind, 'other')]))
    return 0 if sum(v for (k, cls), v in n.items() if cls == 'other') == 0 else 1


if __name__ == '__main__':
    sys.exit(main())
