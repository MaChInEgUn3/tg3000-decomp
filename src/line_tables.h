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

// $80:FA5B: a curved two-edge line table at $1500 (D = $0100; r = the byte D+$42). The step
// q = $A000 / r (the CPU divider: $FFFF for 0) is kept in D+$17; a position p (D+$14, from
// 0) moves on by q a line. For lines $B8 down to 2 (2 bytes each), while p is below $8000:
// t = bits 8-15 of the word $81:DA4C/$DACC + (p >> 8) times the signed r (the PPU
// multiplier, left with that word and r); the line's bytes at $1501 + y are $7F - t (0 if
// negative) and $7E + t (at most $FF). Once p reaches $8000 the remaining lines are $FF,
// $00. The lines are then mirrored below ($1503-$15B8 reversed to $15BC-$1671), with $DD
// line counts at $1500 and $15BB and a 0 end at $1674.
void window_curve_build(void);

#endif
