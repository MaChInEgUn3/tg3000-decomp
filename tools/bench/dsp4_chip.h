// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef TG3000_DSP4_CHIP_H
#define TG3000_DSP4_CHIP_H

#include <stdbool.h>
#include <stdint.h>

#include "dsp4.h"

// The chip as the game talks to it, built from src/dsp4.c's commands (dsp4-chip-1): a byte
// stream, the command word first (low byte, then 0), then its parameters, then its answer,
// read low byte first. A bench instrument only (tools/bench/dsp4_compare scores src/dsp4.c
// with it, the bench's bridge answers the game with it): a model of the chip's interface has
// no place in a port (docs/DECISIONS.md, Scope), where the game's C calls src/dsp4.c's
// functions directly. In the multiply's answer a write skips a byte, as on the chip.
// The commands it has: $00, $0A, $11, $03, $0B, and the streams: the road $01 (dsp4-01p: 15
// header words, then rows until the word $8000), the second road $07 (11 header words, rows of
// 6) and the windows $08 (45 header words and one answer word, rows of 9, after $8000 one more
// word). Any other one (the sprite streams, $06/$09/$0F, the road's edge $8001) leaves it lost:
// it answers 0 and takes no more commands until reset (or dsp4_chip_abort). Idle, it reads $FF.
// It follows the sprite list's writes all the same (dsp4-01w: `follow`), for the rounding.
//
// The road's rounding is the chip's own (edge2, edge3), not src/dsp4.c's global: each native
// call runs with the chip's copy swapped into the global and the global put back after. Only
// $0A and the roads change the copy.

#define DSP4_CHIP_LINES 512                        // the lines (or window pairs) kept per row
#define DSP4_CHIP_OUT_MAX (4 + 8 * DSP4_CHIP_LINES)  // the longest answer: an $08 row

typedef struct {
  uint8_t phase;      // 0, 1: the command word's bytes; 2: its parameters; 3: its answer; 4: lost;
                      // 5: a stream's header; 6: a stream row's words; 7: a stream's answer
  uint8_t cmd;
  uint16_t count;     // bytes taken (or given) in this phase
  uint8_t in[10];
  uint8_t out[8];
  uint8_t inSize, outSize;
  bool flagEnds;
  int16_t edge2, edge3;  // the chip's rounding: the last $0A's last two edges, $8000 at power-on
                         // and after a road's first row that draws
  // the streams ($01, $07, $08): the header's or row's words as they come, the answer
  uint8_t streamIn[90];
  uint16_t streamLen;   // the bytes of the answer
  bool roadFirst;       // ($01) the next row is the first
  bool streamEnd;       // the answer due is the command's last ($08 after $8000)
  bool outside;         // the header is outside what dsp4.c measured: rows are not computed
  unsigned long unmodelled;  // answers the native code does not model (dsp4_chip_unmodelled)
  Dsp4Road road;
  Dsp4Window window;
  // dsp4-01w: the sprite list followed from the game's writes alone (its answers are the other
  // chip's), on the chip's own Dsp4Sprites, swapped in and out of src/dsp4.c's as the rounding
  // is: a $0003 opens it; at its stream's end the road's second rounding word becomes lines
  // 127/128's budget left (dsp4-01v). Kept by dsp4_chip_abort.
  struct {
    uint8_t state, kind, got, need, lists;
    bool half;
    uint8_t lo;
    uint16_t args[9];
    Dsp4Sprites list;
  } follow;
  uint8_t streamOut[DSP4_CHIP_OUT_MAX];
} Dsp4Chip;

void dsp4_chip_reset(Dsp4Chip* chip);
void dsp4_chip_write(Dsp4Chip* chip, uint8_t value);
uint8_t dsp4_chip_read(Dsp4Chip* chip);
// Whether it met a command it does not have (or an access its commands do not allow).
bool dsp4_chip_lost(const Dsp4Chip* chip);

// For a bench feeding it alongside another chip: whether it is between commands; the bytes of
// the answer still due (0: none); and drop the command it is in (back to idle, not lost),
// keeping its rounding and its unmodelled count.
bool dsp4_chip_idle(const Dsp4Chip* chip);
unsigned dsp4_chip_pending(const Dsp4Chip* chip);
void dsp4_chip_abort(Dsp4Chip* chip);
// dsp4_chip_abort, then a command word whose low byte was already written (dsp4-01w: the list
// followed sees each byte once)
void dsp4_chip_start(Dsp4Chip* chip, uint8_t lo, uint8_t hi);

// How many answers so far the native code does not model (it gives one anyway): a $01 or $07
// row whose segment (the previous row's line less this one's) is 88 lines or more; an $08 row
// whose two lines differ (row words 2 and 4); a row of more than DSP4_CHIP_LINES lines (the
// count word then says DSP4_CHIP_LINES); every row of a header outside what is measured ($01
// and $07: word 1 below word 2; $08: word 38 or 40 below word 24 or 26: there dsp4.c can divide
// by 0, so those rows are not computed and the road and rounding stay as they were; this does
// not guard lines beyond 16 bits). Also counted, and the chip lost: a row starting with $8001 in $01 or $07 (an edge).
unsigned long dsp4_chip_unmodelled(const Dsp4Chip* chip);

#endif
