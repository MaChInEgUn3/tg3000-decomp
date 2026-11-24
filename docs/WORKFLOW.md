# Workflow — Top Gear 3000 disassembly + readable C

Operational minimum. `DECISIONS.md` has the scope and the reasoning; read it once.
Progress and the current to-do list live in claude-mem work state, list `tg3000`.

## Layout (target)

```
TG3000.sfc          your ROM (ignored by git; SHA-1 058aadf1...)
docs/               DECISIONS.md, WORKFLOW.md, notes
tools/bench/        reference runner: runs the ROM, traces, diffs the C (may emulate)
tools/              scripts: mapper, listing generator, checks
map/                code/data map produced from traces (committed)
asm/                generated disassembly (committed; must reassemble to the exact ROM)
src/                readable, portable C of the game logic
```

## Bench quick start

```
tools/bench/fetch.sh            # ext/ at pinned commits + DSP-4 patch (refuses to wipe edits)
tools/bench/export-patch.sh     # after editing ext/lakesnes-dinkc64: save edits into patches/
make -C tools/bench             # -> build/bench/bench
build/bench/bench TG3000.sfc 600 --dsp4-log dsp4.log
build/bench/bench TG3000.sfc 3600 --movie tools/bench/movies/start_taps.txt --dump /tmp/f --every 120
build/bench/bench TG3000.sfc 60000 --movie tools/bench/movies/autopilot_60k.txt --coverage traces/coverage.bin
build/bench/bench TG3000.sfc 60000 --movie tools/bench/movies/race1.txt --autopilot 8000 --record new.txt   # new autopilot run
build/bench/bench TG3000.sfc 7650 --movie tools/bench/movies/race1.txt --coverage /tmp/c.bin --explore 40 --explore-cands 6 --explore-known /tmp/known.bin --record new.txt   # coverage explorer (+ --explore-want ADDR, --explore-goal WRAMADR=VAL)
python3 -I tools/map.py && python3 -I tools/progress.py   # -> map/*.txt, README numbers
build/bench/bench TG3000.sfc 60000 --movie tools/bench/movies/autopilot_60k.txt --hdma-report /tmp/hdma.txt   # HDMA setup and writers, handlers, two-task screens
build/bench/bench TG3000.sfc 60000 --movie tools/bench/movies/autopilot_60k.txt --apu-report /tmp/apu.txt     # APU port traffic
build/bench/bench TG3000.sfc 60000 --movie tools/bench/movies/autopilot_60k.txt --dsp4-report /tmp/d4.txt    # DSP-4 commands by site, bytes per run
tools/bench/selftest.sh         # determinism + fresh-process save/load + race reached + all rows (sharded); SELFTEST_DIFFTEST=0 skips the rows
python3 -I tools/listing.py && tools/build_asm.sh   # asm/ -> exact ROM (SHA-1 + code-from-text proof)
tools/refbench/fetch.sh && make -C tools/refbench    # TAS reference: bsnes from BizHawk 1.9.4
build/refbench/refbench TG3000.sfc "Input Log.txt" --firmware ext/firmware/dsp4.rom --bizhawk --coverage traces/coverage.bin
build/refbench/refbench TG3000.sfc "Input Log.txt" --firmware ext/firmware/dsp4.rom --bizhawk --dsp4-mul /tmp/mul.txt   # src/dsp4.c vs the chip
build/refbench/refbench TG3000.sfc "Input Log.txt" --firmware ext/firmware/dsp4.rom --bizhawk --dsp4-catalogue /tmp/cat.txt   # commands + clean-room shadow
build/refbench/refbench TG3000.sfc "Input Log.txt" --firmware ext/firmware/dsp4.rom --bizhawk --frames 50000 --dsp4-cmdlog 0A /tmp/0a.txt   # one command's runs (in, out); checked against src/dsp4.c
build/refbench/refbench TG3000.sfc "Input Log.txt" --firmware ext/firmware/dsp4.rom --bizhawk --frames 20000 --dsp4-tape /tmp/t.bin 0 && python3 -I tools/dsp4_tape.py /tmp/t.bin   # every DSP-4 data byte in order; the $09 streams measured
```

