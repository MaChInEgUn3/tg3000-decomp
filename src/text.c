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

// TG3000_ROUTINE($8A:C13F) text_print_tiles
uint16_t text_print_tiles(uint32_t str) {
  wr16(DP + 0x48, (uint16_t)str);
  uint16_t i = 0;
  for(;; i++) {
    uint8_t ch = bus_rd8(str + i);
    if(ch == 0) return i;
    if(ch == 0x80) {  // new line: next row, column 0
      wr16(TEXT_CURSOR_Y, (uint16_t)(rd16(TEXT_CURSOR_Y) + 1));
      wr16(TEXT_CURSOR_X, 0);
      continue;
    }
    text_put_tile_char(ch);
  }
}

// TG3000_ROUTINE($8A:C137) text_print_tiles_at
uint16_t text_print_tiles_at(uint32_t str, uint16_t column, uint16_t row) {
  wr16(TEXT_CURSOR_X, column);
  wr16(TEXT_CURSOR_Y, row);
  return text_print_tiles(str);
}

// TG3000_ROUTINE($80:84BE) sprite_text_centered
uint16_t sprite_text_centered(uint32_t str, uint16_t x) {
  uint16_t first = rd16(SPRITE_TEXT_SLOT);
  uint16_t end = sprite_text(str, x);
  // half of (start - end) on the low bytes, as a 16-bit LSR: the shift left in $0114
  uint16_t shift = (uint16_t)((uint16_t)((x & 0xFF) - (end & 0xFF)) >> 1);
  wr16(SPRITE_TEXT_BASE, shift);
  // (with an empty string the original's do-while walks all of memory; never happens)
  for(uint16_t s = first; s != rd16(SPRITE_TEXT_SLOT); s = (uint16_t)(s + 4))
    wr8(0x0900 + s, (uint8_t)(rd8(0x0900 + s) + shift));
  return shift;
}

#define SPRITE_DIGIT_GLYPHS 0x34  // glyphs of the hex digits 0-F

// TG3000_ROUTINE($80:84F0) sprite_number
uint16_t sprite_number(uint32_t value, uint16_t x) {
  wr16(SPRITE_TEXT_X, x);
  wr16(DP + 0x18, (uint16_t)(value >> 16));
  wr16(DP + 0x1A, (uint16_t)value);
  uint16_t slot = rd16(SPRITE_TEXT_SLOT);
  int pos = 7;  // digit position, 7 = most significant
  uint16_t digit;
  while((digit = take_nibble()) == 0 && --pos >= 0) {}  // skip leading zeros
  for(;;) {
    uint16_t glyph = (uint16_t)(digit + SPRITE_DIGIT_GLYPHS);
    wr16(0x0902 + slot, (uint16_t)(glyph + rd16(SPRITE_TEXT_BASE)));
    wr16(0x0B01 + slot, 0);
    wr16(0x0900 + slot, rd16(SPRITE_TEXT_X));
    wr16(SPRITE_TEXT_X, (uint16_t)(rom8(SPRITE_GLYPH_WIDTHS + glyph) + rd16(SPRITE_TEXT_X)));
    slot = (uint16_t)(slot + 4);
    digit = take_nibble();
    if(pos == 6 || pos == 3) wr16(SPRITE_TEXT_X, (uint16_t)(rd16(SPRITE_TEXT_X) + 1));  // a gap
    if(--pos < 0) break;
  }
  wr16(SPRITE_TEXT_SLOT, slot);
  return rd16(SPRITE_TEXT_X);
}

