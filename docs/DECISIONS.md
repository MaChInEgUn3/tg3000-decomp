# Decisions & confirmed facts

Single source of truth for project scope and state. Update it whenever a decision
changes or a fact is confirmed, so any person or agent session can resume without
re-deriving anything. Dates are absolute.

## Scope (2026-11-13)

- **This repository = byte-for-byte disassembly + readable C of the game logic.**
  Platform ports (Linux x86-64, Windows x86-64, maybe Mega Drive/Genesis one day)
  live in a **separate repository** that consumes the C from here.
- **Readable C from the start.** No static recompiler and no "ugly generated C"
  phase. Offered and rejected.
- **Two deliverables (owner, confirmed 2026-11-13):** a byte-for-byte disassembly
  (the generated `.asm` must assemble back to the exact ROM, SHA-1 checked) **and**
  the readable C. Earlier the `.asm` was planned as a reference listing only; the
  owner expects both, and making the generated listing reassemble is cheap, so it
  is a deliverable again. Byte-matching applies to the `.asm` only: the C cannot
  byte-match hand-written 65816, so the C is proven by differential testing.
- **"Not even 1% emulator" in the shipped game.** No CPU/PPU/SPC700/DSP-4 model in
  any port. DSP-4 commands become native game functions; video and the sound driver
  are rewritten natively in the port repository.
- **The bench (`tools/bench/`) is allowed to emulate.** It runs the original ROM to
  produce the ground truth the C is compared against. It is a measuring instrument,
  never linked into the game, never shipped.

## Portability rules for the C (because of a possible 68000 port)

- Fixed-width types only (`uint8_t`, `int16_t`, ...).
- ROM data read byte by byte with explicit little-endian helpers; never cast a
  pointer into ROM/RAM to a wider type.
- No floating point; keep the original fixed-point math.
- Game state lives at the original WRAM offsets, so the bench can diff it. Access
  16-bit values through explicit lo/hi helpers, never by aliasing two bytes.
- Video, sound and DSP-4 sit behind interfaces. **Do not design `video_*`/`sound_*`
  yet**: the boundary is wherever the code/data map shows the game building its
  RAM buffers and uploading them.

## Verification

- Per-routine differential test (primary): when the bench's interpreter reaches the
  entry of a ported routine, it snapshots state, runs the C on a copy, lets the
  original run to its return, then diffs WRAM, returned registers/flags and the
  MMIO/DSP-4 writes. This allows porting leaves first, each with pass/fail.
- Per-frame WRAM hash (secondary), once whole frame paths are ported.
- Video/sound are rewritten, so they are checked by eye/ear or tolerant screenshot
  comparison, not by lockstep.

## Bench core (2026-11-13)

- **`dinkc64/LakeSnes`** (fork of `angelo-wf/LakeSnes`, which is archived), commit
  `048a0d7` (2025-04-11). MIT, plain C, SPC700 + S-DSP included, already has a
  coprocessor mapper hook in `snes/cart.c` (the CX4, cart type 4) to copy for the
  DSP-4. The 12 `snes/*.c` files compile headless with `gcc -O2 -Wall`, no SDL
  needed. Fetched into `ext/` (ignored), not vendored yet.
- Rejected: snes9x (non-free licence), bsnes/Mesen2 (GPL, large, and their DSP-4
  is low-level emulation needing the chip firmware, which we don't have).
- The SPC700 is only proven to work once the ROM gets past the sound-driver
  handshake (bench-3).

## Bench boot check (bench-3, 2026-11-13)

- Headless boot works: Kemco/Gremlin title (~frame 120), the full intro story
  ("Once every millennium...", frames ~400-2900), then the main menu
  (Championship / VS Mode) at ~frame 3000. The SPC700 handshake therefore passes,
  and the picture renders correctly (`--dump DIR --every N`, 256x224 PNG).
- Start does not skip the intro. In the menu, Start/A/B taps lead to a
  Difficulty / Music / Exit screen; no race reached by blind tapping, so **still zero
  DSP-4 writes after 9,000 frames**. Expected: the chip is for the road. Reaching a
  race is left to the automated driver (the server is headless; the owner does
  not play by hand — decided 2026-11-13).
- Movie format: `tools/bench/movies/*.txt`, one `<frame> <buttons>` per line, keyed
  by the bench's loop index (not `snes->frames`, which skipped one value around
  frame 2,900 in a run; check this in bench-4).

