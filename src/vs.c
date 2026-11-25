// SPDX-License-Identifier: GPL-3.0-or-later
#include "vs.h"

#include <stdbool.h>

#include "misc.h"
#include "decompress.h"
#include "frame.h"
#include "platform.h"
#include "rng.h"
#include "util.h"
#include "video_dma.h"
#include "tg3000.h"

static uint16_t table8a(uint16_t at, uint16_t x) { return bus_rd16(0x8A0000 | (uint16_t)(at + x)); }

// TG3000_ROUTINE($8A:A7CD) vs_palette_load
void vs_palette_load(void) { upload_cgram_now(0x00, 0x8A, 0xC5D8, 0x0080); }

// TG3000_ROUTINE($8A:AD58) vs_cursor_moving
bool vs_cursor_moving(uint16_t x) {
  return rd16((uint16_t)(0x0D88 + x)) != rd16((uint16_t)(0x0D80 + x)) ||
         rd16((uint16_t)(0x0D78 + x)) != rd16((uint16_t)(0x0D70 + x)) ||
         rd16((uint16_t)(0x0D90 + x)) != rd16((uint16_t)(0x0D68 + x));
}

// TG3000_ROUTINE($8A:ABCF) vs_panel_sprite
void vs_panel_sprite(uint16_t x) {
  wr16((uint16_t)(0x06F8 + x), (uint16_t)((table8a(0xACD4, x) << 3) + rd16(DP + 0x8A)));
  wr16((uint16_t)(0x07F8 + x), (uint16_t)((table8a(0xACDC, x) << 3) + 7));
  wr16((uint16_t)(0x08F8 + x), 0x3002);
}

// TG3000_ROUTINE($8A:AA58) vs_cursor_home
void vs_cursor_home(uint16_t x) {
  wr16((uint16_t)(0x0D88 + x), table8a(0xAA82, x));
  wr16((uint16_t)(0x0D90 + x), table8a(0xAA8A, x));
  wr16((uint16_t)(0x0D78 + x), 0x0004);
  wr16((uint16_t)(0x0DAC + x), 0xFFFF);
}

// TG3000_ROUTINE($8A:ACE4) vs_letter_index
uint16_t vs_letter_index(uint16_t x) {
  const uint16_t i = (uint16_t)(rd16((uint16_t)(0x0DB4 + x)) * 7 + rd16((uint16_t)(0x0DAC + x)));
  wr16(DP + 0x4E, i);
  return i;
}

// TG3000_ROUTINE($8A:A72F) vs_panel_screens
void vs_panel_screens(uint16_t x) {
  if(x == 0 || x == 4) wr8(DP + 0xFB, 0x13);
  else if(x == 2 || x == 6) wr8(DP + 0xFC, 0x13);
}

// TG3000_ROUTINE($8A:AEDA) vs_letter_next
//
// Only its end case runs in a movie ($0DB4 is 4 there); the steps are checked by the fuzz.
void vs_letter_next(uint16_t x) {
  const uint16_t was = rd16((uint16_t)(0x0DB4 + x));
  for(;;) {
    if(rd16((uint16_t)(0x0DB4 + x)) == 4) {
      wr16((uint16_t)(0x0DB4 + x), was);
      return;
    }
    wr16((uint16_t)(0x0DB4 + x), (uint16_t)(rd16((uint16_t)(0x0DB4 + x)) + 1));
    const uint8_t c = bus_rd8(0x8A0000 | (uint16_t)(0xAFF1 + vs_letter_index(x)));
    if(c == 0x20) continue;
    if(c == 0x3C) {
      wr16((uint16_t)(0x0DAC + x), (uint16_t)(rd16((uint16_t)(0x0DAC + x)) - 1));
      continue;
    }
    return;
  }
}

// TG3000_ROUTINE($8A:AA0E) vs_cursor_column
void vs_cursor_column(uint16_t x) {
  wr16((uint16_t)(0x0D88 + x), (uint16_t)((rd16((uint16_t)(0x0DB4 + x)) << 4) + table8a(0xAA82, x)));
  wr16((uint16_t)(0x0D90 + x), table8a(0xAA8A, x));
}

// TG3000_ROUTINE($8A:AAA2) vs_caret_blink
void vs_caret_blink(uint16_t x) {
  if(rd16((uint16_t)(0x0DDC + x)) == 0xFFFE) wr16((uint16_t)(0x05F8 + x), 0);
  else wr16((uint16_t)(0x05F8 + x), (rd16(DP + 0x66) & 1) ? 0xFFFF : 0x0000);
}

// TG3000_ROUTINE($8A:ACB2) vs_name_clear
void vs_name_clear(uint16_t x) {
  if(rd16((uint16_t)(0x0DEC + x)) != 0) return;
  wr16((uint16_t)(0x0DEC + x), (uint16_t)(rd16((uint16_t)(0x0DEC + x)) + 1));
  wr16((uint16_t)(0x0DD4 + x), 0);
  const uint16_t at = table8a(0xA804, x);
  wr16(DP + 0x48, at);
  for(uint16_t y = 0; y < 0x0C; y += 2) bus_wr16(0x8A0000 | (uint16_t)(at + y), 0);
}

