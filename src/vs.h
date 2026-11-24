// SPDX-License-Identifier: GPL-3.0-or-later
// VS MODE's screen ($8A:A573-$8A:B0xx): four panels, one a player (X: 0, 2, 4, 6), each with
// its name entry and then its car's choice.
#ifndef TG3000_VS_H
#define TG3000_VS_H

#include <stdbool.h>
#include <stdint.h>

// A panel's cursor, as on the password screen: $0D88 (x) and $0D90 (y) where it goes, $0D80
// and $0D68 where it is, $0D78 and $0D70 its width going and now. $0DB4 is its letter's
// column on the grid's row, $0DAC the row's first letter (7 a row, in the grid at $8A:AFF1).

// $8A:A7CD: the screen's 64 colours from $8A:C5D8.
void vs_palette_load(void);
// $8A:AD58: false (C clear) once the panel's cursor has arrived where it goes.
bool vs_cursor_moving(uint16_t x);
// $8A:ABCF: the panel's frame sprite (slot $F8 + X): from the tables at $8A:ACD4 and $8A:ACDC
// (8 pixels a step), x from D+$8A.
void vs_panel_sprite(uint16_t x);
// $8A:AA58: the panel's cursor back on its first letter (the tables at $8A:AA82, $8A:AA8A).
void vs_cursor_home(uint16_t x);
// $8A:ACE4: the panel's letter (7 a row): its index, also in D+$4E.
uint16_t vs_letter_index(uint16_t x);
// $8A:A72F: the panel's split-screen byte ($13) into D+$FB (panels 0 and 4) or D+$FC (2, 6).
void vs_panel_screens(uint16_t x);
// $8A:AEDA: the panel's cursor one letter on along its row, past the spaces ($20); a $3C
// steps the row's start back. At the row's end ($0DB4 = 4) it stays where it was.
void vs_letter_next(uint16_t x);
// $8A:AA0E: the panel's cursor on its letter's column (16 pixels a column).
void vs_cursor_column(uint16_t x);
// $8A:AAA2: the panel's caret ($05F8 + X): hidden when $0DDC is $FFFE, else blinking with
// D+$66.
void vs_caret_blink(uint16_t x);
// $8A:ACB2: the panel's name cleared, once ($0DEC set): $0DD4 and the six words at the
// table $8A:A804's pointer.
void vs_name_clear(uint16_t x);
// $8A:AA23: a panel's cursor stepped along its row once it has arrived (while $0D38 is
// $FFFF): Right ($0DCC bit 8) up to column 3, Left (bit 9) down to 0.
void vs_cursor_step(uint16_t x);
// $8A:AE20: the panel's cursor on the letter `c` (the 3-byte table at $8A:AF57: letter, row
// start, column).
void vs_letter_find(uint16_t x, uint8_t c);
// $8A:AD34: where the panel's cursor goes: its letter's column and row (16 pixels each) from
// the panel's corner ($0DBC, $0DC4), x less $80 plus D+$50.
void vs_cursor_target(uint16_t x);
// $8A:A795: the players in ($0DF4 = $FFFF) counted in D+$42, those of them ready ($0DDC =
// $FFFD) in D+$44; when all are ready, the screen fades out ($0198 = 0, $0196 = $FFC0).
void vs_all_ready(void);
// $8A:A728: as vs_panel_screens, the byte $11 (BG2 off).
void vs_panel_screens_bg2_off(uint16_t x);
// $8A:AE6E: the panel's text offset in D+$70 (0, $22, $3C0, $3E2 for panels 0-6).
void vs_panel_text_base(uint16_t x);
// $8A:A97F: the text cursor's column D+$3A moved right by 6 less half the length of the
// string at $8A:`str` (D+$48 that half).
void vs_text_center(uint16_t str);
// $8A:AF08: the panel's row start ($0DAC) one on, past spaces and $3C, up to 6 (then it
// stays where it was).
void vs_row_next(uint16_t x);

#endif
