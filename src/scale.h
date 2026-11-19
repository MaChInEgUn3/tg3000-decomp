// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef TG3000_SCALE_H
#define TG3000_SCALE_H

#include <stdbool.h>
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

// $80:F226: a zoom step: mode7_zoom_view; dp+$40 counts frames, and from $320 on dp+$7C
// falls by $10 a frame (to 0); then inverse_scale and func_80F745, whose result is
// returned.
uint16_t mode7_zoom_step(uint16_t dp);

// Steps of the title sequence's zoom (D = $0100); each ends with inverse_scale and
// func_80F745, whose result it returns (unless it stops early):
// - title_zoom_pan ($80:F2A0): D+$30 counts up to $0F. Once the high bytes of D+$3C and
//   D+$3A are $C0 and $1C, D+$3E counts on and that is all (*done). Otherwise D+$3C moves
//   by D+$40, which turns to -$800 at $F000 and to $800 below $1000, and D+$3A falls $80;
// - title_zoom_cycle ($80:F2EE): D+$30 counts on, from $1F back to $0F;
// - title_zoom_drop ($80:F595): D+$7E falls by 1 to -$48 (there D+$3E counts on), and two
//   rows of 12 tiles at $7F:0214/$7F:0254 are set to $1001/$1002.
uint16_t title_zoom_pan(bool* done);
uint16_t title_zoom_cycle(void);
uint16_t title_zoom_drop(void);

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

// $80:F5BF: a title zoom step: D+$30 counts down (at 0, D+$3E counts up), then inverse_scale
// and func_80F745, whose result it returns.
uint16_t title_zoom_count(void);

#endif
