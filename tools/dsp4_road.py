#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# dsp4-01d (port request A): the road command's runs ($01, or $0F with D+$E4) read from a
# DSP-4 tape, in road_rows_project's grammar ($85:B3D7, src/misc.c), one JSON line a run:
#   cmd, e4 ($0F's word), head (the 15 words after the command word), rows: h (the row's ROM
#   height), first (the first row's 3 extra words), in (3 words: $3B6E/$3AEE/$3BEC + k), out
#   (the 4 answers stored at $34EC-$37EC), count (the answer after them), lines (count
#   answers of 3 words: the line's index, then the words for $0798 and $0796), obj (an
#   object's words sent and read, when not copied), edge (the 3 words after $8001).
#   python3 -I tools/dsp4_road.py TAPE OUT.jsonl [--ref]
# The tape: bench --dsp4-tape-out (each record's PC is the one after the accessing
# instruction) or, with --ref, refbench --dsp4-tape (the instruction's own PC). Outputs only:
# the tape holds what the game sent and got.
import json
import sys


def main():
    if len(sys.argv) < 3:
        sys.exit('usage: dsp4_road.py TAPE OUT.jsonl [--ref]')
    ref = '--ref' in sys.argv[3:]

    def at(pc):  # the bench's PC after a 3-byte store or load, as this tape records it
        return pc - 3 if ref else pc

    t = open(sys.argv[1], 'rb').read()
    n = len(t) // 5
    words = []  # (pc, value, read): two bytes at one PC in one direction make a word
    i = 0
    while i < n - 1:
        a, b = t[5 * i:5 * i + 5], t[5 * i + 5:5 * i + 10]
        pa, pb = a[0] | a[1] << 8 | a[2] << 16, b[0] | b[1] << 8 | b[2] << 16
        if pa == pb and (a[4] & 1) == (b[4] & 1):
            words.append((pa & 0xFFFF, a[3] | b[3] << 8, a[4] & 1))
            i += 2
        else:
            words.append((pa & 0xFFFF, a[3], (a[4] & 1) | 2))
            i += 1
    heights = (at(0xB604), at(0xB622))
    entries = (at(0xB735), at(0xB747))
    runs = []
    k, N = 0, len(words)

    def end_word(w):
        return w[2] == 0 and w[1] == 0x8000 and w[0] > 0xB700

    while k < N - 1:
        pc, v, rd = words[k]
        if not (rd == 0 and pc in (at(0xB53E), at(0xB539)) and v in (0x01, 0x0F)):
            k += 1
            continue
        run = {'cmd': v, 'head': [], 'rows': []}
        k += 1
        if v == 0x0F:
            run['e4'] = words[k][1]
            k += 1
        while k < N and words[k][0] != at(0xB604):
            run['head'].append(words[k][1])
            k += 1
        first, ok = True, True
        while k + 8 < N:
            pc, v, rd = words[k]
            if end_word(words[k]):
                k += 1
                break
            if pc not in heights:
                ok = False
                break
            row = {'h': v}
            k += 1
            if first:
                row['first'] = [w[1] for w in words[k:k + 3]]
                k += 3
                first = False
            row['in'] = [w[1] for w in words[k:k + 3]]
            k += 3
            row['out'] = [w[1] for w in words[k:k + 4]]
            k += 4
            row['count'] = words[k][1]
            k += 1
            obj = []
            while k < N and words[k][2] == 0 and words[k][0] not in heights and not end_word(words[k]) \
                    and words[k][1] != 0x8001:
                obj.append(words[k][1])
                k += 1
                if k < N and words[k][2] == 1 and words[k][0] not in entries:
                    obj.append(['read', words[k][1]])
                    k += 1
            if obj:
                row['obj'] = obj
            row['lines'] = [[w[1] for w in words[k + 3 * j:k + 3 * j + 3]] for j in range(row['count'])]
            k += 3 * row['count']
            if k < N and words[k][2] == 0 and words[k][1] == 0x8001:
                row['edge'] = [w[1] for w in words[k + 1:k + 4]]
                k += 4
            run['rows'].append(row)
        run['ok'] = ok
        runs.append(run)
    with open(sys.argv[2], 'w') as f:
        for r in runs:
            f.write(json.dumps(r) + '\n')
    good = sum(1 for r in runs if r['ok'])
    print('%d runs, %d read whole' % (len(runs), good))
    return 0 if runs and good == len(runs) else 1


if __name__ == '__main__':
    sys.exit(main())
