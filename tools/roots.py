#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Which unported routines block the most others? From map/census.txt (measured calls,
including routines reached through indirect jumps) and map/verified.txt.

    python3 -I tools/roots.py [N]

A *root* is an unported routine none of whose callees is unported: porting it is
possible without porting anything else first (if its own hardware use allows). Its
weight is how many unported routines call it, directly or not.
"""
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def load():
    routines = {}
    for line in open(os.path.join(ROOT, "map", "census.txt")):
        if line.startswith("#"):
            continue
        p = line.split()
        f = dict(x.split("=", 1) for x in p[4:])
        calls = [] if f["calls"] == "-" else [int(c, 16) for c in f["calls"].split(",")]
        indirect = [] if f["indirect"] == "-" else [int(c, 16) for c in f["indirect"].split(",")]
        routines[int(p[0], 16)] = {"kind": p[1], "entries": int(p[2]), "interrupted": int(p[3]),
                                   "hw": f["hw"], "states": f["states"], "calls": calls, "indirect": indirect}
    verified = set()
    vpath = os.path.join(ROOT, "map", "verified.txt")
    if os.path.exists(vpath):
        for line in open(vpath):
            p = line.split()
            if not line.startswith("#") and len(p) > 7 and p[7] == "verified":
                verified.add(int(p[0], 16))
    return routines, verified


def main():
    limit = int(sys.argv[1]) if len(sys.argv) > 1 else 15
    routines, verified = load()
    names = {}
    for line in open(os.path.join(ROOT, "map", "symbols.txt")):
        p = line.split(";")[0].split()
        if len(p) >= 3 and p[0] == "func":
            names[int(p[1], 16)] = p[2]
    unported = {e for e, r in routines.items() if e not in verified and r["kind"] == "sub"}
    callers = {e: set() for e in routines}
    for e, r in routines.items():
        for c in r["calls"]:
            callers.setdefault(c, set()).add(e)
    roots = [e for e in unported if not any(c in unported for c in routines[e]["calls"])]
    weight = {}
    for root in roots:
        seen, todo = set(), [root]
        while todo:
            for c in callers.get(todo.pop(), ()):
                if c in unported and c not in seen:
                    seen.add(c)
                    todo.append(c)
        weight[root] = len(seen)
    print(f"{len(unported)} unported routines seen, {len(roots)} roots")
    for e in sorted(roots, key=lambda e: -weight[e])[:limit]:
        r = routines[e]
        print(f"${e:06X} {names.get(e, '')}\tblocks {weight[e]:3d}  entries={r['entries']} "
              f"interrupted={r['interrupted']} hw={r['hw']} indirect={len(r['indirect'])}")


if __name__ == "__main__":
    main()