// TG3000_ROUTINE($8A:AA23) vs_cursor_step
void vs_cursor_step(uint16_t x) {
  if(rd16((uint16_t)(0x0D38 + x)) != 0xFFFF) return;
  if(vs_cursor_moving(x)) return;
  const uint16_t pad = rd16((uint16_t)(0x0DCC + x));
  const uint16_t col = rd16((uint16_t)(0x0DB4 + x));
  if(pad & 0x0100) {
    if(col != 3) wr16((uint16_t)(0x0DB4 + x), (uint16_t)(col + 1));
  } else if(pad & 0x0200) {
    if(col != 0) wr16((uint16_t)(0x0DB4 + x), (uint16_t)(col - 1));
  }
}

// TG3000_ROUTINE($8A:AE20) vs_letter_find
// (A letter missing from the table: the original searches on past it.)
void vs_letter_find(uint16_t x, uint8_t c) {
  uint16_t y = 0;
  while(bus_rd8(0x8A0000 | (uint16_t)(0xAF57 + y)) != c) y = (uint16_t)(y + 3);
  wr16((uint16_t)(0x0DAC + x), bus_rd8(0x8A0000 | (uint16_t)(0xAF58 + y)));
  wr16((uint16_t)(0x0DB4 + x), bus_rd8(0x8A0000 | (uint16_t)(0xAF59 + y)));
}

// TG3000_ROUTINE($8A:AD34) vs_cursor_target
void vs_cursor_target(uint16_t x) {
  wr16((uint16_t)(0x0D88 + x), (uint16_t)((rd16((uint16_t)(0x0DAC + x)) << 4) + rd16((uint16_t)(0x0DBC + x)) -
                                          0x0080 + rd16(DP + 0x50)));
  wr16((uint16_t)(0x0D90 + x), (uint16_t)((rd16((uint16_t)(0x0DB4 + x)) << 4) + rd16((uint16_t)(0x0DC4 + x))));
}

// TG3000_ROUTINE($8A:A795) vs_all_ready
void vs_all_ready(void) {
  wr16(DP + 0x42, 0);
  wr16(DP + 0x44, 0);
  for(uint16_t x = 0; x < 8; x += 2) {
    if(rd16((uint16_t)(0x0DF4 + x)) != 0xFFFF) continue;
    wr16(DP + 0x42, (uint16_t)(rd16(DP + 0x42) + 1));
    if(rd16((uint16_t)(0x0DDC + x)) == 0xFFFD) wr16(DP + 0x44, (uint16_t)(rd16(DP + 0x44) + 1));
  }
  if(rd16(DP + 0x42) != rd16(DP + 0x44)) return;
  wr16(0x0198, 0);
  wr16(0x0196, 0xFFC0);
}

// TG3000_ROUTINE($8A:A728) vs_panel_screens_bg2_off
void vs_panel_screens_bg2_off(uint16_t x) {
  if(x == 0 || x == 4) wr8(DP + 0xFB, 0x11);
  else if(x == 2 || x == 6) wr8(DP + 0xFC, 0x11);
}

// TG3000_ROUTINE($8A:AE6E) vs_panel_text_base
void vs_panel_text_base(uint16_t x) {
  static const uint16_t base[4] = {0x0000, 0x0022, 0x03C0, 0x03E2};
  if(x <= 6 && !(x & 1)) wr16(DP + 0x70, base[x >> 1]);
}

// TG3000_ROUTINE($8A:A97F) vs_text_center
void vs_text_center(uint16_t str) {
  wr16(DP + 0x48, str);
  uint16_t n = 0;
  while(bus_rd8(0x8A0000 | (uint16_t)(str + n)) != 0) n++;
  wr16(DP + 0x48, (uint16_t)(n >> 1));
  wr16(DP + 0x3A, (uint16_t)(0x0006 - rd16(DP + 0x48) + rd16(DP + 0x3A)));
}

// TG3000_ROUTINE($8A:AF08) vs_row_next
//
// Its step that lands on a letter ($8A:AF2A) runs in no movie; it is checked by the fuzz.
void vs_row_next(uint16_t x) {
  const uint16_t was = rd16((uint16_t)(0x0DAC + x));
  for(;;) {
    if(rd16((uint16_t)(0x0DAC + x)) == 6) {
      wr16((uint16_t)(0x0DAC + x), was);
      return;
    }
    wr16((uint16_t)(0x0DAC + x), (uint16_t)(rd16((uint16_t)(0x0DAC + x)) + 1));
    const uint8_t c = bus_rd8(0x8A0000 | (uint16_t)(0xAFF1 + vs_letter_index(x)));
    if(c != 0x20 && c != 0x3C) return;
  }
}

// TG3000_ROUTINE($8A:A9F2) vs_panel_reset
bool vs_panel_reset(uint16_t x) {
  bool carry = true;
  wr16((uint16_t)(0x0DB4 + x), 0);
  if(rd16((uint16_t)(0x0D38 + x)) != 0xFFFF)
    wr16((uint16_t)(0x0DB4 + x), (uint16_t)(rng_next((uint16_t)(DP + 0x10), &carry) & 0x0003));
  vs_cursor_home(x);
  return carry;
}

// TG3000_ROUTINE($8A:AE46) vs_panel_text_clear
void vs_panel_text_clear(uint16_t x) {
  wr16(DP + 0x72, 0x007F);
  vs_panel_text_base(x);
  for(int row = 0; row < 14; row++) {
    for(uint16_t y = 0; y < 0x1E; y += 2) wr16(0x7F0000 | (uint16_t)(rd16(DP + 0x70) + y), 0x003D);
    wr16(DP + 0x70, (uint16_t)(rd16(DP + 0x70) + 0x40));
  }
}

