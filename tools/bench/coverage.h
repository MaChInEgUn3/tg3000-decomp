// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef BENCH_COVERAGE_H
#define BENCH_COVERAGE_H

#include <stdbool.h>
#include <stdint.h>

#include "cpu.h"

// Per-ROM-byte flags in the coverage file (traces/coverage.bin, OR-merged across runs).
enum {
  COV_START = 0x01,   // an instruction started here
  COV_CODE = 0x02,    // part of an executed instruction (opcode or operand)
  COV_READ = 0x04,    // read by the CPU or DMA (data, unless also code)
  COV_M8X8 = 0x08,    // register widths seen at an instruction start
  COV_M8X16 = 0x10,
  COV_M16X8 = 0x20,
  COV_M16X16 = 0x40,
  COV_EMU = 0x80,     // executed in emulation mode
};

void coverage_start(Cpu* cpu, const uint8_t* rom);  // rom: the 1 MB image as loaded
void coverage_on_insn(Cpu* cpu);                     // from the bench's CPU trace hook
bool coverage_merge_save(const char* path);         // also writes <path>.ram
void coverage_report(void);
int coverage_insn_length(uint8_t opcode, bool m8, bool x8);
uint8_t cart_peekRom(uint32_t offset);

#endif
