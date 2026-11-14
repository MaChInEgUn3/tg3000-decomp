// SPDX-License-Identifier: GPL-3.0-or-later
#include "text.h"

#include "misc.h"
#include "tg3000.h"

#define TEXT_MAP_TOP 0x10000     // $7F:0000
#define TEXT_MAP_BOTTOM 0x10040  // $7F:0040
#define TEXT_PALETTE (DP + 0x42) // tile attribute bits ORed in (main font)

// Glyph pairs (top, bottom) in ROM, 4 bytes per glyph, for the main font; the alternate
// font keeps tops and bottoms in two word tables. The main font's punctuation is looked
// up in a list of 15 characters ("&\x9c[.+={}?!]<>/_") at $8A:B8FA, glyphs $24-$32;
// anything not found uses glyph $33 (the space).
#define MAIN_GLYPHS 0x81CCD9
#define ALT_TOPS 0x8AC9C4
#define ALT_BOTTOMS 0x8ACA3A
#define PUNCTUATION 0x8AB8FA

static uint16_t main_glyph(uint8_t ch) {
  if(ch >= 0x30 && ch < 0x3A) {  // digits use the second palette
    wr16(TEXT_PALETTE, 0x3C00);
    return (uint16_t)(ch - 0x16);
  }
  if(ch >= 0x41 && ch < 0x5B) return (uint16_t)(ch - 0x41);  // letters
  if(ch == 0x20) return 0x33;
  uint16_t i;
  for(i = 0x24; i < 0x33; i++) {
    if(rom8(PUNCTUATION + i - 0x24) == ch) break;
  }
  return i;
}

// TG3000_ROUTINE($8A:B83C) text_put_char
uint16_t text_put_char(uint8_t ch) {
  wr16(DP + 0x36, rd16(TEXT_CURSOR_X));
  wr16(DP + 0x38, rd16(TEXT_CURSOR_Y));
  uint16_t at = tilemap_offset();
  uint16_t top, bottom;
  if(rd16(TEXT_FONT_SEL) == 0) {
    uint16_t i = (uint16_t)((ch - 0x20) << 1);
    top = rom16(ALT_TOPS + i) | 0x3C00;
    bottom = rom16(ALT_BOTTOMS + i) | 0x3C00;
  } else {
    wr16(TEXT_PALETTE, 0x3800);
    uint16_t i = (uint16_t)(main_glyph(ch) << 2);
    top = (uint16_t)((rom16(MAIN_GLYPHS + i) | rd16(TEXT_PALETTE)) + 0x0100);
    bottom = (uint16_t)((rom16(MAIN_GLYPHS + 2 + i) | rd16(TEXT_PALETTE)) + 0x0100);
  }
  wr16(TEXT_MAP_TOP + at, top);
  wr16(TEXT_MAP_BOTTOM + at, bottom);
  wr16(TEXT_CURSOR_X, (uint16_t)(rd16(TEXT_CURSOR_X) + 1));
  return at;
}

// TG3000_ROUTINE($8A:C2A2) char_to_tile_index
uint16_t char_to_tile_index(uint16_t ch) {
  if(ch == 0x20) return 0x25;
  if(ch == 0x2E) return 0x24;
  if(ch == 0x21) return 0x26;
  if((int16_t)(ch - 0x41) < 0) return (uint16_t)(ch - 0x16);
  return (uint16_t)(ch - 0x41);
}

// TG3000_ROUTINE($8A:B6AA) text_print_centered
void text_print_centered(uint32_t str) {
  uint16_t left = rd16(DP + 0x3E);
  wr16(DP + 0x42, (uint16_t)((rd16(DP + 0x40) - left) >> 1));
  wr16(DP + 0x44, (uint16_t)str);
  uint16_t len = 0;
  while(bus_rd8(str + len) != 0) len++;
  wr16(DP + 0x5A, len);
  wr16(DP + 0x46, (uint16_t)(len >> 1));
  wr16(TEXT_CURSOR_X, (uint16_t)(rd16(DP + 0x42) - rd16(DP + 0x46) + left + 1));
  for(uint16_t i = 0;; i++) {
    uint8_t ch = bus_rd8(str + i);
    if(ch == 0) break;
    text_put_char(rd8(DP + 0x68) != 0 ? ' ' : ch);
  }
  if(rd16(DP + 0x86) == 0) func_8AB700();
}

// TG3000_ROUTINE($8A:C278) text_put_tile_char
uint16_t text_put_tile_char(uint8_t ch) {
  wr16(DP + 0x36, rd16(TEXT_CURSOR_X));
  wr16(DP + 0x38, rd16(TEXT_CURSOR_Y));
  uint16_t at = tilemap_offset();
  uint16_t tile = (uint16_t)(char_to_tile_index(ch) | (rd16(DP + 0x90) ? 0x0400 : 0) | 0x3880);
  wr16(TEXT_MAP_TOP + at, tile);
  wr16(TEXT_CURSOR_X, (uint16_t)(rd16(TEXT_CURSOR_X) + 1));
  return tile;
}

// TG3000_ROUTINE($8A:A37A) text_cursor_down3
void text_cursor_down3(void) { wr16(TEXT_CURSOR_Y, (uint16_t)(rd16(TEXT_CURSOR_Y) + 3)); }

#define SPRITE_TEXT_SLOT (DP + 0x74)
#define SPRITE_TEXT_X (DP + 0x16)
#define SPRITE_TEXT_BASE (DP + 0x14)
#define SPRITE_GLYPH_WIDTHS 0x8085E5
#define SPRITE_PUNCTUATION 0x8084B0  // "&#[.+={}?!-~]_", glyphs $3E-$4B; others: the code

static uint16_t sprite_glyph(uint8_t ch) {
  if(ch >= 0x30 && ch < 0x3A) return (uint8_t)(ch + 4);       // digits $34-$3D
  if(ch >= 0x41 && ch < 0x5B) return (uint8_t)(ch - 0x27);     // capitals $1A-$33
  if(ch >= 0x61 && ch < 0x7B) return (uint8_t)(ch - 0x61);     // lower case $00-$19
  for(uint16_t g = 0x3E; g < 0x4C; g++) {
    if(rom8(SPRITE_PUNCTUATION + g - 0x3E) == ch) return g;
  }
  return ch;  // not in the list: the original falls through with the character itself
}

// TG3000_ROUTINE($80:841A) sprite_text
uint16_t sprite_text(uint32_t str, uint16_t x) {
  wr16(SPRITE_TEXT_X, x);
  uint16_t slot = rd16(SPRITE_TEXT_SLOT);
  for(uint16_t i = 0;; i++) {
    uint8_t ch = bus_rd8((str & 0xFF0000) | (uint16_t)(str + i));
    if(ch == 0) break;
    if(ch & 0x80) continue;
    if(ch == 0x20) {
      wr8(SPRITE_TEXT_X, (uint8_t)(rd8(SPRITE_TEXT_X) + 6));
      continue;
    }
    uint16_t g = sprite_glyph(ch);
    wr16(0x0902 + slot, (uint16_t)(g + rd16(SPRITE_TEXT_BASE)));
    wr16(0x0900 + slot, rd16(SPRITE_TEXT_X));
    wr16(0x0B01 + slot, 0);
    wr8(SPRITE_TEXT_X, (uint8_t)(rom8(SPRITE_GLYPH_WIDTHS + g) + rd8(SPRITE_TEXT_X)));
    slot = (uint16_t)(slot + 4);
  }
  wr16(SPRITE_TEXT_SLOT, slot);
  return rd16(SPRITE_TEXT_X);
}
