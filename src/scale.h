// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef TG3000_SCALE_H
#define TG3000_SCALE_H

#include <stdint.h>

// The routine works on direct-page variables; `dp` is the direct page it runs with.
#define SCALE_DIVISOR 0x7A   // input (word)
#define SCALE_QUOTIENT 0x14  // scratch: the 8-bit quotient (word)
#define SCALE_RESULT 0x38    // output (word)

// result = 42 * quotient, where quotient = 8 bits of 65536 / divisor (restoring division
// that starts from $200 and keeps the 16-bit overflow of the original). A divisor of
// exactly $100 short-cuts to $2A00. Returns the result.
uint16_t inverse_scale(uint16_t dp);

#define ZOOM_SCALE 0x7A     // input (word): Mode 7 scale, 8.8
#define ZOOM_CENTRE_Y 0x3B  // input (byte)
#define ZOOM_CENTRE_X 0x3D  // input (byte)

// Mode 7 without rotation: matrix A = D = the scale, and the offset that puts the centre
// (x, y) at the screen point ($80, $6F): ($80 - x, $6F - y) as 16-bit values. Returns the
// vertical offset (the original leaves its high byte in A's low byte).
uint16_t mode7_zoom_view(uint16_t dp);

#define VIEW_MATRIX 0x14   // input: Mode 7 matrix A, B, C, D (4 words)
#define VIEW_SCROLL 0x3A   // input (word): BG2 horizontal scroll
#define VIEW_LINE_A 0x4A   // input (word): copied to $0D21, $0D24, $0D27
#define VIEW_LINE_B 0x4C   // input (word): copied to $0D01, $0D04, $0D07
#define VIEW_BG2_Y 0x62    // input (word): BG2 vertical position
#define VIEW_SCENE 0x1536  // the scene index (absolute): 8-byte entries at $8A:9351

// Mode 7 view with rotation: the matrix from D+$14..$1B, centre ($80, $40), BG2 scrolled
// by D+$3A; patches entries of the line tables at $0D00 (D+$4C) and $0D20 (D+$4A), and
// $0D2A = 2 * (scroll + $200). BG2's vertical scroll depends on bit 2 of the scene's
// entry: clear, it is $80; set, it follows D+$62. In both cases $0D0A = D+$62 unless
// D+$62 + $40 is negative (D+$62 between $7FC0 and $FFBF), and then a set bit also leaves
// the vertical scroll alone. Returns D+$62.
uint16_t mode7_view(uint16_t dp);

#endif
