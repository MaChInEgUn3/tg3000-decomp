// SPDX-License-Identifier: GPL-3.0-or-later
// Text drawing into the tilemap buffer at $7F:0000 (32 columns of words; each character
// is two tiles high: the top at +$0000, the bottom at +$0040 of its cell).
#ifndef TG3000_TEXT_H
#define TG3000_TEXT_H

#include <stdint.h>

#define TEXT_CURSOR_X (DP + 0x3A)  // column
#define TEXT_CURSOR_Y (DP + 0x3C)  // row (in 2-tile rows: the map offset uses row * 64)
#define TEXT_FONT_SEL 0x0D7C       // 0: the alternate font tables

// Draw character `ch` (ASCII) at the cursor and advance the cursor by one column.
// Returns the map offset it drew at.
uint16_t text_put_char(uint8_t ch);

// Map an ASCII character to a glyph index: 'A'-'Z' -> 0-25, ' ' -> $25, '.' -> $24,
// '!' -> $26, anything else below 'A' -> code - $16 (so '0'-'9' -> $1A-$23).
uint16_t char_to_tile_index(uint16_t ch);

// Print the zero-terminated string at bus address `str` centred between columns $013E and
// $0140 (on the current row), or as many spaces when $0168 is non-zero (erasing it).
// Stores the length in $015A; afterwards, when $0186 is 0, calls func_8AB700.
void text_print_centered(uint32_t str);

// Draw `ch` as a single tile (glyph index from char_to_tile_index, attribute $3880, or
// $3C80 when $0190 is set) at the cursor and advance it. Returns the tile written.
uint16_t text_put_tile_char(uint8_t ch);

// Move the text cursor down three rows.
void text_cursor_down3(void);

// Lay out the zero-terminated string at bus address `str` as sprites with a
// proportional font: one 4-byte entry per glyph from slot $0174 (x word at $0900, tile word
// at $0902 = glyph + base $0114, word $0B01 = 0), x starting at `x` and advancing by the
// glyph's width (8-bit, table $80:85E5); a space advances 6, bytes with bit 7 set are
// skipped; a character neither alphanumeric nor in the punctuation list uses its own
// code as the glyph. Leaves the next slot in $0174 and returns the final x (also in $0116).
uint16_t sprite_text(uint32_t str, uint16_t x);

// Print the zero-terminated string at bus address `str` (the original takes its low word
// in A, also left in $0148, and the bank from DB) one tile per character with
// text_put_tile_char; $80 moves the cursor to column 0 of the next row. Returns the
// string's length (the original's Y).
uint16_t text_print_tiles(uint32_t str);

// text_print_tiles starting at (`column`, `row`).
uint16_t text_print_tiles_at(uint32_t str, uint16_t column, uint16_t row);

#endif
