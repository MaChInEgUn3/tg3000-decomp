// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef TG3000_DSP4_CHIP_H
#define TG3000_DSP4_CHIP_H

#include <stdbool.h>
#include <stdint.h>

#include "dsp4.h"

// The chip as the game talks to it, built from src/dsp4.c's commands (dsp4-chip-1): a byte
// stream, the command word first (low byte, then 0), then its parameters, then its answer,
// read low byte first. A bench instrument only (tools/bench/dsp4_compare scores src/dsp4.c
// with it): a model of the chip's interface has no place in a port (docs/DECISIONS.md, Scope),
// where the game's C calls src/dsp4.c's functions directly. In the multiply's answer a
// write skips a byte, as on the chip. The commands it has: $00, $0A, $11, $03, $0B and the
// road, $01 (dsp4-01p: its header, then rows until the word $8000). Any other one (the
// sprite streams, $06/$07/$08/$09/$0F, and the road's objects and edges, not done yet)
// leaves it lost: it answers 0 and takes no more commands until reset. Idle, it reads $FF.
typedef struct {
  uint8_t phase;      // 0, 1: the command word's bytes; 2: its parameters; 3: its answer; 4: lost
  uint8_t cmd;
  uint8_t count;      // bytes taken in this phase
  uint8_t in[10];
  uint8_t out[8];
  uint8_t inSize, outSize;
  bool flagEnds;
  int16_t edge2, edge3;  // the last $0A's last two edges ($8000 at power-on): the road's first rounding
  // the road ($01): its header and row words as they come, its state, the row's answer
  uint8_t roadIn[30];
  uint16_t roadLen;     // the bytes of the answer
  bool roadFirst;       // the next row is the first
  Dsp4Road road;
  uint8_t roadOut[10 + 6 * 512];
} Dsp4Chip;

void dsp4_chip_reset(Dsp4Chip* chip);
void dsp4_chip_write(Dsp4Chip* chip, uint8_t value);
uint8_t dsp4_chip_read(Dsp4Chip* chip);
// Whether it met a command it does not have (or an access its commands do not allow).
bool dsp4_chip_lost(const Dsp4Chip* chip);


#endif
