// SPDX-License-Identifier: GPL-3.0-or-later
// Two per-scanline tables in the HDMA format (a line count byte, then a word), double
// buffered: the routine writes at the back-buffer offset ($0134) and then swaps it with
// the front one ($0136). Built from inverse_scale's result ($0138) and $013A/$013C/$0132;
// probably the split between sky and road (not confirmed until the HDMA side is ported).
#ifndef TG3000_LINE_TABLES_H
#define TG3000_LINE_TABLES_H

#include <stdint.h>

#define LINES_A 0x0D00
#define LINES_B 0x0D40

// Build both tables into the back buffer and swap buffers. Returns the old back offset.
uint16_t func_80F745(void);

#endif
