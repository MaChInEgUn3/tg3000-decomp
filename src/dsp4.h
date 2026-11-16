// SPDX-License-Identifier: GPL-3.0-or-later
// The DSP-4 coprocessor's commands as native functions. The game talks to the chip
// through $30:8000 (command word, parameters, results); a port calls these instead.
// Each one is checked against the chip's real program (the reference bench with the
// firmware, tools/refbench) and, inside ported routines, against the bench's
// differential test.
#ifndef TG3000_DSP4_H
#define TG3000_DSP4_H

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

#endif
