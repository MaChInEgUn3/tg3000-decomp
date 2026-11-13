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
} Regs;

// A ported routine: where the original lives and how to call the C for it.
typedef struct {
  const char* name;
  uint32_t entry;          // CPU address, $80-$FF mirror
  uint32_t end;            // extent [entry, end): instruction starts in it must all be covered
  bool rtl;                // returns with RTL (else RTS)
  const char* source;      // src/ file holding the C, hashed into the results
  bool needM16, needX16;   // entry contract: register widths the C assumes (else either)
  // Run the C from `in` (g_ram already holds the entry WRAM); fill the outputs in `out`
  // (start: a copy of `in`).
  void (*call)(const Regs* in, Regs* out);
  bool compareA, compareX, compareY, compareC;  // which outputs are part of the contract
  bool lowWramDb;          // absolute ($0000-$1FFF) accesses through DB must hit low WRAM
  // Fuzz: given registers already randomised and D/DB/widths from a real entry, make the
  // inputs valid for this routine (WRAM through `ram`, registers through `r`). NULL = no fuzz.
  void (*fuzz)(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
} Routine;

void difftest_init(Snes* snes, const uint8_t* rom, bool negative);
void difftest_on_insn(Cpu* cpu);              // from the CPU trace hook
void difftest_fuzz(int iterations);          // at the end of a run
bool difftest_report(const char* resultsPath);  // prints a summary; false if any routine failed

#endif