// Entries of one list (6 bytes each, until a zero word): tile, x, y offsets. The sprites go
// to $0900/$0B00 + the slot in the table's bank (STA $0900,X with DB = that bank): the
// 65816 adds X in 24 bits, so a slot from $F700 up writes into the next bank.
static uint16_t sprite_group_list(uint16_t dp, uint32_t* at, uint16_t slot, uint16_t flag) {
  uint32_t bank = *at & 0xFF0000;
  for(;;) {
    uint16_t tile = bus_rd16(*at);
    if(tile == 0) return slot;
    bus_wr16(bank + 0x0902 + slot, (uint16_t)(tile + rd16(dp + 0x18)));
    uint16_t xpos = (uint16_t)((uint16_t)(bus_rd16(*at + 2) + rd16(dp + 0x16)) & 0xFF);
    wr16(dp + 0x1A, (uint16_t)(xpos << 8));
    uint16_t ypos = (uint16_t)(bus_rd16(*at + 4) + rd16(dp + 0x14));
    if(ypos & 0xFF00) ypos |= 0xFF00;  // keep only the sign
    bus_wr16(bank + 0x0B00 + slot, ypos);
    bus_wr16(bank + 0x0900 + slot, (uint16_t)((ypos & 0xFF) | (xpos << 8)));
    bus_wr16(bank + 0x0B02 + slot, flag);
    slot = (uint16_t)(slot + 4);
    *at = bank | (uint16_t)(*at + 6);
  }
}

// TG3000_ROUTINE($80:B49E) sprite_group
uint32_t sprite_group(uint16_t dp, uint32_t table) {
  uint16_t slot = rd16(dp + 0x74);
  slot = sprite_group_list(dp, &table, slot, 1);
  table = (table & 0xFF0000) | (uint16_t)(table + 2);
  slot = sprite_group_list(dp, &table, slot, 0);
  wr16(dp + 0x74, slot);
  return table;
}

// TG3000_ROUTINE($80:8698) sprite_number_right
uint16_t sprite_number_right(uint32_t value, uint16_t x) {
  wr16(SPRITE_TEXT_X, (uint16_t)value);
  uint16_t first = rd16(SPRITE_TEXT_SLOT);
  uint16_t end = sprite_number(value, x);
  uint16_t shift = (uint16_t)((x & 0xFF) - (end & 0xFF));
  wr16(SPRITE_TEXT_BASE, shift);
  // a do-while in the original: a number always has at least one digit
  for(uint16_t s = first; s != rd16(SPRITE_TEXT_SLOT); s = (uint16_t)(s + 4))
    wr8(0x0900 + s, (uint8_t)(rd8(0x0900 + s) + shift));
  return shift;
}

// TG3000_ROUTINE($80:8635) sprite_number_centered
uint16_t sprite_number_centered(uint32_t value, uint16_t x) {
  wr16(SPRITE_TEXT_X, (uint16_t)value);
  uint16_t first = rd16(SPRITE_TEXT_SLOT);
  uint16_t end = sprite_number(value, x);
  uint16_t shift = (uint16_t)((uint16_t)((x & 0xFF) - (end & 0xFF)) >> 1);
  wr16(SPRITE_TEXT_BASE, shift);
  for(uint16_t s = first; s != rd16(SPRITE_TEXT_SLOT); s = (uint16_t)(s + 4))
    wr8(0x0900 + s, (uint8_t)(rd8(0x0900 + s) + shift));
  return shift;
}

static uint16_t signed_high_byte(uint16_t v) {  // the high byte, sign-extended to 16 bits
  uint16_t h = v >> 8;
  return (h & 0x80) ? (uint16_t)(h | 0xFF00) : h;
}

// TG3000_ROUTINE($80:EB66) func_80EB66
uint32_t func_80EB66(uint16_t dp) {
  uint16_t x = (uint16_t)(((uint16_t)(rd16(dp + 0x7C) - signed_high_byte(rd16(dp + 0x22))) & 0xFF) - 0x100);
  wr16(dp + 0x14, x);
  uint16_t w = rd16(dp + 0x7E);
  w = (uint16_t)(w << 8 | w >> 8);
  w = (uint16_t)(w << 1 | w >> 15);
  uint16_t y = (uint16_t)(((w + 0x24) & 0xFF) + rd16(dp + 0xE8));
  wr16(dp + 0x16, (uint16_t)(y - signed_high_byte(rd16(dp + 0x24))));
  wr16(dp + 0x18, 0xF000);
  return sprite_group(dp, 0x81CBB8);
}

