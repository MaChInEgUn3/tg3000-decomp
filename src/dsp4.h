// SPDX-License-Identifier: GPL-3.0-or-later
// The DSP-4 coprocessor's commands as native functions. The game talks to the chip
// through $30:8000 (command word, parameters, results); a port calls these instead.
// Each one is checked against the chip's real program (the reference bench with the
// firmware, tools/refbench) and, inside ported routines, against the bench's
// differential test.
#ifndef TG3000_DSP4_H
#define TG3000_DSP4_H

#include <stdbool.h>
#include <stdint.h>

// Command $0000: the signed 16 x 16 -> 32-bit product (results read low word first).
int32_t dsp4_multiply(int16_t a, int16_t b);

// Command $000A: four edges between two rows. Each row word holds four signed 4-bit
// widths (bits 15-12, 11-8, 7-4, 3-0), each worth 48; edge j goes from row `near`'s width
// j (t = 0) to row `far`'s (t = $8000), at t = `t` (the floor of the linear step).
// Measured on the firmware over the TAS (tools/refbench --dsp4-cmdlog 0A): t is a
// multiple of $80 below $8000, widths 0 and 2 are -7..-1 and widths 1 and 3 are 0..7 in
// every run the game makes; outside that the formula is the natural extension, not a
// measured one (docs/DECISIONS.md, "DSP-4 command $0A").
void dsp4_edges(uint16_t t, uint16_t near, uint16_t far, int16_t out[4]);

// Command $0011, the inverse of $000A: a row word from four edges, each edge to a signed
// 4-bit width, floor(edge * 1364 / 65536) (about edge / 48), edge 0 in bits 15-12 ... edge 3
// in bits 3-0. First measured on the TAS (90 distinct inputs), which could not tell the
// rounding apart; settled on chosen inputs to the firmware chip (tools/bench/dsp4_probe,
// docs/DECISIONS.md dsp4-11a): exact over the whole 16-bit range.
uint16_t dsp4_row(const int16_t edge[4]);

// Command $000B, a sprite entry (and command $0003, which takes one word, $0005 in every
// run, then the first entry's $000B and the entry): three words in (x, y, an attribute
// word). Out: a flag; when it is 1 (the sprite is shown), the low bytes of x and y as one
// word (x in bits 7-0, y in bits 15-8) and the attribute unchanged; when it is 0 (hidden),
// nothing more (the game reads only the flag). `out` gets the two words; the function
// returns the flag.
// Measured on the firmware: over the whole TAS (refbench --dsp4-cmdlog 03 and 0B: 117,650
// and 117,664 runs, the mini-map's two cars, y 32-67, all shown, 0 differ), and on chosen
// inputs (tools/bench/dsp4_probe, docs/DECISIONS.md dsp4-03b/03c). Hidden, by y alone:
// below -7, and rows 233 and 235-511 of every 512. Not modelled, the chip's state: it hides
// the 34th entry of a list on one row and after (the game sends two a list), and rows 233,
// 235, 236 and 238 depend on its history (the C gives the fresh chip's answer).
uint16_t dsp4_sprite(uint16_t x, uint16_t y, uint16_t attr, uint16_t out[2]);

// The chip as the game talks to it (port request A, dsp4-chip-1): a byte stream, the command
// word first (low byte, then 0), then its parameters, then its answer, read low byte first. A
// port puts it behind dsp4_port_write16/read16 (two bytes each). In the multiply's answer a
// write skips a byte, as on the chip. The commands it has: $00, $0A, $11, $03 and $0B. Any
// other one (the road and sprite streams, $01/$06/$07/$08/$09/$0F, not done yet) leaves it
// lost: it answers 0 and takes no more commands until reset. Idle, it reads $FF.
typedef struct {
  uint8_t phase;      // 0, 1: the command word's bytes; 2: its parameters; 3: its answer; 4: lost
  uint8_t cmd;
  uint8_t count;      // bytes taken in this phase
  uint8_t in[10];
  uint8_t out[8];
  uint8_t inSize, outSize;
  bool flagEnds;
  bool roadBit;       // bit 7 of the last $0A's near row: the road command reads it (dsp4-01b/c)
} Dsp4Chip;

void dsp4_chip_reset(Dsp4Chip* chip);
void dsp4_chip_write(Dsp4Chip* chip, uint8_t value);
uint8_t dsp4_chip_read(Dsp4Chip* chip);
// Whether it met a command it does not have (or an access its commands do not allow).
bool dsp4_chip_lost(const Dsp4Chip* chip);

#endif
