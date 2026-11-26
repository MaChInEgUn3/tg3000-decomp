#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# dsp4-01p (port request A): src/dsp4.c's road ($01) against the firmware DSP-4, outputs only.
#   make -C tools/bench probe && python3 -I tools/dsp4_road_check.py [N] [TAPE]
# 1. N random runs (default 6,000) as the game builds them (header word 2, the line limit,
#    15-80; word 1, the camera's line, equal to it or up to 60 above, as road_rows_project
#    sends for $0242 >= 0; eight rows of falling heights; curves, hills, crests; the $0A the
#    game sends before, with its two rows equal, and 0-3 multiplies in between; one run in ten
#    without a $0A, counted apart: its first rounding is what the run before left), on the firmware (build/bench/dsp4_road_probe) and on src/dsp4.c
#    (build/bench/dsp4_road_native): every answer and line must agree. Runs with a segment of
#    88 lines or more (the chip's other regime, dsp4-01m; the game's are under 52) are counted
#    apart. The first run of the probe's process is sent twice and its first copy dropped
#    (the fresh chip's first run).
# 2. TAPE (default build/cache/race1.tape: bench --dsp4-firmware --dsp4-tape-out over
#    race1.txt): every $01 run after a $0A must agree (the runs before the race's first $0A
#    take their first rounding from the sprite streams, not modelled yet: counted apart, with
#    the largest difference in a line word). A $0F run, an edge ($8001) or a header whose
#    words 1 and 2 are not $33 (a nonzero $0242) fails: never seen, not checked.
# 3. dsp4-07a: the second road, $07: N random runs as road_rows_project2 builds them (rows it
#    places itself: lines falling 0-12 a row, offsets and a $0798 addend), on both; and every
#    $07 run of TAPE, which follows a $01 that drew (the rounding then $8000, as fresh).
# Exit status 0 when all agree.
import os
import random
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PROBE = os.path.join(ROOT, 'build', 'bench', 'dsp4_road_probe')
NATIVE = os.path.join(ROOT, 'build', 'bench', 'dsp4_road_native')
FIRMWARE = os.environ.get('TG3000_DSP4_FIRMWARE', os.path.join(ROOT, 'ext', 'firmware', 'dsp4.rom'))
HEAD = [0, 0x33, 0x33, 0xFF5B, 0xBF, 0xDC, 0, 0x3F, 0x180, 0x6BC, 0xC0, 0, 0, 0, 0]


def spec(words):
    return ' '.join('%X' % (w & 0xFFFF) for w in words)


def random_cases(n, rnd):
    cases = []
    for _ in range(n):
        hd = list(HEAD)
        hd[2] = rnd.randint(15, 80)
        hd[1] = hd[2] + rnd.choice([0, 0, rnd.randint(1, 60)])
        hd[4], hd[5] = rnd.randint(0, 400), rnd.randint(0, 400)
        hd[6] = rnd.randint(0, 0xFFFF)
        hd[7], hd[8] = rnd.randint(-200, 800), rnd.randint(-200, 800)
        hd[9] = rnd.choice([0x6BC, 0xD7C])
        hd[10] = rnd.randint(1, 1500)
        hd[11], hd[12] = rnd.randint(0, 0xFFFF), rnd.randint(-3, 3)
        hd[13], hd[14] = rnd.randint(0, 0xFFFF), rnd.randint(-3, 3)
        first = [rnd.randint(-3000, 3000), rnd.randint(-3000, 3000), rnd.randint(-5, 5)]
        rows = []
        for h in sorted(rnd.sample(range(1000, 32767), 8), reverse=True):
            rows += [h, rnd.randint(-300, 300), rnd.randint(-300, 300), rnd.randint(-200, 200)]
        pre = ''
        if rnd.random() >= 0.1:
            row = rnd.choice([0xC4C4, 0xE2E2, 0xE1E2, 0xC3C4, 0x4444, 0x0000])
            pre = 'E %X %X %X ' % (rnd.randint(0, 0x7F) << 8, row, row)
            pre += ''.join('M %X %X ' % (rnd.randint(0, 0xFFFF), rnd.randint(0, 0xFFFF)) for _ in range(rnd.randint(0, 3)))
        cases.append(pre + spec(hd + first + rows))
    return cases


