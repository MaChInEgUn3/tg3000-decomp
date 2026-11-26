#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# dsp4-01p (port request A): src/dsp4.c's road ($01) against the firmware DSP-4, outputs only.
#   make -C tools/bench probe && python3 -I tools/dsp4_road_check.py [N] [TAPE]
# 1. N random runs (default 6,000) as the game builds them (header words 1 = 2, 15-80 lines of
#    camera; eight rows of falling heights; curves, hills, crests; the $0A the game sends
#    before, with its two rows equal, and 0-3 multiplies in between; one run in ten without
#    a $0A), on the firmware (build/bench/dsp4_road_probe) and on src/dsp4.c
#    (build/bench/dsp4_road_native): every answer and line must agree. The first run of the
#    probe's process is sent twice and its first copy dropped (the fresh chip's first run).
# 2. TAPE (default build/cache/race1.tape: bench --dsp4-firmware --dsp4-tape-out over
#    race1.txt): every $01 run after a $0A must agree (the runs before the race's first $0A
#    take their first rounding from the sprite streams, not modelled yet: counted apart).
# Exit status 0 when both agree.
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
        hd[1] = hd[2] = rnd.randint(15, 80)
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
        if start and val[i] == 0x01:
            j = i + 2
            words = [w(j + 2 * k) for k in range(15)]
            j += 30
            expect, first = [], True
            while True:
                h = w(j)
                j += 2
                if h == 0x8000:
                    break
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


def main():
    n = int(sys.argv[1]) if len(sys.argv) > 1 else 6000
    tape = sys.argv[2] if len(sys.argv) > 2 else os.path.join(ROOT, 'build', 'cache', 'race1.tape')
    cases = random_cases(n, random.Random(2026))
    fw = run(PROBE, cases[:1] + cases)[1:]
    nat = run(NATIVE, cases)
    bad = sum(1 for a, b in zip(fw, nat) if a.split() != b.split())
    print('random runs: %d, differ %d' % (n, bad))
    ok = bad == 0 and len(fw) == len(nat) == n
    if os.path.exists(tape):
        runs = tape_runs(tape)
        got = run(NATIVE, [r[1] for r in runs])
        after = [g.split('#')[-1].split() == r[2].split() for g, r in zip(got, runs) if r[0]]
        before = [g.split('#')[-1].split() == r[2].split() for g, r in zip(got, runs) if not r[0]]
        print('%s: %d runs after a $0A, differ %d; %d before any (not modelled), differ %d' % (
            os.path.basename(tape), len(after), after.count(False), len(before), before.count(False)))
        ok = ok and after.count(False) == 0 and len(after) > 0
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
