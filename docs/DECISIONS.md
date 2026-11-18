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

## Platform interface: video uploads (difftest-upload, 2026-11-13)

- **`src/platform.h`** is the boundary between the game logic and a port. First two
  services: `video_upload_vram(vram_word, src, size)` and
  `video_upload_cgram(index, src, size)`; `src` is a 24-bit SNES bus address (LoROM ROM
  or WRAM), `size` 1-65536. Ports implement them natively (copy bytes into their VRAM /
  palette); the bench implements them in `tools/bench/platform_bench.c` by recording
  an effect. `src/` never writes hardware registers.
- **Effect comparison:** on the original side, every write to MDMAEN (`$420B`) becomes
  one effect per channel, read from the channel registers and the PPU's address state
  at that moment: *VRAM upload* (B-bus `$18`, mode 1, A→B, no fixed/decrement, VMAIN
  increment 1 word after the high byte, no remap) at the current VRAM address; *CGRAM
  upload* (B-bus `$22`, mode 0/2) at the current CGRAM index; anything else is a raw
  DMA effect that no C can match yet. The setup writes (`$4300-$437F`, `$2116/7`, `$2121`)
  have no effect of their own. The C's platform calls must produce the same list.
  VMAIN (`$2115`) is an implicit input: an upload under another increment mode does
  not count as a VRAM upload.
- `upload_vram_now` ($80:83AD, JSL) and `upload_cgram_now` ($80:83CB, JSL) in
  `src/video_dma.c`: full movie 362/362 and 17/17 live (4 interrupted samples discarded), fuzz 500 each (ROM and WRAM
  sources, sizes 1-2 KB), all paths. A negative variant uploading one byte too many
  fails with "original VRAM upload $3000 <- $7F0000 x2048, C ... x2049". The selftest
  now requires *every* negative variant to fail.
- The verification hash also covers `tools/bench/platform_bench.c`.

## C calling C (port-batch-2, 2026-11-14)

- Routines whose callees are already verified are portable: the sample includes the
  original callee, and the C calls the C callee. A scan (`asm` + `map/symbols.txt` +
  `map/verified.txt`) finds them.
- `src/tg3000.h` gains `DP` (`$0100`: every entry seen so far ran with it; routines that
  assume it declare `fixedD` and the bench fails them otherwise), a data-bank contract
  (`db`), and `rom8/rom16` to read ROM tables by bus address.
- New: `rng_next_long` ($80:87DF), `star_spawn` ($80:FA3F; copy $8A:9863 registered,
  not reached yet), `star_random_y` ($80:E166), `upload_7f_to_3000` ($8A:BA7D),
  `upload_tiles_3300` ($85:8245) and `upload_tiles_3700` ($85:8260). The `$0F00` table of
  (x, y, depth) words filled with random positions is most likely the intro starfield.
- **The flag check paid off twice:** callers of `rng_next_long` *do* read the carry
  (the bit shifted out), so `rng_next` takes `bool* carry` again (NULL when unused);
  `star_random_y` exits with C = 0 by construction (the rejection loop ends on
  `CMP #$DC00` with the carry clear).
- **The data-bank check paid off:** the tile uploads in bank `$85` read their offset
  table through DB = `$83`, not their own bank (`$83:84B1`). The `$8A:8D81/8DA0`
  variants are left in asm until a movie runs them and shows their DB.
- Adapters derive mechanical flag results (N/Z from a value, C/V of a final ADC whose
  sum the upload effect already checks); the value logic stays in `src/`.

## Batch 3 and naming (port-batch-3, 2026-11-14)

- `tools/portable.py` lists routines whose calls all go to verified routines and whose
  only hardware is the multiply unit (the scan is a guide; the bench decides).
