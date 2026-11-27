#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""dsp4-09e: the sprite stream $09's scenery parts, from a tape recorded with the firmware chip
(bench --dsp4-firmware --dsp4-tape-out), against a projection rule. Outputs only.

    python3 -I tools/dsp4_09_scenery.py TAPE

A scenery object (sprite_put_rows, $82:D934) sends 7 words: two row lines, the row's height h,
the road's x P, the object's sideways place D+$20, D+$22, its attribute D+$26 (with flags); then
two lists of parts (word w, a, b; w = 0 ends a list). The chip answers each part with a flag,
and per 1 two words and another flag. The rule tested on each entry whose second word is not
$00EE:
    x = $80 - P + (D+$20 * h >> 15) + b,  y = $A9 + line + a + (D+$22 * h >> 15),
    the entry: (x & $FF) | (y & $FF) << 8, then D+$26 + w.
($80 and $A9 are the stream's first two words after $0009.) Entries of $00EE (tile $EE) come
before a part's own, at its x, a few lines above (their rule is not known yet). Exit 0 when
every other entry follows the rule and every part with x outside -8..255 is hidden.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsp4_09_deps import frames_of, load  # noqa: E402


def s16(v):
    return v - 0x10000 if v & 0x8000 else v


def words(r, a, b):
    out, k = [], a
    while k < b - 1:
        out.append((r[k][0], r[k][1] | r[k + 1][1] << 8, r[k][2]))
        k += 2
    return out


def scenery(r, frame):
    """the frame's scenery objects: (head, [(list, w, a, b, answers)])"""
    a0, k1, k7, k8, k9, e = frame
    ws, i, objs = words(r, k9, e), 0, []
    while i < len(ws):
        if ws[i][0] != 0x82D93D or ws[i][2]:  # (the bench logs the instruction after the write)
            i += 1
            continue
        head = [ws[i + q][1] for q in range(7)]
        i += 7
        parts = []
        for lst in range(2):
            w = ws[i][1]
            i += 1
            while w != 0:
                pa, pb = ws[i][1], ws[i + 1][1]
                i += 2
                out, f = [], ws[i][1]
                i += 1
                while f != 0:
                    out.append((ws[i][1], ws[i + 1][1]))
                    i += 2
                    f = ws[i][1]
                    i += 1
                parts.append((lst, w, pa, pb, out))
                w = ws[i][1]
                i += 1
        objs.append((head, parts))
    return objs


def main():
    r = load(sys.argv[1])
    n = entries = exact = ee = offHidden = offShown = onHidden = 0
    for frame in frames_of(r):
        for (l2, h, P, l1, d20, d22, att), parts in scenery(r, frame):
            for lst, w, a, b, out in parts:
                n += 1
                x = 0x80 - s16(P) + (s16(d20) * h >> 15) + s16(b)
                y = 0xA9 + s16(l1) + s16(a) + (s16(d22) * h >> 15)
                on = -8 <= x <= 255
                for o in out:
                    if o[1] == 0x00EE:
                        ee += 1
                        continue
                    entries += 1
                    exact += o == ((x & 0xFF) | (y & 0xFF) << 8, (att + w) & 0xFFFF) and l1 == l2
                if out:
                    offShown += not on
                else:
                    offHidden += not on
                    onHidden += on
    print('scenery parts %d: entries %d (the rule exact for %d), $00EE entries %d; parts hidden %d off screen '
          'and %d on it; shown off screen %d' % (n, entries, exact, ee, offHidden, onHidden, offShown))
    return 0 if n and exact == entries and offShown == 0 else 1


if __name__ == '__main__':
    sys.exit(main())