// TG3000_ROUTINE($8A:A768) vs_ready_check
void vs_ready_check(void) {
  vs_all_ready();
  for(uint16_t x = 0; x < 8; x += 2)
    if(rd16((uint16_t)(0x0DDC + x)) != 0xFFFD) return;
  wr16(0x0198, 0);
  wr16(0x0196, 0xFF80);
}

// TG3000_ROUTINE($8A:A8E5) vs_name_default
//
// The copy runs in no recorded movie (the player's name is never empty there); it is
// checked by the fuzz.
uint16_t vs_name_default(uint16_t x) {
  wr16(DP + 0x46, 0);
  const uint16_t name = table8a(0xA804, x);
  wr16(DP + 0x42, name);
  for(uint16_t y = 0;; y++) {
    const uint8_t c = bus_rd8(0x8A0000 | (uint16_t)(name + y));
    if(c == 0) break;
    if(c >= 0x41 && c < 0x5B) wr16(DP + 0x46, (uint16_t)(rd16(DP + 0x46) + 1));
  }
  const uint16_t count = rd16(DP + 0x46);
  if(count != 0) return count;
  const uint16_t from = table8a(0xA814, x);
  wr16(DP + 0x44, from);
  for(uint16_t y = 0;; y++) {
    const uint8_t c = bus_rd8(0x8A0000 | (uint16_t)(from + y));
    bus_wr8(0x8A0000 | (uint16_t)(name + y), c);
    if(c == 0) break;
  }
  return 0;
}

// TG3000_ROUTINE($8A:ACFF) vs_letter_width
//
// The 's' and 'd' cases run in no recorded movie; they are checked by the fuzz.
void vs_letter_width(uint16_t x) {
  const uint16_t y = rd16(DP + 0x4E);
  wr16((uint16_t)(0x0D78 + x), bus_rd8(0x8A0000 | (uint16_t)(0xB014 + y)));
  const uint8_t c = bus_rd8(0x8A0000 | (uint16_t)(0xAFF1 + y));
  wr16(DP + 0x50, (c == 0x73 || c == 0x65) ? 9 : (c == 0x64) ? 8 : 0);
}

// TG3000_ROUTINE($8A:A7DB) vs_players_in
void vs_players_in(void) {
  uint16_t a = (uint16_t)(rd16(0x03DC) >> 4);
  for(uint16_t x = 0; x < 8; x += 2) {
    const uint16_t v = (a & 1) ? 0xFFFF : 0x0000;
    a = (uint16_t)(a >> 1);
    wr16((uint16_t)(0x0DDC + x), v);
    wr16((uint16_t)(0x0DF4 + x), v);
  }
}

// TG3000_ROUTINE($8A:A922) vs_names_load
void vs_names_load(void) {
  static const uint16_t from[4] = {0xCD7C, 0xCD87, 0xCD92, 0xCD9F};
  static const uint16_t to[4] = {0x03E4, 0x03F1, 0x03FE, 0x040B};
  for(int i = 0; i < 4; i++) {
    uint16_t n = 0;
    uint8_t c;
    do {
      c = bus_rd8(0x8A0000 | (uint16_t)(from[i] + n));
      wr8((uint16_t)(to[i] + n), c);
      if(c != 0) n++;
    } while(c != 0);
    wr16((uint16_t)(0x0DD4 + 2 * i), n);
  }
}

// TG3000_ROUTINE($8A:AC73) vs_name_put
void vs_name_put(uint16_t x, uint8_t c) {
  vs_panel_screens_bg2_off(x);
  static const uint16_t names[4] = {0x03E4, 0x03F1, 0x03FE, 0x040B};
  const uint16_t name = (x <= 6 && !(x & 1)) ? names[x >> 1] : names[0];
  wr8((uint16_t)(name + rd16((uint16_t)(0x0DD4 + x))), c);
}

// TG3000_ROUTINE($8A:B730) vs_put_char
uint16_t vs_put_char(uint8_t c) {
  const uint16_t glyph = (uint16_t)(((uint16_t)c - 0x20) << 1);
  if(rd16(DP + 0x88) != 0) {
    uint16_t step = 7;
    if(c != 0x20) {
      const uint16_t tile = (uint16_t)(bus_rd16(0x8A0000 | (uint16_t)(0xCB31 + glyph)) | 0x3E80);
      const uint16_t slot = take_slot(DP + 0x8C);
      wr16((uint16_t)(0x0600 + slot), rd16(DP + 0x3A));
      wr16((uint16_t)(0x0700 + slot), rd16(DP + 0x3C));
      wr16((uint16_t)(0x0500 + slot), 0xFFFF);
      wr16((uint16_t)(0x0800 + slot), tile);
      step = bus_rd16(0x8A0000 | (uint16_t)(0xB7AC + glyph));
    }
    // (the second add takes the first's carry)
    const uint32_t x = (uint32_t)rd16(DP + 0x3A) + step;
    wr16(DP + 0x3A, (uint16_t)x);
    wr16(DP + 0x8A, (uint16_t)(rd16(DP + 0x8A) + step + (x > 0xFFFF)));
    return rd16(DP + 0x8A);
  }
  wr16(DP + 0x36, rd16(DP + 0x3A));
  wr16(DP + 0x38, rd16(DP + 0x3C));
  const uint16_t at = tilemap_offset();
  const uint16_t tile = (uint16_t)(bus_rd16(0x8A0000 | (uint16_t)(0xCB31 + glyph)) | 0x3880);
  wr16(0x7F0000 | at, tile);
  wr16(DP + 0x3A, (uint16_t)(rd16(DP + 0x3A) + 1));
  return tile;
}