## Bench determinism (bench-4, 2026-11-13)

- The bench now steps one video frame per iteration by running until LakeSnes's
  frame counter changes (`snes_runFrame` stops at vblank and once covered two frames
  in one call). Movie frame N is video frame N.
- `--hashes FILE` writes `<frame> <FNV-1a 64 of WRAM>` per frame. Two fresh runs of
  `movies/to_race.txt`, 9,000 frames: **identical**, and 8,959 distinct hashes, so
  the hash moves almost every frame (it would catch a divergence).
- WRAM powers on zero-filled (LakeSnes `ramFill = 0`). Real hardware is not zeroed;
  any routine that reads uninitialised WRAM will be found by the differential test.

## Bench save states and patch safety (bench-fix, 2026-11-13)

- LakeSnes save states now include the DSP-4 (cart type 5 → the bridge's fixed-size
  blob from `Dsp4::saveState`; a rejected blob aborts the bench rather than continue
  with a wrong chip). `--state-test N`: save, run N frames, reload, run them again;
  WRAM hashes and the final full state must match. At frame 3,100 + 600 frames:
  identical (state = 270,705 bytes). Caveat: no DSP-4 command has run by then, so
  the chip's part of the round trip is only exercised once a race is reached.
- `fetch.sh` refuses to run when `ext/lakesnes-dinkc64` has edits that differ from
  the patch (they would be wiped); `tools/bench/export-patch.sh` saves them,
  `FORCE=1` discards them. Tested: clean re-run, refuse-and-keep, force, export
  round trip.

## Code/data map (map-1, 2026-11-13)

- LakeSnes patch adds two hooks: `cpu_traceHook` (before each opcode fetch, not
  interrupts) and `cart_romReadHook` (every LoROM ROM read, CPU or DMA).
- `--coverage traces/coverage.bin` keeps one flag byte per ROM byte (instruction
  start / code byte / read / M-X widths seen / emulation mode), OR-merged across
  runs; addresses executed outside ROM go to `coverage.bin.ram`.
- `tools/map.py` turns it into the committed `map/rom_map.txt` (code/data/unknown
  ranges), `map/code_starts.txt` (instruction starts with widths, CPU address in the
  `$80-$FF` FastROM mirror) and `map/ram_exec.txt`. It fails if any start lies inside
  another start's operand under the same widths (65816 length check).
- **Code in data = "read and not code"**, so data the code reads but never runs is
  data; code that is only read (e.g. copied) would count as data until executed.
- Interrupt vectors (native): COP/BRK → `$8000`, ABORT → `$89CC`, **NMI → `$00:0000`
  and IRQ → `$00:0004` in WRAM** (trampolines the game writes into RAM; the NMI one
  runs once a frame, the IRQ one during races). Emulation: **RESET → `$8002`**,
  COP/BRK → `$8000`, others → `$89CC`. `$8000` is `80 FE` = `BRA *`, a hang trap for
  BRK/COP (never executed so far).
- First map, from the two menu movies only (no race): **11,360 code bytes, 72,074
  data bytes, 7.96% of the ROM classified**, 5,194 instruction starts (mostly 16-bit
  A with 8-bit index, or all 16-bit).

## Reaching a race; save-state fix (driver-1, 2026-11-13)

- Menu route, found screen by screen from save states (`movies/race1.txt`):
  intro (Start taps; it cannot be skipped) → main menu, **A** opens the Championship
  submenu (OPTIONS / 1 PLAYER FULL / 1 PLAYER SPLIT / 2 PLAYERS / PASSWORD / EXIT),
  Down + A = 1 PLAYER FULL → NAME / CONTROL / SPEED / RACE, Down x3 + A → track
  description (system Merak, track Misk) → A → shop (Paintshop, Engine, Tires,
  Gearbox, Armor, Boost, EXIT below after scrolling) → Down x6 + A → **race**.
  First DSP-4 write at frame 7,930; the race renders correctly on the bench
  (HUD, mountains, road, car).
