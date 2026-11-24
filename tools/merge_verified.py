#!/usr/bin/env python3
"""Merge the differential test's results over several movies into map/verified.txt.

    python3 -I tools/merge_verified.py OUT IN1 IN2 ...
    python3 -I tools/merge_verified.py --update FILE NEW

A routine is verified when some movie verified it and none failed it; failed when any
movie failed it; unreached when no movie reached it. The first verifying row is kept (its
live/fuzz/paths counts), with the live flags of every verifying row joined.

Rows that carry the bench's raw evidence (harness-8: watched: read: mism: hard: compared:
vis:) are judged on the evidence pooled over the movies instead:
- a hard failure on any movie fails the routine (a compared mismatch other than a flag's, a
  lost case, a hardware access, a broken contract, a failed or lost replay);
- a movie that reached the routine but compared nothing gives no evidence (a waiting row a
  slice left unsampled);
- the flags: MIN_WATCHED (100) applies to the returns watched summed over the movies, the
  live set is the union of the flags seen read, and a mismatch on a flag in that set, on
  any movie, fails the routine;
- the paths: the union of the paths visited must cover every instruction start;
- verified then needs at least one movie that compared something;
- harness-17: a flag every watch of every movie found written in the straight-line code
  after a cut (sdead:, endAt rows) is dead there even below MIN_WATCHED;
- harness-15: lagged samples that failed (lagfail:) fail the routine only when no movie has a
  sample without lag that passed (cleanpass:).

--update (tools/verify_new.sh): NEW's rows, already merged over the movies, replace
FILE's rows of the same entry; new entries go at the end; FILE's other rows stay.
"""
import sys

MIN_WATCHED = 100  # tools/bench/difftest.c


def evidence(row):
    """The raw evidence fields of a bench row (harness-8), or None for an older row."""
    ev = {}
    for tok in row[8:]:
        if ":" in tok:
            k, v = tok.split(":", 1)
            ev[k] = v
    return ev if "watched" in ev and "vis" in ev else None


def pooled(rows):
    """The merged row for one routine whose rows all carry evidence (see the docstring)."""
    reached = [r for r in rows if r[7] != "unreached"]
    if not reached:
        return list(rows[0])[:10]
    evs = [(r, evidence(r)) for r in reached]
    hard = [r for r, e in evs if e["hard"] == "1"]
    if hard:
        return list(hard[0])[:10]
    compared = [(r, e) for r, e in evs if int(e["compared"]) > 0]
    watched = sum(int(e["watched"]) for _, e in evs)
    read = set("".join(e["read"] for _, e in evs).replace("-", ""))
    live = set("NVZC") if watched < MIN_WATCHED else set(read)
    watching = [e for _, e in evs if int(e["watched"]) > 0]
    if watching and all("sdead" in e for e in watching):  # harness-17
        sdead = set("NVZC")
        for e in watching:
            sdead &= set(e["sdead"].replace("-", ""))
        live -= sdead - read
    mism = set("".join(e["mism"] for _, e in compared).replace("-", ""))
    # vis: one bit per instruction start, the first the most significant, padded with zero
    # bits to whole hex digits
    seen_bits = 0
    need = int(reached[0][6].split("/")[1])
    for _, e in evs:
        if e["vis"] != "-":
            seen_bits |= int(e["vis"], 16)
    width = (need + 3) // 4 * 4
    full = ((1 << need) - 1) << (width - need) if need else 0
    paths_ok = (seen_bits & full) == full
    seen = bin(seen_bits & full).count("1")
    base = list(compared[0][0]) if compared else list(reached[0])
    base[6] = f"{seen}/{need}"
    base[8] = "live-flags:" + ("".join(c for c in "NVZC" if c in live) or "-")
    lagfail = sum(int(e.get("lagfail", "0")) for _, e in evs)
    cleanpass = sum(int(e.get("cleanpass", "0")) for _, e in evs)
    ok = bool(compared) and paths_ok and not (live & mism) and (lagfail == 0 or cleanpass > 0)
    base[7] = "verified" if ok else "failed"
    return base[:10]  # the committed columns (entry .. replayed), without the raw evidence


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


def update(path, new_path):
    heads = [l.rstrip("\n") for l in open(path) if l.startswith("#")]  # the full-run mark too
    _, rows, order = read(path)
    _, new_rows, new_order = read(new_path)
    for entry in new_order:
        if entry not in rows:
            order.append(entry)
        rows[entry] = new_rows[entry]
    with open(path, "w") as f:
        f.write("\n".join(heads + [" ".join(rows[e]) for e in order]) + "\n")


def main():
    if sys.argv[1] == "--update":
        update(sys.argv[2], sys.argv[3])
        return
    out, inputs = sys.argv[1], sys.argv[2:]
    tables = [read(p) for p in inputs]
    header = tables[0][0]
    order = list(tables[0][2])
    for _, _, o in tables[1:]:
        order += [e for e in o if e not in order]
    lines = [header.rstrip("\n")]
    for entry in order:
        rows = [t[1][entry] for t in tables if entry in t[1]]
        if all(evidence(r) is not None or r[7] == "unreached" for r in rows) and any(evidence(r) for r in rows):
            lines.append(" ".join(pooled([r for r in rows if evidence(r) is not None] or rows)))
            continue
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