// TG3000_ROUTINE($8A:B721) vs_put_sprite_char
uint16_t vs_put_sprite_char(uint16_t x, uint16_t y, uint8_t c) {
  wr16(DP + 0x3A, x);
  wr16(DP + 0x3C, y);
  wr16(DP + 0x88, 0x0001);
  const uint16_t a = vs_put_char(c);
  wr16(DP + 0x88, 0);
  return a;
}

// TG3000_ROUTINE($8A:B072) vs_put_string
uint16_t vs_put_string(uint16_t str) {
  wr16(DP + 0x8A, 0);
  wr16(DP + 0x44, str);
  uint16_t a = str;
  for(uint16_t y = 0;; y++) {
    const uint8_t c = bus_rd8(0x8A0000 | (uint16_t)(str + y));
    a = (uint16_t)((a & 0xFF00) | c);
    if(c == 0) return a;
    a = vs_put_char(c);
  }
}

// TG3000_ROUTINE($8A:AB6B) vs_panel_marks
uint16_t vs_panel_marks(uint16_t panel) {
  const uint16_t x = (panel & 1) ? 0x0080 : 0, y = (panel & 2) ? 0x0070 : 0;
  vs_put_sprite_char((uint16_t)(0x0021 + x), (uint16_t)(0x0018 + y), 'I');
  return vs_put_sprite_char((uint16_t)(0x0063 + x), (uint16_t)(0x0048 + y), 'T');
}

// TG3000_ROUTINE($8A:A721) vs_panel_screens_bg1_off (never run)
void vs_panel_screens_bg1_off(uint16_t x) {
  if(x == 0 || x == 4) wr8(DP + 0xFB, 0x12);
  else if(x == 2 || x == 6) wr8(DP + 0xFC, 0x12);
}

// TG3000_ROUTINE($8A:AEB9) vs_letter_prev (never run)
void vs_letter_prev(uint16_t x) {
  const uint16_t was = rd16((uint16_t)(0x0DB4 + x));
  for(;;) {
    if(rd16((uint16_t)(0x0DB4 + x)) == 0) {
      wr16((uint16_t)(0x0DB4 + x), was);
      return;
    }
    wr16((uint16_t)(0x0DB4 + x), (uint16_t)(rd16((uint16_t)(0x0DB4 + x)) - 1));
    if(bus_rd8(0x8A0000 | (uint16_t)(0xAFF1 + vs_letter_index(x))) != 0x20) return;
  }
}

// TG3000_ROUTINE($8A:AF31) vs_row_prev (never run)
void vs_row_prev(uint16_t x) {
  const uint16_t was = rd16((uint16_t)(0x0DAC + x));
  for(;;) {
    if(rd16((uint16_t)(0x0DAC + x)) == 0) {
      wr16((uint16_t)(0x0DAC + x), was);
      return;
    }
    wr16((uint16_t)(0x0DAC + x), (uint16_t)(rd16((uint16_t)(0x0DAC + x)) - 1));
    const uint8_t c = bus_rd8(0x8A0000 | (uint16_t)(0xAFF1 + vs_letter_index(x)));
    if(c != 0x20 && c != 0x3C) return;
  }
}

// TG3000_ROUTINE($8A:ABEF) vs_name_key
//
// DEL, SPACE and a full name run in no recorded movie; they are checked by the fuzz.
void vs_name_key(uint16_t x) {
  const uint16_t pad = rd16((uint16_t)(0x0DCC + x));
  if(!(pad & 0xC0C0)) return;
  wr16((uint16_t)(0x0DCC + x), (uint16_t)((pad & 0xC0C0) ^ 0xFFFF));
  const uint8_t c = bus_rd8(0x8A0000 | (uint16_t)(0xAFF1 + rd16(DP + 0x4E)));
  const uint16_t len = (uint16_t)(0x0DD4 + x);
  if(c == 0x65) {  // END
    vs_name_put(x, 0);
    if(vs_cursor_moving(x)) return;
    vs_panel_text_clear(x);
    wr16((uint16_t)(0x0D38 + x), rd16((uint16_t)(0x0DDC + x)));
    wr16((uint16_t)(0x0DDC + x), 0xFFFE);
    vs_panel_reset(x);
    vs_panel_screens(x);
    return;
  }
  vs_name_clear(x);
  if(c == 0x64) {  // DEL
    if(rd16(len) == 0) {
      vs_panel_screens_bg1_off(x);
      return;
    }
    wr16(len, (uint16_t)(rd16(len) - 1));
    vs_name_put(x, 0);
    return;
  }
  if(rd16(len) == 0x000C) {  // full
    vs_panel_screens_bg1_off(x);
    return;
  }
  if(c == 0x73) {  // SPACE
    vs_name_clear(x);
    vs_name_put(x, ' ');
  } else {
    vs_name_put(x, c);
  }
  wr16(len, (uint16_t)(rd16(len) + 1));
}

