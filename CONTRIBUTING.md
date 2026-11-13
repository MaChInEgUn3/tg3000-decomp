# Contributing

Thanks for helping! Issues and PRs can be written in English or Portuguese.

## Ground rules

- **No game data, ever.** No ROM bytes, no screenshots, no extracted graphics, music
  or text tables. The CI rejects common asset file types; reviewers check the rest.
- **Numbers are measured, not typed.** Progress figures come from `tools/progress.py`;
  facts in `docs/DECISIONS.md` say how they were established.
- **Licence:** contributions are accepted under GPL-3.0-or-later.

## Getting set up

```sh
tools/bench/fetch.sh
make -C tools/bench
build/bench/bench /path/to/TG3000.sfc 600
```

Read `docs/WORKFLOW.md` first and `docs/DECISIONS.md` when you need the reasoning.

## Porting a routine

1. Pick a routine from `asm/`, preferably a leaf (it calls nothing unported and
   touches only WRAM).
2. Write it in `src/` as readable C, following the portability rules in
   `docs/DECISIONS.md`: fixed-width types, WRAM only through `rd16`/`wr16` and
   friends (`src/tg3000.h`), no floating point, no bench headers.
3. Give it a name in `map/symbols.txt` (the asm picks the name up via
   `python3 -I tools/listing.py`).
4. Register it in `tools/bench/routines.c` with an adapter (registers -> arguments),
   its extent, entry contract and fuzz inputs.
5. Run `tools/verify_all.sh` (needs your ROM): it runs the differential test for every
   routine and rewrites `map/verified.txt` and the README numbers. Never edit
   `map/verified.txt` by hand; CI rejects rows whose C, registry or harness changed.
   Hardware rules: multiply/divide is plain C; uploads to video memory are semantic
   calls, never register writes (see `docs/DECISIONS.md`).

## Mapping and research

Notes about RAM variables, data formats and DSP-4 commands are just as welcome.
Say how you found each fact (trace, breakpoint, experiment) so others can re-check it.

## Pull requests

- One topic per PR; describe what you verified and how.
- CI must be green: it fetches the bench, builds it and checks the progress block.
  It cannot run the game (no ROM in CI), so say in the PR which differential tests
  you ran locally.
