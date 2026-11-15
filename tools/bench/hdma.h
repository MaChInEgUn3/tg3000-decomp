// SPDX-License-Identifier: GPL-3.0-or-later
// HDMA measurement (--hdma-report FILE): what the game sets up for HDMA and who does it.
// A measuring instrument only: it records, it does not take part in the differential test.
#ifndef BENCH_HDMA_H
#define BENCH_HDMA_H

#include <stdbool.h>
#include <stdint.h>

#include "snes.h"

void hdma_start(Snes* snes);
void hdma_on_insn(Cpu* cpu);                             // from the CPU trace hook
void hdma_on_access(uint32_t adr, uint8_t val, bool write); // from the CPU access hook
void hdma_on_frame(int frame);                           // after each emulated frame
bool hdma_save(const char* path);

#endif
