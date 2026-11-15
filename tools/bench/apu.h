// SPDX-License-Identifier: GPL-3.0-or-later
// APU port measurement (--apu-report FILE): who reads and writes $2140-$2143, how often
// per frame, and with which values. A measuring instrument; not part of the difftest.
#ifndef BENCH_APU_H
#define BENCH_APU_H

#include <stdbool.h>
#include <stdint.h>

#include "snes.h"

void apu_start(Snes* snes);
void apu_on_insn(Cpu* cpu);
void apu_on_access(uint32_t adr, uint8_t val, bool write);
void apu_on_frame(int frame);
bool apu_save(const char* path);

#endif
