#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Split a refbench --dsp4-tape capture into the game's $09 sprite streams and measure how
far each record's answer is a function of what the game sent.

A record is the words the game writes after the chip's last answer, then the words it
reads (the answer). The keys tried, each wider than the last: the record's own words; with
the stream's preamble (the words before the first object head) and the object's head (the
words $82:D934-$82:D9DD and $82:D267-$82:D2CB write); with the frame's earlier DSP-4
traffic. Reports, for each, the keys that got more than one answer.

Outputs only: the capture holds what the game wrote and read, never the chip's memory.

    build/refbench/refbench TG3000.sfc "Input Log.txt" --firmware ext/firmware/dsp4.rom \\
        --bizhawk --frames 20000 --dsp4-tape tape.bin 0
    python3 -I tools/dsp4_tape.py tape.bin
"""
import collections
import hashlib
import sys

OPEN = 0x82CC8F    # the $0009 command word ($82:CBD4)
CLOSE = 0x82AE1C   # the next command word ($0006, oam_high_load)


def is_head(pc):
    return 0x82D934 <= pc < 0x82D9DD or 0x82D267 <= pc < 0x82D2CC


def words_of(data):
    """The tape's bytes paired into words (same PC, same direction, in a row)."""
    recs = [(data[i] | data[i + 1] << 8 | data[i + 2] << 16, data[i + 3], data[i + 4] & 1)
            for i in range(0, len(data) - 4, 5)]
    out, i = [], 0
    while i < len(recs):
        pc, v, rd = recs[i]
        if i + 1 < len(recs) and recs[i + 1][0] == pc and recs[i + 1][2] == rd:
            out.append((pc, v | recs[i + 1][1] << 8, rd))
            i += 2
        else:
            out.append((pc, v, rd))
            i += 1
    return out


def streams_of(words):
    """Each $09 stream, with a digest of the DSP-4 traffic since the previous one."""
    streams, before, cur = [], [], None
    for pc, val, rd in words:
        if not rd and pc == OPEN and val == 0x0009:
            cur = []
            streams.append((hashlib.md5(repr(before).encode()).hexdigest(), cur))
            before = []
            continue
        if cur is not None and not rd and pc == CLOSE:
            cur = None
        (cur if cur is not None else before).append((pc, val, rd))
    return streams


def records_of(stream):
    """(preamble, head, own words, answer) for each record of a stream."""
    pre = []
    for pc, val, rd in stream:
        if is_head(pc) or rd:
            break
        pre.append(val)
    pre = tuple(pre)
    head, writes, reads, out = (), [], [], []
    for pc, val, rd in stream + [(0, 0, 0)]:
        if rd:
            reads.append(val)
            continue
        if reads:
            out.append((pre, head, tuple(v for _, v in writes), tuple(reads)))
            writes, reads = [], []
        if is_head(pc):
            if not any(is_head(p) for p, _ in writes):
                head = ()
            head = head + (val,)
        writes.append((pc, val))
    return out


def main():
    streams = streams_of(words_of(open(sys.argv[1], 'rb').read()))
    keys = {name: collections.defaultdict(set) for name in ('own', 'head', 'frame')}
    n = 0
    for ctx, s in streams:
        for pre, head, own, answer in records_of(s):
            n += 1
            keys['own'][own].add(answer)
            keys['head'][(pre, head, own)].add(answer)
            keys['frame'][(ctx, pre, head, own)].add(answer)
    print(f"{len(streams)} streams, {n} records")
    for name, label in (('own', "own words"), ('head', "+ preamble and head"), ('frame', "+ the frame's earlier traffic")):
        d = keys[name]
        bad = sum(1 for v in d.values() if len(v) > 1)
        print(f"{label}: {bad} of {len(d)} keys ({100.0 * bad / max(1, len(d)):.2f}%) got more than one answer")


if __name__ == '__main__':
    main()