- **LakeSnes bug fixed (in our patch):** the dinkc64 fork caches memory access times
  in a table built from `fastMem`; loading a save state restored `fastMem` but not the
  table, so a state loaded in a *fresh process* ran FastROM code at SlowROM speed
  (the race then broke: garbled raster effects, one instruction spanning 5-7 frames).
  The table is now rebuilt after loading. The earlier `--state-test` missed it because
  it saved and loaded inside one process.
- `tools/bench/selftest.sh` (local, needs the ROM): determinism over 9,450 frames,
  race reached with DSP-4 traffic, and a state saved at frame 7,650 then loaded in a
  new process matches the uninterrupted run for 1,800 race frames.
- Coverage with the race: **41,306 code bytes, 211,528 data bytes, 24.11% of the ROM
  classified**; the IRQ trampoline at `$00:0004` runs during the race.
- Driving is not automated yet: the car just sits/accelerates as the movie leaves it.

## Byte-for-byte disassembly (listing-1, 2026-11-13)

- `tools/listing.py` writes `asm/main.asm` + `asm/bank_XX.asm` (asar syntax, `lorom`,
  one file per 32 KB bank at its `$80-$9F` FastROM address). Executed code becomes
  instructions with explicit sizes (`.b/.w/.l`) and `CODE_bbaaaa` labels on branch
  targets (asar needs labels for branches; a raw number is taken as the offset).
  Code that cannot be decoded unambiguously (width-dependent opcode seen with two
  widths, crossing a bank, `per`, branch to a non-instruction) is `db`. Everything else
  is `incbin !baserom:$a..$b` from the user's ROM — no game data in the repository.
- `tools/build_asm.sh` assembles with asar v1.91 (built by `fetch.sh` into
  `ext/asar-build/`) and checks SHA-1 `058aadf1...`. A second build feeds asar a copy
  of the ROM with all code bytes zeroed and must still match: **all code comes from
  the asm text**. First result: 17,590 instructions, 122 code bytes as `db`,
  1,073 labels — identical ROM on the first try.
- The asm grows with the map: as coverage rises, `incbin` turns into instructions.

## Race controls and first WRAM variables (driver-2, 2026-11-13)

Found by holding buttons from save states and diffing WRAM dumps (`--wram-dump`,
128 KB per frame) with numpy (project `.venv`, ignored):
- Controls (default config): **X accelerates**, **A brakes**, D-pad Left/Right steers.
  The countdown ends with "GO!" ~600 frames after the race screen appears.
- **`$7E:020F` (16-bit): player speed.** 0 at rest, ~10,475 after 3 s of throttle,
  falls when coasting (the speedometer shows ~183 mph around 11,700; roughly
  mph ≈ value / 64). Mirrored at `$7E:FACA`.
- **`$7E:0202` (signed 16-bit): player lateral position**, +64 on the starting grid;
  Left lowers it, Right raises it, curves push it. Mirrored at `$7E:0206` and in the
  `$7E:34EC..` table (one per car?).
- Not found yet: distance along the track (no monotonic 16-bit counter; probably
  segment index + offset), lap counter, race position.

These names are working hypotheses for the driver; they become labels once the code
that reads/writes them is understood.

**Autopilot** (`bench --autopilot STEP`, 2026-11-13): in a race (detected by DSP-4
writes in the last frame) it holds X and steers L/R to keep `$0202` within ±12; outside
races it loops "Down x6, A" (passes the league table and track intro, leaves the shop
through EXIT). `--record FILE` saves the buttons as a movie. From `race1.txt` + autopilot
at step 8,000 it drove 60,000 frames through several tracks (desert, night, plains;
"qualified in 6th position, 8888 credits"); saved as `movies/autopilot_60k.txt`, whose
replay without the autopilot gives the identical DSP-4 traffic (70,943,352 writes).
Coverage with it: **55,874 code bytes, 381,461 data bytes, 41.71% classified**;
the generated asm grows to 23,921 instructions / 1,489 labels and still rebuilds the
ROM byte for byte. The shop is only ever exited, never used (upgrades not covered).