def run(tool, cases):
    args = [tool] + ([FIRMWARE] if tool == PROBE else [])
    out = subprocess.run(args, input='\n'.join(cases) + '\n', capture_output=True, text=True, check=True).stdout
    return out.splitlines()


def long_segment(line, head1):
    """Whether the firmware's answer has a segment of 88 lines or more."""
    prev = head1
    for part in line.split('#')[-1].split(' | '):
        y = int(part.split(':')[0].split()[3], 16)
        y = y - 0x10000 if y & 0x8000 else y
        if prev - y >= 88:
            return True
        prev = y
    return False


def tape_runs(path):
    t = open(path, 'rb').read()
    n = len(t) // 5
    val = t[3::5]
    rd = [b & 1 for b in t[4::5]]

    def w(i):
        return val[i] | val[i + 1] << 8
    runs, edges, i = [], None, 0
    while i < n - 1:
        start = not rd[i] and val[i + 1] == 0 and not rd[i + 1] and (i == 0 or rd[i - 1])
        if start and val[i] == 0x0A:
            edges = (w(i + 2), w(i + 4), w(i + 6))
            i += 16
            continue
        if start and val[i] == 0x0F:
            raise SystemExit('%s: a $0F run at record %d: not checked' % (path, i))
        if start and val[i] == 0x01:
            j = i + 2
            words = [w(j + 2 * k) for k in range(15)]
            if words[1] != 0x33 or words[2] != 0x33:
                raise SystemExit('%s: a header with words 1, 2 = %X, %X at record %d: not checked' % (path, words[1], words[2], i))
            j += 30
            expect, first = [], True
            while True:
                h = w(j)
                j += 2
                if h == 0x8000:
                    break
                if h == 0x8001:
                    raise SystemExit('%s: an edge ($8001) at record %d: not checked' % (path, j))
                k = 6 if first else 3
                words += [h] + [w(j + 2 * q) for q in range(k)]
                j += 2 * k
                first = False
                ans = [w(j + 2 * q) for q in range(5)]
                j += 10
                lines = [w(j + 2 * q) for q in range(3 * ans[4])]
                j += 6 * ans[4]
                expect.append('%04X %04X %04X %04X %d:' % tuple(ans) + ''.join(
                    ' %04X,%04X,%04X' % tuple(lines[3 * q:3 * q + 3]) for q in range(ans[4])))
            # the probe's spec puts the first row's 3 words after the header
            head, row0 = words[:15], words[15:22]
            body = head + row0[1:4] + [row0[0]] + row0[4:7] + words[22:]
            pre = 'E %X %X %X ' % edges if edges else ''
            runs.append((edges is not None, pre + spec(body), ' | '.join(expect)))
            edges = None
            i = j
            continue
        i += 1
    return runs


def random_cases7(n, rnd):
    cases = []
    for _ in range(n):
        hd = [0, 0x33, 0x33, 0xFF5B, 0xBF, 0xDC, 0, 0x40, 0x180, 0x35C, 0xC0]
        hd[2] = rnd.randint(15, 80)
        hd[1] = hd[2] + rnd.choice([0, 0, rnd.randint(1, 40)])
        hd[4], hd[5] = rnd.randint(0, 400), rnd.randint(0, 400)
        hd[6], hd[7], hd[8] = rnd.randint(0, 0xFFFF), rnd.randint(-200, 800), rnd.randint(-200, 800)
        hd[9] = rnd.choice([0x35C, 0x50C])
        hd[10] = rnd.randint(1, 1500)
        rows, y = [], hd[1]
        for _ in range(8):
            y -= rnd.randint(0, 12)
            rows += [rnd.randint(1000, 32767), y, rnd.randint(-40, 40), rnd.randint(-200, 200), rnd.randint(-60, 60),
                     rnd.randint(-100, 100)]
        row = rnd.choice([0xC4C4, 0x4444, 0x0000])
        cases.append('E 0 %X %X R7 ' % (row, row) + spec(hd + rows))
    return cases


def long_segment7(line, head1):
    prev = head1
    for part in line.split('#')[-1].split(' | '):
        y = int(part.split(':')[0].split()[1], 16)
        y = y - 0x10000 if y & 0x8000 else y
        if prev - y >= 88:
            return True
        prev = y
    return False


