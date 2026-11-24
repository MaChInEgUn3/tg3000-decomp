#!/usr/bin/env python3
"""harness-24: rerun alone the rows a slice's waiting row hid entries from.

    python3 -I tools/rerun_hidden.py TMP ROM

Called by tools/verify_all.sh after the first merge. In a slice of a movie, a waiting row's
sample records the interrupts that land in it, and a row entered inside one of those
interrupt handlers is not sampled there ("N entries inside interrupt handlers a waiting
sample recorded (not sampled)" in the slice's log). Which rows share a slice moves whenever
a row is added, so such a row can lose the only entries that reach some of its paths.

For every row the merge failed (map/verified.txt) with no hard failure on any movie (hard:0
in every movie's results), and for every movie whose log reports hidden entries for it, the
row runs again alone over that movie (one process each: rows run together can hide each
other) and its line replaces the one in that movie's results file (replaced, not added: the
same returns must not count twice toward the 100-watched flag rule). verify_all.sh then
merges again. Prints the rows rerun; exits 0 whether or not any was.
"""
import os
import re
import subprocess
import sys

# result file stem, log glob prefix, movie, frames (None: from the movie's header)
MOVIES = [
    ("main", "main", "autopilot_60k", 60000),
    ("shop", "shop", "explore_shop", None),
    ("buy", "buy", "explore_buy", None),
    ("pw", "pw", "explore_pw", None),
    ("info", "info", "code_infodata", None),
    ("menu", "menu", "explore_menu", None),
    ("opt", "opt", "explore_options", None),
    ("vs", "vs", "vs_race", None),
]
# the VS race runs with the firmware DSP-4, the VS screen's frame not fuzzed there (harness-28)
EXTRA = {"vs_race": ["--dsp4-firmware", os.environ.get("TG3000_DSP4_FIRMWARE", "ext/firmware/dsp4.rom"),
                     "--fuzz-skip", "vs_screen_frame,track_rows_build,player_car_frame"]}
FUZZ = {}
HIDDEN = "inside interrupt handlers a waiting sample recorded"


def frames_of(movie):
    text = open(os.path.join("tools/bench/movies", movie + ".txt")).read()
    m = re.search(r"Run it for (\d+) steps", text)
    return int(m.group(1))


def result_lines(path):
    rows = {}
    if not os.path.exists(path):
        return rows
    for line in open(path):
        if line.startswith("#"):
            continue
        parts = line.split()
        if len(parts) > 1:
            rows[parts[1]] = line
    return rows


def hidden_rows(tmp, prefix):
    names = set()
    for f in sorted(os.listdir(tmp)):
        if not (f.startswith(prefix) and f.endswith(".log")):
            continue
        rest = f[len(prefix):-4]
        if rest and not rest.isdigit():
            continue
        current = None
        for line in open(os.path.join(tmp, f), errors="replace"):
            m = re.match(r"difftest (\S+)\s", line)
            if m:
                current = m.group(1)
            elif current and HIDDEN in line:
                names.add(current)
    return names


def main():
    tmp, rom = sys.argv[1], sys.argv[2]
    failed = []
    for line in open("map/verified.txt"):
        parts = line.split()
        if not line.startswith("#") and len(parts) > 7 and parts[7] == "failed":
            failed.append(parts[1])
    if not failed:
        return
    results = {stem: result_lines(os.path.join(tmp, stem + ".txt")) for stem, _, _, _ in MOVIES}
    jobs = []
    for name in failed:
        lines = [results[stem][name] for stem, _, _, _ in MOVIES if name in results[stem]]
        if any(not re.search(r"\bhard:0\b", ln) for ln in lines):
            continue  # a hard failure stands
        for stem, prefix, movie, frames in MOVIES:
            if name in results[stem] and name in hidden_rows(tmp, prefix):
                jobs.append((name, stem, movie, frames or frames_of(movie)))
    procs = []
    for name, stem, movie, frames in jobs:
        out = os.path.join(tmp, "rerun-%s-%s.txt" % (stem, name))
        log = open(os.path.join(tmp, "rerun-%s-%s.log" % (stem, name)), "w")
        cmd = (["build/bench/bench", rom, str(frames), "--movie", "tools/bench/movies/%s.txt" % movie]
               + EXTRA.get(movie, []) + ["--difftest", out, "--difftest-only", name, "--fuzz", FUZZ.get(movie, "500")])
        procs.append((name, stem, out, subprocess.Popen(cmd, stdout=log, stderr=subprocess.STDOUT)))
    for name, stem, out, p in procs:
        status = p.wait()
        if status not in (0, 4):
            print("rerun_hidden: %s on %s: bench status %d" % (name, stem, status), file=sys.stderr)
            continue
        solo = result_lines(out).get(name)
        if solo is None:
            continue
        path = os.path.join(tmp, stem + ".txt")
        text = open(path).read().split("\n")
        text = [solo.rstrip("\n") if (not ln.startswith("#") and len(ln.split()) > 1 and ln.split()[1] == name)
                else ln for ln in text]
        open(path, "w").write("\n".join(text))
        print("harness-24: %s rerun alone over %s (a waiting row hid its entries in its slice)" % (name, stem))


if __name__ == "__main__":
    main()
