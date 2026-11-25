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

## The DSP-4's streamed commands are in scope (the owner's decision, 2026-11-21)

- **The decision.** The owner leaned yes and asked the advisor, who agreed. Native C for
  `$09` and `$06` (sprites) and `$01`/`$07`/`$08` (the road) belongs in this project, in
  `src/dsp4.c`, in stages.
  - Why here: `src/dsp4.c` already has five of the game's ten commands, and the firmware
    oracle and the TAS exist only here.
  - Without the native commands, every taped routine's C ends at `dsp4_port_*` with
    nothing behind it.
- **How.**
  - The routines' C stays as it is. The native chip goes behind
    `dsp4_port_write16`/`read16` and consumes the same word stream.
  - The checks compose: the tape checks a routine against the stream, and refbench
    checks the stream against the firmware.
- **The first unit is an instrument.** refbench gets an ordered read/write tape like
  harness-4's, anchored on the game's PCs (`$0009` is written at `$82:CC8F`). It is not
  anchored on the shadow's busy flag, which produced the stray runs.
- **Then one measurement.** Split the firmware tape into the record shapes the ported
  routines encode, and check whether each record's answer is a function of its own words.
  - If it is, `$09` is derived as `$0A` was, and `$06` follows.
  - If not, the dependencies come first.
  - The numbers go to the owner.
- **No promise yet on the road commands** (`$01`/`$07`/`$08`) until the same measurement.
- **The method stays outputs only.** Chosen inputs to the idle chip are allowed (as in
  the `$0A` section) and get noted at first use. If outputs-only proves insufficient for a
  command, stop and ask: the alternatives are licence questions.
- **Pace.** These units move neither the routine count nor the byte percentage. They are
  interleaved with roots, and long refbench captures run in the background, never during
  a gate.

## A password movie (explore-3, 2026-11-23)

**`tools/bench/movies/explore_pw.txt` is the fourth movie** of `verify_all` (two slices)
and `verify_new`. It reaches what no movie did: the later championship's shop lists, the
password screen, a skipped track's bonus.
- **What gates the shop's lists.** Not money: a list shows only while its byte at `$036E` +
  its index is non-zero (`shop_list_label`). `result_tally` ($84:A34B) raises those bytes
  after certain tracks (`$0444`), by the variant (`$03DC` bits 8-9). For example, in
  variant 0 list 6 opens after track 15 and list 7 after track 33. The 60k movie never gets
  that far.
- **The password's fields** (`password_pack`'s table at `$8A:CF61`) hold:
  - `$03DC`, the track `$0444`, and the colour `$0392`;
  - the level bytes `$0393-$039C`;
  - 13 bits of the money `$038E`;
  - player 2's copies.
- **How the password was made (a recipe, not in the repo):**
  - A temporary hook in the bench wrote `$0444` = 30 and `$038E` = 8,000 when the 60k
    movie entered `password_pack` ($8A:BB99, after `func_8ABF4E` has turned the money into
    binary).
  - The game's own `password_pack`/`password_encode` then made `T9Y9LFR1M?N9B6R1VQ9`, and
    its decoder accepted it.
  - The shop then showed "Credits 8000000"; how the display relates to `$038E` was not
    measured.
- **The typing:** `tools/password_movie.py` writes the taps from the grid, and reproduces
  the movie's typing.
  - The grid's last row (`.?!`) is reached from its own columns.
  - EXIT's A waits 60 steps, because the cursor glides there and an early A does nothing.
- **The movie:** race1 to the Championship menu, PASSWORD (Down x4, A), the typing, EXIT,
  RACE, the track description and the shop. Then the explorer: 40 + 24 segments of 300
  steps, aimed at the lists. 29,000 steps in all.
  - It buys from lists 1-3, 6 and 8, runs the refund, the dispatcher `$80:9039` and
    `shop_level_blocks`.
  - It does not buy from lists 4, 5 or 7: `explore_buy.txt` stays, the only movie that buys
    from list 4.
  - It ends inside a race (lap 1 of 2, not paused): the fuzz base of every row it enters.
- **Before wiring it, every row ran over it at two slices:** 18 failed, 2 of them hard
  (harness-9's). Of the 16 others, the first full gate still failed 8 after pooling. Five
  of them, and `shop_gauge_line`, are rows no movie had reached, with no fuzzer and no flag
  model. This unit verifies them for the first time:
  - **lists 6-8's views** `shop_item_view_h`/`_i`/`_j` ($80:AEBE, $AE4E, $AC37). Their
    tail: D+$40 += `$40`, wrapping at `$1E00`/`$0F00` for h and i (a `SBC`), then
    `INC $3E`. So N/Z come from D+$3E, and C/V from the `CMP`, the `SBC` or the `ADC`. A
    fuzzer per view (after views e/f's), with the spin near its wrap;
  - **`shop_level_blocks`** ($80:AF36). It ends on the `CMP #$2D` that matched (Z and C set,
    N and V clear), or, with all seven blocks full and three or more left over, at a `JMP`
    to its `RTS`: the `SBC #3`'s N/Z/V, C set, nothing drawn. The movie runs that path;
    `map/static_code.txt` had it as never run. The C now returns which. A fuzzer over the
    stage D+$3E;
  - **`shop_gauge_lines`** ($80:ACB6) **and `shop_gauge_line`** ($80:ADD9). A line ends past
    column `$48` (the `CMP`'s N/Z/C) or at its string's 0 word (that `LDA`'s N/Z, the
    `CMP`'s C); V is the last `ADC #8`'s. The four lines end with `PLB`: N/Z of the entry's
    bank. A fuzzer for the lines, with the column D+$14 from `$C0` up, where the sign path
    `$80:ACD6` runs.
- **Lists 1-3:** the movie buys from them, so their buying paths are compared now.
  - The money short, with or without a refund, still runs on no movie: holes, as list 4's.
  - List 1's row now ends at its own `endAt` (`$80:90FD`, the `JMP` to the settle), as
    `hud_markers_write` does.
  - At that `JMP` list 1 has its own exit model: owned, `shop_item_show_icon`'s N/Z (D+$44);
    bought, `shop_money_store`'s (D+$54). Either way C is set and V clear, from the
    redraw's last `SBC #$0900` in `oam_finish`.
  - Still unchecked: list 1's order of its two calls when short after a refund.
- **Its coverage is merged into the map:** code grows from 70,518 to 71,889 bytes. Eight
  `map/static_code.txt` ranges it executes left that file (`func_80D6F3`'s negations,
  `shop_item_view_h`'s wrap, `shop_level_blocks`' full row, `track_snap`'s skip,
  `money_skip_bonus`).
- **Gate:** the full one, over four movies. It took 2,510 s (about 42 minutes) with the
  selftest, on 8 cores.

## Lists 6-8's buying (shop-19, 2026-11-23)

- **Ported:** `shop_buy_list6`/`7`/`8` ($80:94DE, $80:93FC, $80:931A), one function
  `shop_buy_upgrade` with the field 5-7. They are lists 1-5's buying, with three changes:
  - the level byte (`$0398`/`$0399`/`$039A` + D+$94) holds the row + 1, so 0 means nothing
    bought yet, and the part is owned when `(level & $7F) - 1` is the row;
  - the first purchase skips the panel's rise: D+$42 = `$B4` and D+$44 = `$0F` at once
    (no `TSB` of D+$76 bit 1), a second `func_80B58D`, then `shop_item_show_icon`;
  - a level held from elsewhere (bit 7 of the level - 1 clear) is not sold back.
- **Rows:** lists 6 and 8 pass on the password movie (2 and 3 samples, every path outside
  the holes). List 7's row is unreached: only the TAS runs it. Its C is the same function.
- **Holes, per list:** the part bought here sold back, a level held from elsewhere, the
  money short and the panel's rise. No movie runs them, and a waiting row has no fuzz. List
  7 also has its owned path as a hole. The rise is the same code as `shop_buy_part`'s, which
  lists 1-5 verify; it is written apart for now (`shop_panel_rise`), so the change stays
  additive. Making `shop_buy_part` share it waits for a unit that runs the full gate.
- **To get rid of the holes:** a movie that buys a second part in lists 6-8 would run the
  refund and the rise.
- **Gate:** incremental (`verify_new.sh`): new lines in `src/` and `tools/bench/` only.

## The buying dispatch, lists 9 and 10 (shop-20, 2026-11-23)

- **`shop_buy_dispatch` ($80:9039) is ported.** It runs the word at `$81:A89F` in the item's
  entry (list D+$48, row D+$4A). The ROM pushes its own `RTS` ($80:9052) as the return,
  then the handler's address - 1, and `RTS`es into it. The C is a `switch` over the eleven
  words the table holds and returns the handler's `ShopBuyEnd`.
  - Its row was a waiting row around the handlers' waiting rows. On its own it passed on
    the shop, buy and password movies (134, 35 and 46 samples, every path). The claim
    written here then, that nesting needed no harness change, was wrong: see shop-21.
  - Exit: the handler's. After the colour (list 0) C and V are the dispatch's `ADC` of the
    entry, and N/Z are the `PLA` in `func_80E4DB`: the handler's address - 1 as the high
    byte, D+$4A as the low.
- **The table has two more lists:**
  - list 9 ($80:966E, level byte `$039C`);
  - list 10 ($80:9750, level byte `$039B`), whose one pointer is the word at `$81:AE20`.
  - Both use `shop_buy_upgrade`. List 9's row (the holes of lists 6-8, plus its owned path)
    is unreached: only the TAS runs it. List 10's code runs nowhere. It is now a
    `map/static_code.txt` range, so the listing decodes it; it has no row.
- **Holes are not counted.** `tools/progress.py` counts code bytes in verified extents
  minus their holes, so a hole never adds to the verified percentage.

## The dispatch's row ends at its RTS (shop-21, 2026-11-23)

- **What went wrong.** shop-20's waiting row for `shop_buy_dispatch` enclosed the handlers'
  rows. In a full run, the handlers' samples taken inside it were not compared. List 1 had
  6 entries on the password movie and 4 compared; list 2 had 11 and 6. Their refund paths
  went unvisited: 56/60 and 58/62, failed.
  - `verify_new.sh` re-ran only the new rows, so the gate did not see it.
  - Commits c425554, a841198 and ada9863 publish lists 1 and 2 as verified on rows carried
    from 927d0f7, which a full run then fails. The owner is told.
- **The fix (the advisor's call, open to the owner's veto):** no harness change.
  - The row now ends at the dispatch's own `RTS` into the handler (`$80:9051`, `endAt` =
    `end`), so it encloses no other row and needs no waiting.
  - It checks the lookup: `shop_buy_handler` returns the word at `$81:A89F` in the entry.
    The adapter models A (the handler's address - 1, N/Z of the `DEC`), X (the entry), the
    `ADC`'s C/V and the two pushes of the `RTS` trick.
  - A fuzzer runs over the table's lists (0-10) and rows (0-7). Off those, the entry runs
    past bank `$81` into `$82`'s WRAM and I/O mirror, which is not a game state.
  - `shop_buy_dispatch`'s `switch` now calls the handlers by name (`shop_buy_list1()` ...).
    No row checks the `switch` itself (word to function); each handler's row checks its
    own C.
- **`verify_new.sh` refuses a new waiting row** and sends it to the full run (also in
  `WORKFLOW.md`). This is the advisor's call, open to veto.
- **Still open:** with the dispatch enclosing it, list 1 started no sample for 2 of its 6
  entries, with none abandoned or interrupted. That condition in `difftest.c` is not found
  yet. It does not block this fix, but it blocks a row for the password screen's loop
  (`$8A:B295`), which has the same shape.
- **Gate:** the full one. Lists 1 and 2 are back at 60/60 and 62/62.

## The password screen's grid cursor (password-1, 2026-11-23)

- **Picked from `tools/portable.py`:** after the password movie, most of its list is the
  password screen (`$8A:B1xx-B6xx`). This unit ports its leaves, in a new `src/password.c`:
  - the four pad handlers (`password_pad_right`/`left`/`up`/`down`, $8A:B4D2-B585). A new
    press on any of the four pads (`$000E`, `$0012`, `$0016`, `$001A`) moves the cursor
    once it has stopped gliding;
  - the four steps (`password_cursor_*`, $8A:B586-B611) over the grid's coordinates
    (`$0D88`/`$0D90`, held at its edges; the last column `$28` on);
  - `password_lines_print` ($8A:B612): the four lines of text;
  - the caret: `password_caret_place` ($8A:B644) and `password_caret_blink` ($8A:B2BC);
  - `asl_n` ($8A:B152), `lsr_n`'s mirror, with Y kept.
- **Check:** all twelve pass on the password movie with a fuzzer each, comparing A, X, Y and
  the flags (each exit is modelled from the state before the call). The fuzz runs two
  branches no movie takes, now `map/static_code.txt` ranges: right on the row of three and
  left off the last column.
- **Not yet:** `$8A:B4A4` (A on SPACE, DEL or EXIT) calls the typing routines `$8A:B403`
  and `$8A:B420`, which are not ported.

## The password screen's typing (password-2, 2026-11-23)

- **Ported in `src/password.c`:** the typed text (up to `$15` characters at `$0D40`, their
  count at D+$80):
  - `password_type_char` ($8A:B403), `password_delete` ($8A:B420): each redraws the text and
    beeps (`func_8AA48D`). When the text is full, or empty on DEL, they refuse with
    `func_8AA4AF` ($8A:A4AF, `$01FB = $12`; now ported too);
  - `password_text_draw` ($8A:B43D): the text from column 2, the caret on the space after
    it. A space follows each group of four, but only while the first four characters hold
    no vowel (`password_no_vowel`, $8A:B47C). The password alphabet has no vowels;
  - `password_press_last_column` ($8A:B4A4): A on SPACE, DEL or EXIT; returns EXIT.
- **Check:** they pass on the password movie with a fuzzer that types from the alphabet (a
  vowel now and then), with the text full or empty one time in four, on every cell of the
  last column.
  - `password_delete`'s row is unreached, because no movie presses DEL. Its C is still
    compared, through `password_press_last_column`'s fuzz on DEL.
  - The paths only the fuzz runs are `map/static_code.txt` ranges.

## A on the password grid (password-3, 2026-11-23)

- **`password_press_a` ($8A:B339) is ported.** A, B, X or Y newly pressed on a pad (`TRB`
  takes the press off the pad) goes first to `password_press_last_column`. On a letter's
  column it types that cell of the grid's table at `$8A:C572` (12 bytes a row: `A-L`,
  `M-X`, `Y Z 0-9`, `. ? !`).
  - The column comes from where the cursor is (`$0D80`), the row from where it goes
    (`$0D90`), as in the ROM.
  - Its row has one hole: `$8A:B35B-B3A6` is another routine placed inside it.
- **That routine checks the typed text against a table of codes.** When a password does not
  decode, `$8A:B2E8` calls `$8A:B35B`. It compares the text with the strings at `$8A:CDAB`
  (stored with `XOR $9A`, each after its handler's address) and jumps to the handler of the
  one that matches (`JMP ($0182)`). No movie types one, so this is left for a movie that
  does.
- **Harness:** `call_password_press_last_column`'s model is split into a before-state and an
  exit (`pwLastColumnBefore`/`pwLastColumnExit`), so `password_press_a` reuses it. That
  changes existing lines in `tools/bench/routines.c`, so the gate is the full one.

## The password read back (password-4, 2026-11-23)

- **Ported in `src/password.c`:**
  - `password_decode` ($8A:BCEA): the 256-bit number is cleared, then every character
    but the last two is pushed in from the end, five bits each: its index in the alphabet
    at `$8A:BC5F` (`BCDFGHJKLN?PQRSTVWXYZ0123456789M`). `func_8ABBF4` sums the checksum
    D+$92 on the way;
  - `password_checksum_ok` ($8A:BD25): the last two characters (index + index * 32) against
    D+$92 masked to 10 bits.
  - The ROM searches the alphabet until a byte matches. For a character outside it, the
    search runs on through the bank: the farthest any grid character goes is 1,498 bytes
    (`I`), still in bank `$8A`. The C searches the same way.
  - A text shorter than three makes the index negative: the ROM reads below `$0D40`
    through the bank's WRAM mirror, and the C does too.
- **Check:** each has one live sample on the password movie and a fuzzer. The fuzzer types
  0-21 characters, mostly from the alphabet with the grid's others now and then, and makes
  the checksum match half the time. The mismatch (`SEC`, `$8A:BD6A`) runs only in the fuzz:
  it is a static range now.
- **shop-21's open question, answered:** `difftest.c`'s `checkEntries` starts no sample for
  a waiting row while another one records the interrupts ("one interrupt recorder at a
  time"). That rule is by design, so a waiting row must not enclose other waiting rows.
  For the password screen's loop (`$8A:B295`), that means a row would hide the samples of
  the waiting rows it calls. It gets no row, as shop-21's dispatch no longer encloses any.

## The password's fields unpacked (password-5, 2026-11-23)

- **`password_unpack` ($8A:BAFF) is ported** (in `src/password.c`). It is `password_pack`
  undone: the fields of the table at `$8A:CF61` come back out of the 256-bit number, last
  field first.
  - For each: `width` bits shifted out (`func_8ABB5C`, which also leaves the width's mask
    in D+$46), XOR the width's mask at `$8A:BC3D`, shifted into place (`asl_n`) and written
    over those bits of the field's word.
  - The first bit shifted in at the top is the caller's carry. That is clear on the one
    call, after `password_checksum_ok`; the C takes it as a parameter.
- **Check:** its one entry on the password movie is interrupted, so it is replayed from its
  entry state in the fuzz phase. Then 500 fuzz cases run over the big number, the fields and
  the carry. Exit: X `$FFFA` (N/Z of the last `DEX`), A the last field's word, Y the first
  field's shift, C of the last `asl_n` (the mask shifted).

## Two leaves from the password movie, and a harness limit (roots-113, 2026-11-23)

- **`text_centred_line_at` ($82:8911)** is `text_centred_line` for string X in palette A, in
  the window's plane.
  - The ROM pushes `$837E` and pulls one byte for the data bank, then the other at its
    end, so it returns with DB = `$83` and N set by that `PLB`.
  - Its callers do not read DB after it, and the harness does not compare the exit DB, so
    the quirk is only written down, in `src/text.h`.
  - The password movie runs it 110 times. It has a fuzzer from `text_centred_line`'s.
- **`money_skip_bonus` ($8A:C355)** had C but no row. The password movie runs its entry 25
  times, with `$03DC` bit 12 clear. A fuzzer runs the bonus, the decimal adds of 75,000;
  that path is now a static range.
- **Not ported then:** the password screen's way in and out. The reason given here, a
  harness limit with another task's `$420D` write, was wrong: see password-6.

## Second entries of the results routines (roots-114, 2026-11-23)

- **A census over the three explored movies.** `map/census.txt` stays the 60k movie's.
  `tools/roots.py` over the explored movies' censuses adds new roots: `$84:A31E` and
  `$84:A34E` (32 entries each on the password movie), and `$82:884B` (a waiting routine).
- **`$84:A31E` and `$84:A34E` are second entries** of `result_slot_step` and `result_tally`,
  past their `LDX $0444`. `$80:8AA4` calls them with X counting up to `$0444`: with a
  password, the tracks before it are tallied.
  - The C gets `result_slot_step_at(track)` and `result_tally_at(track)`. The two old
    functions now call them with `$0444`, and the shared adapter takes X from its caller
    for the second entries.
  - Both pass on the password movie (32 samples each and a fuzzer). The old rows still pass.
- **Gate:** the full one: the old functions and their adapter changed.

## The race paused (roots-115, 2026-11-23)

- **`race_pause` ($82:884B) is ported.** Start in a race (`$82:86BB`) calls it.
  - While it lasts, `$0026` bit 4 is set and `$03E0` = -4. The line word at `$7E:4E4D` is
    saved and cleared.
  - "PAUSE" (string `$88`) and a blank (string `$8C`) take turns every `$40` frames
    (`text_centred_line_at`, palette `$4E61`) until Start is pressed again. Then the race's
    line (string `$0233` * 2) is put back.
  - L and R held together quit the race. The ROM leaves its caller for `$82:877F` there.
    The C returns true instead, for its caller to do the same. No movie runs it.
- **Its row** is a waiting row with a budget (`maxInsns` 40M, as `title_card_show`'s). On
  the password movie it has 31 entries: 5 compared and passed on every path, 26 abandoned
  as longer pauses. It has two holes, never run: `$03E2` bit 2 and the quit.
- **It encloses `wait_frames_82`'s samples** while it waits (one interrupt recorder at a
  time). On the password movie that row drops from 92 compared samples to 3 and still passes
  on every path. The full gate pools the four movies' evidence.
- **The harness model** of `text_centred_line`'s C/V is now one function,
  `textCentredLineCV`, used by its row, `text_centred_line_at`'s and this one.
## The password screen's way in and out (password-6, 2026-11-23)

- **Ported:** `password_screen_open` ($8A:B1A5) and `password_screen_close` ($8A:B1E2),
  with their HDMA steps `password_hdma_off`/`on` ($8A:B18C, $8A:B197).
  - Open: fade out; the password screen's tiles (`$88:8601` to VRAM `$1000`), map and
    palettes (`func_8ABA55`, then `$85:FA37` into colours `$60-$7F`); fade in. D+$8E is 0
    while it loads, and `$0D7C` (the font select) counts up.
  - Close: the same back to the menu's tiles (`$96:EE52` to VRAM `$4000`,
    `func_8AB994`), with `$0D7C` = 0.
- **roots-113 was wrong about them.** The extra `$420D` write is theirs, not another task's:
  `STZ $420C` and `STA $420C` with a 16-bit A also write `$420D` (0). The C now writes it
  too (`platform_set_rom_speed(0)`). A print in `difftest.c`, not committed, showed no
  `$420D` write landing on the other waiting rows of that screen.
- **Rows:** the two screen routines are waiting rows. The close has a budget (`maxInsns`
  40M); without it, its one sample was abandoned. Each passes its one sample on the password
  movie on every path.
  - The HDMA steps are only called inside them, where one interrupt recorder at a time
    hides their samples. They have no rows: their effects are compared as part of the
    screen routines'.
- **Gate:** the full one (new waiting rows).

## The buying's shared steps (shop-22, 2026-11-23)

- **Deduplication, no change in behaviour.** `shop_buy_part` (lists 1-5) and
  `shop_buy_upgrade` (lists 6-10) now share two static steps in `src/shop.c`:
  - `shop_panel_rise(height)`;
  - `shop_buy_finish(at, level, settle)`: the level byte, the upload, the money after the
    price, the panel's fall, the money stored and the settle.
  - This was owed since shop-19, which wrote the rise apart to keep that unit additive.
- **Check:** every buying row passes on the buy and password movies as before. The gate is
  the full one, because existing lines of `src/` changed.

## A carried row may match older commits (harness-10, 2026-11-23)

- **The bug:** shop-22's commit failed CI in `tools/test_carry.sh` ("additions (rows
  carried)").
  - `progress.py`'s `row_status` took the oldest recent commit whose hash matched the row,
    and required only additions since it.
  - After a full run that changed other files, shop-22's `src/shop.c` here, a row whose own
    files did not change also matches commits from before that run. The oldest of them sees
    the deletion, so every such row turned stale at the next additive change: the next
    `verify_new` unit would have been refused.
- **The fix (the advisor's domain, my call, open to veto):** take the oldest matching commit
  that has only additions since. The row's code there equals the code its run compared.
  `test_carry.sh` passes.
- **Check:** `tools/test_carry.sh`, `tools/test_merge.py` and `progress.py --check`. No
  routine's code or row changed, so no verification run.

## Lists 6-10's level getters, and list 10's level (shop-23, 2026-11-23)

- **Ported:** the five getters of the `$80:9EEA` table for lists 6-10. Each is
  `LDA $039x,Y; RTS`, the level byte as it is (the row + 1), with no `+ 1` as lists 0-5's
  have:
  - `shop_part_level_8` ($80:9F30, `$0398`, list 6), `_9` ($80:9F2C, `$0399`, list 7);
  - `_a` ($80:9F28, `$039A`, list 8), `_c` ($80:9F34, `$039C`, list 9);
  - `_b` ($80:9F38, `$039B`, list 10).
- **A fix:** `shop_list_level` in the C read `$039C` for list 10. The table's entry 10 is
  `$80:9F38`, so it reads `$039B`. It now calls the getters.
  - `shop_level_marks`' fuzzer covers list 10 too: that runs `$80:9F38`, now a static range.
  - The table's entries past 10 point at `$80:9F3C` (`LDY #0; RTS`, A unchanged); no list
    uses them.
- **Check:** `_8`, `_9` and `_a` pass on the password movie (72, 34 and 57 samples, and
  fuzz). `_c` runs only in the TAS and `_b` nowhere: their rows are unreached.
  `shop_level_marks` passes. The gate is the full one.

## The shop panel's frame (shop-24, 2026-11-23)

- **`shop_panel_frame` ($80:B149) is ported.** It is the shop panel's per-frame callback,
  which `shop_panel_blank` puts at D+$EC; the explored movies' census found it (456 entries on
  the password movie).
  - While the icon animates (D+$44 negative, counting down to `$8000`), it copies the
    frame's six words from `$7E:4EDA` on. While the text is still typing, it also redraws
    the icon, the level marks and the labels.
  - Once the text is typed, it runs a typewriter step and draws the panel's static: random
    tiles in the panel map at `$7F:00C4` and random words at `$7F:0710`. Both are uploaded
    (`func_80B537`), and D+$4E counts the frames.
- **Check:** it passes on the password and shop movies (173 and 578 samples, every one of
  its 159 paths). No flag is live after it (an indirect callback): the C differs on C
  there, which is not compared. It has no fuzzer yet, since its callees need the shop's
  pinned states.
- **Gate:** incremental.

## The gate made faster (harness-11, 2026-11-23)

- **Why.** About nine hours of 2026-11-23 were gate wall time: full gates took 2,500-2,700
  s and incremental ones about 1,000 s, even for one row. The owner asked for a faster
  pace ("a gente diminuiu o ritmo ao extremo só fazer testes"). They also said the gate
  rules were mine to set ("as regras foram você mesmo quem definiu"); these choices are
  mine, made with the advisor.
- **Measured first** (race1 movie, 9,450 frames, one process):
  - plain emulation took 84 s. The profile (`gprof`) put about 67% in the PPU drawing
    pixels (`ppu_getPixel`, `ppu_handlePixel`, `ppu_getWindowState`);
  - the selftest's all-rows difftest (fuzz 200) took 621 s and its negative rows 103 s:
    with four serial emulations, that is the ~16-minute floor of every gate.
- **What changed (no verification rule changed):**
  - **The bench draws only for `--dump`** (or the new `--render`). LakeSnes' `ppu_runLine`
    returns after sprite evaluation (the range and time-over flags still run), at the
    point its own comment marks for frameskipping. `skipRender` is a new `Ppu` field, not
    part of the state; the patch is in `tools/bench/patches/`.
    - Emulation is 84 s → 29 s.
    - The per-frame WRAM hashes are identical with and without drawing on race1 (9,450
      frames) and on the password movie (29,000).
  - **Two per-instruction scans of the whole row table** in `difftest.c` now look only
    where needed:
    - `checkEntries` first tests a bitmap of the rows' entry PCs;
    - `watchAll` steps a list of the active flag watches. A watch is only deactivated
      inside its own step there, so it leaves the list at once and cannot be listed twice.
    - With the drawing change, race1's all-rows difftest is 621 s → 199 s, and its result
      file is identical but for the hash column.
  - **`selftest.sh`:** steps 1-3 run side by side, and the all-rows difftest runs in
    `SELFTEST_SHARDS` slices (default 6). The selftest went from ~1,000 s to 149 s.
    - `SELFTEST_DIFFTEST=0` skips the rows (34 s). The full gate uses it: `verify_all`
      runs every row with fuzz 500, and the negative rows. All 287 rows race1 reaches are
      verified over the four movies.
    - The full gate runs the selftest beside `verify_all`, not after it.
- **Check:** the full gate. Its `map/verified.txt` equals the last one's in every field
  but the hash and the full-run line, for every row but one.
  - That row is `shop_panel_frame`: 578 compared samples in shop-24's incremental run,
    467 now, and still verified on all 159 paths.
  - Shop-24 verified it alone. In a full run, other rows record interrupt handlers, and
    `checkEntries` hides a non-`rti` row's entries inside a recorded handler. The NMI calls
    `shop_panel_frame` through `$01EC`, so some of its entries fall inside one. (Corrected
    in shop-26: an earlier version blamed "one interrupt recorder at a time".) So the
    difference is not harness-11's.
- **The full gate took 684 s** (it was 2,510-2,680 s): 3.8 times faster.

## `verify_new` skips movies that never reach the rows (harness-12, 2026-11-23)

- **What.** A movie that never executes any named row's entry gives the merge no evidence:
  it is skipped.
  - Each movie's coverage (`bench --coverage`, one plain run) is cached in `build/cache/`.
    The cache key is the movie, the emulator patch and `bench.c`; a stale or missing file
    is made again, side by side with the others.
  - The row's entry comes from `tools/bench/routines.c`. The names are checked there too,
    since the main movie may now be skipped.
  - When no movie reaches the rows, the main movie still runs once to record them as
    unreached.
- **Why it is safe:** the merge (harness-8) takes evidence only from movies that compared
  something. A movie whose coverage never hits the entry starts no sample for the row; a
  fuzz phase needs live entries.
- **Measured:** `verify_new password_unpack` (a password-only row) took 103 s with the
  caches built (three movies skipped), and 290 s building them. Before, it ran all four
  movies.
- **Check:** that run left `map/verified.txt` unchanged; `sh -n`. This is the advisor's
  idea and my call, open to veto.

## What is left, split three ways (2026-11-23)

- **16,362 code bytes are outside verified rows** (of 71,889). `rom_map`'s code includes
  the TAS's coverage, while difftest runs only on bench movies. Each byte is tagged with
  the four movies' coverage (`build/cache/*.cov`):
  - **A, 601 bytes:** C exists, but its row is unreached or missing. These need only a
    movie: `shop_ring`, `shop_item_view_f`, lists 5, 7 and 9.
  - **B, 12,393 bytes:** no C, and a bench movie runs them. They can be ported now.
    - The largest runs: `$80:BC77` (1,645; the NMI handlers around `$80:B97B`) and
      `$80:D168` (880, parked: a stack-dropping exit).
    - Then `$80:EE56` (871, the ticker), `$80:E4E6` (551), `$80:BA6C` (520), `$8A:94C3`
      (402), `$80:8BC5` (379), `$82:84C8` (375), `$80:9C47` (373).
  - **C, 3,368 bytes:** no C, and only the TAS runs them. They need a movie first. Most
    are the menu task's handlers (`$80:C5E0`: `$80:C6D4`, `$80:CEE2`, `$80:CA35`, ...).
- **Order (with the advisor):**
  - A first, picking the state from the tables (`$036E` via `$85:D590`), not by blind
    exploring;
  - then B, leaf first;
  - the hardware screen roots batched into one full gate;
  - C movie by movie: one short movie per family of the password screen's codes. INFODATA's
    `$03E2` bit also opens `race_pause`'s hole.
  - The menu task `$80:C5E0` waits: it is entered with D = `$0200` as well as `$0100`, and
    most of its handlers are C bytes.

## Lists 0 and 6-10's icon handlers (shop-25, 2026-11-23)

- **Ported as functions with rows:** `shop_item_icon_0` ($80:A096; a fixed icon, no
  upload), `_6`, `_7`, `_8` ($80:A121, A0E0, A09F), `_9` ($80:A162) and `_10` ($80:A1A3).
  - Lists 6-8 were inline in `shop_item_dispatch` before.
  - **A fix:** lists 9 and 10 fell into its `default` and did nothing, while the ROM
    draws their icon. The dispatch's fuzzer and adapter now cover lists 0-10 (list 9's level
    byte is `$039C`, list 10's `$039B`).
  - List 10's handler runs only in the dispatches' fuzz: a static range.
- **Flags:** these rows have under 100 returns each, so every flag counts. The draw path's
  exit is modelled from `shop_item_icon_draw`:
  - C and V of the group's last `ADC #6`, else C of the `CMP #$10` of the step * 2;
  - a negative step leaves by its `BMI` (N set);
  - else N/Z of the new step, C of its `CMP #$F` / `#7`, and V of `BIT D+$76` at step 7.
  - Lists 1-5's rows keep their older model; only V is live there.
- **Check:** `_0`, `_6`, `_7` and `_8` pass on the password movie (594, 73, 34 and 58
  samples, all paths, 500 fuzz cases each). `_9` (TAS only) and `_10` (no run) are
  unreached. Both dispatches pass on the three shop movies. Full gate.

## The INFODATA movie (explore-4, 2026-11-23)

- **The password screen checks codes.** When a password does not decode, `$8A:B35B`
  compares the text with 17 strings at `$8A:CDAB`, stored with `XOR $9A`, and jumps to the
  matching code's handler:
  - 13 star names (`MERAK` ... `KRAZ`) set the track `$0444` (0 ... `$2A`);
  - `DISPLAY CREDITS` sets `$03DC` bit 15;
  - `MICK4CLAUDIA`, `MICKSMELANIE` and `INFODATA` set `$03E2` bits 0, 1 and 2;
  - `RELAX` runs a mosaic and another screen.
  - No run reached any of them, not even the TAS.
- **`tools/bench/movies/code_infodata.txt`** (11,500 steps) is the fifth movie of
  `verify_all` (one slice) and `verify_new`. It is built from parts that already work:
  - `explore_pw.txt`'s way to the password screen;
  - `INFODATA` typed with `tools/password_movie.py`, then EXIT. The game sets `$03E2`
    bit 2 and goes back to the menu, cursor on PASSWORD;
  - Up x3 to 1 PLAYER FULL, A, then `race1.txt`'s way into a race, shifted;
  - Start (`S`; `s` is Select) to pause, and Start again.
- **It runs what nothing did:**
  - the codes' check and INFODATA's handler (`$8A:AC013`);
  - the race loop's `$03E2` bit 2 path (`$82:86C6`);
  - `race_pause`'s `$03E2` bit 2 path (`$82:8860-886D`). That hole is gone; the quit stays
    a hole.
- **Its coverage is in the map:** code grows from 71,889 to 72,062 bytes. Five static
  ranges it executes left `map/static_code.txt`.
- **Next:** port the codes' check, the handlers' simple cases, and the race loop's new
  path once its row exists.
- **Gate:** the full one.

## The password codes (password-7, 2026-11-23)

- **`password_code_find` ($8A:B35B)** returns the handler of the code typed at `$0D40`, or
  0. D+$82 keeps the last handler word tried. Its string compare (`$8A:B37B`) is part of it.
  - Its row ends at the `JMP ($0182)` into the handler (`endAt` `$8A:B3A4`, as shop-21's
    dispatch), after the `JSR` that pushed `$8A:B3A1`; the adapter writes that push.
  - The `SEC; RTS` at `$8A:B3A2`, after the handler returns, is outside the sample and
    outside the extent.
  - A fuzzer types a code (any but RELAX) one time in two, sometimes one bit off, else the
    password alphabet.
- **The handlers:**
  - `password_code_infodata` ($8A:C013, `$03E2` |= 4) has a row; the INFODATA movie runs it;
  - in C without a row, since no movie runs them: the star names' tail
    `password_code_track` ($8A:BE28), `password_code_credits` ($8A:C004), and
    `password_code_mick4claudia`/`micksmelanie` ($8A:C01D, C018).
  - Not ported: RELAX ($8A:BD9D) calls `$8A:BF23` and `$8A:BF0D`, which have no C, and so
    no C dispatch of the handlers yet.
- **Check:** both rows pass on the INFODATA movie (one live sample each, 500 fuzz cases).
  Incremental gate.

## The panel's picture handlers (shop-26, 2026-11-23)

- **The inventory first** (with the advisor): every B run (no C, run by a bench movie),
  with the census entry around it. Of the 12,244 B bytes, the twelve largest runs hold
  6,847; the rest are smaller leaves, such as these handlers, the typewriter's control-code
  handlers (`$80:B6F2`), `$8A:A2AF` and `$8A:A419`.
- **Ported as functions with rows:** `shop_item_dispatch_panel`'s seven handlers, inline in
  its `switch` before:
  - `shop_item_panel_1`, `_2`, `_3`, `_5`, `_4` ($80:9C47, 9C85, 9CC3, 9D02, 9D41; two
    picture uploads each);
  - `_0` ($80:9D80) and `_6` ($80:9D9E, lists 6-10; one upload, a `JMP` to `func_80B537`).
  - Their rows declare `func_80B537` as part of them (`alsoIn`).
  - Exit: the last `func_80B537`'s (A the next index, X the index + 2, C/V of its `ADC`),
    and Y = D+$5C + the last upload's VRAM offset.
  - The fuzzer pins the ring's base to a multiple of `$40`, as the panel dispatch's does;
    an unpinned base let the queue overwrite the stack.
- **Check:** all seven pass on the pooled shop, buy and password movies (3-32 samples
  each, 500 fuzz cases, every path). Full gate, since the dispatch changed.

## The refund's row (shop-27, 2026-11-23)

- **`shop_money_after_refund` ($80:990D) had C since shop-17 but no row.** The buy and
  password movies run it (6 entries).
  - It ends with a `JMP` into `shop_money_after_price`'s tail (`$80:997E`), declared in
    its row as `alsoIn`. Its exit is that tail's.
  - The C gets `shop_money_after_refund_flags` (as `shop_money_after_price_flags`), which
    the old entry now calls.
  - A fuzzer from `shop_money_after_price`'s, with A a row of the list.
- **The inventory, corrected.** It had ignored rows' `alsoIn` ranges, so code reached by
  tail jumps counted as without C: the typewriter's control-code handlers (`$80:B6F2` on)
  are `text_type_step`'s. Counted again, B is 11,297 bytes.
  - `progress.py` likewise counts only `[entry, end)` minus holes. `alsoIn` code is
    compared in its rows' samples but not path-checked there, so it is not counted as
    verified. That stays.
- **Not ported:**
  - `$8A:A419`: a menu item chosen with A/B/X/Y jumps to the menu's callback (`JMP
    ($015C)`, an open set);
  - `$8A:A2AF`, which calls it.
  - Both wait for a design of indirect callbacks, as RELAX's codes do.
- **Check:** both rows pass on the buy and password movies. Full gate.

## The menus' NMI frame (nmi-1, 2026-11-23)

- **`nmi_menu_frame` ($80:B97B) is ported**, up to the two tasks' stack switch at
  `$80:BC54` (`TSC`/`TCS`; with the advisor). That is 729 of its 800 bytes.
  - Its row is an interrupt row (`.rti`) with `endAt` `$80:BC54`, as `nmi_prologue`'s.
  - The tail (the stack switch, the frame callback `jsr ($01EC)`, `nmi_tail`) has no row.
- **What it does:**
  - forced blank, and the fade at `$0296-$029A` (as `nmi_prologue`'s);
  - both pads' new presses (`$000C` to `$010C`/`$010E`, `$0010` to `$020C`/`$020E`), and
    the frame's task (`$0190`/`$0290` by the counter's bit 0);
  - that task's upload ring drained against a `$0C00` budget (task 1 at `$018E-$018C`,
    task 2 at `$028E-$028C`, whose index keeps its `+ $0100`), sending part of an entry
    that does not fit;
  - the shadowed screen registers `$019C-$01C1`. The vertical scrolls are less
    `$01C2`/`$01C4`, byte by byte as the ROM does;
  - the palette (`$81:0500`) and sprites (`$81:` + `$016E`), and the raster IRQ (line
    `$6C`, dot `$10`, `$80:BC9B` through the trampoline);
  - the large upload at `$0164-$016A` in `$1C00`-byte parts. The bank is read after its
    carry is added, as the ROM does;
  - the brightness `$019B` (0: forced blank).
- **Check:** the password movie gives 2,661 live samples with every path (302), and 500
  fuzz cases. No flag is live after it. Full gate.

## Fuzz and replay keep interrupts off (harness-13, 2026-11-23)

- `nmi_menu_frame` writes `$4200 = $B1` itself, enabling the NMI and the IRQs. A fuzz case
  that crossed the start of vblank after that write was interrupted and lost, 37-84 of 500.
  - The fuzz already starts with interrupts off ("run on the interpreter from the entry,
    interrupts disabled"). It now keeps them off at every instruction, in fuzz and
    replay: the PPU's NMI and IRQ enables, and the CPU's pending requests.
  - The pending requests matter because a `$4200` write in vblank raises an NMI at once.
- The write itself is still compared, as a byte register.
- My call, open to the owner's veto. Full gate.

## Names from public cheat lists (docs-1, 2026-11-23)

- **The owner's idea:** look for public documentation. There is no public RAM map, ROM map
  or disassembly of Top Gear 3000. Public cheat lists, though, give RAM addresses with
  their meaning (Pro Action Replay codes are an address and a value). They are facts about
  the game; nothing is copied from them, and they agree with what the ROM showed:
  - `$0393-$0397`: Engine, Tires, Gearbox, Armor, Boost, which are lists 1-5's level
    bytes. Their "Max" value `$85` is level 5 with bit 7 ("bought here") set;
  - `$0398` Jump, `$0399` Attractor, `$039A` Warp, `$039B` 'P', `$039C` Infra-Red: lists
    6, 7, 8, 10 and 9. List 0 is the shop's Paintshop (`$0392`, the colour);
  - player 2's copies at `+ $2F` (money `$03BF`/`$03C0`): the car offset D+$94;
  - in a race: fuel `$0258`, boosts `$025C`, the boost flag `$0232`, attractors `$0250`,
    warps `$0251`, the bonus `$027C`, points `$041C`.
- **The codes:**
  - INFODATA is the cheat mode: in a race, Select shows the circuit number or a sound-test
    index on the speedometer. Those are the `$03E2` bit 2 paths explore-4 opened;
  - MICK4CLAUDIA unlocks every power-up, and DISPLAY CREDITS shows the credits.
- **Used for:** the shop's list names, now in `src/shop.h` and `map/symbols.txt`'s
  comments. This is a comment and documentation change only, so no row changes.
- **Sources:** almarsguides.com's Top Gear 3000 Pro Action Replay and Game Genie pages;
  GameSpot's and Critical Hits' cheat pages; The Cutting Room Floor's (a stub: a hidden
  sound test, the cheat mode).
- **The KRAZ code**, tried as a movie (not wired): it jumps to a later championship and
  reaches its shop with 2,325,000 credits, running about 500 new bytes. The explorer, even
  aimed at the race setup, did not leave the shop in 40 segments.

## The second task's frame, and the WRAM port (nmi-2, harness-14, 2026-11-23)

- **`irq_menu_frame` ($80:BC9B) is ported**, up to its stack switch at `$80:BDC1`. It is
  the raster IRQ that `nmi_menu_frame` sets up (`$80:BC9B` through the trampoline at
  `$0005`), and it gives the second task its half of the screen.
  - It sets the line interrupt at dot `$10A` (`$4200 = $91`) and waits for it twice
    (`WAI`).
  - The screen is blanked. The second task's shadowed registers (`$029C-$02C1`) take
    effect: the vertical scrolls less `$74`, but the window logic from the first task's
    `$01B2` (the ROM reads `$01B2` here).
  - Its sprites (`$81:` + `$026E`) and palette (`$81:0700`) are sent; then its brightness
    `$029B` (0: forced blank) and `$4200 = $B1`.
  - The sprites' DMA is kicked by the high byte of a 16-bit store at `$420A` (VTIMEH, then
    MDMAEN).
  - The C skips the two `WAI`s, as `irq_split_bottom` does.
- **Its row** is an interrupt row with `endAt` `$80:BDC1`, without a fuzzer: the fuzz keeps
  interrupts off (harness-13), so a `WAI` would never wake. The password movie gives 2,661
  live samples, every path (135).
- **harness-14, the WRAM port** (my call, open to veto). The frame reads its registers
  through `$2180` from the address it sets at `$2181-$2183`.
  - `$2181-$2183` are now byte registers, compared by their last value; the C sets them
    with the new `platform_set_wram_address`.
  - A read of `$2180` is not a hardware access. It is a WRAM read at that address, which
    the C makes directly. The port's auto-incremented address is not compared.
  - `platform_set_irq_v_high` sets `$420A` alone.
- **Full gate:** the harness changed.

## The results screen (screen-1, 2026-11-23)

- **`results_screen` ($8A:94C3, long) is ported in two parts** (in `src/misc.c`):
  - the load: text, image, NMI hook, graphics, palettes, the cleared `$7F` map, the stars,
    the colour HDMA, the typed text uploaded;
  - the show (`results_screen_show`, $8A:95EB): a frame at a time, the map to VRAM `$4000`,
    the stars and the sprites. A or Start begins the fade out, and it returns at 0.
- **Why two parts (with the advisor):** the load runs for many frames with no wait (three
  decompressions, a `$3800`-word clear, `clear_vram`), and about 14 NMIs land in it. The C
  replays recorded interrupts by position, at its waits, so one waiting row over the whole
  screen fed the loop the load's NMIs. It saw no fade and left after one frame. This was
  not a harness limit.
  - The load's row ends at the first wait (`endAt` `$8A:95EB`) and is not a waiting row.
    Its four live entries are interrupted, so it is verified by replays from their entry
    state, on all 107 paths. It compares nothing the NMIs do during the load.
- **The show had no row in screen-1.** On the 60k movie 3 of its 4 samples passed. In the
  fourth the C took one frame more (157 uploads against 156). screen-1 guessed an IRQ after
  the NMI; that was wrong. It is an NMI in the loop body (a lag frame, harness-15 below).
- **Check:** incremental gate (`verify_new results_screen`).

## Lag frames in waiting rows (harness-15, 2026-11-23)

- **What was measured:** every interrupt a waiting sample recorded, on all five movies, with
  the last instruction before it.
  - Nearly all land in a polling loop: `wait_frames` and its copies, the shop's `$4E`,
    `$8C`/`$8E` and `$64` polls, the fade polls.
  - The rest stop the routine's own work: decompressions, `text_centred_line`, clears,
    `split_screen_setup`'s table loop, `race_pause`'s body and the results screen's loop body
    (`$8A:9C61`, inside `hdma_table_0d00_settle`).
  - Such an NMI is a lag frame. The original does a frame's work late, and a frame loop can
    run one pass fewer. A port that waits in step with the frame has none.
- **Lagged samples (from the original's run alone):** a waiting sample one of whose
  interrupts came while the last instruction lay outside the wait sites. The sites are a
  table in `difftest.c`, from the poll's target read (the `ADC $0024`) to its branch. After
  the `CLC` or the `JSR` the NMI still counts as lag.
- **The rule: a lagged sample may pass, but may not fail the row.**
  - Lagged and passing: counted as before.
  - Lagged and failing: counted apart (`lagfail:` in the evidence), its paths and flags not
    credited. It fails the routine only when no movie has a sample without lag that passed
    (`cleanpass:`, pooled in `merge_verified.py`). A missing wait site therefore cannot excuse
    a row: all its samples would be lagged.
- **Why not skip every lagged sample (the first plan, with the advisor):** the counts showed
  it would leave `title_card_show` (5 of 5 lagged) and both password rows (2 of 2) with
  nothing compared, and drop 45 of 55 `queue_unpacked_upload` samples. All of those pass
  today. They also show a weaker point: a C that replayed fewer interrupts than were
  recorded still passes (the rest are applied after its last wait). Those rows pass on
  lagged samples only, and are candidates for the first-wait cut later.
- **Why not fold a lag interrupt into the next wait:** the ported `wait_frames` sets a target
  from `$0024` and polls for it, so two NMIs in one wait jump past the target. Folding would
  also need this rule as its fallback.
- **`results_screen_show` ($8A:95EB) now has its row:** on the 60k movie 3 samples pass, 1
  lagged one fails (the lag frame at `$8A:9C61`), 49/49 paths from the passes.
- **Check:** full gate; every other row as at HEAD, all negatives still fail.

## The title screen (screen-2, 2026-11-23)

- **`title_screen` ($8A:800B, long) is ported in three parts** (in `src/misc.c`), cut at its
  first two waits:
  - the load: the image, the NMI hook, cleared DP scratch and VRAM;
  - the scene (`title_screen_scene`, $8A:8042): the track preview, gradients, the mode 7
    planes and scene, the sprites, the title HDMA (slot 6, indirect, `$7E:2C00`), the
    palettes;
  - the show (`title_screen_show`, $8A:80C8): HDMA and NMI on, then a frame at a time until
    D+$58 is set, or D+$8C with D+$9A at 0; then 8 frames.
- **Why the second cut:** with the scene inside the show, all four samples were lagged
  (NMIs in `title_scene_load`'s decompressions) and failed, so harness-15 failed the row.
  The scene's own samples are lagged too but pass (their late interrupts are applied after
  its one wait); the show's are clean.
- **The unrun block** `$8A:80F8-$8A:8141` (D+$5A: one of three VRAM uploads from `$7E`) is
  decoded and in the C, and is the show row's hole.
- **Results:** 60k movie 4/4 for each part (the load by replays), buy movie 1/1.
- **Also:** `bus_rd8`'s index starts at 0 (a GCC may-be-uninitialised warning, no change in
  behaviour).
- **Check:** full gate (new waiting rows).

## The ticker screen, and interrupts recorded as changes (screen-3, harness-16, 2026-11-24)

- **`ticker_screen` ($80:E4E6) is ported in two parts** around its first wait, which is
  inside `split_screen_setup` (a row of its own):
  - the load: the image, three decompressions, the raw VRAM DMAs written as stream setups
    (high bytes from `$7F`, low bytes filled from `$81:CBF8`, seven `$11`-byte rows), four
    indirect HDMA channels;
  - the show (`ticker_screen_show`, $80:E5FA, after `split_screen_setup`): the sprites, the
    ticker record, three MVNs into bank `$81`, the name and car blocks at `$7F:2000`, then a
    frame at a time (the zoom in, or the pass) until D+$9A's low 15 bits are 0.
- **Why the show starts after `split_screen_setup`:** started at the JSR, two of its four
  samples were lagged (NMIs in that routine's sprite-buffer clear) and failed. Their paths
  were not credited, so the row failed on paths. The 3-byte JSR at `$80:E5F7` is in no row.
- **The carry:** the show returns the carry the original leaves. That is the last
  `ticker_scene_step`'s, which now returns its own carry: the computation moved from its
  adapter into the C, and the row passes as before.
- **SEI at the exit (`$80:E87D`):** written as a comment, as the CLIs in `split_screen_setup`
  and the race NMI. The I flag is not part of the platform calls, and the bench does not
  compare it.
- **The unrun paths, decoded and in the C:** D+$76 bit 5, `$03A8` from 10 up (two copies,
  and a still-view loop at `$80:E88E` past the RTS, as `alsoIn`), D+$84 below `$FE3E`.
- **harness-16:** a waiting row's interrupts are recorded as the WRAM bytes each handler
  changed, not as two whole copies of WRAM. The replay applied only the changed bytes
  anyway, so behaviour is unchanged. The cap goes from 512 to 8192 interrupts per sample:
  the show records about 1,000 (an NMI and a raster IRQ per frame) and was abandoned at 512.
  The static 128 MB of copies is gone.
- **Results:** 60k movie: load 4/4 (by replays), show 4/4 with 292/292 paths. Buy movie:
  1/1 each.
- **Check:** full gate (a harness change and a new waiting row).

## Flags at a cut (harness-17, 2026-11-24, with the advisor)

- **The problem:** a row cut inside a routine (`endAt`, the screens' load rows) with fewer
  than 100 watched returns counts every flag as live. At such a cut the code after it is
  written over by the next instructions (`$80:F115`: `LDA #1`, then `CLC`, `ADC $0024`), but
  the rule made the adapter model flags nothing reads (V after `func_80F745`, say).
- **The rule:** for a sample that ended at its `endAt`, the watch notes each flag written
  while still in the straight-line code after the cut. Straight-line code ends at a
  conditional branch, an indirect jump, a return, BRK/COP, PHP/PLP, or an interrupt. A flag
  every watch found written there (`sdead:` in the evidence, pooled over the movies) is
  dead at the cut, even below 100 returns.
  - This is sound because the code after a cut is the same every time: up to its first
    branch, one watch has seen all of it.
  - Returns keep the 100-return rule: one watched caller says nothing about another. A
    dispatcher whose `endAt` is its RTS stops being straight at once, so it keeps the old
    rule too.
- **What changed:** the cut rows `results_screen`, `title_screen` and `title_screen_scene`
  now have no live flags. `ticker_screen` and `hud_markers_write` keep V and C, which the
  code after them does not write before a branch.
- **Negative row `ticker_screen_wrong_carry`:** the ticker load with C set. C is not written
  before `split_screen_setup`'s first branch, so it stays live, and the row fails on its
  two replayed samples.
- **Check:** full gate. Every row's status is as at HEAD, and every negative fails.

## The intro, and long rows' logs (screen-4, harness-18, 2026-11-24)

- **`intro_screen` ($80:EE56) is ported in two parts**, cut at its first wait:
  - the load: the image, five decompressions, the mode 7 characters and map (raw DMAs to
    the high and low bytes, an overlapping 16-bit copy with DB = `$7F`), VRAM words, seven
    MVNs into bank `$81`, the stars, mode 7, six HDMA channels;
  - the show (`intro_screen_show`, $80:F115): a frame at a time, the map upload, the
    background colour, the BG2 scroll, the HDMA lists, one step of D+$3E's table, the
    stars, the typed text, until the fade is done.
- **The table step:** `$80:F1E3`'s 33 handlers (all verified) are a switch in
  `intro_step`, with their carries as constants: clear for the zoom and slice steps, set
  for the frame and fade steps, unchanged for the empty steps and `title_scroll_start`.
  - C is live in every handler's row, so each constant is verified.
  - `title_stars_spread` takes the carry the ASL of D+$3E leaves.
- **Exit state:**
  - The load leaves DB = `$81` (its `PEA $817F` and two PLBs; the bench compares DB). Its
    flags are dead after the cut (harness-17).
  - The show returns Z from its last TRB of `$0026` bit 2, and V from the last pad read.
- **The unrun path:** a button's fade (`$80:F1BD-$80:F1C9`), because the movies let the
  intro run out; it is the show's hole.
- **harness-18:** a row with a budget (`maxInsns`) keeps up to 65,536 effects and 524,288
  input reads. The 2,553-frame show makes thousands of effects and about 70 serial pad
  reads a frame.
  - The logs moved out of the sample, into per-depth buffers, so the other rows keep their
    caps (512 and 256) and their cost.
  - The C is served each register's reads through a cursor, not a scan from the start.
  - A repeated input value is recorded again after an interrupt: a frame's own read. Only
    the repeats inside a polling loop are one input now.
- **Results:** the one sample, on the 60k and the buy movies: the load by replay
  (262/262 paths), the show 1/1 (83/83 paths).
- **Check:** full gate.

## The shop's item screen (shop-23, 2026-11-24)

- **`shop_item_screen` ($80:8EB4) is ported** (in `src/shop.c`). It is a list's item screen:
  it opens, then runs a frame at a time for good, left by the buy handlers and the task
  switch. On the list D+$F0 names it does a fade out instead.
- **Four rows, cut so that none encloses a waiting row.** The frame's pad input calls
  `shop_item_down`, `shop_item_up` and the buy lists, and its tail calls `shop_panel_clear`
  and `shop_item_slide_list_in`, all waiting rows. A waiting row around them would hide
  their samples (shop-21).
  - The opening: from the entry up to `shop_panel_clear` (`endAt $80:8F3B`). It is a waiting
    row for its two polls and its wait, and includes the fade-out path's RTS.
  - The frame's view (`$80:8F4D-$80:8F5D`), its drawing (`$80:8F64-$80:8F70`), and its tail
    (`$80:8F94` back to `$80:8F4D`, with the BRA at `$80:8F4A` as `alsoIn`).
  - Left out, about 60 bytes: the opening's last 15 bytes, the frame's `$0024` poll, and its
    pad input and wait.
- **Wait sites added (harness-15):** the opening's `$8C`/`$8E` polls, D+$4E up to 4, and the
  frame's `$0024` poll.
- **Results:**
  - The frame rows: 3,365 (shop), 485 (buy) and 791 (pw) samples, all passed.
  - The opening: the clean samples pass on the buy and pw movies, 3 and 2 of them. Its
    lagged samples fail only on D+$4E, the frame counter it copies, which a lag frame moves.
    On the shop movie every sample is lagged, so the opening is verified on the evidence
    pooled over the movies.
- **Check:** full gate (a new waiting row, and wait sites in `difftest.c`).

## The screens' bank $80 wrappers (screen-5, 2026-11-24)

- **`title_screen_run` ($80:FBA5) and `results_screen_run` ($80:FBDC)** call the bank `$8A`
  screens from bank `$80`. Each has two rows around its JSL:
  - **The prologue** ends at the JSL (`endAt`). Its PHB of the caller's DB is the only push
    left above the cut, and the adapter writes it. Then DB = `$8A` and D = `$0100`. The title's
    prologue also stores the track in `$1536` and decompresses `$9A:EB44` to `$7E:8000`.
  - **The epilogue** starts after the JSL and ends at the RTL (`endAt`): HDMA off, and the
    pushed DB pulled back (its N and Z).
- **The RTL itself is in no row** (one byte each). Because of the pulled byte, the bench
  cannot read the epilogue's return address at its entry.
- **Results:** the 60k movie 4/4 for each row (the title's prologue by replays), the buy
  movie 1/1.
- **harness-19, the selftest's row check:** the incremental gate failed on `shop_item_screen`
  (shop-23). The short movie reaches it on its fade-out path only (8 of 59 paths), and the
  selftest failed any row a movie reaches on part of its paths. The full gate skips the
  selftest's rows, which is why shop-23 passed.
  - Now the selftest fails a row only on a hard failure: `hard:1` in its evidence, meaning a
    compared mismatch, a lost case or a broken contract.
  - The path and flag rules count over all the movies together (harness-8), and only
    `verify_all` and `verify_new` judge them.
- **Check:** full gate (`selftest.sh` changed), and the selftest itself.

## The shop's main screen, the parts between its waits (shop-24, 2026-11-24)

- **`$80:8BC5`, the shop's main screen**, loads, waits, sets up, and then runs a frame loop
  that calls `shop_item_screen`. Five rows cover the parts with no wait in them (in
  `src/shop.c`):
  - `shop_screen_begin` ($80:8BC5): the image;
  - `shop_screen_graphics` ($80:8BD7): five decompressions, VRAM words 0-15 cleared;
  - `shop_screen_records` ($80:8C22): the car's record, three MVNs, ten colours;
  - `shop_screen_text_setup` ($80:8CB9): the text words;
  - `shop_screen_list_setup` ($80:8D1A): the fade in, the sprite group, the list panel.
- **Both players' tasks** run the records and the text words, with D = `$0100` and
  `$0200`, so those two take `dp`, as the census's one state hid. The 60k movie samples each
  ten times, five per player.
- **Left for later:** the waits between the parts (`split_screen_setup` and
  `queue_unpacked_upload`, each its own row; the `$64` polls) and the frame loop from
  `$80:8D9A`, which calls the item screen.
- **The unrun paths:** `$03DC` bits 8-9 clear or both set (`$80:8D40-$80:8D51`, the hole).
  The record a track has (`$80:8D5E`) runs on the 60k and pw movies only, and is pooled.
- **Check:** incremental gate (new rows only).

## The shop's main screen, its frame loop (shop-25, 2026-11-24)

- **`shop_screen` ($80:8BC5) is now in C whole** (in `src/shop.c`): the parts of shop-24, the
  waits between them, and the frame loop.
- **Four more rows:**
  - `shop_screen_frame` ($80:8DA0, after the frame's wait): the list's labels, the scroll
    toward the cursor, the sprites;
  - `shop_screen_after_items` ($80:8E12): back from the item screen, D+$4A restored. It starts
    after its PLA, because a row whose SP rises above its entry would see its own JSRs as
    leaving it;
  - `shop_screen_frame_end` ($80:8E2A): the cursor, the colour cycle and the typed text. It
    returns its CMP's carry;
  - `shop_screen_close` ($80:8E91): the upgrade bits cleared, D+$76 waited for, the race's
    NMI and IRQ targets. It is a waiting row; the SEI is a comment.
- **Not in a row:**
  - the item-screen call and its button test (`$80:8E04-$80:8E11`);
  - the `$9A` test and its JMP (`$80:8E87-$80:8E90`);
  - the frame's wait;
  - the loop's head `$80:8D86`, which is inside `shop_screen_list_setup`'s row.
- **The unrun path:** the held cursor's repeat (`$80:8E53-$80:8E6B`) is decoded and in the C,
  and is the frame end's hole. The scroll's other ways run on the shop movie only.
- **Check:** full gate (a new waiting row, which hides `wait_flag_76`'s samples from this one
  caller).

## The menu bar's line window (menu-1, 2026-11-24)

- **`$80:C5E0` is the menu task's per-frame callback.** Its address is put in D+$EC, and the
  menus' NMI calls it with `JSR ($01EC,X)` after its stack switch, for either player's D. It
  opens and closes the menu bar's HDMA line window at `$1B00 + D+$F2`, by D+$4C (0 to `$20`
  in steps of 4), choosing the step by D+$4E through the table at `$80:C636`.
- **Ported as a dispatcher** (as `shop_buy_dispatch`), in `src/misc.c`:
  - `menu_bar_lines` ($80:C5E0): the middle lines, ending at the table's `JMP ($C636,X)`.
    Its 8-bit `SEC / SBC / SBC` borrows the first SBC's carry;
  - `menu_bar_open_step` ($80:C642): step 1, opening;
  - `menu_bar_lines_end` ($80:C6B9): the end every step shares (step 0 is it alone).
- **Steps 2-5 are not rows.** Steps 2 and 3 never run. Steps 4 and 5 call `$80:CCE1` and
  `$80:D026`, menu-task code not ported yet.
- **Results:** 395 compared samples of 1,121 entries on each movie, with the same evidence
  on all three that reach it.
- **Check:** incremental gate (new rows only).

## The menus' movie (explore-5, 2026-11-24)

- **A sixth verification movie, `tools/bench/movies/explore_menu.txt` (9,300 steps).** No
  movie reached the menu's first three choices, the sub-screens at `$80:C6D4`, `$80:CB4A` and
  `$80:CEE2` that `$80:C4F9`'s table names. The 60k movie always takes the fourth (`$80:D168`,
  the race).
  - The movie is the 60k movie up to its main menu (frame 4300), then A on the first choice
    (frame 4460).
  - Then the coverage explorer from frame 4500: 16 segments of 300 steps, each the best of 6,
    measured against the other five movies' coverage.
- **What it adds:** 3,040 code bytes no other movie runs.
  - Explorer runs from the second and third choices reached the same bytes, all but 5 to 16:
    the explorer leaves one sub-screen for the others. One movie is enough.
  - Its replay reaches exactly the bytes the explorer counted.
- **`verify_all` runs it as one slice and `verify_new` as a sixth movie.** It adds about 30 s
  to the full gate.
- **Check:** full gate. A new movie can fail rows it reaches for the first time.
- **Result:** every row's status is as before. Only `func_809DD0`'s live count moved.

## The menus' movie in the map (explore-6, 2026-11-24)

- **The menus' movie is a coverage source.** It follows the game's intended path, button
  presses only. Its coverage is merged into `traces/coverage.bin`, and `map/` and `asm/` are
  regenerated.
- **Effect:** code grows from 72,062 to 72,986 bytes, the 924 bytes the TAS never ran
  either. The ROM classified goes from 80.45% to 80.88%. The verified share drops from 84.02%
  to 82.95%, because the denominator grew: nothing verified was lost.
- The menu's routines the listing now shows (`$80:CE7F`, `$80:CC7E` and others) are the next
  ports.
- **Check:** `asm/` rebuilds the exact ROM, and `progress.py --check`.

## The menus' sprite text (menu-2, 2026-11-24)

- **`sprite_text_menu` ($80:8704) is ported** (in `src/text.c`). It is the menus' copy of
  `sprite_text`, with its own font:
  - capitals `$00-$19`, digits `$1A-$23`, and three marks from `$80:879B` (`$24-$26`);
  - widths from `$80:87B8`;
  - a glyph's tile doubled into the 16-wide sheet (scratch D+$18);
  - OAM high bits `$0100`, the large size.
- **The movie:** only the menus' movie reaches it, where it is sampled 1,152 times of 3,393
  entries. It also passes 500 fuzz cases.
- **Never run in the movies:** the space and high-bit characters. The fuzz runs both.
- **Check:** incremental gate.

## A menu bit's word (menu-3, 2026-11-24)

- **`menu_bit_word` ($80:CE7F) is ported** (in `src/misc.c`). It gives the word of
  `$80:CE96` for A's lowest set bit, with a `SEC / ROR` loop that counts it in Y (twice the
  bit's number).
- **Results:** on the menus' movie, 811 of 2,370 entries sampled, and 500 fuzz cases (a bit
  alone, or with random bits above it).
- **Check:** incremental gate.

## A menu frame's sprites (menu-4, 2026-11-24)

- **`menu_frame_sprites` ($80:CA97) is ported** (in `src/misc.c`):
  - four corner sprites (tile `$150`, flipped three ways) around a box D+$4A wide, placed by
    D+$7A/D+$7C and D+$E7;
  - its top and bottom edges, every 8 pixels;
  - then the group of `$81:BF40` (`sprite_group`). Its outputs are the row's exit, since it
    ends on that JSR.
- **Results:** on the menus' movie, 210 of 567 entries sampled, all 81 paths.
- **Check:** incremental gate.

## A menu panel's sprites (menu-5, 2026-11-24)

- **`menu_panel_sprites` ($80:D026) is ported** (in `src/misc.c`). It draws a menu panel by
  D+$7C, the choice:
  - two sprite groups;
  - the choice's colour word;
  - D+$7C and D+$7E stepped one toward the targets the car's `$03A7` picks;
  - a frame D+$7E high: six corner and side tiles, then its edges every 8 pixels.
- **The unrun paths are decoded and in the C, and are its holes:** the choices from `$0E`
  (two other group tables), the step down for D+$7C, and any step for D+$7E.
- **Results:** on the menus' movie, 122 of 302 entries sampled, all 137 paths.
- **Check:** incremental gate.

## The controls menu's button (menu-6, 2026-11-24)

- **`menu_button_assign` ($80:CC7E) is ported** (in `src/misc.c`). It gives the buttons just
  pressed to line D+$48 of the player's five (`$039D + D+$94`):
  - one button only;
  - the line that held it gets the old one.
  - On line 5 (EXIT), a B/Y/A/X press sets D+$4E = 4 and leaves the caller too, with a PLA
    and an RTS. The C returns true then, and the row is `mayLeaveCaller`.
- **Its 11 samples are below the 100-return rule,** so every flag is live, and the adapter
  models each exit.
- **The unrun paths are holes:** no bit pressed, and the exit while D+$4C is set.
- **Results:** 11 of 11 entries sampled, all 53 paths.
- **Check:** incremental gate.

## The controls menu's sprites (menu-7, 2026-11-24)

- **`menu_controls_sprites` ($80:CCE1) is ported** (in `src/misc.c`):
  - its frame;
  - D+$4A and D+$7A stepped toward the cursor line's targets;
  - the title;
  - the five lines' button names (`menu_bit_word`, `sprite_text_menu`);
  - the cursor's colours.
- **Holes:** the two clamps for D+$7A overshooting its target, which the movies never need.
- **Results:** on the menus' movie, 179 of 474 entries sampled.
- **Check:** incremental gate.

## The menu bar's other steps (menu-8, 2026-11-24)

- **Steps 2-5 of the menu bar's window are rows now**, since the menus' movie runs them and
  their callees are ported:
  - 2 (`$80:C651`): closing;
  - 3 (`$80:C65D`): opening with four colour words by the height;
  - 4 (`$80:C68D`): opening with `menu_controls_sprites`;
  - 5 (`$80:C6A2`): opening with `menu_panel_sprites`.
- **Each ends at the shared end** (`endAt $80:C6B9`, its flags dead after the cut, by
  harness-17), with the STZ of D+$4E at `$80:C6B7` as `alsoIn`.
- **Results:** 93, 18, 33 and 39 samples, all paths. `menu_bar_lines` itself now has 1,885
  on the menus' movie.
- **Check:** full gate. Two comments that said steps 2-5 were not rows changed, so the change
  is not additions only.

## The name entry's keys, and writes to nothing (menu-9, harness-20, 2026-11-24)

- **`name_entry_key` ($80:C9A6) is ported** (in `src/misc.c`). It handles the name entry's
  key D+$48:
  - a letter from `$81:C16C` typed into the name at `$037E` (13 at most, the cursor `_`
    after it);
  - `$1D`/`$1E` delete;
  - `$1F`/`$20` end. With D+$4C at 0, the frame and the name are redrawn a frame at a time
    while the colours run through a table; then D+$4E = 3, and it leaves the caller too (PLA,
    RTS). It is a waiting, `mayLeaveCaller` row.
- **A quirk kept:** in the end's loop the colours go to `$0122 + Y` through DB = `$81`. Y is
  not reloaded: it is what `sprite_text_menu` and `oam_finish_near` leave, a screen position
  such as `$5C86`. So the game writes them to `$81:5DA8`, where no device answers, and they
  are lost. The C does the same through `bus_wr16`, which writes WRAM only.
- **harness-20:** a write to an address no device answers is dropped, as on the console. In
  the system banks these are `$2000-$20FF`, `$2200-$3FFF` and `$4400-$7FFF`, outside the
  DSP-4's banks. Before, it marked the sample as a hardware access.
  - Reads there (open bus) stay hardware accesses.
  - A C that wrote WRAM where the original wrote to nothing would still fail on WRAM.
- **Results:** on the menus' movie, 13 of 13 entries sampled, all 102 paths. The unrun paths
  (a full name, a delete with nothing typed) are holes.
- **Check:** full gate (`difftest.c` changed).

## The controls menu's parts (menu-10, 2026-11-24)

- **The controls menu (`$80:CB4A`, the menu's second choice) has four rows** between its waits
  (in `src/misc.c`):
  - `menu_controls_open` ($80:CB4A): the window opening, two graphics, the rows copied. Its
    PHA of the second length is above the cut, and the adapter writes it;
  - `menu_controls_lines` ($80:CB8D): the cursor on EXIT, the frame's size;
  - `menu_controls_setup` ($80:CBB0, after the PLA of that length): three panels, the
    palette, the sprites, the colours;
  - `menu_controls_frame` ($80:CC45, after the loop's wait): the cursor, a button to the line,
    the sprites. On EXIT `menu_button_assign` pulls its own return, so the sample ends at the
    controls menu's own return. Its RTS at `$80:CCE0` is `alsoIn`.
- **Left out:** the two polls (the window open, the text typed), the loop's wait, and the
  CLC/PLA bytes at the cuts.
- **The unrun path:** up pressed, a hole; the movie only goes down.
- **Results:** on the menus' movie: the opening by replays, 4 and 4 for the lines and the
  setup, 169 frames.
- **Check:** incremental gate.

## The name entry's parts (menu-11, 2026-11-24)

- **The name entry (`$80:C6D4`, the menu's first choice) has six rows**, cut as the controls
  menu's (in `src/misc.c`):
  - the opening (its PHA above the cut);
  - the lines;
  - the setup (two panels, the palette by MVN, the cursor on END, the name's end in
    D+$40/D+$42);
  - the ready part;
  - the frame, in two around the key handler. `name_entry_key` is a waiting row, and a row
    around it would hide its samples:
    - `name_entry_frame` ($80:C7E8): the colours, the cursor's blink, the cursor on the
      13-wide key grid with the pad's repeats, the frame stepped toward the key;
    - `name_entry_draw` ($80:C97C): the sprites and the name.
- **Holes:** left past the wide keys, right from the last rows' keys, and the height's
  clamps. The last is one hole with the +2 step between the clamps, because of the
  four-hole limit.
- **Results:** on the menus' movie: the opening by replays; 2, 2 and 2 for the setups; 204
  frames each for the frame halves.
- **Check:** incremental gate.

## The menu's third choice (menu-12, 2026-11-24)

- **`$80:CEE2`, the menu's third choice**, sets the player's `$03A7` (0 or 1, the panel
  `menu_panel_sprites` draws). It has five rows between its waits (in `src/misc.c`):
  - the opening (three graphics; its two PHAs of lengths above the cut);
  - the lines;
  - the first panel and the setup, cut at the second PLA for the same reason as the controls
    menu's;
  - the frame: up/down pick, and B/Y/A/X with the window open leave (D+$4E = 5). The sample
    then ends at the screen's own RTS.
- **Hole:** down pressed; the movie only presses up.
- **Results:** on the menus' movie: the opening by replays, 6 samples each for the lines, the
  panel and the setup, and 104 frames.
- **Check:** incremental gate.

## The menu's choice dispatcher (menu-13, 2026-11-24)

- **`$80:C4F9`, the menu's choice**, has six rows between its waits and its PLAs (in
  `src/misc.c`):
  - **the dispatch** (`menu_choice_begin`): its two PHAs (D+$48, D+$4A) above the cut. It
    ends at its `JSR ($C5D5,X)` into the choice's screen.
  - **back from a choice**, for the first three only: the race does not return here:
    - `menu_choice_back`: the menu's two graphics, the rows, eight cleared map rows;
    - `menu_choice_lines`;
    - `menu_choice_panels`: two panels, ending at the third panel's JSR;
    - `menu_choice_redraw`: after the PLAs of D+$4A and D+$48;
    - `menu_choice_done`: the RTS.
- **Left out:** the two polls, the PLAs, and the third panel's own JSR (`func_80B537` is a row
  of its own).
- **Results:** on the menus' movie, 13 dispatches; 12 returns (`back` by replays); the 60k
  movie's one dispatch, to the race.
- **The full run found menu-12's frame row wrong:** sharded, it watched 96 returns, under
  the 100-return rule. Its watches mix the cut, where the flags are dead (harness-17), with
  the screen's returns, where they are live. So every flag stayed live, and its adapter
  modelled none at the cut: N failed 94 times. The incremental gate had seen 104 returns.
  - Fixed: the adapter now gives the cut's N Z (its `LDA #$C0C0` / TRB, or the LDA of D+$4C)
    and C V (`oam_finish_near`'s).
  - **Lesson:** a cut row that can also return needs its flags modelled at the cut too.
- **Check:** full gate (the last full run was 4 commits back).

## The race choice's setup (menu-14, 2026-11-24)

- **`$80:D168`, the menu's fourth choice (the race)**, drops two returns with four PLAs, so
  the menu task leaves. It closes the bar and loads the pre-race screen. Its setup has five
  rows between its waits, hardware polls and PLA (in `src/misc.c`):
  - `race_start_close` ($80:D16C). Either player's D: the `$80:BE32` screen jumps here too;
  - `race_start_bar` ($80:D17F): `$036A`, the window lines, BG2's scroll;
  - `race_start_load` ($80:D1B4): the targets, three graphics, the maps, the scenery, the star
    layers, the sprites cleared. Its PHA of a length is above the cut;
  - `race_start_setup` ($80:D269): five HDMA channels, the windows, the palettes, seven VRAM
    queue entries;
  - `race_start_setup_end` ($80:D3BA, after the PLA): four more queue entries, steering, the
    meter, the HUD, the music choice.
- **Not portable as rows:**
  - the polls on `$4212` (H-blank) and on channel 1's HDMA line counter `$431A`, and its write
    there (the harness does not model those counters);
  - the PLAs and the waits.
- **Left for later:** the pre-race frame loop from `$80:D46F`.
- **Results:** on the 60k movie, 5 samples each (2 for the close, both players' tasks); on
  the menus' movie, 1 each.
- **Check:** incremental gate.

## The pre-race frame, DMA modes 2 and 6, raster IRQs (menu-15, harness-21, harness-22, 2026-11-24)

- **`race_start_frame_draw` ($80:D475) is the pre-race screen's frame**, after its wait. In it:
  - the meter;
  - the animated tiles' high bytes to VRAM (raw mode-2 DMAs);
  - the big animation through the PPU multiplier, never run, a hole;
  - the camera eased toward the track;
  - the stars;
  - the text colour between the course's markers (CPU divider and PPU multiplier);
  - a button's fade;
  - the course's slide;
  - the HUD's sprites;
  - the colour cycle.
- **Its row is a waiting row ending at the fade test** (`endAt $80:D65D`). Its end,
  `race_start_end` ($80:D668), is a row of its own: the race's targets back, HDMA off, the
  track's bit and number.
  - Reason: the raster IRQ moves the fade mid-frame. The C, which gets the frame's interrupts
    after it, would see the fade one frame late.
- **harness-21:** a raw DMA in mode 2 or 6 (one register written twice per unit) now steps the
  C's VRAM pointer as mode 0 does. A second such DMA without a new address was compared
  against the first one's address.
- **harness-22:** only a frame's interrupt makes a sample lagged (harness-15): one that moved
  the frame counter `$0024`, the NMI.
  - The raster IRQs of a split screen land mid-body every frame and lose no frame, but they
    made every sample of this row lagged, so none could pass clean. This is the narrowing the
    advisor foresaw in harness-15.
- **Results:** on the 60k movie, 685 frames and 5 ends, all paths. The menus' movie also
  passes.
- **Check:** full gate (`difftest.c` changed). The narrowing can turn an old lagged failure
  into a hard one; the full run says.
- **Result:** no old row changed.

## The championship screen's setup (menu-16, 2026-11-24)

- **`$80:BE32`, the championship menu screen** (entered at `$80:BE32`, or at `$80:BEC6`
  without the records cleared), has nine rows for its setup, between its waits and its five
  queued uploads (`queue_unpacked_upload` is a waiting row of its own). In `src/misc.c`:
  - **the start**, on both entries: the image;
  - **the setup** (`champ_setup`, and `champ_setup_fresh` with the clears): the window's HDMA
    list and channel, the tasks' offsets;
  - **from the records on, run by both players' tasks** (D = `$0100` and `$0200`, `dp`):
    - the records;
    - the load: two graphics, the rows, the bar's block, the queue, `$800` bytes cleared by an
      overlapping MVN;
    - the map's queue;
    - the panel;
    - the fade in.
- **Left for later:** the screen's frame loop from `$80:C053` (rng_next and its menus). Its
  exits jump into the race choice (`$80:D16C`, `$80:D17C`).
- **Results:** on the 60k movie, 4 fresh starts, 1 other, and 10 samples (both tasks) of
  each shared part. The menus' and password movies also pass.
- **Check:** incremental gate.

## The championship screen's opening animation (menu-17, 2026-11-24)

- **Five rows between its waits and polls** (in `src/misc.c`, each with `dp`):
  - `champ_slide_step` ($80:C05C): a frame of the bar's slide; a panel every 4 frames;
  - `champ_clear_panels` ($80:C096);
  - `champ_noise` ($80:C144): a frame of static from `rng_next` on the task's state (D+$10);
  - `champ_noise_panels` ($80:C22D);
  - `champ_menu_ready` ($80:C261): the static over, the menu bar.
- **Left for later:** the menu's frame loop from `$80:C2BB`.
- **Results:** 36 slide steps, 7 static frames (one more by replay), 8 panel pairs, 1 each
  for the others.
- **Check:** incremental gate.

## The championship menu's pass (menu-18, 2026-11-24)

- **`champ_menu_frame` ($80:C2BB)** is a pass of the championship menu's loop: the cursor's
  colours, the menu bar, the sprites, up/down. It has no wait, because the task runs while
  the other one's frames switch to it.
- **Cut at the button test** (`$80:C306`): A/B/X/Y opens the choice through
  `menu_choice_begin`, whose rows cover it. The test and the JSR are left out.
- **The row is not a waiting row.** The NMIs that land in it make those samples interrupted,
  and they are replayed from their entry state.
- **The unrun path:** up pressed, a hole.
- **Results:** 732 passes on the 60k movie and 1,065 on the menus' movie.
- **Check:** incremental gate.

## The options menus' interpreter, first parts (menu-19, 2026-11-24)

- **`$8A:A2AF` interprets the options menus**, the championship's password and INFODATA
  screens. A menu is a list of 8-byte items at `$8A:(D+$4A)`: a string list, an action word,
  and the flags' index (`$1516 + X`).
  - Each item is printed centred.
  - The focused one takes the cursor, and its selection is tested (`$8A:A419`).
  - A press puts the item's action in D+$5C (`$015C` with D = `$0100`) and jumps through it
    (`JMP ($015C)`, `$8A:A466`).
  - The actions recurse: `$8A:AA4FB` opens a submenu through `$8A:B659`, which calls
    `$8A:A2AF` again a frame at a time.
- **Four rows now** (in `src/misc.c`), cut around the selection test:
  - `menu_draw` (up to the focused item's test);
  - `menu_draw_rest` (after it: its carry decides);
  - `menu_select` (up to the jump; its four pushes above the cut). The movies select in
    entries the sampling skips, so its 500 fuzz cases cover that path;
  - `menu_action_rookie` (`$8A:B174`).
- **Left for later:** the dispatch through `$015C`, the submenu loop `$8A:B659`, the
  password's action `$8A:B21C` and its typing loop `$8A:B232` (not ported yet: the dispatch's
  C needs it).
- **Results:** 109, 109, 109 and 1 on the 60k movie; 229-230 on the INFODATA movie.
- **Check:** full gate (the last full run was 4 commits back).

## The password entry screen (menu-20, 2026-11-24)

- **`$8A:B232` is the password menu item's screen.** It sets up the windows and lines, slides
  the caret in, then loops a frame at a time: the pads move on the grid and A/B/X/Y types a
  letter. EXIT checks the typed text.
  - A valid password is unpacked into the game's state and leaves the menu.
  - Otherwise the codes are searched (`password_code_find`, which jumps to the code's
    handler).
- **Seven rows** in `src/password.c`, cut between the waits:
  - `password_entry_open`, `_sprites`, `_slide`, `_ready`, `_frame` (ends at its BCS on
    `password_press_a`'s exit, so its adapter shares that model), `_pads`;
  - `password_entry_check` (up to the code search). The valid path's carry is
    `func_8ABF76`'s.
- **Results:** 849 typing frames on the password movie, 492 on the INFODATA movie. The check
  runs once per movie, interrupted, and passes from its entry state.
- **Check:** full gate (`password_press_a`'s adapter was split, so not additions-only).

## The options menus' actions and submenus; the options movie (menu-21, explore-7, harness-23, 2026-11-24)

- **The dispatch through `$015C` is a switch in C** (`menu_action`, `src/misc.c`), with one
  named case per action word.
  - The actions are found by walking the five menus at `$8A:CBA7` (main), `$8A:CBD2`
    (championship), `$8A:CC45` (the same, after a star name), `$8A:CC83` (options) and
    `$8A:CCEB` (VS MODE's players).
  - Three actions open a submenu, two set an option (DIFFICULTY, MUSIC), and three choose
    the game's mode. The others are EXIT, PASSWORD and CANCEL PASSWORD.
  - Unported targets stop the program (`tg3000_unported`, a trap, not a silent return):
    VS MODE's player counts (`$8A:A53C-$8A:A557`; ported by vs-1), the RELAX code
    (`$8A:BD9D`) and the wrong-password message (`$8A:BD6E`), all never run then.
- **The composed C:**
  - `menu_run`: a menu's frame, the whole of `$8A:A2AF`. It keeps D+$62 and D+$4A around the
    action, as the original's pushes do; a submenu draws its own menu.
  - `menu_submenu`: `$8A:B659`.
  - `password_entry`: the whole of `$8A:B232`, with `password_code_run`, the codes' handlers
    by address.
- **The rows** are cut at the waits, and the leaves run through `menu_action`, so the
  word-to-case mapping is what they check:
  - `menu_action_submenu`, `_options` and `_vs` end at their JSR to the submenu (A the menu);
  - `menu_action_difficulty`, `_music` and `_exit`;
  - `menu_options_apply` (`$8A:B11A`);
  - `menu_password_back` (`$8A:B225`);
  - `menu_submenu_open`, `_tick`, `_frame` and `_chosen`;
  - `menu_select_restore` (`$8A:A462`, after the selection tail's last PLA);
  - `password_entry_found` (`$8A:B2FD`).
- **harness-23: a row may have a second cut (`endAt2`).** `menu_submenu_chosen` branches back
  to the loop's head (`$8A:B65E`) or goes on to its wipe (`$8A:B67C`); both are cuts, and both
  lead to a wait. The advisor's call, open to the owner's veto.
- **explore-7: a seventh movie, `explore_options.txt`** (4,400 steps then; explore-8 extends
  it).
  - It follows `code_infodata.txt`'s way to CHAMPIONSHIP, then goes into OPTIONS: DIFFICULTY
    three times, MUSIC twice, and EXIT. Then EXIT from the championship's menu, and VS MODE.
  - OPTIONS and its submenu ran only in the TAS reference's coverage, in none of the six
    movies.
  - It adds 115 bytes of code: MUSIC, VS MODE and its menu. `func_8AA32B`'s bit-7 path
    (`$8A:A345`) now runs, so its static range is dropped.
  - `verify_all.sh` and `verify_new.sh` run it as one slice.
- **Check:** full gate (the harness and the scripts changed).

## VS MODE's menus in the options movie; the VS race stops the bench (explore-8, 2026-11-24)

- **`explore_options.txt` goes on into VS MODE** and now runs 5,450 steps. It chooses
  1 PLAYER (the action `$8A:A53C`) and opens the VS name entry, a grid with four panels:
  three computer drivers READY.
  - Its EXIT (Down x4, Right x2, A) leads to the car's choice.
  - There it presses Right, Right and Left: THE CHEETAH, THE DEMON and the others, with
    their acceleration, top speed and boost.
- **Coverage:** 5,812 new bytes, all in regions the map had as unknown (code 73,034 ->
  75,593). VS MODE was reachable code no movie ran, and it is in scope (reachable code is
  ported).
  - `$8A:AE9B`, rng_next's third copy, now runs, so its static range is dropped.
- **Choosing a car breaks the bench.** The VS race starts: DSP-4 commands from `$82:BBBA`
  and `$81:E652` at step 5,533, and code in `$82`, `$85`, `$87`, `$89` and `$9B` the map has
  as unknown. The screen stays black, though.
  - By step 6,543 the CPU runs at `$EE:FFF0`, then in low WRAM (`$00:0006`, `$00:0104`).
  - Settled by explore-9 (below): the bench's DSP-4 is wrong; the input and the game are
    fine.
  - The movie ends before the car's choice.
- **Check:** full gate (a movie the rows run over changed).

## The VS race on the reference runner; its code in the map (explore-9, 2026-11-24)

- **The same inputs on the reference runner** (bsnes with the DSP-4 firmware) run the VS
  race: a four-way split screen, the player and three computer drivers, speeds shown. The
  menus line up step for step with the bench.
  - `tools/movie_to_log.py` turns a bench movie into the runner's input log.
  - The movie is `tools/bench/movies/vs_race.txt`: the reference runner only, 9,000 steps.
- **Without the firmware the runner fails too**, using StackBlender's clean-room chip (the
  DSP-4 activity stops early: 11.6% of the frames against 36.8%). The bench uses that same
  chip.
- **The catalogue** (`--dsp4-catalogue`, the chip as the firmware's shadow) puts the first
  difference at step 5,676, on command `$08` read at `$85:A670`. From there on the shadow's
  runs merge (80,000+ bytes in), so it has lost step at once.
  - `$08` is the road command the clean-room chip already gets wrong in most runs on the
    TAS: 6,907 of 9,595 (the catalogue of 2026-11-15).
  - In the one-player races that only changes values. In the VS race it crashes the game.
- **So the bench needs correct DSP-4 answers in the race.** First tried: a firmware tape
  replayed on the bench (harness-26, below). The races' timing differs between the two
  emulators, so that cannot work. That leaves our own road commands (`$01`/`$07`/`$08` in
  `src/dsp4.c`, measured against the firmware): the owner's standing decision of
  2026-11-15, multi-unit research, not started. Until then:
  - the VS race runs on the reference runner only;
  - its rows cannot be difftested, and the VS menus are ported first.
- **Its coverage is in the map** (the reference runner is a trusted run, as the TAS was).
  - The 9,000 steps add 29,632 bytes to `traces/coverage.bin`. Code grows from 75,593 to
    94,830 bytes, all from regions the map had as unknown.
  - The 15 static ranges it executes are dropped from `map/static_code.txt`: the multitap
    pad read, `player_road_limits`' far-row cases, `car_sync_slot`'s handlers 8-14,
    `track_setup_load`'s variant 2, `race_bands_merge`, `player_in_top5`'s two players,
    object kind 0's handler and others.
  - Their "never run" comments in `src/` are now out of date; they are fixed as each
    routine is revisited.
- **Check:** full gate (the map and the listing changed).

## VS MODE's player counts (vs-1, 2026-11-24)

- **The four counts** (`$8A:A53C`, `$8A:A543`, `$8A:A54A`, `$8A:A551`: 1 to 4 PLAYERS) each
  load Y with the count and call `$8A:A558`. That routine sets `$03DC` and opens the VS screen
  (`$8A:A573`); the menus are left when the screen returns (`$8A:A570`).
- **The pads' mask (`$8A:A509`)** walks D+$56's bits from the lowest, shifting each into the
  top of D+$42, up to its Y-th set bit, one for each pad present.
  - Then it shifts down by what is left of 4 (its X), and by 8 (`lsr_n`).
  - It loops forever when D+$56 has fewer set bits than Y. The fuzz gives it at least as
    many.
  - Its absent-pad path (`$8A:A531`) runs only in the fuzz, so it is a static range.
- **`$8A:A558` sets `$03DC` to the mask ORed with 2.** It masks `$03DC` first, but then
  stores the mask over it.
- **In C** (`src/misc.c`):
  - `menu_action_players`, the count for an action word;
  - `vs_players_mask` and `vs_players_set`;
  - `vs_players_done` (`$8A:A570`, no row: it runs after the car's choice, past the movies).
  - `menu_action`'s cases for the four words. The VS screen's setup is still a trap
    (`tg3000_unported`).
- **Rows:**
  - `menu_action_players` ends at its JSR (Y);
  - `vs_players_mask` (500 fuzz cases);
  - `vs_players_set` ends at the VS screen's JSR (500 fuzz cases).
- **Check:** full gate (`menu_action`'s default case changed).

## VS MODE's screen: ten panel leaves (vs-2, 2026-11-24)

- **A new file, `src/vs.c`** (`src/vs.h`), for VS MODE's screen (`$8A:A573` on). The screen
  has four panels, one a player, with X 0, 2, 4 or 6 selecting the panel. Each panel has a
  name entry like the password screen's and then the car's choice.
- **Ten leaves in it:**
  - `vs_palette_load` (`$8A:A7CD`);
  - `vs_cursor_moving` (`$8A:AD58`);
  - `vs_panel_sprite` (`$8A:ABCF`);
  - `vs_cursor_home` (`$8A:AA58`);
  - `vs_letter_index` (`$8A:ACE4`);
  - `vs_panel_screens` (`$8A:A72F`);
  - `vs_letter_next` (`$8A:AEDA`);
  - `vs_cursor_column` (`$8A:AA0E`);
  - `vs_caret_blink` (`$8A:AAA2`);
  - `vs_name_clear` (`$8A:ACB2`).
- **Two of them run only partly in the movie**, so the fuzz checks the rest (its panels, the
  letter column 0-4):
  - `vs_letter_next` runs only its end case (`$0DB4` is 4 there). Its adapter replays the
    steps to model the flags, which are all live under 100 watched returns.
  - `vs_panel_screens` never gets a panel other than 0-6.
  - Their fuzz-only ranges are in `map/static_code.txt`.
- **Results on the options movie:** 526-589 samples for the per-frame ones, 4-24 for the
  others.
- **The sections from menu-20 to vs-1 were missing until this commit.** The script that
  inserts each unit's section anchored on a line that occurs twice, failed its check, and
  the gate ran anyway (`;`, not `&&`). The insertion is now gated with `&&`.
- **Check:** incremental gate (additions only in `src/` and `tools/bench/`).

## VS MODE's screen: nine more routines (vs-3, 2026-11-24)

- **In `src/vs.c`:**
  - `vs_cursor_step` (`$8A:AA23`): Right and Left along the row, once the cursor has arrived;
  - `vs_letter_find` (`$8A:AE20`): the cursor put on a given letter, through the 3-byte
    table at `$8A:AF57`;
  - `vs_cursor_target` (`$8A:AD34`);
  - `vs_all_ready` (`$8A:A795`): the players in and the ready ones counted, and the fade-out
    once all are ready;
  - `vs_panel_screens_bg2_off` (`$8A:A728`): the byte `$11` through `vs_panel_screens`'
    code (`alsoIn`);
  - `vs_panel_text_base` (`$8A:AE6E`);
  - `vs_text_center` (`$8A:A97F`);
  - `vs_row_next` (`$8A:AF08`, the row's `vs_letter_next`).
- **`rng_next_8a_2` (`$8A:AE9B`)** is a byte-for-byte copy of `rng_next`. Its row runs the same
  C (`call_rng_next`), as `rng_next_8a` does.
- **The movie stops before the race,** so some paths run only in the fuzz:
  - `vs_cursor_step`'s Left and `vs_all_ready`'s all-ready path (500 cases each);
  - `vs_row_next`'s landing step (`$8A:AF2A`) and `vs_panel_text_base`'s other panel
    (`$8A:AE82`), which are static ranges now.
- **Results on the options movie:** 292-606 samples for the per-frame ones, 3-31 for the
  others.
- **Check:** incremental gate (additions only in `src/` and `tools/bench/`).

## VS MODE's screen: five more routines (vs-4, 2026-11-24)

- **In `src/vs.c`:**
  - `vs_panel_reset` (`$8A:A9F2`): the cursor goes home on column 0 for a player, or on a
    random column 0-3 for the computer (`rng_next`);
  - `vs_panel_text_clear` (`$8A:AE46`): 14 rows of 15 tiles `$3D` at `$7F:(D+$70)`;
  - `vs_ready_check` (`$8A:A768`): `vs_all_ready`, and with all four panels ready, the
    slower fade-out (`$0196 = $FF80`);
  - `vs_name_default` (`$8A:A8E5`): a name with no letter A-Z gets the panel's default
    name;
  - `vs_letter_width` (`$8A:ACFF`): the cursor's width and the text's shift for the
    letter ('s' and 'e' 9, 'd' 8).
- **Fuzz-only paths** (static ranges now): the default-name copy (`$8A:A90A`) and the 's'
  and 'd' cases (`$8A:AD22`).
- **Results on the options movie:** 526 and 606 samples for the per-frame ones, 3-4 for the
  others, with 500 fuzz cases where the movie misses paths.
- **Check:** incremental gate (additions only in `src/` and `tools/bench/`).

## VS MODE's screen: names and characters (vs-5, 2026-11-24)

- **In `src/vs.c`:**
  - `vs_players_in` (`$8A:A7DB`): the panels in play, from `$03DC` bits 4-7;
  - `vs_names_load` (`$8A:A922`): the four default names copied to `$03E4`, `$03F1`,
    `$03FE` and `$040B`, with their lengths;
  - `vs_name_put` (`$8A:AC73`): a letter put at the end of the panel's name;
  - `vs_put_char` (`$8A:B730`): one character of the screen's text, as a sprite or as a
    tile at `$7F`;
  - `vs_put_sprite_char` (`$8A:B721`): the same as a sprite at (X, Y).
- **`vs_put_char` saves and restores P** (PHP/PLP), so its callers' flags and register widths
  are their own. Its callers enter it with an 8-bit A as well, so its row takes either width
  (`needM16` false).
  - In sprite mode its two adds chain their carry: D+$3A, then D+$8A with the first's carry.
  - It has 4,000 samples on the options movie (33,001 entries).
- **Results on the options movie:** 1-31 for the others, `vs_put_sprite_char` 1,030.
- **Its full gate failed on an older row, not on these.** `race_start_frame_draw` ($80:D475)
  reached 175 of its 179 paths. The new rows moved it into another slice of the 60k movie,
  and there a waiting row's recording hid 610 of its 1,990 entries, the only ones that reach
  `$80:D569-$80:D570`. On its own the row passes, 179/179. The harness fix is harness-24.
- **Check:** incremental gate (additions only in `src/` and `tools/bench/`).

## Hidden entries rerun alone (harness-24, 2026-11-24)

**The advisor's call, open to the owner's veto.**
- **The fault:** in a slice of a movie, a waiting row's sample records the interrupts that
  land in it. A row entered inside one of those handlers is not sampled there; the slice's
  log says "N entries inside interrupt handlers a waiting sample recorded (not sampled)".
  Which rows share a slice moves whenever a row is added. So a row can lose, by slice
  luck, the only entries that reach some of its paths.
- **The case:** `race_start_frame_draw` lost 610 of its 1,990 entries on the 60k movie and
  four paths with them. The commit before vs-5 already had the same slice mates, so its own
  full run would have failed too. The earlier full run passed on the old slice split.
- **The fix, in `tools/verify_all.sh`:** after the merge, `tools/rerun_hidden.py`:
  - takes every row the merge failed with no hard failure on any movie (`hard:0` in every
    movie's results);
  - finds the movies whose slice logs report hidden entries for it;
  - runs the row alone over each of those movies, one process each (rows run together
    could hide each other again);
  - replaces the slice's line with the solo line, not adds it, so the same returns never
    count twice toward the 100-watched flag rule.
  Then the movies merge again. A hard failure anywhere still fails, and the rows rerun are
  printed.
- **Not chosen:** reordering rows or changing the slice count (it breaks again at the next
  row added), and sampling rows inside a recorded interrupt (it rewrites harness-16).
- **Check:** full gate (the verification script changed; the last full run was 5 commits
  back).

## VS MODE's screen: the name entry (vs-6, 2026-11-24)

- **In `src/vs.c`:**
  - `vs_put_string` (`$8A:B072`);
  - `vs_panel_marks` (`$8A:AB6B`, AB84, AB9D, ABB6: one C function, four rows), the
    panel's two marks as sprites;
  - `vs_name_key` (`$8A:ABEF`): a press adds the cursor's letter to the name. 's' is a
    space, 'd' deletes and 'e' ends the name; once the cursor has arrived, the panel goes on
    to the car's choice;
  - `vs_name_entry_frame` (`$8A:AD74`): a frame of name entry. A player moves the cursor
    with the pad; the computer, after a random delay of 16-48 frames, presses the next
    letter of its own name;
  - without rows of their own: `vs_panel_screens_bg1_off` (`$8A:A721`), `vs_letter_prev`
    (`$8A:AEB9`) and `vs_row_prev` (`$8A:AF31`). They run only through the two rows above,
    in their fuzz.
- **The press is taken by inverting its bits:** `$0DCC` becomes `(pad & $C0C0) ^ $FFFF`. A
  first draft inverted the whole word, and the movie caught it at once.
- **Flags:** only V is live in `vs_name_key`, and only END changes it (`vs_cursor_moving`'s
  SBC, or `vs_panel_text_clear`'s ADDs). The adapter takes its inputs before the call.
- **`$8A:ADCC-$8A:ADD4`** inside `vs_name_entry_frame` is unreachable (REP, TYA, STA, PLY,
  RTS after an RTS) and is a hole in its row.
- **Fuzz-only ranges** (seven, `map/static_code.txt`): the full name, SPACE and DEL, the
  player's Up and Left, and the three routines above.
- **Results on the options movie:** 526 frames for the two entry rows (500 fuzz cases
  each), 1,696 strings, 94-269 for the marks.
- **Check:** incremental gate (additions only in `src/` and `tools/bench/`).

## VS MODE's screen: the car's choice (vs-7, 2026-11-24)

- **In `src/vs.c`:**
  - `vs_car_title` (`$8A:AA71`);
  - `vs_grid_draw` (`$8A:B037`): the letter grid's five rows;
  - `vs_panel_name_draw` (`$8A:AAC1`): the panel's name as sprites, with its frame sprite
    and its marks;
  - `vs_car_confirm` (`$8A:A852`): the car's choice taken, by a press for a player and at
    once for the computer. The panel becomes ready and the car's column goes to
    `$0418 + panel`;
  - `vs_ready_draw` (`$8A:A869`, entered by a branch from the panel's frame): the name and
    READY, blinking;
  - `vs_car_info` (`$8A:A9A7`): the car's description, redrawn when the column changes.
- **`vs_put_string` returns with N and Z from its last PLX** (X kept), not from the string's
  ending zero. Its own row had those flags dead, so vs-6 did not catch it.
  - `vs_car_title` and `vs_grid_draw` end on it with those flags live, and they failed
    until its adapter was fixed.
- **`vs_panel_name_draw`'s panel other than 0-6** (`$8A:AADB`) runs only in the fuzz, so it
  is a static range.
- **Results on the options movie:** 292-1,183 samples for the per-frame ones, 3-7 for the
  title and the grid.
- **Check:** full gate (`vs_put_string`'s adapter changed).

## VS MODE's screen: a panel's frame and the graphics; long effect logs (vs-8, harness-25, 2026-11-24)

- **`vs_panel_frame` (`$8A:A81C`)** runs a panel's frame by its state `$0DDC`:
  - the car's choice (`$FFFE`);
  - ready (`$FFFD`): it branches into `vs_ready_draw`'s code, which is in its row's
    `alsoIn`;
  - otherwise the name entry.
  It has 1,957 samples on the options movie.
- **`vs_graphics_load` (`$8A:B9AF`)** loads the screen's tiles from `$8D:F696` to VRAM `$4000`.
  It unpacks the panels' map from `$8A:C86C` to `$7F`, then writes VRAM `$7000` on word by
  word: 1,024 words of tile `$23`, then 28 rows of 32.
- **harness-25: `manyEffects`.** That is 1,920 VRAM words, more than a sample's usual 512
  hardware effects. Until now only waiting rows with `maxInsns` had the long log (65,536);
  a row may now ask for it with `manyEffects`. What is compared does not change.
  - Its one live entry is interrupted and passes from its entry state.
  - The advisor's kind of call, open to the owner's veto.
- **Check:** full gate (`difftest.c` changed).

## VS MODE's screen: its setup and its frames (vs-9, 2026-11-24)

- **The VS screen (`$8A:A573`) in parts between its waits** (`src/vs.c`):
  - `vs_screen_blank` (`$8A:A576`): HDMA off. Its 16-bit STZ of `$420C` also writes
    `$420D` (the ROM speed, 0);
  - `vs_screen_setup` (`$8A:A582`, up to the first frame's wait): palettes, graphics, colour
    0 (`video_set_color`), BG1's scroll, mode 1, the map cleared, the four panels' cursors
    and corners, the four grids, the names, the players in, the main screen. Its 16-bit
    store of `$212C` also writes `$212D` with 0. Then the fade in. With 2,000+ VRAM words it
    takes the long effect log (harness-25);
  - `vs_screen_frame` (`$8A:A682`): the players' pads into their panels, the four panels'
    frames, the pads back. It goes on while `$019A` (the fade) runs. Otherwise the absent
    players' names (`vs_names_absent`, `$8A:A6F6`), and the main screen and HDMA (`$82`)
    back.
  - `vs_screen`: the whole, composed with the waits.
- **`menu_action`'s VS player counts now run it** (the trap is gone).
- **The movie stops before the race**, so these run only in the fuzz (500 cases), and are
  static ranges now:
  - the frame's end and its `vs_names_absent`;
  - the other players' pads (`$8A:A692`, `$8A:A69D`, `$8A:A6A8`).
- **Results:** 505 frames on the options movie; the setup's one entry (interrupted) passes
  from its entry state.
- **Check:** full gate (`menu_action`'s case changed).

## A firmware tape replayed on the bench (harness-26, 2026-11-24)

**The advisor's call, open to the owner's veto.**
- **The idea:** to run the VS race on the bench, answer the DSP-4 from a tape of the
  firmware's bytes instead of the clean-room chip.
  - The tape is recorded on the reference runner with the same inputs
    (`refbench --dsp4-tape`, from power-on).
  - `bench --dsp4-tape-replay FILE` (`tools/bench/dsp4_bridge.cpp`) consumes it in order,
    not by frame. Each data read returns the tape's next byte. Each data write must equal
    it. The status reads ready.
  - The first mismatch stops the run and prints the step, the byte, and both sides' PCs and
    values. The default mode is untouched.
  - Tapes are firmware output: they stay in `build/cache` and are made again, never
    committed.
- **Result: the races diverge between the emulators.**
  - The VS race replays its first 30,576 bytes, then at step 5,683 (`$83:F78A`) the bench
    writes `$1E9E` = 4 where the firmware run has 5. `$1E9E` is the number of 60 Hz steps a
    game frame took, which the race's physics multiplies by.
  - The one-player race (`race1.txt`) diverges the same way at step 8,117 (`$82:D959`),
    about 190 frames into the race.
  - LakeSnes honours the ROM speed (`$420D`). The DSP-4 status is not polled before the
    divergence either, so it is not the chip's compute time. It is the emulators' timing,
    which the game's frame-step count turns into different game states.
- **So a tape cannot drive the bench through a race.** A chip that runs the firmware can, as
  can our own road commands: either answers whatever the bench's own run sends. harness-27
  (below) does the first.
  - The tool stays as a diagnostic: it pinpoints where two runs part.
- **The TAS (`Input Log.txt`) was not on disk any more.** WORKFLOW's reference commands and
  the road-command plan (measured on the TAS) depend on it.
  - It had been kept in the session's temporary folder. A cleanup on 2026-11-22 (a disk
    quota in `/tmp`) deleted it along with real throwaway measurements, wrongly reported as
    regenerable.
  - Restored the same day (2026-11-24): `tools/fetch_tas.sh` downloads it from TASVideos
    into `traces/tas/` (ignored by git), checks that its header names our ROM, and unpacks
    `traces/tas/unpacked/Input Log.txt`. WORKFLOW's commands point there.
- **Check:** full gate (`tools/bench` changed; the default path is untouched).

## The DSP-4 run from its firmware on the bench (harness-27, 2026-11-24; the owner's decision)

- **The owner's decision (2026-11-24):** the firmware file he supplied may also run inside the
  bench, for verification only. It is still never committed, never read or disassembled
  by the project, and never used by the port's C.
- **How:** `bench --dsp4-firmware ext/firmware/dsp4.rom`.
  - bsnes v087's NEC uPD7725 core (GPLv3, the reference runner's) is compiled from `ext/`
    (`tools/refbench/fetch.sh`) with a small shim (`tools/bench/necdsp_shim/snes/snes.hpp`:
    nall's types, stubs for the scheduler and the CPU's synchronisation).
    `tools/bench/dsp4_lle.cpp` drives it. Nothing of bsnes is vendored.
  - Without `ext/bizhawk-1.9.4` (as in CI), the bench builds without it and the option
    says so.
  - The default path, the clean-room chip, is untouched: no row's result changes.
- **Timing:** on each data access the chip runs until it next waits for the CPU, i.e. until
  it is about to test RQM (`JRQM`/`JNRQM`) while RQM is set. So it always looks ready.
  - A first draft stopped as soon as RQM rose. The chip sets it when it reads the CPU's
    word, before its answer exists: every multiply came back as its own input, and the
    race never started.
- **Result:**
  - The VS race (`vs_race.txt`, 9,000 steps) runs on the bench: the four-way split, the
    speeds, FINAL LAP, with 3.7M data reads and 5.1M writes. Two runs give the same
    per-frame WRAM hashes.
  - A save and reload in the race gives 600 identical frames.
  - The one-player race (`race1.txt`) looks as it should.
  - The bench's coverage of the VS race against the reference runner's: 1,023 bytes only
    the bench ran and 327 only the reference ran, mostly in bank `$84`. The two races play
    out differently by timing (harness-26), so their after-race screens differ.
- **Next (not this unit):** run `vs_race` in `verify_all` with the firmware chip, so the VS
  race's rows can be difftested. Rows reached only there will need their checks
  (expect the 15 dropped static ranges' rows to see new paths).
- **Check:** full gate (`tools/bench` changed; the default path is untouched).

## Video in the port: the owner's rule applied (2026-11-24)

- **Asked by the owner:** what does not break "not even 1% emulator" for the port's video?
  His own Scope rule answers it ("no CPU/PPU/SPC700/DSP-4 model in any port"), with a test he
  can apply.
  - **The test:** if the port's video (or sound) code could show (or play) *another* SNES
    game when fed that game's video (or sound) memory, it is an emulator. If it only knows
    how to draw Top Gear 3000's menus, panels and road, it is the game.
  - **Fine:** turning the cartridge's tiles and palettes into ordinary images; drawing from
    the buffers the game builds in RAM (tile maps, sprite slots, palettes, the road's row
    tables), with drawing code per kind of screen.
  - **Not fine:** port code that takes VRAM addresses, register values or HDMA channel
    setups and reproduces what the chip does with them.
  - **Grey, his to rule when it comes:** per-line effects. Drawing each line from a table
    the game computed is the game's effect; a general per-line register mechanism is the
    chip.
  - **Sound:** the same test; nothing runs the SPC700 program.
- **The cost:** `src/platform.h` is chip-shaped on purpose (the bench compares with the
  original). The port needs its boundary one level up, at the game's RAM buffers, as
  "Portability rules" already says. That work is not counted in the 73% and is not designed
  yet.
- **The bench is unaffected:** it is a measuring instrument and may emulate (a C-only run's
  picture can come from its own PPU).

## The VS race in the verification (verify-vs, 2026-11-24)

- **`tools/bench/movies/vs_race.txt` is the eighth movie** of `verify_all.sh`,
  `verify_new.sh` and `tools/rerun_hidden.py`. It runs with the firmware DSP-4
  (`--dsp4-firmware`, `TG3000_DSP4_FIRMWARE` to point elsewhere) as one slice of about two
  minutes.
- **Live samples only (`--fuzz 0`).** A row's fuzz starts from the RAM the movie leaves.
  This movie ends in a race, so the VS screen's rows (and others) got race RAM as their
  base: names 30,000 letters long, sound queue indices past 7. The original then does what
  the game never does (writes into the hardware registers), and the C cannot match it. The
  rows' fuzz evidence still comes from the other seven movies.
- **What the race found in the C, all fixed:**
  - **`player_road_limits` assumed player 1's record** (D = `$0200` written into the C). In
    the VS race it runs for the other players' records too (D = `$02B3`, ...), and 664 of
    850 samples failed. It now takes the record (`player_road_limits(dp)`); its caller
    passes `$0200`.
  - **The sound request queue (`func_80894E`, `func_80899F`)** read and wrote `$1B30 +
    index` as low WRAM. The original goes through DB = `$81`, so an index past 7 reaches
    ROM or the hardware. Found by the fuzz on race RAM; the C now goes through the bus.
  - **`race_bands_merge` enters with DB = `$30`** in the VS race. Its absolute addresses are
    all in low WRAM, so its row now asks only for a low-WRAM DB (`lowWramDb`) instead of
    `$83`.
  - **`vs_name_put`'s fuzz** keeps the name's length real (0-12).
- **On the VS race alone,** every row passes live. The rows that fail there fail only by
  paths not reached without fuzz, which the pooling over the movies covers.
- **Check:** full gate (scripts, adapters and the C changed).

## The glue's first test: a screen whole (glue-1, 2026-11-24)

- **Why:** the 73% are routines verified one by one. The functions that compose them (a
  screen whole, with its waits) had never run, so they are the first thing a C-only run
  would exercise. The advisor's call, open to the owner's veto: test them first, smallest
  first.
- **How: composed rows.** `tools/bench/routines.c` gets a third table, `composedRoutines[]`,
  beside the main one and the negative rows. `bench --difftest-composed` runs it.
  - A composed row is a waiting row (`waitsForInterrupts`, `maxInsns`) whose C is the
    composition. It encloses its parts' rows and other waiting rows, so it runs apart from
    the main table's slices, one row per process.
  - `verify_all.sh` runs each one over the 60k movie with live samples only, and merges it
    like any row. `progress.py` counts its extent too: the calls between the parts' rows
    (the JSRs to the waits) are verified by it.
- **First row: `password_show` (`$8A:BE2E`-`$8A:BF0D`, the player's password shown whole).**
  - Its C, `password_show()`, runs `password_show_open`, `fade_out_wait`,
    `password_hdma_off`, the wipes, `password_show_graphics`, `fade_in_start`, the waits,
    `password_show_text` and `password_show_wait`.
  - Its one entry on the 60k movie matches the original in WRAM and in every hardware
    effect. The only difference was the exit carry: it comes from the 8-frame wait's last
    `CMP` (equal), which nothing after it changes, and is now modelled.
  - The interrupts are the original's, replayed, so the C's NMI handlers are not
    exercised. This is not yet "menus only in C".
- **A composed row shares its first part's entry** (`password_show` and `password_show_open`
  both start at `$8A:BE2E`). The first full run failed on it: `merge_verified.py` and
  `progress.py` keyed `map/verified.txt`'s rows by entry, so the two rows' evidence mixed. Rows
  are now keyed by entry and name.
- **Next:** `password_entry`, then `vs_screen` (it returns only in the VS race, live-only
  anyway). `menu_submenu` and `options_screen` come last: `menu_submenu` re-enters itself
  through `menu_run`, and the harness's handling of a row entered while its own sample is
  open must be checked first.
- **Check:** full gate (`difftest.c` and the scripts changed).

## The password entry whole (glue-2, 2026-11-24)

- **`password_entry` (`$8A:B232`) is the second composed row.** It covers the caret's slide,
  the typing frames and the check, with the code search and its handlers.
  - The never-run tails are holes: the wrong password's message, and the menus' submenu
    after a star name.
  - It matches the original in WRAM and every effect on the password movie (a valid
    password) and on the INFODATA movie (a code).
  - Each movie misses the other's path, and the pooling covers both.
- **Its exit flags depend on the way out.** A valid password returns after its last STA of
  `$03DC`, with C from `func_8ABF76` and V clear. A code returns at the found test's RTS,
  with A = D+$52 and C set by the search. `password_entry` now returns which (true for a
  valid password) and the carry.
- **Composed rows name their movies.** A `// movies:` comment before each row in
  `composedRoutines[]` names them, and `verify_all.sh` runs the row on each, one process
  per movie. The VS race gets the firmware DSP-4.
- **Check:** full gate.

## The VS screen's frames whole (glue-3, 2026-11-24)

- **`vs_screen_loop` (`$8A:A67B` to the RTS) is the third composed row**, run on the VS race.
  It covers a frame at a time of the four panels until the fade out ends: names, the
  car's choice, the computer drivers, the readiness, then the absent players' names. It
  matches the original over its one sample, every frame from the first to the return.
- **First tried whole, from `$8A:A573`.** Its one sample failed only as a lagged sample
  (harness-15: set apart, not counted), with 987 bytes different. An NMI lands during the
  graphics' unpacking in the setup: the menus' NMI target is still live there, unlike
  `password_show_open`, which parks `$0008` first. So the composed row starts at the loop's
  head; the setup stays with its cut rows (`vs_screen_blank`, `vs_screen_setup`).
  - The C splits the same way: `vs_screen_loop()`, and `vs_screen()` calls it.
- **Reading a composed row's result:** a lagged failure shows only in the line "N lagged
  sample(s) failed, set apart" and as `lagfail:` in the results.
- **Check:** full gate (`vs_screen`'s C was split).

## The road command's runs measured on the firmware (dsp4-01a, 2026-11-24)

- **The measurement the 2026-11-21 decision asks for before any promise on the road
  commands**, now on the bench's own runs with the firmware chip (harness-27) rather than on
  the TAS.
- **`tools/dsp4_runs.py`** reads `bench --dsp4-log` and splits the data bytes into runs at
  the road command's word: `$000F` at `$85:B536`, or `$0001` at `$85:B53B` when D+$E4 is
  negative (`road_rows_project`).
  - In a race, a run is one frame's DSP-4 traffic: every command from the road command to
    the next frame's.
  - It asks two things: is a whole run's output a function of its whole input, and is each
    answer byte a function of its run's input so far (and its place among the answers since
    the last input byte)?
- **The one-player race (`race1.txt`, 9,450 steps):**
  - 585 runs and 469 distinct inputs. 4 inputs repeat, and none got another output.
  - 1,691,698 answer bytes, keyed by their run's input so far, give 372,926 keys. Only 17
    keys got more than one answer.
  - So within a frame, the chip's answers are almost entirely a function of what the game
    sent it in that frame. The 17 exceptions (0.005%) are what is left to explain: state
    carried from an earlier frame, or the split.
- **The VS race uses another road driver**, whose first different read was at `$85:A670`
  (explore-9). The tool's anchors do not find its runs yet.
- **What this means for native road commands:** a per-frame, stream-shaped function of the
  frame's input is what the C has to compute, and the firmware chip on the bench can check
  it byte for byte on the bench's own runs, with no TAS drift. No promise yet on the size
  of the work: the commands' algorithms are still unmeasured beyond this.
- **Check:** the tool's run above (a measurement; no row or C changed).

## The VS race's first rows (vs-race-1, 2026-11-24)

- **The first two routines that run only in the VS race**, verified on the bench with the
  firmware DSP-4:
  - `vs_cars_ahead` (`$82:9B62`): how many of the four players' cars are ahead of the record
    at D (records at `$0200`, `$0262`, `$02B3`, `$0315`; their race distance in D+$25,
    D+$27 and D+$35).
    - It is one subtraction over the three words. Its borrow in is the `CPX` of the player's
      index against each car's, so an equal distance counts for the lower index.
    - The C keeps that subtraction, its last carry and overflow included.
    - 1,070 live samples, from all four records, and 500 fuzz cases. Its tie path
      (`$8A:9B79`) runs only in the fuzz, so the VS movie is fuzzed again (harness-28).
  - `vs_cars_ring` (`$81:F68B`): the four cars linked in a ring of next pointers at
    `$7E:FACD` + car. 6 live samples.
- **Check:** incremental gate (additions only).

## The main options menu (menu-22, 2026-11-24)

- **`$8A:9E5B` is the options screens' long entry.**
  - It clears `$0D00-$0DFF`.
  - Then X selects: the main options menu (0), the title card (2), `$8A:C379` (3, never
    run), and anything else shows the player's password (`$8A:BE2E`, next unit).
- **The main options menu (`$8A:9E82`)** loads its screen and sets up the cursor, the timer
  and the menu's head. Then, a frame at a time, it draws the main menu `$8A:CBA7` with
  `menu_run`, which covers the submenus and their actions, until a choice leaves the menus.
  Then the fade out, and the NMI handler (`$0008`) is put back.
- **In `src/misc.c`**, rows cut at the waits:
  - `options_state_clear` (`$8A:9E5B`, up to its PLX);
  - `options_screen_open` (up to the wipe; the pushed handler is written above the cut). It
    takes the long effect log, and its one entry (interrupted) passes from its entry state;
  - `options_screen_setup` (up to the first frame's wait);
  - `options_screen_tick` (`$8A:9F47`). It ends back at the frame's wait or at the fade's
    wait loop (harness-23's second cut), and its exit is checked by the fuzz.
  - `options_screen` and `options_screens` compose them. The password shown is a trap
    until the next unit.
- **Results:** 81 and 118 frame ends on the 60k and options movies; one entry for each of
  the others.
- **Check:** full gate. The incremental gate failed first: the tie path is fuzz-only, and the
  VS movie ran live-only (harness-28 below).

## The VS movie fuzzed again, minus three rows (harness-28, 2026-11-24)

**The advisor's call, open to the owner's veto.**
- **verify-vs ran the VS race live-only.** Its last RAM is a race, which is no base for the
  VS screen's frame's fuzz. But that also took the fuzz from rows reached only there: the
  first one, `vs_cars_ahead`, has a fuzz-only path and failed the incremental gate.
- **Now `bench --fuzz-skip NAMES` skips the fuzz of the named rows on that movie.** Every
  other row is fuzzed on the VS race again (500 cases). `verify_all.sh`, `verify_new.sh` and
  `tools/rerun_hidden.py` skip three rows there:
  - `vs_screen_frame`: the race RAM is no base for the VS screen (verify-vs);
  - `track_rows_build` and `player_car_frame`: their fuzz on the race RAM, with the firmware
    chip answering, finds what is below.
- **Found: `dsp4_row` (`src/dsp4.c`) is not exact against the firmware.** On the default
  bench, the clean-room chip's routes answer the game with `dsp4_row` itself, so the
  original and the C always agreed. With the firmware answering, 5 of `track_rows_build`'s
  500 fuzz cases differ by one in a row table byte (`$7E:D1EE`: firmware `$6A`, C `$6B`).
  `player_car_frame` loses 31 cases on the same base. These rows' live samples still pass.
  Next unit: measure `dsp4_row` and `dsp4_edges` against the firmware on fuzzed edges, and
  fix them. The firmware on the bench is the oracle for that now.
- **Check:** full gate (`difftest.c` changed).

## dsp4_row exact against the firmware (dsp4-11a, harness-29, 2026-11-24)

- **`tools/bench/dsp4_probe`** (`make -C tools/bench probe`, needs bsnes in `ext/`) sends
  chosen inputs to the firmware chip (harness-27) and compares the answers with
  `src/dsp4.c`. This is outputs only: what the chip answers to a command on an idle chip,
  never its memory.
  - Command `$11` (`dsp4_row`): four edges in, one row word out.
  - Command `$0A` (`dsp4_edges`).
- **`dsp4_edges` was already exact:** 0 differences in every range tried, up to the whole
  16-bit input.
- **`dsp4_row` was not.** About 4% of random edges in the game's range differed by one in a
  nibble, and far more beyond it.
  - Sweeping one edge at a time through the firmware shows the rule. Every edge is treated
    alike: the edge times 1364 / 65536, rounded down. It is not a division by 48.
  - So 48 gives 0 and 49 gives 1, -48 gives -1 and -49 gives -2, and past about 1,000 the
    steps drift from 48 by one now and then.
  - The old C (floor(edge / 48), minus one for edges 1 and 3) had been fitted to the TAS's
    90 inputs. That data could not tell the rounding apart, as its own comment said.
  - Now exact: 0 of 50,000 differ in each range up to ±32,768.
- **The default bench was blind to it:** the clean-room chip's route for `$11` answers with
  `dsp4_row` itself. The firmware chip on the VS movie's fuzz (harness-28) found it, through
  `track_rows_build`. Its fuzz there now passes (500/500), so it is fuzzed on that movie
  again.
- **harness-29:** with the firmware chip, a replay (an interrupted entry run again from its
  entry state) restores the chip's state as it was at the entry, rather than resetting it.
  A fuzz case that scripts the DSP-4 resets the firmware chip as it does the clean-room one.
- **`player_car_frame` stays off the VS movie's fuzz.** All 469 of its cases that finish
  pass. The other 31 run past the instruction budget: on race RAM with the firmware's
  answers, its car loop runs longer. That is a fuzz base problem, not a mismatch.
- **`car_sprite_head` joins it.** The first full run failed on it: its live samples pass
  (2,569), but every fuzz case on the VS movie's race RAM makes the original read hardware
  (`$84:2E2E`, an index from the four-car state). Its fuzz still runs on the other movies
  (500/500).
- **And on the password movie.** On the second full run, `car_sprite_head` failed the same
  way there: 0 of 500 fuzz cases, all reading hardware at `$84:2E2E`, and all 2,569 live
  samples pass.
  - That movie passes through races. The default bench answers `$11` with `dsp4_row`, so
    the fix changes how they play, and with them the movie's last RAM, which is the fuzz
    base.
  - Every other row still reaches all its paths over the pooled movies.
  - So its fuzz is skipped there too. It still runs 500/500 on the 60k and shop-buy movies.
- **Check:** full gate (`src/dsp4.c` and `difftest.c` changed); the probe (0 of 50,000 in
  each range).

## The mini-map's sprite command measured on the firmware (dsp4-03b, 2026-11-24)

- **`tools/bench/dsp4_probe` now also sends the sprite commands** the way the game does
  (`$82:CBD4`, `$82:D8C6`): `$0003`, `$0005`, then each entry as `$000B`, x, y and the
  attribute. Read from the TAS (`refbench --dsp4-cmdlog 03`): the first entry's `$000B`
  belongs to `$03`'s run. The game reads two more words only when the flag is not 0
  (`beq` at `$82:D91E`).
- **When the firmware shows an entry, its words are `dsp4_sprite`'s.** The low bytes of x
  and y are packed into one word and the attribute comes back unchanged: 0 differ.
- **But the firmware hides entries, and `dsp4_sprite` shows every one.**
  - On a fresh list, the flag depends on y alone, never on x or the attribute:
    - shown for y -7..232 and for y 234;
    - hidden for y -8 and below, 233, and 235..511;
    - for y 512 and above, the same pattern repeating every 512.
  - Within one list it also counts. With every entry on one row (y 50), the firmware hides
    the 34th entry and every one after it; entries 8 rows lower are still shown. That
    looks like a limit on sprites per row.
  - Some answers depend on the chip's history from other commands. After random other
    commands, rows 233, 235, 236 and 238 are sometimes shown and sometimes hidden. The
    lone shown row 234 is probably the same effect.
- **The game is not affected so far.** Its runs (the mini-map's two cars, y 32-67 over the
  whole TAS) are all shown, and two entries per list come nowhere near the count.
  - But the game's y can range over 0..255 (an 8-bit sum), and the VS screen may send
    more cars.
  - So `dsp4_sprite` is right only for the game's runs seen so far.
- **The fix is the next unit: the chip's sprite state in the C.** A pure function cannot
  express the per-row count. Once `dsp4_sprite` can answer 0, two places must follow the
  flag and read 2 bytes instead of 6:
  - the bridge's answer lengths (`{0x03, 10, 6}`, `{0x0B, 6, 6}`);
  - the difftest's command table.
- **Check:** the probe (`$11` and `$0A` 0 of 50,000 differ; the sprite words 0 differ), and
  `progress.py --check`. It only adds lines to `tools/bench/dsp4_probe.cpp`.

## dsp4_sprite hides by row (dsp4-03c, 2026-11-24)

- **Measured further on the firmware, outputs only.**
  - **The per-row count is per list.** Within one list, 33 entries on one row are shown
    and the 34th is hidden. After them, entries from 7 rows above to 7 rows below are
    hidden, which fits a sprite 8 rows tall, but with a gap: 6 rows above is shown, the
    same odd gap as the lone shown row 234.
  - **x and the attribute do not count.** A new `$0003` list starts the count again.
  - **The count changes with `$03`'s word** (word 4 gives 31, word 8 gives 40, word 6
    gives 1). The game always sends 5.
  - **The game sends one list per frame for the mini-map, with two entries,** so the count
    never reaches 34 there.
- **So the C keeps the fresh list's rule for a single entry, a function of y:**
  - shown for y -7..-1;
  - from 0 on, shown for rows 0-232 and 234 of every 512;
  - hidden otherwise.
  - The game's y is an 8-bit sum (`minimap_car`), so rows 233 and 235-255 can be hidden in
    the game. The TAS never reached them.
- **Not modelled, stated in `src/dsp4.h`:**
  - the per-row count (the 34th entry of a list on one row);
  - rows 233, 235, 236 and 238 after other commands, where the answer depends on the
    chip's history; the C gives the fresh chip's answer.
- **When the flag is 0, the answer is the flag alone, as the game reads it** (`beq` at
  `$82:D91E`). The bench follows:
  - `dsp4_bridge.cpp`: the routes `$03`/`$0B` end after the flag when it is 0. The
    clean-room chip shows every entry (checked: flag 1 at y 240 and 255, and 60 of 60 on
    one row), so the bridge reads its other four bytes away to keep it in step.
  - `difftest.c`'s command table: the sprite commands end after a zero flag.
  - `refbench` already compared only the words the game read.
- **Check:**
  - `dsp4_probe`: the flag over every y in -512..511 on a fresh list, 0 differ, and the
    words 0 differ.
  - `minimap_car`: its fuzz now reaches hidden rows, 500/500.
  - The full gate (`src/dsp4.c`, `difftest.c` and the bridge changed).

## What a frame's DSP-4 answers depend on (dsp4-09b, 2026-11-25)

**The `$09` measurement's deferred chosen-input step (dsp4-09a), done with the firmware chip on
the bench (harness-27) rather than built anew.**

**`tools/bench/dsp4_replay`** (`make -C tools/bench probe`) replays a `refbench --dsp4-tape`
capture into the firmware chip: the game's writes as written, the reads where it read. Every
read is compared. This is outputs only: the tape is what the game sent and got.

**The capture:** the TAS's first 8,000 frames, 2,151 `$09` streams and 7.0 million bytes read.

**The whole tape, from a chip just powered on: 0 of 7,013,918 bytes differ.**
- The bench's way of running the chip, to its next wait at each access, gives exactly what
  bsnes's timed chip gave the game.
- That holds even where the game reads three words in a row without looking at the chip's
  status (`$85:B732`-`$85:B750`).

**A `$09` stream alone does not repeat its answers.** All 2,151 streams differ, whether each
runs on a fresh chip or the streams run back to back. The sprites need the road command the
game sent earlier in the frame.

**A frame from its road command on (`$000F` at `$85:B536`, or `$0001` at `$85:B53B`):**
- On a fresh chip, 127 frames are the same and 28,603 of 5.6 million bytes differ.
- Every difference is in the road command's own reads (`$85:B735`, `$85:B73B`, `$85:B747`,
  `$85:B74D`), and each is one more on the tape than on the fresh chip.
- *(Corrected by dsp4-01b: the first count for starting 2, 3 or 5 frames back compared
  every replayed frame, the first one included, which lacked its own history. Comparing
  only the stream's own frame, starting one frame earlier makes all 2,151 the same.)*

**What it does need, bisected for six frames.** The latest command start from which the frame
repeats exactly lies a little before the road command:
- in three of the six, the `$000A` edges command at `$81:F90E` and what follows it (a multiply
  at `$81:FA69`), 196 records earlier;
- once, a `$0007` command at `$85:B95F`;
- once, a sprite stream's part, and once the `$0008` at `$81:861A`.

So the road command reads chip memory that commands it does not own left behind. `src/dsp4.c`
models `$0A` with no state, so a native road command needs that shared state.

**Next (dsp4-01b):** find which earlier inputs change the road command's answers. Replay a
recorded frame with the `$0A` before it changed one input at a time, on the firmware chip.
That makes the shared state measured, not guessed.

**Check:** the tool's runs above. This is a measurement: it adds lines to `tools/bench/` and
changes no row or C.

## The road command's one bit from `$0A` (dsp4-01b, 2026-11-25)

**The shared state dsp4-09b found, measured by chosen inputs on the firmware chip, outputs only.**

**One frame where the road command needs the `$0A` before it** (tape record 2,612,848; the
`$0A` is 196 records earlier, followed by 18 multiplies and the frame's other commands).
Replayed on a fresh chip without that `$0A`, 3 bytes differ. Then a `$0A` was put back in its
place with one input changed:
- **t and the far row:** any value, 0 differ.
- **The near row:** only bit 7 matters, the sign of its third width (bits 7-4).
  - Nibble 0-7 there gives 3 differ, the same as no `$0A` at all; 8-F gives 0.
  - The other 12 bits make no difference.
- **A multiply or a `$11` in the `$0A`'s place,** with random inputs, gives 3 differ, the
  same as none.

**Every frame of the capture, from its road command, after the last `$0A` before it:**
- **all 1,979 frames that have one repeat exactly: 0 of 5,179,618 bytes differ;**
- **with that `$0A`'s bit 7 flipped, 1,898 differ** (27,723 bytes);
- 172 frames, at the start, have no `$0A` before them.
- The frame alone, after no `$0A`, still gives the 28,603 bytes of dsp4-09b.

**So a race frame's whole DSP-4 traffic is a function of the frame's own input plus one bit:**
bit 7 of the near row of the last `$0A` the game sent.
- That covers the road command, the sprites' `$09` stream and all.
- Starting one frame earlier also makes all 2,151 frames the same, since the frame before
  holds that `$0A`.
- dsp4-09b's other needed starts (a `$0007` at `$85:B95F`, a `$0008` at `$81:861A`, a sprite
  part) are probably other commands that set the same state. That is not tested yet.

**What it means for the C:**
- `dsp4_edges` (`$0A`) keeps its answers, and also leaves this bit for the road command.
- When `src/dsp4.c` gets the road command, the bit becomes state shared between the two.
- What the road command does with it is the road command's own measurement, still to come.

**`tools/bench/dsp4_replay`** gets the two modes above ("frame + `$0A`" and "bit 7 flipped").
Its "frames N" modes now compare only the stream's own frame.

**Check:** the tool's runs above, and the full gate (`tools/bench/` changed by more than
additions).

## The VS cars linked both ways and their parts (vs-race-2, 2026-11-25)

**Two more routines that only the VS race runs,** found by a routine census over the VS movie
(`bench --census`, with the firmware DSP-4). They are verified on that movie.

- **`vs_cars_link` (`$82:97BE`)** links the four cars in the order at `$1E6A`-`$1E70` both
  ways, car by car:
  - the one before it at `$7E:FACF` + car;
  - the one after it at `$7E:FACD` + car.

  `vs_cars_ring` writes only the second. It returns the last car, as the original leaves X.
  - Live: 303 samples. Fuzz: 500 cases, with the records' offsets in any order and
    sometimes any word.
- **`vs_car_parts` (`$83:E878`)** sets the car at D's engine words (`$1EA8`, `$1EC0`) and
  tyre words (`$1EB4`, `$1EB6`). They come from the same tables `player_frame_setup` uses
  for the one player.
  - Each part is chosen by the car's entry D+$14, through its own table (`$86:DD6D` for the
    engine, `$86:DD7D` for the tyres).
  - Live: 1,109 samples, from all four cars. Fuzz: 500 cases.

**Check:** the incremental gate (additions only).

## The third and fourth cars' race messages (vs-race-3, 2026-11-25)

**`race_message_request_0295` (`$82:9C01`) and `race_message_request_0348` (`$82:9C1D`)** are
the siblings of the first two cars' requests (`$82:9BC9`, `$82:9BE5`). Each puts a message in
its car's slot D+$33 when the message's priority, from the table at `$83:A0A4`, is at least the
current one's. The slots are `$0295` and `$0348`, the records at `$0262` and `$0315`.
- **Only the VS race runs them:** 2 and 3 live samples. The fuzz takes 500 cases for each, with
  every output compared, the flags included.
- **The fourth car's "lower priority" exit** (`$82:9C37`, the `BCS` at `$82:9C30`) never ran in
  a movie. The row covers it, and the fuzz reaches it.

**Check:** the incremental gate (additions only).

## The VS split screen's four view positions (vs-race-4, 2026-11-25)

**`vs_view_scrolls` (`$83:886A`, in `src/race_frame.c`)** is `func_838989` for the VS race's split
screen. Its caller is the split screen's interrupt handler (`$83:98A6`, not ported yet).
- **Unless the game is paused** (`$0026` bit 4), four 24-bit positions move by their signed
  steps: `$00D4` by `$01CC`, `$00D7` by `$01D4`, `$00DA` by `$01D0` and `$00DD` by `$01D8`. Each
  is a 16-bit add, then the carry and the step's sign go into the high byte.
- **Then the upper words go to the views' words**, indexed by `$00E2`:
  - the first two, plus `$3C`, to `$1536` and `$1546`;
  - the last two, minus `$4C`, to `$1556` and `$1566`.
- **The first step negative** (`$83:887C`) never ran in a movie; the fuzz runs it.
- **Live:** 1,128 samples. **Fuzz:** 500 cases. **Paths:** 69/69. The caller reloads the
  registers, so they are not compared, as for `func_838989`.

**Check:** the incremental gate (additions only).

## The VS race's start grid (vs-race-5, 2026-11-25)

**`vs_grid_setup` (`$82:BD0E`)** sets the four cars' records (`$7E:FAB5` + `$35` a car) for the
start. It is the VS race's counterpart of `race_grid_setup`, which sets the one-player race's
twenty records.
- **Each record:**
  - the distance and the words at +`$08`, +`$0A` and +`$10` are cleared, and +`$29` is set
    to 2;
  - its place on the track (+1) runs from `$019A` - 4 up, one a car;
  - its lane (+`$1C`) runs from `$0800` by `$0200`;
  - its kind (+`$10`) runs from 8 by 2.
- **The record after the fourth** gets place `$7FFF` and kind 6, an end mark.
- **The race order at `$1E6A`** is the four cars in turn; its other 16 words are that end
  record.
- **D+`$14`, D+`$16` and D+`$18` hold the three counters**, as the original leaves them.
- **Live:** 1 sample, the race's start. **Fuzz:** 500 cases, with `$019A` varied. **Paths:**
  61/61. Every output is compared.

**Check:** the full gate. The last full run was 5 commits back, so the incremental gate's
carry rule no longer holds.

## The VS cars' steps shared out (vs-race-6, 2026-11-25)

- **`vs_step_share` (`$82:BDDA`):** minus a signed word over the step count `$1E9E`, on the
  CPU's 16 / 8 divider. It takes the magnitude's quotient (`$FFFF` for a zero count). A negative
  word gives it as it is; a positive one, or 0, gives 0 less it.
  - **It branches on the caller's N flag**, not on the word. Its first instruction is a `BPL`,
    and the caller's last flag-setting instruction is its load of the word.
  - The first fuzz run found this: with random flags, 220 of 500 cases differed. The C takes the
    flag as an argument.
  - Live: 1,115 samples. Fuzz: 500 cases. Every output is compared.
- **`vs_steps_share` (`$82:BDA9`):** for the four cars in turn (records `$0200`, `$02B3`,
  `$0262`, `$0315`), it saves the car's word (`$01CE`, `$01D2`, `$01D6`, `$01DA`) to the
  record's +`$29`. Then the word becomes `vs_step_share` of itself.
  - Live: 263 samples. Fuzz: 500 cases.

**Check:** the incremental gate (additions only).

## The VS race's players' frame setup (vs-race-7, 2026-11-25)

**`vs_player1_frame_setup` to `vs_player4_frame_setup`** (`$83:E597`, `$83:E64D`, `$83:E706`,
`$83:E7BF`) are `player_frame_setup` for the VS race's four players. All four are one function
in `src/vs.c` over a table of what differs:
- the view's bit for `$1EBE`;
- the choice byte (`$0418`-`$041B`);
- the record (`$0200`, `$02B3`, `$0262`, `$0315`);
- the record's offset for `$1EBA`;
- the view's base for `$1EBC`;
- the player's bit of `$03DC`;
- the pad (`$000C`, `$0010`, `$0014`, `$0018`, held, then pressed);
- the controls' masks (`$039D` for players 1 and 3, `$03CC` for 2 and 4).

**What each one does:**
- **It sets the frame's car:**
  - D+`$14` = twice the choice, and `$1EB2` = the choice's entry at `$86:DD75`;
  - the record's +`$2B` = `func_83EA9B` of twice +`$1F`, plus the word at `$86:DAD9` + twice
    the entry's low 7 bits;
  - the engine and tyre words, from `vs_car_parts`;
  - `$1EB8`, `$1ECA` (the player's number - 1), `$1EBA`, `$1EBC`, and `$1E9C` cleared.
- **Then it takes the controls or the road limits:**
  - when the record's +`$44` bit 2 is clear and the player's bit of `$03DC` is set, it reads
    the controls into `$1E9C` from the pad and returns;
  - otherwise it runs `player_road_limits` for the record (D = the record).
- **The controls' path never ran in the VS movie for players 2-4.** Their bytes were never
  decoded by the listing (`$83:E6BF`, `$83:E778`, `$83:E831`), so they were read from the ROM by
  hand. For player 1, only its button presses never ran.

**Results:**
- **Live:** 303, 296, 281 and 126 samples.
- **Fuzz:** 500 cases each, half of them on the controls' path.
- **Paths:** 71/71 and 72/72.

The callers reload the registers, so they are not compared.

**Check:** the incremental gate (additions only).

## The VS players' places (vs-race-8, 2026-11-25)

**`vs_places_update` (`$82:9AF4`)** handles the four players in turn (records `$0200`, `$02B3`,
`$0262`, `$0315`, indices 4 to 1).
- For each one, `vs_cars_ahead` counts the cars ahead of it.
- When the count differs from the player's last one (`$1ED2` + 2 a player), the count is kept
  and the place's message (6 + the count) is requested in the player's slot. The request goes
  through that player's `race_message_request_*` (vs-race-3).
- **Live:** 239 samples. **Fuzz:** 500 cases, with the cars' distances near each other and the
  last counts and the slots at random. **Paths:** 48/48.
- Its caller reloads the registers, so they are not compared.

**Check:** the incremental gate (additions only).

## The VS race's cars' frame (vs-race-9, 2026-11-25)

**`vs_cars_frame` (`$83:E28C`, RTL)** is `player1_frame` for the VS race's four cars.
- It counts the frame at `$7E:4CDD` + `$1E9E` × 2.
- Then, for each player in turn, it runs the player's frame setup (vs-race-7), then
  `player_car_frame` and `car_track_step` with D = the player's record.
- **Live:** 65 samples pass. The other entries were interrupted, or were nested in rows that
  ran at the same time. **Paths:** 35/35.

**Its fuzz is skipped on the VS movie**, its only movie, like `player_car_frame`'s
(harness-28, dsp4-11a).
- A first fuzz moved one car as `player_car_frame`'s own fuzz does: 395 of 500 cases ran past
  the instruction budget.
- With only the players' setup varied, and the cars left as the movie left them, 26 to 33 of
  500 still ran past it.
- Every case that finished passed (467/467, then 474/474). So the lost cases are the VS race
  RAM's long car frames, not a mismatch.
- `verify_all.sh`, `verify_new.sh` and `tools/rerun_hidden.py` add `vs_cars_frame` to the VS
  movie's `--fuzz-skip`.

**Check:** the full gate (the scripts changed by more than additions).

## What the road command does with the bit (dsp4-01c, 2026-11-25)

**The bit dsp4-01b found, measured over the first 400 race frames of the TAS capture.** Each
frame was replayed from its last `$0A`, with bit 7 of that command's near row as sent and then
flipped, on the firmware chip. This is outputs only.

- **Only the road command's first word of a row changes.** That is the word the game stores at
  `$0798` + X: read at `$85:B735`, or at `$85:B747` for the last row.
  - 5,299 bytes change, 4,910 at the first PC and 389 at the second.
  - Each one is exactly 1 more with the bit set than with it clear, always in the low byte.
  - Only the first rows of each frame change, about 13 a frame: every change is within 196
    records of the road command. The row's other word, the X index, and everything after are
    the same, the sprites' `$09` stream included.
- **The game always sends the bit set here.** In the 8,000 frames, all 37,601 `$0A` commands at
  `$81:F90E` send the near row `$C4C4`, whose bit 7 is set.
  - So on this track the road command always gets the "+1".
  - A native road command must still take the bit as state shared with `$0A`. A chip just
    powered on has it clear, and other tracks may send other rows.
- **`tools/bench/dsp4_replay`** now prints this measurement (the changes by PC and by
  difference, and the near rows sent).

**Check:** the tool's run above. It adds lines to `tools/bench/` only and changes no row or C.

## The VS race's second band merge (vs-race-10, 2026-11-25)

**`race_bands_merge2` (`$85:B2B7`)** is `race_bands_merge` for the VS race's second pair of views:
- the lists come from `$7E:4D96` + `$0166` and `$7E:4DE2` + `$016A`;
- the start depends on `$00B6`, and is kept in `$00C6`;
- the first marks go to `$0482`;
- the merged lines go to `$050E` + Y, with their flags at `$061E` + Y.

`src/misc.c` now has one `bands_merge` over a table of the two views, and `race_bands_merge` is
its first view.

**`vs_bands_merge` (`$85:B1F9`)** is the two merges in turn: a `JSR` to the first, then a `JMP`
into the second.
- Its row declares that tail jump (`alsoIn`).
- `race_bands_merge2` has no live samples of its own, since it is only reached by that `JMP`.
  Its fuzz takes 500 cases and covers its 79 paths.

**Results:**
- `vs_bands_merge`: 222 live samples, 500 fuzz cases.
- `race_bands_merge` keeps passing on the race and VS movies: 181 and 256 live samples.

**Check:** the full gate (`src/misc.c` changed by more than additions).

## The VS race's centred line (vs-race-11, 2026-11-25)

**`vs_centred_line` (`$82:A134`)** is `text_centred_line` in the VS race's font.
- The font's parts: the blank at `$83:9F52`, and the two rows' tiles at `$83:9F6A` and
  `$83:9FB8`.
- **The marks:** space, `!` and `+` are tiles `$26`, `$24` and `$25`.
  - The font has no `.`: it falls to the digits' "- `$16`".
  - Its `+` and its palette bytes never ran in a movie. Their bytes (`$82:A182`, `$82:A1A5`)
    were read from the ROM by hand, and the fuzz runs them.
- **`src/text.c` now has one `centred_line` over the font** (`CentredFont`: blank, tiles and
  marks). `text_centred_line` is the race's font, and the adapter's C/V rule takes the font's
  marks.
- **Results:**
  - `vs_centred_line`: 27 live samples on the VS movie, 500 fuzz cases, 73/73 paths.
  - `text_centred_line`: still 6 live samples and 500 fuzz cases on the race movie.

**Check:** the full gate (`src/text.c` changed by more than additions).

## The VS race's message update (vs-race-12, 2026-11-25)

**`vs_message_update` (`$82:A0C6`)** is `race_message_update` in the VS race's font.
- It draws the new message with `vs_centred_line`, and blanks it with the VS blank
  (`$83:9F52`) when its time is up.
- `src/text.c` now has one `message_update` over the font, as `centred_line` is (vs-race-11).
- Its callers are the VS race's HUD, one call a player from `$82:A0AB` and its siblings. That
  HUD also talks to the DSP-4 and is not ported yet.

**Results:**
- **`vs_message_update`:** 1,149 live samples on the VS movie, from all four players' lines;
  500 fuzz cases; 51/51 paths.
- **`race_message_update`:** still 216 live samples and 500 fuzz cases on the race movie.

**Check:** the full gate (`src/text.c` changed by more than additions).

## Two VS cars close in distance or sideways, and their collision (vs-race-13, 2026-11-25)

**`vs_cars_tie` (`$81:F6AE`) and `vs_cars_apart` (`$81:F6F2`)** take two of the VS race's cars, X
and Y, as record offsets into `$7E:FAB5`. Both entry and exit with D = `$0100` and DB = `$7E`.

**`vs_cars_tie`:** the two cars' high distance words are equal.
- If their low words are `$100` or more apart, `$013C` is set to 0 and it returns with C clear.
- Otherwise `$013C` counts up, and D+`$14` takes its value. Then it walks the ring from Y
  (`$FACD`), each step counting D+`$14` down:
  - a car less than `$40` sideways from X goes to the shared tail;
  - a car `$100` or more ahead ends the walk with C set;
  - when the count runs out, it returns with C clear. This never ran in a movie.

**`vs_cars_apart`:** cars `$40` or more apart sideways return C clear. Closer ones go to the shared
tail, which never ran in a movie.

**The shared tail (`$81:F70B`)** first stores, in the direct page:
- the two records;
- their players' records, taken from `$81:FDEE` by record / `$35` on the CPU divider;
- the 32-bit gap of their saved distances (`$FAC1`/`$FAC3`).

Then one of two things happens:
- **If the gap is under `$100`,** car X is put `$40` to the side of car Y, on the side away from
  Y by their `$FAD5` words. The player's `$0206` gets the same.
- **Otherwise the cars collide (`$81:F7A5`):**
  - each player's `$0216`/`$0218` gets the other's `$020E`/`$0210`, doubled, divided by its own
    `$022B`. The division is a 40-step shift-subtract loop kept literally, since steps 33-40
    divide the quotient's own bits;
  - car X's distance becomes Y's less `$100`, but not below its saved one (never ran);
  - the distance X lost also comes off its player's race distance (`$0225`/`$0227`), unless that
    player's `$0244` bit 0 is set.

**Three never-decoded spots were read from the ROM by hand:**
- `$81:F6E7` (the walk's count);
- `$81:F709` (`PHX`, `PHY` into the tail);
- `$81:F83C` (the clamp).

**The collision block (`$81:F7A5`-`$81:F875`) was "never run" in the map but runs in the VS
race** with the firmware DSP-4: the first test caught the original leaving its extent there.
- So the firmware VS run's coverage is now merged into `traces/coverage.bin`. That added 218
  bytes, and `map/` and `asm/bank_01.asm` were regenerated.

**First use of delegated agents** (the owner's MCP; their output untrusted, checked here):
- `ask_claude` (balanced, read-only, billed to the API key) drafted both routines from the
  listing, and then the collision block. They cost US$0.47 and US$0.20.
- The drafts were right where they had the bytes. The never-run spots, the ring walk and the
  16-bit `DEC` were completed here.
- The agents never get the ROM or the firmware: the bytes they needed were decoded locally.

**Results:**
- `vs_cars_tie`: 661 live samples, 500 fuzz cases, 34/34 paths.
- `vs_cars_apart`: 6 live samples, 500 fuzz cases, 14/14 paths.
- The fuzz reaches the walk, the push and the collision.

**Check:** the incremental gate (additions only).

## The VS race's car order, and its players' cars' motion (vs-race-14, 2026-11-25)

**`vs_cars_order` (`$81:F4AF`, RTL, DB `$83` at exit)** puts the VS race's four cars in order
each frame. A delegated agent drafted it (US$0.51). Its two never-run lap paths were decoded
here, with the listing's own opcode table run locally, and completed. Each frame:

1. **The ring** of next pointers (`$FACD`) is relinked in the order, last place first.
2. **Each car** gets its side copied to `$FAD5`, its distance saved to `$FAC1`/`$FAC3`, `$FAC7`
   cleared, and its handler run by kind (`cars_handler`, now shared with the one-player race's
   `src/misc.c`).
3. **An unrolled insertion sort** runs over the three pairs from the back:
   - equal high distance words: `vs_cars_tie`, no swap;
   - otherwise `$013C` = 0, and a car behind swaps unless `vs_cars_apart` leaves C set;
   - a car moved up is compared again one slot higher.
4. **The laps** (the `$FAB6` word, the distance / 256, against `$019A`):
   - the cars at the back that are past the lap lose a lap and go to the front;
   - otherwise a first place below 0, and those after it, gain one and go to the back. This
     never ran.
   - **The order turns once a car, but only three times for all four**: the original counts
     no fourth. The first fuzz caught this, with 4 of 500 cases.
   - Then the sort runs again.

**The VS players' handlers were C but never verified:** `car_sync_slot` (`$81:FC72`, `$81:FCD1`,
`$81:FD30`, `$81:FD8F`; handlers 8-14) is the car taking its record's motion. Each now has its own
entry and row (`car_sync_slot_0200` ... `_0315`): 281-292 live samples each and 500 fuzz cases.

**The order's fuzz has limits** (`fuzz_vs_cars_order`): made-up states where the original runs
for ever were found one lost case at a time.
- **At most one car past the lap, as in a race.** Two or more past it, with cars side by side,
  collide and lose laps for ages.
- **All four below 0 needs a lap length of `$8000` or more.** The leader is tested against the lap
  unsigned first, so with a shorter lap a negative leader counts as past it. That case also keeps
  the cars apart in distance and sideways, with their handlers the RTS.

**Results:**
- `vs_cars_order`: 183 live samples, 500 fuzz cases, 180/180 paths.

**Check:** the full gate (`src/misc.c`'s `cars_handler` and `src/track.c` changed).

## The VS players' speed and band words (vs-race-15, 2026-11-25)

**`vs_car_bands` (`$82:9806`)** is `func_82AC52` for the VS race's four players in turn: records
`$0200`, `$02B3`, `$0262`, `$0315`, with cars 0, `$35`, `$6A`, `$9F`. A delegated agent drafted
it (US$0.35) and recognised it as `func_82AC52`'s sibling. Its never-run parts were decoded here
with the listing's opcode table: a cap, two zero floors, and a `$4C`-byte block a player.

For each player:
- **The speed word** (`$7E:4E2D` + 2 a player) comes from the DSP-4 product of the player's +`$13`
  and `$0300`: bits 8-23, less `$0B00`. It is capped at the player's limit: `$3100`, `$2E00`,
  `$3100`, `$2D80`.
- **The band word** (`$7E:4E35` + 2 a player) is set when the player's +`$44` bit 0 is set:
  - it is `$58` less the car's lead over the player in its high distance word;
  - the lead is at least 0, and the result is rounded down to 8.
- **With bit 2 set**, the speed is worked out again from the lead on the CPU divider. This never
  ran.

The one-player race's version has a limit of `$3000` and its band from twice the lead.

**Results:** 278 live samples, 500 fuzz cases, 303/303 paths. The fuzz runs every never-run part.

**Check:** the incremental gate (additions only).

## The VS race's HUD (vs-race-16, 2026-11-25)

**`vs_hud_digits` (`$82:9C39`)** is `race_hud_digits`' first half for the VS race's four players in
turn (records `$0200`, `$02B3`, `$0262`, `$0315`). A delegated agent drafted it (US$0.47). Its
never-run parts were decoded here; one of them settled the agent's open guess.

For each player, unless the record's +`$44` bit 2 is set:
- **The bar:** the high word of +`$13` times `$0160`, at most `$0C`. A negative value is negated
  (never run).
- **The speed's three digit tiles** (`$83:9F54`; the blank at `$83:9F52` stands for leading
  zeros): from the high word of 8 × +`$10` times `$4000`, split on the CPU divider.
- **The offset word:** `$55` less the high word of +`$40` times `$019E`, at least 0, plus the
  view's base.

**Then, always,** the player's message is updated in the VS font (`vs_message_update`, vs-race-12)
through the D-page words.

**The agent's open guess:** bit 2 set skips to the message. The never-run bytes at `$82:9C41` are
`JMP $9D21`, which jumps past the bar, the digits and the offset.

The DSP-4 commands used are all `$0000`, the multiply, which the C has natively.

**Results:** 303 live samples, 500 fuzz cases, 461/461 paths.

**Check:** the incremental gate (additions only).

## The VS race's track objects' graphics (vs-race-17, 2026-11-25)

**`vs_track_objects_stream` (`$82:B902`)** is `track_objects_stream` for one of the VS race's four
views. A delegated agent drafted it (US$0.49) and found its four callers, the setup wrappers at
`$82:B87A`-`$82:B8DF`, which are not ported yet. Each wrapper sets the parameters in the direct page:

| Word | What it holds |
|---|---|
| D+`$36` | the player's distance |
| D+`$38` | the list index, kept as it moves |
| D+`$3A` | the view's VRAM base |
| D+`$3C` | its block in the slot tables, `$DC` bytes a view |
| D+`$34` | a bias added to the slots stored |

A holds the block's first word.

**How it differs from the one-player routine:**
- It does not skip a kind with no graphic.
- Its check that the VRAM word is within `$1000`-`$2FFF` does nothing: the never-run byte the
  check branches to is a `NOP`, decoded here.

**The never-run spots, decoded here:**
- the "no record reached" exit, a `JMP` to the RTS;
- the kind-0 record: the block's first word becomes 2 × slot + bias.

The fuzz runs both.

**Results:** its 4 entries in the VS movie are all interrupted; the 2 kept replayed from their entry
state pass. Fuzz: 500 cases. Paths: 128/128.

**Check:** the incremental gate (additions only).

## The VS race's four views' object graphics (vs-race-18, 2026-11-25)

**`vs_track_objects_view0` to `vs_track_objects_view3`** (`$82:B87A`, `$82:B89C`, `$82:B8BF`,
`$82:B8DF`) set up `vs_track_objects_stream` (vs-race-17) for each view, and keep its list index.
- **The view's constants:** the slot-table block (0, `$DC`, `$1B8`, `$294`), the VRAM base (0,
  `$800`, `$1000`, `$1800`), the bias (0, `$80`, 0, `$80`) and the first word (`$100`, `$180`,
  `$100`, `$180`).
- **The player's distance:** its record's +`$26` (`$0226`, `$02D9`, `$0288`, `$033B`).
- **The list index:** read from D+`$A0`, `$A4`, `$A8`, `$AC`, and stored back.

They are one C function over a table.

**Results:** one entry each in the VS movie, interrupted, replayed from its entry state: pass. Fuzz:
500 cases each, on the stream's records. Paths: 14-15 each.

**Check:** the incremental gate (additions only).

## VRAM reads served to the C (harness-30, 2026-11-25; the advisor's practice, open to the owner's veto)

- **The need.** The VS HUD (next section) reads a tilemap word back from VRAM and writes it
  back ORed with some bits. Until now the C could only write VRAM: harness-9 lets a row's dead
  VMDATA read through, but no live one.
- **A tape, as for the DSP-4 (harness-4).**
  - A row lists its live read sites (`vramLiveAt`, up to 4 PCs of a 16-bit `LDA $2139`) and
    more dead ones (`vramDeadAt`, besides `vramReadAt`).
  - At a live site, the words the original reads are kept in order, up to 8.
  - The C reads VRAM through a new platform call, `video_read_vram_word` (`src/platform.h`).
    On the bench it returns those words in order. The C must read exactly as many: a short or
    long count fails the sample.
  - The values themselves are checked through what the C does with them: the words written
    back are compared as effects.
- **The negative check, done by hand and not kept as a row.** A C that read one word more failed
  every live sample and every fuzz case: "the C read VRAM word 4, past the original's 4".
  - It is not a negative row, because `verify_all.sh` runs those over the 60k movie only, and an
    unreached negative row fails it.
- **Check:** the full gate (`difftest.c` and `platform_bench.c` changed).

## The VS split screen's HUD in VRAM (vs-race-19, 2026-11-25)

**`vs_hud_draw` (`$83:9B49`)** is `race_hud_draw` for the VS split screen's four players. Its caller is
the split screen's interrupt handler (D = 0, DB = `$83`). A delegated agent drafted it (US$0.63)
and pointed out the live VRAM read that harness-30 now serves. Player 1's two never-run spans were
decoded here: its bar's loops.

For each player it writes:
- **a 6-word bar** from its bar word (`$3E`, `$40`, `$64`, `$68`): full segments (`$83:9F2E`), a
  half one when the bar is odd (`$83:9F3A`), then empty ones (`$83:9F46`);
- **three words**;
- **a 6-word count** from the low byte of `$024E`, `$0301`, `$02B0` or `$0363`: n of `$83:9F68`,
  then `$83:9F52`;
- **one tilemap word read back:** the second read after a dead one, at the VRAM address in `$A4`,
  `$AC`, `$A6` or `$AE`. It is rewritten ORed with `$6000` or `$0006`;
- **two 26-byte strips** from `$7E:4E61` + `$34` a player, or from `$83:A3DD` when its accumulator
  pair (`$7E:4E4D`/`$4E4E`, ...) adds up negative.

**Results:** 1,121 live samples, 500 fuzz cases, 377/377 paths.

**Check:** the full gate (with harness-30).

## The VS split screen's NMI frame (vs-race-20, 2026-11-25)

**`nmi_vs_full` (`$83:9777`)**, with **`nmi_vs_prologue` (`$83:9721`)** and its tail
**`nmi_vs_tail` (`$83:9734`)**, is the VS race's NMI: `nmi_race_full`'s counterpart.
- **Where it comes from:** `$82:945F` points the NMI trampoline at `$83:9721`, which dispatches
  through `$0008` to `$83:9777`. That target is set at `$82:9459`.
- **Drafting:** a delegated agent drafted it (US$1.09, the largest so far). Its never-run spans
  were decoded here: a VRAM transfer, the two clocks' minute carries, two sign-extends, and the
  bands' empty cases. One branch was fixed: a negative entry takes the zero path.

**What the frame does:**
- **Every request** (`$00B8`, cleared whole): it sets the scroll tables (slots 5 and 6, from
  `$00D0`/`$00D2`), flips the double buffer, copies 19 per-frame words, and runs the VRAM
  transfer described at `$1DFE`-`$1E04` (never run).
- **Every frame:**
  - the sprite table;
  - colours `$21` and `$31` from ROM `$90`, by the word at `$7E:5395`;
  - the race VRAM queue;
  - the HUD, when `$0800` of the `$1000` budget is left (`vs_hud_draw`);
  - the four views' scrolls (`vs_view_scrolls`).
- **The scroll registers' interleaved byte writes** leave BG1 H = M7 H = `$0100`, BG1 V = 0,
  M7 V = `$0053` and BG2 H = 0. That is worked out from the PPU's write-twice latches.
  - BG1 V and M7 V end apart, so `src/platform.h` gains `video_set_m7_vofs` (Mode 7's vertical
    offset alone).
- **Two clocks** in BCD (`$5A`-`$5C`, `$5D`-`$5F`), with no pause test, unlike the race's.
- **HDMA and the streams**, as in the race, but with the window from `$83:A411` and only slots 5
  and 6 enabled.
- **The two views' band IRQs:**
  - the first view through `$0486`/`$0596`;
  - the second view through `$050E`/`$061E` into `$0030`-`$0036`.

  Their IRQ handlers (`$83:8A26`, `$83:8A84`, `$83:8BE3`) are not ported yet.
- **One of the four cars' sound request,** picked by the frame count through the jump table at
  `$83:9AF0`.

**Results:**
- `nmi_vs_full`: 1,128 live samples, 500 fuzz cases, 377/377 paths. The fuzz runs every never-run
  span.
- `nmi_vs_prologue`: 1,128 live samples, 500 fuzz cases.

**Check:** the full gate (`src/platform.h` and `platform_bench.c` changed).

## The VS race's raster IRQs (vs-race-21, 2026-11-25)

**`irq_vs_band` (`$83:8A26`) and `irq_vs_band2` (`$83:8BE3`)** are `irq_race_band` for the VS race's
two pairs of views. They share one helper.
- **Each band:** it sends the band's stream chunk, naming the slots from the mask table, then moves
  the IRQ to the next band's line at dot `$63`.
- **The tables:**
  - top views: cursor `$00BC`, masks `$0596`, lines `$0486`;
  - bottom views: cursor `$00C2`, masks `$061E`, lines `$050E`.
- **After the last band:**
  - the top pair hands over to the split (`$83:8A84`) at line `$6C`, dot `$78`;
  - the bottom pair sets the IRQ line to 0 and leaves the trampoline as it is.
- **Results:** 4,000 live samples each, paths 44/44 and 40/40. There is no fuzz, as for
  `irq_race_band`: they wait (`WAI`) for an interrupt the fuzz never raises.

A delegated agent drafted all four VS IRQ targets (US$0.52). Two are not ported:
- **The split (`$83:8A84`):** it stores the HDMA register `$4342` (slot 4's current table address)
  into `$7F:FFFE`, a hardware read the C has no call for yet.
- **The first target (`$83:8B8E`):** it only acknowledges, and each NMI frame replaces it, so no
  movie runs it.

**Check:** the incremental gate (additions only).

## A DMA register read served to the C (harness-31) and re-pointed streams (harness-32), 2026-11-25

**The advisor's practice, open to the owner's veto.** The VS split screen's split IRQ (next
section) needed two things the bench did not model.

**harness-31: a DMA register read.**
- The split stores HDMA slot 4's table address, read back from `$4342`, into `$7F:FFFE`.
- Like harness-30's VRAM reads, a row lists its read sites (`dmaReadAt`). The words the original
  reads there are kept in order and served to the C through a new platform call,
  `dma_hdma_get_table`. The C must read as many.

**harness-32: a stream re-pointed inside a routine.**
- **What the split does:** it sets only the source (and on slot 7, the mode and target too) of
  streams the NMI set up. Then it kicks them. Before, any partly set-up channel at a kick was
  refused.
- **The new rule:** a channel is a stream re-pointed when its kick is not an upload.
  - Some of its mode, target or source were set in the sample, but not its bank.
  - Or a new mode and target were set since its last kick, its bank kept. This holds even when
    an upload set the channel up earlier: slot 7 carried the sprite table just before.
  - A new source alone after such an upload is another upload (`race_start_frame_draw`). The
    first full run, with a rule that took any register set since the last kick, failed it.
  - The new registers are an effect of their own (`EFFECT_STREAM_SOURCE`), compared value for
    value, before the stream kick.
  - The C gets a new platform call, `dma_stream_set_source` (a stream's source address alone),
    and uses `dma_hdma_set_mode` for the mode and target.
- **Streams and uploads in one kick** are now modelled when no upload goes to CGRAM. Then the
  CGRAM address the streams start from is the kick's on both sides.
- A refusal of a partly set-up channel now names the channel and its registers.

**Check:** the full gate. Every row is judged again under the new rules.

## The VS split screen's split IRQ (vs-race-22, 2026-11-25)

**`irq_vs_split` (`$83:8A84`)** is the IRQ at line `$6C`, after the top pair's last band, between the
VS race's two halves. A delegated agent drafted it (in vs-race-21's request).

**At dot `$10A`** (the original waits, `WAI`):
- forced blank;
- the bottom pair's sprite table (`$19B6`/`$1BD6` by `$00B6`);
- colours `$21` and `$31`;
- the streams re-pointed to the bottom pair's sources (harness-32);
- their first chunk (`$0032`);
- OBSEL = 8.

**At line `$73`**, the black divider (it waits again):
- the display back (`$014B`);
- colour 7;
- slot 4's table address to `$7F:FFFE` (harness-31);
- the bottom pair's band IRQ: the handler word at `$0036`, line `$0034`, dot `$63`.

**Results:** 1,128 live samples, 111/111 paths. There is no fuzz: it waits twice.

**Check:** the full gate (with harness-31 and harness-32).

## The player's password shown (menu-23, 2026-11-24)

- **`$8A:BE2E` shows the player's password** after a championship race.
  - It loads the password screen's graphics and packs and encodes the password.
  - It prints the password centred and waits for a press on any pad, between the fades.
  - It runs once in the 60k movie.
- **Rows in `src/password.c`**, cut at the waits:
  - `password_show_open` (up to the fade out; the pushed NMI handler above the cut);
  - `password_show_graphics`;
  - `password_show_text` (up to its 8-frame wait);
  - `password_show_wait`, a waiting row: the fade in, a press (the first pad with B/Y/A/X
    has its press taken), the fade out.
  - `password_show` composes them with `fade_out_wait`, `password_hdma_off`, the wipes and
    the waits. `options_screens` now calls it (the trap is gone).
- **Results:** each passes on the 60k movie. Three are interrupted and pass from their
  entry state.
- **Check:** full gate (`options_screens` changed).

## A dead VRAM read; the effect cap (harness-9, 2026-11-23)

**The advisor's call, open to the owner's veto.** A password movie (next unit) runs
`hud_markers_write` with a marker set. That path was a hole: the original steps over a set
marker's word with `LDA $2139`, a 16-bit read of VMDATA, and the harness took any such read
as a hardware access.
- **A row now names the PC of a dead VMDATA read** (`vramReadAt`). At that PC, for every
  row, a read of `$2139`/`$213A` is not a hardware access.
  - The value is dead by the listing: A is reloaded before any use.
  - Its effect is the step of the VRAM address. The PPU's own pointer takes it, and the
    harness records VRAM word writes at that pointer, so the next words land where the
    original's do.
  - No platform call: the platform's word writes name their addresses, so a port has
    nothing to do for a skipped word.
- **Measured at the read:** VMAIN = `$80`, one word after the high byte, so the 16-bit read
  moves the pointer from `$7C57` to `$7C58`. `$7C57 + i` holds.
- **`hud_markers_write`:**
  - Its hole is gone, and the fuzz sets the markers (`$0398-$039B`, 0 or a part's byte).
  - With all four set and no badge, the original sets `$2116` and writes nothing, so the
    address stays set for the next routine. The C now says so then
    (`video_set_vram_address($7C57)`), and only then.
  - A negative row (`hud_markers_write_wrong`: the cleared markers at consecutive addresses)
    fails on the 60k movie, live 5/5 but fuzz 17/50: a wrong address no longer passes.
  - Result: live 1/1 (with a marker set, on the password movie), fuzz 500/500, paths 26/26
    on race1 and on the password movie.
- **The effect cap** (`MAX_EFFECTS`) goes from 128 to 512. On the password movie (a later
  track) `track_objects_stream`'s replay makes 158 effects, all of one kind, and it failed
  on the cap. It passes now: fuzz 500/500, paths 108/108.
- **Gate:** the full one.

## Buying a part (shop-18, 2026-11-22)

**The buy dispatch's handlers** (`$80:9039` picks one from `$81:A89F` by list and row,
through a pushed address and `RTS`; it waits for lists 6-8's handlers before its own port).
- **Lists 1-5** (`$80:9053`, `$9100`, `$91AE`, `$925C`, `$95C0`) are the same code, as one
  C function (`shop_buy_part`). Their differences:
  - the level byte, `$0393`-`$0397` + D+$94 (bit 7: bought here);
  - the panel's height: `$78` for list 1, `$B4` for the others;
  - on one path, list 1 calls `shop_money_after_price` before `shop_flag_fb`, the others
    after;
  - list 1 ends with a `JMP` to `shop_item_icon_settle`, so its row ends there (`endAt`,
    as harness-7's case).
- **The flow:**
  - A part already owned: a sound, the icon shown and settled.
  - Else the money is taken. A part bought here before is sold back first
    (`shop_money_after_refund` $80:990D, a BCD add). If the money does not cover the price,
    `shop_flag_fb` and out.
  - Bought: the panel rises 6 a frame with the money rolling. The level byte becomes the
    row | `$80`, the upload is queued, and the money after the price is computed. Then the
    panel falls with the money rolling to it, the money is stored and the icon settles.
- **List 0** (`$80:930A`): the car's colour byte `$0392` + D+$94 = the row, then
  `func_80E4DB`.
- **Exit flags** (few returns, so all four are compared):
  - through `shop_item_icon_settle`: N and C set, Z clear;
  - the money short: `shop_flag_fb`'s `PLA` of the A the redraw left (`oam_finish`: the new
    D+$6C, never 0 or negative);
  - short after a refund: `shop_money_after_price`'s.
- **Verified (pooled over the movies):** `shop_buy_list4` (every path) and
  `shop_buy_colour` (live 28/28, fuzz 500/500).
  - list 4's "short, no refund" path (`$80:929E-$92A3`) is a hole: no movie runs it, and a
    waiting row has no fuzz.
  - Lists 1-3: the shop movies run their owned and short paths. Their refund and buying
    paths were holes until the password movie bought from them (explore-3).
  - Unchecked: list 4's sequence "refund, then affordable, then bought" (its one refund
    sample ended short; both halves are visited separately, so path coverage cannot show
    the gap).
  - List 5's row is left out: the movie only shows an owned part there, so its buying paths
    would stay unvisited. Its C is the same function list 4 verifies.
- **The refund path does run.** It ran once on the movie (a level-1 part, then row 2 too
  dear), so `$80:990D` is no longer "never run".
- **The movie's coverage is now merged into the map:** code grows from 70,199 to 70,518
  bytes. Two `player_car_frame` ranges it executes left `map/static_code.txt`.

## The buying's money (shop-17, 2026-11-22)

**Three routines that only the shop-buy movie reaches**, in `src/shop.c`. Money is in BCD:
the player's at `$038E/$0390` + D+$94, the target at D+$52/D+$54.
- **`shop_money_after_price` ($80:9957):** the money less the current item's price, a
  decimal-mode `SBC` (the price at `$81:A890`/`$A88E` + the item's word at `$81:A89B`), to
  D+$52/D+$54. Then it marks the digits that are settled.
  - From the top, the digits that neither number has, or that stay the same, go in D+$58.
    The mask tested each time is every digit from the top down to the current one: the
    original does `ORA $58` before the four `LSR`s.
  - If all four high digits settle, the low word's leading zeros go in D+$56 the same way.
    No run takes that path; `map/static_code.txt` declares it, and the fuzz runs it.
  - Then D+$5A = 4.
  - **Its exit flags are modelled** (only 3 returns watched):
    - V is bit 14 of the last word `BIT` tested (D+$7C, or D+$7A in the second loop);
    - C is the last `CMP`'s or the last `LSR`'s.
- **`shop_money_roll` ($80:99D1):** one step of the money rolling down to its target.
  - The settled digits from the top are passed over.
  - At the first other digit, D+$5A counts down. At 0 that digit takes its new value and
    settles (D+$5A = 4). Otherwise it and every digit below roll down by one, 0 wrapping to
    9 (a 32-bit `SBC` of `$1111` under the mask, then `ADC #$AAAA` on a borrow).
  - C is the last `ROR`'s. V is the last `SBC`'s or `ADC`'s, or as on entry when none ran.
- **`shop_money_store` ($80:9ABB):** D+$52/D+$54 become the money.
- **The C's decimal `SBC`** (`bcd_sub16`) is exact for valid BCD only. The game's money and
  prices always are, and so is the fuzz's.
- **Result on the shop-buy movie:** live 3/3, 61/61 and 2/2; fuzz 500/500 each; paths
  65/65, 106/106 and 6/6; every flag compared.

## The shop-buy movie wired in (explore-2, 2026-11-22)

**`tools/bench/movies/explore_buy.txt` is now the third movie** of `verify_all` (two slices)
and `verify_new`, judged with the other two under harness-8.
- **Its header** says how it was made and that it ends in a paused race, the fuzz base of
  every row it enters. The unmeasured "8,888 credits plus the bonus" is gone.
- **What it brought before being wired in:** fix-1 and fix-2 (two defects in verified C),
  kind 0, fuzz-pin-1 and fuzz-pin-2.
- **It reaches the buying code** (`$80:9039`, `$80:99D1`, `$80:9957`, `$80:9ABB`). Those
  routines are next to port.
- **The check:** the full gate, now over three movies.

## Evidence pooled over the movies (harness-8, 2026-11-22)

**The advisor's design, open to the owner's veto.** Every routine is verified over each
movie. Until now, a movie that entered a routine only a few times failed it by rules meant
for a whole run:
- fewer than 100 returns watched made every flag live;
- every path had to be visited on that movie;
- a waiting row that a slice left unsampled had nothing compared.

On the shop-buy movie, about 17 rows failed this way and nothing else.
- **The bench is unchanged in its verdict.** The selftest and `--difftest-only` rely on it.
  Each result line now also carries that movie's raw evidence:
  - `watched:` the returns watched;
  - `read:` the flags seen read, before the 100 rule;
  - `mism:` the flags with a compared mismatch;
  - `hard:` a failure other than the flag and path rules (a compared mismatch, a lost case,
    a hardware access, a broken contract, a failed or lost replay);
  - `compared:` the cases compared;
  - `vis:` the paths visited, one bit per instruction start.
- **`tools/merge_verified.py` judges on the pooled evidence:**
  - a hard failure on any movie fails the routine;
  - a movie that reached it but compared nothing gives no evidence;
  - the 100 rule applies to the returns watched, summed over the movies;
  - the live flags are the union of the flags seen read, and a mismatch on one of them, on
    any movie, fails the routine;
  - the paths are the union over the movies;
  - verified needs at least one movie that compared something.

  The committed columns stay as they were.
- **`verify_all` and `verify_new`** take the verdict from the merge, not from the bench's
  status 4. Any other status still fails the slice.
- **Checks:**
  - `tools/test_merge.py` (no ROM, in CI): nine cases, among them a hard failure on one
    movie failing the merge.
  - A full run against the last commit's `map/verified.txt`: every status and count is the
    same (469 rows).
  - No row's live flags grew. 13 shrank, because their summed returns now pass 100:
    - all four flags dropped for `func_80B58D`, `star_random_y`, `tilemap_offset_c2cc`,
      `char_to_tile_index`, `window_curve_build` and `palette_fade_60`;
    - only V left for `func_80E4C5` and `text_put_tile_char`;
    - only C left for the five title rows.
  - The 22 negative rows all fail.

## The shop-buy movie's fuzz bases (fuzz-pin-1, 2026-11-22)

**`tools/bench/movies/explore_buy.txt`** (not wired in yet) runs the 60k movie up to step
16,720, the shop after the first race. From there the explorer, aimed at the buying code,
reaches `$80:9039`, `$80:99D1`, `$80:9957` and `$80:9ABB`. Its last state is a **paused
race** (lap 1 of 5). Every row it enters is fuzzed from that state, which no movie had given
before. It exposed fix-1 and fix-2, plus two rows whose fuzz relied on the old bases:
- **`shop_item_redraw`.** The paused race leaves D+$6C = `$27B4` and D+$74 = `$FFBC`.
  - `oam_high_table` sets its direct page to D+$6C + `$200`, which lands on the hardware
    registers.
  - Its clearing loop (from D+$74 up to D+$72) ran over the whole bank and the stack; the
    original crashed.
  - The fuzz now pins the shop's values, measured at every live entry of both shop movies:
    D+$6C/D+$6E are `$0900`/`$0D00` (swapped every frame), D+$74 = D+$6C - `$0900`, and
    D+$70/D+$72 run from `$00DC` to `$05A8`.
- **`text_line_push`.** The index D+$58 of `byte_buffer_push`'s `STA $7F7D00,X` came in at
  `$8300` or more. The long store ran into bank `$80`, low WRAM, and overwrote the return
  address on the stack. The fuzz pins it to `$0000-$020F`; live entries on the 60k and
  shop-buy movies show `$0003-$020B`.
- **Both fuzzers are used only by their own rows.** On both shop movies both rows pass
  500/500.
- **`object_sprites_stream`'s lost cases** (fuzz-pin-2). The original crashed inside the
  player's bar.
  - `sprite_bar_put`'s full path (`$81:8508`, a hole of its own row) returns with `RTL`
    without pulling its two pushes.
  - The paused race left D+$86 = `$00E4` and D+$8E = 0, so the stream started with `$11C`
    bytes free instead of `$200`. At every live entry the two are equal, 0 or `$220`, as
    `race_frame_draw` sets them. The fuzz now pins them so.
  - That only lowers the odds, so the chip's script is bounded as well. Every sprite stored
    follows a non-zero flag word, so fewer than 128 of them cannot fill the `$200` bytes.
    The script keeps at most 120 whenever the player's car is drawn its own way (`$0244`
    bit 0 clear, the only way its bar is drawn), and stays dense otherwise.
  - The kind-0 scene draws the player the rival's way, so the buffer still fills inside kind
    0's handler. Counted: its first call filled 2-3 times on race1, 8-15 on the 60k movie
    and 4-7 on the shop-buy movie.
  - All three movies: 0 lost, paths 603/603.
  - **The evidence is measured, not proven:** at least `$168` bytes free when the bar
    starts, at most `$20` used, over 5,763 + 1,591 live bars (the two movies share their
    first 16,720 frames).
  - **The C goes on where the original would crash.** This is noted at the bar's call in
    `player_car_sprites`, and it is the owner's to veto.
- **Still open on that movie:** the rows that fail only by the per-movie flag and path
  rules, and the waiting rows a slice leaves unsampled. A harness unit will pool the movies'
  evidence.

## Object kind 0 (roots-112, 2026-11-22)

**The owner's decision:** "Porque sem portar? Assim nunca vamos chegar no 100% né não?"
roots-110 had left kind 0 out because nothing runs it. That was wrong: it is code the game
can run, and code that can be reached is to be ported and verified.
- **`object_sprite_kind0` ($82:DCEC-$82:DD3A):** kind 0's handler (the table at
  `$82:80CE`).
  - It is `object_sprite_shadowed` up to D+$22, without its `$8CB5` bit test.
  - Then it calls `sprite_put_rows_c`. Unless that fills the buffer, it sets D+$22 = 0 and
    D+$24 = `$BC30`, and runs `sprite_put_rows_c` again.
- **`sprite_put_rows_c` ($82:CAE5-$82:CB38):** a third variant of the object's seven words.
  - Its rows are `$37EC`, `$38EC`, `$35EC` and `$37EC`.
  - Its part set is looked up at `$84:8080`, not `$84:8000`.
  - `sprite_put_rows` now takes the four rows and the table, so `_a`, `_b` and `_c` are one
    line each.
- **The map:** both ranges were unknown bytes. `map/static_code.txt` declares them now,
  since object_sprites_stream's fuzz executes them, and the listing labels them.
- **Kind 9 has no routine.** Its handler entry is `$0000`, so the original's
  `JSR ($80CE,X)` would run low WRAM (`$82:0000`) as code. No track places it. The C's
  switch says so, and the fuzz clears kind 9 entries.
- **Verified inside `object_sprites_stream`.** Neither routine has a live entry, so neither
  gets a contract or a row of its own.
  - The fuzz now places kind 0 among the rotated kinds, plus a seventh scene with kind 0
    objects on the rows in view.
  - Counted with a temporary print: the handler ran 128-255 times on race1 and 256-511 on
    the 60k movie. Its first call filled the buffer 8-15 and 32-63 times.
  - race1: live 58/58, fuzz 200/200, paths 603/603. 60k movie: live 1,831/1,831, fuzz
    500/500, paths 603/603.
- **Gate:** the full one (`sprite_put_rows` changed).

## sprite_group's stores across the bank's end (fix-2, 2026-11-22)

**The same class of defect as fix-1, found the same way.** Six shop icon rows failed fuzz
cases started from the shop-buy movie's last state, a paused race. The C wrote sprites at
`$7E:08BC` and `$7E:0ABC`, while the original wrote them at `$7F:08BC` and `$7F:0ABC`.
- **The cause.** `sprite_group` ($80:B49E) stores each sprite at `$0900`/`$0902`/`$0B00`/
  `$0B02` + the slot D+$74, as `STA $0900,X` with DB = the table's bank.
  - The 65816 adds X in 24 bits, so a slot from `$F500` up stores into the next bank. The C
    wrapped inside the bank.
  - In a paused race, D+$74 is not the shop's slot (it held `$FFBC`). The icon rows' fuzz
    does not set it, so that state reached them.
  - The table reads already crossed the bank in the C (24-bit `bus_rd16`).
- **The fuzz** (`fuzz_sprite_group`) now sets the slot from `$F500` up in one case in four.
- **Check:**
  - With the new fuzz, the old C fails: `sprite_group` 147/200.
  - The new C passes 500/500 on both shop movies, and so do the icon rows' fuzz cases on the
    shop-buy movie. `shop_item_icon_1` and `_3` still show FAIL there, but only by the flag
    rule (24 and 22 returns watched); see the shop-buy movie's to-do.
  - The full gate.

## The decompressor's reads across the bank's end (fix-1, 2026-11-22)

**A defect in verified C, found by a new movie's end state.** `title_scene_load` failed
every fuzz case started from the shop-buy movie's last state (see below; that movie is not
wired in yet): `$7F:2002` and `$7F:4004` held 0 on entry, and the original wrote `$5C` there
while the C did not.
- **The cause.** `decompress` reads the output it has written as words through DB = the
  output's bank and an index, `LDA $0000,X`. The 65816 adds the index in 24 bits.
  - A copy from before the output's start reads at `$FFFF`, and that word takes its high
    byte from `$80:0000` (low WRAM). The C wrapped inside the bank, to `$7F:0000`.
  - On the other movies' end states, those bytes already held what the C wrote. So the
    difference was never compared.
- **The short copies** (`$C0-$DF`) read their last word as `LDA $0002,X`. An odd length
  reads its last two as `$0002,X` and `$0003,X`, with X stepped by `INX` up to there. So a
  copy that passes `$FFFF` takes those words from bank `$80` as well.
  - The long copies' loop steps X to every word (`$0000,X` only).
  - Measured on a fuzz case: the original read `$80:0001-$80:0002` at `$82:FEDD` with
    X = `$FFFF`.
- **Writes stay in the bank** (`put16`). No output the game unpacks runs across `$FFFF`.
  Measured over the three movies' live entries: 18 distinct unpackings, starting at
  `$7F:0000`, `$4000` or `$8000`, the furthest ending at `$7F:A7E0`. A fuzz scene that did
  run across showed a difference after the wrap that is not understood. That scene was
  dropped, and the case is recorded as not modelled.
- **The fuzz** (`fuzz_decompress`, shared by `decompress_near` and
  `queue_unpacked_upload`) now alternates two scenes:
  - the output at `$7F:2100`, as before;
  - the output at `$7F:0000`-`$001F`, with back references reaching before it. The end of
    bank `$7F` and WRAM `$0000-$0001` are random.
- **Check:**
  - With the new fuzz, the old C fails: `decompress` 171/200, `decompress_near` 168/200.
  - The new C passes 500/500 on race1 and on the shop-buy movie, and
    `title_scene_load` passes there 500/500.
  - The full gate.

## The race frame's drawing (roots-111, 2026-11-22), and an endAt after a return (harness-7)

**`race_frame_draw` ($82:8923-$82:8944):** a root once `object_sprites_stream` was ported.
- **What it does.** It sets the sprite pointers D+$86/D+$8E to 0, or to `$220` with the
  second buffer (`$00B4`). Then it calls the road (`race_road_frame`), its rows
  (`road_rows_stream`), `shake_step`, `object_sprites_stream` and `func_82A9B7`, and jumps
  to `oam_high_load`.
  - All six were ported and verified before.
  - **A DSP-4 tape row,** whose callees' native commands are judged as whole commands
    (harness-5).
- **Its tail.** The row ends at its tail jump (`endAt $82:8944`), as `hud_markers_write`
  does, so `oam_high_load` keeps its own row and live samples.
  - The C is `race_frame_draw_body` plus `oam_high_load`.
  - The row's outputs are `func_82A9B7`'s: DB = `$83`, N set and Z clear (its `PLB`).
    `oam_high_load` reloads the flags before reading any. A, X and Y are not compared.
- **No live sample.** It takes most of a frame, so every sampled entry is interrupted: 216 on
  race1, 6,208 on the 60k movie. The flag watch still covers those returns (harness-6): no
  flag is live.
  - Its cases are the two replays and the fuzz. The replays are the first two sampled
    entries, rerun from their real entry state with the interrupts off.
  - Every fuzz case starts from the movie's last state, which is inside a race on both
    movies: race1, and lap 2 of 6 at the end of the 60k movie.
  - In the fuzz, the stand-in chip answers, with no script. The fuzz sets the buffer either
    way and applies the shake's and the view targets' fuzz now and then.
  - Many of the roots left show entries = interrupted in `roots.py`. If two replays become
    their only real-state evidence, a stronger replay rule is a harness unit of its own.
- **The race's stack** (`fuzzStack $1FF1`, as its road callees' rows). With the default
  fuzz stack every case diverged at `road_rows_project`'s `$0200` word; with `$1FF1` none do.
  `road_shift_frame`'s row records why: the default fuzz stack lies over the game's words
  `$1EEE-$1EFD`, which the road's handlers write.
- **Harness-7 (the advisor's call, open to the owner's veto).** `func_82A9B7`'s `RTS` lands
  on `$82:8944`, which is also the outer row's `endAt`.
  - The loop that ends samples stopped after the inner one, so the outer sample ran on into
    `oam_high_load`. The harness does not start a sample in a frame already sampled, so
    `oam_high_load` lost all its live samples (186 to 0 on race1).
  - Now, after ending a sample, the loop also ends the next one out when its `endAt` is this
    very instruction. Nothing else changes.
  - **Check:** the six callees on race1 with and without the new row are identical (live,
    interrupted, abandoned, watched returns, live flags, fuzz). `oam_high_load` is 186/186
    again.
- **Fixed on the way: `oam_high_load`'s C flag model** (`oamHighOut`). When room is left, C is
  the room's bit 1 (its second `LSR`), not the `ADC`'s carry. It went unseen because C is not
  live.
- **Gate:** the full one (`difftest.c` and `oamHighOut` changed).
- **Result:** race1 fuzz 200/200, replays 2/2, paths 12/12; the 60k movie is in the gate's
  table.

## The race's object and car sprites (roots-110, 2026-11-22)

**`object_sprites_stream` ($82:CBD4-$82:D267):** the race frame's sprite stream to the DSP-4.
It runs once per race frame (18,559 entries on the 60k movie).
- **A taped row with a native prologue (harness-5).** Commands `$03` (the mini-map's dots)
  and `$0B` (`minimap_car` ×2) run before the `$0009` word at `$82:CC8F`. From there on,
  everything is on the tape: the `$09` stream's head, then each drawn row.
- **The rows.** From D+$14 = 0 to D+$90, each row's object comes from `$7E:8CB3` (4 bytes
  per track row, from `$0226` × 4, wrapped at `$019A` × 4).
  - The armed jump (`$1EDE`, by `$1EE0`) and the segment's end (start + `$3F`, by its length
    at +`$10`; 0 with `$1E66` negative) move the index.
  - Then come the race order's cars on that row (`$1E6A` from D+$28, rows at `$FAB6`): the
    player (`player_car_sprites`: shadow car, marker, body, bar, item) or a rival
    (`rival_car_sprites`, the branch road by `$FADE`).
  - When the OAM buffer fills, the stream ends early.
- **The objects' handlers** (`$82:80CE` by kind): shadowed, group, flat, raised, hover (a new
  routine, `object_sprite_hover` $82:DAAF), the CLC stub and `track_event_none`.
  - The near/mid/far rows (`$7E:39EC` < `$82`, < `$442`, otherwise) choose `$26` and the
    carry the handler gets.
- **The end** (`$82:D1E7`): words 0 and `$8000`, the highlight `$1DFE-$1E04`, `$0186` =
  `$0184`, and `$1E52-$1E56` from `$023A`.
- **Kinds 0 and 9:** at first left out (my call). The owner overruled it for kind 0 (see
  roots-112), and kind 9 has no routine.
- A second entry at `$82:CCC7` (never run) and a dead `JMP $D1E7` at `$82:CED7` are holes.
- **Flags.** On return: N set and Z clear (the caller's `PLB`), and C clear (three ASLs of a
  byte). With harness-6, V is no longer live: 216 watched returns on race1, 6,208 on the
  60k movie.
- **The fuzz** flips the player's flags, scatters object kinds in turn (never 0 or 9), and
  sets the row places as `road_rows_project` writes them (from `$80` - (`$0225` >> 1 &
  `$7E`), by `$80`, up to `$E00`). It also rotates six scenes, so the selftest's 200 cases
  reach all 603 paths:
  - the second road;
  - the armed jump, by 1-16 rows landing before the lap's end (`$1EE0`; the game loads it
    only in a case never run and zeroes it otherwise). Measured: every scene-1 case jumps
    by a non-zero amount (34 on race1, 84 on the 60k movie). The second ASL's carry (bit 14
    set, an odd table index) is not reached;
  - a segment's end. It uses an entry of the race's segment table whose start (+`$3F`) and
    jump (its +`$10` word) both stay inside the lap. On race1's track, every entry's +`$10`
    word (`$F8EF` and the like) sends the index out of the object table, so it uses a 0
    jump through `$1E66` there. No track of the 60k movie has such an entry either.
    **The add and the `>> 2` are compared only with a 0 length:** 32 fuzz cases on race1,
    83 on the 60k movie, and its 3 live entries, all at 0 too;
  - a rival brought into view on the branch road;
  - two cars on one row;
  - none.

  On race1 the player leads and no rival is in view, so a car is moved between its
  neighbours in the race order onto the rows drawn (not row 0, whose place is often
  negative). The fuzz also clears kind 0/9 entries
  anywhere a row can reach.
  - **The player drawn the rival's way** (`$0244` bit 0) is flipped only when its part lists
    stay in the ROM. That means every turn band and every row, row 0's place being the one
    the routine writes (`$39EE - $80`, often negative).
  - **Every fuzz failure on the way was the fuzz leaving the game's states:** rows past the
    table, a segment whose own +`$10` word sends the index out of the object table
    (`$1E5E` = `$28` on race1's track: `$F8EF` × 4 = `$E3BC`), and that player-as-rival case
    (`$86:5286`). Each
    one reached kind 0 or a non-ROM read in the original; none was the C.
- **Result:**
  - race1: live 58/58, fuzz 200/200, paths 603/603.
  - 60k movie: live 1,831/1,831, fuzz 500/500, paths 603/603.
  - The shop movie doesn't reach it.

## Flags watched after interrupted returns (harness-6, 2026-11-22)

**The advisor's call, open to the owner's veto.**
- **The rule it relaxes.** A row's flags count as outputs only when its caller reads
  them. Until now the watch was armed only after a compared return, and with fewer than
  100 watched returns every flag counted as live.
- **The cost.** Many race rows are interrupted in most entries: their returns were never
  watched, and on race1 their flags counted as live. `$82:CBD4` showed it: 58 compared and
  158 interrupted returns, with V depending on whichever routine last did arithmetic.
- **The change.** An interrupted live return still isn't compared, but the watch now
  follows its caller. The interrupt landed inside the routine and does not change what
  the caller does with the flags after it, and the watch already pauses across handlers.
- **Not a declared override,** which the advisor ruled out for `hud_markers_write`.
  Nothing is declared: more returns are measured.
- **On race1** every per-frame row now has 216 watched returns: compared plus interrupted.
- **The check:** the full gate, and the `live-flags` column of `map/verified.txt` before and
  after. A set that shrinks is only the rule relaxing. A set that grows would be an
  unmodelled output to fix in this unit.

## Native commands beside a tape (harness-5, 2026-11-22)

**The advisor's call, open to the owner's veto.** `$82:CBD4` opens its `$09` stream after a
native prologue: commands `$03` (`$82:CBE4`/`$CC02`) and `$0B` (`minimap_car`, from
`$82:CC63`/`$CC89`). Both come before the `$0009` word at `$82:CC8F` (the race1 DSP-4
report).
- **The problem.** Until now a taped row took every data-port byte into its tape. The C of
  a native command computes its answer and leaves no port traffic, so the tape and the C
  fell out of step.
- **The rule.** A data-port byte stays off every tape when the instruction that makes it
  is in a native site, and is judged as an untaped row's byte would be (whole commands of
  `src/dsp4.c`).
  - **Native sites:** the extent and tail ranges of every untaped row, plus the ranges a
    taped row declares (`.dsp4Native`).
  - **From the whole registry,** not from the rows a run tests, so the rule holds whichever
    row is active. `$82:8923` will wrap CBD4, and CBD4's prologue must stay off its tape
    too.
  - **Keyed on the game's PC**, by a bitmap over the LoROM halves of banks `$00`-`$7F`.
- **Why the PC, and not another key** (the advisor's reasoning):
  - a hook in the natives would put a bench concern into portable C, and `src/dsp4.c` stays
    as it is;
  - the chip's state fails in fuzz, where the scripted chip cannot tell when a stream
    closes;
  - the sample's depth misses natives written inline, like CBD4's own `$03`.
- **In fuzz:**
  - a native site's bytes go to the real stand-in chip, not the script;
  - the stand-in is reset when a case starts scripting, so those commands start on an idle
    chip. The first try failed every fuzz case: the live state left the chip mid-command;
  - the script holds 4,096 words now, up from 256, since a `$09` stream reads up to 978
    bytes (race1).
- **The check.**
  - Every existing row is unchanged (the full gate). The taped rows were rerun on race1
    first, all as before.
  - In a scratch build, `road_rows_project`'s six products were declared a native site.
    The unchanged C, which writes them through the port, failed at tape byte 0, as it
    should.
  - With `dsp4_multiply` in their place, and the products dropped from its script, the
    row passed live 215/215 and fuzz 100/100.
  - The scratch build was reverted.
- **Next: `$82:CBD4` itself** (about 1,500 bytes, a handler table at `$82:80CE`, undecoded
  ranges), with its prologue declared native.

## Player 1's race frame (roots-109, 2026-11-22)

**`player1_frame` ($83:E241, RTL):** a root once `player_car_frame` was ported, with 18,498
entries on the 60k movie.
- It counts the frame at `$7E:4CDD` + `$1E9E` × 2, a histogram of the steps per frame.
- Then it runs `player_frame_setup`, and, with D = `$0200` (player 1's record),
  `player_car_frame` and `car_track_step`.
- It returns after `PLD` (N and Z clear) with DB = `$83`.
- **The fuzz** is player_car_frame's, applied with D = `$0200`.
- **Result:** live 140/140 (race1), fuzz 200/200, paths 14/14.

## The player's car frame (roots-108, 2026-11-22)

**`player_car_frame` ($83:EAC6-$83:F6E0):** about 3,100 bytes. It runs once per race frame
(18,560 entries on the 60k movie) with D = `$0200` (the player's record) and DB = `$30`, and
returns with DB = `$83`.
- **A plain row.** All of its DSP-4 use is command 0, which the C does with
  `dsp4_multiply`.
- **Its writes into a pending answer** (`$83:ED72`, `$F227`/`$F236`, `$F4EB`, `$F03F`) only
  stand in for an answer word the game doesn't read (roots-107). At `$F03F` the write comes
  before the read, so it takes the first word and the read gets the high one.
  - The tape could not confirm `$F03F`: over 20,000 frames that product is always 0, on a
    flat road.
  - The bench's chip agrees on every fuzz case, which sets the road's slope.
- **With `$1ECE` set** (the car follows another), only the view's distance D+$12/D+$14
  moves, and `follow_car_step` runs when it is negative.
- **Otherwise, in order:**
  - the race's warnings (`func_83FE9A`), the counters D+$31/D+$4E/D+$51/D+$7E, and the pads'
    bits in `$1E9C`;
  - the drag on a car or a record (`$83:EC33`, by D+$81). The record's case never ran; its
    unindexed `STZ $0216` is kept;
  - the speed D+$16/D+$18, stepped `$1E9E` times (the throttle `$1EA6`/`$1EA8`, the drag by
    its square);
  - the view's distance;
  - the engine's pitch D+$1F (`$86:DAD9`, `func_83EA9B`) and a 40-step division by `$1EA0`;
  - the drags in `$013A`/`$013C`;
  - the road's slope D+$9E, then the side speed D+$21 (eased back to 0 without changing sign,
    and the steering clamped to ±`$80`);
  - the side D+$04/D+$06, kept within `$280` of the middle or of D+$9E, with the limit at
    `$83:F3C5` chosen by D+$60 through the table at `$83:F3B9`;
  - the gear D+$3A, from the tables at `$86:DCF7`-`$DD1B`, or the side speed's band;
  - the items under the car (D+$53 1-4, through the table at `$83:F54B`, which starts inside
    its own `JMP`'s operand);
  - D+$48, D+$4C and D+$97, then D+$1E's byte.
- **One multiply by 0 is kept** (`$83:EF97`): it adds nothing to `$013A`/`$013C`.
- **Decoded as static code:** eleven never-run ranges. `$83:F403`-`$F422` is decoded by hand
  in the C, because the map has a data read at `$F403`. `$83:F6E1`, a never-run 32 × 16
  product, is the C's `car_step_product`.
- **The fuzz** starts from the race, moves a few inputs, and draws two to five rarer states
  per case: following, the drags (from a negative value), the speeds up to D+$10 ≥ `$80`,
  the throttle past its clamp, the slope of either sign, and the counters' ends.
  - **Thirteen scenarios set last, in turn.** The draws can undo one another, and so can
    the frame itself: it sets D+$1A and bit 4 of D+$44 before `$83:F32F` reads them. The
    turns are the four items, the side speed's band, the gear at its table's end, both
    sides of the `$83:F3C5` limit, both steering clamps at the top speed, both sides of the
    plain `$280` limit, and none.
  - Taking them in turn lets the selftest's 200 cases reach every path. The first gate
    failed there, at 1,189 of 1,259.
  - `$1E9E` stays at 1-4: at 0 the step loop would run 65,536 times.
  - The C passed every live and fuzz case from the first run. The rounds were all the
    fuzz's.
- **The holes are the two jump tables.**
- **Result:** live 4,000/4,000 (60k) and 152/152 (race1), fuzz 500/500 and 200/200, paths
  1,259/1,259.

## The HUD's markers (roots-107, 2026-11-21)

**`hud_markers_write` ($82:8961):** it runs once per race setup, 5 entries on the 60k
movie, and ends with a tail jump to `race_hud_digits`.
- **The markers:** VRAM word `$7C57` + i gets tile `$0157` where byte i of `$81:0398` is 0.
  For a set one, the original steps over the word by reading `$2139`.
- **The badge:** with `$03A7`'s low byte set, four words of `$83:A4FC` go to `$7F45`-`$7F48`.
- **The row ends at the tail jump** (`.endAt` = `.end` = `$82:89B2`), and race_hud_digits
  keeps its own row.
  - The first try ran into race_hud_digits. With 5 entries every flag counts as live, and
    that routine's C and V come from deep inside `race_message_update`.
  - The advisor ruled out a "flags dead" override: it would turn the measured rule into a
    declared one.
  - Ended at the jump, the outputs owed are the routine's own: X = 4, Y = `$0157`, C set
    (the `CPX #4`), V as on entry, and A/N/Z from the last load.
- **A hole: the step over a set marker** (`$82:8976`-`$897A`, the `$2139` read). It is
  unverified: no movie reaches it, and the harness has no VRAM read, so the fuzz keeps the
  marker bytes at 0. In C, the step is the skipped word, since every write names its
  address. The VRAM address the original leaves behind is not compared, because the
  harness doesn't count it as left set.
- **Result:** live 5/5 (60k), fuzz 500/500, paths 24/24.

**`$83:EAC6` is not blocked after all** (this corrects roots-102). Its writes at `$83:ED72`
and `$83:F4EB` land in the second word of a product's answer that the game didn't read.
On the 20,000-frame tape the next command `$0000` returns the exact product: for example
`$13C0` × `$200` = `$278000` right after the write at `$ED72`, and 3 × `$800` after
`$F4EB`. This is the idiom roots-94 found in road_rows_build's tilt. `$83:F227`/`$F236`
don't occur in those frames. So `$83:EAC6` can be a plain row with `dsp4_multiply`. The
stop rule that parked it ("every sequence opens with `$0000`") was the wrong test.

## A track's preview when shown (roots-106, 2026-11-21)

- **`track_preview_maybe` ($8A:8768):** when bit 1 of the track `$1536`'s entry in the
  8-byte table at `$8A:9351` is set, it runs `track_tiles_preview` with DB = `$83`
  (`PEA $8383`).
- **Outputs when it doesn't run:** A = 0, X = `$1536` × 8, and C = bit 13 of `$1536`, the
  carry left by the third `ASL`.
- **Outputs when it runs:** track_tiles_preview's, with N set and Z clear from the `PLB` of
  `$8A`.
- All four live entries are interrupted, so every flag counts as live. The first run
  failed on that C in 62 fuzz cases.
- **Result:** fuzz 500/500, replayed 2/2, paths 17/17.

## The title scene's load (roots-105, 2026-11-21)

**`title_scene_load` ($8A:891C):**
- **The tiles:** three sets unpacked to VRAM (`$7E:8256` at `$2000`, `$7E:B385` at
  `$2A00`, `$7E:9F07` at `$3000`), and `$0800` bytes of `$7E:A071` uploaded to `$3600`.
- **The objects** are placed from (0, `$200`) by `screen_objects_place`.
- **The starting state:** D+$36 = 3 and D+$4A/D+$38/D+$4C = 0. The car's sprite goes to
  (`$FF36`, `$9B`) with tile `$0100`, shown.
- **All four live entries are interrupted,** because the unpacking runs across frames. Two
  were replayed from their entry state, and the fuzz replays the live state 500 times.
  - With no return watched, every flag counts as live. So V is modelled: it comes from
    screen_objects_place's last ADC, since D+$3C = 0 always takes that path (or, with
    D+$5E set, from decompress_to_vram's clear V).
  - X and Y are not compared: they are screen_objects_place's, and mode7_view (next) does
    not read them.
- **Result:** fuzz 500/500, replayed 2/2, paths 43/43.

## The split screen's setup (roots-104, 2026-11-21)

**`split_screen_setup` ($80:B7E7-$80:B94E):** the two players' views set up for a split
screen. It has 14 entries on the 60k movie, from four callers.
- **The view records** at `$0196` and `$0296` are copied from `$84:A2EF` (`MVN`, `$2C`
  bytes). Their counters are cleared, and their scroll and buffer words set (player 2's at
  `$4000`/`$8000`, the buffers `$0800`-`$1500`).
- **`$0276` bit 5** is set unless `$03DC`'s low nibble is set without `$10` in its `$30`
  bits. That test (`$80:B828`) never ran: it is decoded as static code and is a hole, because
  no movie reaches it and a fuzz case cannot wait for a frame.
- **The sprite buffers** `$0900`-`$17FF` are cleared to `$E0E0`/0.
- **Then it waits one frame** (`wait_frames`) and sets up the interrupts:
  - the IRQ chain at `$1D8C`, holding the routine's entry DB and the caller's return
    address, which the original reads off its own stack (`PLA; PHA`);
  - `$0008` = `$B97B` and `$0005` = `$BC9B`;
  - the line interrupt at line `$6C`, dot `$10`, with `$4200` = `$B1`; then `CLI`.
- **No fuzz:** a fuzz case gets no frames, so all 500 were lost while it waited. The two
  movies reach every other path.
- **Outputs:** A = `$BC9B`, X = 0, Y = `$02C2` (the second `MVN`'s end); N set, Z clear; C
  and V from `wait_frames`' add.
- **Result:** live 14/14 (60k) and 2/2 (race1), paths 127/127.

## A screen's scene; `$8A:82C0` unparked (roots-103, 2026-11-21)

**`scene_step` ($8A:82C0):** one step a frame of a screen's scene (D = `$0100`). It
was parked because its state machine (`JMP ($0144)`: the state pointer at D+$44) leads
into the 627 unknown bytes. Those bytes are all code, now decoded as static code
(`$8A:8322`-`$8324`, `$833C`-`$84B8`, `$8524`-`$85AE`; `$84B9`-`$8523` came with roots-102).
- D+$48 counts the frames to wait, then the state runs. The states:
  - **`$82CA`** then **`$82EC`** (the only two that ran): the car slides (D+$3C +=
    D+$56, D+$36 -= D+$56; in `$82EC` D+$56 slows by one every other frame), placed by
    `func_8A8325`;
  - **`$833D`**: the sprite moves right to `$6C`;
  - **`$8361`**: 18 words of `$7E:8096` into `$0800`, 6 frames apart;
  - **`$8393`**: D+$5C counts up every 8 frames;
  - **`$83D6`**: D+$38 accelerates, and a 12-word row from `$7E:BDF2` (`$8A:83BA`) goes
    to `$0812` when D+$4C is in range;
  - **`$8412`**: D+$74/D+$76 step every 4 frames (`$0160`);
  - **`$843D`** and **`$856D`**: the 16 objects jitter (`$8A:8524`), the car speeds up, the
    rows move (`$8A:855A`) and `$7E:2C01`/`$2C04` scroll;
  - **`$8322`**: D+$58 counts up.
- **A quirk kept:** in `$843D`, when D+$56 is `$63`, the next `CMP #$64` sees the `LDA #3` of
  the branch, not D+$56.
- **One row** covers the dispatcher, the states and the helpers (`$8A:82C0`-`$85AE`).
  `func_8A8325`, inside the range, keeps its own row.
  - The holes are a lone RTS no state points at (`$833C`) and `objects_scatter_init` with
    its `rng_next` copy (screen_objects_place's).
  - There are no register outputs: the caller goes on to `func_8A8232`, which loads A.
- **The fuzz** picks any of the ten states. It puts each counter at the point where its
  state ends, because three ends needed a counter aligned on its step (`$0160` by 4, D+$4E
  by 8, D+$46 at `$33`).
- **Result:** live 284/284 (60k), fuzz 500/500, paths 303/303.

## A screen's objects placed; `$8A:898F` unparked (roots-102, 2026-11-21)

- **Why it was parked:** a never-run path of `$8A:898F` calls `$8A:84B9`, inside 627 unknown
  bytes.
- **What `$8A:84B9` turned out to be:** short and self-contained.
  - It loops over 16 objects, 2 bytes apart, filling `$0510`/`$054C`/`$0588` = `$FFFF`,
    `$0810` = `$0402`, `$084C`/`$0888` = `$0403`, and `$0710`/`$074C`/`$0788` = `$FE84`
    + 15 each.
  - It also writes three `rng_next` words at `$0610`/`$064C`/`$0688`, the last kept in
    D+$88.
  - Its random step is `$8A:8506`, a fourth copy of `rng_next` (on D+$10, `RNG_STATE`).
  - Both are now static code, with C as `objects_scatter_init` and the existing
    `rng_next`.
- **`screen_objects_place` ($8A:898F-$8A:8AE6)** has three ways out:
  - **D+$5E set:** an RTS (never run).
  - **D+$3C / 4 - $100 at `$120` or more:** D+$5E counts up, then `objects_scatter_init`
    (never run).
  - **Otherwise:** the four corners at `$0602`-`$0608`/`$0702`-`$0708`; six rows of 12 by
    `table_062a_fill`; the 72 kinds from `$7E:8152` to `$082A`, skipping 4 and adding `$3000`
    below the 23rd; then the right column (`$0612`-`$0628`, `$0712`-`$0728`).
- **One ADC chain.** The right column is a single chain of ADCs with one `CLC` at the start,
  so a carry passes from the `$072A` steps into the `$062A` steps. The C keeps the chain,
  and the adapter rebuilds its last carry and V.
- **One hole:** `$8A:89E1`-`$8A:8A42`, unknown bytes after the routine's `JMP` that it never
  reaches.
- **Result:** live 284/284 (60k), fuzz 500/500, paths 103/103.
- **Parked instead: `$83:EAC6`** (the player's car frame, about 3,100 bytes).
  - In three places (`$83:ED72`, `$83:F227`/`$F236`, `$83:F4EB`) it writes a value it has
    just computed to the chip's port. That happens after reading only the first word of a
    product, or right after a product's answer.
  - The chip may take that value as a command word. That is chip state which the native
    multiply in C does not model.
  - Its C waits until the tape or the native chip can show what the chip does with those
    words.

## The race's road for the frame (roots-101, 2026-11-21)

- **`race_road_frame` ($85:B3A9) and `race_road_frame_alt` ($85:B36F)** are two short RTL
  routines, both DSP-4 tape rows. Each runs `road_rows_project`, `road_rows_project2`,
  `scroll_tables_build` and `race_bands_merge`, and returns race_bands_merge's result.
  - **`$85:B3A9`** runs every race frame. After the scroll tables it copies `$00D0`/`$00D2`
    to `$1E46`/`$1E48`, `$015C`/`$0160` to `$016C`/`$0170`, and `$00E0` to `$01DC`.
  - **`$85:B36F`** has 5 entries on the 60k movie. It copies `$00D0` to `$1E46` and `$1E4A`
    before the scroll tables, then all four of `$015C`-`$0162` and `$00E0` twice.
- **Outputs** are race_bands_merge's. V comes from scroll_tables_build's SBC, because
  nothing after it touches V.
- **The fuzz** varies the buffer and scripts the chip empty, so every answer is 0. The two
  projections are checked with real answers in their own rows.
- **Result:** race1 live 135/135 and 1/1, fuzz 200/200 each, paths 15/15 and 19/19.

## The second view's road rows (roots-100, 2026-11-21)

**`road_rows_project2` ($85:B828-$85:BC1F), a DSP-4 tape row:** 1,016 bytes. The race calls
it right after `road_rows_project` (`$85:B36F`/`$85:B3A9`). It is a separate routine, not a
copy: only 95 bytes match `road_rows_project`'s at the same offsets, so its C is its own.
- **Its outputs:** the bands at `$7E:4D49`, buffer ends `$292B`/`$2A2B`, `$017C`, `$0160`,
  `$0168` and `$1E40`.
- **A first product of its own.** The row pair's `$7E:3E6E-$3E6C` by `$0224`'s row, plus
  `$0242` and `$3E6C`, gives `$1EF2`, which then plays `$0242`'s part. `$376C`/`$356C` start
  from the pair. Four more products give `$017C` and the offsets, from `$1EDA` instead of
  `$0208`.
- **Command `$07`** (or `$10` followed by D+$E4) gets the constants and `$1EF4`/`$1EF6`. It
  has none of the 8.8 offset words.
- **Each row:**
  - **It sends** its height, the first view's widths (`$7E:37EC`/`$35EC`) and its pair.
  - **It stores** the sums at `$376C`/`$356C`, then the chip's two answers at
    `$366C`/`$386C`, plus the height and the place.
  - **An object's 8 bytes** come from the chip or are copied. The source is bank `$90` or
    bank `$7E`, by the sign of the pointer at `$7E:3FEC`. For the copy that is the
    pointer's own sign; for the chip it is the sign of the pointer less 7.
  - **Its entries** go to `$0E56`/`$0E58`. There is no edge section.
- **Decoded as static code:** five ranges, the `$1EF2`-negative branches. The fuzz reaches
  them by choosing the first product's answer so that `$1EF2` comes out negative.
- **Result:** live 142/142 (race1), fuzz 200/200, paths 419/419.

## The road's rows projected by the DSP-4 (roots-99, 2026-11-21)

**`road_rows_project` ($85:B3D7-$85:B827), a DSP-4 tape row:** 1,105 bytes. It runs with
D = `$0100` and DB = `$30`, and returns with DB = `$83`. Its output is the row tables
`$7E:34EC`-`$39EC`, which `road_rows_stream` (roots-98) sends on.
- **Six products by the port** (command `$0000` inline) of the player's row pair
  (`$7E:3AEC`/`$3B6E`/`$3AEE` at (`$0226` × 2) & `$3E`) and its sides (`$0208`,
  `$020B`/`$020D`) by `$0225`'s place in its row. They give `$0174` (rounded on the low
  word) and the view's offsets D+$1E, D+$20, D+$26/D+$28 and D+$2A/D+$2C.
  - **Carries kept from the original:** one ADC takes the carry the second `ROL` of a
    product's ×4 leaves (bit 14 of its high word); another adds a 24-bit side, sign-extended
    from the byte at `$020D`.
- **Command `$01`** (or `$0F` followed by D+$E4, when D+$E4 is not negative) gets:
  - the screen constants, moved in by `$0242` when it is negative;
  - `$0200`, and `$0202` stepped one toward 0;
  - the buffer `$00B4` picks;
  - the offsets as 8.8 values, written as unaligned words (D+$1D, D+$25, D+$29).
- **Each row** (D+$14 from $80 - (`$0225` / 2 & $7E), stepping by $80, below `$0E00`):
  - **What it sends:** its ROM height (`$83:ACEC`) and pair.
  - **What it stores:** the chip's four answers at `$7E:34EC`-`$37EC`, plus its height and
    place at `$38EC`/`$39EC`.
  - **A nonzero fifth answer is an object.** Its band's byte at `$7E:4CFD` counts down. Its
    8 bytes go below D+$18, either from the chip (four products with `$7E:6C33`) or copied
    from bank `$90` with `MVP` (D+$E4 negative). Then come the chip's (index, word, word)
    entries at `$0796`/`$0798`.
  - **A row flagged at `$7E:3D6A`** (bit 15) sends `$8001` and its edges, from the pair at
    +2 or at -2 by bit 1.
- **Last, the bands' ends:** `$7E:4CFD`, `$0164`, `$015C` (D+$18 + 1) and `$1E3E`.
- **Decoded as static code:** six ranges, the `$0242`-negative branches (`$85:B54B`-`$B810`).
  They never ran, and the fuzz reaches them.
- **Outputs:** A = D+$1A; X = the band (`$23` or `$45`); Y = `$015C`. N is set and Z clear (the
  `PLB` of `$83`). C comes from the `CMP #4` (`$0242` negative) or the rows' `CMP #$0E00`;
  V is clear.
- **The fuzz** scripts the chip in read order: six products, then each row's four answers
  and object count.
  - Objects are mostly absent. When there is one, it has four answers (unless copied) and
    1-2 entries in low WRAM; its bank-`$90` pointer points into ROM.
  - It varies the buffer, the place, `$0242`, `$0202`, `$020D`, D+$E4's sign, and the rows'
    edge flags and bytes.
- **Result:** live 215/215 (race1), fuzz 200/200, paths 461/461.

## The road's rows streamed to the DSP-4 (roots-98, 2026-11-21)

**`road_rows_stream` ($81:8564-$81:89F4), a DSP-4 tape row:** 1,169 bytes. It runs with
D = `$0100` and DB = `$30` (the chip's bank, whose `$0000`-`$1FFF` is low WRAM). It returns
with DB = `$83`.
- **The row buffer.** It sits at `$7E:2E2C` when `$00B4` is set, else at `$7E:318C`
  (D+$16, `$1E4E`), and is cleared to `$00FF`/`$FF00` pairs.
- **Two products by the port.** Command `$0000` is written inline, not through
  `dsp4_multiply`, so the tape holds its bytes in place. The products are of the player's
  row pair's differences (`$7E:3C6E-$3C6C` and `$3F6E-$3F6C` at (`$0226` × 2) & `$3E`) by
  `$0225`'s row, and they give D+$1C/D+$1E.
- **Command `$08` gets:** its constants; the buffer's two ends, moved in by `$0242`/`$1EF2`
  when negative and kept from passing the buffer's start; the players' values; and the row
  pair.
  - When `$0242` is negative, its first answer fills -`$0242` entries (at most `$D8`) of
    the buffer, every 4 bytes. Otherwise the answer is read and dropped.
- **Each row** (D+$14 from 2 up to `$0190`) sends its heights and widths and a pair, then
  stores the chip's answers (`dsp4_answers_store`, roots-97). The pair is the row's own when
  bit 0 of `$7E:3D6E` is set, and in that case the row is sent a second time with the next
  pair.
  - That bit is read again after the answers are stored, as in the original: an answer can
    land anywhere in bank `$7E`.
- **Last, a `$8000` word.** Its answer, 0 or not, picks:
  - the HDMA bytes `$01B6`-`$01BB` and D+$C8;
  - whether the scan up to `$0194` takes the highest or the lowest of `$7E:37EC`. The
    lowest of `$7E:386C` is taken either way.
- **The split fill.** When that value lies below the other, two more products give the edge
  bytes, which fill the buffer's entries in between.
- **Then `$00EA`** steps by `$11`, going back from `$A422` to `$A411`.
- **Decoded as static code** (seven ranges, `$81:8679`-`$89E0`): the `$0242`/`$1EF2`
  negative branches, the first answer's fill, the nonzero last answer, and the split fill.
  None of them ran, and the fuzz reaches all of them.
- **Outputs:** A = `$00EA` and Y = 0. X is the fill's end, or `$0194` when there is no fill.
  N is set and Z clear (the `PLB` of `$83`); C and V come from `$00EA`'s step.
- **The fuzz** scripts the chip in the order the routine reads:
  - two products, the fill word, each row's runs (mostly empty, with pairs from `$6000` up,
    away from the tables), the last answer, and the two split products;
  - it varies the row count (1-16), the rows' bit 0, the buffer, the two negative values,
    and, in a third of the cases, tables that make the split happen.
- **Result:** live 39/39 (race1; most entries are interrupted), fuzz 200/200, paths
  474/474.

## The DSP-4's answers stored where it says (roots-97, 2026-11-21)

- **`dsp4_answers_store` ($81:89F5-$81:8A49), a DSP-4 tape row.** On the 60k movie it runs
  about 524,000 times, called twice by the road's row loop at `$81:8802`/`$81:8851`.
- **What it does:** two runs, each a count followed by that many (address, value) pairs.
  Each value goes to `$7E:address`; a pair at `$FFFF` writes its high byte to `$7F:0000`.
  - The original waits on the chip's status (`$C000`) before each count. The tape leaves
    status reads free, and the C has no wait.
  - It unrolls the last pair of each run (`DEY; BEQ` before the loop); the C loops.
- **Outputs:** X is the last pair's address and Y is 0.
  - A is the last pair's value. When the second run is empty, A is the chip's status word
    instead, which the tape does not hold, so A is not compared. The callers load A.
  - N and Z come from that value, or Z is set by the `LDY` of a 0 count. C and V are as on
    entry.
- **The fuzz** scripts two runs of 0-6 pairs. The addresses fall anywhere in bank `$7E`
  from `$2000` up (below lie the stack and the direct page), with `$FFFF` now and then.
- **Result:** live 4,000/4,000 (race1), fuzz 200/200, paths 41/41.

## The 20 cars' frame (roots-96, 2026-11-21)

**`race_cars_frame` ($81:E812-$81:F1FB):** 2,538 bytes, 18,560 entries on the 60k movie. It
runs with D = `$0100` and DB = `$7E` (its `PEA $837E; PLB`), and returns with DB = `$83`.
The original unrolls everything over the 20 cars; the C uses loops.
- **The race order `$1E6A`** holds 20 car offsets, last place first. It is linked both
  ways and round: +$1A is the car behind, +$18 the car ahead.
- **Each car** (+$35 apart from `$7E:FAB5`) keeps its place (+$0C/+$0E) and side (+$20),
  clears +$12, and runs its handler: +$10 indexes the table at `$81:F876`.
  - The game uses `car_road_step` and `car_sync_player`.
  - Entry 3 is an RTS.
  - Entry 2, `$81:FB20`, is the second player's `car_sync_player` (the record `+$B3`, the
    branch flag `$1E68`).
  - Entries 4-7 (`$81:FC72`/`$FCD1`/`$FD30`/`$FD8F`) are one handler for four records
    (`$0244` + 0, `$B3`, `$62`, `$115`): the record's motion only.
  - None of these ran, and their bytes were undecoded. They are static code now, in C as
    `car_sync_player2` and `car_sync_slot`. `car_sync_player` became the record-0 case of
    one helper; its row passes as before.
- **Then, until nothing wraps:**
  - **An insertion sort by place.** A car moves up past each car whose 32-bit place's high
    word is lower; each swap rebuilds the links (`func_81F1FC`).
    - When the high words are equal, `car_contacts_chain` runs.
    - A difference whose SBC leaves the carry set while N is set counts as in order. A
      lone RTS (`func_81F251`) sits between the test and that carry.
  - **The forward wrap.** The cars at or past the lap's end (`$019A`, on the place's middle
    word +$01) are put back by a lap, from the leader down. Each goes to the end of the
    order, and gets +$1E counted up, flag bit 0 and +$2B = `$019C`.
  - **Or the backward wrap**, when the leader has not wrapped. The cars behind the line (+$01
    negative) are put forward by a lap, from the last up. Each goes to the front, with +$1E
    counted down and bit 0 cleared.
- **Last, the finishers.** A car with bit 0 set, bit 1 clear and +$1E = `$1F06` gets bit 2,
  and its number goes to `$0446` + `$1F08`. Bits 0 and 7 are cleared, and the links rebuilt.
- **Original quirks kept in the C:**
  - The forward wrap skips the 15th to 17th cars: a `BRA` at `$81:EEB2` jumps over their
    code (`$81:EEB6`-`$EEE2`, unreachable).
  - Neither wrap counts the last car it moves, so the rotations are one short when every car
    checked wraps.
- **Decoded as static code:**
  - the forward wrap's cars 6-20 (`$81:EE28`-`$EF0F`);
  - the backward wrap (`$81:EFA6`-`$EFA8` and `$81:EFB4`-`$F17C`);
  - the five handlers (`$81:FB20`-`$FBB0` and `$81:FC72`-`$FDED`).
- **Two holes in the extent:**
  - the dead code above;
  - the backward wrap's 20th car (`$81:F0E9`-`$F0EF`). To reach it the leader would be
    behind the line too. But the forward check compares unsigned, so that leader wraps
    forward first, unless a lap is `$8000` or more.
- **Outputs:** A = `$1E6C`, X = `$1E6A`, Y = 0, N set and Z clear (the final `PLB` of `$83`),
  C set (the finish loop's `CPY`), and V from the last wrap check's SBC. The leader's place
  is not touched after that SBC, so the adapter recomputes it.
- **The fuzz** starts from the race and applies one of five cases:
  - the order shuffled a little;
  - two neighbours on one high word;
  - 1-20 cars from the leader past the lap's end;
  - 1-19 cars from the last behind the line;
  - finishers.

  Cases that don't move cars also give some cars the other handlers, with the records
  direct or by the DSP-4. Three limits keep the original under the harness's 4 million
  instructions per case. A run that hits them is lost, which counts as failing.
  - The moved cars get the empty handler, so the road step doesn't walk its segment table a
    long way.
  - The speed is small.
  - Not all 20 cars go behind the line: with the leader there too, it wraps forward about
    a hundred times.
- **Result:** live 2,782/2,782 (60k), fuzz 500/500, paths 924/924.

**Harness fix: `MAX_EXTENT`.** The differential test kept a row's instruction starts and
visits in arrays of `$600` bytes.
- `findStarts` stopped at `$600` without a word, so a longer row's paths were listed only
  that far. The visits were written past the end, into the flag counters: the first run of
  this row printed a watched count in the quadrillions.
- No row was longer before this one, so no earlier result changes.
- The arrays are `$1000` now, and a longer extent stops the bench with a message.

## The race frame's call to the road's rows (roots-95, 2026-11-21)

- **`road_rows_build_call` ($82:DFA9):** `JSR road_rows_build; RTS`. The race frame
  (`$82:8428`) calls it 18,564 times on the 60k movie.
- **The row** reuses `road_rows_build`'s fuzz. Its stack is one call above that row's
  (`.fuzzStack` `$1FF3`), so the game's words at `$1EE6`-`$1EFA` stay clear of it.
- **The flags are not judged.** As for `road_rows_build`, the code it returns into loads
  its own registers.

## DSP-4 command $09 measured (dsp4-09a, 2026-11-21)

**The first step of the owner's decision: an instrument and one measurement.**
- **`refbench --dsp4-tape FILE FROM`** writes every DSP-4 data-port byte from frame FROM on,
  in order. Each is a 5-byte record: the PC (3 bytes), the value, and read or write. This
  is outputs only: what the game sends and gets back, never the chip's memory.
- **`tools/dsp4_tape.py`** splits the capture into the game's `$09` streams, anchored on the
  game's PCs, not the shadow:
  - a stream runs from the `$0009` word at `$82:CC8F` to the `$0006` word at `$82:AE1C`;
  - a record is the words written since the chip's last answer, then the answer;
  - an object starts at a head's first word: `$82:D93A` (scenery), `$82:CEF4` (cars),
    `$82:D26B`, or a bar at `$81:84B3`. A car's head is split by a read at `$82:D14E`.
- **The key:** the stream's preamble, every word the object wrote before this record, and
  the record's own words. A key counts as repeated only when it occurs after different
  stream contents: an identical stream cannot give another answer.
- **Measured over the first 20,000 TAS frames.**
  - The capture: 213 MB of tape, 42.7 million bytes, none unpaired. It holds 6,062
    streams and 837,462 records, 414,126 of them hidden (`$0000` alone).
  - The records give 687,318 keys, but only 12,980 repeat after different stream contents.
  - **230 of those (1.77%) got more than one answer, and none within a single stream.** In
    229 of the 230, the object is shown in one stream and hidden in another.
  - In 202, every hidden answer came after more of its stream's sprites than every shown
    one.
- **A first draft of this section was wrong.** It said the answers depend on state carried
  from part to part. That came from a parser that knew only two heads: the cars' head
  (`$82:CEF4`-`$82:D160`) was read as part words. Keyed on the whole object, the conflicts
  inside a stream are gone.
- **A per-line ceiling: a strong pile-up, not a hard limit.**
  - Each shown sprite's size comes from the high OAM table the chip returns for the next
    `$0006`: 16 words, two bits a sprite, in the stream's order. 5,823 of the 6,062 streams
    line up with it.
  - Over those streams, taking 8 lines for a small sprite and 16 for a large one, these are
    the sprites accepted with k sprites already on one of their lines:

    | k | 30 | 31 | 32 | 33 | 34 | 35 | 36 |
    |---|---|---|---|---|---|---|---|
    | sprites | 3,034 | 2,566 | 1,834 | 369 | 250 | 208 | 41 |

  - The count falls off sharply past 32, and the most common peak load of a stream is 33.
  - But 918 sprites, in 365 streams, were accepted with 33 or more already on a line. So it
    is not a clean ceiling under this model.
  - The model's guesses: the heights, which depend on OBSEL (`$2101`, written at four
    places) and are not measured here; the line a sprite starts on; and the sprites'
    order in the high table.
- **What is measured:** within one stream, a record's answer is a function of its stream's
  preamble and its object's words. Across streams it is, in 98.23% of the keys that repeat
  after different contents.
- **What is hypothesis:** the residual is a per-line sprite budget near 32 (the SNES's own
  limit), and the heights or the counting rule above are what keeps it from being clean.
  The road commands before the stream are not ruled out.
- **Next, deferred:** the chosen-input experiment (replaying a stream, without the road
  commands before it, or one object alone) waits behind the `$06` data above.
  - First, settle the heights from the game's OBSEL during races and the parts' sizes.
  - Then retest the ceiling.
  - The chosen-input tool is built only if the tape cannot settle it.

  `tools/dsp4_tape.py` prints every number above. Routines come back first.

## The road's rows (roots-94, 2026-11-21)

**`road_rows_build` ($82:DFAD-$82:E502):** about 1,360 bytes, 18,564 entries on the 60k
movie. It runs with D = `$0100`, DB = `$7E` inside and `$83` after: the second `PEA` is
pulled at `$82:E429`, the first by the last `PLB`.
- **The road's sideways place `$01CE`** comes from the rows crossed (`$7E:3AEE`), by DSP-4
  products. The first is the product's bits 8-23. Then an `ADC` chain starts with the
  carry of `SEC; SBC $023C`, which is set only when `$023C` is 0. Last comes a product's
  high word.
- **Each new row** (row `D+$16`, `D+$14` rows; the armed jump `$1EDE` moves the rows on by
  `$1EE0`):
  - **Its row-table entry:** `D+$98` + (row / 64) × 10 (DSP-4).
  - **Its heights and widths** (`$7E:3AEC`-`$3D2C`) from banks `$84`/`$85` and the header at
    `$7E:5391`. One `ADC` takes the carry of the `ASL` of `D+$16`, without a `CLC`.
  - **Its segment's curve and edges** (`$3DEC`-`$402C`):
    - the shape table at `$82:E261` gives a curve shape or a flat one;
    - the segment cursor `D+$A2` moves on past each segment's end, adding the segment's
      length via `$1EE6` while bit 15 of `$1E66` is clear;
    - before a segment, the edges come from the track events' words
      (`$1F0A`/`$1F0E`/`$1F10`);
    - flags `$8001`/`$8002`/`$0008` go to `$3D6C`.
  - **Then its object's track event** (the table at `$82:819E`).
- **After the rows:**
  - the road objects whose place has come up (the stream at `$7E:5B91`, slots
    `$F433`-`$F933`, `func_85CDAF`);
  - the lap's wrap;
  - the road kind `$0260`/`$0142` at the player;
  - the road's tilt `$0237`, from `func_82EA49` one row ahead and three behind (DSP-4; the
    original skips the product's low word with a write).
- **Both roads' track events.**
  - The first road's two never-run handlers, `$82:EA93` (pattern `$20C7` at the object's
    place) and `$82:EAFE` (positions only), are already decoded.
  - The second road's events (`$82:EB45`-`$82:EC08`) were undecoded. They mirror the
    first set on `$1F0C`, `$1F12`/`$1F14` and the rows `$41EC`/`$44EC`/`$42EC`, and are now
    static code.
  - In the C one helper writes either road. The handlers have no rows: this row's fuzz
    runs them.
- **Two holes in the extent.** No movie or fuzz case reaches them:
  - a row list's second pair (`$82:E185`, decoded);
  - the shape table (`$82:E261`) and its flat shape (`$82:E265`, decoded from inside the
    table).

  Both depend only on ROM data that no track scanned here uses.
- **The fuzz** needed many rounds. It works in three scenarios:
  - **(0) one of the segment-like tables:** only where its next 8 segments' lists stay in
    ROM and its rows stay inside this track. Bit 15 of `$1E66` is clear only when the
    simulated chain of length jumps ends inside those checks. The rows are put before,
    inside, past, or exactly `$3E` into a segment;
  - **(1) the race's object stream rewound** to an entry, with the place at it or at the
    next one;
  - **(2) the lap's ends:** the rows reaching `$019A`, the place passing it, or the lap's
    start.

  It also writes the byte `$0225` and not the word, since the word reaches `$0226`. The
  next rows' objects get every kind the event table handles, including the second road's.
  The row fuzzes on the game's stack (`.fuzzStack`, as `road_shift_frame`).
- **Result:** live 4,000/4,000 (60k) and 216/216 (race1), fuzz 500/500 and 200/200, paths
  549/549. The live samples passed from the first run; the rounds were all the fuzz's.

## The race's state reset (roots-93, 2026-11-21)

**`race_state_reset` ($82:A28B):** about 1,000 bytes, 5 entries, all of them interrupted.
- **The players' and the race's words** are cleared or set to their start, about 150
  stores. The second player copies the first's car (`$0393`/`$0395`) unless the variant's
  bits 4-5 are both set, and it moves on one unless that car is 5.
- **Tables and colours:**
  - 8 tables of 4 words from `$7E:272C` are set to `$18A6`;
  - the players' colour offsets (`$4E55`-`$4E5F`) come from their palettes' `$6A33`/`$6B33`:
    the 5-bit channels less 6, 7 and 8, with bits `$20`/`$40`/`$80` for the first player.
- **The cars:**
  - the cars' order `$1E6A` = `$0446`'s byte × `$35` (DSP-4);
  - `race_grid_setup`;
  - `$11` bytes from `$83:A411` to `$7E:4CCC`;
  - each player's `$1E`-byte car record from `$85:A118` + `$1E` × the car (DSP-4; the
    original skips the product's high word with a write) to `$7E:2140`/`$215E`.
- **Then:** `track_palettes_load`, 40 entries from `$97:A2B8` to four words each at
  `$7E:2000`, and `$0446`-`$0458` = `$FFFF`.
- **The C:** the runs of constant stores were transcribed from the listing by a script,
  which flags any instruction it does not handle (none were left). The branches, the
  8-bit stores, the copies and the loops were written by hand.
- **Registers.** A, X, Y, DB and every flag are compared (A = `$FFFF`, X = 0, Y = `$140`).
- **The fuzz:** `race_grid_setup`'s and `track_palettes_load`'s inputs, the variant's bits,
  the first player's car (5 now and then), a shuffled car order, the players' cars and
  the palettes' colours.
- **Result:** replays 2/2 (60k) and 1/1 (race1), fuzz 500/500 and 200/200, paths 360/360,
  first try.

## The race's starting grid (roots-92, 2026-11-21)

**`race_grid_setup` ($85:9ADB, `RTL`, DB = `$85` for its tables):** 5 entries, 3 of them
interrupted. The 20 cars' records at `$7E:FAB5` (`$35` bytes each) are cleared and
numbered.
- **Each car's lane `D+$1A`** moves on by `$200` a car, wraps at `$E00`, and skips the
  players' lanes ((`$0391`/`$03C0` & `$FF00`) × 2).
- **Its forward offset:**
  - `+$22` = its `$85:9DDF` entry, less a random 0-`$1F`, less `D+$1E`. `D+$1E` is the
    variant's `$85:9FA7` byte plus the track's entry in `$85:9E2F`, `$85:9E8D`, or
    `$85:9EEB` for variant 0 (never run, now static code);
  - `+$24` = its `$85:9E07` entry less the track's `$85:9F49` one.
- **Its drift `+$26`:** random, redrawn while it lies in -`$10`..`$0F`, sign-extended from 9
  bits.
- **Then:**
  - the cars in the order at `$1E6A` are placed from `$019A` - `$14` on, one apart, their
    sides alternating `$40`/-`$41`, and staggered by `$600`/`$300` a place;
  - the players' places (`$0225`/`$02D8`) = their cars' place less `$4040`;
  - the two section seeks and the two segment steps run;
  - the players' sides come from their cars.
- **Flags.** With 5 entries every flag counts. N and Z come from the last load. C and V
  come from `player2_segment_step_long`'s last `SBC`, with the shared helper.
- **The fuzz** keeps the race's track and car order, since the segment steps walk its
  table. It varies the variant, the players' lanes and the random state.
- **Result:** live 2/2 plus replays 2/2 (60k) and 1/1 (race1), fuzz 500/500 and 200/200,
  paths 226/226, first try.

## A bar of sprite parts (roots-91, 2026-11-21)

**`sprite_bar_put` ($81:8487, `RTL`):** a bar A pixels long centred on X (17,654 entries on
the 60k movie).
- **Length 0:** two 0 words, nothing else.
- **Otherwise:**
  - `D+$32` = Y, `D+$14` = A | 1, and `D+$16` = X - `D+$14` / 2, with the old `D+$14`/`D+$16`
    saved on the stack;
  - a 0 word;
  - then `sprite_part_put` of the first part (`$20F6`), and of the middle ones (`$20F7`)
    every 8 pixels while `D+$14`, 8 less each time, stays positive;
  - then the mirrored end (`$60F6`), moved back by the rest;
  - finally `D+$14`/`D+$16` put back, a 0 word, and `D+$84` = the OAM place.
- **A flaw of the original's, never run.** The buffer filling up (`$81:8508`) drains the
  chip's answers, writes a 0 word, sets `D+$84` and returns with `RTL`, without pulling
  the two words it pushed. On the console that would return through them.
  - It is static code but a hole in the row's extent.
  - The fuzz never fills the buffer: no case could compare that path, since the bench's
    sentinel return would be skipped.
  - The C keeps the path's memory effects, `D+$14`/`D+$16` not put back, and documents it.
- **Registers.** A, X, Y and every flag are compared:
  - length 0: A = 0, Z set;
  - otherwise: A = the old `D+$14` (from its `PLA`), X = `$60F6`, Y = the OAM place, C
    clear, and V of the last part's `ADC`.
- **Result:** live 4,000/4,000 (60k) and 192/192 (race1), fuzz 500/500 and 200/200, paths
  65/65, first try.

## One sprite part (roots-90, 2026-11-21)

**`sprite_part_put` ($81:8527):** one sprite part streamed (141,203 entries on the 60k movie).
- It writes X, `D+$32` - 8 and `D+$16`. Then, while the chip's flag is not 0, it reads OAM
  word pairs to `$1574` + Y.
- Y comes and goes in the register: the callers set it once from `D+$84` and chain the
  calls. So the row compares Y.
- **The buffer filling up.** Y reaching `D+$1A` ends the routine at once (`SEC; RTS` at
  `$81:8562`, never run, now static code). Unlike `$82:DDBF`, it drains nothing.
- **Flags:** C = the buffer filled; N clear and Z set; V of the `ADC` of `D+$32` and -8.
- **Result:** live 4,000/4,000 (60k) and 1,389/1,389 (race1), fuzz 500/500 and 200/200,
  paths 29/29, first try.

## A track object and its second sprite (roots-89, 2026-11-21)

**`object_sprite_shadowed` ($82:DA48):** 75,701 entries on the 60k movie.
- **Setup:** `D+$24`, `D+$20` and `D+$26` as for the other object sprites. `D+$22` = the
  height at `$84:A200` + r, less `$40` (r = (`$0024` - the object's word at +3) & `$3E`).
- **Then** `sprite_put_rows_a`. Unless that fills the buffer, a second sprite: `D+$22` = 0,
  `D+$24` = `$BC30`, then `_b` when bit 4 of the word at `$7E:8CB5` + r is set, else `_a`.
- **A quirk of the original, kept.** That test reads `$8CB5` indexed by r (X was reloaded
  with the height row), not by the object's offset.
- **Flags.** race1 reaches it only 22 times, so all flags count there. The adapter models
  them:
  - N clear and Z set: every exit of the stream's tail follows a 0 word read or written;
  - V clear: its `ADC`s (a graphics place plus an offset, a list's +6) stay small in the
    game;
  - one fuzz case on the 60k movie had V set, where V is not live (4,000 returns, only C).
- **The fuzz** writes the parts lists for both graphics words (`$BC30` first), and
  randomizes the words the second sprite's test reads.
- **Result:** live 4,000/4,000 (60k) and 22/22 (race1), fuzz 500/500 and 200/200, paths
  43/43.

## A track object as a group of four sprites (roots-88, 2026-11-21)

**`object_sprite_group` ($82:DB87):** 146,642 entries on the 60k movie.
- **Setup:** `D+$24` and `D+$26`'s bits as in `object_sprite_flat`, `D+$22` = 0, and
  `D+$20` = the object's place p.
- **Then by bits 4-5 of the object's +2 byte** (a jump table at `$82:DBAA`, a hole in the
  row's extent):
  - kinds 0 and 3: `sprite_put_rows_a` at p, -p, -p - `$80` and p + `$80` (four sprites
    about the road's middle);
  - kind 1: `_a` and `_b` at p, then at -p;
  - kind 2: `_b` at the four places.
- It stops at the first sprite that fills the buffer and returns its result (the carry).
- **Never run:** kind 2's sign extension of a negative place (`$82:DC38`), now static code.
- **Result:** live 4,000/4,000 (60k) and 1,208/1,208 (race1), fuzz 500/500 and 200/200,
  paths 102/102.

## Two track objects' sprites (roots-87, 2026-11-21)

**`object_sprite_raised` ($82:DAFD) and `object_sprite_flat` ($82:DB4D):** handlers from the
table of `$82:DB87`. Each sets up one track object's sprite, then jumps to
`sprite_put_rows_a` or `_b`.
- **The common setup:**
  - `D+$24` = the graphics word at `$86:AF06` + X;
  - `D+$20` = the object's place (its byte × 16, signed 12-bit);
  - `D+$26` gains its bits 8-10, moved up one.
- **The raised one:** `D+$22` = a height from `$84:A200` + ((`$0024` - its word at +3) &
  `$3E`), less its +2 byte's high nibble × 8 and less `$20`.
- **The flat one:** `D+$22` = 0, then `_b` when its +2 byte has bit 4, else `_a`.
- **The rows** follow the jump through `.alsoIn` (`$82:D934`-`$82:D9DD`, `$82:DDBF`-`$82:DEA8`)
  and carry a tape.
- **The fuzz** picks an X whose graphics word leads into ROM (from `$8000`). It gives that
  word to roots-84's parts-list fuzz, which writes the lists where the routine will look.
- **Result** (60k; race1 does not reach them): live 1,342/1,342 and 2,166/2,166, fuzz
  500/500 each, paths 35/35 and 24/24; only C live.

## A car's sprite head (roots-86, 2026-11-21)

**`car_sprite_head` ($82:D267):** 43,690 entries on the 60k movie. It writes nine words
through the port:
- the row `$7E:386A` + X;
- `$9000`;
- the car's word `$7E:FAB4` + Y;
- twice its height: `$0243`'s byte, signed, + `$33` + `D+$34`;
- 0;
- `D+$32`;
- its frame (the word at `$83:ACEC` + (`$FAB5` / 2 & `$7E`) + the row `$39EC` at `D+$14`);
- 0.

Then it reads the chip's answer into `D+$2A`.
- **Never run:** the negative height (`ORA #$FF00` at `$82:D289`), now static code.
- **Registers.** A = the answer, and X = Y (the caller reads `$FABF`,X next), with C and V
  of the frame's `ADC`.
- **The fuzz** keeps the race's `D+$14` and its row, so the frame table in bank `$83`
  stays in ROM. The answer is scripted.
- **Result:** live 4,000/4,000 (60k) and 364/364 (race1), fuzz 500/500 and 200/200, paths
  42/42, first try.

## A sprite's parts from a ROM list (roots-85, 2026-11-21)

- **`sprite_parts_stream_rom` ($82:DEA8)** is `sprite_parts_stream` for a parts list in
  ROM, at `$86:0000` + X, with X given by the caller (68,619 entries on the 60k movie).
  - Its two never-run stretches match `$82:DE63` byte for byte: the first list's overflow
    (`$82:DF4C`) and the drain's pairs (`$82:DF7A`). Both are now static code.
  - The C shares a helper, `sprite_parts_list_at`, that takes the list's bus base. The
    first copy, `sprite_parts_list`, keeps its own body; folding them together is not an
    additive change.
- **The fuzz** searches for an X whose two lists both end within 8 parts, as the game's
  lists do. A random X runs into lists hundreds of parts long and overflows the tape. The
  OAM limit and the script are as in roots-84.
- **Result:** live 4,000/4,000 (60k) and 1,386/1,386 (race1), fuzz 500/500 and 200/200,
  paths 109/109; only C live.

## The objects' sprites streamed to the DSP-4 (roots-84, 2026-11-21)

**`sprite_put_rows_a` ($82:D934) and `sprite_put_rows_b` ($82:D988)**, the first real
users of the tape: one race object's sprite sent to an open `$09` sprite stream.
- **Seven words through the port:**
  - the object's rows `$386C`, `$38EC`, then `$35EC`/`$37EC` (a) or `$366C`/`$386C` (b),
    at `D+$14`;
  - `D+$20`, `D+$22`;
  - `D+$26 | D+$3A` (a) or `| $1000` (b).
- **Then its parts, through the shared tail `sprite_parts_stream` ($82:DDBF).**
  - The parts list is at `$7F:0000` + the object's graphics place: its `$7E:6591` slot,
    from `$86:0000 + D+$24`, plus `$86:0002` + its part set's offset.
  - There are two lists, each of 6-byte parts ended by a 0 word. For each part the chip
    answers OAM word pairs, read to `$1574` + Y (Y from `D+$84`, 4 bytes a pair), while
    its flag is not 0.
  - **The buffer filling up.** When Y reaches `D+$1A`, the rest of the answers are
    drained, the lists still open are ended with a 0 word each, and the routine returns
    with carry set.
  - The first list's overflow (`$82:DE63`, never run) was undecoded and is now static
    code: two 0 words, where the second list's overflow writes one.
- **Outputs.** C is the overflow, which the callers' `BCS` reads. The other flags are
  judged by liveness, and only C came out live.
- **The fuzz.**
  - It keeps the race's object (`D+$14`, `D+$24`, whose tables lead into ROM). It computes
    the place the routine will read and writes both lists there (up to three parts each;
    the first one empty half the time).
  - The OAM limit sometimes falls exactly on a pair. The chip's answers are scripted: 40
    words, a third of them 0.
  - A temporary count over 200 cases: the first list's overflow 87 times, the second's 6.
- **Result:** 60k: a live 4,000/4,000 (748,825 entries), b 706/706, fuzz 500/500 each.
  race1: a 4,000/4,000, fuzz 200/200.

## A DSP-4 tape for the streamed commands (harness-4, 2026-11-21; decided by the advisor, open to the owner's veto)

- **Why.** The remaining DSP-4 roots stream commands `src/dsp4.c` does not have: the
  sprite commands `$09` and `$06`, and the road commands `$01`/`$07`/`$08`. `$06` is
  stateful: it returns the high table the earlier sprite commands built. So these cannot
  be checked as pure functions, and their routines could not be verified at all.
- **The port.** `src/platform.h` gains `dsp4_port_write16` and `dsp4_port_read16`: one word
  written to or read from the data port, low byte first, in the original's order. A port
  answers from its own chip, and waiting on the status register is the platform's job.
- **The tape.** A row may set `.dsp4Tape`.
  - The original side then records every data-port byte of the sample in order (read or
    write, value), instead of checking it as whole known commands. Status reads stay free.
  - The C's port calls are checked against the tape in place. A write must equal the
    tape's write at that position. A read is served only where the tape has a read, and
    returns the original's byte. The whole tape must be consumed.
  - Over-reads, under-reads, and port use in a row without a tape all fail the sample,
    with a message that names the tape byte.
  - It does not rely on the shadow's command boundaries: one ordered stream, compared
    byte by byte. The tapes live in a buffer per nesting depth (16 KB each), not in the
    sample, which every entry clears.
- **The fuzz scripts the chip.** In a taped row's fuzz case the bench chip is scripted
  (`dsp4_bridge_script`):
  - its status always reads ready;
  - data reads come from a list of words the row's fuzz writes, and are 0 past its end;
  - writes are recorded and not fed to the chip;
  - a chip reset (each case) clears the script.

  Each row keeps the answers inside its routine's domain, and the 0 past the end closes
  read loops.
- **What it checks.** A taped row's live samples check the routine against the bench
  chip's answers. Its fuzz checks it against scripted answers. Neither checks the chip:
  that stays refbench's job, command by command.
- **Not in this unit:** native commands (`dsp4_multiply` and the others) inside a taped
  sample. Their bytes would be left on the tape, and the whole-tape rule fails such a
  sample. They come with the first row that needs them (`$82:CBD4`).
- **The pilot, `oam_high_load` ($82:AE15).** It is the `JMP`'d tail of `$82:8923`.
  - It sends `$0006` through the port and stores the chip's 16 words (the sprites' high
    table) at `$1776` + `D+$8E`.
  - Then it moves the sprites left unused off screen (`$E0E0` every 4 bytes from `$1576` +
    `D+$86`). A count below 4 bytes is taken as 65536, as in the original's `DEX` loop.
  - race1: live 186/186 against the bench chip, fuzz 200/200 against scripted answers,
    paths 59/59. A, X, Y, DB and every flag are compared.
- **Negative rows.**
  - `oam_high_load_wrong_command` (command `$0007`) fails on tape byte 0 (`$06` against
    `$07`): live 0/186, fuzz 0/50. It is the registry's negative row.
  - A second one, one word too few read, failed when run on its own: "the C used 32 of the
    DSP-4 tape's 34 bytes". It is not kept, because two rows cannot share an entry in one
    run, and `verify_all.sh` runs every negative row together.
- **Next:** `$82:D934`, `$82:D988` and `$82:DDBF` as rows-only incremental units. Keep
  `D+$84` and `D+$1A` as the race has them, and cap the script's non-zero answers by the
  room left. Then natives on the tape for `$82:CBD4`. The owner's question stands: native
  `$09`/road commands in scope?

## The saved data checked at boot (roots-83, 2026-11-21)

**`save_data_check` ($80:8119):** 1 entry, at boot. `$03E2` = 0, then three cases:
- **The stored copy is valid.** The `$78` bytes at `$7E:FA3D`, summed as bytes into a
  word and `^ $7645`, equal `$7E:FA3B`. The copy is restored to `$0368` (an `MVN` of `$7A`
  bytes) and `$045A` loses its low byte. Never run.
- **The data itself is valid** (the same check over `$0368` against `$0366`). `$045A`
  loses its low byte. Never run.
- **Neither:** the defaults (`$7A` bytes from `$83:85A9`) go to `$0366`, `save_data_seal`
  runs and `$045A`'s low byte is set.
- **Return value.** The C returns whether the defaults were loaded, which is the
  original's carry. The running sum stays in `$0114`, as the original leaves it.
- **The two valid-data branches** (`$80:813F`, `$80:8177`) were undecoded and are now
  static code. On the bench the saved data is never valid, so they run only in the fuzz.
  The fuzz writes a correct checksum a third of the time for each.
- **Registers.** With 1 entry, every flag and register is compared:
  - A = `$00FF`, N clear;
  - Z from the `TSB`/`TRB` of `$045A`;
  - X, Y and V per case: past the `MVN`, the loop's end, or `save_data_seal`'s exit, with
    V of the last sum's `ADC`.
- **Result:** live 1/1, fuzz 500/500, paths 52/52.

## The results text (roots-82, 2026-11-21)

**`results_text` ($8A:9908, D = `$0100`, DB = `$8A`, `RTL`):** the results screen's text, 4
entries on the 60k movie, all of them interrupted.
- **Normal path:** `results_text_build`, `func_8A9BE2`, `D+$5E` = 0 and `variant_66`. Then,
  for each slot `func_8A9BB5` takes, one line:
  - the rank;
  - a space;
  - the name (`text_line_push`);
  - padding to `$16`;
  - three spaces;
  - its value (`$041C` + slot);
  - the line's end.

  The routine ends with the string at `$8A:9DBD` (up to `$FF`).
- **Never run, now static code** (both with bit 15 of `$1B37`):
  - a one-slot variant (`$8A:9946`) with a header and a footer from `$8A:9DBF`/`$9DD5`;
  - **`text_line_centered` ($8A:99FC):** a text map line centred in `$1C` columns.
- **Flags.** Only 4 returns are watched, so every flag counts as live. The adapter models
  them: N, Z and C from the final `CMP #$FF` (equal); V = bit 14 of `$1B37`, from its `BIT`,
  which no later callee touches.
- **The fuzz needed the game's buffer state.** Three times, the original walked past the
  instruction budget:
  - Slot values up to 1000. The slot taker walks the kinds down one by one, so they are
    now below 16.
  - Text map lines without a zero (they are now short names).
  - The byte buffer's count. `D+$60` is a word: its `INC` is 16-bit, so a random high
    byte made `byte_buffer_pad` push about 65,536 spaces. The push is a long store to
    `$7F:7D00` + `D+$58`, and past `$82FF` it crosses into bank `$80` (low WRAM, the direct
    page and the stack). So the fuzz zeroes the count word and keeps the index small.

  The C of `byte_buffer_push` masks to the WRAM size there instead of crossing banks. That
  differs from the hardware only outside the game's domain.
- **Result:** replays 2/2, fuzz 500/500, paths 65/65, flags 0 mismatches.

## The track's setup load (roots-81, 2026-11-20)

**`track_setup_load` ($82:A7F9, D = `$0100`):** 5 entries on the 60k movie, all of them
interrupted. It was parked as "a recipe" until the decompressor and the segment work were
in C. For track `$047B`, through tables in bank `$84`, it:
- unpacks the header to `$7E:5391` (from bank `$9B`) and the object table to `$7E:8CB3`;
- copies a block from bank `$8B` to `$7E:5B91` (an `MVN`);
- unpacks the scenery to `$7F:0000`, by the variant or `$01E4`:
  - `$03DC & 3 = 2`: `$89:8512` (never run);
  - `$01E4` negative: `$96:8020`; zero: `$96:8097`; positive: `$97:A24D`;
- sets `$003A` and `$0038` from the bank-`$85` tables (`$85:A3CC` when `$039C`/`$03CB` is
  set, `$85:A35E` otherwise; `$03CB`'s case never ran). A negative entry is replaced by
  the word at `$90:8000` + the header's `$5395` - `$8007`;
- unpacks each object kind's graphics once into `$7F`, from `D+$14` = `$D2` on (`$0612` for
  variant 2, never run). Each kind's slot in the table at `$7E:6591`, set to `$FFFF` first,
  takes its place.
- **Never run, now static code:** three stretches, `$82:A84B`, `$82:A8C4` and `$82:A8F7`.
- **Tracks:** 0-47 have consistent table entries. From 51 on, the block's source would
  wrap inside bank `$8B`. The fuzz keeps the race's own track, because `$019A` (the object
  table's length) must agree with it, and varies the variant, `$01E4` and the two
  selectors.
- **Flags.** Only 5 returns are watched, so every flag counts as live. The adapter models
  them from the RAM after the call:
  - N set and Z clear (the final `PLB` of `$83`);
  - C from the last object kind walked: its `ASL` clears it, a free slot's `CMP #$FFFF`
    sets it, and that slot's `ADC` (the graphics' length plus `D+$14`) then decides it. With
    no kind at all, it is the variant's `CMP #2`;
  - V from the last such `ADC`; else the `SBC #$8007` of a looked-up entry; else 0.
- **Result:** replays 2/2 (60k) and 1/1 (race1), fuzz 500/500 and 200/200, paths 146/146,
  flags 0 mismatches.

## The segment steps' long wrappers; the plan for the streamed DSP-4 commands (roots-80, 2026-11-20)

- **`player_segment_step_long` ($82:A9AF) and `player2_segment_step_long` ($82:A9B3):**
  `JSR` the segment step, then `RTL`. The bank-`$85` code reaches them by `JSL` after a
  race section seek.
  - Each has 5 entries, so every flag counts as live. A shared helper models them from
    the callee's last `SBC`.
  - Live 4/4 (plus 1 replayed) and 5/5, fuzz 500/500, paths 2/2, flags 0 mismatches.
- **`$06` is stateful.** It takes no parameters and returns 32 bytes, which `$82:AE15`
  copies to `$1776`+Y: the OAM high table, 2 bits for each of 128 sprites. The chip
  builds it from the earlier sprite commands (`$09`, `$03`/`$0B`), so it cannot be
  measured as a pure function the way `$0A`, `$11` and `$0B` were. This is inferred from
  its shape; one `--dsp4-cmdlog 06` run would confirm it.
- **The plan, after the advisor (2026-11-20).** The ordinary roots are not used up.
  `roots.py` prints only 15 of its 33 roots (`tools/roots.py 40` shows them all). They
  come first, in incremental units: `$8A:9908`, `$82:A7F9`, `$8A:898F` and the parked ones,
  re-triaged.
- **The tape: harness-4, the unit that pays the next full gate.** It records every DSP-4
  data byte of a taped sample in order, and the C side consumes all of it.
  - It does not rely on the shadow's command boundaries, which are not reliable. The 195
    and 528 runs from other writers show it. So does `$03`'s "10 bytes": the game waits
    on the status register before `$0005` and before the first `$000B`.
  - The bench consumes a native's whole transaction (and compares its operands) where a
    taped routine's callee uses one.
  - The pilot is `$82:D934` (writes only), then the `$06` reader `$82:AE15`.
  - It is opt-in per row and has a negative row.
- **Open question for the owner.** Routines verified on the tape still need a chip behind
  the port: native `$09` (sprites) and the road commands `$01`/`$07`/`$08` in
  `src/dsp4.c`. Whether that is in this project's scope, or left to the ports, is the
  owner's call. Until then the tape verifies how the routines use the chip, not the chip.

## The track-event table's empty handler (roots-79, 2026-11-20)

- **`track_event_none` ($82:EA80):** an `RTS`, one entry of the track-event table (8,350
  entries on the 60k movie). Nothing changes. Live 2,805/2,805, fuzz 500/500.

## A per-row fuzz stack (harness-3, 2026-11-20; decided by the advisor, open to the owner's veto)

- **The problem.** The fuzz runs each case's original with a sentinel frame topped at
  `$1F00` (stack pointer `$1EFE`). That lies over the game's own words `$1EEE`-`$1EFD`
  (`$1EF4`/`$1EF6` the road's place, `$1EF8`/`$1EFA` the edges' factors). A routine whose
  callees write them overwrote its own return addresses. One that reads them read its own
  pushed words. Roots-78 had to work around this.
- **The change.** A row may set `.fuzzStack`, the highest byte of its fuzz sentinel frame.
  0 keeps `$1F00`. All three frame layouts (`RTS`, `RTL`, interrupt) are now built from
  that one base, so rows without the field are byte-identical to before.
- **The value comes from a live measurement.** Each live sample's entry stack pointer is
  recorded. For `road_shift_frame` it is `$1FEF` on the 60k movie (the game sets `$1FFF`
  at boot and runs the race there). So the row uses `$1FF1`, and its wrapper `$82:EC32`
  `$1FF3`. The C's game words in `$1Fxx` stop at `$1F34`, far below.
- **Workarounds removed in the same unit:**
  - index 4 (`road_shift_by_place`) is back in the fuzz;
  - `$1EFA` is an independent random word;
  - `road_shift_frame_call` (`$82:EC32`) has its row.
- **Acceptance (race1).** It is the first direct test of the stack diagnosis, which had
  been inferred from addresses.
  - `$0260` pinned to 4: before, 0/0 with 100 lost; now 100/100.
  - `$0260` pinned to 2 with a random `$1EFA`: 100/100.
  - Both rows: fuzz 200/200, paths 76/76 and 2/2.
- **Still open.** On the default stack, bytes an original pushes over `$1EEE`-`$1EFD` are
  skipped by the fuzz compare (live samples still compare them). Moving the default for
  every row is a larger change, not made.
- **Checked after the full run.** `map/verified.txt` was diffed against the previous commit
  with the hash column left out:
  - one new row (`road_shift_frame_call`), none gone;
  - two rows changed only in their live sample counts: `race_cycle_blocks` 3,715 to 3,713
    and `wait_frames_82` 23 to 19. Both stay verified, with the same fuzz and path counts.

  Those counts depend on which row records each interrupt, which the rows added since the
  last full run change (WORKFLOW notes the effect). The fuzz stack does not touch live
  samples.

## The road shift's frame step (roots-78, 2026-11-20)

**`road_shift_frame` ($82:EC36, D = `$0100`, DB = `$30` inside, `$83` after):** 18,559
entries on the 60k movie.
- **The road's place at the car.**
  - `D+$16` = the row offset (`$0226` × 2, & `$3E`).
  - `D+$14` = t (`$0224` / 2, & `$FF80`).
  - `$1EF4:$1EF6` = the row at `$7E:3DEC` plus its slope × t (a DSP-4 multiply). The high
    word is less 1 when positive.
- **A first handler, by `$0261`** (the table at `$82:ED23`):
  - `race_clear_0242_0208`;
  - or `road_edges_side` (`$82:ED27`, never run). It compares the car against the rows
    `$3C6C`/`$3F6C` at t and sets `$0242`, `$0208` and `D+$3C` from row `$3E6C` and
    `$1EF8`.
- **A second handler, by `$0260`** (the table at `$82:ED17`):
  - `race_scroll_commit`;
  - `road_edge_shift` (`$82:EE2A`, never run), which scales `$020C` from `$1EFA` and the
    row when `D+$3C` is set;
  - `road_shift_by_place`;
  - by row (`$82:EFCC`, never run): settle below row `$18`, else apply;
  - `race_scroll_commit` again;
  - `road_shift_apply`.
- **After both:** `$1EC2:$1EC4` take `$1EF4:$1EF6` as they were between the two
  handlers. Then `$0229` = `$01CE`, and `$01CE` = |`$01CE`| / the low byte of `$1E9E`
  (the CPU divider; `$FFFF` for a divisor of 0), with the opposite sign (a DSP-4 multiply
  by ±1).
- **The two never-run handlers** sat in about 400 undecoded bytes (`$82:ED27`-`$82:EEB7`).
  They decode as coherent code and are now static code. Their C functions have no rows:
  this row's fuzz reaches them, as with `track_object_random`.
- **The fuzz's stack collided with the game's words.** At first the fuzz ran the original
  with its stack at `$1EFE`, over the game's words `$1EEE`-`$1EFD`, which these handlers
  write. That needed workarounds (one handler left out of the fuzz, `$1EFA` forced, no row
  for the `$82:EC32` wrapper). Harness-3, the next unit, removed them.
- **Fuzz bias.** Half the cases put the car between the road's two edges and pair both
  never-run handlers. A temporary count saw the edges set 107 times and the scaled shift
  66 times in 200 cases.
- **No register outputs but DB.** The caller returns into a routine that loads its own
  registers.
- **Result:** live 4,000/4,000 (60k) and 216/216 (race1), fuzz 200/200 (race1), paths
  76/76.

## The road shift by the player's place (roots-77, 2026-11-20)

**`road_shift_by_place` ($82:EEB7, DB = `$30`):** one of the road-shift handlers that
`$82:EC36` reaches through its table at `$82:ED17`. 123 entries on the 60k movie, none on
race1.
- **Bit 5 of `$0226` set:** `road_shift_apply` runs with `$1E66`'s bit 0 moved to bit 15
  (the rest shifted right), then `$1E66` is put back.
- **Clear:** `road_shift_settle` runs. Then `$1E66` keeps its bits 15-1 from before and
  takes bit 0 from the bit 15 the settle left.
- **Flags.** The caller pulls and stores saved words next, so no registers are compared.
  But only 62 returns are watched, so every flag counts as live:
  - N and Z come from the final `PLA`;
  - V comes from the callees' last `ADC` (`$1EF4:$1EF6` plus the move), as their adapters
    compute it;
  - C also comes from that `ADC` after the apply, and is 0 after the settle (a `ROL` of
    0).
- **Result:** live 62/62, fuzz 500/500, paths 26/26, flags 0 mismatches.

## The second player's segment step (roots-76, 2026-11-20)

**`player2_segment_step` ($82:FA4F):** `player_segment_step` for the second player, 5
entries on the 60k movie.
- **Its variables:** its place `$02D9`/`$02F1`, its jump `$1EE2`/`$1EE4`, its cursors
  `$1E62`/`$1E64`, its road rows at `$7E:436C`/`$43AC`, its words `$02B5`-`$02BF` (where
  the first player has `$0202`-`$020C`), `$1EC6`/`$1EC8`, `$1E68` and `$1EE8`. The handler
  table is at `$82:FB10`.
- **Three stretches never ran:**
  - the armed jump (`$82:FA59`);
  - the kind 6/10 handler's armed side (`$82:FB22`);
  - the kind 8 handler (`$82:FB6C`, `STZ $1E68; RTS`).

  They were undecoded and are now static code. Each mirrors the first player's code
  byte for byte, with these addresses.
- **The C:** a helper `segment_step` that takes the player's addresses. The new routine
  calls it. `player_segment_step` keeps its own body: changing it now would not be an
  additive change, and could share the helper in a later full-gate unit.
- **Flags.** Only 5 returns are watched, so every flag counts as live. The adapter models
  them from the last `SBC`: N set, Z clear, C and V of the subtraction, carry-in from the
  `ADC #2`.
- The fuzz is `player_segment_step`'s, on the second player's words.
- **Result:** live 5/5 (60k) and 1/1 (race1), fuzz 500/500 and 200/200, paths 79/79,
  flags 0 mismatches.

## The player crosses the track's segments (roots-75, 2026-11-20)

**`player_segment_step` ($82:F2B3, D = `$0100`):** 18,564 entries on the 60k movie. It was
parked for a handler that had never run and was not decoded.
- **What it does** (the player's version of `segment_advance`, `$81:FBB1`):
  - an armed jump (`$0226` at or past `$1EDE`) moves `$0226`/`$023E` by `$1EE0`,
    rebuilds the road's curve row for `$1EDE` at `$7E:3DEC`/`$3E2C`, moves `$0206`/`$0202`
    and disarms;
  - then every segment (20 bytes at `$84:0000 + $019C`, cursor `$1E5E`) whose start is
    `$3F` or more behind `$0226 + 1` runs its kind's handler (byte +2; the table at
    `$82:F374`, kinds 0-10, even);
  - a second cursor (`$1E60`) follows the starts reached and sets bit 1 of `D+$2A`.
- **The handlers:**
  - kinds 0, 2 and 4 do nothing (`func_82F380`);
  - kinds 6 and 10 (`$82:F381`, never run in a movie) do one of two things. With bit 15
    of `$1E66` they move `$0204:$0206` by `$1EC2:$1EC4` and arm the segment's jump.
    Otherwise they move `$0226`/`$023E` by `$1EE6`;
  - kind 8 (`$82:F3D0`, 4 bytes never decoded before) is `STZ $1E66; RTS`. It is now
    static code.
- **Kind 8 is in no real segment table.** A scan of bank `$84` for runs of valid
  segments finds kinds 6 and 10 but not 8. So that handler's C is written from the bytes
  and reached by neither the movies nor the fuzz.
- **The fuzz:**
  - Both movies end on a track whose table has no segment within reach (its first start
    is `$7000`). So half the cases use byte runs of bank `$84` that parse as segments
    holding kinds 6 and 10 (`$84:C1C1`, `$E171`, `$E675`, `$F257`; bank `$84` is ROM only
    from `$8000`). Roots-94 found that some of them are not real tracks' tables: their
    pointers can lead out of ROM.
  - The player's place stays well before the table's end, even after the armed jump and
    `$1EE6` move it on: past the end, the original jumps through stray table entries.
  - A temporary count saw both branches of the kind 6/10 handler (45 and 38 times in 200
    cases).
- **No register outputs.** Neither continuation (`$82:DFA9`, then `$82:A9B3` after the
  `RTL`) reads them.
- **Result:** live 4,000/4,000 (60k) and 215/215 (race1), fuzz 500/500 and 200/200, paths
  79/79.

## $03/$0B in the harness; the mini-map's cars (dsp4-1b, 2026-11-20)

- **In the difftest's DSP-4 command table:**
  - `$03`: 10 bytes in (its word, then the first sprite entry, `$000B` word included),
    6 out;
  - `$0B`: 6 bytes in, 6 out.

  A sample starting at `$82:D8C6` sees its own `$000B` as a command word even when the
  chip takes it as `$03`'s data. The recogniser starts each sample at the command
  boundary, so both shapes fit.
- **The bench chip answers both with `dsp4_sprite`** (the bridge's route table), as for
  `$0A` and `$11`. The game's own runs do not change: every row the full re-verification
  runs still passes.
- **`minimap_car` ($82:D8C6, DB = `$30`):** one car's dot on the mini-map, two calls a
  race frame.
  - Its place in the map table `$7E:4F31` is X. When the `$82:DC68` entry for `$047B`
    is negative and `D+$14` is set, the place is the track's start or end instead.
  - The table's two bytes give x (+ `$BE`) and y (+ A, low byte). Y is the attribute.
  - A shown entry's words go to `$1576`/`$1578` + `$0184`, which moves on by 4.
  - Both callers reload A, X and Y next, so the row has no register outputs.
  - 60k: live 4,000/4,000, fuzz 500/500, paths 53/53. race1: 393/393.
  - **Negative row `minimap_car_wrong`:** the sprite's word with x and y swapped. It fails:
    race1 0/393 live, 0/50 fuzz.

## DSP-4 commands $03 and $0B (dsp4-03, 2026-11-20)

- **Measured, outputs only**, as for `$0A` and `$11` (`refbench --dsp4-cmdlog 03` and
  `0B`).
- **The protocol.** `$82:CBD4` sends `$0003` and one word, `$0005` in every run. Then
  `$82:D8C6`, the mini-map's two cars, sends one entry for each car:
  - The first car's entry goes straight after `$03`'s word. The shadow counts it as part
    of `$03`: 10 bytes in, 6 out.
  - The second car's entry is a command `$000B` of its own: 6 bytes in, 6 out.
  - Each entry is three words in: x (a map byte + `$BE`), y, and an attribute.
  - Three words come out: a flag (the game skips the entry when it is 0), the low bytes
    of x and y packed into one word (x in bits 7-0), and the attribute unchanged.
- **`dsp4_sprite` (src/dsp4.c)** is that formula. refbench checks every one of the game's
  entries over the whole TAS: **`$03` 117,650 runs, `$0B` 117,664, 0 differ.**
- **Only the game's entries are checked.** These are the runs whose command word is
  written at `$82:CBE1` or `$82:D8FA`. Elsewhere the shadow sometimes takes a data word
  `$0003` or `$000B` of another command for a command: 195 and 528 runs, of odd sizes.
  The first unfiltered pass counted those as differing until the filter was added.
- **The domain is small:** x 192-227, y 32-67, attributes `$30FD`/`$32FD`, and the flag is
  always 1.
  - The chip's 0 (an off-screen entry, probably) never occurs, so it is not measured. The
    C shows every entry, and `src/dsp4.h` says so.
  - Chosen inputs fed to the idle chip would measure it (outputs only). They were not
    needed for the game's runs.
- **Next:** `$03` and `$0B` join the difftest's command table and the bench bridge's
  routing. Then `$82:D8C6` becomes a row, and `$82:CBD4` after the `$09` stream.

## The player's car moves and meets the track objects (roots-74, 2026-11-20)

**`car_track_step` ($83:F7F9, D = `$0200`, DB = `$30` inside):** the last multiply-only
DSP-4 root.
- **The move, by `D+$44`.** Free running moves the position `D+$25:$27` by
  `$1E9E × D+$10`. Bit 5 is a scripted approach: `D+$70` ramps by `4 × $1E9E` (at most
  `$100`) and the position moves `$1E9E × D+$70` until it reaches the stop at `D+$5E`.
  With bit 8, `D+$5E` counts down instead and `D+$70` ramps down to `D+$72`.
- **Reaching the end** sets bit 7 and requests effect `$FF2A`.
- **Then** `car_lap_check` runs, and every 256-unit segment crossed runs its object from
  `$7E:8CB3` (4 bytes a segment, wrapping at `4 × $019A`). Each object goes through its
  kind's entry in the handler table at `$83:7FFE`. `$011E` sums the road's curve as it
  goes.
- **The table has 104 kinds**, not 63 as first read. A first test failed live on one
  sample (`$0253`): kinds 80-97 use six more handlers, the `D+$53` markers (`$83:FD26`,
  `FD31`, `FD3C`, `FD45`, `FD5D`) and a stop marker (`$83:FD68`, which sets `D+$5E`).
  Kinds 9, 121 and 122 have a null entry; past 104 the table is other data. The C maps
  each handler address to its C.
- **The random pickup** (`$83:FB4A`, kinds 24 and 93) never runs in a movie. Its C,
  `track_object_random`, has no row of its own (a row needs live samples): it is checked
  through this row's fuzz.
  - On a hit it clears the entry and requests effect `$FF19`.
  - One of 16 prizes is drawn by `rng_next_long` (the table at `$83:FB76`):
    - money + 4000, + 1000 or + 10000 (BCD);
    - `D+$4E` + 1 up to 10;
    - `D+$58`/`D+$56`;
    - `D+$5A` = 0;
    - `D+$48` = `$300`.
  - Each prize also sets `D+$33`.
  - The 95 bytes that never run are decoded as static code (`$83:FB96`, `$83:FBF2`).
- **The fuzz moves the car forward.** A backward move makes the segment count negative,
  so the original walks tens of thousands of segments, through the null kinds, and
  crashes. The game never moves backward here.
- **The fuzz aims half the pickups at the car** (across within ±32, height 0). Without
  that, `func_83FC05`'s hit test almost never passed. With it, a debugging count saw all 7
  prizes in one 500-case run.
- **No register outputs but DB** (`$83`, its final `PLB`). The caller `PLD`s, and the next
  routine loads A and X first.
- **Result:** race1 196/196 live, fuzz 500/500, paths 201/201.

## The race HUD's numbers (roots-73, 2026-11-20)

**`race_hud_digits` ($82:B4ED, DB = `$83`):** about 650 bytes, the last of the
multiply-only DSP-4 roots ported so far.
- **Gear:** `$003E` = the high word of `$0213` * `$160` (on the chip), capped at `$0C`.
- **Speed:** the high word of `$0210` * 8 * `$3000` (`$4D3B` with `$03A7` set), on the
  chip. Its digits come from the CPU divider and go out as tile pairs from `$83:A4AC`:
  hundreds, tens and ones into `$0042`-`$004C`, with blanks (`$0157`) where there are none.
- **Two tiles** for `D+$42` (`$0064`/`$0066`).
- **An 8-cell bar** from `$0258`: `$006C`-`$0074`, the partial cell's rows at `$0080`.
- **Two more tile sets** from `$025B` and `D+$4C`.
- **Then the car's map block** (`func_82B838`, `func_82B769`) and `race_message_update` on
  the messages at `$7E:4E3D`/`$0233`/`$4E45`/`$4E4D`.
- **The block's offset.** The C stores `func_82B7B0`'s block offset, the X that `STX`
  saves, computed the same way, because `func_82B769`'s C returns nothing.
- **Never run**, decoded as static code and reached by the fuzz:
  - two negations of negative products;
  - the `$1E94` + 1 override;
  - a thousands digit;
  - both players' blocks.
- **No register outputs.** Both callers set what they read; the bench saw no live flag.
- **Still to do:** `$83:F7F9`, the track object dispatcher, waits for `$83:FB4A`, the
  random-pickup handler (ported next, in roots-74).
- Live 4,000/4,000 (60k) and 216/216 (race1), fuzz 500/500 and 200/200, paths 269/269,
  first try.

## The player's car takes the player's motion (roots-72, 2026-11-20)

**`car_sync_player` ($81:FA8F, DB = `$7E`):** the fourth multiply-only DSP-4 root, about
18,500 entries.
- **The motion,** two ways by bit 0 of `$0244`:
  - **directly:** the car's 32-bit position gains `$022D:$022F`, and it takes the player's
    speed and +`$04`/+`$06`;
  - **or by the chip:** the position gains `$1E9E` * `$0210` (a DSP-4 multiply), and the
    speed is taken as before.
- **Then** +`$12` = `$FFFF` and `func_81FBB1` (the segment advance, in C since long
  before). Directly, the car's flags then take bit 5 from bit 15 of `$1E66`, and bit 4.
- **The exit is `func_81FBB1`'s,** read through its adapter by a hook (`fbb1Run`), as for
  the contact chain.
  - The adapter writes the car's record as it stands at that call. The product is
    computed in plain C, so there is no second chip multiply.
  - The hook puts the record back before the C runs.
  - Directly, A is then the new flags and V is the `BIT` of `$1E66`.
  - Editing that adapter's line is why this unit takes the full gate.
- Live 4,000/4,000 (60k) and 205/205 (race1), fuzz 500/500 and 200/200, paths 50/50,
  first try. `func_81FBB1` passes unchanged.

## A duplicate port, reverted (roots-71 reverted, 2026-11-20)

- **What happened.** roots-71 ported `$81:FBB1` as `segment_advance` in `src/misc.c`.
  It already had C: `func_81FBB1` in `src/track.c`, verified. My check for an
  existing port grepped `map/symbols.txt` for `^81FBB1`, but its lines start with
  `func`, so it found nothing.
- **Why it slipped through.**
  - The new row replaced the old one in `map/verified.txt` (same entry), so the count
    stayed at 410.
  - No check refused two names for one address.
  - Both versions passed, so nothing compared was wrong.
- **Undone by a revert.** The original row and C stand.
- **`progress.py --check` now refuses** a ROM address named twice in `map/symbols.txt`.
- **The next unit** is `$81:FA8F`, whose callee was already in C, as `roots.py` had said.

## The view follows a car (roots-70, 2026-11-20)

**`follow_car_step` ($83:F73E, D = `$0200`, DB = `$30`):** the third multiply-only DSP-4
root.
- **The step:** while bit 0 of `D+$44` is set, the 32-bit view `D+$25:$27` moves toward
  car `$1EBA`'s position less `$40`. The step is the gap / 16 + 8, at most `$80`, times
  `$1E9E`, on the chip. The new gap is left in `$0114:$0116`.
- **Arrival** means the new gap lands between 0 and `$40`: the original's carry is that
  of the last `SBC #$40` alone. An overshoot wraps the 32-bit gap past `$40` and counts as
  still behind. On arrival it snaps to the car less `$40`, clears the bit, and counts
  `$1ECE` down (`$8000` becomes `$118`).
- **Never run:** with the bit clear, a `JMP` to the `RTS`, decoded as static code.
- **The fuzz took three tries.**
  - My first "arrives" case used a high speed, so every case overshot.
  - Two attempts to decorrelate the generator's bits changed nothing, because the
    generator was not the problem.
  - Landing in the window needs a crawl (speed 0-8 from a few units behind).
- Live 136/136 (60k) and 43/43 (race1), fuzz 500/500 and 200/200, paths 74/74 on both.

## The road shift settled (roots-69, 2026-11-20)

**`road_shift_settle` ($82:EFDA, DB = `$30`):** `road_shift_apply`'s twin.
- **Shared code:** the same front (the two multiplies, the negated sum) and the same tail
  (the offset applied to the car). Both are now shared functions; that edits verified C,
  hence the full gate.
- **The difference: how the offset is settled.**
  - **Zeroed** when `$0206` and `D+$24` (-`$1EF6`) differ in sign (`$020C` = 0, `$1E66`
    bit 15 cleared).
  - **Snapped** to `$1EF4` - `$1EC2` when `$0206` has passed `D+$24` (bit 15 set).
  - **Otherwise scaled** by a factor v. Below `$100` it is a fraction on the CPU divider:
    the remainder, times 256, over |`D+$24`|. Above, it is the gap less half of it, plus
    `$80`, zeroed or snapped out of range.
  - v scales `$020C` and `$1EF6` - `$1EC4` through two more DSP-4 multiplies: `$020C` =
    product >> 8, the offset = product << 8.
- **Never run:** the flagged-row width (`$82:F010`, as in `road_shift_apply`), decoded as
  static code. The bytes at `$82:F0D6` (a 32-bit halving after a `JMP`) are dead; nothing
  branches there, and the row declares them a hole.
- Live 42/42, fuzz 500/500, paths 198/198, first try. `road_shift_apply` passes again
  (41/41).

## The player car's shift along the road (roots-68, 2026-11-20)

**`road_shift_apply` ($82:F1C1, DB = `$30`):** the first of the six multiply-only DSP-4
roots.
- **The sum:** two chip multiplies. The road row's step (`$7E:3DEE` - `$3DEC`, times 4)
  times (`$8000` - `D+$14`) / 2, plus the next step's width times 2 times `D+$14`. The
  width comes from `$3DF0`, or for a flagged row from `$3DEE` less its signed byte at
  `$3AEF`.
- **The 32-bit sum, negated,** goes into `$020A`/`$020C`, after `$020B` is read.
- **The car's offset** (`$1EF4` - `$1EC2`, or 0 while `$1E66` is not negative) moves its
  `$0200`-`$0206` and `$1EF4`/`$1EF6`.
- **The C** uses `dsp4_multiply`, whose traffic the harness checks as whole `$00`
  commands.
- **A slip:** the first try read the tables at WRAM `$1xxxx` (bank `$7F`) instead of
  `$7E`. Every live sample failed.
- Live 41/41, fuzz 500/500, paths 99/99 (the flagged branch only by the fuzz).

## DSP-4 step 2: which command each root uses (dsp4-1, 2026-11-20)

**The instrument.** `bench ... --dsp4-report FILE` attributes every data access to the
command running around it. A write that starts a run belongs to the new command, and the
read that ends one to the command that ran. It sums accesses by PC and direction, with
bytes per run.
- The chip is the bench's (clean-room, `src/dsp4.c` answering `$0A`/`$11`), so the
  boundaries are exact for what the game is answered here. The road commands' read counts
  are this chip's, not the firmware's.
- The chip turns busy after the command word's high byte and idle before the last result
  byte. Corrected for that, the sizes over the 60k movie are below. `$0A` and `$11` match
  the harness table; the catalogue's result counts were one short, as the advisor said.

| cmd | runs | parameter bytes | result bytes |
|---|---|---|---|
| `$00` multiply | 2,553,460 | 4 (+ skips) | 2-4 |
| `$0A` | 333,895 | 6 | 8 |
| `$11` | 4,416 | 8 | 2 |
| `$03` | 18,559 | 10 | 6 |
| `$0B` | 18,559 | 6 | 6 |
| `$06` | 18,559 | 0 | 32 |
| `$09` | 18,559 | 118-3,520 | 1-1,389 |
| `$01` | 18,564 | 254-262 | 463-947 |
| `$07` | 18,564 | 348-360 | 355-835 |
| `$08` | 18,559 | 578-632 | 368-1,004 |

**By root,** from the access sites inside each one's own code:
- **Only whole `$00` multiplies** (portable now; "stream" was the wrong label):
  `$82:F1C1`, `$82:EFDA`, `$83:F73E`, `$81:FA8F`, `$83:F7F9`, `$82:B4ED`.
- **`$03` and `$0B`, whole commands:** `$82:D8C6`.
- **Road commands opened here:** `$85:B828` (`$07` and `$00`), `$85:B3D7` (`$01` and `$00`).
- **`$09` (sprites), streamed:** `$82:D934` (writes only), `$81:8527`, `$82:D988`,
  `$82:D267`, `$82:DEA8`. Its bulk is in the callee `$82:DB87`.
- **`$08` drained:** `$81:89F5` (reads only). `$81:8564` opens it.
- **Through callees only:** `$82:DAFD`, `$82:DB4D`, `$82:8961`.

**Next for DSP-4:** the six `$00`-only roots, then `$03`/`$0B` in `src/dsp4.c` (checked on the
firmware with refbench; the TAS is at the session scratchpad's `tas_hdr/`). After those,
the replay tape for the streamed `$09`/road commands.

## Raw VMDATA DMA carries its VRAM state (harness-2, 2026-11-20)

**The harness.** This is the second change decided above.
- **The new field:** a DMA recorded raw (not a plain `$2118` word upload, a fill or CGRAM)
  to `$2118`/`$2119` now carries `vram`: the word pointer and `VMAIN`'s fields (increment
  after the high byte, the step 1/32/128, the remap mode).
- **The original's side** reads them from the PPU at the kick.
- **The C's side** uses the address the C claimed (`video_set_vram_address`), or else
  where its last such DMA left the pointer, or else the entry's.
  - `VMAIN` is the one the C wrote (`video_set_vram_increment`), or else the entry's.
  - After each kick the pointer moves on as the hardware's does (in modes 0 and 1), and
    the claim is used up.
- **The proof:** a negative row, `vram_planes_build_wrong`, with its second upload one word
  off, fails on exactly that field (VRAM `80011200` against `80011201`).
- No verified C made a raw VMDATA DMA before, so no verified row changes.

**`vram_planes_build` ($8A:8828), now verified.** It builds VRAM one register at a time
with DMA channel 0:
- the high bytes of words 0-`$8FF` and `$1200`-`$155F`, from `$7E:8633` and `$7E:9833`;
- with `VMAIN` = 0, the low bytes of words 0-`$3FFF` filled from `$8A:88DD`;
- six rows of 15 low bytes from `$7E:80BA` on, the source going on;
- then `VMAIN` = `$80`.

Live 3/3, paths 71/71 (no fuzz: no inputs besides WRAM it only copies). Still to check
against this: `$80:EE56`, which sets up a DMA to `$2119`.

## Two rows lost by harness-1, and the gate that now refuses that (harness-1b, 2026-11-20)

**What happened.** harness-1's full gate passed while two verified rows, `func_82F380` and
`func_82FB1C` (one byte each), came out `unreached`: 404 routines instead of 406.
- Inside an interrupt that a waiting sample records, only handler rows are sampled.
- With `MAX_IRQS` = 64, `wait_flag_76`'s recording overflowed and stopped, so the routines
  its handlers call (the only place those two run) were sampled again.
- At 512 the recording went on and hid them. A temporary print in `difftest.c`, reverted
  afterwards, found the owner: `wait_flag_76`, at 64 interrupts or more.

**The fixes.**
- **Recorded interrupts:** 512 only for a row with a budget (`maxInsns`, the title card);
  every other waiting row keeps 64 (`MAX_IRQS_DEFAULT`).
- **The report** counts entries skipped inside recorded handlers, so a hidden row no
  longer reads as merely `unreached`.
- **`progress.py --check`** refuses a row verified at HEAD that the new
  `map/verified.txt` no longer has as verified. `test_carry.sh` has the case ("a verified
  row lost").
- Both checks only tighten, and are recorded under the owner's delegation, open to veto.
- The two rows pass again (1/1 and 3/3), with `title_card_show` and every waiting row.

## A per-row instruction budget, and the title card (harness-1, 2026-11-20)

**The harness.** This is the first of the two changes decided above.
- A row may set `maxInsns`, honoured only when it also sets `waitsForInterrupts`; the
  default stays `MAX_INSNS`, 4M.
- `MAX_IRQS` (the interrupts recorded in one sample, each with two WRAM snapshots) goes
  from 64 to 512. A 200-frame wait records more than 64.
- The arrays are BSS and touched only when used, so other rows cost nothing.
- **Corrected (harness-1b):** "nothing compared changes" was wrong. Raising `MAX_IRQS` for
  every row changed what is sampled, and two rows were lost (see below).

**`title_card_show` ($8A:C056), now verified.** It was written in roots-56 and reverted.
- It clears the fade state and the direct page.
- It loads the screen and clears VRAM, unpacks the tiles, then loads the text palettes,
  the sprites and the tilemap.
- It sets the frame handler to `$FC5F` and BG1's vertical scroll, then lays out six sprite
  slots and five lines of tile text.
- Then it fades in, waits 180 frames one at a time, fades out and waits for the fade's end.
- **The flags.** The C returns the frame its last one-frame wait ended on, so the adapter
  can model V (`wait_frames(1)` from `$7FFF`). Y is `func_8AC0F2`'s, `$14`, fixed by the
  ROM's lines.
- Live 1/1, `maxInsns` = 40M, paths 57/57. It has no fuzz: it waits for interrupts.

## Audit: return values no adapter compares (audit-1, 2026-11-20)

**Why.** roots-53 showed it: a C function's return value is unverified when no adapter
compares it. The rule now is that a non-`void` return feeds a compared output, or it is
documented as what it is. The `bool` functions were checked against their adapters.
- **`car_contacts_chain`:** the early exit (`$81:F228`, `BCS` to the `RTS`) leaves the
  carry set, but the C returned false. It now returns the original's carry. The adapter
  had set C itself, so nothing compared changes.
- **`func_83FE9A`'s header** said its return is the original's carry. Off the cooldown
  path the original keeps the caller's carry; the header now says so.
- The rest either feed their adapters (`race_upload_queue_drain` the DB, the shop moves,
  the hit tests' carry through `calleeRun`) or are the carry that their adapters model.

**The title steps' probe.** `titleUnpackEnd` found the input end by unpacking a second time
after the C, into the WRAM under comparison. It now runs before the C and puts WRAM back,
so the compared bytes are the C's alone.

All four rows pass again: `car_contacts_chain` 4,000/4,000; the three title steps 84/84,
58/58 and 1/1, each with fuzz 500/500.

## Two harness changes, decided by the advisor (2026-11-20)

The owner's standing delegation covers these, since neither weakens a comparison. They
are recorded here and open to the owner's veto, and each comes as a unit of its own.
- **Raw DMA to VRAM.** A DMA the harness records raw, to `$2118`/`$2119`, will carry the
  VRAM pointer and `VMAIN`, on both the original's side and the C's. It comes with a
  negative row (a wrong address), and the full gate audits the existing rows. This
  unblocks `$8A:8828`; `$80:EE56` (a DMA to `$2119`) is to be checked against the same gap.
- **Instruction cap.** A per-row budget for `waitsForInterrupts` rows only, so the title
  cards (`$8A:C056`, `$8A:9E82`) that wait about 200 frames are compared instead of
  abandoned at `MAX_INSNS`. Nothing compared changes.

**Also found:** the TAS for refbench is at the session scratchpad's `tas_hdr/Input
Log.txt` (the `.bk2` in `tas_dl/`).

## The track tiles for a preview (roots-66, 2026-11-20)

**`track_tiles_preview` ($8A:8BC2, DB = `$83`):** the bank-`$8A` twin of
`track_tiles_load`.
- **The front is the same:** the kind from `$83:8410` and its picture unpacked to `$7F`.
- **The layouts:** four, chosen by `$0444` through a 55-entry table at `$8A:8BED` (a
  hole). Each runs `tiles_planar_upload`, `upload_tiles_c280`, `upload_tiles_c680`, then
  `palette_load_70`.
- **The exit:** `palette_load_70`'s, which keeps C and V, so those flags are
  `upload_tiles_c680`'s `ADC`.
- All 4 live entries were interrupted; the replays passed (2/2). Fuzz 500/500, paths
  57/57, first try.

## The track tiles (roots-65, 2026-11-20)

**`track_tiles_load` ($85:8000, long, DB = `$83`):** loads the track's tiles.
- **The kind:** the byte at `$83:8410` + the track picks it. Its picture unpacks to
  `$7F:0000`, by the bank and address at `$83:8597`/`$8585`.
- **The layout,** by `$047B`, through a 55-entry table at `$85:8025` (a hole in the row)
  pointing to four layouts. Each layout:
  - clears or sets bit 7 of `$1F33`;
  - runs `tiles_planar_upload_5fe0` and the four `upload_tiles_*` at its own offsets;
  - then `row_copy_6a33`/`6b33`.
- **Player rows:** in the third and fourth layouts the rows move on by `$20` for a player
  whose flag (`$039C`, `$03CB`) is set. Every callee keeps X, so all calls get the kind.
- **Never run:** the third and fourth layouts, and three 3-byte `LDA #$20`, decoded as
  static code.
- **Parked this session:** `$80:8EB4` (its `BNE` goes past the first `RTS` into a whole
  shop screen, `$80:8EC6`, with waits, MVN and many callees; the C written for the first
  17 bytes was reverted) and `$80:C5E0`. Four of `$80:C5E0`'s six handlers never ran, and
  two of those call never-run, unported `$80:D026`/`$80:CCE1`.
- All live entries were interrupted; the replays passed (2/2, 1/1). Fuzz 500/500 and
  200/200, paths 102/102.

## A menu with a sliding bar (roots-64, 2026-11-20)

**`menu_bar_draw` ($80:C35E):** a four-item menu, about 3,200 entries.
- **The bar's position:** `D+$4A` & `$FF00`, placed between the keys at `$81:BE0A` (0,
  `$B00`, `$1600`, `$2100`). At a key it takes that key's value from `$81:BE12`.
  Between two keys it takes the value less (key - position) / (the step between the keys'
  high bytes, on the CPU divider) times the values' step (bits 8-23, on the PPU
  multiplier).
- **Drawing:** two 8-sprite bars (`sprite_bar8`), then the four labels centred,
  `$B00` apart, the chosen item in attribute `$300`.
- **The slide:** `D+$4B` moves one step toward the chosen item's key.
- **No outputs:** both callers go on to `oam_finish_near`.
- Live 836/836, fuzz 500/500, paths 127/127, first try.

## The title frames restarted (roots-63, 2026-11-20)

**`title_frame_begin` ($80:F5CD):** resets the title frames.
- It sets `D+$40` = 0, `D+$42` = 1, `D+$1E` = `$3C`, and `D+$3E` counts.
- Then it jumps to `$80:F670`, `title_frame_step`'s tail: the curve from frame `$15C` (not
  here), frame 0's picture, then `D+$40` = 2.
- The row declares that tail with `alsoIn`. In the C, the tail is a shared function
  (`title_frame_tail`).
- **A correction.** Reading this showed that roots-62's header put the windows inside the
  `$80:F670` part; they come before it. Header and doc are fixed.
- This edits verified code, so the unit takes the full gate. `title_frame_step` passes
  again (84/84, 500/500).
- Live 1/1, fuzz 500/500, paths 7/7.

## Two title steps (roots-62, 2026-11-20)

**`title_frame_step` ($80:F5DE):** one step of the title picture, by its frame `D+$40`.
- At frames `$13A`-`$140` it sends a `$614`-byte VRAM upload from bank `$7E`.
- Then the windows: both at 255-0, BG1/BG2 masked by window 1 (`$2123` = `$0A`, then
  `$02`), the HDMA line tables cleared.
- From `$80:F670` on (corrected in roots-63: the windows come before it): from frame
  `$15C` on, `D+$42` counts and `window_curve_build` runs.
- Then the frame's picture unpacks from the pointer table at `$7E:A920`.
- `$80:F5CD` (parked for its tail into `$80:F670`) can come next.

**`title_fade_step` ($80:F6AD):** `D+$42` counts. The level (two ramps, at most `$3F`) goes
to `palette_fade_60`, then `window_curve_build`. At `$3F` the picture `$7E:C730` unpacks.

**Both, in common:**
- They end with `SEC`. Their other exit registers are `decompress_near`'s or
  `window_curve_build`'s. The adapters get the input end by unpacking the same stream a
  second time, which writes the same bytes.
- The pictures are compressed in WRAM bank `$7E`, which is junk in the race RAM the fuzz
  starts from. The fuzz copies a compressed stream from the ROM (`$88:929A`) there and
  points the table at it.
- Live 84/84 and 58/58, fuzz 500/500, paths 82/82 and 36/36, first try.

## The scrolling-text scene, one frame (roots-61, 2026-11-20)

**`ticker_scene_step` ($80:E925):** about 2,500 entries.
- **Camera:** the 24-bit `D+$7B` gains the sign-extended `D+$80`, and `D+$7E` gains
  `D+$82`, which decays by `$12`.
- **View words:** `D+$2E`-`$3C`.
- **Text state machine on `D+$46`.** It feeds `ticker_draw` with the line at
  `$7F:2000 + D+$5E`, or with the list at `$81:D5CF`, stepping through it by the bits of
  `D+$48`. It also starts the fades.
- **The rest:** sparks, `func_80EB66`, `oam_finish_near`.
- **Never run**, decoded as static code: `D+$82` decaying from below, and a 9-bit sign
  extension.
- **C is read after it.** On a negative `D+$46` it is the `CMP #$FFFF`'s; otherwise it is
  `oam_finish_near`'s: its `SBC #$0900` of the old `D+$6E`, which is `D+$6C` afterwards.
  The adapter simulates the `D+$46` steps to tell which.
- **Fuzz lessons:**
  - `D+$5E` is the `$7F` double buffer (0 or `$8000`) shared by the sparks and the ticker,
    so their state has to agree.
  - The fuzz starts from the RAM at the end of the movie, a race, whose `$6C` (`$270C`)
    sent the OAM packer's direct page into I/O. The fuzz now sets the title's OAM buffers
    first.
  - A temporary print of the PC in `difftest.c`, reverted afterwards, found it at
    `$81:810E`.
- Live 852/852, fuzz 500/500, paths 159/159.

## A car's contacts down the chain (roots-60, 2026-11-19)

**`car_contacts_chain` ($81:F21E):** about 357,000 entries in the 60k movie.
- Unless object y is `$60` or more ahead of x, `D+$3C` counts up. Then
  `car_pair_contact(x, y)` runs along the chain of links at +`$18` (`$7E:FACD`), for each
  object less than `$60` ahead.
- It stops at the first swap, when the chain leaves that range, or when `D+$3C` (counting
  down) reaches 0. `D+$3C` is put back to its counted-up value.
- **A correction to roots-53.** `car_pair_contact`'s push-aside exit ends with `CLC`, as a
  miss; only the swap sets the carry. The C returned true for both. The row passed
  regardless, because its adapter models the carry itself and no C read the return value
  until this caller, whose fuzz found it (39/500). The C now returns the original's carry.
- **Flags.** On a swap they are `car_pair_contact`'s at that call. Its adapter gained a hook
  (`carContactRun`) that runs this routine's C at the contact call, as `calleeRun` does for
  the track-object handlers. This edits a line of a verified adapter, so the unit takes the
  full gate.
  - The walk finds that call from the gates. A miss writes nothing, and a push aside writes
    only x's +`$06`, which the walk keeps and hands to that call (held, then put back
    before the C runs).
- Live 4,000/4,000 and 3,707/3,707, fuzz 500/500 and 200/200, paths 27/27. `car_pair_contact`
  passes again unchanged (4,000/4,000, 262/262).

## The track scenery (roots-59, 2026-11-19)

**`scenery_build` ($80:DCBB):** about 1 KB that builds a track's scenery from its list of
14-byte records (`$81:C3E8`; the list chosen through `$84:A240` by `$0444`).
- **Each record** appends to the lists at `$0882` and, for negative kinds unless masked
  by `$036A`, to `$0802`. It then dispatches on its kind through a table at `$80:DD65`,
  which the row declares as a hole.
- **Kind 2:** a 6×6 block of map words, the record's sprite and two palettes.
- **Kinds 4-10:** a 4×4, 4×3 or 2×2 block copied from `$7F` into the map. Each word adds
  `D+$84` with the carry of the add before: one `CLC`, then a chain, which the C carries
  along. Then two palette rows, an unpack from bank `$91` to `$7E:D+$7C`, a 4×4 block of
  map words and a 10-byte sprite entry.
- **The end:** kind 0, or `$8000`, whose handler is the table's first entry.
- **Flags.** No register outputs (its caller goes on to `star_layers_init`). Five entries,
  so every flag is modelled. The adapter walks the records from the ROM to find which end
  was reached.
- **Never run:** the 6×6 block's map row for a column below `$20`. No real track has such
  a record. The ROM's only one is list 41, reached with `$0444` = 225, which the fuzz uses
  now and then.
- All live entries were interrupted, and the replays passed (2/2 and 1/1). Fuzz 500/500
  and 200/200, paths 458/458.

## The text buffer screen (roots-58, 2026-11-19)

**`text_buffer_screen` ($8A:C15D):** the text buffer at `$7F:7D00` printed as tiles, up to
its 0 byte.
- A `$80` byte starts a new row from column 2. From row `$0C` on, `D+$90` = 0, and row
  `$0C` itself is skipped. Every other byte goes to `text_put_tile_char`.
- Then `text_screen_init` (roots-50) with the string `$8A:C224`, or `$C238` when `$1B37`
  bit 15 is set. That `LDX` never runs and is decoded as static code.
- Its exit is `text_screen_init`'s, modelled from the state after it, as that adapter does.
- All 4 live entries were interrupted; 2 were replayed from their entry state (2/2), with
  fuzz 500/500 and paths 26/26.

## The race HUD sprites (roots-57, 2026-11-19)

**`race_hud_sprites` ($80:D823):** the sprite side of the race HUD, once a frame. The
tilemap side is `race_hud_draw` (`$85:8814`); the first name tried here clashed with it.
- **Windows:** the two window shapes.
- **Readouts** in attribute `$2200`:
  - the shown speed, with a point;
  - the same speed times `$4D` * 2 / 256 in the other unit, rounded by the product's bit
    6 through an `ASL` of `D+$16`;
  - `D+$35` * `$63` / 256 and `D+$37` * `$41` / 128.
  - Each product comes from the PPU multiplier, and the C sets matrix A and B as the
    original leaves them.
- **Names:** three names centred from the tables at `$86:CEA7`, `$CEBF` and `$D0A7`.
- **Lights and speed:** `race_lights_draw`, then the shown speed moves toward `D+$32` by
  `$10` at most.
- **Object list** at `$0700`: the sprite groups that are on screen. The original's loop
  passes over the second entry twice (its index starts at `$0A`, and so does the counter
  after the first pass), and the C does the same.
- **OAM tail:** filled from the `$0F00` table: two 2-tile sprites, then 1-tile ones.
- **Never run**, decoded as static code: the speed falling by `$10`, and the first `$0700`
  entry on screen.
- **No outputs.** Both callers go on to `oam_finish_0b00_near`, which sets A and X first
  and never reads Y. The bench saw no live flag in 648 watched returns.
- Live 648/648 (60k) and 482/482 (race1), fuzz 500/500, paths 243/243, first try.

**Parked this session:** `$8A:82C0` (a state in the 627 unknown bytes, like `$8A:898F`) and
`$8A:C056`. The C of `$8A:C056` was written, then reverted: its one live sample waits about
200 frames and is abandoned at `MAX_INSNS` (4M). It needs a per-row instruction cap in
`difftest.c`.

## The scrolling text line (roots-55, 2026-11-19)

**`ticker_draw` ($80:EBF7):** a line of scrolling text drawn as 2-tile sprites, about 600
bytes with a control-code table.
- **Characters:** digits and capitals come from `$81:CCD9`, a space advances 8 pixels, and
  marks are looked up in `$80:EDE7`.
- **Control codes** (a 0 byte, then the code): 0 is the end, and the pointer stays on it
  as padding. 1 sets x. 2 is a counted repeat. 3 sets the stop bit of `$E1`. 4/5 turn on
  and off a 1-tile sprite stream from `$81:D2E3`.
- **A queue** of 28 words at `$7F:D+$5E` decides how far the line is revealed and its
  palette bands. An end marker fades through `$E1`.
- **Never run, decoded as static code:** the mark lookup (`$80:ED04`) and codes 2 and 3
  (`$80:EE0D`-`$EE45`). The row declares the table and the marks as a hole.
- **No outputs.** Both callers reload every register and set every flag before reading.
- **A quirk in the original.** The `$C6` step loop reloads A with `D+$CC` after its first
  step and loops back to the subtract. It takes one `$100` step per call, or more only
  while the counter is `$100` or more. The fuzz found it: 280/500 failed with the obvious
  loop. The C keeps the quirk; the fuzz's counter stays below `$100`.
- Live 852/852, fuzz 500/500, paths 269/269.

## A track object tested on both sides (roots-54, 2026-11-19)

**`track_object_test_sides` ($83:FC65):** a handler in `$83:F7F9`'s table, like
`track_object_test`.
- Bits 4-5 of the object's byte at `$8CB5` pick a branch through a jump table at
  `$83:FC90`, which the row declares as a hole. The branch runs up to four hit tests at
  `$0128` = x, -x, -x - `$80` and x + `$80`, stopping at the first hit.
  - Indexes 0 and 3: `func_83FD8A` at each position.
  - Index 1: `func_83FD8A`, then `func_83FDCD`, at x and again at -x.
  - Index 2: `func_83FDCD` at each position. It never hits.
- **Flags.** It has no register outputs, as its siblings don't. The exit flags are the last
  test's, and that test's adapter reuses the verified model (`calleeRun`).
  - A miss writes nothing, so the adapter finds the last test from `func_83FD8A`'s
    conditions alone.
  - It hands that adapter the state at the call: A = `$0128`, X = the jump index,
    Y = `$011A`.
- Live 1,628/1,628, fuzz 500/500, paths 77/77, first try.

## Two cars in contact (roots-53, 2026-11-19)

**`car_pair_contact` ($81:F252):** contact between two race objects (offsets into the
tables at `$7E:FAB5`, stride `$35`). It runs about 25,000 times in the 60k movie.
- **No contact** when either object is flagged (bits 3 and 7 of +`$29`), or their +`$0A`
  or +`$06` words are too far apart. The gates test the sign of the difference, not a true
  signed compare, and the C keeps that.
- **Pushed aside:** when y is less than `$60` ahead, x goes `$40` beside y and its car's
  `$0206` follows.
- **Swap and drop:** otherwise the two swap speeds and x drops to `$60` behind y. Each car
  slot recomputes `$0216` with a 40-step shift-and-subtract division, ported step by step.
  x's car loses the drop from `$0225`, and `effect_request_p1($FF24)` fires unless a car
  is flagged 4.
- **Two never-run blocks**, decoded as static code. One is the object-to-slot table
  `$81:FE16` through the CPU divider (x / `$35`), used when `$03DC & 3` is set: objects 0
  and 20 to slot 0, object 1 to `$B3`. The other is the drop held at the object's own
  floor. Only the fuzz reaches them.
- Live 4,000/4,000 (60k) and 133/133 (race1), fuzz 500/500, paths 262/262, first try.

## Window HDMA, text tiles and palettes (roots-52, 2026-11-19)

**`hdma_window_setup` ($8A:A06D):** `hdma_window_tables_init`, then HDMA channels 7 (BG2's
horizontal scroll) and 1 (the fixed colour) from `$81:0D00` and `$0D20`.
- Its `SEP #$10` clears X's and Y's high bytes. The first adapter kept Y whole, and the
  fuzz caught it.

**`text_tiles_put_0` ($8A:BACB):** 97 words from `$8A:C8E4` to VRAM 0 (word writes). It is
the same data `text_screen_init` puts at `$1000`.

**`text_palettes_load` ($8A:C24B):** CGRAM `$20`-`$4F` cleared by direct writes, then two
palettes from bank `$8A` by `upload_cgram_now`.

All three: live 1/1, fuzz 500/500.

## The title Mode 7 setup and the stars twin (roots-51, 2026-11-19)

**`stars_scatter` ($80:F8F8):** the bank-`$80` copy of `stars_scatter_8a`, calling
`rng_next` instead of its bank-`$8A` copy. It shares the same C and adapter.

**`title_mode7_setup` ($80:F424):** sets up the title's Mode 7 screen.
- VRAM words 0-15 are cleared by direct word writes.
- `$7E:2940` is uploaded to VRAM `$3C00`, which becomes BG1's tilemap (`$2107` = `$3C`).
- BG1 scroll is 0/3, and HDMA channels 1 and 3 get new tables in their bank.
- Then the zoom pair (`inverse_scale`, `func_80F745`), with V carried in from the entry.

Both: live 1/1, fuzz 500/500, first try.

## A palette and a text screen (roots-50, 2026-11-19)

**`palette_load_70` ($8A:8DBF):** a palette chosen through `$83:84B1`.
- Colour 0 is written directly (`$2121`/`$2122`, `video_set_color`).
- 16 colours from `$7F` go to CGRAM `$70` by `upload_cgram_now`.
- The closing `PLX` sets N and Z from X. The first adapter missed it, and the fuzz caught it
  with X = 0.
- Live 4/4, fuzz 500/500.

**`text_screen_init` ($8A:C1C6):** a text box of columns 0-`$1F`.
- 97 words from `$8A:C8E4` go to VRAM `$1000` as word writes, a row of `$0502` tiles to
  `$7F:0184`, and the string at `$8A:X` is centred.
- The 128 tiles at `$7F:0080` then lose their palette bits (10-12).
- C and V come from `text_print_centered`'s exit, as its own adapter models them.
- Live 4/4, fuzz 500/500.

**A gap found, not closed (owner's call).** A DMA the harness records raw (not a plain
`$2118` word upload, a fill or CGRAM) keeps its target, mode, source and size, but not
the VRAM address or `VMAIN`. The original's kick to `$2118`/`$2119` consumes the address
it set, so a C that never sets the address would pass. `$8A:8828` (high-byte and
low-byte uploads) needs this, so it waits until the effect carries the VRAM address and
`VMAIN` for those targets. That change to the comparison is a verification-rule change.

## The title HDMA channels (roots-49, 2026-11-19)

**`hdma_title_setup` ($8A:8787):** six HDMA channels set up, but not enabled:
- the BG mode, the main screen, the BG1/BG2 tile addresses and BG1's tilemap, from ROM
  tables;
- BG1's two scrolls, from `$0D00` and `$0D20`. Their 13 bytes each are copied from
  `$8A:8902`/`$890F` by MVN.
- Channels 5 and 7 also get an indirect bank, unused in their mode, which the harness
  compares as written.
- Live 4/4, fuzz 500/500, paths 63/63, first try.

**Left: `$8A:8828`.** It sends to the high byte of VRAM words alone (DMA mode 0 to
`$2119`), and later with `VMAIN` = 0 to the low byte. `src/platform.h` has no call for a
one-register VRAM upload. Adding one changes `platform_bench.c` (a full gate), so it waits
for a unit of its own.

## Stars scattered and a colour HDMA (roots-48, 2026-11-19)

**`stars_scatter_8a` ($8A:971B):** the 128 stars placed.
- The depth starts at `$2000` and grows by `$BE` a star.
- x and y are bits 8-23 of the depth times a signed random byte, on the PPU multiplier.
  The bytes are the low byte of one `rng_next`, then the high byte of the next.
- Its bank-`$80` twin is `$80:F8F8` (roots-51). A byte search missed it because the
  `rng_next` call targets differ.

**`hdma_coldata_setup` ($8A:9C69):** `hdma_table_0d00_init`, then HDMA channel 2 on the
fixed colour (`$2132`, one byte a line) from `$81:0D00`. It does not enable the channel.

Both: live 4/4, fuzz 500/500, reached only on the 60k movie, first try.

## A 16-colour fade (roots-47, 2026-11-19)

**`palette_fade_60` ($80:FAEE):** 16 colours faded by a step and sent to CGRAM `$60`-`$6F`
by DMA.
- The fade amount is the low 5 bits of `$81:D9AC` + 2 * step.
- Each colour's red, green and blue words (`$81:DB4C`, `$DB6C`, `$DB8C`) lose it, clamped
  at 0. They are packed into BGR555 at `$0500` by the original's byte swaps and shifts,
  modelled as written.
- The exit carry is the bit the last blue's second `ASL` shifted out.
- Live 58/58, fuzz 500/500, paths 53/53, first try.

## A curved line table (roots-46, 2026-11-19)

**`window_curve_build` ($80:FA5B):** a two-edge line table at `$1500`, likely a window
for HDMA (to be confirmed when the HDMA side is ported).
- Called from `$80:F687` and `$80:F6E5`.
- The step `$A000 / r` (r = `$0142`) comes from the CPU divider. The C models its `$FFFF`
  for a divisor of 0.
- Each line's edges are `$7F - t` and `$7E + t`, clamped. t comes from the PPU multiplier:
  bits 8-15 of the word at `$81:DA4C`/`$DACC` times the signed r.
- The C sets matrix A and B as the original leaves them. B takes r over the latch, which
  holds the high byte just written, as in `draw_meter`.
- Once the position passes `$8000`, the rest of the lines are `$FF`, `$00`. The table is
  mirrored below, with `$DD` line counts and a 0 end.
- Live 63/63, fuzz 500/500, paths 71/71, first try.

## The title stars drawn (roots-45, 2026-11-19)

**`stars_draw` ($80:F950) and `stars_draw_8a` ($8A:9773):** the title's 128 stars drawn
as sprites, once per frame.
- Each star (6 bytes from `$0F00`) is projected with the CPU's multiplier and divider:
  screen x = `$80` + (x + speed * m) / speed, y = `$5C` + (y - speed * m) / speed. The
  quotient is taken from the magnitude, then added or subtracted.
- The C models the divider's `$FFFF` for a divisor of 0 (the fuzz keeps speed-0 stars).
- A star off screen, or whose depth less the step falls below `$100`, is spawned again
  (`star_spawn`) and redone.
- The bank-`$8A` copy differs only in its tile table (`$86:F46B`) and its own
  `star_spawn` copy, already verified as the same C (`star_spawn_8a`). Both rows share
  one C body.
- The fuzz keeps speeds from `$40` and a small multiplier. A slow star under a large one is
  spawned dozens of times, past the bench's instruction limit (15-20% of samples were
  abandoned at first).
- Live 772/772 (`$80:F950`) and 229/229 (`$8A:9773`, reached only on the 60k movie), fuzz
  500/500, paths 113/113.
- This unit runs the full gate: the full-run mark was 4 commits back.

## Two title steps with the CPU multiplier (roots-44, 2026-11-19)

**The frontier, after the advisor (2026-11-19).**
- The 14 open-command DSP-4 roots stream commands `src/dsp4.c` does not have yet: the
  sprite commands `$03`, `$0B`, `$06`, `$09` and the road commands `$01`, `$07`, `$08`.
- Before any chip-state design:
  1. Port the roots tagged `math`, `ppu`, `dma` or `hdma`, which the harness already
     covers (about 20; `apu` needs its support checked first).
  2. Measure which command each DSP-4 root uses (also `sta.w $8000` under DB = `$30`),
     its real sizes (the catalogue's counts are one byte short for `$0A` and `$11`), and
     its order of reads and writes.
  3. Then a replay tape of the original's chip traffic per sample, instead of a
     snapshot.
  4. The commands themselves come as separate units checked on the firmware with
     refbench, `$0B` and `$03` first.

**`title_zoom_settle` ($80:F303):** a title zoom step after `mode7_zoom_view`.
- `$017C` = (`$017A` - `$E2`) / 2 (signed), at most `$200`; `$017A` loses a quarter of
  that, not below `$100`; `$0186` loses an eighth (at most `$D0`) and one, not below 0.
- `$013C` and `$013A` come from `$80` * `$0187` on the CPU multiplier, computed in C.
- The counter `$0130` moves on, then `inverse_scale` and `func_80F745`.
- Live 87/87, fuzz 500/500, paths 65/65.

**`title_stars_spread` ($80:F247):** the title's 128 stars (6 bytes each from `$0F00`).
- Each star's speed byte times `$40` (CPU multiplier) is added to its +2 word and taken
  from its +0 word.
- The first add has no `CLC`: it takes the entry carry, a parameter of the C. Later adds
  take the clear carry of the index step.
- Then the zoom starts: `inverse_scale` and `func_80F745`. Y ends at `$300`, which the
  first adapter missed.
- Live 1/1, fuzz 500/500, paths 40/40.

## Both players' palette rows (roots-43, 2026-11-19)

- **`track_palettes_load` ($82:A672):** the third and last whole-command DSP-4 root.
  - It unpacks `$88:929A` to `$7F:0000`.
  - The rows start at `$7F` + (`$047B` * `$E0`, the DSP-4 product's low word; the high
    word is read and dropped) for both players, and at `$85:A1EC` + (0, or `$70` when
    `$01E4` is above 0).
  - On a track whose word at `$83:9583` is 0, a player whose flag byte (`$039C`, `$03CB`)
    is set takes the next rows (`$70` on, and `$E0`). Player 2's flag never runs: its 13
    bytes at `$82:A6DF` are now static code.
  - Then 7 rows, `$20` apart: 8 words from `$7F` to `$7E:6A59`/`$6B59`, and 7 words from
    bank `$85` around them (`$6A55`, `$6A57`, `$6A69`-`$6A71`, the same for `$6B..`).
- **Results:** 5 interrupted live entries; replays 2/2, fuzz 500/500, paths 140/140 (and
  on race1).
- **What is left with the DSP-4.** The 14 remaining DSP-4 roots keep a command open
  across calls. They wait for a chip-state design, to be weighed with the advisor before
  any code.

## A track block in three copies (roots-42, 2026-11-19)

- **`track_block_triple` ($82:ADC2):** the second whole-command DSP-4 root.
  - It unpacks `$99:97E4` to `$7F:0000`.
  - The DSP-4 multiplies `$047B` * `$190`. Only the product's low word is used: the
    original writes the chip instead of reading the high word, as `func_82EA49` does.
  - It copies the `$190` bytes at `$7F` + that product to `$7E:217C`, then those to
    `$7E:230C` and `$7E:249C` (`MVN`).
- **Results:** its 5 live entries are interrupted; replays 2/2, fuzz 500/500, paths 29/29
  (and 29/29 on race1).

## The track row words; the DSP-4 roots classified (roots-41, 2026-11-19)

**The DSP-4 roots, classified.** 17 roots touch the chip.
- 14 only write parameters to `$30:8000` (DB = `$30`) with no command word: a command
  opened elsewhere stays open across calls (`$82:D934`, 514,743 entries, is one). They wait
  for the chip-state design.
- 3 send whole commands: `$81:E631` (commands `$00` and `$11`), `$82:A672` and `$82:ADC2`.

**Two more roots stay parked.**
- `$8A:898F`: a never-run path calls `$8A:84B9`, inside 627 unknown bytes of another
  screen's code.
- `$82:F2B3`: its handler table has four never-run targets, one entering another routine
  mid-way (`$82:2C60`).

**`track_rows_build` ($81:E631, `JSL`):** the track's row words at `$7E:D1B1`, rows
`$019A` down to 1, at race start.
- The track table index starts at `$0198` + (the high byte of 4 * rows, less 1) * 10, a
  DSP-4 multiply; it steps back 10 every `$40` rows.
- Per row:
  - the limits are `$1F28`, or `$1F2A` where the table's word at +4 is `$F8F3`;
  - the four edges move toward them;
  - the row's track object clips them, its kind picking `far_7fff_4` or
    `track_entry_bounds` from the table at `$81:7FFE` (every entry but kind 9's, which
    is `$81:0000`);
  - command `$11` (`dsp4_row`, checked against the firmware in dsp4-11) makes the row
    word, and each negative nibble moves up by one. The low nibble's case never runs:
    its 16 bytes at `$81:E797` are now static code.
- At the end, `MVN` copies the first 16 bytes past the last row.

**Verification.**
- Its 5 live entries are interrupted; replays 2/2, fuzz 500/500, paths 189/189 on the 60k
  movie (and 189/189 on race1).
- The first fuzz overwrote each entry's kind byte while filling the entries, so the
  original jumped through garbage handlers and never returned (500 abandoned). The DSP-4
  log showed it: 89 rows where at most 63 can run. The fuzz now fills the entries
  first, then writes valid kinds at their 4-byte stride.
- The fuzz also aims half its runs at a real `$F8F3` marker, the one path random tables
  missed.

## A tile animation that never runs (roots-40, 2026-11-19)

- **`tile_anim_step` ($8A:85AF):** 785 entries in the 60k movie, all with D+`$42` = 0, where
  it does nothing.
  - Mode 1: D+`$40` counts 0-7, and frames `$5A`, `$78` and `$3C` show at 1, 3 and 5.
  - Other modes: D+`$40` counts 0-15, and the frame is 0 or `$1E` by its bit 1.
- **`anim_tiles_put` ($8A:8600):** a 5x3 block of tile words from `$7E:8000` + frame,
  written straight to VRAM at `$0496`, `$0516` and `$0596` (`$2116`/`$2118`, through
  `video_write_vram_word`).
- **Never-run code.** Everything past the mode-0 check (200 bytes, `$8A:85B3`-`$8A:867C`)
  was unknown in the map and is now static code. Its branches past the routine's `RTS`
  stay inside that block, so the row's extent covers it.
- **Exit.** A = the frame's last word with X = the frame; or, in mode 1 with no frame, the
  count with its `CMP #5`'s flags.
- **Full gate.** Four incremental commits had passed since the last full run.

## Player 1's effect request (roots-39, 2026-11-19)

- **`effect_request_p1` ($81:F48B):** `func_83FE9A` for player 1's car, its cooldown at
  `$0223` (D = `$0200`); its C calls that function. 136 entries in the 60k movie.
  - The cooldown never runs in the movies. Its 12 bytes and the `PLA`, `RTS` it branches
    to past the routine's end (`$81:F4AD`, in the unknown region after it) are now
    static code; the row lists that pair as a range it continues into.
  - This was parked as a branch that leaves the routine; the roots-30 recipe (static
    code plus an `alsoIn` range) settles it.
- **Its flags differ from `func_83FE9A`'s on the cooldown path.** The keep exit here is
  `PLA`, `RTS` with no `SEC`, so C and V are the `SBC`'s. The adapter models them from the
  state before.

## The track snap and a money bonus (roots-38, 2026-11-19)

- **`track_snap` ($8A:C2DD, `JSL`):** `$0444` is moved to the first track of the
  variant's list (the lists `track_next` uses) at or above it, judged by the sign of the
  difference (`BEQ`/`BPL`).
  - Each track skipped calls `money_skip_bonus`.
  - In the movies the track is always already in the list, so skipping never runs, and
    neither does the variant-0/`$300` path. 50 bytes (`$8A:C2EF`, `$8A:C320`, `$8A:C342`)
    were unknown and are now static code.
- **`money_skip_bonus` ($8A:C355):** with bit 12 of `$03DC`, both players' money becomes
  player 1's + 75,000, in decimal (low word + `$5000`, high word + 7 and the carry).
  - It looks like a bonus for each race skipped.
  - It never runs, so it has no row of its own: its 36 bytes are static code, and
    `track_snap`'s fuzz runs it, with V modelled as in `prize_money_award` (roots-32).

## Four from the fresh roots list (roots-37, 2026-11-19)

- **`results_text_build` ($8A:9AA5):** `$0154` = 0, then `variant_lines`,
  `player_names_put` and `text_lines_copy`. Its exit is `text_lines_copy`'s, derived from
  the state after: the line count is the variant's, and the lines are read from
  `$8A:9DEE`.
- **`player_in_top5` ($8A:9CB0, `JSL`):** `slot_order_fill`, then whether the player's
  slot is among the first five of `$0D00`, then `clear_041c`.
  - The player's slot is 0; with two players shown it is 0 or 1. Two players means a
    variant other than 0 whose bits 4-5 are not `$10`.
  - It returns the carry: set when the slot is not there.
  - Only the found-first case runs in the movies. The rest (47 bytes at `$8A:9CBC` and
    `$8A:9CCE`) was unknown and is now static code.
  - Y is not compared: it is the count `slot_order_fill` leaves.
- **`title_scroll_start` ($80:F6FD):** `$0198` = 0, `$0196` = `$FF80`, and `$013E` counts up.
- **`save_data_seal` ($80:8199, `JSL`):** `$0366` = (the sum of the `$78` bytes at `$0368`)
  ^ `$7645`. Then `MVN` copies the `$7A` bytes from `$0366` to `$7E:FA3B`. It looks like the
  saved data's checksum.
- **The first gate failed on two adapters of mine.**
  - `title_scroll_start` leaves A = `$FF80` (its `LDA #`).
  - In `results_text_build`, each name `player_names_put` writes counts `$0156` down
    (`func_8A9BAA`), so the text lines left are the variant's count less the names. With
    variant 2 (four lines, four names) none are left, and the exit is the last name's
    `text_line_put`.

## Where to go next (2026-11-19, with the advisor)

The owner is away and reads decisions here.

- **Corrections.** The never-run paths decoded since roots-16 were unknown in the map,
  not data: data stayed at 770,615 bytes all along while unknown fell. roots-34's count
  is 118 bytes, not 126.
- **The order from here:**
  1. Leftover plain roots, from a fresh `roots.py` (14 routines landed since the last
     run, so callers may have become roots): `$8A:C2DD`, `$80:F6FD`, `$80:8199`,
     `$8A:898F`, `$82:F2B3` (an indirect table: a variant switch whose targets get
     decoded), and `$81:F48B` and `$8A:85AF` (their never-run branches leave the routine,
     solved since roots-30 by static code plus an `alsoIn` range).
  2. DSP-4 roots, classified first by the command words they write to `$30:8000`. Port
     the ones that use only commands `src/dsp4.c` has and finish every command before
     returning, as `shake_step`, `func_82EA49` and `track.c` do. The ones that leave a
     command open wait for the chip-state design.
     - The bench's DSP-4 answers the original during a difftest, so a pass shows the C
       matches the bench's chip, not the real one. Each command used needs the refbench
       firmware check (`--dsp4-cmdlog`).
  3. `$82:A7F9` (track load). Its registers may not be compared (check the call site);
     its N and Z come from the final `PLB`; its C and V follow the records, and the last
     decompress length is the final D+`$14` minus the word stored at `$7E:6591,y` before
     it.
  4. Hardware setup (`$80:B7E7`, `$80:D168`): 1-14 entries each, I/O writes and a
     second stack. Last.

## The track objects' graphics stream; a full run (roots-36, 2026-11-19)

- **`track_objects_stream` ($82:AF33):** the track objects' graphics, sent as the car
  (`$0226`) passes their entries.
  - The entries are 5-byte records at `$7E:5B91` + `$01A0`: where it starts (a word), its
    tile slot (a byte), and the object kind (a word).
  - For each record reached, kind 0 sets `$7E:45EC` = 2 * slot. Any other kind picks an
    entry: `$86:AE36` + 2 * kind, or `$86:AC36` when the kind's high byte is set.
  - That entry gives the source (an offset plus the word at `$7E:6591` + its base) and
    the size. The VRAM word is slot * 32, and `$7E:45EC` + 2 * (kind's low byte) is set to
    2 * slot (`$7E:495C` for the kinds with a high byte). Then `upload_vram_now` sends
    the graphics from bank `$7F`, and `$01A0` moves on by 5.
  - The first C wrote the high kinds' slots into `$45EC` too. The replays caught it at
    `$7E:4970` (0/2) before any commit.
  - The fuzz then drew kinds whose entry points below `$8000` in bank `$86` (I/O space,
    which the bench refuses). It now draws only kinds whose entry is empty or in ROM.
  - Its random `$7E:6591` words also made sources that ran past `$7F:FFFF`. The DMA wraps
    within the bank, which the bench counts as two effects, against one from the C. The
    game's data never does that, so the fuzz keeps each source inside the bank. Whether
    the platform layer should wrap like the hardware is a question for the port.
  - A high kind of `$29` or more makes the index run past bank `$86` (into `$87:6C36`).
    The fuzz keeps high kinds below `$29`.
  - **A quirk the fuzz found.** When a kind's entry is 0, the original goes on to the next
    record with X still holding 2 * kind, not the record's index, so `$01A0` becomes
    2 * kind + 5. The game's kinds all have entries, so it never shows in play. The C
    and the adapter do the same.
  - The fuzz tests the quirk with low kinds only (a high kind's 2 * kind is far off).
    Then the quirk record is the last one, with a record at 2 * kind + 5 that ends it,
    so the routine never reads past bank `$86` on garbage. Checked on all three movies
    before the full gate.
  - When the first record is not reached yet, a `JMP` to the `RTS` ends it. Those 3 bytes
    never ran and are now static code.
- **Exit.** The choices read only state the routine does not change, so the adapter
  follows them: Y = the last upload's VRAM word, V = its `ADC`'s, and A, C, N, Z from
  the comparison that ended it.
  - Its live entries are interrupted (long VRAM uploads), so replays and fuzz carry it.
- The note on roots-35: `shake_step` passed with 4 Z mismatches in the fuzz, which do not
  count because no flag of it is read (4,000 returns watched); the Z model there is not
  exact.
- **Full gate.** Four incremental commits had passed since the last full run.

## The shake (roots-35, 2026-11-19)

- **`shake_step` ($82:8F8F):** a shake counter (`$1F18`, offsets `$1F1C`/`$1F20`), each
  race frame, about 18,500 entries.
  - With `$0231` clear it is off (`$8000`).
  - A negative value starts it: 0, or `$30` when the byte `$0397` is at least 4.
  - Below `$98` it runs: offsets -3 and 2, and it moves on by 2 * `$1E9E`. At `$98` and
    up (by its multiple of 8) it becomes `$B0`; crossing `$30` from below makes it jump to
    `$98`.
- **The ending never runs in the movies.** At `$98` and up, both offsets come from DSP-4
  multiplies: (rng / 2) * -10 and (rng / 2) * 10, keeping the products' high words
  (`dsp4_multiply`). The new value is a random pick (0, 2 or 4) of a 3-word table:
  `$82:9139` below `$B0`, else `$82:913F`.
  - The census marked the routine as not touching hardware because this path never ran.
  - The fuzz reaches it, with the bench's DSP-4 answering the original.
- **Exit:** modelled per path. The ending's X (the pick) is recovered from the stored
  word, since the two tables hold no repeats.

## The players' names as text lines (roots-34, 2026-11-19)

- **`player_names_put` ($8A:9AF1):** the players' names as text lines (`text_line_put`,
  bank `$81`), by the variant in the low nibble of `$03DC`:
  - 0: player 1's (`$037E`);
  - 2: four names (`$03E4`, `$03F1`, `$03FE`, `$040B`);
  - 1, by bits 4-5: `$10` alone gives player 1's and the string `PLAYER2` (`$8A:9B7D`);
    otherwise player 1's when bit 4 is set and player 2's (`$03AD`) when bit 5 is;
  - 3-15: nothing.
- **Never-run code found.** Only variant 0 runs in the movies. Variants 1 and 2 are 118
  bytes of code (`$8A:9AFD`, 48, and `$8A:9B37`, 70) that the map had as unknown. They are
  now static code, ported, and run by the fuzz. The 8 bytes after them (`$8A:9B7D`) stay data: the
  string `PLAYER2`.
- **Exit.** The last `text_line_put`'s, followed call by call (A kept, X = the `$0154`
  before it, Y = the string's length, C and V of `$0154` + `$10`). Without a call: the
  `CMP #2`'s flags (variants 3-15) or the last `LSR`'s (variant 1).

## A title zoom step and the slots' order (roots-33, 2026-11-19)

- **`title_zoom_count` ($80:F5BF):** a title zoom step. D+`$30` counts down (and at 0 D+`$3E`
  counts up), then `inverse_scale` and `func_80F745`. Its exit is modelled as
  `title_zoom_cycle`'s (roots-6).
- **`slot_order_fill` ($8A:9CF8):** the slots' order.
  - The 20 slot words at `$7F:7FBC` are cleared, then `func_8A9BE2` (the largest kind)
    and `func_8A99D0` (20 slots) run.
  - Each slot `func_8A9BB5` takes, largest kind first, goes to `$0D00` + Y as its index,
    until none is left.
  - Exit: the last `func_8A9BB5`'s (A = 0, X = D+`$66`, C set), with N and Z of the final
    Y.

## The prize money (roots-32, 2026-11-19)

- **`prize_money_award` ($84:A388, `JSL`):** both players' prize money, in decimal mode
  (`SED` ... `CLD`), eight BCD digits each (low word, then high word).
  - Player 1: place `$03A8`, money `$038E`/`$0390`, bonus bits `$03AB`. Player 2: `$03D7`,
    `$03BD`/`$03BF`, `$03DA`.
  - A player with a place (not negative) gets the pair at `$84:A2B3` + 4 * place.
  - Then the bonus word is shifted right until it is 0 (at least once). Each bit shifted
    out that is set adds the pair at `$84:A2DB` + 4 * bit.
  - The C returns the carry the original leaves: that of the last bonus step, or the
    entry carry when neither player has a place.
- **The decimal add, as the CPU does it.** The bonus table holds `$FFFF` from bit 7 on
  (bonuses the game does not define). A valid-BCD add would differ there, so
  `adc_decimal16` follows the 65816 digit by digit, also for digits above 9, the way the
  bench's CPU does. The fuzz uses all 16 bonus bits.
- **V is modelled too.** Too few returns are watched (4) to show the caller ignores the
  flags, and the first gate failed on V alone (243 fuzz mismatches). `adc_decimal16`
  gives the CPU's decimal V: taken before the last digit's +`$6000` adjustment.

## The next track and the results reset; a full run (roots-31, 2026-11-19)

- **`track_next` ($84:A41B, `JSL`):** the next track in the variant's order.
  - The orders: variant `$100` uses the list at `$8A:CADE`, `$200` the one at `$8A:CB01`,
    and the rest (0 and `$300`) the one at `$8A:CAC4`. That last path never ran (30 bytes
    at `$84:A42D`, now static code).
  - It finds the first byte equal to `$0444` (a 16-bit compare of the byte) and takes the
    one after it.
  - The search has no bound, so the fuzz draws `$0444` from the list itself.
- **`results_init` ($84:A4ED, `JSL`):** the results reset.
  - `$0444` = 0. Three `MVN`s (destination bank `$81`, source `$83`, the order `54 dd ss`
    noted in roots-18) copy `$83:85AB` to `$0368`, `$83:85D1` to `$038E` and `$83:8600` to
    `$03BD`.
  - The ten words at `$0446` become `$0100`, `$0302`, and so on, and `$036C` = 0.
  - With bit 1 of `$03E2` it also sets eight item bytes of both players to 1 (`$0398`-`$039C`,
    `$03C7`-`$03CB`). That never runs in the movies (30 bytes at `$84:A534`, now static
    code).
- **Full gate.** Four incremental commits had passed since the last full run, so this unit
  goes through `verify_all.sh`.

## The player's name and car blocks to $7F:2000 (roots-30, 2026-11-19)

- **`name_copy_7f2000` ($80:E8CE):** the player's name (bytes at `$037E` + D+`$94`, up to
  its zero) to `$7F:2000` + X, then the words `$0100` and `$1400`.
- **`car_blocks_7f2000` ($80:E8F4):** the player's car (the word `$03A8` + D+`$94`) picks a
  block from `$81:D3C9`, then block `$BE` follows (`block_copy_7f2000`, roots-13). With no
  car (negative) it copies block `$A8` alone: a tail jump that never runs.
- **`car_block_7f2000` ($80:E911):** one block from `$81:D3F1` by car; with no car it uses
  car `$13`, which never runs either.
- The never-run paths (9 bytes, `$80:E8FB`, `$80:E918`) are now static code; the fuzz picks
  no car a quarter of the time.
- The first gate failed in the fuzz, for two reasons:
  - the tail jump into `block_copy_7f2000` left the declared extent, so the row now lists
    `$80:E8B5`-`$80:E8CE` as a range it continues into;
  - the adapters kept C as on entry. C is in fact clear after the car's `ASL`, which
    `block_copy_7f2000` leaves alone, except on the no-car tail jump.

## Two results counters and a clear (roots-29, 2026-11-19)

- **The results tables, by variant.** Bits 8-9 of `$03DC` select one: 0 uses `$85:D501`,
  `$100` uses `$85:D530`, anything else `$85:D55F`, each indexed by `$0444`.
  - Variant 0 never runs in the movies. Its paths (`$84:A331`, `$84:A364`, 15 bytes) were
    unknown in the map; now they are static code, and the fuzz picks every variant.
- **`result_slot_step` ($84:A31B, `JSL`):** when the variant's byte is set, `$036C` moves
  on by 4.
- **`result_tally` ($84:A34B, `JSL`):** when the byte is set, the word `$036E` + (the word
  at `$85:D590` + (0, `$1C` or `$44` by variant) + `$036C`) counts up.
- **`clear_041c` ($8A:9D1E):** the 20 words at `$041C` = 0.

## The scene's objects (roots-28, 2026-11-19)

- **`scene_objects_place` ($8A:86AB):** when bit 0 of the word at `$8A:9351` + 8 *
  `$1536` is set, it places eight objects of the scene (otherwise it returns at once,
  with X = 8 * `$1536`).
  - From `$013C` and `$0162` it derives D+`$7E`, `$80`, `$82` (scaled down) and D+`$84`,
    `$86`, `$88` (negated), then calls `func_8A9202` twice and `func_8A92CF` six times
    with D+`$7A` and D+`$7C` set, X their running slot.
  - The original adds D+`$7C` and then D+`$7A` with no `CLC` between, so each D+`$7A`
    takes the carry of its D+`$7C` sum.
  - The last D+`$7C` is a `CLC` then `SBC #$0A`, which subtracts `$0B`; its carry (no
    borrow) goes into the next `ADC`. The C spells both out.
- **Exit:** `func_8A92CF`'s for the last object.

## Player 1's car each frame (roots-27, 2026-11-19)

- **`player_frame_setup` ($83:E2D7):** about 18,500 entries in the 60k movie.
  - It sets the car's settings from bank-`$86` tables, indexed by the bytes `$0393`-`$0397`,
    and `$022B` through `func_83EA9B`. It clears `$1ECA`, `$1EBA`, `$1ECC` and `$1E9C`,
    and copies `$1E66` to `$1F2C`.
  - Then, unless the controls are read (`$0244` bit 2 clear and `$03DC` bit 4 set), it
    runs `player_road_limits` with D = `$0200`.
  - With the controls (`$000C` held, `$000E` pressed, button masks at `$039D`-`$03A5`), it
    sets flag bits in `$1E9C` and `$7E:2142` = `$58` or `$109F`. It steps the item in
    `$014C` down or up to the next one carried (bytes `$0397` + item, up to 4), and sets
    `$025C` = the byte at `$024E` + item and `$1F2E` = the item. The C's `pressed_take`
    is the original's `TRB` from `$000E`.
- **Exit.** Registers are not compared, as for `player_road_limits`' row. The flags are
  modelled:
  - without the controls, N and Z are clear (the `PLD` of `$0100`);
  - with them, V = bit 14 of `$000C` (the last `BIT` of a memory word), C comes from the
    last `CPX #5` or the item's `ASL` (else clear, from the `ADC #0`), and Z from the
    item's byte.

## A second planar copy, the HDMA gradients; a full run (roots-26, 2026-11-19)

- **`tiles_planar_upload_5fe0` ($85:817B):** byte for byte `tiles_planar_upload`
  (`$8A:8CB7`), except its own loop's jump address and the VRAM word, `$5FE0`.
  - The C became one helper with the VRAM word as a parameter, so this unit changes
    existing lines and goes through `verify_all.sh`. The full run was due anyway: four
    incremental commits since the last mark.
- **`hdma_gradients_init` ($8A:8F18, `JSL`):** the HDMA gradient tables.
  - Their headers at `$7E:2C00`: `$FF` and `$7E:2800`, `$FF` and `$7E:2A00`, then 0.
  - Then three `hdma_gradient_build` runs into `$7E:2000` + D+`$6C`: all colours zero
    with patterns 0; colours 0, 0 and the word at `$8A:934B` + 8 * `$1536`; and, with
    patterns 1, the words at `$8A:934B` and `$8A:934D` + 8 * `$1536` and `$0172`.
  - Its exit flags are the last build's, modelled from the state afterwards exactly as
    that row's adapter does; registers are not compared, as for that row.

## The race's band table (roots-25, 2026-11-19)

- **`race_bands_merge` ($85:B1FF):** merges the two players' line lists into one band
  table. The lists are zero-ended and ascending: player 1's at `$7E:4CFE` + `$0164`,
  player 2's at `$7E:4D4A` + `$0168`.
  - Each line goes to `$0486` + Y (Y from 2, or `$34` when `$00B4` is 0), with its flags
    in `$0596` + Y: `$02` player 1's, `$0C` player 2's, `$0E` both, plus `$80` on each
    list's first.
  - The first marks go to `$047E` + X: `$39` when the lists start on the same line,
    otherwise `$19` or `$29`, then `$39`.
  - The list indices (`$0114`, `$0116`) move on by an 8-bit `INC`, so only their low byte
    wraps.
  - The movies only have lists that match line for line, so the code for "a line in one
    list only" and "the lists start apart" never runs. Its 82 bytes (`$85:B232`,
    `$85:B265`) were unknown in the map; they were decoded as static code and ported, and
    the fuzz's lists, which often differ, run them.

## The race positions (roots-24, 2026-11-19)

- **`race_positions_update` ($82:AD2C):** the two players' race positions, every frame of
  a race.
  - Each of the 20 cars (`$35` bytes apart from `$7E:FAB5`; player 1 is car 0, player 2
    car 1) has a 48-bit progress: the words at +0, +2 and +`$1E`, low to high.
  - `$0142` (player 1) and `$0144` (player 2) count the cars whose progress is at least
    the player's, the player included. The original's `CMP`, `SBC`, `SBC` and `BMI` decide
    by the sign of the 48-bit difference.
  - A position that differs from the last one shown (`$1ED2`, `$1ED4`) is stored there,
    and its message `$25` + position is requested (`race_message_request_0233` and
    `_02e6`, roots-11).
  - Exit: the final `PLB` pulls the `$83` its own `PEA $837E` pushed, so DB = `$83`, N is
    set and Z clear whatever the caller's bank.

## The track's backdrop (roots-23, 2026-11-19)

- **`track_backdrop_load` ($82:A1D3):** unpacks `$98:9FEB` to `$7F:0000`.
  - With k = 4 * (the byte at `$84:A66C` + `$047B`): `$1F28` and `$1F2A` = the pair at
    `$85:A33A` + k.
  - With x = 2k, it uploads from bank `$7F`: the graphics (`$85:A346` + x, size
    `$85:A348` + x) to VRAM `$3800`, and the two maps (`$85:A34A` + x and `$85:A34C` + x,
    `$600` bytes each) to `$3000` and `$3400`.
  - Then the first map's `$600` words get bit 13 cleared and `$400` added (another
    palette and tile base), and both maps go again to `$7400` and `$7800`.
  - Its live entries are all interrupted; it rests on replays, fuzz over the tracks, and
    paths.

## Packed pixels to planar tiles (roots-22, 2026-11-19)

- **`tiles_planar_upload` ($8A:8CB7):** graphic X (even) at `$7F:(word $83:84B1 + X + A)`,
  `$83:851B + X` bytes of packed pixels: two 4-bit pixels a byte, high nibble first,
  eight a row, eight rows a tile.
  - Each tile is turned into the SNES's 4-bit planar format in place. Per row, each
    pixel's bit 0 is rotated into plane 0, bit 1 into plane 1 (bytes 0-15, a row's two
    planes together), and bits 2 and 3 into planes 2 and 3 (bytes 16-31). `$0114`-`$0133`
    hold the tile being built.
  - Then the whole graphic goes to VRAM word `$6FE0` (`upload_vram_now`).
  - Exit: A = `$1801` (`upload_vram_now`'s), X = the entry X (`PLX`), Y = `$6FE0`, C and
    V of the `ADC` that found the graphic.
  - Its live entries are all interrupted, so this is the first row the replays cover from
    the start.

## Interrupted entries replayed; nesting cap 8 (2026-11-19)

**Why.** An interrupt landing inside a live sample drops it, because the handler changes
memory the C does not model. Eight verified rows had every sampled entry interrupted
(`clear_vram`: 11 of 11), so their only evidence was fuzz on synthetic inputs. Those
inputs overwrite the very fields that select the work, like the track number.

**How replay works.**
- While a routine has no completed live sample yet, the bench keeps its entry states:
  WRAM, PPU, DMA, the registers and the stack pointer. At most 2 per routine, 64 in all.
- A kept state whose sample an interrupt dropped is run again in the fuzz phase, from
  that exact state, with interrupts off, from the real stack, so the sample ends at the
  real return. It is compared like any sample.
- These cases are counted apart: the report line `replayed N/M` and a `replayed:N`
  token in `map/verified.txt`. They are not live samples, because the original did not
  run them that way in the movie.
- A replay that fails or is lost fails the row.

**Proof that a replay can fail.** The `track_map_load` adapter was made deliberately
wrong (one bit of `$7E:6C40` flipped), run, then put back. The row failed, with the first
failure on a replay: `WRAM $06C40: original 7B, C 7A`.

**The nesting cap.** `MAX_DEPTH` went from 4 to 8: once every early entry is sampled,
more outer samples hold slots. `wait_frames` went back to 3,358 live samples (from
2,629), and `star_layers_init` to 4.

**Result on the eight rows.** All replays passed: 2/2 each for the six with two or more
interrupted entries, 1/1 for the other two. The full run follows.

## Every early entry sampled; a full run (2026-11-19)

**Why.** The bench compared one entry in three (`SAMPLE_STRIDE`). A routine the movies
enter only a few times could end up with no compared live sample, when the sampled
entries were interrupted. Eight verified rows stood at live 0 that way (roots-21 note).

**The change (`difftest.c`).** The first `SAMPLE_ALL_FIRST` (32) entries of every routine
are now all sampled, and one in three after that. Rare routines get every entry compared;
frequent ones get at most 32 more samples.

**This needs a full run.** It is a harness change, so `verify_all.sh` re-ran every row
and wrote a new full-run mark. Three incremental commits had passed since the last mark.

**Measured result.**
- Live samples went from 318,655 to 319,770; 225 rows gained.
- It did not fix the live-0 rows: still 8 (`func_8ABA55`, `func_8AB994`,
  `star_layers_init`, `password_pack`, `hdma_gradient_build`, `track_map_load`,
  `track_segments_load`, `clear_vram`). Their counters show every entry was interrupted
  (11 of 11 for `clear_vram`, 12 of 12 for `hdma_gradient_build`). The cause is the
  interrupt, not the sampling.
- 19 rows lost some samples, mostly leaves called inside other samples (`wait_frames`
  3,337 to 2,629, `binary_to_bcd` 2,448 to 1,863). The likely cause is the nesting cap
  `MAX_DEPTH` (4): more outer samples now hold the slots.

**Next (with the next full run).** Raise the nesting cap. Compare one case per captured
entry state with the fuzz function skipped, so the original runs from the real entry
state without an interrupt; report those as "replayed", not live.

**Two tool fixes, committed after it.**
- `verify_all.sh` stamps the full-run mark only when the run passed.
- `progress.py` counts any uncommitted tracked change as "a commit is being made" (it
  looked only at `src/` and `tools/bench/`). A docs-only commit that is the sixth after
  the full run could otherwise pass locally and fail in CI.

## A wait, the variant flags, six sprite slots (roots-21, 2026-11-19)

- **`wait_03e0` ($84:A4CD, `JSL`):** waits until the word `$03E0` is 0 (the interrupts
  count it down). A row that waits for interrupts: no fuzz.
- **`variant_flags_apply` ($8A:B0E1):** bits 2-5 of `$1528` become bit 10 of `$03DC`,
  moved to bit 2; bits 2-5 of `$1526` become bits 8-9 of `$03DC`, moved to bits 2-3
  (`lsr_n`).
  - Exit: A = the new `$1526`, Y = 0, C of `lsr_n`'s last shift.
- **`sprite_slots_six` ($8A:C19C):** six sprite slots. `$0800` = the word at `$8A:C443`
  | `$2000`, `$0600` = `$0142` (which moves on by `$10`), `$0700` = `$0144`, and `$0500`
  = 1.
- **A note on roots-20:** `track_map_load` and `track_segments_load` passed with **no
  live sample** (live 0/0).
  - The bench samples one entry in three (`SAMPLE_STRIDE`), and both sampled entries were
    interrupted inside the long `decompress`, so they were dropped.
  - Their verification rests on the 500 fuzz cases (from the live entry states) and full
    path coverage. That meets the bench's rule; 6 earlier rows are in the same state
    (`clear_vram`, `password_pack`, ...).
  - Sampling every entry of rarely-entered routines needs a harness change, which comes
    with the next full run.

## The track's map and segments (roots-20, 2026-11-19)

- **`track_map_load` ($85:9D7B, `JSL`):** the map at `$7E:6C33`.
  - On a track whose word at `$85:A439` + `$047B` has bit 15 set, with the byte `$039C`
    set: `$820` words of `$3FFF`. Exit: A = `$3FFF`, X = `$1040`, C and Z set by the
    `CPX`.
  - Otherwise `decompress` unpacks `$8C:95E3` into it, and the exit is `decompress`'s.
  - The C returns the length and the source's end, so the adapter models that exit
    exactly as `call_decompress` does.
- **`track_segments_load` ($82:A954):** the track's segments.
  - It unpacks `$9A:8000` to `$7F:0000`, then `MVN` copies the track's part (start
    `$85:9FAA`, length `$85:A008`, by 2 * `$047B`) to `$7E:4F31`.
  - Both words of each 4-byte entry then become running sums.
  - Every length in `$85:A008` is at least 4 (checked over the 47 entries), so the fuzz
    can pick any track. A length of 0 would make the `MVN` copy 64 KB.

## Four more leaves, with the selftest in parallel (roots-19, 2026-11-19)

The gate now runs `verify_new.sh` and the selftest at the same time. Both must pass,
each checked by its own `wait`.

- **`queue_7f_init` ($80:EBCE):** a queue reset (D = `$0100` or `$0200`).
  - D+`$CC`, D+`$C6` and D+`$D1` = 0; D+`$E5` = `$80`; D+`$E1` = `$40`.
  - Its list at `$7F:(D+$5E)` becomes 28 words `$FFFF`, then `$FFFE`.
- **`text_map_clear` ($8A:9BF6, `JSL`):** `$0160`, `$0114` and `$0158` = 0, and the text
  map words `$7F:7918`-`$7A59` and `$7F:7FBC`-`$7FE3` = 0.
- **`variant_offsets_add` ($8A:98B4, `JSL`):** after `variant_5a`, for `$015A` entries
  (`$0A`, or 4 with variant 2):
  - the word at `$041C` + 2 * (the byte at `$0446` + entry) gains the entry's word of
    `$8A:9D8D`;
  - variant 2 uses `$8A:9DB5` instead. That path never runs, so its 12 bytes at
    `$8A:98D3` are now static code.
- **`palettes_load_8a` ($8A:81BA):** five palettes into CGRAM through `upload_cgram_now`,
  from bank `$7E`: `$81E2`, `$8202`, `$8222`, `$8114` (`$1E` bytes), and the one the
  table at `$8A:934F` + 8 * `$1536` points at.
  - Its first gate failed in the fuzz: a random `$1536` sent the table read past bank
    `$8A` into the DMA registers, which the bench refuses. The fuzz keeps the table's 16
    entries; live 2/2.

## The full-run mark (2026-11-18)

The first incremental unit's CI failed in `tools/test_carry.sh`. The rule had refused
correctly. The test assumed every row in its clone starts current, but by then 330 rows
were carried, and five more commits took them past the window.

The advisor also pointed out a gap: a changed line in a file with no rows of its own (a
header, `bench.c`, the Makefile) staled nothing. That was true before the incremental rule
too, because the hash never covered those files.

**The fixes.**
- `verify_all.sh` writes `# full run: <UTC time>` as the second line of
  `map/verified.txt`.
- `progress.py --check` finds that line's commit with `git blame`. It refuses when that
  commit is more than 5 commits back (counting the one being made), or when `src/` or
  `tools/bench/` changed since by more than additions. A changed line anywhere there now
  needs a full run.
- The README shows the mark's time. A commit hash would only exist after the commit, so
  the block generated before it would already be stale.
- Carried rows take the oldest recent commit whose code their hash matches, and the
  additions check covers all of `src/` and `tools/bench/` from there.
- `verify_new.sh` refuses up front when `git diff HEAD` has a removed or changed line
  there, or any change to `difftest.c` or `platform_bench.c`.
- `merge_verified.py --update` keeps the mark.
- `test_carry.sh`:
  - its setup is now a full run (rows current, mark committed);
  - it has a new case: a changed header line is refused;
  - 10 cases pass.

## Four leaves verified incrementally (roots-18, 2026-11-18)

The first unit gated by `tools/verify_new.sh`: the new rows over the 60k and shop movies,
the selftest, and the older rows carried from the full run at `6f18db6`.

- **`tile_block_5x5` ($82:AD99):** a 5x5 block of tile numbers, counting up from A (kept
  in `$0116`), written column by column into the 32-wide map at `$7F:0130` (rows `$40`
  bytes apart).
  - It always ends with A = X = `$0A`, Y = 0, C set and V clear (the `SBC #$13E`), N
    clear and Z set (the last `DEC`).
- **`track_settings_load` ($85:831C, `JSL`):** the track's settings, by `$047B`.
  - `$0198` and `$019A` come from `$84:A590` + 4 * track, and `$019C` from `$84:FF59`.
  - `$1F06`, `$01E4`, `$01E6` and `$1F34` come from tables in bank `$83`.
  - `$01E4` uses `$83:95F1` instead of `$83:9583` when the byte `$039C` is set; `$01E6`
    does the same for `$03CB`. The `$03CB` path never runs, so its seven bytes at
    `$85:8368` are now static code.
- **`rows_7f_copy8` ($86:CE4A, `JSL`):** eight `MVN` copies of `$38` bytes within bank
  `$7F`, the destination rows `$40` apart.
  - Exit: A = `$FFFF`; N and Z from the `PLB`; C and V of the last `ADC #8`.
- **`block_c4_build` ($80:C31A):** the block at [D+`$C4`] in low WRAM.
  - `MVN` copies `$60` bytes from `$9B:8000` and `$20` from `$80:B2D2` into it (bank
    `$81`), then six words are set at +`$102` and +`$122`.
  - `MVN` is encoded `54 dd ss`, destination bank first, so the listing's `mvn $81,$9B`
    reads destination, source. The first C read it the other way round, and the gate
    caught it (0/2 live, 0/500 fuzz) before any commit.
  - The original reads D+`$C4` again after the copies, so the fuzz keeps the block clear
    of it and of the stack.

## Incremental verification (owner, 2026-11-18)

The owner approved it on 2026-11-18 ("a 1 e 3 funciona", confirmed later the same day). A
full `verify_all.sh` re-runs every routine (about 20 minutes with the selftest), though
most units only add routines.

**The rule (`tools/progress.py`).**
- A verified row is *current* when its hash matches the code now.
- It is *carried* when its hash matches the code at one of the last 5 commits (counting
  the one being made), and since then:
  - the row's source and `tools/bench/routines.c` only gained lines (no `-` line in
    `git diff -U0`);
  - `difftest.c` and `platform_bench.c` did not change at all.
- Any other row is stale, and `--check` (CI) fails.

**Where the rule shows.**
- The README counts carried rows and names the oldest run's commit.
- CI checks out the full history (`fetch-depth: 0`).
- CI runs `tools/test_carry.sh`. In a scratch clone, without the ROM, it checks that:
  - additions are carried;
  - a changed registry line is refused;
  - an addition to `difftest.c` is refused;
  - the sixth commit since the run is refused.

**Per unit.**
- `tools/verify_new.sh name1,...` runs the new rows over the 60k and shop movies. It then
  merges them with `merge_verified.py --update`: same entry replaced, new ones appended.
- Every unit still runs the selftest (every routine the short movie reaches, 193 now), the
  byte-exact asm rebuild, the personal-data grep and CI.
- Anything else runs `verify_all.sh`, which the rule itself forces at least every 6th
  commit.

**Known gap.**
- A line inserted inside an existing function counts as an addition but changes
  behaviour.
- The selftest catches it for routines the short movie reaches; the next full run (at
  most 5 commits on) catches the rest.
- A unit that edits inside an existing function should run the full verification anyway.

**Also:** `progress.py` now caches the hashes per source. A check took 2 minutes (the
pure-Python hash over `routines.c`, once per row) and now takes about 1.5 seconds.

## A counted pickup, the section seeks, two row copies (roots-17, 2026-11-18)

- **`track_object_count` ($83:FB15):** another track-object handler, like
  `track_object_money`. On a hit, unless the low byte of D+`$4E` is already 10, it:
  - counts D+`$4E` up (a 16-bit `INC`) and clears the entry;
  - requests effect `$FF19` and sets D+`$4A` = `$801E` and message `$4C`.

  When the count is full it leaves `func_83FC05`'s V with N clear and Z and C set (its
  `CMP #$0A`); the other exits go through the callee adapters (`calleeRun`).
  Live 3/3, fuzz 500/500, paths 20/20.
- **`race_section_seek_1` ($85:9D39) and `race_section_seek_2` ($85:9D5A):** while
  player 1's position `$023E` (player 2's `$02F1`) is at least `$40` past the word at
  `$84:(D+$9C + D+$A2)` (D+`$A6`), the offset moves on by `$14`, one entry.
  - Exit: A = the last difference; X = D+`$9C` + the offset; C clear (both exits are
    `BCC`); N and Z of the `SBC` or of the `CMP #$40`; V of the `SBC`.
  - On race1 (the selftest) the position never lies past an entry, so the fuzz left the
    move-on path unrun. Half the time it now puts the position just past the current
    entry: paths 16/16 on both movies.
- **`row_copy_6a33` ($85:82D8) and `row_copy_6b33` ($85:82FA):** 16 words from
  `$7F:(A + the word at $83:84B1 + X)` (DB = `$83`) to `$7E:6A33` or `$7E:6B33`.
  - Exit: X kept (N and Z of its `PLX`), Y = `$20`, C set, V of the `ADC`.
  - Paths 20/20 each.

## Three variant settings and a text copy (roots-16, 2026-11-18)

- **The variant.** The low nibble of `$03DC` holds a random 0-2, set by `$8A:9F76`.
  Three routines in bank `$8A` set a value by it, and variant 2 gives the second value:
  - `variant_lines` ($8A:9AB1): `$0156` = `$14`, or 4;
  - `variant_5a` ($8A:98F0): `$015A` = `$0A`, or 4;
  - `variant_66` ($8A:99BE): `$0166` = `$28` (its tail `func_8A99D0`, in the row's
    extent), or 8.
- **Variant 2 never runs in the movies.** Its six-byte paths (`LDA #`, `STA`, `RTS`)
  were unknown in the map. They are now static code: `8A:9AC3`, `8A:98FC` and `8A:99CA`.
  The fuzz picks variant 2 a third of the time.
- **Exit.** A = the value (N and Z clear), C of the `CMP #2`, V as on entry.
- **`text_lines_copy` ($8A:9AC9):** `$0156` lines of text from `$8A:9DEE` (zero-ended
  strings, one after another) into the text map at `$7F:7918` + `$0154`, each with its
  zero; `$0154` moves on by 16 a line.
  - Exit: A = the new `$0154` (C and V of its `ADC`), X = the last line's start + its
    length, Y = past the last zero, N clear and Z set by the last `DEC`. With no lines,
    A = 0.
- **Results:** live 2/2 each (one screen of the 60k movie); fuzz 500/500; paths 10/10,
  10/10, 7/7 and 20/20.

## A car's byte $53 and two more frame waits (roots-15, 2026-11-18)

- **`car_set_53_1` ($83:FD26) and `car_set_53_2` ($83:FD31):** the byte dp+`$53` = 1 or
  2, then their shared tail at `$83:FD45` (in both rows' extents): dp+`$B1` = track
  object `$011A`'s bytes at `$7E:8CB3` (low) and `$8CB5` (high).
  - Exit: A = that word (N Z of its `ORA`), X = `$011A`, C and V as on entry.
  - `car_clear_53` ($83:FD3C, roots-13) is the third sibling, without the tail.
  - Live 7/7 each, fuzz 500/500, paths 5/5.
- **`wait_frames_8a8bb6` ($8A:8BB6) and `wait_frames_8a9880` ($8A:9880):** two more
  long-call copies of `wait_frames`, which makes five in all.
  - They share `wait_frames`'s C and adapter.
  - Live 266/266 and 211/211; no fuzz, because they wait for interrupts.

## Three track-object handlers (roots-14, 2026-11-18)

`$83:F7F9` calls a handler per track object through its table at `$83:7FFE`. The object
is entry `$011A` of `$7E:8CB3`, of kind `$011C`, and D = `$0200`.

- **`track_object_test` ($83:FA34):** `$012A` = 0 and `$0128` = the entry's x (a signed
  12-bit value). Then it runs the hit test `func_83FD8A` at that x, or `func_83FDCD`
  when bits 4-5 of the byte at `$8CB5` are set (a tail at `$83:FA63`, in the row's
  extent).
- **`track_object_bump` ($83:FA6A):** when x - D+`$06` lies within the kind's limits
  (judged by sign, as `func_83FC05` does) and D+`$1A` is 0, it sets D+`$7E` = 8 and
  D+`$1A` = 6 and requests effect `$FF1D`.
- **`track_object_money` ($83:FAD7):** on a hit (`func_83FC05`), it clears the entry,
  adds 4000 (decimal mode, BCD) to the money of car `$1ECC` (`$038E` and `$0390`, with
  the carry), requests effect `$FF19`, and sets D+`$4A` = `$801E` and message `$47` in
  D+`$33`.
  - The C adds valid BCD only; the game's money is always BCD, and so is the fuzz's.
- **Exit flags.** The caller reloads A, X and Y and sets every flag before reading them,
  but fewer than 100 returns are watched (81, 4 and 44), so the bench counts every flag
  as live and the exit flags are modelled.
  - They are mostly the last callee's. A hook, `calleeRun`, makes the adapters of
    `func_83FE9A`, `func_83FC05`, `func_83FD8A` and `func_83FDCD` run the whole outer
    routine in place of their own, so the verified flag model of the callee is used as
    is.
  - The test's adapter sets `$012A` and `$0128` for the callee adapter's read, then puts
    them back before the C runs, so the C makes its own writes.
  - The four callees were retested with the hook: unchanged.
- **Results:** live 81/81, 4/4 and 44/44; fuzz 500/500 each; paths 19/19, 45/45 and 24/24.

## Three small ones (roots-13, 2026-11-18)

- **`car_clear_53` ($83:FD3C):** the byte dp+`$53` = 0 (the car's direct page). Exit: A's
  low byte 0, so N clear and Z set. Live 19/19. Its siblings `$83:FD26` and `$83:FD31` set
  1 and 2 and jump to `$83:FD45`, not yet in C.
- **`block_copy_7f2000` ($80:E8B5, DB = `$81`):** a block from `$81:D419` + Y copied to
  `$7F:2000` + X.
  - The block starts with its length; only the word's low byte counts (an 8-bit `DEC` of
    `$0114`; 0 = 256).
  - The fuzz's random Y carried the DB-relative reads past `$81:FFFF` into bank `$82`,
    whose low addresses are WRAM. The C read ROM, so the C now reads through the bus,
    and the fuzz keeps Y within the bank.
  - Live 6/6, fuzz 500/500, paths 13/13.
- **`text_line_push` ($8A:99E2):** line X / 2 of the text map at `$7F:7918` (16 bytes a
  line) goes to the byte buffer (`byte_buffer_push`) up to its zero.
  - Exit: X kept; C is the last `ASL` of the line offset (bit 13 of X), which a live
    sample showed `byte_buffer_push` leaves alone.
  - Live 27/27, paths 17/17.

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
