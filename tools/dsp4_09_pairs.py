#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""dsp4-01x: the sprite list's per-line budget on chosen lists, on the firmware chip
(build/bench/dsp4_tape_run), outputs only; and src/dsp4.c's list on the same lists
(build/bench/dsp4_sprite_native).

    make -C tools/bench probe && python3 -I tools/dsp4_09_pairs.py TAPE [LIST]

TAPE is a tape recorded with the firmware (bench --dsp4-firmware --dsp4-tape-out). Its list
number LIST (default 0) gives the stream's opening words and a scenery object's head. Two
measurements:
1. The shift test: the whole list again, its scene moved up or down by d lines (the centre y,
   the top and bottom edges, the dots' y), its parts' answers compared but for their y byte.
2. The probe table: one object, a line filled (33 small parts, or 16 large ones) from line y0,
   then one part at y0 + delta, for delta -17..17: shown (S) or hidden (.) on the firmware, and
   on src/dsp4.c. (The large-probe row sends its probe first, in the object's first list, and
   reports the last small part instead.)
Exit status 0.
"""
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RUN = os.path.join(ROOT, 'build', 'bench', 'dsp4_tape_run')
FIRMWARE = os.environ.get('TG3000_DSP4_FIRMWARE', os.path.join(ROOT, 'ext', 'firmware', 'dsp4.rom'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import dsp4_09_check as c  # noqa: E402

NEED = {'B': 3, 'S': 7, 'L': 1, 'N': 7, 'C': 9, 'T': 2, 'P': 3}


def ops(script):
    """a list's script (dsp4_sprite_native's) as (op, words)"""
    t, j, out = script.split(), 0, []
    while j < len(t):
        op, n = t[j], NEED.get(t[j], 0)
        out.append((op, [int(x, 16) for x in t[j + 1:j + 1 + n]]))
        j += 1 + n
    return out


def W(v):
    return [(0, v & 255, 0), (0, v >> 8 & 255, 0)]


def records(o):
    """the list as tape records: writes, and the answers as dsp4_tape_run's kinds (1 a read,
    3 a part's, 5 a $000B's)"""
    t = []
    for op, a in o:
        words = [x for v in a for x in W(v)]
        t += {'O': W(3), 'Z': W(5), 'B': W(0xB) + words + [(0, 0, 5)], 'S': W(9) + words,
              'L': W(a[0]) + W(0) if a else [], 'N': words, 'C': words + [(0, 0, 1), (0, 0, 1)],
              'T': words, 'P': words + [(0, 0, 3)], 'E': W(0), 'X': W(0) + W(0x8000)}.get(op, [])
    return t


def shifted(o, d):
    out = []
    for op, a in o:
        a = list(a)
        if op == 'B':
            a[1] = (a[1] + d) & 0xFFFF
        if op == 'S':
            for i in (1, 5, 6):
                a[i] = (a[i] + d) & 0xFFFF
        out.append((op, a))
    return out


def firmware_parts(o, path):
    """the firmware's answer to each part of o: its entries (x, y, attribute)"""
    with open(path, 'wb') as f:
        for pc, v, rd in records(o):
            f.write(bytes([pc & 255, pc >> 8 & 255, pc >> 16, v, rd]))
    out = subprocess.run([RUN, FIRMWARE, '0', path], capture_output=True, text=True, check=True,
                         env={'TAPE_RUN_DUMP': '1'}).stdout
    h, i, parts = bytes.fromhex(out.split(': ', 1)[1].strip()), 0, []
    for op, a in o:
        if op == 'C':
            i += 2
        elif op == 'P':
            ents = []
            while True:
                flag = h[i] | h[i + 1] << 8
                i += 2
                if not flag:
                    break
                ents.append((h[i], h[i + 1], h[i + 2] | h[i + 3] << 8))
                i += 4
            parts.append(ents)
    return parts


def native_last(o):
    """whether src/dsp4.c shows the last part of o"""
    s = ' '.join(op + ''.join(' %X' % v for v in a) for op, a in o)
    toks = [int(x, 16) for x in subprocess.run([c.NATIVE], input=s + '\n', capture_output=True, text=True,
                                               check=True).stdout.split()]
    i, last = 0, None
    for op, a in o:
        if op == 'C':
            i += 1
        elif op == 'B':
            i += 3 if toks[i] else 1
        elif op == 'P':
            last = toks[i]
            i += 1 + 2 * toks[i]
    return bool(last)


def main():
    tape = sys.argv[1]
    which = int(sys.argv[2]) if len(sys.argv) > 2 else 0
    scratch = os.path.join(os.environ.get('TMPDIR', '/tmp'), 'dsp4_09_pairs.tape')
    lists = list(c.lists(c.words(tape)))
    o = ops(lists[which][0])
    if 'X' in [op for op, a in o]:
        o = o[:[op for op, a in o].index('X') + 1]
    base = firmware_parts(o, scratch)
    print('list %d: %d parts, %d shown' % (which, len(base), sum(1 for e in base if e)))
    for d in (1, -1, 2, -2, 8, -8):
        got = firmware_parts(shifted(o, d), scratch)
        diff = [n for n, (x, y) in enumerate(zip(base, got)) if [(e[0], e[2]) for e in x] != [(e[0], e[2]) for e in y]]
        print('  shifted %+d: %d part answers differ%s' % (d, len(diff), (' (part %s)' % diff[:4]) if diff else ''))
    s9 = next(x for x in o if x[0] == 'S')
    n0 = next(x for x in o if x[0] == 'N')
    y_obj = None
    for a in range(0, 0x10000, 0x40):  # the object's y: where a part with offset 0 lands
        if firmware_parts([('O', []), ('Z', []), s9, n0, ('E', []), ('P', [0x2000, a, 254]), ('E', []), ('X', [])], scratch)[0]:
            got = firmware_parts([('O', []), ('Z', []), s9, n0, ('E', []), ('P', [0x2000, a, 254]), ('E', []), ('X', [])], scratch)[0][0]
            y_obj = (got[1] - a) & 0xFF
            break
    if y_obj is None:
        print('no part of that object shows: no probe table')
        return 0

    def probe(y0, delta, fill, small_fill, small_probe):
        fills = [('P', [0x2000, (y0 - y_obj) & 0xFFFF, 254])] * fill
        pr = [('P', [0x2001, (y0 + delta - y_obj) & 0xFFFF, 254])]
        l0, l1 = [], []
        (l1 if small_fill else l0).extend(fills)
        (l1 if small_probe else l0).extend(pr)
        return [('O', []), ('Z', []), s9, n0] + l0 + [('E', [])] + l1 + [('E', []), ('X', [])]
    rows = ((True, True, 33, 'small full, small probe'), (False, False, 16, 'large full, large probe'),
            (False, True, 16, 'large full, small probe'), (True, False, 33, 'small full, large first'))
    for small_fill, small_probe, fill, name in rows:
        for y0 in (128, 129):
            fw = ''.join('S' if firmware_parts(probe(y0, d, fill, small_fill, small_probe), scratch)[-1] else '.'
                         for d in range(-17, 18))
            nat = ''.join('S' if native_last(probe(y0, d, fill, small_fill, small_probe)) else '.' for d in range(-17, 18))
            print('  %-24s y0 %d  firmware %s  src/dsp4.c %s' % (name, y0, fw, nat))
    return 0


if __name__ == '__main__':
    sys.exit(main())
