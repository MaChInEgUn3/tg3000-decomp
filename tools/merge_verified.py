#!/usr/bin/env python3
"""Merge the differential test's results over several movies into map/verified.txt.

    python3 -I tools/merge_verified.py OUT IN1 IN2 ...

A routine is verified when some movie verified it and none failed it; failed when any
movie failed it; unreached when no movie reached it. The first verifying row is kept (its
live/fuzz/paths counts), with the live flags of every verifying row joined.
"""
import sys


def read(path):
    header, rows, order = None, {}, []
    with open(path) as f:
        for line in f:
            if line.startswith("#"):
                header = header or line
                continue
            parts = line.split()
            if len(parts) < 8:
                continue
            rows[parts[0]] = parts
            order.append(parts[0])
    return header, rows, order


def main():
    out, inputs = sys.argv[1], sys.argv[2:]
    tables = [read(p) for p in inputs]
    header = tables[0][0]
    order = list(tables[0][2])
    for _, _, o in tables[1:]:
        order += [e for e in o if e not in order]
    lines = [header.rstrip("\n")]
    for entry in order:
        rows = [t[1][entry] for t in tables if entry in t[1]]
        failed = [r for r in rows if r[7] == "failed"]
        verified = [r for r in rows if r[7] == "verified"]
        if failed:
            row = failed[0]
        elif verified:
            row = list(verified[0])
            flags = set()
            for r in verified:
                f = r[8].split(":", 1)[1] if len(r) > 8 and ":" in r[8] else "-"
                flags |= set(f.replace("-", ""))
            row[8] = "live-flags:" + ("".join(c for c in "NVZC" if c in flags) or "-")
        else:
            row = rows[0]
        lines.append(" ".join(row))
    with open(out, "w") as f:
        f.write("\n".join(lines) + "\n")


if __name__ == "__main__":
    main()
