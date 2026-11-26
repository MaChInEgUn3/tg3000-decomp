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
// The menus' copy ($80:8704): their own font (capitals, digits, three marks; widths from
// $80:87B8), large sprites.
uint16_t sprite_text_menu(uint16_t dp, uint32_t str, uint16_t x);

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
uint16_t sprite_number(uint16_t dp, uint32_t value, uint16_t x);

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
uint16_t sprite_number_right(uint16_t dp, uint32_t value, uint16_t x);

// The same, centred on `x`: the shift is ((x & $FF) - (end & $FF)) >> 1 (16-bit).
// ($80:8635)
uint16_t sprite_number_centered(uint16_t dp, uint32_t value, uint16_t x);

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

// $80:B59D: the typewriter, one frame. Its speed D+$E5 adds to D+$C6; each whole $100
// spent prints one more step of the text at [D+$CC] (24-bit) into the map at $7F:(D+$C8 +
// D+$CA). A letter appears in stages: the queue D+$CF-D+$DD holds the map words of the
// last letters, each pass adding $4C (one stage; $31C at the fourth) until the letter's
// own tile (kept in D+$DF) lands. Letters: a space, 13 marks ($80:B7D9: & # [ . + = { } ?
// ! - ~ ]), digits and the rest by their codes. Control codes $80-$86: next line; column
// 0; jump; blink a cursor (D+$E1, a 16-frame cycle); wait D+$E3 frames, blinking; clear
// the line and back up one; set bit 0 of D+$F8.
void text_type_step(uint16_t dp);

// $82:B18B (DB = $7E): a centred line of 13 two-row characters at $7E:D+$18 (top row) and
// + $1A (bottom). Both rows are first filled with $83:A5A4's blank; the string is the
// zero-ended one at $83:(the word at $83:A006 + D+$3C). Its characters are counted (bytes
// with bit 7 set are not; the string may be in ROM or, at $83:037E-$040B, in WRAM through
// the bank's mirror), the line starts (13 - count) / 2 cells in, and each character's
// two tiles come from $83:A504 / $83:A554, ORed with D+$3A. A byte with bit 7 set changes
// D+$3A to its low 3 bits << 10 (a palette). Codes: space, !, ., + as tiles $27, $24, $25,
// $26; below ':' (digits) - $16; letters - $41.
void text_centred_line(void);
// $82:A134: text_centred_line in the VS race's font: the blank at $83:9F52, the tiles at
// $83:9F6A / $83:9FB8, and space, !, + as tiles $26, $24, $25 ('.' is a digit's - $16 there).
void vs_centred_line(void);
// $82:8911: text_centred_line for string `string` (D+$3C) in palette `palette` (D+$18), in the
// window's plane (D+$3A = $0C00). The ROM pushes $837E and pulls one byte for the data bank
// ($7E), then the other at its end: it returns with DB = $83, which its callers' code does
// not read.
void text_centred_line_at(uint16_t palette, uint16_t string);

// $82:B11F (DB = $7E within, $83 after): the race's message line. A new message D+$16
// replaces the shown one D+$14 if its priority ($83:A0A4 + 2 * message) is at least the
// shown one's: it is drawn (text_centred_line) and its time D+$1A ($83:A142) and D+$1C
// ($83:A1E0) set. Otherwise the shown message's time counts down by $1E9E; when it runs
// out the line is blanked and D+$14 = D+$16 = 0.
void race_message_update(void);
// $82:A0C6: race_message_update in the VS race's font (vs_centred_line, its blank).
void vs_message_update(void);

// $80:855A: the 4-digit value `value` (BCD, kept in D+$1A and read a digit at a time by
// take_nibble16) as sprites from slot D+$74 at x `x` (kept in D+$16), tiles $34 + digit
// + D+$14 placed by their widths ($80:85A6 + tile); one leading zero is dropped, and a
// point (tile $41) goes before the last two digits. Returns the x after the last one.
uint16_t sprite_number_point(uint16_t dp, uint16_t value, uint16_t x);

// $80:8667: sprite_number_point right-aligned: the number is drawn from x `x`, then every
// sprite it added moves left by its width ((x - the x after it), low bytes, in D+$14).
// Returns the slot after it (D+$74).
uint16_t sprite_number_point_right(uint16_t dp, uint16_t value, uint16_t x);

// $80:C475: a bar of 8 sprites at slot `slot`, D+$14 wide, centred on D+$18: four left
// pieces (tile D+$1A + $150, then + $151) 8 apart from its left edge, four right pieces
// ($151 ..., the last mirrored $150 | $4000) from its right edge - $20 (8, 8, 6 apart).
// Returns slot + $20.
uint16_t sprite_bar8(uint16_t dp, uint16_t slot);

#endif