## C state model and the differential test (difftest-1, 2026-11-13)

Technical decisions (advisor-reviewed; not the owner's call):
- **State:** one flat `uint8_t g_ram[0x20000]` at the original WRAM offsets plus
  `const uint8_t* g_rom`, touched only through `rd8/rd16/rd32/wr8/wr16/wr32`
  (explicit little-endian; `src/tg3000.h`). No struct overlays (68000 safety).
- **Signatures** carry only what callers use (`uint16_t rng_next(uint16_t state, bool* carry)`).
  The register-to-argument mapping lives in `tools/bench/routines.c` adapters, never
  in `src/`. `src/` includes no bench/LakeSnes header and is compiled with
  `-std=c99 -pedantic -Wextra`.
- **Names:** `map/symbols.txt` is the one list of names; `tools/listing.py` uses it for
  labels and `jsr/jsl` operands (e.g. `jsr.w rng_next`), so asm and C agree.
- **Harness** (`bench --difftest map/verified.txt --fuzz N`): at a registered entry
  the bench copies WRAM and registers (no save states — they proved lossy once), lets
  the original run to its return, then runs the C from the copy and compares WRAM
  (minus the stack bytes pushed below the entry SP), A/X/Y, N/Z/C/V, D and DB.
  An interrupt inside the routine discards the sample (patch counts dispatches); any
  CPU access outside WRAM/ROM fails it (hardware not modelled yet). The entry state
  (D, DB, widths) is recorded and a routine fails if it breaks the C's assumptions.
  Fuzz runs the original on the interpreter from the entry with random inputs
  (declared direct-page bytes and registers), interrupts off, so all branches run.
  **Verified** = no failure, ≥1 compared sample, and every instruction start of the
  routine's extent executed in a compared sample.
- **Results** go to `map/verified.txt` (entry, source file, FNV-1a of the source,
  counts, status), written by the bench only. `progress.py` counts verified rows whose
  source is unchanged, and `--check` (CI) fails if a verified source changed since.
- **Negative test:** `--difftest-negative` swaps in a C with one XOR constant off; it
  must fail (it does: first difference at WRAM `$0110`, original `45`, C `44`).

First routine: **`rng_next` ($80:87E9), `src/rng.c`** — a 32-bit LFSR, shifted left,
XORed with `$1D872B45` when the top bit was clear (the reverse of the usual Galois
form); the bit shifted out is left in the carry. Live: 4,000/4,000 compared samples
(of 14,644 entries in `race1.txt`), fuzz 500/500, paths 16/16. Every entry seen had
**D = $0100**, so the state is at `$0110-$0113` (the long wrapper `$80:87DF` sets
D = $0100 itself). Carry: no caller was seen to read it right away, but `lda/sta/xba/
sep` keep it, so it stays part of the contract. The identical copy at `$8A:988C`
(`rng_next_8a`, same C) runs only later in `autopilot_60k.txt`: 2,070/2,070 live,
fuzz 500/500, paths 16/16, entered with D = $0100 and DB = $81 or $8A.

## More leaf routines (port-leaves, 2026-11-13)

- **VRAM transfer queue** (`src/vram_queue.c`): a ring of 32 transfers in four word
  tables — size `$1900`, source `$1940`, bank `$1980`, VRAM word address `$19C0` —
  with head `$0028` and tail `$002A` (byte offsets, even, `$00-$3E`). The NMI part at
  `$80:88B9` (`vram_queue_drain`) sends them with DMA channel 0 (`$1801` → `$2118`),
  splitting transfers to respect a per-frame byte budget in `$2C`. **`vram_queue_push`
  ($80:83FC)**: A = size, Y = dest, X = source, D+$14 = bank word; no full-queue
  check. Verified: 4 live (it runs rarely), 500 fuzz, paths 15/15.
- **`take_slot` ($8A:A25D)** (`src/util.c`): returns the word at D+$8C and adds 2;
  callers fill object tables at `$0500/$0600/$0800,y`. 2,451 live, 500 fuzz.
- **`lsr_n` ($8A:B15E)** (`src/util.c`): A >>= Y one bit at a time; the last bit out is
  left in the carry, and with Y = 0 the carry is set (from `CPY #0`). 1 live, 500 fuzz
  (counts 0-19), paths 6/6.
- Harness now: fuzz is a per-routine callback that makes inputs valid (e.g. the queue
  head even and < $40), and a `lowWramDb` contract for routines that reach low WRAM
  through DB-relative absolute addressing (DB must be a bank that mirrors it).
- Deferred on purpose: `$80:B537` (a second queue push with base D+$8A whose masking only
  makes sense for base 0 — needs its callers understood first) and `$80:B7BB` (leaves a
  comparison result in the carry; port it together with a caller that shows whether
  the carry matters). `$80:8373` waits for the frame counter `$0024`, so every live
  sample is interrupted by design — it needs an interrupt-aware test.

## Hardware rules and flag liveness (difftest-hw part 1, 2026-11-13)

Advisor-reviewed; they follow from "0% emulator", not new owner decisions.
- **CPU multiply/divide unit** (`$4202-$4206` operands, `$4214-$4217` results): no
  interface — the C computes natively (`*`, `/`). The harness accepts those accesses
  inside a sample, but a result read is only allowed after an operand write in the
  same sample. (Division by zero must still be checked against the bench when the first
  divide routine is ported.) `$211B/$211C` is also the Mode 7 matrix: it gets the same
  rule only once the movies show Mode 7 is never selected.
- **Uploads to video memory** (immediate DMA `$80:83AD` VRAM, `$80:83CB` CGRAM, the
  queue drain): will be **semantic calls** in `src/` ("upload N bytes from bank:addr to
  VRAM word address / CGRAM index") — never register writes, which a port could only
  consume with a PPU register model. Verified by comparing effect logs (next unit).
  A routine whose hardware effect cannot be named yet stays asm.
- **Flag liveness:** after each live sample returns, the bench keeps watching the
  original (per-opcode read/write masks from `tools/gen_opflags.py` →
  `tools/bench/opflags.h`, REP/SEP by operand, interrupt handlers skipped) until each of
  N/V/Z/C is read (live) or overwritten (dead), within 400 instructions (unresolved =
  live). Flags are always compared, but a mismatch fails the routine only if the flag
  was live at least once, or if fewer than 100 returns were watched. `verified.txt`
  records the live flags. Result: `rng_next`'s carry is dead over 4,000 returns, so its
  `carry` parameter was removed.
- **Decoder** tracks REP/SEP when listing a routine's instruction starts (a `SEP #$20`
  followed by `LDA #imm` was mis-decoded before). **Extent guard:** a sample fails if,
  at the routine's own stack depth, the PC leaves the declared extent.
- **Stale rows:** the hash in `verified.txt` now covers the C source *and*
  `tools/bench/routines.c` and `tools/bench/difftest.c`; `tools/verify_all.sh` rewrites
  the file from the full autopilot movie (~10 min). Fuzz now runs after the save state
  and coverage are written, with coverage off.
- **`inverse_scale` ($80:F709)** (`src/scale.c`): D+$38 = 42 × q, q = 8-bit quotient of a
  restoring division of $200·2⁷ by D+$7A (= ⌊65536/divisor⌋ for divisors above $100),
  divisor $100 short-cuts to $2A00; the ×42 uses the hardware multiplier. Race movie:
  535/535 live, fuzz 500/500 (divisor 0, $100, small and random), paths 29/29; its C
  and V differ from the C's but are dead in all 535 watched returns. Entered with
  D = $0100, so the variables are `$017A`, `$0114`, `$0138`.

## Coverage without a player (2026-11-13)

The server has no display and the owner will not play by hand. Coverage comes from:
1. **Driver:** a scripted bot that reads WRAM state (game mode, menu cursor, car
   position/speed) to navigate menus and drive races, recorded as movies.
2. **Explorer:** from save states along those movies, try input variations and keep
   the ones that execute new code (coverage-guided), so rare paths get reached.
Optional later: a movie the owner records on a desktop emulator, converted to our
format (risk: a different DSP-4 implementation could desync it).

## Legal posture

Never committed: the ROM (`*.sfc`), DSP firmware, extracted assets, third-party
code fetched at build time (e.g. snes-dsp4). The build asks for the user's own ROM
and checks its hash.

## Publishing (requested 2026-11-13)

The owner wants a **public** GitHub repository, polished, accepting PRs
(account `MaChInEgUn3`, `gh` already logged in). The owner delegated the details to
the advisor, who settled them on 2026-11-13:

- **Name:** `MaChInEgUn3/tg3000-decomp`.
- **Commits:** local commits start now, one per finished loop unit. Author identity
  is the GitHub noreply address (`28672726+MaChInEgUn3@users.noreply.github.com`),
  never the owner's real email.
- **Publishing gate:** bench-3 and bench-4 pass, and a fresh clone builds with
  `fetch.sh` + `make`. Then create the repo **private**, push, list the remote tree
  to confirm no ROM / `ext/` / `build/` / frame dumps, and only then make it public.
- **Repo contents for launch:** README in English + `README.pt-br.md`, CONTRIBUTING,
  PR and issue templates, CI that runs `fetch.sh` + `make` without the ROM,
  a generated progress block (as in `memories-decomp`), `CREDITS.md`.
- Never pushed: ROM, `ext/`, firmware, `build/`, frame dumps (game assets).
  `CLAUDE.md` is local, not versioned.
- **Done 2026-11-13:** fresh clone passed the CI steps locally and ran the ROM;
  created private, pushed `74b5438`, remote tree checked (24 files, no assets, all
  commits under the noreply address), GitHub Actions green, then made **public**
  at https://github.com/MaChInEgUn3/tg3000-decomp with issues on and topics set.
  From now on each finished unit is committed and pushed (CI must stay green).

## Licence (decided 2026-11-13)

**GPL-3.0-or-later**, chosen by the owner. This allows using
`StackBlender/snes-dsp4` (GPL-3.0-or-later) both in the bench and as a source for the
native DSP-4 C, with credit. The LICENSE file goes in with the GitHub setup.

## DSP-4 on the bench (2026-11-13)

- The game talks to the chip at **`$30:8000`/`$30:8001`** (data register, word = low
  byte then high byte): 560 static long `LDA`/`STA` to `$30:8000` in the ROM. No
  static access to the status half (`$C000-$FFFF`) was found; the mapper still
  routes it (banks `$30-$3F`/`$B0-$BF`, `$8000-$BFFF` data, `$C000-$FFFF` status).
- Boot handshake at `$80:8080` (frame 50): `PEA $3030; PLB; PLB`, then read the data
  register until it has returned `$FFFF` 256 times in a row, then `JSR $89DB`, then
  `BRA *` (idle loop; the game runs from NMI). The bench passes it.
- LakeSnes patch: `tools/bench/patches/lakesnes-dsp4.patch` (cart type 5, selected by
  the header: LoROM + coprocessor 0 + title `TOP GEAR 3000`). `tools/bench/fetch.sh`
  re-creates `ext/` at pinned commits and applies it.
- Known nit: LakeSnes allocates 1 KB cart RAM from the header even though the header
  says no SRAM; harmless unless the game touches banks `$70-$7D`.

## Target ROM

- `TG3000.sfc` — Top Gear 3000 (USA), 1,048,576 bytes, no copier header.
- Header: title `TOP GEAR 3000`, map `$30` (LoROM, FastROM), type `$03`
  (ROM + DSP), region `$01` (USA), version 0, checksum `$5327` / complement `$ACD8`;
  computed checksum matches.
- CRC32 `a20be998`, MD5 `23eaa07e3f3315fa43f4b4d94ec97a7b`,
  SHA-1 `058aadf1ee719cb9e0c7333e665797f990a6381d`.
- No-Intro hash not yet cross-checked (DAT-o-MATIC).

## Related prior art

- `pedro-walter/top-gear-snes-re` — Top Gear 1 (same studio) RE notes.
- `StackBlender/snes-dsp4` — clean-room DSP-4 (GPL), 96–100% exact on recordings.
- `snesrev/zelda3` — model for a readable-C SNES port (it keeps a PPU; we won't).
