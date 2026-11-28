#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""dsp4-0f-1: the two-view race's road commands $0F and $10 (the race wrapper's mode 1), read off a
tape recorded with the firmware chip (bench --dsp4-firmware --dsp4-tape-out over a movie that
reaches mode 1, such as autopilot_60k). Outputs only.

    make -C tools/bench probe && python3 -I tools/dsp4_0f.py TAPE

The grammar, as the game drives it:
- $0F: one word (always $737B so far), then $01's whole command: 15 header words, rows (the
  first 7 words, then 4: a height, 3 track words) until $8000, a row's answer 4 words and a count,
  then that many lines of 3 words. $8001 and 3 words is an edge, with no answer.
- $10: one word ($737B), then $07's: 11 header words, rows of 6 words, an answer of 2 words and a
  count, then the lines.
- In both, a row whose count is not 0 adds, between its answer and its lines, 4 exchanges: the
  game writes two words and reads one.
Checks: every run parses; $10's runs are src/dsp4.c's $07 (dsp4_road_native's R7, its rounding as a
road leaves it), answers and lines (a run that differs only in lines' third words that the chip gives
as -128, $FF80, is counted apart: not modelled); $0F's first row answers are src/dsp4.c's $01 (its lines need
the rounding the run starts on, not taken here); each exchange's answer is a function of its two
words, the same for $0F and $10; the extra word is constant. Exit status 0 when all hold.
"""
import collections
import mmap
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import dsp4_road_check as road  # noqa: E402


class Broken(Exception):
    pass


def runs(path):
    """every $0F/$10 run: (command, record, header words, rows); a row is (words, answer,
    exchanges, lines) or ('edge', words)"""
    f = open(path, 'rb')
    t = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)
    n = len(t) // 5
    out, stats = [], collections.Counter()

    def rd(k):
        return t[5 * k + 4] & 1

    k = 0
    while k < n - 1:
        if not (not rd(k) and (k == 0 or rd(k - 1)) and not rd(k + 1) and t[5 * k + 8] == 0 and t[5 * k + 3] in (0x0F, 0x10)):
            k += 1
            continue
        cmd, j = t[5 * k + 3], k + 2

        def take(d, count):
            nonlocal j
            words = []
            for _ in range(count):
                if j + 1 >= n or rd(j) != d or rd(j + 1) != d:
                    raise Broken('record %d' % j)
                words.append(t[5 * j + 3] | t[5 * j + 8] << 8)
                j += 2
            return words
        try:
            head = take(0, 15 if cmd == 0x0F else 12)
            rows, first = [], True
            while True:
                h = take(0, 1)[0]
                if h == 0x8000:
                    break
                if h == 0x8001:
                    rows.append(('edge', take(0, 3)))
                    continue
                words = [h] + take(0, (7 if first else 3) if cmd == 0x0F else 5)
                first = False
                ans = take(1, 5 if cmd == 0x0F else 3)
                ex = []
                for _ in range(4 if ans[-1] else 0):
                    a, b = take(0, 2)
                    ex.append((a, b, take(1, 1)[0]))
                rows.append((words, ans, ex, take(1, 3 * ans[-1])))
            out.append((cmd, k, head, rows))
            stats['%02X' % cmd] += 1
        except Broken as why:
            stats['broken %02X' % cmd] += 1
            print('%s: a $%02X run at record %d does not parse (%s)' % (path, cmd, k, why))
        k = max(j, k + 1)
    return out, stats


def answers(rows, n):
    return ' | '.join(('%04X ' * (n - 1)) % tuple(ans[:-1]) + '%d:' % ans[-1] +
                      ''.join(' %04X,%04X,%04X' % tuple(lines[3 * q:3 * q + 3]) for q in range(ans[-1]))
                      for words, ans, ex, lines in rows)


def main():
    tape = sys.argv[1]
    all_runs, stats = runs(tape)
    ok = not any(k.startswith('broken') for k in stats) and bool(all_runs)
    edges = sum(1 for r in all_runs for row in r[3] if row[0] == 'edge')
    print('%s: $0F runs %d, $10 runs %d, edges ($8001) %d, unparsed %d' % (
        os.path.basename(tape), stats['0F'], stats['10'], edges, sum(v for k, v in stats.items() if k.startswith('broken'))))
    clean = [r for r in all_runs if not any(row[0] == 'edge' for row in r[3])]
    extra = collections.Counter(r[2][0] for r in all_runs)
    print('  the extra word: %s' % ', '.join('$%04X x%d' % kv for kv in extra.most_common(4)))
    ok = ok and len(extra) == 1
    # $10 as $07
    t10 = [r for r in clean if r[0] == 0x10]
    got = road.run(road.NATIVE, ['Q 8000 8000 R7 ' + road.spec(r[2][1:] + [w for row in r[3] for w in row[0]]) for r in t10])
    eq10 = clamp10 = 0
    for g, r in zip(got, t10):
        mine, theirs = g.split('#')[-1].split(), answers(r[3], 3).split()
        if mine == theirs:
            eq10 += 1
        elif len(mine) == len(theirs) and all(a == b or b.endswith(',FF80') and a[:10] == b[:10]
                                              for a, b in zip(mine, theirs)):
            clamp10 += 1  # (only lines' third word, -128 on the chip: not modelled, dsp4-0f-1)
    print('  $10 runs answered as src/dsp4.c\'s $07 (answers and lines): %d of %d; %d more differ only in a line\'s '
          'third word, $FF80 (-128) on the chip: not modelled' % (eq10, len(t10), clamp10))
    ok = ok and eq10 + clamp10 == len(t10)
    # $0F's first rows as $01
    t0f = [r for r in clean if r[0] == 0x0F]
    cases = []
    for r in t0f:
        words = r[2][1:] + [w for row in r[3] for w in row[0]]
        h15, r0 = words[:15], words[15:22]
        cases.append('Q 8000 8000 ' + road.spec(h15 + r0[1:4] + [r0[0]] + r0[4:7]))
    got = road.run(road.NATIVE, cases)
    eq0f = sum(g.split('#')[-1].split()[:5] == answers(r[3][:1], 5).split()[:5] for g, r in zip(got, t0f))
    print('  $0F first rows\' answers as src/dsp4.c\'s $01: %d of %d' % (eq0f, len(t0f)))
    ok = ok and eq0f == len(t0f)
    # the exchanges
    seen, conflicts, total = {}, 0, collections.Counter()
    for r in all_runs:
        for row in r[3]:
            if row[0] == 'edge':
                continue
            for a, b, v in row[2]:
                total[r[0]] += 1
                if seen.setdefault((a, b), v) != v:
                    conflicts += 1
    print('  exchanges: $0F %d, $10 %d; distinct inputs %d; an input with two answers: %d' % (
        total[0x0F], total[0x10], len(seen), conflicts))
    ok = ok and conflicts == 0
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
