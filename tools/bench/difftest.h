// SPDX-License-Identifier: GPL-3.0-or-later
// Per-routine differential test: the original routine runs first (recorded from entry to
// return), then the C version runs from the same entry state, and WRAM, registers and
// flags must match. See docs/DECISIONS.md ("Differential test").
#ifndef BENCH_DIFFTEST_H
#define BENCH_DIFFTEST_H

#include <stdbool.h>
#include <stdint.h>

#include "snes.h"

typedef struct {
  uint16_t a, x, y, d;
  uint8_t db;
  bool n, v, z, c, m8, x8, e;
  uint16_t sp;  // at entry (an interrupt handler's adapter reads the pushed P through it)
} Regs;

// A ported routine: where the original lives and how to call the C for it.
typedef struct {
  const char* name;
  uint32_t entry;          // CPU address, $80-$FF mirror
  uint32_t end;            // extent [entry, end): instruction starts in it must all be covered
  bool rtl;                // returns with RTL (else RTS)
  bool rti;                // an interrupt handler entered at its vector target: returns with
                           // RTI to the pushed PC (its reads of $4210-$4212, the interrupt
                           // acknowledges, are free)
  uint8_t rtiFrame;        // for a handler's tail: bytes its prologue pushed above the
                           // interrupt frame (D, Y, X, A, DB = 9 for the common NMI prologue)
  uint32_t alsoIn[2][2];   // [start, end) ranges the routine may continue into at its own
                           // stack depth: a tail jump into another (verified) routine
  uint32_t endAt;          // a handler's prologue: the sample ends when it reaches this
                           // instruction (its dispatch, an indirect jump to the target, which
                           // has its own row); D and DB are compared there
  const char* source;      // src/ file holding the C, hashed into the results
  bool needM16, needX16;   // entry contract: register widths the C assumes (else either)
  // Run the C from `in` (g_ram already holds the entry WRAM); fill the outputs in `out`
  // (start: a copy of `in`).
  void (*call)(const Regs* in, Regs* out);
  bool compareA, compareX, compareY;  // which registers are outputs (flags: by liveness)
  bool lowWramDb;          // absolute ($0000-$1FFF) accesses through DB must hit low WRAM
  bool fixedD;             // the C assumes D = DP ($0100)
  uint8_t db;              // the C assumes this data bank (0 = any)
  // Fuzz: given registers already randomised and D/DB/widths from a real entry, make the
  // inputs valid for this routine (WRAM through `ram`, registers through `r`). NULL = no fuzz.
  void (*fuzz)(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
  // The routine waits for interrupts: the ones that land inside a sample are recorded
  // (their WRAM changes) and replayed when the C calls platform_wait_vblank(), instead
  // of discarding the sample. Their hardware accesses belong to the handler.
  bool waitsForInterrupts;
  // Data inside the extent (jump tables): [start, end) ranges skipped when listing its
  // instruction starts.
  uint32_t holes[4][2];
  // Addresses where the linear decode must switch to 8-bit A (code reached by a branch
  // from an 8-bit section after a REP that the sweep would otherwise carry over).
  uint32_t m8At[2];
} Routine;

// Hardware effects, in canonical form: what the original did through DMA and what the C
// asked the platform for (tools/bench/platform_bench.c) must be the same list.
enum { EFFECT_VRAM = 1, EFFECT_CGRAM = 2, EFFECT_RAW_DMA = 3, EFFECT_VRAM_FILL = 4, EFFECT_VRAM_WORD = 5, EFFECT_APU = 6,
       EFFECT_CGRAM_WORD = 7, EFFECT_OAM = 8, EFFECT_STREAM = 9 };
void difftest_effect(int kind, uint32_t target, uint32_t src, uint32_t size);
void difftest_replay_interrupt(void);

// Inputs: the values the original read from an input register ($2140-$2143, $213F, the
// joypad registers), served
// to the C in order (the first unread value of that register). Returns 0 and fails the
// sample if the original read no more of it.
uint8_t difftest_input(uint16_t reg);
// The n-th value (from 0) the original read from `reg` in the sample being compared, for
// adapters that model registers left by input reads (-1 if there is none).
int difftest_input_peek(uint16_t reg, int n);

// PPU fields (Mode 7 matrix and offsets, BG1/BG2 scroll) set by the C, compared by end state.
enum { PF_M7A, PF_M7B, PF_M7C, PF_M7D, PF_M7X, PF_M7Y, PF_M7H, PF_M7V, PF_BG1H, PF_BG1V, PF_BG2H, PF_BG2V, PF_BG3H, PF_BG3V, PF_FIXR, PF_FIXG, PF_FIXB, PF_COUNT };
void difftest_ppu_field(int field, uint16_t value);

// Registers whose state is the last byte written (layer enables, windows, colour math,
// interrupt enable and timers): compared by which were written and their last values.
void difftest_reg(uint16_t adr, uint8_t value);  // platform_wait_vblank() on the bench

// DMA channels as slots (tools/bench/platform_bench.c): the C claims the channel registers
// a routine leaves set up (after the channel's last kick: HDMA setups, streams), and kicks
// streams set up before the routine (a kick that uses registers the routine did not write).
void difftest_dma_claim(int slot, int reg, uint8_t value);  // reg 0-4 or 7 of $43x0-$43x7
void difftest_stream_size(int slot, uint32_t size);
void difftest_stream_kick(uint8_t mask);
void difftest_cgram_address(uint8_t index);

void difftest_init(Snes* snes, const uint8_t* rom, bool negative);
void difftest_on_insn(Cpu* cpu);              // from the CPU trace hook
void difftest_on_access(uint32_t adr, uint8_t val, bool write);  // from the CPU access hook
void difftest_fuzz(int iterations);          // at the end of a run
bool difftest_report(const char* resultsPath);  // prints a summary; false if any routine failed

#endif