// TG3000_ROUTINE($8A:AD74) vs_name_entry_frame
//
// The player's Up and Left and their routines run in no recorded movie; they are checked by
// the fuzz.
void vs_name_entry_frame(uint16_t x) {
  const uint16_t wait = (uint16_t)(0x0DE4 + x);
  if(rd16(wait) != 0) {
    wr16(wait, (uint16_t)(rd16(wait) - 1));
    return;
  }
  if(rd16((uint16_t)(0x0DDC + x)) == 0xFFFF) {  // a player
    const uint16_t pad = rd16((uint16_t)(0x0DCC + x));
    wr16(DP + 0x42, pad);
    const bool rowArrived = rd16((uint16_t)(0x0D90 + x)) == rd16((uint16_t)(0x0D68 + x));
    if((pad & 0x0800) && rowArrived) vs_letter_prev(x);
    if((pad & 0x0400) && rd16((uint16_t)(0x0D90 + x)) == rd16((uint16_t)(0x0D68 + x))) vs_letter_next(x);
    if((pad & 0x0100) && rd16((uint16_t)(0x0D88 + x)) == rd16((uint16_t)(0x0D80 + x))) vs_row_next(x);
    if((pad & 0x0200) && rd16((uint16_t)(0x0D88 + x)) == rd16((uint16_t)(0x0D80 + x))) vs_row_prev(x);
    return;
  }
  if(vs_cursor_moving(x)) return;
  wr16((uint16_t)(0x0DCC + x), (uint16_t)(rd16((uint16_t)(0x0DCC + x)) | 0xC0C0));
  bool carry;
  uint16_t r = (uint16_t)(rng_next((uint16_t)(DP + 0x10), &carry) & 0x0003);
  if(r == 0) r = 2;
  wr16(wait, (uint16_t)(r << 4));
  const uint16_t name = table8a(0xA80C, x);
  wr16(DP + 0x54, name);
  const uint8_t c = bus_rd8(0x8A0000 | (uint16_t)(name + rd16((uint16_t)(0x0DDC + x))));
  if(c != 0) {
    vs_letter_find(x, c);
    wr16((uint16_t)(0x0DDC + x), (uint16_t)(rd16((uint16_t)(0x0DDC + x)) + 1));
    return;
  }
  wr16((uint16_t)(0x0DAC + x), 0x0004);
  wr16((uint16_t)(0x0DB4 + x), 0x0004);
  vs_name_put(x, 0);
}

// TG3000_ROUTINE($8A:AA71) vs_car_title
uint16_t vs_car_title(uint16_t x) {
  wr16(DP + 0x3A, table8a(0xAA92, x));
  wr16(DP + 0x3C, table8a(0xAA9A, x));
  return vs_put_string(0xCE5B);
}

// TG3000_ROUTINE($8A:B037) vs_grid_draw
uint16_t vs_grid_draw(uint16_t x, uint16_t y) {
  wr16(DP + 0x3A, x);
  wr16(DP + 0x3C, y);
  uint16_t a = vs_put_string(0xAFAB);
  for(uint16_t row = 1; row < 5; row++) {
    wr16(DP + 0x3A, x);
    wr16(DP + 0x3C, (uint16_t)(rd16(DP + 0x3C) + 2));
    a = vs_put_string((uint16_t)(0xAFAB + 14 * row));
  }
  return a;
}

// TG3000_ROUTINE($8A:AAC1) vs_panel_name_draw
//
// A panel other than 0-6 draws nothing (never run).
uint16_t vs_panel_name_draw(uint16_t x) {
  static const uint16_t names[4] = {0x03E4, 0x03F1, 0x03FE, 0x040B};
  wr16(DP + 0x88, 0x0001);
  if(x > 6 || (x & 1)) {
    wr16(DP + 0x88, 0);
    return 0x0001;
  }
  wr16(DP + 0x3C, (uint16_t)(table8a(0xACDC, x) << 3));
  wr16(DP + 0x3A, (uint16_t)(table8a(0xACD4, x) << 3));
  vs_put_string(names[x >> 1]);
  vs_panel_sprite(x);
  wr16(DP + 0x88, 0);
  return vs_panel_marks((uint16_t)(x >> 1));
}

// TG3000_ROUTINE($8A:A852) vs_car_confirm
void vs_car_confirm(uint16_t x) {
  if(vs_cursor_moving(x)) return;
  if(rd16((uint16_t)(0x0D38 + x)) == 0xFFFF && !(rd16((uint16_t)(0x0DCC + x)) & 0xC0C0)) return;
  vs_panel_text_clear(x);
  vs_name_default(x);
  wr16((uint16_t)(0x0DDC + x), 0xFFFD);
  vs_panel_screens(x);
  const uint16_t panel = (x <= 6 && !(x & 1)) ? (uint16_t)(x >> 1) : 0;
  wr8((uint16_t)(0x0418 + panel), (uint8_t)rd16((uint16_t)(0x0DB4 + x)));
  vs_ready_check();
}

// TG3000_ROUTINE($8A:A869) vs_ready_draw
void vs_ready_draw(uint16_t x) {
  wr16(DP + 0x3A, table8a(0xAA92, x));
  wr16(DP + 0x3C, (uint16_t)(table8a(0xAA9A, x) + 4));
  if(!(rd16(DP + 0x66) & 1)) {
    vs_panel_text_clear(x);
    return;
  }
  const uint16_t at = rd16(DP + 0x3A);
  const uint16_t name = table8a(0xA804, x);
  vs_text_center(name);
  vs_put_string(name);
  wr16(DP + 0x3A, (uint16_t)(at + 4));
  wr16(DP + 0x3C, (uint16_t)(rd16(DP + 0x3C) + 2));
  vs_put_string(0xA9A1);
}

