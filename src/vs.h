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
// $8A:B072: the string at $8A:`str` put with vs_put_char (D+$8A cleared first). Returns the
// original's A.
uint16_t vs_put_string(uint16_t str);
// $8A:AB6B, AB84, AB9D, ABB6: the panel's two marks, 'I' and 'T', as sprites (panel 0-3).
uint16_t vs_panel_marks(uint16_t panel);
// $8A:A721: as vs_panel_screens, the byte $12 (BG1 off).
void vs_panel_screens_bg1_off(uint16_t x);
// $8A:AEB9: the panel's cursor one letter back along its row, past spaces; at column 0 it
// stays where it was.
void vs_letter_prev(uint16_t x);
// $8A:AF31: the panel's row start one back, past spaces and $3C; at 0 it stays.
void vs_row_prev(uint16_t x);
// $8A:ABEF: a press (B/Y/A/X, $0DCC) on the panel's grid: its letter added to the name ('s'
// a space, 'd' deleting the last, 'e' ending it: the name closed with 0, and once the cursor
// has arrived, the panel goes on to the car's choice: $0D38 = $0DDC, $0DDC = $FFFE). A full
// name (12) or an empty one to delete takes BG1 off the panel instead. The press is taken:
// $0DCC becomes its press bits inverted.
void vs_name_key(uint16_t x);
// $8A:AD74: a panel's name entry for a frame: a player ($0DDC = $FFFF) moves the cursor with
// Up/Down (letters) and Left/Right (rows) once it has arrived; the computer, after a delay
// ($0DE4: 16-48 frames), presses the next letter of its name ($8A:A80C's pointer, index
// $0DDC), and at its end goes to END (4, 4) and closes the name.
void vs_name_entry_frame(uint16_t x);
// $8A:AA71: the car choice's title (the string at $8A:CE5B) at the panel's corner ($8A:AA92,
// $8A:AA9A). Returns vs_put_string's A.
uint16_t vs_car_title(uint16_t x);
// $8A:B037: the letter grid (five rows at $8A:AFAB, 14 bytes apart) from (x, y), a row every
// two lines. Returns vs_put_string's A.
uint16_t vs_grid_draw(uint16_t x, uint16_t y);
// $8A:AAC1: the panel's name drawn as sprites (at the tables $8A:ACD4/$8A:ACDC, 8 pixels a
// step), its frame sprite and its marks. Returns the marks' A.
uint16_t vs_panel_name_draw(uint16_t x);
// $8A:A852: the car's choice confirmed once the cursor has arrived: by a press for a player
// ($0D38 = $FFFF), at once for the computer. The panel's text is cleared, an empty name gets
// its default, the panel is ready ($0DDC = $FFFD), the car's column goes to $0418 + panel
// (a byte), and vs_ready_check runs.
void vs_car_confirm(uint16_t x);
// $8A:A869: a ready panel's text: its name centred and READY ($8A:A9A1) below, blinking with
// D+$66 (cleared on the other frames).
void vs_ready_draw(uint16_t x);
// $8A:A9A7: when the panel's car column ($0DB4) changes (kept in $0DAC), its text is redrawn:
// the title, then the car's description (the pointer at $8A:CE63 + 2 * column: 0 a new
// line, $FF the end). Returns the original's A.
uint16_t vs_car_info(uint16_t x);
// $8A:A81C: a panel's frame, by its state $0DDC: the car's choice ($FFFE: the cursor along
// the row, its column, the car's description, the confirmation, its sprites), ready ($FFFD:
// vs_ready_draw), else the name entry (its name, the entry, the cursor on its letter, its
// sprites, the press, the caret).
void vs_panel_frame(uint16_t x);
// $8A:B9AF: the VS screen's graphics: tiles from $8D:F696 to VRAM $4000, the panels' map from
// $8A:C86C (unpacked to $7F:0000): VRAM $7000 filled with tile $23 (1,024 words), then 28
// rows of 32: 14 rows of two copies of 16 tiles (the second with palette bit $0400), then
// the same 14 with $0800 and $0C00.
void vs_graphics_load(void);
// The VS screen ($8A:A573) in parts between its waits:
// - $8A:A576: HDMA off ($420C/$420D 0), D+$8E cleared;
// - $8A:A582: its setup: palettes, graphics, colour 0, BG1's scroll, mode 1, the map cleared,
//   the four panels' cursors and corners, the four grids, the names, the players in, the
//   main screen ($13), D+$8E set and the fade in started;
// - $8A:A682: a frame: the players' pads into their panels ($0DCC-$0DD2, for the players in),
//   the sprites cleared, the four panels' frames, the pads back; returns true while the
//   screen goes on ($019A, the fade), else the absent players' names ($8A:A6F6), the main
//   screen and HDMA back ($82).
void vs_screen_blank(void);
void vs_screen_setup(void);
bool vs_screen_frame(void);
// $8A:A6F6: the names of the panels not in play ($0DF4 = 0) set to their defaults (from the
// pointer at $8A:A80C to the one at $8A:A804).
void vs_names_absent(void);
// The VS screen whole ($8A:A573).
void vs_screen(void);
void vs_screen_loop(void);
// $82:9B62: the players' cars ahead of the record at D (X its index 1-4); *a, *carry and
// *overflow are the last subtraction's, as the original leaves them.
uint16_t vs_cars_ahead(uint16_t dp, uint16_t x, uint16_t* a, bool* carry, bool* overflow);
// $81:F68B: the four players' cars linked in a ring ($7E:FACD + car).
void vs_cars_ring(void);
// $82:9AF4: each player's place (vs_cars_ahead); a changed place requests its message.
void vs_places_update(void);
// $81:F6AE / $81:F6F2: two cars of the VS race's order (X, Y, record offsets) close in distance
// or sideways; A and the flags as the original's RTS leaves them (X and Y are restored).
typedef struct {
  uint16_t a;
  bool n, v, z, c;
} VsCarsExit;
VsCarsExit vs_cars_tie(uint16_t x, uint16_t y);
VsCarsExit vs_cars_apart(uint16_t x, uint16_t y);
// $82:97BE: the four cars linked both ways in the order at $1E6A-$1E70 (previous at
// $7E:FACF + car, next at $7E:FACD + car). Returns the last car, as the original leaves X.
uint16_t vs_cars_link(void);
// $83:E878: the car at D's engine and tyre words from the VS tables, by D+$14 (as
// player_frame_setup takes the one player's from its parts). Returns the tyre table's index,
// as the original leaves X.
uint16_t vs_car_parts(uint16_t dp);
// $82:BD0E: the VS race's four cars' records set for the start (D+$14-$19 its counters).
void vs_grid_setup(uint16_t dp);
// $82:BDDA: minus `a` (signed) over the step count $1E9E, on the CPU's divider (the quotient of
// the magnitude; $FFFF for a zero count). `negative` is the caller's N flag (its load of `a`):
// the original branches on it, not on `a` itself.
uint16_t vs_step_share(uint16_t a, bool negative);
// $82:BDA9: the four words $01CE, $01D2, $01D6, $01DA each saved to its car's record (+$29)
// and replaced by vs_step_share of it. Returns the last.
uint16_t vs_steps_share(void);
// $83:E597, $83:E64D, $83:E706, $83:E7BF: player_frame_setup for the VS race's players 1-4
// (D = $0100): the car's settings, then its controls or its road limits.
void vs_player1_frame_setup(void);
void vs_player2_frame_setup(void);
void vs_player3_frame_setup(void);
void vs_player4_frame_setup(void);
// $83:E28C: player1_frame for the VS race's four cars (RTL): the frame counted at $7E:4CDD +
// $1E9E * 2, then for each player its frame setup, player_car_frame and car_track_step.
void vs_cars_frame(void);

#endif