def tape_runs7(path):
    t = open(path, 'rb').read()
    n = len(t) // 5
    val = t[3::5]
    rd = [b & 1 for b in t[4::5]]

    def w(i):
        return val[i] | val[i + 1] << 8
    runs, i = [], 0
    while i < n - 1:
        start = not rd[i] and val[i + 1] == 0 and not rd[i + 1] and (i == 0 or rd[i - 1])
        if start and val[i] == 0x10:
            raise SystemExit('%s: a $10 run at record %d: not checked' % (path, i))
        if start and val[i] == 0x07:
            j = i + 2
            words = [w(j + 2 * k) for k in range(11)]
            j += 22
            expect = []
            while True:
                h = w(j)
                if h == 0x8000:
                    j += 2
                    break
                words += [w(j + 2 * q) for q in range(6)]
                j += 12
                ans = [w(j + 2 * q) for q in range(3)]
                j += 6
                lines = [w(j + 2 * q) for q in range(3 * ans[2])]
                j += 6 * ans[2]
                expect.append('%04X %04X %d:' % tuple(ans) + ''.join(
                    ' %04X,%04X,%04X' % tuple(lines[3 * q:3 * q + 3]) for q in range(ans[2])))
            runs.append(('R7 ' + spec(words), ' | '.join(expect)))
            i = j
            continue
        i += 1
    return runs


def main():
    n = int(sys.argv[1]) if len(sys.argv) > 1 else 6000
    tape = sys.argv[2] if len(sys.argv) > 2 else os.path.join(ROOT, 'build', 'cache', 'race1.tape')
    cases = random_cases(n, random.Random(2026))
    fw = run(PROBE, cases[:1] + cases)[1:]
    nat = run(NATIVE, cases)
    bad = long = longbad = none = nonebad = 0
    for c, a, b in zip(cases, fw, nat):
        words = [x for x in c.split() if x not in ('E', 'M')]
        pre = 3 * c.split().count('E') + 2 * c.split().count('M')
        head1 = int(words[pre + 1], 16)
        if long_segment(a, head1):
            long += 1
            longbad += a.split() != b.split()
        elif 'E' not in c.split():  # the rounding the previous run left: not modelled
            none += 1
            nonebad += a.split() != b.split()
        else:
            bad += a.split() != b.split()
    print('random runs: %d after a $0A, differ %d; apart (not modelled): %d without a $0A, differ %d; %d with a '
          'segment of 88+ lines, differ %d' % (n - long - none, bad, none, nonebad, long, longbad))
    ok = bad == 0 and len(fw) == len(nat) == n
    cases7 = random_cases7(n, random.Random(2027))
    fw7 = run(PROBE, cases7[:1] + cases7)[1:]
    nat7 = run(NATIVE, cases7)
    bad7 = long7 = 0
    for c, a, b in zip(cases7, fw7, nat7):
        if long_segment7(a, int(c.split('R7')[1].split()[1], 16)):
            long7 += 1
        else:
            bad7 += a.split() != b.split()
    print('$07 random runs: %d, differ %d; %d with a segment of 88+ lines (not modelled)' % (n - long7, bad7, long7))
    ok = ok and bad7 == 0 and len(fw7) == len(nat7) == n
    if os.path.exists(tape):
        runs = tape_runs(tape)
        got = run(NATIVE, [r[1] for r in runs])
        after = [g.split('#')[-1].split() == r[2].split() for g, r in zip(got, runs) if r[0]]
        before = [g.split('#')[-1].split() == r[2].split() for g, r in zip(got, runs) if not r[0]]
        worst = 0
        for g, r in zip(got, runs):
            if r[0]:
                continue
            for x, y in zip(g.split('#')[-1].split(), r[2].split()):
                if ',' in x and ',' in y:
                    for u, v in zip(x.split(','), y.split(',')):
                        d = (int(u, 16) - int(v, 16)) & 0xFFFF
                        worst = max(worst, min(d, 0x10000 - d))
        print('%s: %d runs after a $0A, differ %d; %d before any (not modelled), differ %d, by at most %d' % (
            os.path.basename(tape), len(after), after.count(False), len(before), before.count(False), worst))
        ok = ok and after.count(False) == 0 and len(after) > 0
        runs7 = tape_runs7(tape)
        got7 = run(NATIVE, [r[0] for r in runs7])
        bad7 = sum(1 for g, r in zip(got7, runs7) if g.split() != r[1].split())
        print('%s: %d $07 runs, differ %d' % (os.path.basename(tape), len(runs7), bad7))
        ok = ok and bad7 == 0
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