// TG3000_ROUTINE($8A:C0F2) func_8AC0F2
uint16_t func_8AC0F2(void) {
  wr16(DP + 0x90, 1);
  text_print_tiles_at(0x8AC44F, 12, 6);
  wr16(DP + 0x90, 0);
  text_print_tiles_at(0x8AC458, 6, 22);
  text_print_tiles_at(0x8AC46D, 5, 23);
  text_print_tiles_at(0x8AC484, 5, 24);
  wr16(DP + 0x90, 1);
  return text_print_tiles_at(0x8AC49B, 6, 26);
}

// TG3000_ROUTINE($80:9DD0) func_809DD0
//
// The $3E00 variant ($80:9DDC) runs in no recorded movie; it is checked by the fuzz.
uint16_t func_809DD0(void) {
  uint16_t flags = rd16(DP + 0x76);
  bool alt = (flags & 0x0002) && !(flags & 0x0004);
  uint16_t base = alt ? 0x3E00 : 0x3C00;
  wr16(SPRITE_TEXT_BASE, base);
  sprite_text(alt ? 0x809E42 : 0x809E3A, (uint16_t)(0x5C5C + rd16(DP + 0xE7)));
  uint16_t i = rd16(DP + 0x94);
  wr16(SPRITE_TEXT_BASE, base);
  uint32_t value = (uint32_t)rd16((uint16_t)(0x0390 + i)) << 16 | rd16((uint16_t)(0x038E + i));
  return sprite_number(value, (uint16_t)(0x5C94 + rd16(DP + 0xE7)));
}

#define TYPE_MAP 0x10000  // $7F:0000

// the text's byte at [D+$CC] + `offset`: [dp],y adds across the bank
static uint8_t type_byte(uint16_t offset) {
  return bus_rd8((((uint32_t)rd8(DP + 0xCE) << 16 | rd16(DP + 0xCC)) + offset) & 0xFFFFFF);
}

// the text pointer moves on by `n` (its bank word counts the carry, $CF's low byte included)
static void type_advance(uint16_t n) {
  uint32_t sum = (uint32_t)rd16(DP + 0xCC) + n;
  wr16(DP + 0xCC, (uint16_t)sum);
  if(sum > 0xFFFF) wr16(DP + 0xCE, (uint16_t)(rd16(DP + 0xCE) + 1));  // never run
}

static void type_map_add(uint16_t at, uint16_t add) {
  if(at != 0xFFFF) wr16(TYPE_MAP + at, (uint16_t)(rd16(TYPE_MAP + at) + add));
}

// the cursor's blink: 16 frames, shown at frame 8, its own tile at 0 (a new cycle)
static void type_blink(void) {
  uint16_t left = (uint16_t)(rd16(DP + 0xE1) - 1);
  wr16(DP + 0xE1, left);
  if(left == 0) {
    wr16(DP + 0xDF, 0);
    wr16(DP + 0xDD, (uint16_t)(rd16(DP + 0xC8) + rd16(DP + 0xCA)));
    wr16(DP + 0xE1, 0x0010);
    func_80B7BB();
  } else if(left == 0x0008) {
    wr16(DP + 0xDF, 0x284C);
    wr16(DP + 0xDD, (uint16_t)(rd16(DP + 0xC8) + rd16(DP + 0xCA)));
    func_80B7BB();
  } else {
    wr16(DP + 0xDD, 0xFFFF);
  }
}

