#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Measure whether the DSP-4's answers are a function of what the game sent (dsp4-01a).

    build/bench/bench TG3000.sfc 9450 --movie tools/bench/movies/race1.txt \\
        --dsp4-firmware ext/firmware/dsp4.rom --dsp4-log /tmp/log.txt
    python3 -I tools/dsp4_runs.py /tmp/log.txt

Splits the logged data bytes (bench --dsp4-log: frame, PC, access, address, value) into runs
at the road command's word: $000F written at $85:B536, or $0001 at $85:B53B when D+$E4 is
negative (road_rows_project). A run is everything the game wrote and read from there to the
next run: in a race, one frame's DSP-4 traffic, every command. Reports:
- whole runs: how many inputs repeat and whether a repeated input ever got another output;
- each answer byte keyed by its run's input so far (and its place among the answers since
  the last input byte): how many keys got more than one answer.
Outputs only: what the game wrote and read, never the chip's memory.
"""
import collections
import hashlib
import sys

ROAD = 0x85


def site(pc_bank, pc, lo, hi):
    # the bench logs the PC after the operand: a site's instruction ends within 4 bytes
    return pc_bank == ROAD and lo < pc <= hi


def runs_of(path):
    runs, cur, prev = [], None, None
    for line in open(path):
        p = line.split()
        if len(p) < 5 or p[2] not in ("RD", "WD"):
            continue
        bank, adr = p[1].split(":")
        bank, adr, v = int(bank, 16) | 0x80, int(adr, 16), int(p[4], 16)
        w = p[2] == "WD"
        at36 = site(bank, adr, 0xB536, 0xB53A)
        at3b = site(bank, adr, 0xB53B, 0xB53F)
        if w and (at36 or at3b):
            after36 = prev is not None and site(prev[0], prev[1], 0xB536, 0xB53A)
            if (at36 or not after36) and (cur is None or cur["started"]):
                cur = {"frame": int(p[0]), "ev": [], "started": False}
                runs.append(cur)
        prev = (bank, adr)
        if cur is None:
            continue
        cur["ev"].append((w, v))
        if w and not (at36 or at3b):
            cur["started"] = True
    return runs


def main():
    runs = runs_of(sys.argv[1])
    whole = collections.defaultdict(set)
    for r in runs:
        ins = bytes(v for w, v in r["ev"] if w)
        outs = bytes(v for w, v in r["ev"] if not w)
        whole[ins].add(outs)
    repeated = sum(1 for k, v in whole.items() if sum(1 for r in runs if bytes(x for w, x in r["ev"] if w) == k) > 1)
    print(f"runs {len(runs)}, distinct inputs {len(whole)}, repeated inputs {repeated}, "
          f"of them with another output {sum(1 for v in whole.values() if len(v) > 1)}")
    pref = collections.defaultdict(set)
    total = 0
    for r in runs:
        h, since = hashlib.sha1(), 0
        for w, v in r["ev"]:
            if w:
                h.update(bytes([1, v]))
                since = 0
            else:
                pref[(h.digest(), since)].add(v)
                since += 1
                total += 1
    conf = sum(1 for s in pref.values() if len(s) > 1)
    print(f"answer bytes {total}, keys (input so far, place) {len(pref)}, keys with more than one answer {conf}")


if __name__ == "__main__":
    main()
