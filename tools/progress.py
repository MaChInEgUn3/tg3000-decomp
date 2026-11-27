#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Regenerate the progress block in README.md and README.pt-br.md.

    python3 -I tools/progress.py          # rewrite the blocks
    python3 -I tools/progress.py --check  # exit 1 if a block is stale (CI)

Sources (all optional until they exist): map/rom_map.txt for the code/data map and
map/verified.txt (written by `bench --difftest`) for ported routines; --check also fails
when a verified routine's C source changed after it was verified. The numbers are counted, never typed by hand.

A verified row is current when its hash matches the code as it is now. A row from an
earlier run is carried when its hash matches the code at a commit at most MAX_BEHIND
commits back and the code changed only by additions since (no line anywhere in src/ or
tools/bench/ removed or changed; difftest.c and platform_bench.c not touched at all):
tools/verify_new.sh verifies new rows alone, tools/verify_all.sh every row. Any other
row is stale.

Whatever the rows, --check also fails when the last full run (the "# full run:" line
tools/verify_all.sh writes into map/verified.txt; its commit by git blame) is more than
MAX_BEHIND commits back, or when src/ or tools/bench/ changed since by more than
additions: a changed line with no row of its own (a header, the bench's main) is caught
there.
"""
import functools
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROM_SIZE = 0x100000
BEGIN, END = "<!-- BEGIN GENERATED PROGRESS -->", "<!-- END GENERATED PROGRESS -->"


def classified_bytes():
    """map/rom_map.txt: lines "<start hex> <end hex exclusive> <code|data|unknown> ...".
    Returns (code, data) byte counts."""
    path = os.path.join(ROOT, "map", "rom_map.txt")
    code = data = 0
    if os.path.exists(path):
        with open(path) as f:
            for line in f:
                parts = line.split()
                if len(parts) < 3 or line.startswith("#"):
                    continue
                n = int(parts[1], 16) - int(parts[0], 16)
                if parts[2] == "code":
                    code += n
                elif parts[2] == "data":
                    data += n
    return code, data


HASHED = ("tools/bench/routines.c", "tools/bench/difftest.c", "tools/bench/platform_bench.c")
FROZEN = ("tools/bench/difftest.c", "tools/bench/platform_bench.c")  # no change at all
MAX_BEHIND = 5  # commits a carried row's run may lie back (then tools/verify_all.sh)


def fnv(chunks):
    h = 0xcbf29ce484222325
    for data in chunks:
        for b in data:
            h = ((h ^ b) * 0x100000001b3) & 0xFFFFFFFFFFFFFFFF
    return h


@functools.lru_cache(maxsize=None)
def verification_hash(source):
    """FNV-1a 64 over the C source, the bench's registry and its harness, in that order
    (the bench's verificationHash in tools/bench/difftest.c)."""
    chunks = []
    for rel in (source,) + HASHED:
        path = os.path.join(ROOT, rel)
        if not os.path.exists(path):
            return None
        chunks.append(open(path, "rb").read())
    return fnv(chunks)


def git(*args):
    try:
        return subprocess.run(("git", "-C", ROOT) + args, capture_output=True, check=True).stdout
    except (OSError, subprocess.CalledProcessError):
        return None


_commits = None


def recent_commits():
    """The commits a carried row may come from: the last MAX_BEHIND counting the one being
    made (with any uncommitted change to a tracked file, HEAD is one back already: the
    check runs before that commit)."""
    global _commits
    if _commits is None:
        dirty = git("status", "--porcelain", "--untracked-files=no")
        start = "HEAD" if dirty else "HEAD~1"
        out = git("rev-list", "-n", str(MAX_BEHIND), start, "--")
        _commits = out.decode().split() if out else []
    return _commits


_hash_at = {}


def hash_at(commit, source):
    key = (commit, source)
    if key not in _hash_at:
        chunks = [git("show", f"{commit}:{rel}") for rel in (source,) + HASHED]
        _hash_at[key] = None if any(c is None for c in chunks) else fnv(chunks)
    return _hash_at[key]


@functools.lru_cache(maxsize=None)
def only_additions(commit):
    """True when, from `commit` to now, src/ and tools/bench/ gained lines only and the
    harness is untouched."""
    if git("diff", "--quiet", commit, "--", *FROZEN) is None:
        return False
    diff = git("diff", "-U0", "--no-color", commit, "--", "src", "tools/bench")
    if diff is None:
        return False
    return not any(l.startswith("-") and not l.startswith("---") for l in diff.decode().splitlines())


FULL_RUN = "# full run:"


def last_full_run():
    """(line number, commit) of the full-run mark in map/verified.txt: commit None when
    the mark is not committed yet (a full run now); (None, None) without a mark."""
    path = os.path.join(ROOT, "map", "verified.txt")
    if not os.path.exists(path):
        return None, None
    for n, line in enumerate(open(path), 1):
        if line.startswith(FULL_RUN):
            out = git("blame", "--porcelain", "-L", f"{n},{n}", "--", "map/verified.txt")
            sha = out.decode().split()[0] if out else None
            return n, (None if not sha or set(sha) == {"0"} else sha)
    return None, None


def full_run_when():
    """The full-run mark's own text (its time): it does not change when committed."""
    path = os.path.join(ROOT, "map", "verified.txt")
    if os.path.exists(path):
        for line in open(path):
            if line.startswith(FULL_RUN):
                return line[len(FULL_RUN):].split("(")[0].strip()
    return None


def full_run_problem():
    """Why the last full run no longer covers the code, or None."""
    n, commit = last_full_run()
    if n is None:
        return "map/verified.txt has no full-run mark: run tools/verify_all.sh"
    if commit is None:
        return None  # the full run is part of the commit being made
    head = git("rev-parse", "HEAD")
    allowed = set(recent_commits())
    dirty = git("status", "--porcelain", "--untracked-files=no")
    if not dirty and head:
        allowed.add(head.decode().strip())
    if commit not in allowed:
        return f"the last full run ({commit[:7]}) is more than {MAX_BEHIND} commits back: run tools/verify_all.sh"
    if not only_additions(commit):
        return (f"src/ or tools/bench/ changed by more than additions since the last full run ({commit[:7]}): "
                "run tools/verify_all.sh")
    return None


def row_status(parts):
    """("current", None), ("carried", commit) or ("stale", None) for a verified row."""
    want = int(parts[3], 16)
    if verification_hash(parts[2]) == want:
        return "current", None
    # a recent commit with that code, src/ and tools/bench/ only added to since (the oldest such:
    # the row's code there is the code its run compared, wherever in the window the run was).
    # Not just the oldest match: a row whose own files did not change can match commits from
    # before a full run that changed other files, and the additions rule would fail there.
    matches = [c for c in recent_commits() if hash_at(c, parts[2]) == want]
    for c in reversed(matches):
        if only_additions(c):
            return "carried", c
    return "stale", None


@functools.lru_cache(maxsize=None)
def verified_rows():
    """map/verified.txt's verified rows with their status."""
    path = os.path.join(ROOT, "map", "verified.txt")
    rows = []
    if os.path.exists(path):
        for line in open(path):
            parts = line.split()
            if line.startswith("#") or len(parts) < 8 or parts[7] != "verified":
                continue
            rows.append((tuple(parts),) + row_status(parts))
    return tuple(rows)


def ported_routines():
    """Routine entry points the bench verified (map/verified.txt, written by
    `bench --difftest`), current or carried. Returns (count, stale rows, carried counts by
    commit)."""
    count, stale, carried = 0, [], {}
    for parts, status, commit in verified_rows():
        if status == "stale":
            stale.append(f"{parts[0]} {parts[1]} ({parts[2]}: the registry or the harness changed since it "
                         f"was verified, by more than additions or more than {MAX_BEHIND} commits ago)")
            continue
        count += 1
        if status == "carried":
            carried[commit] = carried.get(commit, 0) + 1
    return count, stale, carried


def verified_code_bytes():
    """ROM code bytes (rom_map "code") inside the extents of the routines map/verified.txt
    lists as verified and current: [entry, end) minus holes, from the bench's registry
    (tools/bench/routines.c: `routines[]` and the composed rows `composedRoutines[]`, a screen
    whole, whose extent also covers the calls between its parts). Overlapping extents count
    once."""
    reg_path = os.path.join(ROOT, "tools", "bench", "routines.c")
    ver_path = os.path.join(ROOT, "map", "verified.txt")
    map_path = os.path.join(ROOT, "map", "rom_map.txt")
    if not (os.path.exists(reg_path) and os.path.exists(ver_path) and os.path.exists(map_path)):
        return 0
    good = {(int(parts[0], 16), parts[1]) for parts, status, _ in verified_rows() if status != "stale"}
    whole = open(reg_path).read()
    text = whole[whole.index("const Routine routines[]"):whole.index("const int routineCount")]
    if "const Routine composedRoutines[]" in whole:
        text += whole[whole.index("const Routine composedRoutines[]"):whole.index("const int composedRoutineCount")]
    covered = set()
    for row in re.split(r"\n\s*\{\.name = ", text)[1:]:
        m = re.search(r"\.entry = 0x([0-9A-Fa-f]+), \.end = 0x([0-9A-Fa-f]+)", row)
        if not m or (int(m.group(1), 16), row.split('"')[1]) not in good:
            continue
        holes = [(int(a, 16), int(b, 16)) for a, b in re.findall(r"\{0x([0-9A-Fa-f]+), 0x([0-9A-Fa-f]+)\}", row)]
        for adr in range(int(m.group(1), 16), int(m.group(2), 16)):
            if not any(a <= adr < b for a, b in holes):
                covered.add(((adr >> 16) & 0x7F) << 15 | (adr & 0x7FFF))
    code = bytearray(ROM_SIZE)
    for line in open(map_path):
        parts = line.split()
        if line.startswith("#") or len(parts) < 3 or parts[2] != "code":
            continue
        for o in range(int(parts[0], 16), int(parts[1], 16)):
            code[o] = 1
    return sum(1 for o in covered if code[o])


def pct(n, d):
    return f"{100 * n / d:.2f}%" if d else "n/a"


def block(lang):
    code, data = classified_bytes()
    known = code + data
    ported, _, carried = ported_routines()
    in_c = verified_code_bytes()
    n_carried = sum(carried.values())
    full_txt = full_run_when() or "-"
    oldest = max(carried, key=recent_commits().index) if carried else None  # the furthest back
    short = oldest[:7] if oldest else ""
    rows = {
        "en": [
            ("ROM bytes classified (code + data)", f"**{known:,} / {ROM_SIZE:,} ({pct(known, ROM_SIZE)})**"),
            ("Code bytes", f"{code:,} (`0x{code:X}`)"),
            ("Data bytes", f"{data:,} (`0x{data:X}`)"),
            ("Routine entry points in C, verified by the differential test", f"**{ported:,}**"),
            ("Last full re-verification of every routine", full_txt),
            ("Of those, verified at an earlier commit, the code only added to since",
             f"{n_carried:,} (oldest run: `{short}`)" if n_carried else "0 (all against this code)"),
            ("Code bytes inside verified C routines", f"**{in_c:,} / {code:,} ({pct(in_c, code)})**"),
        ],
        "pt": [
            ("Bytes da ROM classificados (código + dados)", f"**{known:,} / {ROM_SIZE:,} ({pct(known, ROM_SIZE)})**"),
            ("Bytes de código", f"{code:,} (`0x{code:X}`)"),
            ("Bytes de dados", f"{data:,} (`0x{data:X}`)"),
            ("Pontos de entrada de rotinas em C, aprovados no teste diferencial", f"**{ported:,}**"),
            ("Última reverificação completa de todas as rotinas", full_txt),
            ("Destes, aprovados num commit anterior, com o código só acrescido desde então",
             f"{n_carried:,} (rodada mais antiga: `{short}`)" if n_carried else "0 (todos contra este código)"),
            ("Bytes de código dentro de rotinas em C aprovadas", f"**{in_c:,} / {code:,} ({pct(in_c, code)})**"),
        ],
    }[lang]
    head = ("| Metric | Current |" if lang == "en" else "| Métrica | Atual |")
    foot = ("_Generated by `tools/progress.py` from `map/` and `src/`._" if lang == "en"
            else "_Gerado por `tools/progress.py` a partir de `map/` e `src/`._")
    lines = [BEGIN, "", head, "|---|---:|"] + [f"| {k} | {v} |" for k, v in rows] + ["", foot, "", END]
    return "\n".join(lines)


def lost_rows():
    """Rows verified at HEAD that the working map/verified.txt no longer has as verified."""
    try:
        head = subprocess.run(["git", "-C", ROOT, "show", "HEAD:map/verified.txt"], capture_output=True,
                              text=True, check=True).stdout
    except (OSError, subprocess.CalledProcessError):
        return []
    def verified(text):
        rows = {}
        for line in text.splitlines():
            parts = line.split()
            if line.startswith("#") or len(parts) < 8:
                continue
            rows[parts[0] + " " + parts[1]] = (parts[1], parts[7])  # (a composed row shares an entry)
        return rows
    before = verified(head)
    with open(os.path.join(ROOT, "map", "verified.txt")) as f:
        now = verified(f.read())
    # dsp4-08g: a routine folded into its only caller's row (map/symbols.txt: "folded into NAME")
    # may leave the table, when that row is verified now
    folded = {}
    with open(os.path.join(ROOT, "map", "symbols.txt")) as f:
        for line in f:
            parts = line.split()
            m = re.search(r"folded into (\w+)", line)
            if len(parts) >= 3 and parts[0] == "func" and m:
                folded[parts[1] + " " + parts[2]] = m.group(1)
    verified_now = {name for name, status in now.values() if status == "verified"}
    lost = []
    for entry, (name, status) in sorted(before.items()):
        if status != "verified":
            continue
        after = now.get(entry, (name, "missing"))[1]
        if after == "missing" and folded.get(entry) in verified_now:
            continue
        if after != "verified":
            lost.append("%s %s (verified at HEAD, now %s)" % (entry, name, after))
    return lost


def duplicate_symbols():
    """ROM addresses map/symbols.txt names more than once (a routine ported twice)."""
    seen, dups = {}, []
    with open(os.path.join(ROOT, "map", "symbols.txt")) as f:
        for line in f:
            parts = line.split()
            if len(parts) < 3 or parts[0] != "func":
                continue
            if parts[1] in seen:
                dups.append("%s %s and %s" % (parts[1], seen[parts[1]], parts[2]))
            else:
                seen[parts[1]] = parts[2]
    return dups


def main():
    check = "--check" in sys.argv[1:]
    _, stale_rows, _ = ported_routines()
    if check:
        dups = duplicate_symbols()
        if dups:
            for d in dups:
                print("named twice in map/symbols.txt: " + d, file=sys.stderr)
            return 1
        lost = lost_rows()
        if lost:
            for r in lost:
                print("lost verification: " + r, file=sys.stderr)
            return 1
    problem = full_run_problem()
    if check and problem:
        print(problem, file=sys.stderr)
        return 1
    if check and stale_rows:
        for r in stale_rows:
            print("stale verification: " + r, file=sys.stderr)
        print("re-run the differential test: tools/verify_all.sh (see CONTRIBUTING.md)", file=sys.stderr)
        return 1
    stale = False
    for name, lang in (("README.md", "en"), ("README.pt-br.md", "pt")):
        path = os.path.join(ROOT, name)
        with open(path) as f:
            text = f.read()
        start, stop = text.index(BEGIN), text.index(END) + len(END)
        new = text[:start] + block(lang) + text[stop:]
        if new != text:
            stale = True
            if not check:
                with open(path, "w") as f:
                    f.write(new)
    if check and stale:
        print("progress block is stale: run python3 -I tools/progress.py", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
