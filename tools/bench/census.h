// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef BENCH_CENSUS_H
#define BENCH_CENSUS_H

#include <stdbool.h>
#include <stdint.h>

#include "snes.h"

void census_start(Snes* snes, const uint8_t* rom);
void census_on_insn(Cpu* cpu);                  // from the bench's CPU trace hook
void census_on_access(uint32_t adr, bool write); // from the CPU access hook
bool census_save(const char* path);

#endif