// one step's text: the control codes that take no time loop here; returns after a
// letter or a code that takes the step
static void type_text(void) {
  for(;;) {
    uint8_t c = type_byte(0);
    if(c & 0x80) {
      switch((uint8_t)(c << 1)) {
      case 0x00:  // $80: next line
        wr16(DP + 0xCA, (uint16_t)((rd16(DP + 0xCA) & 0xFFC0) + 0x0040));
        wr16(DP + 0xF6, (uint16_t)(rd16(DP + 0xF6) + 1));
        type_advance(1);
        continue;
      case 0x02:  // $81: column 0 (never run)
        wr16(DP + 0xCA, 0);
        type_advance(1);
        continue;
      case 0x04:  // $82: jump to the address that follows (never run)
        wr16(DP + 0xCC, (uint16_t)(type_byte(1) | type_byte(2) << 8));
        type_advance(0);
        continue;
      case 0x06:  // $83: blink until the player goes on
        type_blink();
        return;
      case 0x08:  // $84: wait the count that follows, blinking
        if(rd16(DP + 0xE3) == 0) wr16(DP + 0xE3, (uint16_t)(type_byte(1) | type_byte(2) << 8));
        wr16(DP + 0xE3, (uint16_t)(rd16(DP + 0xE3) - 1));
        if(rd16(DP + 0xE3) == 0) {
          type_advance(3);
          continue;
        }
        type_blink();
        return;
      case 0x0A: {  // $85: clear the line, then back up a line until the first
        for(uint32_t a = 0xCF; a <= 0xDD; a += 2) wr16(DP + a, 0xFFFF);
        uint16_t at = (uint16_t)((rd16(DP + 0xCA) & 0xFFC0) + rd16(DP + 0xC8));
        uint16_t n = rd16(DP + 0xF9);
        do {  // the width counts down to 0 (a width of 0 would clear 65536 words)
          wr16(TYPE_MAP + at, 0);
          at = (uint16_t)(at + 2);
        } while(--n);
        uint16_t line = (uint16_t)(rd16(DP + 0xCA) & 0xFFC0);
        wr16(DP + 0xCA, line);
        if(line == 0) {
          type_advance(1);
          continue;
        }
        wr16(DP + 0xCA, (uint16_t)(line - 0x0040));
        wr16(DP + 0xF6, (uint16_t)(rd16(DP + 0xF6) - 1));
        return;
      }
      case 0x0C:  // $86: flag
        wr16(DP + 0xF8, (uint16_t)(rd16(DP + 0xF8) | 0x0001));
        type_advance(1);
        continue;
      default:
        return;  // no other code appears in the game's texts
      }
    }
    uint16_t at = (uint16_t)(rd16(DP + 0xC8) + rd16(DP + 0xCA));
    if(c == 0x20) {
      wr16(DP + 0xDF, 0);
      wr16(TYPE_MAP + at, 0x084C);
    } else {
      uint16_t tile = 0;
      bool mark = false;
      for(uint16_t i = 0x3F; i < 0x4C; i++)
        if(rom8(0x80B79A + i) == c) {
          tile = i;
          mark = true;
          break;
        }
      if(!mark) tile = c < 0x41 ? (uint8_t)(c + 5) : c < 0x61 ? (uint8_t)(c - 0x26) : (uint8_t)(c - 0x60);
      wr16(DP + 0xDF, (uint16_t)(tile | 0x2800));
      wr16(TYPE_MAP + at, 0x284C);
    }
    wr16(DP + 0xDD, at);
    type_advance(1);
    wr16(DP + 0xCA, (uint16_t)(rd16(DP + 0xCA) + 2));
    func_80B7BB();
    wr16(DP + 0xE1, 9);
    return;
  }
}

