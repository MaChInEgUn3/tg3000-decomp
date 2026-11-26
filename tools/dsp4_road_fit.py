#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# dsp4-01e (port request A): the road command's per-row answers, as measured on the firmware
# (outputs only), checked against tools/dsp4_road.py's runs. Exit status 0 when every row is
# exact.
#   python3 -I tools/dsp4_road_fit.py RUNS.jsonl...
# The model (16.16 fixed point; s() is a signed 16-bit word; >> floors):
#   X = s(head7) << 16 | head6, plus the first row's s(first2) << 16 | (first1 & $FF00)
#   G = s(head14) << 16 | (head13 & $FF00)          (the sideways turn rate)
#   E = s(head1) << 16                              (the height above the road)
#   S = s(head12) << 16 | (head11 & $FF00)          (the slope)
#   each row, with h its ROM height:
#     answers  X >> 16, (X >> 16) * h >> 15, E >> 16, (E >> 16) * h >> 15 (its screen line y)
#     count    max(0, m - y), m the lowest y so far (from s(head1)); then m = min(m, y)
#     then     G += s(in1) << 8, S += s(in0) << 8, X += G, E += S
import json
import sys


def s16(x):
    return x - 0x10000 if x & 0x8000 else x


def rows_of(run):
    hd, f = run['head'], run['rows'][0]['first']
    X = (s16(hd[7]) << 16) + hd[6] + (s16(f[2]) << 16) + (f[1] & 0xFF00)
    G = (s16(hd[14]) << 16) | (hd[13] & 0xFF00)
    E = s16(hd[1]) << 16
    S = (s16(hd[12]) << 16) | (hd[11] & 0xFF00)
    m = s16(hd[1])
    for row in run['rows']:
        x, e = X >> 16, E >> 16
        y = (e * row['h']) >> 15
        yield [x, (x * row['h']) >> 15, e, y], max(0, m - y)
        m = min(m, y)
        G += s16(row['in'][1]) << 8
        S += s16(row['in'][0]) << 8
        X += G
        E += S


def main():
    total = bad = 0
    for path in sys.argv[1:]:
        n = wrong = 0
        for line in open(path):
            run = json.loads(line)
            for row, (out, count) in zip(run['rows'], rows_of(run)):
                n += 1
                if [s16(v) for v in row['out']] != out or row['count'] != count:
                    wrong += 1
        print('%s: %d rows, %d differ' % (path, n, wrong))
        total += n
        bad += wrong
    return 0 if total and not bad else 1


if __name__ == '__main__':
    sys.exit(main())
