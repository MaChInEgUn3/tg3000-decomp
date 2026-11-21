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
    """Each $09 stream: from its $0009 word up to the next command word $0006."""
    streams, cur = [], None
    for pc, val, rd in words:
        if not rd and pc == OPEN and val == 0x0009:
            cur = []
            streams.append(cur)
            continue
        if cur is not None and not rd and pc == CLOSE:
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


def main():
    data = open(sys.argv[1], 'rb').read()
    words = words_of(data)
    nbytes = len(data) // 5
    print(f"{nbytes} bytes, {2 * len(words) - nbytes} left unpaired")
    streams = streams_of(words)
    keys = collections.defaultdict(list)
    for si, s in enumerate(streams):
        for key, answer, sprites in records_of(s):
            keys[key].append((answer, si, sprites))
    n = sum(len(v) for v in keys.values())
    repeated = [v for v in keys.values() if len(v) > 1]
    bad = [v for v in repeated if len({a for a, _, _ in v}) > 1]
    one_stream = sum(1 for v in bad if len({si for _, si, _ in v}) == 1)
    later = sum(1 for v in bad
                if [p for a, _, p in v if a == (0,)] and [p for a, _, p in v if a != (0,)]
                and min(p for a, _, p in v if a == (0,)) > max(p for a, _, p in v if a != (0,)))
    print(f"{len(streams)} streams, {n} records, {len(keys)} keys, {len(repeated)} seen more than once")
    print(f"{len(bad)} of those ({100.0 * len(bad) / max(1, len(repeated)):.2f}%) got more than one answer; "
          f"{one_stream} within one stream; in {later} every hidden answer came after more of the stream's "
          "sprites than every shown one")


if __name__ == '__main__':
    main()