- **Naming:** a routine whose purpose is not known yet is `func_<address>` (as in the
  owner's FM decomp) and lives in `src/misc.c` until understood; a guess about purpose
  goes in a comment marked as such, never in the name.
- New: `upload_tiles_7700/7b00` ($85:827B/8296, table again through DB = $83),
  `star_layers_init` ($80:E0D6: the `$0F00` table in a second, 4-word layout — speed, y,
  tile, x — 2/10/24/28 stars at speeds $100/$80/$40/$20), `func_81F251` (a lone RTS that
  19 places call), `clear_dp_scratch` ($8A:8001, zeroes `$0114-$0193`), `func_81F1FC`
  (turns the 20-word list at `$1E6A` into next pointers at `$7E:FACD`; 20 = the number
  of cars, so probably the race order), `func_8AA216` and `func_8A92CF` (fill object /
  sprite tables at `$0500-$0800` and `$05C0-$08C0`).
- The flag watcher follows one return at a time, so routines called right after
  another sample may have few watched returns; those keep all flags in the contract
  (e.g. `func_81F1FC`: its C = 1 from the loop's final `CPX #$26` is now modelled).

## What blocks the rest: census and roots (strategy-1, 2026-11-14)

- Question: after 23 routines the easy leaves ran out; is the next unlock the PPU
  registers, the DSP-4 or more coverage? A regex count over the listing said: of 344
  unported called routines, 103 are blocked *only* by unported callees, 8 only by PPU
  registers, 4 only by the DSP-4. So the call graph decides, and it was measured.
- **`bench --census map/census.txt`** (`tools/bench/census.c`): a shadow call stack over
  the movie records, for every routine entered with JSR/JSL and both interrupt
  handlers, its entries, how many had an interrupt inside, the hardware classes its own
  body touches (math, ppu, dma, hdma, apu, io, dsp4, other), its D/DB/widths, its
  callees and its indirect-jump targets. 60k movie: 432 routines + NMI (`$00:0000`)
  and IRQ (`$00:0004`) trampolines. D is **not** always `$0100`: the frame wait runs
  with `$0200` too, and the IRQ handler with `$0B00` — per-routine contracts matter.
- **`tools/roots.py`** ranks *roots* (unported routines with no unported callee) by how
  many unported routines depend on them. 407 unported, 210 roots. Top: `$82:FB70` (44, a
  jump-table dispatcher with 40 targets), `$80:F745` (28, no hardware, never
  interrupted), `$80:81CC` (20, PPU/DMA/HDMA), `$8A:B909` (17, 8,078 entries),
  `$8A:B700` (16), `$8A:BA90` (15, always interrupted — a wait), `$8A:C2A2/C2CC` (14).
  The frame wait `$80:8373` blocks only 8.
- Plan: port the hardware-free roots by weight; then the frame waits (a platform
  "wait for next frame" with the interrupt's WRAM writes replayed); then the
  dispatcher; the PPU interface after logging which writes come from main code.
- Harness fixes from the review: the entry contract (D, DB, widths) is checked on
  **every** entry, including entries nested inside another routine's sample (before,
  only the sampled third); DMA/PPU address setup that no `$420B` kick consumes now
  fails the sample instead of being silently absorbed.

## Hardware-free roots (roots-1, 2026-11-14)

- `func_80F745` ($80:F745, `src/line_tables.c`, 28 dependents): builds two tables in the
  HDMA format (a line-count byte, then a word) at `$0D00` and `$0D40` + the back-buffer
  offset `$0134`, from `inverse_scale`'s `$0138` and `$013A/$013C/$0132`, then swaps the
  back/front offsets (`$0134/$0136`). Probably the sky/road split; the meaning waits for
  the HDMA side. Its 3-byte entries are written as two overlapping word stores, kept in
  that order. One block (`$80:F898-$80:F8B8`, taken when `$0132`'s high byte is non-zero)
  never ran in any movie, so it was decoded from the ROM and is covered by fuzz:
  paths 209/209, 535 live samples.
- `tilemap_offset` ($8A:B909): X = row `$0138` × 64 + column × 2, doubling `$0136` in
  place — a 32-column map of words. 2,660 live samples.
- `func_8AB700` ($8A:B700): the word at `$0D04 + ($013C − 10)` becomes 4 or 0 by the
  parity of `$015A`. **Game bug:** with `$013C < 10` it returns with Y still pushed, so
  RTS jumps through Y. No movie reaches it; the C does nothing there and fuzz keeps
  `$013C >= 10`.

## Text and small steppers (roots-2, 2026-11-14)

- **Text:** `text_put_char` ($8A:B83C, `src/text.c`, 8,078 calls) draws an ASCII
  character (8-bit A) at the cursor (`$013A` column, `$013C` row) into the word map at
  `$7F:0000` — 32 columns, each character two tiles high (bottom at +`$40`) — and
  advances the cursor. Main font: glyph pairs at `$81:CCD9` (letters → 0-25, digits →
  `$1A-$23` with attribute `$3C00` instead of `$3800`, space → `$33`, punctuation looked up
  in a 15-byte list at `$8A:B8FA`: `&\x9c[.+={}?!]<>/_`). When `$0D7C` = 0 an alternate
  font (tops `$8A:C9C4`, bottoms `$8A:CA3A`) is used; that path never ran in a movie
  and sits right before the punctuation list, so the "table" there was really code
  (decoded from ROM, covered by fuzz: paths 88/88). `char_to_tile_index` ($8A:C2A2) is a
  second, simpler ASCII → glyph map. `$8A:C2CC` is a byte-identical copy of
  `tilemap_offset`.
- **`step_toward`** ($8A:A264/A27D/A296, `src/misc.c`): move a word one step toward a
  target, up to Y steps, stopping when equal (pairs `$0D80→$0D88`, `$0D70→$0D78`,
  `$0D68→$0D90`, indexed by X). One of them had 88 watched returns (< 100), so all its
  flags stay in the contract; C/V are derived from the C's reported last difference.

## Community TAS movies (2026-11-14, suggested by the owner)

- TASVideos lists 8 TAS files for this game. Seven (Technickle, "All Tracks — System N")
  are for the Japanese release. **"Light Spike" by Jmosx36** (user file
  34264942243851572, BizHawk 1.9.4, bsnes core, real DSP-4 firmware, 450,994 frames
  ≈ 2 h) is for **our exact ROM** (SHA-1 `058AADF1...`).
- `tools/bk2_to_movie.py` converts a `.bk2` (TASVideos may serve it gzip-wrapped) into a
  bench movie (player 1). TAS files are **not committed** (their authors' work);
  download them from TASVideos.
- On the bench the TAS **desyncs during the first race** (by frame ~7,600 the car is
  20th of 20 and stopped). Frame offset 0 is right (−3 already desyncs in the name
  entry menu). Likely causes: the clean-room DSP-4 (96-100% exact) and LakeSnes timing
  differing from bsnes + the real chip; a TAS amplifies any difference. Learned anyway:
  the intro can be skipped with A (race at frame ~1,200 instead of ~7,900).
- ~~Desynced input still runs real game code~~ — **withdrawn** (see "Coverage sources"
  below): a desynced run can derail the game into data, and one did (a lone `BRK` at
  `$80:A3B2`, inside a real `LDA #$0100` at `$80:A3B1`). Its coverage was dropped.
- Exact sync would need a bench with a low-level DSP-4 (the chip's firmware, which
  this project does not have) and bsnes-level timing — left as an owner decision.

## Reference bench and the DSP-4 firmware (2026-11-14, owner's decisions)

- **Owner:** downloading emulators for the bench is fine; the owner supplied the DSP-4
  firmware **file** (its provenance is the owner's). It is kept at `ext/firmware/dsp4.rom` (ignored by git:
  `/ext/` and `*.rom`), **never downloaded by the project and never committed**. It
  checks out: 8,192 bytes, SHA-1 `af6478ae…8c72` (what the TAS's `.bk2` header requires)
  and SHA-256 `752d03b2…31da` (what bsnes's cartridge markup expects). Ports never use it.
- **`tools/refbench/`**: the TAS's own emulator, headless — bsnes v087 as shipped in
  BizHawk 1.9.4 (`libsnes`, Compatibility profile, per the `.bk2`'s `Header.txt` and
  `SyncSettings.json`), sparse-fetched by `tools/refbench/fetch.sh` with
  `patches/bsnes087-refbench.patch` (builds on Linux; BizHawk's Windows IPC wrapper left
  out; hooks to swap in the clean-room DSP-4; a DSP-4 write counter). `refbench ROM
  "Input Log.txt"` replays the `.bk2`'s log frame by frame (`--firmware` for the real
  chip, `--hashes`, `--dump`, `--offset`, `--p2-pad`); the share of frames with DSP-4
  traffic measures time spent racing.
- **Light Spike is the whole game** (450,995 log lines, ≈ 2 h 05 min).
- **The TAS syncs to the end** with the real firmware **and** `--bizhawk`, which replays
  what BizHawk 1.9.4's `LibsnesCore` does while recording a movie (read from its source at
  the 1.9.4 tag): a second `power()` after loading the cartridge, and — deterministic
  mode — a save state at load and **before every frame**. In bsnes, `snes_serialize()`
  first runs the system to a save point (`system.runtosave()`), which moves the threads'
  synchronisation; the movie was recorded with that, so it has to be replayed with it.
  Full run: 450,994 frames, **48 races**, 86.7% of frames with DSP-4 traffic; the last
  race ends at frame 450,249 in 1st place (the movie ends 745 frames later).
- First attempt, without `--bizhawk` (recorded here because it was committed as a
  conclusion in b411ff4, wrongly): the car drove slowly in the first race and did not
  finish; port 2, offsets −2…+2 changed nothing. That conclusion ("BizHawk's frontend",
  unverified) is superseded by the above; the instrument had not been validated.
- The clean-room DSP-4 run inside bsnes (hooks on DR/SR) diverges from the firmware run
  at frame 1,378 and later crashes; why is **open** (the hooks bypass the uPD7725
  core's synchronisation — unverified). To measure the clean-room chip, the plan is to
  log the firmware run's DSP-4 command stream and replay it into the clean-room chip
  offline (no timing involved).
- **Value already in hand:** with the firmware, bsnes is a reference for the DSP-4's
  exact output, which is what the native DSP-4 C will be checked against later.

## DSP-4 as native C (2026-11-15)

- `src/dsp4.c` holds the chip's commands as plain functions; ports call them instead of
  talking to `$30:8000`. First: **command `$0000` = `dsp4_multiply`**, the signed
  16 × 16 → 32-bit product (low word read first). The game uses it as its multiplier:
  43 call sites (`LDA #$0000 / STA $30:8000` followed by two operands).
- **Oracle:** `tools/refbench --dsp4-mul FILE` captures every command `$0000` the game
  sends on the reference bench (the real firmware), with its operands and the result
  bytes the chip returns, and checks them against `dsp4_multiply` as the TAS runs
  (a CPU bus hook carrying values was added to the bsnes patch; the run's WRAM hashes are
  unchanged by it). The whole TAS (450,994 frames): **15,431,275 multiplies, all equal**
  to `dsp4_multiply`, and the run's WRAM identical frame by frame to the earlier sync run.
- **The harness accepts whole multiplies:** a routine's traffic to `$30-$3F:8000+` passes
  when it is made of complete command-`$0000` transactions (the word `$0000`, two operands,
  four result bytes read back; status reads free) — anything else, or a multiply left
  unfinished, fails the sample as hardware the C cannot reproduce. Each fuzz case also
  resets the bench's DSP-4 so no earlier case leaves it mid-command.
- First routine using it: `func_83FD8A` ($83:FD8A, the object hit test with the position
  in A; its hit tail at `$83:FE16` adds `(D+$0F / 2) × $1F30 / 32768` to D+`$5A` with
  `dsp4_multiply`, saturating). Two of its byte runs never ran in a movie and were decoded
  from ROM (fuzz-only): `$83:FDB8` (the negative-side limit test, as in `func_83FC05`)
  and `$83:FE50` (`LDA #$FFFF`, the saturation). In the 60k movie `$1F31` always has
  bit 15 set, so the multiply itself is reached only by the fuzz (and checked against the
  chip by the refbench oracle). The first gated run showed the harness counting none of
  its result reads: reads of `$30:8000` were filtered out as ROM reads before the DSP-4
  check; the DSP-4 test now comes first. `func_83FDCD` ($83:FDCD) always answers
  "no hit": an unconditional BRA skips its 33-byte test (dead code, a hole in its extent).
- **Skipping product bytes.** After the operands, each data access takes one of the
  product's 4 bytes: a read returns it, a *write skips it* (the value written is ignored).
  The game uses this for fixed-point work: `$82:AB3F` writes a word, then reads only the
  high word (`a × b >> 16`). The first oracle stopped its capture at such a write, so those
  cases went unchecked; it now treats every product byte as read or skipped. Whole TAS on
  the real chip: **15,882,250 multiplies, 739,386 of them with skipped bytes, all equal**
  to `dsp4_multiply`. The harness uses the same rule (a write during the product is a skip).
- `func_82EA49` ($82:EA49, 37,128 calls: row × 10 on the DSP-4, then a table in bank
  `$84` gives an offset into bank `$85`; the first add's carry feeds the second) and
  `func_82AB3F` ($82:AB3F: two linear interpolations at the fraction `$FAB5,x` / 256, each
  the high word of a product). The fuzz for `func_82EA49` keeps both table offsets in ROM:
  random ones reached bank `$84`'s I/O window (`$2000`–`$7FFF`), which the game never does.
- `func_82A9B7` ($82:A9B7, 18,525 calls): two targets from `func_82AB3F`, each scaled
  by `$0293` on the DSP-4 (one reads the whole product, the other skips its low word with a
  write), then `$028B`/`$028D` approach them by at most `$1E9E × 4` per call (a camera
  following something, probably). A static scan of the DSP-4 roots found the other
  multiply-only ones: `$82:AC52`, `$82:ADC2`, `$82:A672`; the rest (`$81:89F5`, `$81:8527`,
  `$82:D267`, `$81:F887` …) use the road commands (`$0A` and others), not ported yet.
- `func_82AC52` ($82:AC52, 18,521 calls): `$7E:4E2D` from the bits 8-23 of `$0213 × $300`
  (DSP-4) and, by `$0244`'s bits, `$7E:4E35` and a new `$7E:4E2D` from the distance to
  `$0224/$0226` (a 32-bit compare, then the CPU's 16/8 divider) — a distance-driven value
  (fog or horizon, probably). Its one unexecuted byte run (`$82:ACCB`, `LDA #$0000`) is
  fuzz-only.

## DSP-4 command catalogue (2026-11-15)

- `tools/refbench --dsp4-catalogue FILE` runs StackBlender's clean-room chip as a *shadow*
  of the real firmware, fed the same data writes; its busy flag gives command boundaries
  (valid while it agrees with the chip), and every byte the firmware returns is compared
  with the shadow's. Per command: count, input/output bytes, which instructions write and
  read, and how many runs had a different byte. (Measuring tool only: the shadow's private
  fields are read by including its header with `private` defined as `public`.)
- **The game uses 10 commands.** First 30,000 frames of the TAS (before any shadow drift):

  | cmd | runs | runs with a different byte | in → out bytes | written / read by |
  |---|---|---|---|---|
  | `$00` multiply | 1,278,666 | 28 | 4 → 4 | 43 sites (`$82:A590`…) |
  | `$0A` (car physics) | 163,630 | **0** | 6 → 7 | `$81:F90E`…, `$83:E8B2`… |
  | `$03` (sprites start) | 9,600 | **0** | 10 → 5 | `$82:CBE1`, `$82:D8FA`… |
  | `$0B` (one sprite) | 9,600 | **0** | 6 → 5 | `$82:D8FA`… |
  | `$11` | 3,392 | **0** | 8 → 1 | `$81:E72B`… |
  | `$01` road, one track | 9,311 | 3 | ~258 → 468-946 | `$85:B53B`… / `$85:B641`… |
  | `$07` road, rows given | 9,596 | 278 | ~354 → 360-834 | `$85:B95F`… / `$85:BA47`… |
  | `$06` high sprite table | 9,306 | 411 | 0 → 31 | `$82:AE1C` / `$82:AE21`… |
  | `$09` sprites | 9,600 | 2,454 | up to 4,754 → 2,864 | `$82:CC8F`… / `$82:DDDD`… |
  | `$08` road, two tracks | 9,595 | **6,907** | ~600 → up to 1,003 | `$81:861A`… / `$81:878D`, `$81:89FA`… |

  The 28 multiply runs that differ are drift (our `dsp4_multiply` matches the firmware in
  all 15.9M), i.e. the shadow's boundaries slipped after a road command differed.
- Whole TAS: from frame ~57,800 even `$0A`/`$03`/`$0B`/`$11` start to differ, all at once
  and with merged input runs of 100,000+ bytes — the shadow has drifted out of step with the
  chip, so the whole-TAS counts measure the drift, not the commands. The first difference
  of all is at frame 1,385 (command `$01`, read at `$85:B73B`).
- **Decided by the owner (2026-11-15): we write our own.** The road commands go into
  `src/dsp4.c` as our C, measured command by command against the firmware oracle on the
  TAS (as the multiply was); snes-dsp4 is not a source for `src/` and stays only as the
  refbench shadow in `ext/`. Reasons: it is far from exact on the commands that matter
  most (`$08`, `$09` above), and the repo's C is uniformly ours, readable and verified
  per routine. The firmware stays an
  oracle only (outputs compared; its program is never read or disassembled), and stream
  routines (`$81:89F5` drains what `$08` started) need the chip's state handed to a sample
  (a snapshot at entry, or a replay of the original's words).
- **Sources, decided by the owner (2026-11-15):** if measurement against the firmware is
  not enough (the road commands `$08`/`$09`), other sources may inform the C ("pode fazer
  o que for necessário"), provided their licence is compatible with GPL-3.0-or-later
  (checked before reading them) and they are credited. The firmware itself stays an
  oracle (outputs compared, its program not read).
- `decompress_to_vram` ($80:8381, long entry `$80:837D`): unpack a stream to `$7F:0000`
  and DMA the result to VRAM — in C, `decompress_near` then the `video_upload_vram`
  platform call, compared with the original's DMA as before. Its destination bank is
  `A | $7F00` (always `$7F` in the game; the fuzz keeps A's high byte below `$80`). Fuzz
  streams for a destination at 0 have no back references and open with a literal run (the
  real ones never reach before the buffer); the decompressor's own fuzz still covers those.
  Only 3 live samples (most of its 87 calls are interrupted during the unpacking).
- **A third video call:** `video_fill_vram(vram_word, value, size)` — fill VRAM with one
  byte in both halves of every word. The original does it with a fixed-source DMA (mode 1 to
  `$2118`, `$4300` = `$09`); the harness now records such a DMA as a `VRAM fill` effect
  (its value read from the source) and compares it with the C's call. First user:
  `clear_vram` ($8A:8DEC, the whole 64 KB from the byte at `$8A:8E19`). Its live calls are
  all interrupted (a 64 KB DMA outlasts a frame), so the fuzz (interrupts off) verifies it.
- `func_80F3C4` / `func_80F3F4` ($80:F3C4, $80:F3F4): upload one half of a picture built
  at `$7E:2000` (1,184 bytes each, to VRAM `$3100` / `$3350`), step `$013E` and rebuild
  the line tables. With one call each in the movie, all flags count as live, and V had
  never been modelled after `func_80F745` (dead for its own callers): the adapters now
  derive it — from `func_80F745`'s last SBC/ADC (one of them in a run no movie reaches,
  decoded from ROM), or, on its early exit (`$013A` = 0), from `inverse_scale`'s last
  SBC, or as on entry.
- `upload_slice` ($80:F4AD … $80:F561, 16 entries 12 bytes apart): slice k of a picture
  built from `$7E:3040` (1,556 bytes from `$7E:3040 + k × $614` to VRAM `k × $30A`), then
  the same tail as `func_80F3C4` (at `$80:F56D`). One C function with the slice as its
  argument; in the harness each entry's extent runs to the shared tail with the following
  entries as a hole.

## PPU registers (ppu-1, 2026-11-15)

- **Semantic setters, one per register field the console decodes** (`src/platform.h`):
  `video_set_m7a/m7d` (Mode 7 matrix A/D, `$211B/$211E`) and `video_set_bg1_hofs/vofs`
  (`$210D/$210E`). On the console one write to `$210D/$210E` sets both the BG1 scroll
  (10 bits) and the Mode 7 offset (13 bits, signed), so a port gets both from one call.
  Calls are added only when a verified routine needs them.
- **Compared by end state, decoded by the bench's PPU**: the harness notes which fields
  the original wrote inside the sample (`$210D/$210E`, `$211B-$2120`) and reads their
  final values from LakeSnes's PPU at the return; the C's calls must set exactly the same
  fields to the same values (masked to the bits the PPU keeps). The write-twice latches
  are not re-implemented: a sample whose last write to a field completed a latch byte
  written *before* the sample fails ("completed a latch byte written before the
  routine"), because its result depends on state the C does not get.
- **Timing:** a port has no scanlines, and the setters take no line argument: the C
  cannot know the line, and the end-state comparison does not depend on it. The harness
  reports, per routine, the scanlines of the original's field writes, how many land while
  the picture is drawn (line < 225, screen on), the BG modes, the HDMA targets active at
  the time, and how many samples changed a field. A write's timing is invisible when the
  screen is blanked, the value does not change, or HDMA rewrites that field every line;
  otherwise the console shows one frame split at that line, which a port that applies it
  at its next frame does not — a fidelity note for the ports, not an interface change.
- `mode7_zoom_view` ($80:F38D, `src/scale.c`): Mode 7 with no rotation, A = D = the
  scale D+$7A, offset ($80 − x, $6F − y) for the centre bytes D+$3D/D+$3B. Race movie
  350/350 live, fuzz 500/500 (both sides of the offsets' borrow), paths 25/25. Negative
  variant with the offsets swapped fails ("PPU M7HOFS: original 1FC0, C 006F").
  Its writes are all in vblank (lines 247-250, 0 of 2,800 while drawing), in BG modes 0
  and 1, with HDMA on `$2110` only; 34 of 350 samples changed a field.
- `mode7_view` ($8A:8B0A, `src/scale.c`): the full matrix from D+$14..$1B, centre
  ($80, $40), BG2 horizontal scroll D+$3A, patches of the `$0D00`/`$0D20` line tables
  (3-byte entries, the shape of HDMA tables), `$0D2A`, and BG2's vertical scroll by bit 2
  of the scene's 8-byte entry at `$8A:9351`. The set-bit path is code at `$8A:8B9D`–
  `$8A:8BB5` that the map calls data (no recorded run takes it; the fuzz does, and fuzz
  runs do not feed coverage). Full movie 263/263 live (it is not reached in race 1), fuzz
  500/500, paths 73/73. Unlike `mode7_zoom_view` it writes while the picture is drawn
  (lines 31-240) — but with the screen blanked every time (0 of 4,208 writes while the
  picture is drawn; BG modes 1 and 2 at the time; HDMA on `$2121` only; 102 of 263
  samples changed a field), so "applies at the port's next frame" is exact for it too.
- **Mode 7 multiplier** (`$2134-$2136`, matrix A × high byte of B): a read is the
  routine's own work when it wrote A and B inside the sample from bytes written inside it;
  the C computes the product natively *and* sets A and B as the original left them, so
  the end-state comparison covers them (a single write to B leaves the latch, the
  previous byte, as B's low byte; the C sets that too).
- **Direct VRAM words** (`video_write_vram_word`): each `$2118`/`$2119` pair under VMAIN
  = `$80` is one effect (address, word), in order with the DMA effects; `$2116/7` set up
  for them no longer count as a DMA left unstarted. A word written while the screen is
  drawn fails the sample (the console drops it; no scanlines in the C). The fuzz now runs
  each case under forced blank, as the game does its VRAM work; live samples keep the
  game's own timing. Effects per sample: up to 64, and more than that fails the sample
  (it used to drop the rest silently).
- `draw_meter` ($80:DA5F, `src/hud.c`): a 10-tile bar at tilemap word `$00C2` whose length
  in eighths is value × 10 / 256 + 2 (D+$28; product on the PPU multiplier): full tiles
  `$0847`, one partial `$0840 + n`, empty `$0848`. Race movie 461/461 live (lines 240-249,
  in vblank), fuzz 500/500 (values that keep the bar inside its ten tiles: past 80
  eighths the original's empty fill runs round its 16-bit counter, about 65,000 words),
  paths 39/39. Negative variant with the partial tile an eighth short fails ("original
  VRAM word $C2 <- $000842, C ... $000841").
- **Mode at the time of the write:** neither routine runs in BG mode 7 when it writes
  (modes 0/1 and 1/2): the mode is set elsewhere (likely from the NMI), so the names say
  what the routines write, not the mode on screen at that moment.

## Hardware-free batch 4 (port-batch-4, 2026-11-15)

- `$80:F424` was the next PPU root, but it is entered once in the 60k movie and also sets
  HDMA table pointers (channels 1 and 3), which have no interface yet. `tools/portable.py`
  crossed with the census gave 22 hardware-free routines that the bench reaches and whose
  callees are verified; this batch takes six of them. HDMA waits until more routines
  need it.
- `oam_finish_0b00_near` ($80:89D1), `oam_finish_near` ($80:89D6): JSL + RTS wrappers;
  the adapters share the far ones' output model through a function argument.
- `func_80B537`: appends (A, X, Y, D+$14) to four word tables at `$1900/$1940/$19C0/$1980`
  and advances a 32-entry ring index (`D+$8C = ((i + 2) & $3F) + D+$8A`). The fuzz keeps
  the index below `$500`: past that the tables reach the fuzz's own stack at `$1F00`.
- `sprite_number_right` ($80:8698): `sprite_number`, then shifts its sprites so the number
  ends at x (like `sprite_text_centered`, without the halving).
- `func_80EB66`: places the sprite group at `$81:CBB8` from five direct-page words;
  `sprite_group`'s adapter is split into a table walk and an output model so both share it.
- `byte_buffer_space` ($8A:9A99): pushes a space, A kept.
- **Diagnostics fix:** the PPU write counters now skip writes after an interrupt in the
  sample (a handler's, in a sample that is discarded anyway); before, a sprite routine
  showed the NMI's scroll writes as its own.

## Hardware-free batch 5 (port-batch-5, 2026-11-15)

- The byte buffer at `$7F:7D00` (index `$0158`, byte count `$0160`) gets its writers:
  `byte_buffer_pad` ($8A:99B4: spaces until the count word equals A),
  `byte_buffer_end` ($8A:99D6: the end marker `$80`, count reset) and
  `byte_buffer_decimal` ($8A:9A33: A in decimal by repeated subtraction of the powers of
  ten at `$8A:9D83`, leading zeros skipped, "0" for zero).
- `func_80E4C5`/`func_80E4DB` (long): `$01FB` = `$11`/`$13`. `func_80B58D`: four
  direct-page stores. `func_8ABFBC`: a 48-bit shift right by 4 of `D+$46:D+$44:A`, then
  `lsr_n` by 12 (written as word shifts: no 64-bit arithmetic, for the 68000 rule).
- `queue_unpacked_upload` ($80:B95D, waits for interrupts): waits until the upload slot
  `D+$64` is free (the NMI clears it), unpacks into `$7F:(D+$5E)` and fills the slot
  (length, source, bank `$7F`, VRAM word = Y). Its destination bank is `A | $7F00`: the
  fuzz keeps A's high byte below `$80` (above, the bank would be `$FF`, ROM; the game
  passes 0), as for `decompress_to_vram`.
- `$8A:A381` and `$80:9DD0` stay in asm: each has branches into bytes no run executes.

## Hardware-free batch 6 (port-batch-6, 2026-11-15)

- Re-running `tools/portable.py` after batch 5 unlocked new callers. Ported:
  `bcd_to_binary` ($8A:BF9E: five BCD nibbles of `$0144/$0146` to binary in `$0142`,
  weights 1-10000 at `$8A:CABA`, through `func_8ABFBC`), `func_80899F` (a byte into the
  8-byte ring at `$1B30`, skipped when zero; 58,000 calls in the 60k movie),
  `func_8AA022` (the inverse of A's bit 0 shifted into `$0156`, counted in `$0158`),
  `func_80B7BB` (clears to `$FFFF` the words `$01CF-$01DB` equal to `$01DD`),
  `byte_buffer_decimal2` ($8A:9A81: two columns, a space below 10) and `wait_frames_82`
  ($82:841E: a third byte-identical copy of `wait_frames`, C shared).

## Hardware-free batch 7 (port-batch-7, 2026-11-15)

- `func_80894E` (14,600 calls): the four request bytes `$01FB/$01FC/$02FB/$02FC` go into
  the 8-byte ring at `$1B30` and are cleared; one queued byte per call comes out to
  `$0463` (with `$FF` above it, `$0465 = $FF00`, `$0467 = 0`). `$01FB` is what
  `func_80E4C5/DB` set to `$11/$13`; the shape (requests queued, one per frame handed
  on) suggests sound requests, not yet confirmed, so the names stay neutral.
- `binary_to_bcd` ($80:86C9): eight divisions by 10 on the CPU divider (the math-unit
  rule), the remainders shifted into `D+$18:D+$16` as BCD; the C divides natively.
- `func_8AA005`: four `func_8AA022` calls on the words `$0018/$0014/$0010/$000C`.
  `func_80E0C8`: `2a + $B990` below `$40`, else `2a + $2400`. `oam_clear` ($80:83E8):
  the whole sprite table to `$E0`, slot indices reset. `wait_flag_76` ($80:B950, waits
  for interrupts): sets bit 4 of `D+$76`, waits for bit 3.

## Hardware-free batch 8 (port-batch-8, 2026-11-15)

- `func_808FFC` (2,900 calls): two sprites at the slot `D+$74`. The fuzz caught an
  index near `$FFFF`: `abs,X` there carries into bank DB + 1, whose low 8 KB is the same
  WRAM, so the C wraps its addresses to 16 bits instead of reaching `$7F`.
- `func_8AA4BE`: `func_8AB680` for each pointer of a zero-terminated table (through DB,
  `D+$4A` re-read each time). `func_8AC0F2`: five lines of tile text from `$8A:C44F...`.
  `func_8AB994`: two `decompress_to_vram` calls from bank `$88`; its one live entry is
  interrupted, so it is verified on the fuzz alone, and its fuzz (`fuzz_nothing`) only
  varies the registers and flags: in effect one distinct case, zero live samples. The
  same holds for `func_8ABA55` (batch 10).
  `oam_clear_long` ($80:FBD8) and `wait_one_frame` ($8A:B212): wrappers.
- `$81:8527` was listed as portable but streams DSP-4 words: it waits for the DSP-4
  work (the census marks it `dsp4`).

## Hardware-free batch 9 (port-batch-9, 2026-11-15)

- `upload_tiles_c280/c680` ($8A:8D81/$8A:8DA0): the 60k movie now runs them with DB =
  `$83`, so they join the other tile uploads (source + `$20`, VRAM `$C280/$C680`).
- `binary_to_bcd5` ($8A:BFD0): `$0142` in BCD into `$0144/$0146` by repeated
  subtraction of the weights at `$8A:CABA` (the inverse of `bcd_to_binary`).
- `func_82AD12`: `$0463/$0465/$0467` from A and X unless bit 15 of `$047C` is set (the
  same three words `func_80894E` hands its queued bytes on to). `func_8A8232`:
  `$014A = ($0136 >> 2) + $0108`. `func_82F380`: an empty routine (a lone RTS).
- Still in asm for now: `$8A:8290`, `$8A:9F76` (since leaves-17 in C), `$8A:A32B`, `$8A:A381`, `$80:9DD0` (each
  branches into bytes no recorded run executes; they need the static-code route and an
  aimed fuzz) and `$81:8527` (DSP-4).

## Batch 10 and the C-coverage figure (port-batch-10, 2026-11-15)

- `func_8AC03D` / `func_8AC024`: the 32-bit numbers `$0390:$038E` and `$03BF:$03BD`
  shifted left / right by 12. `func_8ABA55`: two `decompress_to_vram` calls (from `$96`
  and `$85`) and a 512-byte palette upload from `$8A:C658`; all three of its live
  entries are interrupted (see `func_8AB994` above for what that leaves).
  `func_82FB1C`: another lone RTS.
- **`tools/progress.py` now reports the code bytes inside verified C routines** (the
  extents `[entry, end)` minus holes from the registry, for rows of `map/verified.txt`
  that are verified and current, intersected with the map's code bytes; overlaps count
  once). It replaces the ad-hoc estimate given to the owner earlier.

## Hardware-free batch 11 (port-batch-11, 2026-11-15)

- After the static-route units unlocked their callers: `func_85CDAF` (long, DB = `$7E`:
  marks the earlier copies of an entry of the word-table pair `$7E:F533/$7E:F733` by
  setting bit 15, walking a byte-wrapped index from `$00FE`), three one-line
  direct-page computations (`func_8A869A`, `func_8A823D`, `func_8A8325`), `func_80B556`
  (resets the `$01C6-$01FA` block, a superset of `func_80B58D`'s stores) and
  `func_8ABF76` (`binary_to_bcd5` of `$038E` and `$03BD` in place, then `func_8AC03D`).

## Hardware-free batch 12 (port-batch-12, 2026-11-15)

- `func_8A9202` (786 calls): ten entries of four object tables (`$06C0/$07C0/$05C0/$08C0`)
  in two rows of five. Its x positions come from a chain of `ADC #$10` without CLC in
  between, and the second row starts again from `D+$7A` keeping the last carry; the C
  carries it the same way (the fuzz puts `D+$7A` near `$FFFF` to exercise it).
- `func_8ABF4E`: the inverse of `func_8ABF76` (`func_8AC024`, then `bcd_to_binary` of
  both numbers in place); `bcd_to_binary`'s adapter is split so that both share its C/V
  model. `func_8A8000`, `func_8A987F`: lone RTLs.
- Next in line: `$85:BC20` (18,507 calls, swaps of `$00E0/$00E4`, `$1E32/$1E36`, ...) and
  `$80:D6F3` (math unit).

## HDMA, measured (hdma-1, 2026-11-15)

- **Instrument:** `bench ... --hdma-report FILE` (`tools/bench/hdma.c`) records, over a
  run: every channel configuration enabled in `$420C` at the end of each frame (B-bus
  target, mode, direct/indirect, table start); the instructions that write `$420C`,
  `$43x0-$43xA` and `$2105` (with whether the write lands in vblank); and the last CPU
  writers of the first 64 bytes of each WRAM table. It does not take part in the
  differential test. Over the 60k movie (9.5 min): 47 configurations.
- **The race** (`$420C = $70` from `$85:8671`, 44,316 frames): channel 4 → `$2126`
  (window 1, mode 4 indirect, table `$7E:4CCC`), channel 5 → `$210D` and channel 6 →
  `$210F` (BG1/BG2 horizontal scroll per line, mode 3 indirect, tables in low WRAM
  around `$06B3-$076A`, three sets in rotation). `$2105 = $09` (mode 1, BG3 priority)
  is written in vblank by `$85:86F7` and `$85:876F`. `$85:BC20` (18,507 calls) is among
  the writers of the scroll tables, with `$80:E00A`, `$80:8C53/$8C5C` and `$8A:8AF5`.
- **Mode 7 screens:** channels 1-4 → `$211B`, `$211D`, `$211F`, `$210D` (mode 3: the
  whole matrix and offsets per line, a perspective plane) from tables at `$81:CBF9-CC0E`.
  **The screen is split by HDMA, not by the CPU** (corrected 2026-11-15: an earlier
  version of this note said the CPU switched `$2105` mid-frame). The CPU writes `$2105`
  while drawing at `$80:BD2D`, but with the value `$80:BB3C` already wrote in vblank
  (same mode, 4,190 and 2,492 frames); only `$80:81F2` (7 times, screen transitions) and
  `$80:F016` (once) write a new mode while drawing. The split comes from channel 1 →
  `$2105` with ROM tables: `$81:D712` = mode 7 for lines 0-183, mode 1 from 184;
  `$8A:88DE` = mode 7 for 168 lines, then mode 2; `$81:D719` = mode 1 throughout. A port
  needs the BG mode per line (from such a table), not a CPU-timed split.
- The intro's channel 5 → `$2105/$2106` (mode 1) reads the WRAM table at `$0D00`, whose
  values `mode7_view` patches at `$0D01/$0D04/$0D07`: BG mode and mosaic per band.
- Menus/intro use HDMA on `$2105`, `$212C`, `$210B`, `$2107`, `$2121` (palette per line,
  indirect from `$7E:2C00`), `$2126/$2128` (windows), `$2132` (fixed colour).
- **Consequence for the PPU end-state rule:** HDMA writes reach the PPU without going
  through the CPU access hook, so a routine that writes a field HDMA also targets in the
  same frame would be compared against HDMA's value. None of the verified routines does
  (their fields: Mode 7 matrix/offsets and BG1/BG2 scroll, written in vblank or under
  forced blank, while the race HDMA starts at line 0); a sample should fail rather than
  pass when HDMA targeted one of its fields during it — to add with the first routine
  that needs it.
- **The table builder needs no interface:** `scroll_tables_build` ($85:BC20, long, 585
  calls in the race movie) is plain WRAM work. It rotates three buffers per table and
  writes the two indirect tables (line count with bit 7 for a repeat run, then a
  pointer), from the end backwards. The bands are, from the top:
  - an optional band for a negative `$0242`/`$1EF2`;
  - the horizon height from `$7E:4CFE`/`$7E:4D4A` (indexed by `$0164`/`$0168`), with
    pointers from `$1E3E`/`$1E40`;
  - two bands at `$0237 + $B0` and `$0237 + $70`;
  - fixed pointers `$A5D8`/`$A5DC`/`$A7D8`/`$A968`, split at 127 lines.

  Live 185/185, fuzz 500/500, paths 313/313. Ten short stretches no recorded run reaches
  are now static code. What will need the interface is the channel setup that points
  HDMA at these tables.
- **Table writers** are now listed by walking each table to its terminator (they were
  the first 64 bytes, more than a table holds, so the earlier list had extra writers).
- **The interrupt handlers are game code** (decided 2026-11-15, advisor): the NMI and IRQ
  handlers are ported to C in `src/` like any routine, with the bench comparing them on
  their own rows. In a port, `platform_wait_vblank()` presents the frame and then runs
  the game's NMI C; the IRQ C runs at the line the game asked for. So the ports'
  renderers are line-ordered (they apply what the handlers set from that line on).
  Before designing their platform calls, measure: the census's single `irq` row split by
  its dispatch targets; which DMA registers each mid-frame kick (`$85:87BA`) inherits from
  the NMI's setup; whether the CPU ever writes an enabled table on lines 0-224 (if not,
  "the port reads tables once per frame" is exact); and whether two enabled channels ever
  share a target (if not, calls keyed by target, not channel). The harness then needs
  RTI rows, an input replay (joypad, `$4210-$4212`, APU ports), and HDMA configuration
  compared by end state at the `$420C` write.
- **Handlers, measured (hdma-2, 2026-11-15; `--hdma-report` over the 60k movie):** the
  vectors point at WRAM trampolines (`$0000` NMI, `$0004` IRQ) that jump to the current
  handler. Two NMI handlers and five IRQ handlers:
  - `$80:8807` (NMI outside the race, 14,587 frames) shares a prologue and dispatches at
    `$80:8931` to `$80:8934`, `$80:FC5F`, `$80:B97B`, `$80:E175`, `$80:D784`, `$80:E28C`,
    `$80:FC14`, `$80:FBD1`;
  - `$85:8377` (the race NMI, 45,360 frames) dispatches at `$85:8387` to `$85:838A` or
    `$85:83CE` (44,316 frames);
  - IRQ `$85:87BA` (603,666 entries: about 14 per race frame), `$80:BC9B` (6,682),
    `$80:E3F7` (1,995), `$80:E48A` (1,970), `$85:95BD` (12).

  `$80:BC9B` (and the NMI target `$80:B97B`) switch stacks between two game tasks; see
  "Two-task screens" below. (An earlier note here said `$80:BC9B` "does long work inside
  the interrupt", the decompressor included. That was wrong: the tracker followed the
  handler across its stack switch into the task it resumes. A handler instance now ends
  at its first RTI; `$80:BC9B` runs 156 to 203 instructions.)
- **DMA kicks inside handlers set all of their registers themselves, except the race
  IRQ** `$85:87BA`: its kick on channels 1, 2, 3 (and 7) writes only the size low byte
  (`$43x5`) and inherits target, mode and source from the race NMI's setup. The source
  address advances across the kicks of one frame. So on each IRQ line, the next slice of
  a source set up once per frame goes to the same target (CGRAM via `$2121`: a palette
  change per band). For the platform this is a named stream (target, source, slice
  sizes), not a register image.
- **Tables written while drawing:** the CPU does rewrite enabled tables mid-frame, so
  "the port reads tables once per frame" is *not* exact:
  - `$80:DC08-$80:DCB7` (`func_80DC5A`'s family) write `$7F:0100-$7F:0174` and
    `$7F:000A-003E` (channels 2/3: windows) while drawing;
  - `$80:FA9A-$80:FAD5` write `$15BC-$1671` and `$1503-$158E` (channel 7);
  - `$8A:9C5E` writes `$0D01-$0D29`.

  On the console, lines below the beam show the new values in the same frame; a port
  that samples tables at the start of the frame shows them one frame later. This is a
  fidelity note for the ports: up to one frame, on intro/menu screens. It does not
  change the C.
- **No two enabled channels ever share a target** (0 cases), so platform calls can be
  keyed by target and mode, not by channel number.
- **Handler rows in the differential test (hdma-3, 2026-11-15):**
  - **Entry and return:** a row with `.rti` is entered at the vector target and returns
    with RTI to the pushed PC and bank, with SP back past the interrupt frame. The adapter
    restores P from the frame (`Regs.sp` gives it the entry SP), with the M and X widths;
    8-bit index registers drop their high bytes.
  - **Acknowledge reads:** reads of `$4210-$4212` are free in these rows. If the C needed
    their value, its outputs would differ and the comparison would catch it.
  - **Inside a waiter's recording:** handlers entered while a waiting sample records its
    interrupts are now sampled too, and only samples that began inside the handler see its
    instructions and accesses. Before this, those entries were skipped.
  - **Fuzz:** it pushes an interrupt frame (PB, PC = `$8000`, a random P with decimal off).
    Each case now also clears the CPU's decimal flag: without it, an RTI case left decimal
    mode on and broke every later routine's fuzz, which is how it was found.
  - **First handler:** `irq_ack_only` (IRQ `$85:95BD`, acknowledge and return): live 2/2,
    fuzz 500/500. A negative row that leaves a WRAM byte behind fails.
 (a) per-line tables of scroll,
  window, palette and Mode 7 parameters, built in WRAM by game code, and (b) a CPU-timed
  BG-mode split. Both are platform services for the ports (`video_*`), to be designed
  from these tables when the first table-building routine (`$85:BC20`) is ported.

## The APU ports, measured (apu-1, 2026-11-15)

- **Instrument:** `bench ... --apu-report FILE` (`tools/bench/apu.c`): every instruction
  that reads or writes `$2140-$2143`, with counts, frames, the most per frame and sample
  values, and a histogram of port writes per frame. It is not part of the difftest.
  Over the 60k movie: 58,840 frames have 1-16 writes, 751 frames have 257-1,024, and 6
  frames have up to 4,096.
- **Three kinds of traffic:**
  - **The driver upload at boot** (`$86:E096-$86:E0DD`, the standard IPL protocol:
    `$AA/$BB` ready, `$CC` start, a byte counter echoed on `$2140`): about 4,500 bytes.
  - **Data uploads at scene changes** (`$86:E0F0` with `$86:E1BA`, `$86:E1E6`,
    `$86:E22A` and `$86:E2B4/$86:E37E`, about 30 times): a custom protocol of three data
    bytes per step on `$2140-$2142`, with `$2143` as the toggle (`$AB`/`$CD`/`$FF`
    markers). These are music and samples for the driver.
  - **A 16-byte message every frame** (`$86:DFFB`, called from `$86:DFE1` in the NMI's
    common tail): it waits for `$77` on `$2141`, then sends four 4-byte groups, with
    bit 7 of `$2140` alternating as the handshake. The groups come from WRAM `$045B-$046C`
    (master and music state `$045B-$0462`, two sound-request slots `$0463-$0467` and
    `$0468-$046C`, cleared after sending) and from bit 4 of `$213F` (the console's
    50/60 Hz flag, as bit 1 of the first byte). `$86:DFAE` before it fades `$045C` by
    `$03E0` per frame.
- **The interface this points to** (to design with the first port):
  - **Per frame:** `sound_send_frame(16 bytes)`. The game C builds the message, and the
    port's native driver receives it; the handshake is the platform's.
  - **At scene changes:** an upload call carrying the data block (source and size) the
    game hands over.
  - **At boot:** the driver upload, which a port with a native driver accepts and ignores
    (recorded on the bench).
  - **Inputs to replay** on the bench: the `$2141`/`$2140` handshake reads, `$213F`, and
    the IPL echo.

  The sound request bytes queued by `func_80894E` (`$0463`, with `$FF` above it) are the
  ones this message carries: the earlier guess, now confirmed.

## Sound interface, first calls (sound-1, 2026-11-15)

- **Platform calls** (`src/platform.h`):
  - `sound_send_frame(msg[16])`: the frame's message to the sound driver, in the order
    the original writes the bytes;
  - `sound_is_ready()`: the original's `$77` on `$2141`;
  - `video_is_pal()`: bit 4 of `$213F`.

  The handshake waits belong to the platform.
- **Harness:** APU port writes are effects (port, value), in order with the DMA effects,
  so the C's message must produce the same sixteen writes. Reads of `$2140-$2143` and
  `$213F` are inputs: the harness records them and serves them to the C's input calls
  in order, the first unread value of each register. A value repeated by a polling loop
  counts as one input. If the C asks for more than the original read, the sample fails.
  The two handshake routines have no fuzz, because the fuzz runs the main CPU alone and
  the sound CPU would never answer.
- **Ported** (`src/sound.c`):
  - `sound_fade_step` ($86:DFAE): the master-volume fade, clamped at `$7F` and 0. Live
    2,365/2,365, fuzz 500/500.
  - `sound_send_message` ($86:DFFB): builds the message, then clears the two request
    slots. Live 2,350/2,350, paths 60/60.
  - `sound_frame` ($86:DFE1, long): the fade, then the message when `$046E` bit 7 is set
    and the driver is ready. Live 2,399/2,399, paths 12/12.

  A negative variant with the two request groups swapped fails at effect 8.

## Controllers (pads-1, 2026-11-15)

- **Platform calls:**
  - `input_pad1()`: controller 1's 16 bits from the automatic read;
  - `input_port1_data()`: port 1's first data line after it;
  - `input_multitap_pair(pair, &d0, &d1, &tail)`: two multitap pads read serially
    through port 2, plus the lines on a 17th read, which a plugged-in pad answers with 1.

  The waits on `$4212` and the latch and I/O-line writes (`$4016`, `$4201`) are the
  platform's.
- **Harness:** `$4212/$4213`, `$4218-$421F`, `$4016` and `$4017` are inputs, replayed
  like the APU ports. The serial lines are not collapsed: each read is a bit. Writes to
  `$4016`/`$4201` are free. The fuzz now gives both controllers random states, so the
  serial reads vary, signature bits included.
- **Ported** (`src/pads.c`; the module is not called `input` because LakeSnes has an
  `input.h`):
  - `pads_read_pair` ($83:87C1): live 4,000/4,000, fuzz 500/500;
  - `pads_read` ($83:86FF): the five pads as (held, pressed) pairs at `$000C-$001F`,
    "pressed" collecting new presses, and an absent pad reading as 1. Live 2,647/2,647,
    fuzz 500/500, paths 89/89;
  - `pads_read_long` ($83:86FB).

  Its "pad A absent" test (`$83:878F`) never ran in a recorded movie and is now static
  code, reached by the random pad states. (The extra random draws shifted every
  routine's fuzz sequence; `scroll_tables_build` then missed one path at the selftest's
  200 cases, so its fuzz now draws height bytes of 1-3 on purpose instead of by chance.)
  V at the exit comes from bit 6 of `$4213`
  (the 16-bit `BIT $4212`). A negative variant that drops pads A and B fails.

## The NMI's common tail (nmi-1, 2026-11-15)

- **Handler tails as rows:** `.rtiFrame` gives the bytes the handler's prologue pushed
  above the interrupt frame. The common NMI prologue (`$80:8807`) pushes DB, A, X, Y and D
  (9 bytes) before it dispatches through `jmp ($0008)`, so a tail's row returns through
  RTI with those registers pulled back. The adapter restores them from the stack, and P
  from the frame. The fuzz pushes random prologue bytes above its interrupt frame.
- **`nmi_tail`** ($80:8934, `src/frame.c`) is where most NMI dispatch targets end:
  - it reads the pads (`pads_read_long`), unless bit 2 of `$0026` is set;
  - steps the sound request ring (`func_80894E`) and sends the sound message
    (`sound_frame`);
  - counts the frame (`$0024`).

  Race movie 2,616/2,616 live, most of them inside a waiter's recording, which the
  handler sampling of hdma-3 now reaches; paths 13/13. It has no fuzz, because of the
  sound handshake. A negative variant that skips the frame count fails.

## NMI targets, first two (nmi-2, 2026-11-15)

- **Tail jumps:** a row's `.alsoIn` lists ranges it may continue into at its own stack
  depth. A dispatch target that ends with `jmp nmi_tail` declares the tail's extent; the
  C calls `nmi_tail()`.
- `oam_from_objects` ($8A:8E1B, long; 1,578 calls in the 60k movie) turns the 128-slot
  object table (`$0500` used/size, `$0600` x, `$0700` y, `$0800` tile) into sprite
  entries and their high-table bits. Its body sits after its own RTL. Race movie
  140/140, fuzz 500/500, paths 92/92.
- `func_8AA050` (long): `upload_7f_to_3000`, `oam_from_objects` and two counters when
  `$018E` is set, then `func_8AA005`.
- `nmi_target_80FC5F` (190/190, paths 14/14) and `nmi_target_80FBD1`: the first two NMI
  dispatch targets in C, each its own work and then `nmi_tail`.

## Raster interrupts, first (raster-1, 2026-11-15)

- **Byte registers:** `$212C/$212D`, `$2123-$2125`, `$2130/$2131`, `$4200` and
  `$4207-$420A` hold the last byte written, with no latch. The harness keeps, per sample,
  which of them the original wrote and their last values, and the C's typed platform
  calls must match both ways. A negative row that drops a layer fails.
- **Fields are read when written:** a field's value is now taken at the instruction after
  the routine's last write to it, not at the return, because HDMA keeps writing the same
  registers on later lines (here channel 4 feeds `$210F`). The BG3 scroll (`$2111/$2112`)
  joins the fields.
- **The IRQ model:**
  - **When:** the interrupt position is a pair of byte registers (`platform_set_irq_h`,
    and the V one with the first handler that sets it). A handler may move it, and the
    next interrupt of the same frame follows.
  - **Which handler:** the trampoline bytes at `$0005-$0007` stay plain WRAM; a game-side
    dispatcher maps them to the C handlers.
  - **The `wai`:** inside the handler it only lines its writes up with the horizontal
    blank of the interrupt's line. `irq_split_bottom` writes on line 179, at dots
    1,070-1,242 of 1,364. So a port runs the IRQ C for line V after line V is drawn, and
    the `wai` is a comment in the C.
  - **Fuzz:** these rows have none, because the fuzz never raises the interrupt a `wai`
    waits for. A `wai` resumed with I set does not count as an interrupt on the bench, so
    the samples are kept.
- **First raster handler:** `irq_split_bottom` (IRQ `$80:E48A`, the bottom of the screen
  `$80:E28C` sets up). It sets the next interrupt's dot to `$100`, scroll 0 for BG1-BG3,
  main screen `$17`, no sub screen, colour math `$05`. Race movie 459/459, paths 29/29.

## Raster interrupts, the top split (raster-2, 2026-11-16)

- **Colours:** `$2121` then two `$2122` writes are one effect (index, colour), and they
  consume the `$2121` setup as a VRAM word consumes `$2116`. A colour completed from a
  byte written before the routine fails the sample.
- **The fixed colour** (`$2132`) is three fields (red, green, blue), each written when its
  select bit is set. The setter takes the raw byte. The field masks are now 32 bits wide.
- **Interrupt control** (`$4200`, `$4209/$420A`) joins the byte-register table, through
  `platform_set_interrupts` and `platform_set_irq_v`.
- **`irq_split_top`** (IRQ `$80:E3F7`), while `$017A` is below 100:
  - BG1-BG3 scrolled by `$0120`, layers `$17/$10`, colour math `$05`, colour 0 =
    `$7CC6`;
  - the next interrupt at line `$B3`, dot `$10`, handled by `irq_split_bottom` (the
    trampoline bytes `$0005-$0007` are written as WRAM).

  Always: fixed colour black, `$2130 = $02`, windows `$AA/$0A/$0A`. Race movie 461/461,
  paths 62/62. A negative row with colour 0 one step off fails.

## The split screen, complete (raster-3, 2026-11-16)

- `nmi_target_80E28C` ($80:E28C), the NMI side of the screen whose two raster IRQs are
  `irq_split_top` and `irq_split_bottom`:
  - it shrinks the split height `$017A` by 4 per frame to 0;
  - rebuilds the HDMA tables at `$1B00`, `$0D00`, `$0D28` and `$0D14` for it, as
    overlapping word writes kept in the original's order;
  - sets up the line interrupt (line `$7E`, dot `$10`, the trampoline to `$80:E3F7`) and
    the top of the screen: scroll 0, colour 0, colour math with the fixed colour from
    `$017C/$017D`, windows and layers;
  - then runs `nmi_tail`.

  Race movie 460/460, paths 130/130, writes in vblank (lines 227-231). With it the whole
  effect is C: the NMI prepares it, the first IRQ splits the screen at line `$7E` and
  schedules the second, and the second restores the bottom part at line `$B3`.

## NMI target $80:E175 (nmi-3, 2026-11-16)

- `nmi_target_80E175` (runs with D = `$0100`): until `$0116` is set, it rebuilds the HDMA
  table at `$1B00` for a band that opens 4 lines a frame (`$0114`, up to `$3A`, then
  `$0116 = 1`), then runs `nmi_tail`. One path shortens a repeat run: it adds to the run's
  count byte in 8 bits, and when that gives `$80` it decrements only the low byte of the
  index it has pushed. The C does the same.
- **NMI rows can fuzz** after all. The tail's sound message is only sent when bit 15 of
  `$046D` is set; with it clear, nothing waits for the sound CPU. The fuzz clears it and
  draws the band height over every value up to `$3A`. That reached the paths no recorded
  run took (heights `$34-$37` and `$3A`): live 10/10, fuzz 500/500, paths 134/134. The
  other NMI rows can take the same fuzz.

## NMI target $80:D784, quiet fuzz for the NMI rows (nmi-4, 2026-11-16)

- **More byte registers:** `$2101` (sprite size and tiles), `$2105` (BG mode) and
  `$2107-$210C` (tilemaps and tile addresses) join the table, with the setters
  `video_set_obj_select`, `video_set_bg_mode`, `video_set_bg_map(bg)` and
  `video_set_bg_tiles(pair)`. The table's masks are now 32 bits wide. The BG3 vertical
  scroll joins the fields.
- **`nmi_target_80D784`:** it copies a 7-byte table (`$81:C207` to `$1B00`, an MVN),
  uploads 4 KB from `$7F:0000` to VRAM and sets the screen up (mode `$09`, BG maps,
  sprites, BG3 scroll, colour 2), then runs the tail. Live 1/1, fuzz 500/500, paths
  61/61.
- **Fuzz with the sound message off** now covers `nmi_tail` (500/500),
  `nmi_target_80FBD1` and `nmi_target_80FC5F`. `nmi_target_80E28C` keeps no fuzz: it sets
  `$4200 = $B1` itself, so the NMI it enables interrupts the fuzz case (23 of 500 were
  lost). Its live samples cover all 130 paths.

## The race NMI's tail (race-nmi-1, 2026-11-16)

- **The race NMI** `$85:8377` pushes the common frame (DB, A, X, Y, D) and sets D = 0. It
  then dispatches through `$0008` to `$85:838A` or `$85:83CE`. `$85:83CE` is the full
  frame: the race's DMA and HDMA setup. It ends by jumping to `$85:838A` (`$85:87B7`).
  So `$85:838A` is the tail of every race frame (45,360 entries on the 60k movie). It
  is reached directly on 1,044 frames and through `$85:83CE` on the rest. (This unit
  first called it the "light frame", `nmi_race_light`. That was wrong, and race-nmi-2
  renamed it.)
- **`nmi_race_tail` ($85:838A):**
  - it steps the brightness fade (`$014A` towards `$0148` by `$0146`, then stops on the
    target) and writes the display;
  - then it runs `sound_frame` and the pads (through `pads_read_long_2` at `$83:96CD`, a
    second copy of `pads_read_long`) and counts the frame in `$0024` and `$00BA`;
  - it runs CLI, so the race's raster IRQs can start before its RTI. The C has no
    interrupt flag: on a port, the IRQ C runs at its line regardless;
  - race1: live 513/513, fuzz 500/500, paths 31/31. The negative row (brightness 0
    written without the forced blank) fails on the `$2100` byte.
- **`$2100` (display: forced blank, brightness) joins the byte registers**, with the
  setter `video_set_display`.

## The race NMI's full frame, first pieces (race-nmi-2, 2026-11-16)

- **What `$85:83CE` does** (still assembly; it ends by jumping to `nmi_race_tail`):
  - it sends up to three VRAM transfers described at `$1DFE-$1E04` and `$1E52-$1E56`, a
    fixed 400 bytes from `$7E:230C`, and one from a table at `$86:C8AC` picked by
    `$1F24`, all on channel 7;
  - it drains the race VRAM queue (`race_upload_queue_drain`), sends the sprite table
    (OAM, from `$1576` or `$1796` by the double-buffer flag `$00B4`), and sends three
    palette blocks;
  - it writes colours 0, 7, `$21` and `$31` directly and runs `func_838989`;
  - it sets up HDMA channels 1-6 (enabling 4, 5 and 6) and the window and colour-math
    registers;
  - it advances a BCD clock (`$5A/$5C`, in decimal mode) and sets the BG mode and the
    IRQ line for the race IRQ `$85:87BA` from `$0486,x`;
  - every fourth frame it calls `$85:9147`, and it queues a sound message.

  Two stretches of it have never run (`$85:86DA`, `$85:8702-$85:8750`).
  - **Still missing before it can be C:** a platform call for the sprite table (OAM) and
    a model of HDMA channel setup.
- **`race_upload_queue_drain` ($83:8644, long form `$83:96D1`):**
  - it sends the race's VRAM queue (a ring of 128 entries in parallel word tables at
    `$7E:F433-$7E:F9B2`) while the frame's DMA budget `$FC` lasts. A whole transfer costs
    its size + 100;
  - a transfer that does not fit sends what the budget allows. Its entry keeps the rest
    for the next frame, and the budget becomes 0;
  - an entry can carry a word to store in `$7E` once it is sent. That path ends with a
    PEA/PLB pair that leaves DB = `$83`, so the C returns whether it stored one and the
    adapter sets DB.

  race1: live 446/446, fuzz 500/500, paths 75/75. The fuzz keeps at least one entry
  live, because a queue of only skipped entries sets channel 7's mode and sends nothing.
  The harness does not model that leftover, it was never seen live, and the race NMI
  rewrites channel 7 before its next kick.
- **`func_838989` ($83:8989, long form `$83:96D5`):** unless paused, it adds the signed
  word `$01CC` to the 24-bit value at `$00D4`. Then it writes the upper word + `$3C` to
  `$1536,x` and `$1556,x` (X = `$00E2`), or the upper word less an eighth to the first
  when `$1F32` is negative. race1: live 445/445, fuzz 500/500, paths 32/32.
- Neither has register outputs: the race NMI overwrites A, X and Y after both.

## `race_cycle_blocks` (race-nmi-3, 2026-11-16)

- **`$85:9147`**, called by the race NMI every fourth frame, sets DB = `$7E` and works on
  `$7E:2000-$7E:213F`. That is WRAM: the `$21xx` operands in it are not PPU registers.
  - The area is five groups of eight 8-byte blocks. Each block becomes four copies of the
    next block's first word, and the last block gets the first block's. So each group's
    pattern moves by one block every fourth frame.
  - It has no register outputs (the next instructions of the race NMI overwrite A and the
    flags, and its tail restores X and Y). The carry it leaves is the caller's, untouched.
  - race1: live 112/112, fuzz 500/500, paths 206/206.

## NMI prologues, the sprite table, PHP/PLP liveness (nmi-5, 2026-11-16)

- **Rows that end at a dispatch (`endAt`):** a handler's prologue ends at its indirect
  jump to the current target, and each target already has its own row. Such a row starts
  at the vector target (`.rti`) and finishes when the PC reaches `endAt`. There it
  compares WRAM, effects and byte registers, and D and DB, the targets' entry state.
  - In C, the port's handler calls the prologue and then the target. The frame the
    original pushes is the targets' business (their adapters restore from it).
  - The flag watch of such a row follows the target up to its RTI, which writes every
    flag. It runs inside a handler that a waiting sample records, and its cap is 20,000
    instructions instead of 400.
- **`nmi_prologue` ($80:8807):**
  - it runs the brightness fade (`$019A` towards `$0198`) and writes the display;
  - when `$0026` asks for them, it sends the palette (`$81:0500`, 512 bytes, CGRAM 0)
    and the sprite table (`$81:0900`, 544 bytes, OAM 0). Each costs `$280` of the
    frame's 4 KB VRAM budget at `$002C`;
  - then it drains the VRAM queue within what is left (`vram_queue_drain`, `$80:88B9`,
    now C).

  A queued transfer of size 0, or a split when the budget is already 0, sends 65,536
  bytes (the DMA size register's 0), as the original does. race1: live 2,616/2,616, fuzz
  500/500, paths 131/131. The negative row (the sprite table sent a second time, one word
  further in) fails.
- **`nmi_race_prologue` ($85:8377):** it only saves, sets D = 0 and DB = `$85`, and
  dispatches. Its C is empty. race1: live 511/511, fuzz 500/500.
- **The sprite table on the platform:**
  - `video_set_oam_address(word)` writes `$2102/$2103`. Both are now byte registers,
    compared as the last value;
  - `video_upload_oam(oam_word, src, size)` is a new effect (`EFFECT_OAM`). The bench
    reads it from a DMA kick to `$2104` (mode 0 or 2) whose target is the OAM word
    address at the kick (bit 8 = the high table).
- **PHP/PLP in the flag liveness:** PHP counted as reading every flag, so a flag that
  passed through `sound_frame` (PHP ... PLP) looked live. The game has five PHPs, each
  at a routine's start and matched by a PLP of the same value, so the flags pass through
  the pair unchanged.
  - **First version (nmi-5):** the watch treated PHP as reading none and PLP as writing
    none. That left a hole (advisor): a flag written between the two counted as dead,
    though the PLP brings the watched value back for a later read.
  - **Now (flags-1):** a PHP saves the watch's state. Inside the pair a flag write only
    shadows that flag (later reads of it are of the new value). The PLP that pulls that
    PHP's byte (matched by SP) restores the state the PHP saw. Reads of flags not
    shadowed are live as before.
  - A PHP whose value is pulled by PLA would be missed; none exists in the code
    classified so far.

## The race HUD and the channel survey (race-nmi-4, 2026-11-16)

- **`race_hud_draw` ($85:8814)**, called by the race NMI's full frame when the frame's
  DMA budget allows, writes the HUD's tilemap words to VRAM. It writes:
  - a 7-word bar from `$003E`, the words at `$0042-$004C` and `$1E94-$1E98`;
  - the BCD clock `$5A-$5C` as digit tiles, and the `$0235` count capped at `$1F06`;
  - a column from `$006C-$0074` (VMAIN `$81`), and the rows `$0080-$008E` and
    `$00A4-$00AA`;
  - the gauge from the word at `$025B`, whose high byte `$025C` is also its count;
  - five lights with the one at `$014C` lit;
  - two 26-byte strips from `$7E:4E61`, or from `$83:A5A4` when the 8-bit accumulator
    `$7E:4E4E` (+= `$7E:4E4D`) goes negative.

  race1: live 195/195, fuzz 500/500, paths 274/274. The negative row (the lights one
  place off) fails on the light's VRAM word.
  - The fuzz keeps the bar at 0-14 and the gauge count at 0-10. Past those, the original
    loops until its 16-bit index wraps.
  - The fifth light's `LDA #$295E` (`$85:8A57`) never ran: it joins `static_code.txt`,
    and the fuzz runs it.
- **Harness:**
  - VRAM words can be written with VMAIN `$81` (a step of 32 words, a tilemap column).
    The C names each word's address, so only the mode check changed;
  - `$2115` (VMAIN) is a byte register, set by `video_set_vram_increment`;
  - a sample may hold 128 effects (was 64).
- **The flag watch after flags-1:** compared with the list before nmi-5 (`c8fdafd`),
  six rows went from a live flag to none: `func_80894E` (NVZC), `func_80899F`,
  `pads_read`, `pads_read_long`, `sound_fade_step` (V) and `race_cycle_blocks` (C). Each
  of those flags reached only `sound_frame`'s PHP (or, for `sound_fade_step`, a PLP that
  replaces it) before the RTI. Their C matched those flags anyway.
- **Survey of the DMA channel writers (`--hdma-report`, 60k movie), for the channel
  model:**
  - **Enabled without being set up:** `$8A:95F5` enables channel 2 (`$420C = $04`)
    without writing any of its registers. The setup is sticky across routines, so the
    channel state must be part of the platform's state, not of each call.
  - **Partly set up:** `$80:F122` (channels 6, 7) and `$80:F3C7` (channels 1, 3) write
    only the table address (`$43x2/3`). `$85:840E` writes the table address of channels
    5 and 6 every race frame, and the rest of them later in the same frame. `$82:85DC`
    writes only the sizes of channels 1-3. So the calls need a "new table address" form
    besides the full setup.
  - **The counters `$43x8-$43xA`:** only `$80:D1AF` writes one (`$431A`, the exit of the
    two-task screen, already parked).
  - No small routine both sets up a channel and enables it. They all sit inside large
    scene setups, and the race NMI's full frame is the main user.
  - The plan (advisor): each channel is an opaque slot (`$420C` is a mask of them).
    Registers written after a channel's last kick are claimed at the sample's end by C
    calls with the same decoded fields: an HDMA setup, a new table address, or a stream
    (mode, target, source) for the IRQ's later `send(slot, size)`.
    - `$420C` joins the byte registers.
    - The "set up but no DMA started" failure is waived only for claimed channels.
    - The comparison is at the sample's end, not at the `$420C` write as the handler
      model section said.

## DMA channels as slots, streams, the race IRQ (dma-1, 2026-11-16)

- **Channel registers are now tracked per sample** (`$43x0-$43x7`; writes to `$43x8-$43xF`
  and reads of any channel register fail the sample). For each channel the harness
  keeps what the routine wrote, and what it left set up after the channel's last kick
  (registers 0-4 and 7).
  - **At the sample's end** what is left set up must be claimed by the C with the same
    values: `dma_hdma_setup(slot, mode, target, table)` (0-4),
    `dma_hdma_set_indirect_bank` (7), `dma_hdma_set_table` (2-3), `dma_stream_setup`
    (0-4).
  - The sizes (5-6) are a kick's own and are not claimed. A stream kick's effect carries
    them.
  - This replaces "DMA set up but never started" for the channel registers (it stays for
    the VRAM and CGRAM addresses). It also replaces "compared at the `$420C` write"
    (hdma-2): the comparison is at the sample's end. `$420C` is a byte register
    (`video_hdma_enable`).
- **A kick is classified per sample:**
  - mode, target and source all written in the sample: an upload, decoded as before;
  - none of them written: a **stream kick**. The routine sends the next chunk of a
    source set up earlier (the platform keeps each slot's place). Its effect is the slot
    mask, the CGRAM address at the kick, and each slot's size (up to 255 bytes);
  - some written: the sample fails (not modelled), and so does a kick that mixes the two
    kinds.

  On the C side: `dma_stream_size(slot, size)`, `video_set_cgram_address(index)`, and
  `dma_stream_kick(slots)`.
- **`irq_race_band` ($85:87BA)**, the race's raster IRQ (about 14 per race frame):
  - it sends the next chunk of the streams the race NMI set up. Slot 1 sends 4 colours
    to CGRAM `$21`; slot 2 sets the CGRAM address from its source byte, for slot 3's 4
    colours; slot 7 sends the BG mode byte. The band's mask at `$0596,x` picks the slots;
  - then it moves the IRQ to the next band's line (`$0486,x`). With none, it moves it to
    line `$FF`, dot `$78`;
  - it writes `$4207`/`$4209` as single bytes, hence `platform_set_irq_h_low` and
    `platform_set_irq_v_low`;
  - it waits (WAI) for dot `$FF` of its line, so it has no fuzz, like the other raster
    IRQs. race1: live 4,000/4,000, paths 42/42;
  - the negative row, the same C with CGRAM address `$22`, fails on the stream kick's
    address.
- All 153 earlier rows pass under the channel model: none kicks a channel it set up only
  in part.

## The race NMI's full frame (race-nmi-5, 2026-11-16)

- **`nmi_race_full` ($85:83CE)**, the race NMI's frame on most race frames (40,316 of
  45,360 on the 60k movie), ends in `nmi_race_tail`. Its frame DMA budget at `$00FC` is
  3.5 KB.
  - **When the game asks for it** (`$00B8` bit 0, flipping the double-buffer flag
    `$00B4`): it sends the VRAM transfers described at `$1DFE-$1E04` and
    `$1E52-$1E56`, 400 bytes from `$7E:230C`, and one from the table at `$86:C8AC`
    (picked by `$1F24`). It sets the BG1/BG2 scroll tables' addresses and copies the
    per-frame values.
  - **Every time:** the sprite table, two palette blocks and colour 0; the race VRAM
    queue; colours 7, `$21`, `$31`; and `race_hud_draw` when the budget is still whole.
  - **Channels:**
    - HDMA slot 4 is the window shape (indirect from `$7E:4CCC`); slots 5 and 6 are the
      BG1/BG2 scroll (bank `$83`);
    - streams: slots 1 and 3 are colours from `$7E:$C8`/`$7E:$CC`, slot 2 the CGRAM
      address byte `$83:8D35`, slot 7 the BG mode bytes;
    - `$420C = $70`.
  - **Then:** the windows and colour math, and the race clock (`$5A-$5C`, BCD). It sets
    the first band's raster IRQ from the band table, or line `$FF` when there is none.
    Every fourth frame it runs `race_cycle_blocks`, and it queues the sound request.
  - **The clock:** the original adds in decimal mode (SED). The C adds BCD digit by
    digit, which is exact for valid BCD operands. The fuzz writes only valid BCD clocks,
    because for other digits the only reference would be what LakeSnes does.
  - **Two stretches that never ran** join `static_code.txt`, and the fuzz runs both:
    - `$85:86DA`, the clock's carry on the +2 step;
    - `$85:8702-$85:8750`, the first band starting within 4 lines (low byte of
      `$0486,x` 1-4), sent from the NMI. It repeats `irq_race_band`'s kick, but on
      channels set up in the same sample.
  - race1: live 445/445, fuzz 500/500, paths 382/382. The negative row (slot 4's
    indirect bank `$7F`) fails on `$4347`.
- **Harness:**
  - **The CGRAM address within one kick:** the channels of one kick run in order, and
    the CGRAM address moves on with every colour sent. A byte sent to `$2121` sets it.
    The original's effects and the C's are both decoded that way.
  - **A C stream kick of slots it set up in the same sample** (all of mode, target and
    source claimed) gives plain effects, as the original's kick of channels it set up
    does. Either way the kicked slots' setup counts as used.
  - New calls `dma_hdma_set_mode` and `dma_hdma_set_bank`, the parts of a setup on
    their own: the race NMI sets slots 5 and 6's table address only on requested
    frames.

## The screen register image (screen-1, 2026-11-16)

- **`video_load_screen` ($80:81CC)**, the top of the roots list, writes every screen
  register from a 68-byte image (bank in A, address in X), `$2100` to `$420D`, in
  order. It turns DMA and HDMA off first. Its callers (six directly, six more through
  `video_load_screen_long`, `$80:81C8`) pass images in ROM banks `$80`-`$8A`.
  - The original makes the image's bank its data bank for the register writes too. An
    image in a WRAM bank would be written over itself, so the fuzz puts its images in
    low WRAM through banks `$00`/`$80`, which map the registers.
  - race1: live 3/3, fuzz 500/500, paths 145/145. On the 60k movie: live 11/11. The
    negative row (window 1's left edge from the next byte) fails on `$2126`.
- **More registers in the harness:**
  - byte registers (last value written): mosaic `$2106`, mode 7 settings `$211A`, window
    edges `$2126-$2129`, window logic `$212A/$212B`, window masks `$212E/$212F`, screen
    settings `$2133`, ROM speed `$420D`. The mask is now 64 bits wide (harness only;
    the game's C keeps its 16/32-bit rule);
  - the BG4 scroll as PPU fields (`$2113/$2114`), with the same latch rules as the other
    scrolls.
- **Addresses left set:** the VRAM address (`$2116/$2117`) or the CGRAM address
  (`$2121`) written and then used by nothing in the sample is compared at the end with
  `video_set_vram_address` / `video_set_cgram_address`. A C upload, colour or stream
  kick uses the address up, as a DMA or data write does in the original. This replaces
  the "address set up but no DMA started" failure for them.
- **Fuzz cases start from the same PPU and DMA state** as well as the same WRAM.
  `video_load_screen`'s fuzz wrote a random VMAIN that the next routine's cases
  inherited (`race_hud_draw` then failed one case on "a VMAIN mode the C does not
  model"). All 156 race1 rows pass with the restore.

## Small roots (roots-3, 2026-11-16)

- **`video_load_screen_long` ($80:81C8):** JSR `video_load_screen`, RTL. It is the top of
  the refreshed roots list (14 blocks behind it). 60k: live 5/5.
- **`fade_in_start` ($8A:BAF2):** sets a fade up to full brightness (target `$0F00`, step
  `$0080`; the NMI prologue moves `$019A`). 60k: live 1/1.
- **`hdma_window_tables_init` ($8A:A09F):** the two window HDMA tables at `$0D00` and
  `$0D20`: seven bands of 24 lines (the second table's words `$E0`), each ending in 0.
  60k: live 1/1.
- Both leaves run once on the 60k movie and take no inputs. Their fuzz only varies the
  registers and flags around them.

## Harness loose ends; the APU uploads parked; what next (review-2, 2026-11-16)

- **A kick uses an address only through its channels' targets.** Before, any `$420B`
  write, even 0 or an OAM kick, cleared the original's "VRAM/CGRAM address left set"
  flags, while the C's claim cleared only on a matching upload. So an address left for a
  later routine could go uncompared. Now:
  - on both sides, a kick clears the VRAM flag only through a channel to `$2118/$2119`,
    and the CGRAM flag only through one to `$2121/$2122`;
  - the C's stream kick tells by the slot's target: the claimed one for slots it set up,
    else the channel's.

  The first run under the narrower rule caught `nmi_race_full`'s never-run band path.
  There a kick with no CGRAM slot leaves `$2121 = $21` set, and the C had cleared its
  claim.
- **The CGRAM address model is checked against the PPU.** Both sides decoded the
  address across one kick with the same model (dma-1), so nothing checked it.
  - After a kick, once LakeSnes has run the transfers (its DMA state back to idle), the
    modelled end address must equal the PPU's. The check runs at the first instruction
    after the transfers, or at the next CGRAM register access, whichever comes first.
  - A mismatch fails every sample in progress. None fails now.
  - (The first try checked at the very next instruction. LakeSnes runs the transfers
    after the kick's own instruction, so that check was too early.)
- **The APU scene uploads are parked** (advisor) until a port designs the sound
  interface, as "The APU ports, measured" already said:
  - almost none of their entries could be compared: `$86:E37E` is interrupted in 133 of
    147 entries, `$86:E1E6` in 15 of 20, and `$86:E1BA` in 2 of 2;
  - the handshake rules out fuzz;
  - one sample holds thousands of port writes, against 128 effects;
  - `$86:E105` is a BRK on a handshake timeout, and `$86:E15F` calls two never-run
    senders for blocks under 3 bytes.

  When their turn comes, the bench should decode the port traffic into blocks
  (destination, size, data), the way kicks become uploads.
- **Next, in order:**
  - the hardware-free routines `tools/portable.py` lists now. It had not run since the
    display, OAM, VMAIN, channel and screen registers were added. Among them: `$80:99D1`
    (14 callers), `$80:9C0B` (10), `$80:9ABB`, `$80:993E`, `$80:98C4`, `$80:9957` (9
    each), `$8A:BB99`, `$80:8635`, `$8A:BC8A`, `$80:DAED`;
  - then DSP-4, starting from the fixed-size commands the stand-in never missed (`$0A`,
    `$11`), not from the two DSP-4 roots;
  - the bench's chip is the stand-in, not the firmware. Each command proved in refbench
    is to be routed in the bridge to `src/dsp4.c`.

## The password text (password-1, 2026-11-16)

- **`password_encode` ($8A:BC8A)** writes the password text at `$0D40`:
  - `bits / 5 + 1` characters of 5 bits each, shifted out of the big number by
    `func_8ABB5C`, each picked from the 32-entry table at `$8A:BC5F`;
  - then two characters from `$0192` (bits 0-4, then 5-9), then a 0 word.

  The first shift's carry in is the borrow of the count division (done by subtraction).
  Each later shift's carry in is what the previous one left.
- **Its flags are read by the caller** (N, V, Z, C live):
  - C is always 0 (the tail's last LSR shifts out a bit its mask cleared);
  - V is the overflow of the division's last `- 5`.

  The adapter computes both. The game C has no flags, so this stays in the bench.
- 60k: live 1/1, fuzz 500/500, paths 46/46.
- **The other entries `portable.py` lists** (`$80:98C4`-`$80:9C0B`, money and menu code
  with BCD) never run on the 60k movie, only on the TAS. So they wait for bench coverage
  (explore-1, tas-2).

## DSP-4 command $0A (dsp4-0a, 2026-11-16)

- **Why it is first** (advisor's order):
  - it is one of the fixed-size commands the stand-in never missed;
  - its users are `$81:F887` (351,807 entries on the 60k movie, the road under each
    car) and `$83:E8AB`;
  - `$11` comes next (`$81:E631`). Then the sprite group (`$03/$0B/$06/$09`), then the
    road.

  The two DSP-4 roots `roots.py` lists (`$82:D934`, `$85:B828`) sit inside `$09` and
  the road commands, so they are not the start.
- **Measured, outputs only:** `refbench --dsp4-cmdlog CMD FILE` logs each run of one
  command as the game sees it: from the command word (the boundary given by the shadow,
  valid while the shadow agrees for that command), the bytes the game writes, then the
  bytes it reads from the firmware. The chip's memory and program stay unread.
  - **The run:** first 50,000 TAS frames, 281,548 runs of `$0A`, 4,281 distinct inputs.
  - **Pure:** equal inputs always gave equal outputs.
- **The command:** three words in (t, a near row, a far row) and four words out.
  - Each row word holds four signed 4-bit widths (bits 15-12, 11-8, 7-4, 3-0), each
    worth 48.
  - Output j goes from the near row's width j to the far row's:
    `near_j + floor((far_j - near_j) * t / $8000)`.
  - The game sends t = (a position's high byte) >> 1, so t is a multiple of `$80` below
    `$8000`.
  - So: four road edges between two rows of the track, interpolated at the car's place.
    Rounding to nearest, or truncating toward 0, each disagree with the chip about 1,300
    times.
- **`dsp4_edges` (src/dsp4.c)** is that formula. refbench checks it on every run it
  logs: **281,548 runs, 0 differ.**
- **The domain:**
  - in every run, widths 0 and 2 are -7..-1, widths 1 and 3 are 0..7, and t is as above;
  - the rows come from a 20-value table the game builds (`$7E:D1B3`);
  - outside that domain the formula is the natural extension, not a measured one, like
    never-run code.

  If the chip ever needs checking there, chosen inputs fed to the idle chip are still
  outputs only (not done).
- **Next:**
  - the bench's chip is the stand-in, and it is exact on `$0A`;
  - the differential test's DSP-4 recogniser becomes a command table (command, bytes
    in, bytes out) for the commands in `src/dsp4.c`;
  - then `$81:F887` and `$83:E8AB` can be rows.

## DSP-4 commands in the harness; the road under each car (dsp4-1, 2026-11-16)

- **The differential test's DSP-4 recogniser is a command table:** command word,
  parameter bytes, result bytes, for the commands `src/dsp4.c` has:
  - `$00` (multiply, 4 in, 4 out; a write may skip a result, measured);
  - `$0A` (edges, 6 in, 8 out; results must be read).

  Any other DSP-4 traffic, or a command left unfinished, still fails the sample.
- **The bench's chip answers `$0A` with `dsp4_edges`:**
  - the bench bridge (`tools/bench/dsp4_bridge.cpp`) watches the clean-room chip's busy
    flag and command, as refbench's catalogue does (bench only, the chip's code
    unchanged);
  - for `$0A` it computes the edges with our C and serves those, while the chip still
    gets every byte, so it stays in step;
  - on the game's own runs nothing changes (the stand-in is exact on `$0A`). The fuzz
    can now use any rows: outside the measured domain both sides are `dsp4_edges`, and
    the rows check how the routine uses it, while refbench checks the chip;
  - this is the first command routed as the advisor planned. Each later one proved in
    refbench goes the same way.
- **`car_road_step` ($81:F887)**, the road under each car, runs once per car per race
  frame (351,807 entries on the 60k movie), from `$81:E812`'s table of jumps. Per frame:
  - the car's 32-bit distance moves on by speed × step (`$1E9E`), and the segment table
    moves it along (`func_81FBB1`);
  - it moves sideways by drift × (speed × step) >> 8;
  - it bounces off the road's edges (`$0A` between the rows at its place), turning the
    drift back inwards;
  - a cooldown and a boost run down;
  - its speed eases towards the target set by the car in front, by at most a limit ×
    step.

  Two quirks of the original are kept:
  - **The sideways sum takes in the segment step's carry.** The original has no CLC
    there; the carry is set only when a jump wrapped the position past `$FFFF`.
  - **The gap to the target loses its low borrow.** It is a 32-bit subtraction whose
    high half takes its borrow from an ADD.

  race1: live 3,483/3,483, fuzz 500/500, paths 201/201. The negative row (the rows' right
  widths one step wider while the C reads them) fails on the edge it writes.
- **Next:**
  - `$83:E8AB`, the other `$0A` user, has five never-run stretches. It needs static code
    with evidence and fuzz;
  - then `$11` (`$81:E631`), measured the same way.

## DSP-4 command $11 (dsp4-11, 2026-11-16)

- **Measured** like `$0A` (`refbench --dsp4-cmdlog 11`, first 50,000 TAS frames): 5,696
  runs, 90 distinct inputs, pure. Four words in, the edges `$16`, `$18`, `$1A`, `$1C` of
  `$81:E631`'s loop; one word out, read 16-bit at `$81:E756`. (The catalogue's "1 byte
  out" counted only up to the shadow's busy flag.)
- **The command is `$0A`'s inverse:** it turns four edges into a row word.
  - Each edge becomes a signed 4-bit width of 48: `floor(edge / 48)` for edges 0 and 2,
    `floor((edge - 1) / 48)` for edges 1 and 3.
  - Edge 0 goes in bits 15-12, ..., edge 3 in bits 3-0.
  - Plain floor division disagrees on 3 of the 90 inputs: 240, 288 and 336 as edge 1
    give 4, 5, 6, not 5, 6, 7.
  - The data cannot tell "minus 1 for edges 1 and 3" from "minus 1 for positive edges".
    In every run edges 0 and 2 are negative, and no edge 1 or 3 is a negative multiple
    of 48. The C takes the first; the domain note in `src/dsp4.h` says so.
- **`dsp4_row` (src/dsp4.c):** refbench checks it on every run: **5,696 runs, 0
  differ.**
- **In the harness:**
  - `$11` joins the difftest's command table (8 in, 2 out);
  - the bench bridge's routing is now a table (`$0A` → `dsp4_edges`, `$11` →
    `dsp4_row`), so the bench chip answers `$11` with our C too. The stand-in is exact on
    it, so the game's own runs are unchanged (race1: all 159 rows pass).
- **No row uses it yet.** Its only user, `$81:E631` (a track setup), runs 5 times on the
  60k movie, and all 5 are interrupted. So it needs a movie that reaches it without an
  interrupt, or the fuzz alone.

## The player's limits across the road (road-2, 2026-11-16)

- **`player_road_limits` ($83:E8AB)** is the other `$0A` user (536 entries on the 60k
  movie). It runs with D = `$0200`, so some of its variables are `$02xx` through the
  direct page and others `$01xx` absolute.
  - It takes the road's edges at the car's place from command `$0A` on the near row,
    then the far row, keeping the wider of the two.
  - It keeps clear of the car in front by `$20`, on the side the flag `$0244` bit 1
    says, and moves that flag over.
  - A pushed side sets both limits there; a bend shifts them.
  - Its middle (`$011C`) can be pinned to a limit by two side flags.
  - Finally it flags in `$1E9C` which way the car is more than `$18` off the middle.
- **Five stretches that never ran** decode as coherent code: the far row's two wider
  left edges, the pushed side, and the two side flags. They join `static_code.txt`, and
  the fuzz runs them all.
- 60k: live 179/179, fuzz 500/500, paths 216/216.
- **Its negative row (the bend flag's bit read wrong) is not reached on race1**, the
  selftest's movie, so the selftest does not exercise it. It was run on the 60k movie
  before this commit and failed, as it must.
- **Closed right after (neg-1):** `tools/verify_all.sh` now also runs every negative row
  over the 60k movie, in parallel with the verification itself (the same wall time). It
  fails if any deliberately wrong C passes or is never reached. All 19 fail over the full
  movie.

## Coverage explorer and the shop movie (explore-1, 2026-11-17)

- **The gap:**
  - 13,802 of the map's 69,676 code bytes (20%) ran only in the TAS (refbench), never
    on the bench's 60k movie. So no routine there could be verified;
  - most of it is bank `$80`, `$80:98xx-$80:B2xx`: the shop, money in BCD, and the
    screens between races;
  - the autopilot leaves every shop through EXIT ("Down x6, A").
- **`bench --explore N`** (with `--coverage`, `--record`, optional `--explore-known
  FILE`, `--explore-cands M`, `--explore-len L`, `--explore-seed S`):
  - from the end of the run, segment after segment, it tries M random button
    sequences (taps of one button with gaps) from the same saved state;
  - it keeps the one that starts the most instructions not seen yet, nor in the
    `--explore-known` coverage file;
  - it rejects a candidate whose WRAM stands still over its last 120 frames (a frozen
    game), and stops when every candidate freezes;
  - the chosen buttons go to `--record`, so the result is an ordinary movie.
- **Two trials:**
  - **From the main menu:** it always finds a 4-player name screen first, with 8,739
    code bytes the TAS never ran either, then the game freezes for good. It is probably
    a mode that waits for a multitap, which the bench does not have. Not used.
  - **From the shop** (`race1.txt` up to step 7,650; 40 segments of 600 steps, best of
    6 each): it reached 4,337 of the TAS-only bytes and 523 bytes no run had executed
    (bank `$80`, menu code).
- **`tools/bench/movies/explore_shop.txt`** keeps that run (32,228 steps). Every routine
  already in C passes on it.
  - `tools/verify_all.sh` runs it in parallel with the 60k movie, and
    `tools/merge_verified.py` joins the two results: verified if a movie verified it and
    none failed it.
  - Its coverage is merged into the map: **code 70,199 bytes** (+523), 80.19% of the
    ROM classified. The listing still rebuilds the ROM byte for byte.
- **Still TAS-only:** about 9,500 bytes (password, options, later championships). More
  starting points for the explorer (password screen, options), or longer runs, would
  bring them to the bench.

## The shop's leaves (shop-1, 2026-11-17)

- **The census over `explore_shop.txt`** lists 40 routines the 60k movie never ran, all
  without hardware access (`$80:9039`, `$80:9847`, `$80:A251`, ...). This unit ports
  twelve of its leaves, into a new `src/shop.c`:
  - `shop_part_level_2`..`_7` (`$80:9F23`, `$80:9F0A`-`$80:9F1E`): one of the car's six
    part levels (`$0392-$0397` + the car's offset), plus 1;
  - `shop_money_take` (`$80:993E`): the player's money (`$038E/$0390` + `$0194`) to
    `$52/$54`, `$56 = $FFFF`, `$76` bit 2;
  - `shop_flag_fb` (`$80:E4D0`, long): `$01FB = $12`;
  - `shop_panel_upload` (`$80:9DBC`): queues the panel's VRAM upload through
    `func_80B537` (a tail jump);
  - `shop_item_picture` (`$80:9BF2`): the current item's picture through
    `func_80B58D`, from the item table at `$81:A892` (list `$48`, row `$4C`);
  - `shop_item_text` (`$80:B096`): copies the current item's text block from bank `$89`
    to `$0100 + $01C4` (an MVN);
  - `shop_sprite_block` (`$80:A7E4`): four sprite entries of a 2x2 block.
- Every one passes all its live samples on the shop movie, from 34 to 3,713 each.
- **The item fuzz keeps to the table's rows:** lists 1-5 have five rows, and list 6 has
  only one.
  - List 6's rows 1-4 read past the table: text blocks of `$FF29`, `$7CA2`... bytes.
  - The first try fuzzed them. The MVN then copied up to 64 KB from `$81:0100` up, over
    the I/O registers, and kicked random DMAs. The shop movie's verification ran for
    over an hour before it was stopped.

## The shop's bars and centred numbers (shop-2, 2026-11-17)

- `sprite_number_centered` ($80:8635): `sprite_number`, then its sprites shifted to
  centre the number on x, by `((x & $FF) - (end & $FF)) >> 1`. Shop movie: live
  4,000/4,000, paths 29/29.
- `shop_bar_cap` ($80:AAF1): a bar's cap, four sprites in a column at the slot `$0174`.
  Live 992/992, paths 44/44.
- `shop_bar_column` ($80:A9BD): a bar's three-sprite column. Live 392/392, paths 40/40.
- The other bar routines (`$80:A403`, `$80:A4B8`, `$80:A87D`) have loops and a never-run
  stretch (`$80:A8A8`). They are next, with their callers `$80:A91E`, `$80:AA59` and
  `$80:A251`.

## The shop's level bars (shop-3, 2026-11-17)

- **`shop_bar_levels` ($80:A403):** six 2-sprite level marks, each at its bar entry's
  level: the stage `$013E / 2` less the entry's start, at most its length (4-byte
  entries at `$81:A892`, picked through `$81:A896` by `$017C`). Then six words of the
  tile table `$81:A882`, by the level (at most 7), stored at `$81:A876`'s offsets +
  `$01C4`. Shop movie: live 1,288/1,288, paths 92/92.
  - It reads the stage again for each mark, as the original does, because a tile word
    may land on it.
  - The fuzz keeps the tiles' destination (`$012E-$013F` + `$01C4`) off the direct
    page's own variables. With `$01C4` landing on itself, the next tiles went to the I/O
    registers (`$81:267B`).
- **`shop_bar_segments` ($80:A4B8):**
  - the bar of the stage past `$10`, drawn in segments of 6 as 3-sprite columns;
  - at `$1E` it steps back to a half segment at `$1B`, with one level;
  - when the stage fills the bar it returns at once: its `JMP $A578` lands on the RTS.

  Live 1,288/1,288, paths 90/90.
- **`shop_bar_segments_long` ($80:A87D):** the same up to `$24`, without the half
  segment. Live 196/196, paths 73/73.
  - Its five bytes after the `JMP $A91D` (`$80:A8A8`: `ADC #$0006 / STA $7E`) are dead,
    a leftover of the half-segment code. Nothing reaches them, so they are a hole in its
    extent, not static code.
- No register outputs: every caller reloads A, X and Y right after.

## The shop's double bar, gauge and price (shop-4, 2026-11-17)

- `shop_bar_double` ($80:A91E): a 2-sprite frame, then two `shop_bar_column` bars for
  the bar entry's first two parts. Shop movie: live 196/196, fuzz 500/500, paths 70/70.
- `shop_gauge` ($80:AA59): the sprite group at `$7E:3438` (`$7E:33DA` at stage `$10`),
  then `shop_bar_cap` caps: a partial one and full ones to `$38` for the stage past
  `$10`, and one for the stage past `$0C`. Live 433/433, paths 71/71.
- `shop_item_price` ($80:A26E): for stages `$10-$17`, two level words of `$7E:4DCA` to
  `$01A2/$01A4` + `$01C4`. Then the item's 32-bit BCD price as a number centred at
  `$5C2E` + `$01E7` (`sprite_number_centered`, sprite base `$3A00`), with `$0114` and
  `$0116` kept. Live 4,000/4,000, paths 49/49.

## The item views (shop-5, 2026-11-17)

- **The item screen's dispatch:** `$80:A251` reads the item's view address from
  `$81:A897` + the entry and its bar entry from `$81:A8A1`. It pushes `$A26C` and the
  view address minus 1, then RTS: the view runs and returns through the RTS at
  `$80:A26D`. The 26 items use seven views.
- **Ported, the three that call only routines already in C.** Each sets `$7A = $16` and
  `$7C` = the bar entry, draws, and ends by moving the stage `$013E` on (stopping at
  `$7FFF`):
  - `shop_item_view_a` ($80:A2CB): level marks, segments, two sprite groups. Shop movie:
    live 1,288/1,288, paths 48/48;
  - `shop_item_view_b` ($80:A831): a double bar, long segments, two groups. Live
    196/196, paths 35/35;
  - `shop_item_view_c` ($80:AA15): a group, the gauge, a group. Live 433/433, paths
    32/32.
- **Never ran:** the stop at `$7FFF` (`DEC $3E` after a `BPL`) is the same 2-byte tail
  in all seven views. The three ported ones join `static_code.txt`, and the fuzz sets the
  stage to `$7FFF` to run them.
- **The fuzz picks items of the view's own kind.** Another kind's bar entry sent `view_a`
  writing sprites over the direct page, and once (through `$01C4`) over the I/O
  registers.
- **Then (shop-6):**
  - `shop_item_view_d` ($80:A717): a group, a bar of 2x2 sprite blocks (the block under
    way at tile `$2316` + (stage / 2) & 7), and two groups. Live 1,076/1,076, paths
    101/101;
  - `shop_item_view_e` ($80:AB54): three groups, then six words of the wave tables at
    `$7E:4E1A-$7E:4EBA`, back and forth over 16 words by `$0140`, to `$0122-$012E` +
    `$01C4`; then `$0140` + 1. Live 664/664, paths 68/68.

  Their stops at `$7FFF` join `static_code.txt` too.
- **Left:** views `$80:A333` (calls `$80:A579`) and `$80:AEBE` (calls `$80:AF36`), then
  the dispatch itself.

## The shop's second item dispatch: icons and panel uploads (shop-7, 2026-11-17)

- **The dispatch:** `$80:9F40` (carry clear) and `$80:9C0B` (carry set) both jump through
  the table at `$81:A8A3`, one handler per list. Five handlers share one shape:
  - the part level is the byte at `$0393`-`$0397` + `$0194`, masked to 7 bits;
  - **carry clear:** the level's icon group goes to `shop_item_icon_draw` (`$80:A1E4`,
    reached by JMP);
  - **carry set:** the level's panel upload is queued through `func_80B537`, with Y saved
    around it.
- **Ported:**
  - `shop_item_icon_draw` ($80:A1E4): the group at (`$0142`, `$01E8`), palette `$0E00`
    below step 8, `$2E00` from it; then, while `$0144` is not negative, two words of
    `$7E:4EDA`/`$7E:4EFA` (and four of `$7E:4F1A`-`$7E:4F4A` below step 8) to the palette
    copy at `$01C4`. The step holds at bit 15 after 15, or after 7 unless `$0176` is
    negative. Live 4,000/4,000, paths 44/44;
  - `shop_item_icon_1`..`_5` ($80:9F5B list 1, $80:9F9A 2, $80:9FD9 3, $80:A057 4,
    $80:A018 5). Each runs the difftest with `alsoIn` = the icon's body; live 1,317,
    1,102, 200, 676 and 437 samples; paths 33/33 each.
- **Their V flag is live after the return**, so the adapters produce it:
  - carry set: V from `func_80B537`'s ADC;
  - carry clear: V from the group's last item ADC #6 (as on entry if the group is
    empty), or from `BIT $0176` at step 7.

  Caught by the fuzz (twice: first only the carry-set path was modelled).
- **No register outputs:** every caller reloads A, X and Y (`$80:9E49`, `$80:9B99`,
  `LDA #2`).
- **Not ported:** `$80:A121` (list 6) is never run by the shop movie, as list 6's view.

## The shop cursor and level marks (shop-8, 2026-11-17)

- **`shop_level_marks` ($80:9E49):** it runs after the two item dispatches at every call
  site.
  - `$01EE` eases one pixel a frame towards the cursor row (`$014A` * 16 + `$14`).
  - Two cursor sprites follow (tiles `$3892` and `$3890`).
  - Then one mark per level of list `$0148`, counted by the byte at `$036E` + list.
  - The part's current level comes from the list's getter in the table at `$80:9EEA`:
    the verified `shop_part_level_2..7` for lists 0-5, the plain bytes `$0398`/`$0399`
    for lists 6 and 7. Its mark takes tile + `$20`.
- **No register outputs:** `func_809DD0` follows at every call.
- **Results:** live 4,000/4,000, fuzz 500/500, paths 83/83.

## The shop panel's map buffer (shop-9, 2026-11-17)

- **`shop_panel_blank` ($80:B0C0):**
  - with DB = `$7F`, it sets 16 rows of 8 words (stride `$20`) at `$7F:0700` +
    `$015E` to `$FFFF`;
  - then `$014E` = 0 and the frame hook `$01EC` = `$B149`;
  - live 45/45, paths 32/32.
- **`shop_panel_clear` ($80:B10C):**
  - it re-arms the frame hook `$01EC` = `$B94F` on every pass of a busy loop until the
    upload queue is empty (`$018C` = `$018E`; the NMI drains it);
  - then it zeroes 9 rows of 9 words (stride `$40`) at `$7F:00C4` + `$015E`;
  - the loop's own branch goes back to the entry, so every pass counts as an entry
    (162,585), from 135 live samples;
  - its C waits with `platform_wait_vblank`, and the row is `waitsForInterrupts`;
  - the fuzz starts with the queue already empty, as it has no interrupts to replay;
  - paths 22/22.

## The sparks (roots-12, 2026-11-18)

- **`spark_step` ($80:EA77),** a particle system. There are 32 sparks at `$7F:1000` +
  (`$013E` + `$015E`), 4 bytes each: life, then position.
  - The index wraps within `$7F`, and the buffer bit `$8000` of `$015E` is kept
    (`AND #$807F`).
  - Each alive spark (bits 1-4 of its life / 2 not 0) loses a point and draws a sprite:
    tile `$81:D2C3` by its life, at its position + `$01E7`.
  - Then, while `$0186` - `$64` is below `$1E`, new sparks are laid at `$013E` (life
    `$1C`): two (positions `$81:D2E3` by that value, and `$808` less), and when `$0186`
    is a multiple of 4 two more (`$81:D323` with life `$1C`, `$81:D367` with life
    `$3C`).
  - The original tests `$0186` & 3 twice in a row; the C tests it once.
- **Results:** live 208/208, fuzz 500/500 (either buffer, the count around the window),
  paths 107/107.

## The race's message slots and a track entry's bounds (roots-11, 2026-11-18)

- **`race_message_request_0233` ($82:9BC9) and `race_message_request_02e6` ($82:9BE5):**
  the priority request of `car_message_request` for the race's two message slots,
  `$0233` and `$02E6`.
  - The three now share one C helper, `message_request(slot, message)`;
    `car_message_request` was retested with it (355/355).
  - `$82:9BE5`'s keep-the-current exit (`PLA`, `RTS` at `$82:9BFF`) is never run; it is
    static code, and the fuzz runs it.
  - Live 30/30 and 29/29, fuzz 500/500, paths 16/16 each.
- **`track_entry_bounds` ($81:E7DA):** the bounds of track entry `$0114`.
  - Its x (the byte at `$7E:8CB3` + entry, the top 8 bits of a signed 12-bit value) is
    kept in `$0128`.
  - `$0124` = `$012C` and `$0126` = `$012E` = x plus the pair at `$83:80D0` + 4 * (the
    byte at `$7E:8CB4` + entry).
  - Live 19/19, paths 27/27.

## Two title-screen pieces (roots-10, 2026-11-18)

- **`title_sprites_place` ($8A:824B):** two sprites placed at `$0174` / `$0176` less
  `$014A` / `$014C`:
  - tiles `$A4` and `$A6`;
  - x `$404D` + ..., y `$53` + ...;
  - the second sprite `$10` to the right of the first.

  Only while `$0162` is `$FFFF`; otherwise a lone `RTS` (`$8A:8252`, never run, static
  code; the fuzz runs it). Live 263/263, fuzz 500/500, paths 31/31.
- **`rotation_from_angle` ($8A:867C):** the Mode 7 rotation for the angle `$0134` (bits
  4-12):
  - `$0114` = `$011A` = cos, from `$81:CEA9`;
  - `$0116` = sin, from `$81:CDA9`;
  - `$0118` = -sin.

  Exit C = bit 2 of the angle (the third `LSR`). Live 263/263, paths 15/15.

## Track events that set a pattern (roots-9, 2026-11-18)

- **Five entries load a pattern into A and jump to a shared body at `$82:EB1A`:**
  `$82:EA81` (`$2007`), `$82:EA87` (`$2047`), `$82:EA8D` (`$2087`), `$82:EACE`
  (`$2107`), `$82:EAD4` (`$C2E7`). In C they are one function, `track_event_set(pattern,
  y)`; DB = `$7E`.
  - The pattern goes to `$1F0A`.
  - `func_82EC09` (verified), for entry Y, gives a position, written to `$1F0E` and, at
    `$011A`, `$7E:3C6C` / `$3CAC`.
  - That position + `$011E` goes to `$1F10` and `$3F6C` / `$3FAC`.
  - Bit 0 of `$3D6C` is set, and `$3DAC` is set to the same word.
- **Each entry is a row,** `alsoIn` the body. Exit: A = the `$3D6C` word, X = `$011A`, Y
  kept, C and V of the `ADC`.
- **Results:**
  - `$2007` live 7/7, `$2047` 7/7, `$C2E7` 8/8; fuzz 500/500; paths 2/2 each.
  - `$82:EA8D` and `$82:EACE` no movie reaches: their rows stay unreached (their body is
    the verified one).

## A right-aligned number, a car's message, a track event cleared (roots-8, 2026-11-18)

- **`sprite_number_point_right` ($80:8667):** `sprite_number_point` drawn from x, then
  every sprite it added moves by (x - the x after it), low bytes, kept in `$0114`, so
  the number ends at x. Exit: A = `$0114`'s high byte : the last moved x (8-bit `ADC`, V),
  X = 0, Y = the slot after (Z and C set by the `CPY`). Live 1,330/1,330, paths 28/28.
- **`car_message_request` ($83:FEBD),** with the car's direct page as a parameter (D =
  `$0200`): the message in A replaces dp+`$33` when its priority (`$83:A0A4` + 2 *
  message) is at least the current one's. Exit: A kept, X = 2 * message, C of the
  priorities' `CMP`. Live 355/355, paths 16/16.
- **`track_event_clear` ($82:EADA, DB = `$7E`):** a track event's state cleared:
  - `$1F0E`, `$1F10`, and at `$011A` the words at `$7E:3C6C`, `$3CAC`, `$3F6C`, `$3FAC`;
  - bit 0 of `$3D6C` set, and `$3DAC` set to that word.

  Live 21/21, paths 13/13.
- **Next:** the events that share `$82:EB1A` (`$82:EA81`, `$82:EA87`, `$82:EAD4`, ...),
  each loading a pattern into A and jumping there.

## Two HDMA window shapes (roots-7, 2026-11-18)

- **`window_shape_a` ($80:DB7D) and `window_shape_b` ($80:DBCB)** build a window table
  for HDMA at `$7F:X` (DB = `$7F` within, `$81` after their last `PLB`). They differ only
  in their widths table (`$81:C872` or `$81:C9AA`), and they share a tail, `$80:DC19`,
  reached by `JMP` (`alsoIn`).
  - `func_80DC5A` (verified) writes the head from A.
  - From `$33` bytes past its end come line pairs, one per row: right edge = width / 2 +
    `$012E` - `$0120`, left edge = right - width + 1 (bytes). The row comes from
    `$81:C852` + (`$011E` & ~1), and there are `$011E` / 2 + `$0C` of them.
  - The rest of `$1A` lines are empty (`$00FF`), then the end (`$00FF`, 0).
  - The `$19` lines above the start mirror those below it, one more when `$011E` is odd.
- **Exit:** the mirror's last copy (A = the word, X its target, Y its source); N set and
  Z clear by the final `PLB`; C = bit 0 of `$011E` (an `LSR`); V of the last line's
  8-bit `SBC`. To know where the lines start, the adapter runs `func_80DC5A` once more
  before the C, which writes the same bytes again.
- **Results:** live 665/665 (60k), 461/461 (shop, race1), fuzz 500/500, paths 40/40 each.

## Three more title-zoom steps (roots-6, 2026-11-18)

- **Three steps of the title sequence's zoom (D = `$0100`).** Each ends with
  `inverse_scale` and `func_80F745`, whose result it returns, with C clear.
  - **`title_zoom_pan` ($80:F2A0):**
    - `$0130` counts up to `$0F`;
    - once the high bytes of `$013C` and `$013A` are `$C0` and `$1C`, `$013E` counts on
      and it returns at once (A = `$1C00`);
    - otherwise `$013C` moves by `$0140`, which turns to -`$800` at `$F000` and to
      `$800` below `$1000`, and `$013A` falls by `$80`.
  - **`title_zoom_cycle` ($80:F2EE):** `$0130` counts on, from `$1F` back to `$0F`.
  - **`title_zoom_drop` ($80:F595):**
    - `$017E` falls by 1 to -`$48`, and there `$013E` counts on;
    - two rows of 12 tiles at `$7F:0214` / `$7F:0254` are set to `$1001` / `$1002`.
- **Their exit V is live,** and it is the one `func_80F745` leaves. The adapters reuse
  `v_after_inverse_scale` and `v_after_func_80F745` (written for `func_80F3C4`), which
  the first adapters left out.
- **Results:** live 85/85, 55/55, 34/34; fuzz 500/500; paths 36/36, 10/10, 19/19.

## The title zoom's step and two small ones (roots-5, 2026-11-18)

- **`mode7_zoom_step` ($80:F226):** a zoom step on the title screen.
  - It calls `mode7_zoom_view`.
  - D+`$40` counts frames; from `$320` on, D+`$7C` falls by `$10` a frame, to 0.
  - Then `inverse_scale` and `func_80F745`, whose outputs it returns (C clear).
  - Its fuzz first wrote to `$81:4500` (ROM) on the shop movie: the fuzz starts from the
    movie's final state, where the line tables' buffer words (`$0134`/`$0136`) are not
    the title's. It now prepares them as `func_80F745`'s own fuzz does.
  - Live 285/285, fuzz 500/500, paths 15/15.
- **`far_7fff_4` ($81:E7CE):** D+`$24`, `$26`, `$2C` and `$2E` = `$7FFF`. Live 720/720.
- **`empty_80F225`, a lone RTS:** it is called (648 entries, 216 samples on each movie,
  the selftest's included).

## A number with a point, an 8-sprite bar, two CLCs (roots-4, 2026-11-18)

- **`sprite_number_point` ($80:855A):** a 4-digit BCD value (kept in `$011A`, read a
  digit at a time by `take_nibble16`) as sprites from slot `$0174` at x `$0116`.
  - Tiles `$34` + digit + `$0114`, placed by their widths at `$80:85A6` + tile.
  - One leading zero is dropped, and a point (tile `$41`) goes before the last two
    digits.
  - Exit: A = 0 (the fifth digit read, the value all shifted out), X = 0, Y = the x
    after the number, C clear.
  - Live 1,330/1,330, fuzz 500/500, paths 56/56.
- **`sprite_bar8` ($80:C475):** a bar of 8 sprites at slot X, `$0114` wide, centred on
  `$0118`.
  - Four left pieces 8 apart from the left edge; four right pieces from the right edge
    less `$20` (8, 8, 6 apart).
  - The right edge's `ADC` takes the carry of the width's `LSR`.
  - Tiles `$011A` + `$150`, then + `$151`, the last mirrored (`$150` | `$4000`).
  - Returns X + `$20`. Live 2,092/2,092, paths 57/57.
- **`clc_82DA46` and `clc_83FA32`:** CLC, RTS (6,929 and 2,669 entries).
  - Checked on the selftest's race1 movie too, after `$82:EA80`'s lesson: there the
    first has samples and the second is unreached, which is no failure.

## A faster verify_all (tools-1, 2026-11-18)

- **The owner asked whether the 40-minute gate was too slow; it was.**
  - `tools/verify_all.sh` ran three processes, and the 60k movie's single run took
    about 30 minutes alone on one of 8 cores.
- **`bench --difftest-shard K/N`** tests slice K (from 0) of N contiguous slices of the
  registry.
  - `verify_all.sh` runs 5 slices of the 60k movie (`TG3000_SHARDS`), 2 of the shop
    movie and the negative rows: 8 processes.
  - Each slice's results are joined in order, so `map/verified.txt` keeps the registry's
    order; `tools/merge_verified.py` joins the movies as before.
  - A slice still runs its whole movie (about 9 minutes of it is the movie itself).
- **Result:** 14 minutes instead of about 35. Every row's status matches the single run.
  Some live counts rose (`8386FB` 3,858 -> 4,000, `80DA5F` 612 -> 665): a waiting row
  nested in another one's sample is sampled when the outer row runs in another slice.

## The race's message line (roots-3, 2026-11-18)

- **`race_message_update` ($82:B11F; DB = `$7E` within, `$83` after its two `PLB`s).**
  - A new message `$0116` replaces the shown one `$0114` if its priority (`$83:A0A4` +
    2 * message) is at least the shown one's. It is drawn with `text_centred_line`, and
    its time `$011A` (`$83:A142`) and `$011C` (`$83:A1E0`) are set.
  - Otherwise the shown message's time counts down by `$1E9E`; when it runs out the
    line is blanked and `$0114` = `$0116` = 0.
  - Exit: N set and Z clear (the `PLB` of `$83`).
  - Live 4,000/4,000, fuzz 500/500, paths 50/50.
- **`func_82B769` ($82:B769):** A as given, or, when the `$82:DC68` entry for `$047B` is
  negative and Y is not 0, `$019A` - 1 for a positive Y and 0 for a negative one; then
  `func_82B78D` and `func_82B7B0` (whose outputs it returns).
  - Its fuzz first left 10 paths unvisited: it never drew one of the table's negative
    entries (17, 27, 47). It now does half the time.
  - Live 4,000/4,000, paths 18/18.
- **`empty_80B94F`,** a lone RTS: the shop's idle frame hook (12,243 entries).
- **`$82:EA80`, another lone RTS, has no row.** The race1 movie of the selftest reaches it
  38 times, but by a jump or a branch (a shared tail), never by a call. That gave
  entries and no samples, and the selftest failed. It needs no C of its own.

## The scroll commit and a car's lap (roots-2, 2026-11-18)

- **`race_scroll_commit` ($82:ECF1):** `$1EEE` and `$1EF0` cleared; `$0200`/`$0202` =
  `$0204`/`$0206`, which also add into the 32-bit `$1EF6:$1EF4` (the high word's `ADC`
  takes the low words' carry); `$020A` and `$020C` cleared. A (the new `$1EF6`) is an
  output. Live 4,000/4,000, paths 14/14.
- **`car_lap_check` ($83:F9D1),** a car's lap, with its direct page as a parameter (`$0200`
  for the player, the only D the movies show):
  - once the frame counter `$019A` has reached dp+`$26`, the word dp+`$35` (at an odd
    address) counts on;
  - when it equals `$1F06` the car has finished:
    - bit 0 of dp+`$7C` if dp+`$31` is set, and bit 2 of `$7E:FADE` + `$1EBA`;
    - dp+`$44` bits 0 and 2 set and 3 cleared; dp+`$25` = 0, dp+`$26` = `$019A`;
    - its place `$1ECA` written at `$0446` + `$1F08` (which counts on), and dp+`$33` =
      `$12` + that place;
  - otherwise dp+`$33` = `$1F06` - dp+`$35` + 9.
  - Live 4,000/4,000, fuzz 500/500, paths 42/42.

## The start countdown (roots-1, 2026-11-18)

- **`race_countdown` ($82:AB9D, DB = `$83`):** the start countdown.
  - While `$1ECE` is positive it falls by `$1E9E` (to 0 at least).
  - As it passes `$D2`, `$8C` and `$46`, the light becomes 2, 3, 4. Past 2 it becomes 5,
    GO: `$1ECE` = 1, bit 3 of `$0244`, `$02F7`, `$02A6` and `$0359` set, and the sound's
    volume `$22` instead of `$1E`.
  - The light is written to `$0233`, `$02E6`, `$0295`, `$0348`. A light that is not 0
    is announced through `func_82AD12` (`$FF00` | volume, `$2000`).
  - A negative `$1ECE` (unless `$047C` is negative) sets `$0233` = `$3D` and `$02E6` =
    `$3E`, or `$45` when bits 4-7 of `$03DC` are not 3.
  - "Passing" a mark is the original's pair of signed compares (old count at least the
    mark, new count below it).
  - Live 4,000/4,000, fuzz 500/500 (counts around each mark), paths 74/74.
- **`race_clear_0242_0208` ($82:ECEA):** `$0242` and `$0208` cleared. Live 4,000/4,000.
- **Next roots:** `$82:8F8F` (it multiplies on the DSP-4 and draws random numbers), then
  `$83:E2D7`, `$85:B1FF`, `$83:F9D1`, ...

## Roots, a faster iteration, a shop list label (leaves-20, 2026-11-18)

- **`tools/portable.py` understates what can be ported.** It reads the static listing.
  `tools/roots.py` reads the measured calls (`map/census.txt`, indirect jumps included).
  - `python3 -I tools/roots.py 1000` lists 140 roots (unported, no unported callee).
  - 100 of them use no hardware, and 95 of those a bench movie reaches.
  - The units now come from that list.
- **`bench --difftest-only name1,name2`** tests only the rows named, to iterate on a few
  (9 minutes over the 60k movie instead of about 35).
  - Its live counts can differ: another row no longer owns the interrupts it waits
    through.
  - `tools/verify_all.sh` always runs every row.
- **`shop_list_label` ($80:B2F2),** the root with the most entries (20,125):
  - list x / 2's name (`$80:B371` + x, a string in bank `$80`) goes through
    `sprite_text_centered` at row `$0122` (+ `$01E7`), while the row is within
    `$1000`-`$57FF` (signed);
  - the row moves on by `$800` and x by 2;
  - at x = `$20` that is all; a list whose count (`$036E` + x / 2) is 0 ends its caller's
    run: `$01F0` = x / 2, x = `$1E`;
  - A, X, Y and the flags are outputs, by exit.
  - Live 4,000/4,000 (shop), fuzz 500/500, paths 34/34.

## The race's centred messages (leaves-19, 2026-11-18)

- **`text_centred_line` ($82:B18B, DB = `$7E`):** a centred line of 13 two-row characters
  at `$7E:$0118` (top row) and + `$1A` (bottom).
  - Both rows are filled with `$83:A5A4`'s blank.
  - The string is the zero-ended one at `$83:`(the word at `$83:A006` + `$013C`): "GO!",
    "1ST".."20TH", "FINAL LAP", "n LAPS LEFT", "RECHARGE!", "PAUSED", "+4000 CREDITS"...
  - Its characters are counted (bytes with bit 7 set are not), and the line starts
    (13 - count) / 2 cells in.
  - Each character's two tiles come from `$83:A504` / `$83:A554`, ORed with the palette
    `$013A`; a byte with bit 7 set changes that palette (its low 3 bits << 10).
- **The table has 79 entries,** and six of them (`$7A`-`$84`) point at `$83:037E`-`$040B`,
  WRAM through bank `$83`'s mirror (strings the game writes, such as names).
  - The first C read every string from ROM; a live sample on one of those entries
    failed, so the C now reads through the bus.
  - The fuzz draws all 79 entries, half of the time a WRAM one holding a random string
    with letters, digits, `'.'` and palette bytes. That reaches the two never-run
    branches (the palette byte, `'.'` as tile `$25`), which are static code.
- **Results:** live 71/71, fuzz 500/500, paths 77/77.

## A text line (leaves-18, 2026-11-18)

- **`text_line_put` ($8A:9B85):**
  - it copies the zero-ended string at Y:X (24-bit, kept in `$0162`/`$0164`), its zero
    included, into the map at `$7F:7918` + `$0154`;
  - then `func_8A9BAA` (verified) moves `$0154` to the next line and counts `$0156` down;
  - the loop's 8-bit `INC $54` is undone by restoring `$0154` from the stack before the
    next line.
- **Exit:** A kept (its `PLA`), X = the old `$0154`, Y = the length, C and V of
  `func_8A9BAA`'s `ADC`.
- **Results:** live 2/2 (60k), fuzz 500/500 (strings of 0-30 bytes in WRAM), paths 20/20.
- **Still in asm:**
  - `$82:F2B3` (it blocks 7): it dispatches track events through the table at
    `$82:F374`, whose handlers are not ported;
  - `$8A:9AF1`, the caller of `text_line_put`, has a never-run 48-byte branch.

## The `$0D7E` timer and rng_next's third copy (leaves-17, 2026-11-18)

- **`timer_0d7e_tick` ($8A:9F76),** parked before because its never-run branch calls
  `$8A:AE9B`, itself never run. That branch and `$8A:AE9B` are now decoded and listed as
  static code:
  - `$0D7E` counts down;
  - at 0: `$03DC` = `$2000` | a random 0-2 (the low two bits of a random word, 3 taken
    as 2), and `$016C` + 1;
  - `$8A:AE9B` is a third copy of `rng_next` (the same left-shifting LFSR on `$0110` /
    `$0112`); the C calls `rng_next`.
- **Its fuzz sets the count to 1 a third of the time,** so the original's never-run
  branch, and `$8A:AE9B` with it, runs and is compared with the C (it has no row of its
  own).
  - Exit when counting: A kept, N and Z of the `DEC`.
  - Exit at 0: A = `$03DC` & 7, C of its `CMP #2`, N and Z of the `INC`.
- **Results:** live 88/88 on both movies, fuzz 500/500, paths 17/17. It was the top of
  `tools/roots.py`, ahead of 10 routines.

## The colour gradient (leaves-16, 2026-11-18)

- **`hdma_gradient_build` ($8A:8FA4, long):** a 256-line colour table for HDMA, from the
  colours `$0134` to `$0136` (lines 0-127) and `$0136` to `$0138` (128-255).
  - Each channel (red, green, blue) keeps five words at `$1516` + 10 * channel: its value
    before and now (8.8), two steps (2 * the difference of its next value, so 128 lines
    cover it), and a dither pattern.
  - When a step crosses a whole value, the pattern (`$013A`, then `$013C`) chooses, a
    line at a time, the new value or the one before, and shifts.
  - Each line writes (0, colour) at `[$0168]` + `$016C`.
  - The 276-instruction original is three copies of one channel's code; the C is a loop
    over the channels.
- **Fuzz only.** The 60k movie enters it 12 times, all of them inside a routine that
  records the interrupts it waits through (so they go unsampled) or interrupted.
  - The fuzz draws any three colours and patterns, with the table in bank `$7F`: 500/500,
    paths 276/276.
  - Exit C and V are blue's last compare and `ADC`, from its final value and step.
- **`flag_03dc_from_52` ($8A:9F64):** bit 12 of `$03DC` is set while `$0152` is not 0,
  cleared otherwise. The setting branch is never run (static code). Live 1/1, fuzz
  500/500, paths 8/8.
- **`$8A:85AF` waits:** its never-run branches jump to `$8A:85CC` and `$8A:85D3`, into
  bytes not yet mapped.

## The steering reset and the HDMA settle (leaves-15, 2026-11-18)

- **`steer_state_init` ($80:D6A0):**
  - `$0128`, `$0134`, `$0136`, `$0124`, `$011E`, `$012C`, `$0138`, `$013A` cleared;
  - `$0130` = `$28`, `$0132` = `$4B0`, `$013C` = `$50`;
  - `$012A` = `$012E` = `$0804`;
  - `$0122` = `$0120` = (`$0804` - `$80`, at least 0, capped at `$100`) less its
    remainder by 3, through the CPU divider (`$4204`/`$4206`/`$4216`), which the C
    computes natively.

  The cap (`LDA #$0100`) is never run and is static code. A shop fuzz that hardly ever
  drew `$0804` below `$80` left the clamp's path unvisited; a third of its cases now
  are. Live 2/2 (60k), 1/1 (shop), fuzz 500/500, paths 39/39.
- **`hdma_table_0d00_settle` ($8A:9C48):** `hdma_table_0d00_step`, then every entry byte
  of the `$0D01` table steps down by one unless it is `$FF` or `$E0`. Exit C is the last
  entry's compare, which the adapter derives allowing for the step having just lowered
  that entry. Live 208/208, paths 16/16.

## The race order (leaves-14, 2026-11-18)

- **`race_order_sort` ($85:A066, long),** with DB = `$7E` from its `PEA $837E` / `PLB`:
  - the 20 cars' word indices at `$7E:4CFD` are bubble sorted by their 48-bit totals
    (`$7E:6991` low, `$7E:69B9`, `$7E:69E1` high), highest first; `CMP`/`SBC`/`SBC`
    keep the order when the first is at least the second;
  - each car without bit 2 of `$7E:6A09` is listed, number = index / 2, at `$0446` +
    `$7E:6A31`, which counts on;
  - the second `PLB` leaves DB = `$83`, not the caller's;
  - `$03A8` and `$03D7` are set to the places of cars 0 and 1 among the first ten of
    `$0446`, or `$FFFF`.
- **Exit:**
  - A = `$03A8` | `$03D7`;
  - N and Z come from that `ORA` when both cars were found, from the `DEY` that reached
    0 otherwise (a live sample showed it);
  - C comes from the listing loop's `CPY`;
  - V comes from the sort's last `SBC`. The adapter replays the sort to know its last
    pair.
- **Results:** live 1/1 (the 60k movie runs it four times; three were interrupted), fuzz
  500/500 (totals equal or apart, cars out of the order), paths 84/84.

## The lights (leaves-13, 2026-11-18)

- **`race_lights_draw` ($80:DAED):** twelve lights as sprites from slot `$0174`, light n at
  the place of `$81:CB14` + 2n (DB = `$81`: `$81:CB2C` is 0, 4, 8, ...).
  - **Lit (below `$0368`):** their own sprite from `$81:CB44`/`$81:CB46`, high table 0
    for the first and 1 for the rest.
  - **Under way (`$0368`):**
    - lit while `$013C` is 0;
    - with `$0126` set, off (tile `$25C3`);
    - otherwise `$013C` counts down (at 0 `func_80D6F3` steers to the light's place,
      `$0114` kept), and the light blinks by bit 4 of the count.
  - **The rest:** unlit (tile `$25C2`).
- **Results:** live 612/612 (60k) and 461/461 (shop), fuzz 500/500, paths 68/68. The fuzz
  starts from `func_80D6F3`'s own fuzz and draws the count lit, the blink count and
  `$0126`.

## The password's bits and an HDMA step (leaves-12, 2026-11-17)

- **`password_pack` ($8A:BB99)** builds the password's bits before `password_encode`:
  - it clears the 256-bit number (`func_8ABC7F`) and zeroes `$0148`, `$0192` and `$0184`;
  - for each field of the table at `$8A:CF61` (6 bytes: a 24-bit address, a shift, a
    width; a zero word ends it), it reads the word there, shifts it right (`lsr_n`),
    XORs the width's mask (`$8A:BC3D`) and pushes it in with `func_8ABBF4`;
  - it returns the bit count;
  - its exit C and V are those of the last `$0192 += $0DCA` (`func_8ABC32`).
- **That row is checked by fuzz only.** The 60k movie runs it once, and an NMI lands
  inside, so no live sample is compared. The fuzz randomises every field the table names
  that lies in WRAM: 500/500, paths 31/31.
- **`hdma_table_0d00_step` ($8A:9C27):** every fourth call (`$016C` counts), the next word
  of the HDMA table at `$0D01` (`$016E`, up to `$2A`) steps down by one unless it is `$E0`.
  Live 208/208, paths 16/16.

## Pieces of the buy flow (shop-16, 2026-11-17)

- **The buy dispatch** `$80:9039` (table `$81:A89F`, a handler per list: `$80:9053`,
  `$80:9100`, ...) waits. The shop movie arrives with 0 credits and never buys, so the
  handlers' buying half (from `$80:909A`: `$80:99D1`, `$80:9957`, `$80:9ABB`) runs only
  in the TAS. This unit takes the pieces the movie reaches:
  - `shop_item_show_icon` ($80:9832): `$0176` bit 15 set, the icon restarted (`$0144`
    bit 15 clear), and the screen runs (`wait_frames`, `shop_item_redraw`) until the
    icon has finished. Live 25/25, paths 9/9;
  - `shop_item_affordable` ($80:98C4): the money `$0154`:`$0152` against the item's
    price (`$81:A88E`/`A890` + its word at `$81:A89B`);
  - `shop_item_icon_settle` ($80:9870): the icon's colours step back down (`$0144` to
    `$8007`), the screen running; then `$0176` bit 15 clears. Live 74/74.
- **How `shop_item_affordable` compares:** `CMP` of the low words, then `SBC` of the
  high words in decimal mode. For BCD amounts that carry is the 32-bit comparison.
  - If the money is short, the price flashes for 16 frames (two colours from
    `$7E:4DDA`/`$7E:4DFA`, the screen running).
  - Its fuzz has no frames to replay, so it draws amounts that can pay; the live samples
    are all short.
  - When the money is enough, the flags N, V and Z of the decimal `SBC` are live. The
    adapter computes them as the bench's CPU does (`sbcDecimal16`, LakeSnes's algorithm).
  - Live 34/34, fuzz 500/500, paths 37/37.
- **Never-run code left out of `shop_item_icon_settle`'s paths** (`holes`):
  - two branches, `$80:9892` and `$80:98B5`, run only for an icon below step 8;
  - no known run reaches them, the TAS included;
  - a waiting routine has no fuzz;
  - so that C is unchecked, and is noted in the row.

## Leaving the item list (shop-15, 2026-11-17)

- **`shop_item_down` ($80:8FA1) and `shop_item_up` ($80:8FD4):** the player moves along
  the item list.
  - The next item slides in: down by `shop_item_slide_list`, up by `shop_item_slide_row`.
  - It is sound `$11`; up plays it even at the top.
  - Past either end the list is left:
    - the icon stops (`$0144` bit 15);
    - the panel blanks;
    - once `$014E` has passed 4 (the frame hook counts it) the panel clears;
    - `PLA`, `RTS` then drop the routine's own return address and return from its caller.
- **The difftest learns that exit:** a row with `mayLeaveCaller` also ends its sample at
  the caller's own return (the word above its return address, SP + 4). The adapter says
  which way the C went (`out->leftCaller`, the C's `true`), and it is compared like a
  register. The flags then are those of the `PLA` (N, Z) and C of the panel's `CMP`.
- **Checked against a wrong C:** an adapter that always reported `false` failed in a
  separate worktree (it cannot be a negative row: those must fail over the 60k movie,
  which never opens the shop).
- **Results:** live 50/50 and 53/53, the leaving paths among them; paths 23/23 and 18/18;
  no fuzz (waits).

## The item slides (shop-14, 2026-11-17)

- **The item changes by sliding, six pixels a frame.** Each frame:
  - `wait_frames(1)`;
  - with the upload queue empty, a typewriter step and the panel's upload;
  - the item drawn at (`$0114`, `$0116`): view, icon, level marks, labels, sprite end.
- **Between the two halves** it waits for the queue to drain, comparing against the slot
  it read once (`LDA $8C` outside a `CMP $8E` / `BNE` loop the NMI ends). Then come a
  typewriter step and the panel, and the new item (row `$014C`) takes over: stage,
  spin and timer cleared, its panel pictures and its text.
- **The routines:**
  - `shop_item_slide_row` ($80:9AC8): up and out (y `$18` down to -`$46`), then in from
    the left (x -`$3E` up to `$10`). Live 34/34, paths 61/61;
  - `shop_item_slide_list` ($80:9B5D): out to the left (x `$10` down to -`$3E`), then
    falls into `shop_item_slide_list_in` (`alsoIn`). Live 47/47, paths 24/24;
  - `shop_item_slide_list_in` ($80:9B99): the new item, down from the top (y -`$36` to
    `$18`). Live 57/57, paths 37/37.
- **They wait for frames,** so their rows are `waitsForInterrupts` with no fuzz: the live
  samples replay the movie's interrupts.
- **Exit:** the last `CMP` matched (N clear, Z and C set), and those flags are live.

## The item screen's frame and panel (shop-13, 2026-11-17)

- **`shop_item_dispatch_panel` ($80:9C28):** the item's handler from the table at
  `$81:A899` queues its panel pictures through `func_80B537` (priority `$7F`): VRAM
  `$015C` + an offset fixed per handler, source `$015E` + `$0900` + the picture's offset,
  and its size.
  - The pictures' (offset, size) pairs are at `$81:A892` + the item's word at `$81:A89B`
    (and + 4 for the second).
  - Lists 1-5 queue two pictures; lists 0 and 6-8 one.
  - The seven handlers differ only in those numbers, so the C is a table in a switch.
  - Exit: `PLX` of the entry (X, N, Z); C and V of the last queue `ADC`.
  - Live 103/103, paths 21/21.
- **`shop_item_redraw` ($80:9847),** the item screen's frame:
  - with the upload queue empty, a typewriter step (`text_type_step`) and the panel's
    upload;
  - then the price at (16, 24), the item's view, its icon, the level marks, the labels
    (`func_809DD0`) and the sprite table's end.

  Every callee was verified. Live 964/964, paths 16/16.
- **Its fuzz found a real error in `shop_level_marks`** (verified in shop-8): list 8's level
  getter (`$80:9F28`) reads `$039A`, while the C read `$0399` as for list 7 (the getter
  table at `$80:9EEA` has ten entries, list 9 `$039C`). Its own fuzz drew lists 0-7 only.
  It now draws 0-9.
- **Two fuzz lessons:**
  - A random queue base (`$018A`) put the panel's second picture at `$1900` + a slot past
    WRAM (a ROM write). The game's base is a multiple of `$40`, and so is the fuzz's.
  - Random part levels made an icon's group pointer land in data with no end, so the C
    walked forever and the first run hit its time limit. The frame's fuzz keeps the
    levels within their icon tables.

## The typewriter (text-1, 2026-11-17)

- **`text_type_step` ($80:B59D)** prints the shop's texts.
  - **Pacing:** the speed `$01E5` adds to `$01C6`, and each whole `$100` spent is one
    step of the text at the 24-bit pointer `$01CC`, into the map at `$7F:($01C8 + $01CA)`.
  - **Letters appear in stages:** a queue of map words (`$01CF`-`$01DD`) adds `$4C` to
    each on every step (`$31C` at the fourth) until the letter's own tile, kept in
    `$01DF`, lands. `func_80B7BB` (verified) takes a reused word out of the queue.
  - **Letter codes:** a space; 13 marks by the table at `$80:B7D9`; digits and other codes
    below `$41` + 5, `$41`-`$60` - `$26`, from `$61` - `$60`.
  - **Control codes `$80`-`$86`,** through the table at `$80:B6BC`:
    - `$80`: next line;
    - `$81`: column 0;
    - `$82`: jump to the address that follows;
    - `$83`: blink a cursor (`$01E1`, a 16-step cycle);
    - `$84`: wait the count that follows (`$01E3`), blinking;
    - `$85`: clear the line and back up one, until line 0;
    - `$86`: set bit 0 of `$01F8`.

    `$81` and `$82` are never run (static code); the fuzz writes texts with them.
- **The row:**
  - its extent skips the code table (`alsoIn` for the handlers);
  - two `m8At` addresses (`$80:B645`, `$80:B65E`) keep the linear decode in 8-bit A
    where the code runs so after a `REP` on another path; without them it listed 11
    instruction starts that never run;
  - the fuzz sometimes puts the text at the end of bank `$7E`, so the never-run bank
    carry (`INC $CE`) runs.
- **That fuzz found a real error:** `LDA [$CC],Y` (code `$84`'s count) adds Y to the full
  24-bit pointer and crosses into the next bank, while the first C wrapped inside the
  bank.
- **Results:** live 4,000/4,000 (shop) and 1,310/1,310 (60k), fuzz 500/500, paths
  130/130.

## The item screen's view dispatch, all ten views (shop-12, 2026-11-17)

- **`shop_item_dispatch_view` ($80:A251):** the current item's view from the table at
  `$81:A897`, called with its bar entry (`$81:A8A1`). The views return to its own `RTS`
  at `$80:A26D`, so its extent runs to `$80:A26E`.
- **Ten views in all:** list 0 `$80:ABF6`; list 1 `$80:A2CB` (row 4 `$80:A333`); lists 2-5
  `$80:A717`, `$80:A831`, `$80:AB54`, `$80:AA15`; lists 6-8 `$80:AEBE`, `$80:AE4E`,
  `$80:AC37`.
- **The four views no movie runs**, and what they call, are new C:
  - `shop_item_view_f` ($80:A333), `shop_ring` ($80:A579) and its pieces `$80:A69F` and
    `$80:A6DB`;
  - `shop_item_view_h` ($80:AEBE) with `shop_level_blocks` ($80:AF36), a row of 2x2
    blocks that fills with the stage;
  - `shop_item_view_i` ($80:AE4E);
  - `shop_item_view_j` ($80:AC37) with `shop_gauge_lines` ($80:ACB6) and
    `shop_gauge_line` ($80:ADD9). Each gauge line's starting column comes from the PPU's
    Mode 7 product (`$0146` & `$7FFF` times a speed byte), and the C sets matrix A and B
    as the original does.
- **They are checked through the dispatch's fuzz,** which draws all nine lists (list 1
  row 4 one case in about 45). Their own rows stay unreached until a movie opens those
  items, and they count as verified only then.
- **The fuzz caught a real error:** the ring parked earlier had been read from `$80:A5F0`
  (a four-piece ring), while `$80:A333` calls `$80:A579`, which draws two pieces. 15 of
  500 cases differed in the sprite slot `$0174`.
- **Static code:** the never-run tails of these views are decoded and listed:
  - the spins' wraps (`SBC #$1E00`, `SBC #$0F00`);
  - the stops at `$7FFF`;
  - the gauge's `ORA #$FF00`;
  - `shop_level_blocks`' all-full branch (`$80:AF5E-AF75`).

  `map/static_code.txt`'s end is exclusive: `shop_list_locked`'s range now takes its
  `RTS` too (`$80:B364-B366`).
- **Results:** live 4,000/4,000, fuzz 500/500, paths 19/19. The parked patch
  (`docs/parked/`) is gone, its C now in `src/shop.c`.

## List 0's view and four more leaves (leaves-11, 2026-11-17)

- **The item screen has a view per list, nine in all** (`$81:A897`):
  - list 0 `$80:ABF6`;
  - lists 1-5 as before (list 1 row 4 `$80:A333`);
  - list 6 `$80:AEBE` (+`$80:AF36`), list 7 `$80:AE4E`, list 8 `$80:AC37` (+`$80:ACB6`,
    which draws through `$80:ADD9` in bank `$7E`).

  The shop movie runs only list 0's, so the dispatch `$80:A251` waits for the others:
  checked through its fuzz, as the second dispatch's handlers are.
- **`shop_item_view_g` ($80:ABF6):** two groups (fields 0 and 2), then the stage step,
  with its stop at `$7FFF` static. Live 3,103/3,103, paths 31/31.
- **`interrupt_vectors_init` ($80:80E4):** `JML $80:8807` (the NMI) at `$0000`,
  `JML $80:89B2` (the IRQ) at `$0004`, the NMI's dispatch target `$0008` = `$8934`;
  then `$0026`, `$019A`, `$002A` and `$0028` are cleared.
- **`hdma_table_0d00_init` ($8A:9C86):** at `$0D00`, `$36` lines of `$FF`, then 21
  entries of 8 lines of `$FF`, then the end.
- **`string_space_fours` ($8A:BF2A):** it copies the zero-ended string at `$0D40` to
  `$0D60`, with a space after every fourth byte read (the zero included).
- **`table_062a_fill` ($8A:8AE7):** 12 words from X: `$062A` + x = `$013C` + 16n,
  `$072A` + x = `$013E`. Live 1,586/1,586.

## The second item dispatch itself (shop-10, 2026-11-17)

- **`shop_item_dispatch_icon` ($80:9F40) and `shop_item_dispatch_upload` ($80:9C0B):**
  the current item's handler from the table at `$81:A8A3` (list `$0148`, row `$014A`),
  entered by an `RTS` with the carry clear or set. In C it is a switch on the table's
  word.
- **The shop has nine lists (0-8), not six.** The first C covered lists 1-6, and live
  samples on lists 0, 7 and 8 failed (1,576/4,000). The table gives:
  - list 0: `$80:A096`, a fixed icon (group `$4F6A`) and no upload;
  - lists 6, 7, 8: `$80:A121`, `$80:A0E0`, `$80:A09F`. They work as lists 1-5 on
    `$0398`, `$0399`, `$039A`, but a level-0 part queues no upload. Lists 7 and 8 take
    the upload size from a table of their own (`$81:BCC8`).
- **These four handlers have no rows of their own** (lists 0 and 6-8 have one row each
  and run only inside the dispatches). They are checked through the dispatch rows,
  whose fuzz draws all nine lists.
- **The upload dispatch's exit:**
  - `PLX` gives back the entry (X, N, Z);
  - with no upload, C comes from the `SEC` and V from the dispatch's own index `ADC`.
    The fuzz caught that V.
- **Results:** icon dispatch live 4,000/4,000, paths 19/19; upload dispatch live 23/23,
  paths 21/21; fuzz 500/500 each.
- **This unblocks** the item dispatch `$80:A251` once its two views are in; after that
  come the screens that call both (`$80:9847`, `$80:9B5D`, `$80:9AC8`, `$80:9B99`).

## Small leaves the movies reach (leaves-9, 2026-11-17)

- **Why these:** the shop's remaining handlers wait for the parked dispatch `$80:A251`.
  This unit takes routines from `tools/portable.py` that the 60k or the shop movie runs.
- **The shop list cursor:**
  - `shop_list_locked` ($80:B359): (`$0148` * 8) byte-swapped plus `$014A`; carry
    set above `$1800` or when negative. The negative exit (`SEC`, `RTS` at
    `$80:B364`) is static code;
  - `shop_list_prev` ($80:B333) and `shop_list_next` ($80:B345) step `$0148` within 0 and
    `$01F0` unless locked, clearing `$014E` and playing sound `$11` (`func_80E4C5`).
  - Their N and Z are live after the return. A step's `DEC`/`INC` acts on memory, and
    `func_80E4C5`'s `PLA` gives back the old `$0148`: the first adapter got that wrong
    and a live sample caught it.
- **`star_layers_move` ($80:DACF):** each of the 64 stars' position (`$0F06`) minus its
  speed (`$0F00`), + `$8800` on a borrow.
- **The fade to black** (target `$0198` = 0, step `$0196` = -$80, run by the NMI's
  fade in `nmi_prologue`):
  - `fade_out_wait` ($8A:BAE3): start it, wait until the step is back to 0. It is a
    `waitsForInterrupts` row with no fuzz;
  - `fade_out_on_confirm` ($8A:9675): a new A/B/X/Y press (`$C0C0`) on controller 1
    (`$000E`) or the first multitap pad (`$0012`) is taken and starts it;
  - `fade_out_on_confirm_once` ($8A:8181): the same while `$018C` is 0, then counts it
    up. Its extent skips `$8A:8196-81AF` (not its code), which the first row counted as
    paths never run, via `alsoIn`.
- **Two one-liners:** `func_8A99D0` (`$0166` = `$28`) and `func_8A96C2` (`$0140` + 1).

## Steering the explorer; two item views parked (explore-2, 2026-11-17)

- **The last two item views** belong to one item each: `$80:A333` to list 1, row 4, and
  `$80:AEBE` to list 6, row 0. The shop movie never opens either, though the TAS runs
  both.
- **The explorer has two ways to aim:**
  - `--explore-want ADDR`: each listed CPU address whose first execution comes in a
    candidate is worth 100,000 new starts;
  - `--explore-goal ADR=VAL`: each WRAM word holding VAL at a candidate's end is worth
    1,000. That rewards partial progress towards a screen.
- **Neither reached them.**
  - With `--explore-want 80A333` alone, 30 segments of 8 candidates never ran it.
  - With the goals `$0148 = 1` and `$014A = 4` as well, every segment kept one goal and
    never the other.
  - The frame shows a part's view with levels 1-4 and "Credits 0". Row 4 is probably a
    level the shop offers only later in the game, which a movie from the first visit
    cannot reach.
- **Parked, not committed as rows:**
  - the C written for view `$80:A333` and for the spinning ring it draws (`$80:A579`,
    and the pieces `$80:A69F`/`$80:A6DB`) was kept as a patch, unverified;
  - it went in with shop-12, checked through the view dispatch's fuzz; the ring was then
    corrected to two pieces.

## Two-task screens (task-1, 2026-11-16)

- **What they are:** one screen runs the game as two tasks, each with its own stack and
  direct page: task 1 has D = `$0100` and the stack near `$1FF0`, task 2 has D = `$0200`
  and a stack seeded at `$1D8C` by `$80:B8F7`. Task 1 reads the port-1 pad (`$000C` into
  `$010C`), task 2 the port-2 pad (`$0010` into `$020C`). The frame dump shows the screen
  split into a top and a bottom panel. So these are probably the two players' halves,
  but the 1-player autopilot is the only movie that reaches the screen so far.
- **The switch:**
  - the NMI target `$80:B97B` saves task 2's SP in `$0278` and loads task 1's from
    `$0178`. It also drains one task's VRAM queue: task 1's (`$018C`/`$018E`) on even
    frames, task 2's (`$028C`/`$028E`) on odd ones. Then it reloads the PPU, sets the
    V-timer for line `$6C` with IRQ `$80:BC9B`, and calls task 1's callback
    `jsr ($01EC)`;
  - the IRQ `$80:BC9B` does the reverse at line `$6C` (SP to `$0178`, `$0278` into SP)
    and calls `jsr ($02EC)`.

  Each task gets one slice per frame: task 1 from vblank to line 108, task 2 from line
  108 to vblank. The callbacks are `$80:B94F` or `$80:C5E0`.
- **Measured (60k movie, `--hdma-report`, which logs the PC each switch interrupted):**
  - the switch is preemptive. Most switches land in a frame wait (task 1 in
    `wait_frames` 5,438 times; task 2 in its idle loop at `$80:B95E` 6,502 times), but
    about 1,000 land mid-work: `sprite_text`, `decompress` (`$82:FB70`), the sprite
    packers at `$81:80xx`, `$80:C2xx-$80:C4xx`. On one frame both tasks were inside
    `decompress`;
  - **no WRAM byte is shared:** outside the handlers and the two stacks, no byte is
    written by both tasks, and none is written by one and read by the other. (Measured
    with the 1-player autopilot, where task 2 is nearly idle. The report must be rerun
    when a movie drives pad 2.) Two bytes
    are read by both and written by neither. Per-task state goes through D; shared
    routines use only D-relative or per-task buffers;
  - **hardware registers:** only task 1 touches any. It uses the divider (`$4204-$4206`,
    `$4214/$4215`) and the mode-7 multiplier (`$211B/$211C`, `$2135/$2136`) at
    `$80:C37D-$80:C3AE`. No handler on these screens writes those registers. The
    multiplier's write latch is shared with the BG1 scroll registers that both handlers
    write, so an interrupt between the two `$211B` writes would corrupt the operand. The
    window is about 15 instructions, and no switch ever landed in it (the closest were
    at `$80:C369` and `$80:C3BC`). The exit code at `$80:D198` writes `$2110` and waits
    on `$431A` when the screen ends.
- **What it means for the C:** each task is ordinary blocking C. A task runs until it
  waits for the frame. A port can run task 1, then task 2, to their waits in each frame,
  because neither sees the other's state. Where a slice ends mid-work, the one
  difference is when the other task's VRAM queue entries and sprites appear, the same
  kind of up-to-one-frame difference the "Tables written while drawing" note records. It
  does not change the C.
- **What the harness still needs before `$80:B97B` and `$80:BC9B` can be rows (the
  stack window and the undeclared-switch check are done, see "Stack switches in the
  differential test" below):** both return through the other task's stack.
  - The RTI row model takes PC and SP from the entry stack's frame. It has to end the
    sample at the handler's own RTI and restore from the frame at the loaded SP.
  - The stack exclusion window `(minSp, entrySp]` assumes one stack. After a switch it
    would hide the WRAM between the stacks, or fail on the other stack's pushes. It needs
    one window per stack, and rows that run `TCS`/`TXS` without declaring it must fail.
- **Instrument:** the `--hdma-report` adds three sections: where each task was when
  switched out, the WRAM both tasks use with at least one writing (merged ranges,
  stacks left out, with byte counts per use pattern), and the hardware registers each
  task touches.

## Stack switches in the differential test (task-2, 2026-11-16)

- **The hole:** a waiting routine's stack window `(minSp, entrySp]` grew with every SP the
  handlers it recorded used. On the two-task screens a handler switches to the other
  task's stack, so `minSp` dropped to that stack (`$1D88`), and the window hid all WRAM
  between the two stacks (`$1D89` up to the waiter's own frame) from the comparison.
- **The fix:**
  - inside a recorded handler, an SP load (TCS/TXS) above the interrupt frame, or more
    than `STACK_REACH` ($100) bytes below it, marks the handler as on a foreign stack;
  - until it loads SP back near its frame, its stack use does not widen the window. The
    other task's WRAM changes are still in the recording, so the C's replay must produce
    them;
  - the report counts the passing samples that waited through such a switch:
    `wait_frames` 681, `queue_unpacked_upload` 3 and `wait_flag_76` 1 on the race1 movie.
    All still pass with the narrower window.
- **Undeclared switches fail:** a sample whose own code (not an interrupt that landed in
  it) loads SP above its return frame, or more than $100 below its entry SP, fails at
  once with "an undeclared stack switch". The samples inside it are counted as
  abandoned. No verified routine does this. The rows for `$80:B97B` and `$80:BC9B` will
  declare their switch (the save and load words, the bytes popped from the new stack),
  and their return will be read from the frame at the loaded SP.

## Statically found code (map-2, 2026-11-15)

- **`map/static_code.txt`** lists code that no trusted run executed but that other
  evidence shows is code; each line is a range, its entry widths and the evidence.
  `tools/map.py` decodes it linearly (following REP/SEP), requires it to lie on unknown
  bytes and to end on an instruction boundary, marks it **`static`** in `rom_map.txt`
  and adds its instruction starts to `code_starts.txt`. The listing disassembles it (each
  line tagged `static`) and the code-from-text proof covers it. It does not count as
  code or data in the progress numbers: coverage stays the only source of "code".
- First entry: `$8A:8B9D`–`$8A:8BB5`, the set-bit path of `mode7_view` (25 bytes),
  executed only by its difftest fuzz and matched there by `src/scale.c`. The branch at
  `$8A:8B82` now assembles as `bne CODE_8A8B9D` instead of raw bytes.
- **The static route for blocked roots (map-3, 2026-11-15):** decode the unexecuted bytes
  by hand and list every branch and call that leaves them; if one leads to other
  unexecuted code, park the routine (no chained static claims); otherwise port it, aim
  the fuzz until `paths N/N` includes the new bytes, and only then add the range.
  - `func_8AA32B` ($8A:A32B, 857 calls): its bit-7 path `$8A:A345-$8A:A34B` (store `$E0`,
    branch to the common exit). Live 286/286, fuzz 500/500, paths 39/39.
  - `func_8AA381` ($8A:A381, 264 calls): its bit-11 path `$8A:A3C2-$8A:A3F3`, reached by
    four BNEs, calling only the verified `func_8AA48D`. Live 88/88, fuzz 500/500, paths
    64/64. The four branches now assemble as `bne CODE_8AA3C2`.
  - Parked: `$8A:9F76` (its unexecuted bytes call `$8A:AE9B`, itself never executed).
  - `func_80D6F3` (math unit): a step toward a target from the 8.8 position `D+$34/$36`,
    $0100 along the longer axis and |shorter| / (|longer| >> 8) on the CPU divider along
    the other (a zero divisor gives `$FFFF`, as the hardware). Its four negations
    (`$80:D700`, `$80:D712`, `$80:D767`, `$80:D773`, 4-8 bytes each) never ran in a
    recorded movie; the fuzz covers them: live 1/1, fuzz 500/500, paths 74/74.
  - Not yet: `$85:BC20` (18,507 calls) builds double-buffered HDMA tables and has several
    unexecuted stretches; it goes with the HDMA unit.
  - `func_8A8290`: everything past its first test, `$8A:8295-$8A:82B3` (a counter that
    every 8th call steps `D+$64` through the cycle 1, 2, 3, 4, 3, 2 at `$8A:82B4`; the
    table stays unclassified). No recorded run reaches it with `D+$54` set.
  - `func_809DD0` (2,875 calls): its `$3E00` variant `$80:9DDC-$80:9E0A`, the same label
    and number with another sprite base and the string at `$80:9E42`; it calls only the
    verified `sprite_text` and `sprite_number`. Live 664/664, fuzz 500/500, paths 49/49.
    Its 47 bytes are **not** added to `static_code.txt`: the map classifies them as
    *data* (some recorded run reads them), and a static range must lie on unknown bytes.
    The C covers them (the fuzz runs them); the map keeps what the runs saw, and who
    reads those bytes is an open question.

## Coverage sources (2026-11-14)

- **Only runs that are known to follow the game's intended path feed
  `traces/coverage.bin`:** the bench's own movies (start taps, to race, race 1, the 60k
  autopilot) and the TAS replayed **in sync** on the reference bench (`--bizhawk`, real
  DSP-4). Desynced TAS runs are not coverage sources: `tools/map.py` rejected the merge
  because the old desynced coverage had an instruction start inside one the synced TAS
  executes, and the bench movies alone never touch `$80:A3B2`.
- Rebuilt from scratch with those sources: **79.94% of the ROM classified** (code
  69,676 bytes, data 768,558, unknown 210,342), up from 46.78% (whose code count,
  65,367, included the derailed bytes). The TAS alone executed 67,823 code bytes and
  read 766,727 data bytes in 450,994 frames; outside ROM only `$00:0000`/`$00:0004` run
  (the idle loop), as on the bench.

## Frame waits (2026-11-14)

- `platform_wait_vblank()` joins `src/platform.h`: return after the next interrupt has
  run (the port does the frame's work there). `wait_frames(n)` ($80:8373 and the long
  copy $8A:BA90, `src/frame.c`) waits until the frame counter `$0024` (incremented by
  the NMI handler) has advanced by n.
- Harness: routines marked `waitsForInterrupts` keep their interrupted samples. Each
  interrupt inside the sample is recorded as WRAM before/after the handler (whose
  hardware accesses are its own, not the routine's), and the bench's
  `platform_wait_vblank()` replays them in order; leftovers are applied after the C
  returns; a C that waits for more interrupts than the original saw is aborted with
  `longjmp` and fails. Race movie: 1,994/1,994 and 155/155 live samples (D = $0100 or
  $0200). Negative variant waiting one frame too many fails. No fuzz for these
  (interrupts are off while fuzzing).

## Text, part 2 (roots-3, 2026-11-14)

- `src/tg3000.h` gains `bus_rd8/bus_rd16/bus_wr16`: access through a 24-bit bus address
  (WRAM for `$7E/$7F` and the low mirror, ROM otherwise) for the original's long and
  DB-relative pointers.
- `text_print_centered` ($8A:B6AA): the zero-terminated string at DB:A is printed
  centred between columns `$013E` and `$0140` — or, when the byte `$0168` is set, as as
  many spaces (erasing it); its length goes to `$015A`, and when `$0186` = 0 it ends with
  `func_8AB700`, which therefore tests the **parity of the string length**. Its callers
  read C/V; they come from the start-column ADC (preserved through `text_put_char`'s
  PHP/PLP) or from `func_8AB700`'s SBC, and the adapter models both. The erase path never
  ran in a movie (decoded from ROM, fuzzed).
- `text_put_tile_char` ($8A:C278, one-tile glyphs, attribute `$3880`/`$3C80`),
  `text_cursor_down3` ($8A:A37A), `byte_buffer_push` ($8A:9C19, buffer `$7F:7D00`) and
  `func_8AB942` ($8A:B942, a step of a two-column wipe through the long pointer at
  `$0170`).

## The data decompressor (2026-11-14)

`decompress` ($82:FB70, long call, `src/decompress.c`; 44 unported routines depended on
it): A = output bank << 8 | input bank, X = input offset, Y = output offset; returns the
length in A. Format (each command a byte; documented in `src/decompress.h`):

| command | effect |
|---|---|
| `$00` | end |
| `$01-$3F` | that many literal bytes |
| `$4n` | n+3 bytes continuing the byte progression of the last two output bytes |
| `$5n` | n+2 words continuing the word progression of the last two words |
| `$6n` | the last byte repeated n+3 times |
| `$7n` | the last word repeated n+2 times |
| `$80-$BF` | 3 bytes copied from (cmd & $3F) + 3 back |
| `$C0-$DF`, d | 4 + ((cmd >> 2) & 7) bytes from ((cmd & 3) << 8 \| d) + 3 back |
| `$E0-$FF`, d, l | l + 5 bytes from ((cmd & $1F) << 8 \| d) + 3 back |

Back copies move 16-bit words in the original's fixed order (two different orders for
odd lengths in `$C0` and `$E0`), which matters when a copy reads what it just wrote, so
the C keeps that order. The routine dispatches through a 128-entry jump table and three
inner ones (unrolled stores entered part-way, and an RTS used as a jump); the harness
gained "holes" so those tables are not mistaken for code. Fuzz builds random but valid
streams with every command (short and long distances, overlapping copies). First run
failed on `$5n` (I had copied `$4n`'s count; the word version stores one fewer before its
loop) — found from the first differing output byte; now race movie 67/67 live, fuzz
400/400, paths 435/435. Harness failure messages now show the entry X/Y and the first
differing bytes, and the instruction cap per sample rose to 4M (long unpacks).

## After the decompressor (2026-11-14)

- Roots recomputed: `decompress_near` ($80:FBA0, a JSL/RTS bridge in bank $80) became the
  top root (29 dependents) and is ported; also `clear_object_slots` ($8A:A47E, the
  object table `$0500` and `take_slot`'s counter), `func_8AA469` ($8A:A469, targets for
  `step_toward`) and `string_list_nth` ($8A:A4CF: skip n zero-terminated strings; the
  original steps over each terminator with an **8-bit** increment of the pointer's low
  byte, so a terminator at `$xxFF` wraps within the page — kept, and fuzzed with lists
  ending at a page boundary).
- Recurring cost: routines with fewer than 100 watched returns keep every flag in the
  contract, so adapters keep modelling incidental C/V. The real fix is the open harness
  task (sample nested entries, one flag watcher per routine).

## Nested samples and per-routine flag watches (2026-11-14)

- The harness kept one sample at a time, so a routine reached only from inside another
  registered routine's sample was never sampled (`func_8AB700`: 0 live samples once
  `text_print_centered` was registered; `tilemap_offset` likewise) and kept every flag
  in its contract. Now samples nest up to 4 deep: each has its own entry WRAM copy,
  registers, stack window, extent guard and effect list; hardware accesses and DMA
  effects count for every sample in progress (an inner upload is also the caller's);
  interrupt recording belongs to at most one sample (a routine that waits for them).
  A sample whose caller returns without it returning is counted as abandoned.
- Flag liveness now has one watch per routine instead of one for all.
- **Fix after the first commit of this change:** an entry reached again in a stack frame
  that is already being sampled is a branch back to the entry (`star_random_y` loops
  with `BCS` to its own first instruction) or a tail jump, not a call; it opened a bogus
  nested sample that took the real sample's return. Such entries no longer start a
  sample. That commit (6fad60e) went in with `star_random_y` marked *failed* (43
  verified) because `tools/verify_all.sh` returned `grep`'s status instead of the
  bench's and the commit was chained with `;`: the script now returns the bench's status,
  and commits are gated on it with `&&`.
- Race movie: `func_8AB700` 250 live samples (was 0), `tilemap_offset` 2,329 (was 0),
  `text_put_char` 2,561; everything still passes and the three negative variants
  still fail.

## Hot roots (2026-11-14)

- `take_nibble` ($80:8542, 61,785 calls — the most called routine so far): shifts the
  32-bit value `$0118:$011A` left by 4 and returns the nibble shifted out; a bit reader.
- `func_83FE9A` ($83:FE9A, runs with D = `$0200`, DB = `$30`): stores a request
  (`$0463` = A, `$0465` = X, `$0467` = 0) unless `$047C` has bit 15 set (C = 0) or a
  cooldown at D+`$23` is still running after subtracting `$1E9E` (refused, C = 1); a
  finished cooldown restarts from Y. The cooldown path never ran in a movie (decoded
  from ROM, fuzzed; it continues into a 3-instruction exit at `$83:FEBA` that the
  extent now includes).
- `sprite_text` ($80:841A): lays a zero-terminated string out as sprites, one 4-byte
  entry per glyph at `$0900+slot` (x, glyph + base `$0114`) with `$0B01+slot` cleared;
  x advances by each glyph's width (table `$80:85E5`), a space by 6, bytes with bit 7
  set are skipped. Glyphs: digits ch+4, A–Z ch−$27, a–z ch−$61, the punctuation list
  `&#[.+={}?!-~]_` at `$80:84B0` → `$3E`–`$4B`; anything else keeps its own code (the
  search loop falls through with A unchanged — the fuzz found it, no movie does it).
  The glyph mapper at `$80:8435` is 8-bit code reached by a branch after a `REP #$20`
  at `$80:842E`, so the linear decode for path coverage got the widths wrong (55/76
  paths); routines can now name addresses where the sweep restarts with M = 8 (`m8At`).
- `sprite_frame` ($8A:A0F9) and `func_8A9FA1` ($8A:9FA1), the next roots: a box of
  sprites three rows high whose width, y and x offset first step towards their targets
  (the step_toward routines), and a mask that shows or hides seven objects through bit 7
  of their `$1516` flags. Both passed on the first run; the fuzz keeps widths below `$60`
  and `$0158` in 0–5, the ranges the game uses (a huge width would make the original
  write slots across all of WRAM).
- Fuzz seeds are per routine now (from its entry address). With one seed for the whole
  run, adding these two routines shifted every later routine's random inputs and
  `text_put_char` lost 2 of its 88 paths in the selftest's short run (200 inputs) — a
  pass or fail that depended on registry order. The gate caught it before the commit.
- Five leaves: `clear_tilemap_7f` ($8A:B981), `func_8ABC32`, `func_8ABC7F`,
  `func_82EC09` (a signed 12-bit difference from two tables at `$7E:8CB3`/`$8CB5`) and
  `text_print_tiles` ($8A:C13F, a string at DB:A printed with text_put_tile_char). The
  latter's `$80` = new line path never ran in a movie; its bytes (`INC $3C; STZ $3A;
  BRA`) were decoded from ROM and the fuzz, which puts strings in DB `$8A`'s low-WRAM
  mirror, covers it. `clear_tilemap_7f`'s 6 live entries were all interrupted or not
  sampled (a 2048-word fill), so it rests on the fuzz alone.
- `wipe_columns` ($8A:B91A), `clear_sprites` ($8A:8F02), `func_8AA48D`/`func_8AA49E`
  and `text_print_tiles_at` ($8A:C137). The wipe waits for frames inside a loop that
  also calls other sampled routines, which showed two harness bugs: a nested waiting
  routine (wait_frames_long) took the interrupt recording away from the outer one, and
  any nested sample's comparison reset the recording owner to none when it finished. Now
  an outer waiting sample keeps the recording (the nested waits go unsampled there) and
  the owner is restored after a nested comparison; up to 64 interrupts are recorded (was
  24). The wipe has no fuzz (it waits) and 2 live samples, both passing.
- `sprite_text_centered` ($80:84BE, 36,567 calls in the 60k movie) and `sprite_number`
  ($80:84F0): the first runs sprite_text and moves the new entries by half of
  (start − end) so the string is centred (the shift is left in `$0114`, clobbering the
  glyph base); the second shows the 8 nibbles of X:A (high:low) as digits (glyphs `$34`–`$43`)
  with leading zeros skipped and a 1-pixel gap after positions 6 and 3. A string without
  a glyph ("", only spaces or bit-7 bytes) makes the centring loop walk all of memory (a
  do-while on `CPY $74`); the game never centres one. **Correction:** this note said the
  fuzz never produced one — it did (glyphless strings), and those cases were silently
  dropped (abandoned at the instruction cap) until lost fuzz cases started failing a
  routine; the fuzz now only builds strings with a glyph.
- `sprite_group` ($80:B49E, 14,024 calls) adds a group of sprites from a table of two
  zero-terminated lists of 6-byte items (tile, x, y), the first list's entries flagged
  `$0B02 = 1`, the second's 0; it runs with D = `$0100` or `$0200` (a parameter in the
  C) and sets DB to the table's bank, so its entry stores also go through that bank
  (`bus_wr16`). `func_83EA9B` ($83:EA9B, 18,548 calls; D/DB `$0100/$83` or `$0200/$30`)
  copies entry X of six parallel word tables in bank `$86` to `$1EA0`–`$1EAC`.
- `oam_high_table` ($81:8101, 8,552 calls; second entry $81:80F6 for the table at
  `$0B00`), the first piece of `src/sprites.c`: 256 unrolled shifts that take bit 7 of
  byte 1 and bit 0 of byte 2 of each of the 128 4-byte entries (shifting them out of
  those bytes) and pack them, 2 bits per entry, into the 32-byte OAM high table written
  over the start of the table; then the slots from D+`$74` to last frame's D+`$72` get
  x/y `$E0` (off screen). In C a 32×4 loop. The `$80F6` entry jumps into the other's body
  after its prologue, so its extent spans the body with that prologue as a hole.
- `func_8ABB5C` / `func_8ABBF4` ($8A:BB5C, $8A:BBF4): the 16 words `$0DAC`–`$0DCB` are a
  256-bit number (most significant word first), shifted right Y times (bits out into A;
  bits in: the entry carry, then the top bits of `$0146`, which fills with 1s) or left
  (bits in from A's low end, each step `$0184` += 1 and `$0192` += `$0DCA`). Purpose not
  confirmed (a password's bit buffer is a guess). Few live samples (17 and 30 calls in
  the 60k movie); the fuzz carries most of the evidence.
- `func_81FBB1` ($81:FBB1, the most called routine: 351,383 calls in the 60k movie),
  the first of `src/track.c`: for the car at Y (records `$35` bytes apart in bank `$7E`:
  position `$FAB6`, flags `$FADE`, segment record `$FAE0`, jump point `$FAE2`, jump
  `$FAE4`) it moves the car's record pointer through a 20-byte-record segment table
  read as `$84:0000+X`, choosing each record's branch once with rng_next_long, adding the
  record's length or setting a jump point, and finally jumps the car once it passes its
  point. Because bank `$84`'s `$0000`–`$1FFF` mirrors low WRAM, the fuzz builds a
  synthetic table at `$1800` that the original and the C both read. The C flag is live
  (callers read it) and matched in all 4,000 live samples.
- The end-of-frame sprite wrappers: `oam_finish_0b00` ($81:80D0; the single table at
  `$0B00`: pack, D+`$72` = D+`$74`, D+`$74` = 0) and `oam_finish` ($81:80DA; two sprite
  buffers: pack the one at D+`$6C`, keep the last two frames' ends in `$70`/`$72`, start
  the next frame in the other buffer, D+`$74` = D+`$6E` − `$0900`, and swap `$6C`/`$6E`).
  `take_nibble16` ($80:85CA, 19,950 calls) is take_nibble's 16-bit sibling on `$011A`.
- A map at `$7E:230C` handled in blocks of 2 rows × 5 words (rows `$50` bytes apart),
  ~18,000 calls each in the 60k movie: `func_82B78D` sets the block coordinates
  `$0156`/`$0158` (two table bytes at `$7E:4F31`, each − 2), `func_82B7B0` ANDs the block
  at those coordinates with one of 8 ten-word masks in bank `$83`, `func_82B838` copies a
  block in from `$7E:217C`. What the map is is not known yet.
- `func_80DC5A` ($80:DC5A) writes a small HDMA-style table (3-byte entries) at DB:X, bank
  `$7F` in the game; `func_8A9BB5` ($8A:9BB5) takes a free slot of a wanted kind from a
  table at `$7F:7FBC`, settling for lower kinds down to 0 (carry set when there is none).
- Process note: a 60k difftest run was killed as "hung" at 23 minutes; it was not —
  a 60k run with 500 fuzz cases per routine now takes ~22-24 minutes (the old 10-12
  minute figure dates from fewer routines). Separately, the short movie with `--fuzz 50`
  fails the fuzz of ~8 already verified routines, on the committed code too, while
  `--fuzz 200` (selftest) and the 60k run with 500 pass: the fuzz base RAM carried over
  from routine to routine, so results depended on how many cases ran before.
- **Fuzz cases are now clean and counted** (advisor's review): before *every* case the
  WRAM is restored and NMI, IRQ and every HDMA channel are switched off (a fuzzed routine
  could leave them on, and the game's HDMA was still running: interrupts fired inside fuzz
  cases, which were then dropped, and the 60k run's frame counter grew with each routine
  added). Each case requested is counted; a case that is neither passed nor failed
  (interrupted, or abandoned at the instruction cap) is *lost*, printed as `(N lost)`,
  and fails the routine — before, lost cases were silently ignored by the gate. The first
  run found one: `sprite_text_centered` (see its correction above).
- **Fixed earlier the same day:** every routine's fuzz now starts from the same WRAM (a copy taken when the
  live run ends). Short movie: `--fuzz 50` 8 failures → 0; `--fuzz 200` 0 → 0. Not
  explained yet (open): in those failures the original left `$36` in a few bytes the
  routine never writes; a guess is hardware state left by an earlier routine's fuzz (an
  HDMA channel writing WRAM during the sample), which the WRAM reset does not cover.
- Four small bank-`$8A` roots, each called only 4-7 times in the 60k movie (the fuzz
  carries the evidence): `func_8A8EEF` (clear `$0500` and the sprites), `func_8A9BAA`,
  `func_8A9BE2` (the largest slot kind, by the sign of the difference as the original's
  BMI does — not a true signed compare) and `func_8AB680` (a list of 8-byte records
  setting `$1516` + offset).
- `func_83FC05` ($83:FC05; D = `$0200`, DB = `$30`): an overlap test — the entry's
  12-bit signed position (byte `$7E:8CB3`+X × 16, kept in `$0128`) minus D+`$06`, then
  −(high nibble of `$8CB5`+X) × 8 minus `$012C`, each checked against per-kind limits in
  bank `$83` by the sign of the difference (BPL/BMI, not a signed compare); carry = hit.
  It was listed as sharing a tail with `$83:FD8A`/`$83:FDCD`; it does not (all its exits
  are its own). **`$83:FD8A`'s tail (`$83:FE16`) is the first root that talks to the
  DSP-4**: command 0 (two 16-bit operands written to `$30:8000`, a 32-bit product read
  back) — the reference bench with the real firmware is the oracle for porting it.

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
native DSP-4 C, with credit (the owner later chose to write the DSP-4 C ourselves:
see "DSP-4 command catalogue"). The LICENSE file goes in with the GitHub setup.

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