// TG3000_ROUTINE($80:B59D) text_type_step
void text_type_step(void) {
  wr16(DP + 0xC6, (uint16_t)(rd16(DP + 0xC6) + rd16(DP + 0xE5)));
  while(rd16(DP + 0xC6) >= 0x0100) {
    wr16(DP + 0xC6, (uint16_t)(rd16(DP + 0xC6) - 0x0100));
    type_map_add(rd16(DP + 0xCF), 0x004C);
    for(uint32_t a = 0xCF; a < 0xDB; a += 2) {
      uint16_t at = rd16(DP + a + 2);
      wr16(DP + a, at);
      type_map_add(at, a == 0xD3 ? 0x031C : 0x004C);
    }
    uint16_t at = rd16(DP + 0xDD);
    wr16(DP + 0xDB, at);
    if(at != 0xFFFF) {
      wr16(TYPE_MAP + at, rd16(DP + 0xDF));
      if(rd16(DP + 0xDF) == 0) wr16(DP + 0xDB, 0xFFFF);
    }
    type_text();
  }
}

// a character code's tile index (text_centred_line)
static uint16_t centred_tile(uint16_t c) {
  switch(c) {
  case 0x20: return 0x27;
  case 0x21: return 0x24;
  case 0x2E: return 0x25;  // never run
  case 0x2B: return 0x26;
  default: return c < 0x3A ? (uint16_t)(c - 0x16) : (uint16_t)(c - 0x41);
  }
}

// TG3000_ROUTINE($82:B18B) text_centred_line
void text_centred_line(void) {
  uint16_t x = rd16(DP + 0x18), blank = rom16(0x83A5A4);
  for(int i = 0; i < 13; i++, x = (uint16_t)(x + 2)) {
    wr16(x, blank);
    wr16((uint16_t)(x + 0x1A), blank);
  }
  const uint16_t start = rom16(0x83A006 + (uint32_t)rd16(DP + 0x3C));
  uint16_t free = 13;
  for(uint16_t s = start;; s++) {
    uint8_t c = bus_rd8(0x830000 | s);
    if(c == 0) break;
    if(!(c & 0x80)) free--;
  }
  uint16_t y = (uint16_t)((free & 0xFFFE) + rd16(DP + 0x18));
  for(uint16_t s = start;; s++) {
    uint8_t c = bus_rd8(0x830000 | s);
    if(c == 0) break;
    if(c & 0x80) {  // never run: a palette change
      wr16(DP + 0x3A, (uint16_t)((c & 0x07) << 10));
      continue;
    }
    const uint32_t t = (uint32_t)(uint16_t)(centred_tile(c) << 1);
    wr16(y, (uint16_t)(rom16(0x83A504 + t) | rd16(DP + 0x3A)));
    wr16((uint16_t)(y + 0x1A), (uint16_t)(rom16(0x83A554 + t) | rd16(DP + 0x3A)));
    y = (uint16_t)(y + 2);
  }
}

// TG3000_ROUTINE($82:B11F) race_message_update
void race_message_update(void) {
  if(rd16(DP + 0x14) != rd16(DP + 0x16)) {
    uint16_t shown = (uint16_t)(rd16(DP + 0x14) << 1), next = (uint16_t)(rd16(DP + 0x16) << 1);
    wr16(DP + 0x3C, next);
    if(rom16(0x83A0A4 + (uint32_t)next) >= rom16(0x83A0A4 + (uint32_t)shown)) {
      text_centred_line();
      wr16(DP + 0x1A, rom16(0x83A142 + (uint32_t)rd16(DP + 0x3C)));
      wr16(DP + 0x1C, rom16(0x83A1E0 + (uint32_t)rd16(DP + 0x3C)));
      wr16(DP + 0x14, rd16(DP + 0x16));
      return;
    }
    wr16(DP + 0x16, rd16(DP + 0x14));
  }
  uint16_t time = rd16(DP + 0x1A);
  if(time == 0) return;
  uint16_t step = rd16(0x1E9E);
  if(time > step) {
    wr16(DP + 0x1A, (uint16_t)(time - step));
    return;
  }
  uint16_t x = rd16(DP + 0x18), blank = rom16(0x83A5A4);  // its time is up: blank the line
  for(int i = 0; i < 13; i++, x = (uint16_t)(x + 2)) {
    wr16(x, blank);
    wr16((uint16_t)(x + 0x1A), blank);
  }
  wr16(DP + 0x16, 0);
  wr16(DP + 0x14, 0);
  wr16(DP + 0x1A, 0);
}