// TG3000_ROUTINE($8A:A9A7) vs_car_info
uint16_t vs_car_info(uint16_t x) {
  const uint16_t col = rd16((uint16_t)(0x0DB4 + x));
  if(col == rd16((uint16_t)(0x0DAC + x))) return col;
  wr16((uint16_t)(0x0DAC + x), col);
  const uint16_t text = table8a(0xCE63, (uint16_t)(col << 1));
  wr16(DP + 0x42, text);
  const uint16_t left = table8a(0xAA92, x), top = table8a(0xAA9A, x);
  vs_panel_text_clear(x);
  uint16_t a = vs_car_title(x);
  wr16(DP + 0x3C, (uint16_t)(top + 1));
  wr16(DP + 0x3A, left);
  for(uint16_t y = 0;; y++) {
    const uint8_t c = bus_rd8(0x8A0000 | (uint16_t)(text + y));
    if(c == 0xFF) return (uint16_t)((a & 0xFF00) | 0xFF);
    if(c == 0) {
      a = left;
      wr16(DP + 0x3A, left);
      wr16(DP + 0x3C, (uint16_t)(rd16(DP + 0x3C) + 1));
      continue;
    }
    a = vs_put_char(c);
  }
}

// TG3000_ROUTINE($8A:A81C) vs_panel_frame
void vs_panel_frame(uint16_t x) {
  const uint16_t state = rd16((uint16_t)(0x0DDC + x));
  if(state == 0xFFFE) {
    vs_cursor_step(x);
    vs_cursor_column(x);
    vs_car_info(x);
    vs_car_confirm(x);
    sprite_frame(x);
    return;
  }
  if(state == 0xFFFD) {
    vs_ready_draw(x);
    return;
  }
  vs_panel_name_draw(x);
  vs_name_entry_frame(x);
  vs_letter_index(x);
  vs_letter_width(x);
  vs_cursor_target(x);
  sprite_frame(x);
  vs_name_key(x);
  vs_caret_blink(x);
}

// TG3000_ROUTINE($8A:B9AF) vs_graphics_load
void vs_graphics_load(void) {
  decompress_to_vram(DP, 0x8D, 0xF696, 0x4000);
  uint16_t end;
  decompress(DP, 0x8A, 0xC86C, 0x7F, 0x0000, &end);
  uint16_t at = 0x7000;
  for(int i = 0; i < 0x400; i++) video_write_vram_word(at++, 0x0023);
  at = 0x7000;
  static const uint16_t bits[2][2] = {{0x0000, 0x0400}, {0x0800, 0x0C00}};
  for(int half = 0; half < 2; half++) {
    wr16(DP + 0x44, 0x000E);
    wr16(DP + 0x42, 0);
    for(int row = 0; row < 14; row++) {
      for(int copy = 0; copy < 2; copy++)
        for(uint16_t k = 0; k < 0x20; k += 2)
          video_write_vram_word(at++, (uint16_t)(rd16(0x7F0000 | (uint16_t)(rd16(DP + 0x42) + k)) | bits[half][copy]));
      wr16(DP + 0x42, (uint16_t)(rd16(DP + 0x42) + 0x20));
      wr16(DP + 0x44, (uint16_t)(rd16(DP + 0x44) - 1));
    }
  }
}

// TG3000_ROUTINE($8A:A576) vs_screen_blank
void vs_screen_blank(void) {
  video_hdma_enable(0);
  platform_set_rom_speed(0);  // (the 16-bit STZ reaches $420D too)
  wr16(DP + 0x8E, 0);
}

// TG3000_ROUTINE($8A:A582) vs_screen_setup
void vs_screen_setup(void) {
  vs_palette_load();
  vs_graphics_load();
  video_set_color(0, bus_rd16(0x8AC4F4));
  video_set_bg1_vofs(bus_rd16(0x8AA766));
  video_set_bg1_hofs(bus_rd16(0x8AA764));
  video_set_bg_mode(0x01);
  wr16(DP + 0x4C, 0x0002);
  clear_tilemap_7f();
  for(uint16_t i = 0; i < 8; i += 2) wr16((uint16_t)(0x0D60 + i), 0x007C);
  for(uint16_t i = 0; i < 8; i += 2) {
    wr16((uint16_t)(0x0D78 + i), 0x0002);
    wr16((uint16_t)(0x0D70 + i), 0x0002);
  }
  wr16(DP + 0x7A, 0x0002);
  wr16(DP + 0x7C, 0x0002);
  wr16(DP + 0x7E, 0x0001);
  static const uint16_t corner[4][2] = {{0x13, 0x06}, {0x93, 0x06}, {0x13, 0x76}, {0x93, 0x76}};
  for(int p = 0; p < 4; p++) {
    wr16((uint16_t)(0x0DBC + 2 * p), corner[p][0]);
    wr16((uint16_t)(0x0DC4 + 2 * p), corner[p][1]);
  }
  vs_grid_draw(0x0002, 0x0001);
  vs_grid_draw(0x0012, 0x0001);
  vs_grid_draw(0x0002, 0x000F);
  vs_grid_draw(0x0012, 0x000F);
  for(uint16_t i = 0; i < 8; i += 2) {
    wr16((uint16_t)(0x0DAC + i), 0x0004);
    wr16((uint16_t)(0x0DB4 + i), 0x0004);
  }
  video_set_main_screen(0x13);
  video_set_sub_screen(0x00);  // (the 16-bit store reaches $212D too)
  vs_names_load();
  vs_players_in();
  wr16(DP + 0x8E, (uint16_t)(rd16(DP + 0x8E) + 1));
  fade_in_start();
}

