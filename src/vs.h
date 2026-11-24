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
// $8A:A9F2: a panel's cursor home, on column 0 for a player and a random one (0-3) for the
// computer ($0D38 not $FFFF). Returns rng's carry for the computer, true for a player.
bool vs_panel_reset(uint16_t x);
// $8A:AE46: the panel's text cleared: 14 rows of 15 tiles $3D at $7F:(D+$70), its base on.
void vs_panel_text_clear(uint16_t x);
// $8A:A768: vs_all_ready, and with all four panels ready ($0DDC-$0DE2 = $FFFD), the slower
// fade out ($0196 = $FF80).
void vs_ready_check(void);
// $8A:A8E5: the panel's name's letters (A-Z) counted in D+$46; with none, its default name
// (the pointer at $8A:A814) copied in (from D+$44 to D+$42). Returns the count.
uint16_t vs_name_default(uint16_t x);
// $8A:ACFF: the cursor's width for the letter D+$4E ($8A:B014's byte) and the text's shift
// D+$50: 9 for 's' and 'e', 8 for 'd', else 0.
void vs_letter_width(uint16_t x);
// $8A:A7DB: the panels in play from $03DC bits 4-7 (one a panel): $0DDC and $0DF4 $FFFF for
// one in, 0 for one out.
void vs_players_in(void);
// $8A:A922: the four default names ($8A:CD7C, CD87, CD92, CD9F) copied to the names at
// $03E4, $03F1, $03FE, $040B, their lengths in $0DD4-$0DDA.
void vs_names_load(void);
// $8A:AC73: the letter `c` put at the end of the panel's name (its length $0DD4), and the
// panel's split-screen byte $11 (BG2 off). (A panel other than 0-6 writes panel 0's name.)
void vs_name_put(uint16_t x, uint8_t c);
// $8A:B730: one character of the VS screen's text at (D+$3A, D+$3C): as a sprite while D+$88
// is set (x stepped by its width at $8A:B7AC, also into D+$8A; a space steps 7), else as a
// tile in the map at $7F (x stepped by one). Its glyph is $8A:CB31's word for c - $20.
// Returns the original's A: D+$8A for a sprite, the tile for the map.
uint16_t vs_put_char(uint8_t c);
// $8A:B721: vs_put_char as a sprite at (x, y).
uint16_t vs_put_sprite_char(uint16_t x, uint16_t y, uint8_t c);

#endif