// TG3000_ROUTINE($80:855A) sprite_number_point
uint16_t sprite_number_point(uint16_t value, uint16_t x) {
  wr16(DP + 0x16, x);
  wr16(DP + 0x1A, value);
  uint16_t slot = rd16(DP + 0x74), n = 4, digit;
  for(;;) {  // one leading zero at most is dropped
    digit = take_nibble16();
    if(digit != 0 || n == 3) break;
    n--;
  }
  do {
    uint16_t tile = (uint16_t)(digit + 0x34);
    const uint32_t s = slot;
    wr16(0x0902 + s, (uint16_t)(tile + rd16(DP + 0x14)));
    wr16(0x0B01 + s, 0);
    wr16(0x0900 + s, (uint16_t)(rom8(0x8085A6 + tile) + rd16(DP + 0x16)));
    wr16(DP + 0x16, (uint16_t)(rd16(DP + 0x16) + 8));
    slot = (uint16_t)(slot + 4);
    if(n == 3) {  // the point
      const uint32_t p = slot;
      wr16(0x0902 + p, (uint16_t)(0x0041 + rd16(DP + 0x14)));
      wr16(0x0B01 + p, 0);
      wr16(0x0900 + p, rd16(DP + 0x16));
      wr16(DP + 0x16, (uint16_t)(rd16(DP + 0x16) + 3));
      slot = (uint16_t)(slot + 4);
    }
    digit = take_nibble16();
  } while(--n);
  wr16(DP + 0x74, slot);
  return rd16(DP + 0x16);
}

// TG3000_ROUTINE($80:C475) sprite_bar8
uint16_t sprite_bar8(uint16_t slot) {
  const uint32_t x = slot;
  uint16_t half = (uint16_t)(rd16(DP + 0x14) >> 1), odd = (uint16_t)(rd16(DP + 0x14) & 1);
  uint16_t at = (uint16_t)(rd16(DP + 0x18) - half);
  for(int i = 0; i < 4; i++, at = (uint16_t)(at + 8)) wr16(0x0900 + x + 4 * (uint32_t)i, at);
  at = (uint16_t)(half + rd16(DP + 0x18) + odd - 0x20);  // the LSR's carry goes into the ADC
  static const uint16_t gap[4] = {0, 8, 6, 8};
  for(int i = 0; i < 4; i++) {
    at = (uint16_t)(at + gap[i]);
    wr16(0x0910 + x + 4 * (uint32_t)i, at);
  }
  uint16_t tile = (uint16_t)(rd16(DP + 0x1A) + 0x0150);
  wr16(0x0902 + x, tile);
  for(int i = 1; i < 7; i++) wr16(0x0902 + x + 4 * (uint32_t)i, (uint16_t)(tile + 1));
  wr16(0x091E + x, (uint16_t)(tile | 0x4000));
  for(int i = 0; i < 8; i++) wr16(0x0B01 + x + 4 * (uint32_t)i, 0);
  return (uint16_t)(slot + 0x20);
}

// TG3000_ROUTINE($80:8667) sprite_number_point_right
uint16_t sprite_number_point_right(uint16_t value, uint16_t x) {
  uint16_t first = rd16(DP + 0x74);
  uint16_t end = sprite_number_point(value, x);
  wr16(DP + 0x14, (uint16_t)((x & 0x00FF) - (end & 0x00FF)));
  uint16_t slot = first;
  do {
    const uint32_t s = slot;
    wr8(0x0900 + s, (uint8_t)(rd8(0x0900 + s) + rd8(DP + 0x14)));
    slot = (uint16_t)(slot + 4);
  } while(slot != rd16(DP + 0x74));
  return slot;
}