// TG3000_ROUTINE($8A:A6F6) vs_names_absent
void vs_names_absent(void) {
  for(int x = 6; x >= 0; x -= 2) {
    if(rd16((uint16_t)(0x0DF4 + x)) != 0) continue;
    const uint16_t to = table8a(0xA804, (uint16_t)x), from = table8a(0xA80C, (uint16_t)x);
    wr16(DP + 0x42, to);
    wr16(DP + 0x44, from);
    for(uint16_t y = 0;; y++) {
      const uint8_t c = bus_rd8(0x8A0000 | (uint16_t)(from + y));
      bus_wr8(0x8A0000 | (uint16_t)(to + y), c);
      if(c == 0) break;
    }
  }
}

// TG3000_ROUTINE($8A:A682) vs_screen_frame
//
// Only the first player's pad and the frames that go on run in a movie (it stops before the
// race); the rest is checked by the fuzz.
bool vs_screen_frame(void) {
  static const uint16_t pads[4] = {0x000E, 0x0012, 0x0016, 0x001A};
  for(uint16_t p = 0; p < 4; p++)
    if(rd16((uint16_t)(0x0DF4 + 2 * p)) != 0) wr16((uint16_t)(0x0DCC + 2 * p), rd16(pads[p]));
  clear_object_slots();
  for(uint16_t x = 0; x < 8; x += 2) vs_panel_frame(x);
  for(uint16_t p = 0; p < 4; p++) wr16(pads[p], rd16((uint16_t)(0x0DCC + 2 * p)));
  if(rd16(0x019A) != 0) return true;
  vs_names_absent();
  video_set_main_screen(0x13);
  video_set_sub_screen(0x00);
  video_hdma_enable(0x82);
  platform_set_rom_speed(0);
  return false;
}

// The VS screen's frames ($8A:A67B on): a frame at a time until the fade out its players'
// readiness started has ended.
void vs_screen_loop(void) {
  do wait_frames(1);
  while(vs_screen_frame());
}

// The VS screen whole ($8A:A573): the fade out, a frame, the setup, then its frames.
void vs_screen(void) {
  fade_out_wait();
  vs_screen_blank();
  wait_frames(1);
  vs_screen_setup();
  vs_screen_loop();
}

// The VS race's four players: records at $0200, $0262, $02B3... ($0225, $0287, $02D8, $033A:
// their D + $25). The race's distance of the record at D: D+$25/D+$27 (32 bits) and D+$35.

// TG3000_ROUTINE($82:9B62) vs_cars_ahead
// How many of the four players' cars are ahead of the record at D: its distance minus
// theirs as one subtraction over D+$25, D+$27 and D+$35, whose borrow in is the CPX of the
// player's index (X: 1-4) against the car's (4, 3, 2, 1), so that an equal distance counts
// for the lower index. Returns the count; *a and *carry / *overflow the last SBC's.
uint16_t vs_cars_ahead(uint16_t dp, uint16_t x, uint16_t* a, bool* carry, bool* overflow) {
  static const uint16_t cars[4] = {0x0225, 0x02D8, 0x0287, 0x033A};
  static const uint16_t index[4] = {4, 3, 2, 1};
  uint16_t count = 0;
  for(int i = 0; i < 4; i++) {
    bool c = x >= index[i];
    const uint16_t at = cars[i];
    const uint16_t pair[3][2] = {{rd16((uint16_t)(dp + 0x25)), rd16(at)},
                                 {rd16((uint16_t)(dp + 0x27)), rd16((uint16_t)(at + 2))},
                                 {rd16((uint16_t)(dp + 0x35)), rd16((uint16_t)(at + 0x10))}};
    uint16_t r = 0;
    for(int k = 0; k < 3; k++) {
      const uint32_t d = (uint32_t)pair[k][0] - pair[k][1] - (c ? 0 : 1);
      r = (uint16_t)d;
      *overflow = ((pair[k][0] ^ pair[k][1]) & (pair[k][0] ^ r) & 0x8000) != 0;
      c = d <= 0xFFFF;
    }
    *a = r;
    *carry = c;
    if(r & 0x8000) count++;
  }
  return count;
}

// TG3000_ROUTINE($81:F68B) vs_cars_ring
// The four players' cars linked in a ring of next pointers at $7E:FACD + car: $1E6A to
// $1E6C to $1E6E to $1E70 and back to $1E6A.
void vs_cars_ring(void) {
  static const uint16_t order[5] = {0x1E6A, 0x1E6C, 0x1E6E, 0x1E70, 0x1E6A};
  for(int i = 0; i < 4; i++) wr16((uint16_t)(0xFACD + rd16(order[i])), rd16(order[i + 1]));
}

