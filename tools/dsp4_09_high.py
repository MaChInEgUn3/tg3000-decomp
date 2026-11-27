#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""dsp4-09j: the high table $0006 answers after a sprite stream, against the entries the stream
gave, from a tape recorded with the firmware chip (bench --dsp4-firmware --dsp4-tape-out).
Outputs only.

    python3 -I tools/dsp4_09_high.py TAPE

The rule: 16 words, 2 bits an entry (8 a word, the first in bits 0-1), in the list's order: the
two $000B dots of the frame, then every entry the $0009 stream answered (its $00EE masks too).
The low bit is set when the part's x lies outside 0..255 (the screen x's ninth bit), the high
bit when the part is in its object's first list (a large sprite; the game sends $0005 in every
race frame). Exit 0 when every frame's table follows it.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsp4_09_budget import ordered  # noqa: E402
from dsp4_09_scenery import frames_of, load, words  # noqa: E402


def main():
    r = load(sys.argv[1])
    n = exact = 0
    for frame in frames_of(r):
        a0, k1, k7, k8, k9, e = frame
        ws = words(r, k9, e)
        at = [i for i, w in enumerate(ws) if w[0] == 0x82AE1F and not w[2]]  # the $0006 word
        if not at or at[0] + 16 >= len(ws):
            continue
        table = [ws[at[0] + 1 + q][1] for q in range(16)]
        bits = [0, 0]  # the two dots: small, on screen
        for kind, lst, x, y, c, attr, out in ordered(r, frame):
            far = 1 if x is not None and not 0 <= x <= 255 else 0
            bits += [far | (2 if lst == 0 else 0)] * len(out)
        model = [0] * 16
        for i, b in enumerate(bits[:128]):
            model[i // 8] |= b << (2 * (i % 8))
        n += 1
        exact += model == table
    print('frames with a high table: %d, the rule exact for %d' % (n, exact))
    return 0 if n and n == exact else 1


if __name__ == '__main__':
    sys.exit(main())
