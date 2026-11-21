#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Split a refbench --dsp4-tape capture into the game's $09 sprite streams and measure how
far each record's answer is a function of what the game sent.

A record is the words the game writes after the chip's last answer, then the words it
reads (the answer). An object starts at the first word of a head ($82:D93A, $82:CEF4,
$82:D26B, or a bar at $81:84B3); a head can be split by a read ($82:D14E). The key is the
stream's preamble (its words before the first object), every word the object wrote before
this record, and the record's own words. Reports the keys seen more than once that got
more than one answer, in one stream or across streams, and for those whether the hidden
answers ($0000 alone) came after more of the stream's sprites than the shown ones.

A key counts as repeated only when it occurs after different stream contents (an identical
stream cannot give another answer). Then a ceiling test over every shown sprite: its size
from the high OAM table the chip returns for `$0006` (16 words after `$82:AE1C`, two bits a
sprite in the stream's order; 8 lines small, 16 large), and the most sprites already on
one of its lines when it was accepted.

Outputs only: the capture holds what the game wrote and read, never the chip's memory.

    build/refbench/refbench TG3000.sfc "Input Log.txt" --firmware ext/firmware/dsp4.rom \\
        --bizhawk --frames 20000 --dsp4-tape tape.bin 0
    python3 -I tools/dsp4_tape.py tape.bin
"""
import collections
import sys

OPEN = 0x82CC8F    # the $0009 command word ($82:CBD4)
CLOSE = 0x82AE1C   # the next command word ($0006, oam_high_load)


STARTS = {0x82D93A, 0x82CEF4, 0x82D26B, 0x8184B3}   # an object's first word


def words_of(data):
    """The tape's bytes paired into words (same PC, same direction, in a row); a byte left
    unpaired stays a word of its own (main() counts them: 0 on the captures so far)."""
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
    """Each $09 stream (from its $0009 word up to the next command word $0006) with the 16
    words the chip answers that $0006 with (the high OAM table)."""
    streams, cur = [], None
    for i, (pc, val, rd) in enumerate(words):
        if not rd and pc == OPEN and val == 0x0009:
            cur = []
            continue
        if cur is not None and not rd and pc == CLOSE:
            streams.append((cur, [w[1] for w in words[i + 1:i + 17]]))
            cur = None
        if cur is not None:
            cur.append((pc, val, rd))
    return streams


def records_of(stream):
    """(key, answer, sprites before) for each record of a stream: the key is (preamble, the
    object's words before this record, this record's words); the sprites before count the
    stream's earlier answers' OAM pairs."""
    pre = []
    for pc, val, rd in stream:
        if pc in STARTS or rd:
            break
        pre.append(val)
    pre = tuple(pre)
    obj, writes, reads, out, sprites = [], [], [], [], 0
    for pc, val, rd in stream + [(0, 0, 0)]:
        if rd:
            reads.append(val)
            continue
        if reads:
            out.append(((pre, tuple(obj), tuple(writes)), tuple(reads), sprites))
            sprites += max(0, (len(reads) - 1) // 2)
            obj.extend(writes)
            writes, reads = [], []
        if pc in STARTS:
            obj = []
        writes.append(val)
    return out


def sprites_of(stream, high):
    """Each shown sprite of a stream: (y, large), large from the high table's size bit."""
    bits = [(w >> (2 * k)) & 3 for w in high for k in range(8)]
    out, n = [], 0
    for _, answer, _ in records_of(stream):
        i = 0
        while i < len(answer) and answer[i] != 0:
            if i + 2 < len(answer):
                out.append((answer[i + 1] >> 8, n < len(bits) and bits[n] >> 1))
            n += 1
            i += 3
    return out, max([i for i, b in enumerate(bits) if b] or [-1]) < n


def main():
    data = open(sys.argv[1], 'rb').read()
    words = words_of(data)
    nbytes = len(data) // 5
    print(f"{nbytes} bytes, {2 * len(words) - nbytes} left unpaired")
    streams = streams_of(words)
    keys = collections.defaultdict(list)
    for si, (s, _) in enumerate(streams):
        prefix = hash(())
        for key, answer, sprites in records_of(s):
            keys[key].append((answer, si, prefix))
            prefix = hash((prefix, key, answer))
    n = sum(len(v) for v in keys.values())
    repeated = [v for v in keys.values() if len({p for _, _, p in v}) > 1]
    bad = [v for v in repeated if len({a for a, _, _ in v}) > 1]
    one_stream = sum(1 for v in bad if len({si for _, si, _ in v}) == 1)
    shown_hidden = sum(1 for v in bad if (0,) in {a for a, _, _ in v})
    print(f"{len(streams)} streams, {n} records ({sum(1 for v in keys.values() for a, _, _ in v if a == (0,))} hidden), "
          f"{len(keys)} keys, {len(repeated)} after different stream contents")
    print(f"{len(bad)} of those ({100.0 * len(bad) / max(1, len(repeated)):.2f}%) got more than one answer, "
          f"{one_stream} within one stream, {shown_hidden} shown in one and hidden in another")
    hist, aligned, over, over_streams = collections.Counter(), 0, 0, 0
    for s, high in streams:
        sprites, lined_up = sprites_of(s, high)
        if not lined_up:
            continue
        aligned += 1
        load, o = [0] * 256, 0
        for y, large in sprites:
            lines = [(y + k) & 0xFF for k in range(16 if large else 8)]
            m = max(load[line] for line in lines)
            hist[m] += 1
            o += m >= 33
            for line in lines:
                load[line] += 1
        over += o
        over_streams += o > 0
    print(f"ceiling: {aligned} of {len(streams)} streams line up with their high table; sprites accepted with k "
          f"already on a line, k = 30..36: {[hist[k] for k in range(30, 37)]}; with 33 or more: {over}, in {over_streams} streams")


if __name__ == '__main__':
    main()
