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

// sprite_text, then the new entries are moved so the string is centred on `x`: each
// entry's x byte gets (x - end) / 2 added (low bytes, 16-bit halving), which is also left
// in $0114 (the glyph base sprite_text used). Returns that shift.
uint16_t sprite_text_centered(uint32_t str, uint16_t x);

// Lay out the 8 hex digits of `value` as sprites from `x` (entries as in sprite_text,
// glyphs $34-$43), leading zeros skipped (zero shows one digit), with one extra pixel
// after the 2nd and the 5th digit position from the left (thousands gaps). The digits are
// read through take_nibble ($0118:$011A). Returns the final x.
uint16_t sprite_number(uint32_t value, uint16_t x);

// Add the sprites of a group to the entries at $0900+slot (slot at D+$74): the table at
// `table` holds two lists of 6-byte items (tile, x, y), each ended by a zero word; items
// of the first list get $0B02 = 1, those of the second $0B02 = 0. Each entry's tile is
// item tile + D+$18, its x byte item x + D+$16 (the high byte of $0900), its y item y
// + D+$14 with the high byte reduced to a sign ($0B00 keeps it, $0900's low byte the low
// byte). The entries are written through the table's bank (the original sets DB to it).
// `dp` is the direct page ($0100 or $0200). Returns the address of the second list's
// terminating zero word.
uint32_t sprite_group(uint16_t dp, uint32_t table);

// sprite_number, then shift its sprites by (x - end) on the low bytes, so that the number
// ends at `x` (right-aligned). D+$16 = the value's low word, D+$14 = the shift. Returns the
// shift (16 bits).
uint16_t sprite_number_right(uint32_t value, uint16_t x);

// The same, centred on `x`: the shift is ((x & $FF) - (end & $FF)) >> 1 (16-bit).
// ($80:8635)
uint16_t sprite_number_centered(uint32_t value, uint16_t x);

// Five lines of tile text from $8A:C44F, C458, C46D, C484, C49B at (12, 6), (6, 22),
// (5, 23), (5, 24), (6, 26), with D+$90 = 1, 0, 0, 0, 1. Returns the last
// text_print_tiles_at's result.
uint16_t func_8AC0F2(void);

// A label and a number as sprites: the string at $80:9E3A at x = $5C5C + D+$E7 and the
// 32-bit number $0390:$038E (+ D+$94) at x = $5C94 + D+$E7, with the sprite base D+$14 =
// $3C00; when bit 1 of D+$76 is set and bit 2 clear, the string at $80:9E42 and base
// $3E00 instead. Returns sprite_number's result.
uint16_t func_809DD0(void);

// The sprite group at $81:CBB8 placed from D+$22, D+$24, D+$7C, D+$7E and D+$E8 (high
// bytes of the first two signed): D+$14 = ((D+$7C - hi(D+$22)) & $FF) - $100, D+$16 =
// ((rotl16(swap(D+$7E)) + $24) & $FF) + D+$E8 - hi(D+$24), D+$18 = $F000. Returns
// sprite_group's result.
uint32_t func_80EB66(uint16_t dp);

#endif