// TG3000_ROUTINE($82:97BE) vs_cars_link
// The four cars in the order at $1E6A, $1E6C, $1E6E, $1E70, each linked to the one before it
// ($7E:FACF + car) and the one after it ($7E:FACD + car), around the ring, car by car.
uint16_t vs_cars_link(void) {
  uint16_t car[4];
  for(int i = 0; i < 4; i++) car[i] = rd16((uint32_t)(0x1E6A + 2 * i));
  for(int i = 0; i < 4; i++) {
    wr16(0xFACF + (uint32_t)car[i], car[(i + 3) & 3]);
    wr16(0xFACD + (uint32_t)car[i], car[(i + 1) & 3]);
  }
  return car[3];
}

// TG3000_ROUTINE($83:E878) vs_car_parts
// The car at D's engine words ($1EA8, $1EC0) and tyre words ($1EB4, $1EB6), from the tables
// player_frame_setup uses, each part chosen by the car's entry (D+$14) in its own table.
uint16_t vs_car_parts(uint16_t dp) {
  const uint32_t car = rd16((uint16_t)(dp + 0x14));
  const uint32_t engine = (uint32_t)bus_rd8(0x86DD6D + car) << 1;
  wr16(0x1EA8, bus_rd16(0x86DCE1 + engine));
  wr16(0x1EC0, bus_rd16(0x86DCED + engine));
  const uint32_t tyres = (uint32_t)bus_rd8(0x86DD7D + car) << 1;
  wr16(0x1EB4, bus_rd16(0x86DD29 + tyres));
  wr16(0x1EB6, bus_rd16(0x86DD35 + tyres));
  return (uint16_t)tyres;
}

// TG3000_ROUTINE($82:BD0E) vs_grid_setup
// The VS race's four cars' records ($7E:FAB5 + $35 a car) set for the start, as
// race_grid_setup does the one-player race's twenty:
// - the distance and the words at +$08, +$0A and +$10 cleared, +$29 = 2;
// - the place on the track (+$01) from $019A - 4 up, one a car;
// - the lane (+$1C) from $0800 by $0200;
// - the kind (+$10) from 8 by 2.
// The record after the fourth gets place $7FFF and kind 6 (an end mark). The race order at
// $1E6A is the four cars in turn, and its other 16 words are that end record. D+$14, D+$16
// and D+$18 hold the three counters, as the original leaves them.
void vs_grid_setup(uint16_t dp) {
  wr16((uint16_t)(dp + 0x14), (uint16_t)(rd16(0x019A) - 4));
  wr16((uint16_t)(dp + 0x16), 0x0800);
  wr16((uint16_t)(dp + 0x18), 0x0008);
  uint32_t x = 0;
  for(int i = 0; i < 4; i++, x += 0x35) {
    static const uint16_t cleared[] = {0xFAB5, 0xFAB7, 0xFABD, 0xFABF, 0xFAC5};
    for(unsigned k = 0; k < sizeof cleared / sizeof cleared[0]; k++) wr16(cleared[k] + x, 0);
    wr16(0xFADE + x, 2);
    wr16(0xFAB6 + x, rd16((uint16_t)(dp + 0x14)));
    wr16((uint16_t)(dp + 0x14), (uint16_t)(rd16((uint16_t)(dp + 0x14)) + 1));
    wr16(0xFAD1 + x, rd16((uint16_t)(dp + 0x16)));
    wr16((uint16_t)(dp + 0x16), (uint16_t)(rd16((uint16_t)(dp + 0x16)) + 0x0200));
    wr16(0xFAC5 + x, rd16((uint16_t)(dp + 0x18)));
    wr16((uint16_t)(dp + 0x18), (uint16_t)(rd16((uint16_t)(dp + 0x18)) + 2));
  }
  wr16(0xFAB6 + x, 0x7FFF);
  wr16(0xFAC5 + x, 0x0006);
  for(int i = 0; i < 4; i++) wr16((uint32_t)(0x1E6A + 2 * i), (uint16_t)(0x35 * i));
  for(int i = 0; i < 16; i++) wr16((uint32_t)(0x1E72 + 2 * i), (uint16_t)x);
}

// TG3000_ROUTINE($82:BDDA) vs_step_share
// The CPU's 16 / 8 divider gives the magnitude's quotient by the step count; a negative word
// gives it as it is, a positive one (or 0) gives 0 less it. The sign is the caller's N flag.
uint16_t vs_step_share(uint16_t a, bool negative) {
  const uint8_t count = rd8(0x1E9E);
  if(negative) {
    const uint16_t m = (uint16_t)(0 - a);
    return count ? (uint16_t)(m / count) : 0xFFFF;
  }
  const uint16_t q = count ? (uint16_t)(a / count) : 0xFFFF;
  return (uint16_t)(0 - q);
}

// TG3000_ROUTINE($82:BDA9) vs_steps_share
// For the four cars in turn (records $0200, $02B3, $0262, $0315): its word ($01CE, $01D2,
// $01D6, $01DA) to the record's +$29, then the word becomes vs_step_share of itself.
uint16_t vs_steps_share(void) {
  static const uint16_t word[4] = {0x01CE, 0x01D2, 0x01D6, 0x01DA};
  static const uint16_t saved[4] = {0x0229, 0x02DC, 0x028B, 0x033E};
  uint16_t a = 0;
  for(int i = 0; i < 4; i++) {
    a = rd16(word[i]);
    wr16(saved[i], a);
    a = vs_step_share(a, (a & 0x8000) != 0);
    wr16(word[i], a);
  }
  return a;
}
