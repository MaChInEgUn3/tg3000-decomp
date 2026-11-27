#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""dsp4-09c: what the sprite stream $09 needs from its frame, by replaying frames cut from a
tape recorded with the firmware chip, with commands left out or changed, into the firmware
(build/bench/dsp4_tape_run), comparing every byte the game read from the stream's $0009 on.

    build/bench/bench TG3000.sfc 9450 --movie tools/bench/movies/race1.txt \\
        --dsp4-firmware ext/firmware/dsp4.rom --dsp4-tape-out /tmp/r1.tape
    make -C tools/bench probe && python3 -I tools/dsp4_09_deps.py /tmp/r1.tape

A race frame sends: its cars' $0A, then $01, $07, $08, then $0003 (a sprite list: a word, its
first $000B entry) and a second $000B, then $0009, then $0006. A frame here runs from the last
$0A before its $01 to the $0006's answer. Outputs only. Exit status 0 when the measured facts
(docs/DECISIONS.md, dsp4-09c) still hold.
"""
import os
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RUN = os.path.join(ROOT, 'build', 'bench', 'dsp4_tape_run')
FW = os.path.join(ROOT, 'ext', 'firmware', 'dsp4.rom')
# the bench logs the instruction after the access (refbench the access's own)
CMD = {0x82AE1F: '06', 0x82CC92: '09', 0x81F912: '0A', 0x85B53E: '01', 0x85B962: '07', 0x81861D: '08'}


def load(p):
    t = open(p, 'rb').read()
    return [(t[i] | t[i + 1] << 8 | t[i + 2] << 16, t[i + 3], t[i + 4] & 1) for i in range(0, len(t) - 4, 5)]


def save(p, recs):
    with open(p, 'wb') as f:
        for pc, v, rd in recs:
            f.write(bytes([pc & 255, pc >> 8 & 255, pc >> 16, v, rd]))


def frames_of(r):
    """(the last $0A, $01, $07, $08, $09, the end) of each frame sent in that order"""
    seq = [(k, CMD[r[k][0]]) for k in range(len(r) - 1) if not r[k][2] and r[k][0] in CMD and r[k + 1][1] == 0
           and not r[k + 1][2] and (k == 0 or r[k - 1][2])]
    frames = []
    for n, (k, c) in enumerate(seq):
        if c != '09' or n < 4 or [x[1] for x in seq[n - 3:n]] != ['01', '07', '08']:
            continue
        e0 = [x for x in seq[n - 4::-1] if x[1] == '0A']
        if not e0:
            continue
        nxt = seq[n + 1] if n + 1 < len(seq) else None
        end = nxt[0] + 34 if nxt and nxt[1] == '06' else (nxt[0] if nxt else len(r))
        frames.append((e0[0][0], seq[n - 3][0], seq[n - 2][0], seq[n - 1][0], k, end))
    return frames


def end8(r, k8):
    """the record after the $08's last answer"""
    j = k8 + 2 + 90 + 2
    while (r[j][1] | r[j + 1][1] << 8) != 0x8000:
        j += 18
        for _ in range(2):
            j += 2 + 4 * (r[j][1] | r[j + 1][1] << 8)
    return j + 4


def W(v):
    return [(0, v & 255, 0), (0, v >> 8, 0)]


def main():
    tape = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, 'build', 'cache', 'race1.tape')
    r = load(tape)
    fr = frames_of(r)
    tmp = tempfile.mkdtemp()
    items = []
    for i, (a, k1, k7, k8, k9, e) in enumerate(fr):
        j = end8(r, k8)
        g = r[j:k9]  # $0003 and its word (4 records), two $000B sprites (14 records each)
        variants = {
            'whole frame': r[a:k9],
            'no $01': r[a:k1] + r[k7:k9],
            'no $01 $07 $08': r[a:k1] + g,
            'no $03 and $0B': r[a:j],
            'no $0B': r[a:k1] + g[0:4],
            "$03's word + 1": r[a:k1] + g[0:2] + W((g[2][1] | g[3][1] << 8) + 1) + g[4:],
        }
        for name, pre in variants.items():
            items.append((name, i, pre + r[k9:e], len(pre)))
    args = []
    for n, (name, i, recs, frm) in enumerate(items):
        p = os.path.join(tmp, '%d.tape' % n)
        save(p, recs)
        args.append('%s@%d' % (p, frm))
    out = []
    for i in range(0, len(args), 2000):
        out += subprocess.run([RUN, FW, '0'] + args[i:i + 2000], capture_output=True, text=True,
                              check=True).stdout.splitlines()
    stats = {}
    for (name, i, recs, frm), line in zip(items, out):
        bad = int(line.split(': ')[1].split(' of')[0])
        s = stats.setdefault(name, [0, 0, 0])
        s[0] += 1
        s[1] += bad > 0
        s[2] += bad
    for name, (n, frames, bad) in stats.items():
        print('%-16s %d frames, %d differ (%d bytes)' % (name, n, frames, bad))
    ok = (stats['whole frame'][1] == 0 and stats['no $01'][1] == 0 and stats['no $01 $07 $08'][1] == 0 and
          stats['no $03 and $0B'][1] == len(fr) and stats["$03's word + 1"][1] == len(fr) and len(fr) > 0)
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
