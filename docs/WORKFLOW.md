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
python3 -I tools/map.py && python3 -I tools/progress.py   # -> map/*.txt, README numbers
tools/bench/selftest.sh         # determinism + fresh-process save/load + race reached
python3 -I tools/listing.py && tools/build_asm.sh   # asm/ -> exact ROM (SHA-1 + code-from-text proof)
```

Frame dumps are game assets: write them outside the repo (they are also ignored).

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
4. **C.** Port leaf routines first; each must pass the per-routine differential test:
   write the C in `src/`, name it in `map/symbols.txt`, register it with an adapter in
   `tools/bench/routines.c`, then run
   `build/bench/bench TG3000.sfc 9450 --movie tools/bench/movies/race1.txt --difftest map/verified.txt --fuzz 500`
   (and other movies until it is reached) and `python3 -I tools/progress.py`.

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