Frame dumps are game assets: write them outside the repo (they are also ignored).
The bench draws the picture only for `--dump` (or `--render`); without it, emulation runs
about three times faster and gives the same per-frame WRAM hashes.

## Phases

1. **Bench.** Embeddable core (licence verified, working SPC700) + DSP-4 mapped at
   the addresses the ROM itself uses. Boot headless, dump PNGs and look at them.
   Prove determinism: same input movie → same per-frame WRAM hash, twice.
   The server is headless and nobody plays by hand, so coverage comes from an
   automated driver: it reads game state from WRAM (menu cursor, game mode, car
   position/speed) to navigate menus and drive races, and a coverage-guided explorer
   tries input variations from save states, keeping whatever executes new code.
2. **Map.** Log every executed PC with its M/X/E/DB/D state; classify every ROM byte
   as code / data / unknown. Statically discovered but never-executed code is
   marked unverified.
3. **Listing.** Generate the disassembly from the map, with labels; it must
   assemble back to the exact ROM (SHA-1 check).
4. **C.** `python3 -I tools/roots.py` ranks what to port next (needs `map/census.txt`:
   `bench ... --census map/census.txt` over the 60k movie); `python3 -I tools/portable.py`
   lists what the listing says is portable now. Port leaf routines first; each must pass the per-routine differential test:
   write the C in `src/`, name it in `map/symbols.txt`, register it with an adapter in
   `tools/bench/routines.c`, then run
   `build/bench/bench TG3000.sfc 9450 --movie tools/bench/movies/race1.txt --difftest map/verified.txt --fuzz 500`
   (and other movies until it is reached) and `python3 -I tools/progress.py`.
   To iterate on a few rows, add `--difftest-only name1,name2` (a scratch results file:
   live counts differ when another row no longer owns the interrupts). Entries an
   interrupt lands in are run again from their entry state in the fuzz phase
   (`replayed N/M`, counted apart from live samples).
   Before committing, verify one of two ways:
   - `tools/verify_new.sh name1,name2` when the unit only adds lines to `src/` and
     `tools/bench/` (new routines, rows and adapters) and does not touch `difftest.c` or
     `platform_bench.c`. It verifies the new rows over the 60k, shop, shop-buy, password, INFODATA, menu and options movies (about 10
     minutes) and merges them into `map/verified.txt`; the other rows keep their last
     result. `tools/bench/selftest.sh` still runs every routine the short movie reaches.
     It refuses a new waiting row (`waitsForInterrupts`): such a row can enclose older rows'
     samples, so it needs the full run.
   - `tools/verify_all.sh` otherwise. It re-verifies every routine from the full autopilot
     movie (in parallel slices: `TG3000_SHARDS`, default 5, plus two each for the explored movies;
     about 14 minutes on 8 cores), and checks that every negative row (deliberately wrong
     C) fails over that movie too.

   Both scripts judge each routine on the evidence pooled over the movies
   (`tools/merge_verified.py`, harness-8). A hard failure on any movie fails it, while the
   100-return flag rule, the live flags and the paths count over all movies together.

   `progress.py --check` (and CI) accepts an older row only while the code changed by
   additions alone since its run, and only for 5 commits. It also refuses when the last
   full run (the `# full run:` line `verify_all.sh` writes into `map/verified.txt`) is
   more than 5 commits back, or when anything in `src/` or `tools/bench/` changed since by
   more than additions. `verify_new.sh` refuses up front in that case.
   `tools/test_carry.sh` tests the rule without the ROM.

## Metrics

- Phase 2–3: bytes classified (code / data / unknown).
- Phase 4: routines passing the differential test / routines discovered.

## Rules

- One unit of work at a time, each with its own check.
- Anything that is the owner's call (licence, scope, commits): stop and ask.
- One local commit per finished unit, gated on its check with `&&`
  (`check && git commit`), never on reading output. Stage paths explicitly.
- The repo is public (github.com/MaChInEgUn3/tg3000-decomp): push each unit's commit
  and check the CI run. Before every commit: `git status --short`, no assets, no
  personal data, and `python3 -I tools/progress.py` if the numbers moved.
