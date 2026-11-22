#!/usr/bin/env python3
"""Tests of tools/merge_verified.py's pooled rule (harness-8), without the ROM.

    python3 -I tools/test_merge.py

Each case writes two per-movie result files as the bench would, merges them, and checks the
routine's merged status, paths and live flags.
"""
import os
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
HEAD = "# entry name source hash live fuzz paths status live-flags  -- written by the bench (--difftest)\n"


def row(status, live, fuzz, seen, need, watched, read, mism, hard, compared, vis, flags="-"):
    return (f"80ABCD test_row src/x.c 0123456789abcdef {live} {fuzz} {seen}/{need} {status} live-flags:{flags} "
            f"replayed:0 watched:{watched} read:{read} mism:{mism} hard:{hard} compared:{compared} vis:{vis}\n")


def merge(a, b):
    with tempfile.TemporaryDirectory() as d:
        paths = []
        for name, line in (("a.txt", a), ("b.txt", b)):
            p = os.path.join(d, name)
            with open(p, "w") as f:
                f.write(HEAD + line)
            paths.append(p)
        out = os.path.join(d, "out.txt")
        subprocess.run([sys.executable, "-I", os.path.join(ROOT, "tools", "merge_verified.py"), out] + paths, check=True)
        lines = [l.split() for l in open(out) if not l.startswith("#")]
        assert len(lines) == 1, lines
        return lines[0]


CASES = [
    # (name, movie A, movie B, status, paths, live flags)
    ("both verify", row("verified", 200, 500, 8, 8, 200, "C", "-", 0, 700, "ff"),
     row("verified", 50, 500, 8, 8, 50, "-", "-", 0, 550, "ff"), "verified", "8/8", "C"),
    ("a hard failure on one movie fails it", row("verified", 200, 500, 8, 8, 200, "-", "-", 0, 700, "ff"),
     row("failed", 50, 499, 8, 8, 50, "-", "-", 1, 549, "ff"), "failed", "8/8", None),
    ("few returns on one movie: pooled over the threshold", row("verified", 200, 500, 8, 8, 200, "-", "VZ", 0, 700, "ff"),
     row("failed", 20, 500, 8, 8, 20, "-", "VZ", 0, 520, "ff"), "verified", "8/8", "-"),
    ("under the threshold even pooled: every flag live", row("failed", 30, 500, 8, 8, 30, "-", "Z", 0, 530, "ff"),
     row("failed", 20, 500, 8, 8, 20, "-", "-", 0, 520, "ff"), "failed", "8/8", "NVZC"),
    ("a flag read on one movie and mismatched on the other fails it",
     row("verified", 200, 500, 8, 8, 200, "C", "-", 0, 700, "ff"),
     row("failed", 20, 500, 8, 8, 20, "-", "C", 0, 520, "ff"), "failed", "8/8", "C"),
    ("paths: the union covers them", row("failed", 200, 500, 5, 8, 200, "-", "-", 0, 700, "f8"),
     row("failed", 50, 500, 3, 8, 50, "-", "-", 0, 550, "07"), "verified", "8/8", "-"),
    ("paths: the union still misses one", row("failed", 200, 500, 5, 6, 200, "-", "-", 0, 700, "f8"),
     row("failed", 50, 500, 3, 6, 50, "-", "-", 0, 550, "e0"), "failed", "5/6", "-"),
    ("reached but nothing compared on one movie: no evidence from it",
     row("verified", 200, 500, 8, 8, 200, "-", "-", 0, 700, "ff"),
     row("failed", 0, 0, 0, 8, 0, "-", "-", 0, 0, "00"), "verified", "8/8", "-"),
    ("nothing compared on any movie fails it", row("failed", 0, 0, 0, 8, 0, "-", "-", 0, 0, "00"),
     row("failed", 0, 0, 0, 8, 0, "-", "-", 0, 0, "00"), "failed", "0/8", None),
]


def main():
    bad = 0
    for name, a, b, status, paths, flags in CASES:
        got = merge(a, b)
        ok = got[7] == status and got[6] == paths and (flags is None or got[8] == "live-flags:" + flags)
        print(("ok  " if ok else "FAIL") + f" {name}: {got[7]} {got[6]} {got[8]}")
        bad += not ok
    if bad:
        sys.exit(1)


if __name__ == "__main__":
    main()
