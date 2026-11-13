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

## Porting a routine (once phase 4 opens)

1. Pick a routine from the listing, preferably a leaf (it calls nothing unported).
2. Write it in `src/` as readable C, following the portability rules in
   `docs/DECISIONS.md`: fixed-width types, explicit little-endian reads from ROM,
   no floating point, game state at its original WRAM offsets through the lo/hi
   helpers, hardware behind the interfaces.
3. Run the differential test for it on at least the standard movies.
4. Mark it `// TG3000_ROUTINE($bb:aaaa) verified` only when it passes, then run
   `python3 -I tools/progress.py`.

## Mapping and research

Notes about RAM variables, data formats and DSP-4 commands are just as welcome.
Say how you found each fact (trace, breakpoint, experiment) so others can re-check it.

## Pull requests

- One topic per PR; describe what you verified and how.
- CI must be green: it fetches the bench, builds it and checks the progress block.
  It cannot run the game (no ROM in CI), so say in the PR which differential tests
  you ran locally.
