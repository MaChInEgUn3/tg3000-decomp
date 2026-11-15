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

#endif
