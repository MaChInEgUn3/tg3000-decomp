#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""dsp4-09i: the sprite stream $09's per-line budget, measured on chosen inputs on the firmware
chip (build/bench/dsp4_tape_run, TAPE_RUN_DUMP), then simulated over a tape recorded with the
firmware (bench --dsp4-firmware --dsp4-tape-out). Outputs only.

    make -C tools/bench probe && python3 -I tools/dsp4_09_budget.py [TAPE]

The rule: each screen line holds at most 33 eight-pixel slivers. A part from an object's first
list takes 2 a line for 16 lines, from its second 1 for 8; its $00EE mask, when it has one,
counts as another entry of the same size. The chip walks the part's lines from the top: at
the first line that would pass 33 it hides the part, and the lines before stay counted.
The chosen inputs below (scenery objects with one part each, crest far below) check that;
exit 0 when they all hold. With TAPE, it also prints how many parts of every frame the rule
gets right (the parts' place from tools/dsp4_09_entries.py; a hidden part's place is the
rule's, never seen).
"""
import collections
import os
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dsp4_09_entries import predict  # noqa: E402
from dsp4_09_scenery import frames_of, load, s16, words  # noqa: E402

RUN = os.path.join(ROOT, 'build', 'bench', 'dsp4_tape_run')
FW = os.path.join(ROOT, 'ext', 'firmware', 'dsp4.rom')
PRE = [0x0080, 0x00A9, 0, 0, 0x0100, 0x0004, 0x00DC]  # the game's words after $0009


def W(v):
    return [(0, v & 0xFF, 0), (0, (v >> 8) & 0xFF, 0)]


def stream(objs):
    """$0003, $0005, $0009 and its words, then scenery objects (one part each); every part's
    answer read with a direction-3 record"""
    t = W(3) + W(5) + W(9) + [x for w in PRE for x in W(w)]
    frm = len(t)
    for x_off, lst, y in objs:
        line = 0x20  # the crest far below: no masks
        t += [x for w in (line, 0x093F, 4, line, x_off & 0xFFFF, 0, 0x19FA) for x in W(w)]
        for k in (0, 1):
            if k == lst:
                t += W(0x2002) + W((y - 0xA9 - line) & 0xFFFF) + W(0xFFFD) + [(0, 0, 3)]
            t += W(0)
    return t, frm


def shown(cases):
    """for each case, which parts the chip showed"""
    tmp = tempfile.mkdtemp()
    args = []
    for i, (t, frm) in enumerate(cases):
        p = os.path.join(tmp, '%d.tape' % i)
        with open(p, 'wb') as f:
            for pc, v, rd in t:
                f.write(bytes([pc & 255, pc >> 8 & 255, pc >> 16, v, rd]))
        args.append('%s@%d' % (p, frm))
    out = subprocess.run([RUN, FW, '0'] + args, capture_output=True, text=True, check=True,
                         env=dict(os.environ, TAPE_RUN_DUMP='1')).stdout.splitlines()
    res = []
    for line in out:
        b = bytes.fromhex(line.split(': ')[1])
        ws = [b[j] | b[j + 1] << 8 for j in range(0, len(b) - 1, 2)]
        got, i = [], 0
        while i < len(ws):
            n = 0
            while ws[i] != 0:
                n += 1
                i += 3
            i += 1
            got.append(n > 0)
        res.append(got)
    return res


def ordered(r, frame):
    """the frame's parts in the stream's order: (kind, list, x, y, crest, attribute, answers);
    a bar's part (sprite_part_put, $81:8527) as ('bar', 1, ..., answers)"""
    a0, k1, k7, k8, k9, e = frame
    ws, i, cur = words(r, k9, e), 0, None

    def answers(i):
        out, f = [], ws[i][1]
        i += 1
        while f != 0:
            out.append((ws[i][1], ws[i + 1][1]))
            i += 2
            f = ws[i][1]
            i += 1
        return out, i
    lists = {0x82DDD1: 0, 0x82DE21: 1, 0x82DEBA: 0, 0x82DF0A: 1}  # a part's a (the bench's PCs)
    try:
        while i < len(ws):
            pc, v, rd = ws[i]
            if rd:
                i += 1
            elif pc == 0x82D93D:  # a scenery object
                cur = ('scenery', [ws[i + q][1] for q in range(7)])
                i += 7
            elif pc in (0x82D26E, 0x82D08F):  # a car
                cur = ('car', [ws[i + q][1] for q in range(9)], ws[i + 9][1], ws[i + 10][1], ws[i + 11][1])
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
                    yield ('scenery', lists[pc], x, y, 0xA9 + s16(l2), (att + w) & 0xFFFF, out)
                else:
                    line, _, d, lA, lB, _, _, h, _ = cur[1]
                    ans, hgt, attr = cur[2], cur[3], cur[4]
                    L = s16(lB) + ((s16(lA) - s16(lB)) * (d >> 8) >> 8)
                    x = 0x80 + (s16(ans) * h >> 15) + b
                    y = 0xA9 + (L * h >> 15) + a + (s16(hgt) * h >> 15)
                    yield ('car', lists[pc], x, y, 0xA9 + s16(line), (attr + w) & 0xFFFF, out)
            else:
                i += 1
    except IndexError:  # a stream cut off by the tape's end
        return


def place(T, p, size):
    """the rule: True when the part is shown; T, the slivers a line, updated"""
    k = 2 if size == 16 else 1
    for xy, attr in p:
        for line in range(xy >> 8, (xy >> 8) + size):
            if T[line & 0xFF] + k > 33:
                return False
            T[line & 0xFF] += k
    return True


def main():
    fill = [(-1500 + 40 * i, 1, 164) for i in range(33)]
    group = [(-1500 + 40 * i, 1, 156) for i in range(34)]
    checks = [
        ('34 small on one line: 33 shown', [(-1500 + 40 * i, 1, 164) for i in range(34)], 33),
        ('12 large on one line (2 a line, no masks): 12 shown', [(-1500 + 40 * i, 0, 164) for i in range(12)], 12),
        ('1 large then 33 small: 31 small shown', [(0, 0, 164)] + [(-1400 + 40 * i, 1, 164) for i in range(33)], 32),
        ('a part hidden at 160 keeps 160-163: 32 of 34 at 156', fill + [(0, 1, 160)] + group, 33 + 32),
        ('3 hidden at 160: 30 of 34 at 156', fill + [(0, 1, 160)] * 3 + group, 33 + 30),
        ('hidden at its first line keeps none: 33 of 34 at 172', fill + [(0, 1, 168)] + [(-1500 + 40 * i, 1, 172) for i in range(34)], 33 + 33),
    ]
    got = shown([stream(objs) for name, objs, n in checks])
    ok = True
    for (name, objs, n), g in zip(checks, got):
        T = collections.Counter()
        model = sum(place(T, [((y & 0xFF) << 8, 0)], 16 if lst == 0 else 8) for x, lst, y in objs)
        good = sum(g) == n == model
        ok = ok and good
        print('%-60s chip %d, rule %d, expected %d%s' % (name, sum(g), model, n, '' if good else '  <-- differs'))
    if len(sys.argv) > 1:
        r = load(sys.argv[1])
        st = collections.Counter()
        for frame in frames_of(r):
            T = collections.Counter()
            for kind, lst, x, y, c, attr, out in ordered(r, frame):
                size = 16 if lst == 0 else 8
                if kind == 'bar':  # (their lines are the HUD's, apart from the road's)
                    for xy, at in out:
                        for line in range(xy >> 8, (xy >> 8) + 8):
                            T[line & 0xFF] += 1
                    continue
                p = predict(x, y, c, attr, size)
                if not p:
                    continue
                st[(kind, place(T, p, size) == bool(out))] += 1
        for kind in ('car', 'scenery'):
            print('%-8s right %d, wrong %d' % (kind, st[(kind, True)], st[(kind, False)]))
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
