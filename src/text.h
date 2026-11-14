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

#endif
