#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""dsp4-09l: src/dsp4.c's sprite list ($0003, $0005, $000B, $0009, $0006) against a tape recorded
with the firmware chip (bench --dsp4-firmware --dsp4-tape-out), list by list. Outputs only.

    make -C tools/bench probe && python3 -I tools/dsp4_09_check.py TAPE

A list runs from a $0003 command word to the $0006's 16 answers. Its commands are read by the
chip's own grammar, not by the game's PCs: $0005; $000B (3 words; a flag, 2 words when shown);
$0009 (7 words), then objects by their first two words ((line, 0) a slot; (line, $9000) a car:
7 more words, its answer, 2 more; (0, $8000) the end; otherwise scenery, 5 more), each with two
lists of parts (w, a, b; w = 0 ends a list; a flag, then per 1 an entry and another flag);
then $0006. Multiplies ($0000) and $000A between them are read past. Each list's calls go to build/bench/dsp4_sprite_native, and every word the game
read is compared. Prints the lists that agree and, of the others, the first difference's kind.
Exit 0 when every list agrees.
"""
import collections
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
NATIVE = os.path.join(ROOT, 'build', 'bench', 'dsp4_sprite_native')


def words(path, at=None):
    """the tape's bytes paired into words (two writes or two reads in a row): (value, read); with
    a list AT, each word's first record goes into it (dsp4-01v)"""
    t = open(path, 'rb').read()
    recs = [(t[i + 3], t[i + 4] & 1) for i in range(0, len(t) - 4, 5)]
    out, i = [], 0
    while i + 1 < len(recs):
        if recs[i][1] == recs[i + 1][1]:
            out.append((recs[i][0] | recs[i + 1][0] << 8, recs[i][1]))
            if at is not None:
                at.append(i)
            i += 2
        else:
            i += 1
    return out


class Cut(Exception):
    pass


CUTS = collections.Counter()  # the lists not checked, by reason


def lists(ws, starts=None):
    """each list: (its script line, the words the game read); with a list STARTS, each list's
    first word's index goes into it (dsp4-01v)"""
    i, n = 0, len(ws)

    def wr():
        nonlocal i
        if i >= n or ws[i][1]:
            raise Cut('a read where the grammar wants a write')
        i += 1
        return ws[i - 1][0]

    def rd():
        nonlocal i
        if i >= n or not ws[i][1]:
            raise Cut('a write where the grammar wants a read')
        i += 1
        return ws[i - 1][0]
    while i < n:
        if not (ws[i] in ((3, 0), (0x0E, 0)) and i and ws[i - 1][1]):  # (dsp4-0e-1: VS's $000E too)
            i += 1
            continue
        start = i
        i += 1
        calls, reads = ['O' if ws[start][0] == 3 else 'V'], []
        try:
            while True:
                cmd = wr()
                if cmd == 0x0E:  # dsp4-0e-1: the VS race's next view
                    calls.append('V')
                elif cmd == 5:
                    calls.append('Z')
                elif cmd == 0x0B:
                    x, y, a = wr(), wr(), wr()
                    calls.append('B %X %X %X' % (x, y, a))
                    f = rd()
                    reads.append(f)
                    if f:
                        reads += [rd(), rd()]
                elif cmd == 9:
                    pre = [wr() for _ in range(7)]
                    calls.append('S ' + ' '.join('%X' % v for v in pre))
                    while True:
                        w0, w1 = wr(), wr()
                        if w0 == 0 and w1 == 0x8000:
                            calls.append('X')
                            break
                        if w1 == 0:
                            calls.append('L %X' % w0)
                            continue
                        if w1 & 0x8000:
                            rest = [wr() for _ in range(7)]
                            calls.append('C ' + ' '.join('%X' % v for v in [w0, w1] + rest))
                            reads.append(rd())
                            calls.append('T %X %X' % (wr(), wr()))
                        else:
                            rest = [wr() for _ in range(5)]
                            calls.append('N ' + ' '.join('%X' % v for v in [w0, w1] + rest))
                        for _ in range(2):
                            while True:
                                w = wr()
                                if w == 0:
                                    calls.append('E')
                                    break
                                a, b = wr(), wr()
                                calls.append('P %X %X %X' % (w, a, b))
                                got, f = [], rd()
                                while f:
                                    got += [rd(), rd()]
                                    f = rd()
                                reads += [len(got) // 2] + got  # (the driver prints the count)
                elif cmd == 0:  # a multiply between them (two words, two answers): not the list's.
                    wr(), wr()  # (a write where an answer is due skips it, as on the chip)
                    for _ in range(2):
                        if i >= n:
                            raise Cut('the tape ends')
                        i += 1
                elif cmd == 0x0A:  # the edges (three words, four answers): not the list's
                    wr(), wr(), wr()
                    rd(), rd(), rd(), rd()
                elif cmd == 6:
                    calls.append('H')
                    reads += [rd() for _ in range(16)]
                    break
                else:
                    raise Cut('command $%04X inside the list' % cmd)  # not a race list
        except Cut as why:
            CUTS[str(why)] += 1
            i = start + 1
            continue
        if starts is not None:
            starts.append(start)
        yield ' '.join(calls), reads


def main():
    ws = words(sys.argv[1])
    items = list(lists(ws))
    out = subprocess.run([NATIVE], input='\n'.join(c for c, r in items) + '\n', capture_output=True, text=True,
                         check=True).stdout.splitlines()
    ok, kinds, vs, vsok = 0, collections.Counter(), 0, 0
    for (calls, reads), line in zip(items, out):
        got = [int(v, 16) for v in line.split()]
        if calls.startswith('V'):  # (dsp4-0e-1: a VS view pair, counted apart)
            vs += 1
            vsok += got == reads
            continue
        if got == reads:
            ok += 1
            continue
        k = next((j for j in range(min(len(got), len(reads))) if got[j] != reads[j]), min(len(got), len(reads)))
        kinds['differs' if k < min(len(got), len(reads)) else 'length'] += 1
    print('sprite lists: %d, the native agrees on %d; %s' % (len(items) - vs, ok, dict(kinds)))
    if vs:
        print('VS view pairs ($000E): %d, the native agrees on %d' % (vs, vsok))
    for why, k in CUTS.most_common():
        print('  not checked: %d, %s' % (k, why))
    return 0 if items and ok == len(items) - vs and vsok == vs else 1


if __name__ == '__main__':
    sys.exit(main())
