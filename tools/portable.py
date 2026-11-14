#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""List routines that are portable now: every call they make goes to a verified routine
(map/verified.txt), and the only hardware they touch is the CPU multiply/divide unit.

    python3 -I tools/portable.py [N]     # the N best candidates (default 25)

A routine's extent is found by following branches from its entry until RTS/RTL/JMP, so
the instruction counts are a guide, not a proof; the bench's differential test decides.
"""
import collections
import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MATH = {"4202", "4203", "4204", "4205", "4206", "4214", "4215", "4216", "4217"}


def load():
    ins, order = {}, []
    for f in sorted(glob.glob(os.path.join(ROOT, "asm", "bank_*.asm"))):
        for line in open(f):
            m = re.match(r"\s{4}(\S.*?)\s*; \$([0-9A-F]{6})$", line.rstrip())
            if m:
                a = int(m.group(2), 16)
                ins[a] = m.group(1).strip()
                order.append(a)
    nxt = {a: order[i + 1] for i, a in enumerate(order[:-1])}
    names = {}
    for line in open(os.path.join(ROOT, "map", "symbols.txt")):
        p = line.split(";")[0].split()
        if len(p) >= 3 and p[0] == "func":
            names[p[2]] = int(p[1], 16)
    verified = set()
    vpath = os.path.join(ROOT, "map", "verified.txt")
    if os.path.exists(vpath):
        for line in open(vpath):
            p = line.split()
            if not line.startswith("#") and len(p) > 7 and p[7] == "verified":
                verified.add(int(p[0], 16))
    return ins, nxt, names, verified


def call_target(a, t, names):
    m = re.match(r"(jsr|jsl)\.[wl] (\S+)$", t)
    if not m:
        return None
    op = m.group(2)
    if op in names:
        return names[op]
    if op.startswith("$"):
        v = int(op[1:], 16)
        return v | 0x800000 if m.group(1) == "jsl" else (a & 0xFF0000) | v
    return -1  # indirect


def extent(e, ins, nxt, names):
    seen, todo = set(), [e]
    while todo:
        a = todo.pop()
        while a in ins and a not in seen and len(seen) < 300:
            seen.add(a)
            t = ins[a]
            if re.match(r"(rts|rtl|rti|jmp|jml|brl)\b", t):
                break
            m = re.match(r"b(cc|cs|eq|ne|mi|pl|vc|vs|ra) (\S+)", t)
            if m:
                tg = m.group(2)
                ta = names.get(tg) or (int(tg[5:], 16) if tg.startswith("CODE_") else None)
                if ta:
                    todo.append(ta)
                if m.group(1) == "ra":
                    break
            a = nxt.get(a)
    return sorted(seen)


def main():
    limit = int(sys.argv[1]) if len(sys.argv) > 1 else 25
    ins, nxt, names, verified = load()
    calls = collections.Counter()
    for a, t in ins.items():
        tg = call_target(a, t, names)
        if tg and tg > 0:
            calls[tg] += 1
    rev = {v: k for k, v in names.items()}
    out = []
    for e, n in calls.items():
        if e not in ins or e in verified:
            continue
        ex = extent(e, ins, nxt, names)
        if len(ex) >= 300:
            continue
        text = " | ".join(ins[a] for a in ex)
        ok, used = True, 0
        for a in ex:
            t = ins[a]
            tg = call_target(a, t, names)
            if tg is not None:
                if tg in verified:
                    used += 1
                    continue
                ok = False
                break
            if re.match(r"(jmp|jml|rti|wai|stp|mv[np]|brk|cop)\b", t):
                ok = False
                break
        if not ok or re.search(r"\.l \$[0-9A-F]{6}", text):
            continue
        hw = set(re.findall(r"\$(2[01][0-9A-F]{2}|4[0-3][0-9A-F]{2})\b", text)) - MATH
        if hw:
            continue
        out.append((-used, -n, len(ex), e))
    out.sort()
    for used, n, size, e in out[:limit]:
        print(f"${e:06X} {rev.get(e, '')}\tcallers={-n} insns={size} calls_verified={-used}")


if __name__ == "__main__":
    main()
