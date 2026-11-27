// SPDX-License-Identifier: GPL-3.0-or-later
#include "vs.h"

#include <stdbool.h>

#include "misc.h"
#include "text.h"
#include "dsp4.h"
#include "track.h"
#include "decompress.h"
#include "frame.h"
#include "platform.h"
#include "race_frame.h"
#include "rng.h"
#include "sound.h"
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
  vs_strip_load(0x7F0000 + 2u * (0x57C0 - 0x4000));  // the progress strip, before the buffer is reused
  uint16_t end;
  decompress(DP, 0x8A, 0xC86C, 0x7F, 0x0000, &end);
  uint16_t at = 0x7000;
  video_note_vs_panels(0x7000, 0x7F0000);  // (port request J)
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

// (vs-race-13: first drafted by a delegated agent from the listing, then completed and checked here)
static bool sub_overflow(uint16_t a, uint16_t b, uint16_t r) { return ((a ^ b) & (a ^ r) & 0x8000) != 0; }
static bool add_overflow(uint16_t a, uint16_t b, uint16_t r) { return (~(a ^ b) & (a ^ r) & 0x8000) != 0; }
static uint16_t car_word(uint32_t base, uint16_t car) { return rd16(base + car); }

// Whether car Y is less than $40 sideways ($FABB) from car X, either way.
static bool cars_close_sideways(uint16_t x, uint16_t y, uint16_t* gap) {
  const uint16_t l = (uint16_t)(car_word(0xFABB, y) - car_word(0xFABB, x));
  *gap = l;
  return (l & 0x8000) ? !((uint16_t)(l - 0xFFC0) & 0x8000) : ((uint16_t)(l - 0x40) & 0x8000) != 0;
}

// One of $81:F7A5's two 40-step shift-subtract divisions, as the original runs it: the 32-bit
// word pair at $020E/$0210 + `dividendAt`, doubled, by the word at $022B + `divisorAt` (D+$24).
// Steps 33-40 keep dividing the quotient's own bits, and the remainder wraps at 16 bits (the
// carry out of its ROL is dropped), so it is not a plain division. D+$20/D+$22 get the result.
static void cars_collide_divide(uint16_t divisorAt, uint16_t dividendAt) {
  const uint16_t divisor = rd16(0x022B + (uint32_t)divisorAt);
  wr16(DP + 0x24, divisor);
  uint16_t q0 = rd16(0x020E + (uint32_t)dividendAt);
  const unsigned c0 = q0 >> 15;
  q0 = (uint16_t)(q0 << 1);
  uint16_t q1 = rd16(0x0210 + (uint32_t)dividendAt);
  const unsigned c1 = q1 >> 15;
  q1 = (uint16_t)((q1 << 1) | c0);
  uint16_t a = (uint16_t)c1;
  for(int n = 0x28; n; n--) {
    const unsigned fits = a >= divisor;
    if(fits) a = (uint16_t)(a - divisor);
    const unsigned out0 = q0 >> 15, out1 = q1 >> 15;
    q0 = (uint16_t)((q0 << 1) | fits);
    q1 = (uint16_t)((q1 << 1) | out0);
    a = (uint16_t)((a << 1) | out1);
  }
  wr16(DP + 0x20, q0);
  wr16(DP + 0x22, q1);
}

// $81:F7A5 (the shared tail's far case: the cars X and Y collide; drafted by a delegated agent,
// checked here). Their players' words at $0216/$0218 get the other's $020E/$0210 doubled over
// their own $022B (cars_collide_divide, Y's first). Car X's distance becomes Y's less $100, but
// not below its saved $FAC1/$FAC3 (never in the movies); D+$1C/$1E the distance X lost, which
// X's player's race distance ($0225/$0227) also loses unless its $0244 bit 0 is set. Ends with
// C set and X, Y pulled.
static void cars_collide(uint16_t pulledX, VsCarsExit* e) {
  const uint16_t px = rd16(DP + 0x18), py = rd16(DP + 0x1A);
  cars_collide_divide(py, px);
  wr16(0x0216 + (uint32_t)py, rd16(DP + 0x20));
  wr16(0x0218 + (uint32_t)py, rd16(DP + 0x22));
  const uint16_t px2 = rd16(DP + 0x18);
  cars_collide_divide(px2, py);
  wr16(0x0216 + (uint32_t)px2, rd16(DP + 0x20));
  wr16(0x0218 + (uint32_t)px2, rd16(DP + 0x22));
  const uint16_t cx = rd16(DP + 0x14), cy = rd16(DP + 0x16);
  wr16(DP + 0x1C, car_word(0xFAB5, cx));
  wr16(DP + 0x1E, car_word(0xFAB7, cx));
  const uint16_t yLo = car_word(0xFAB5, cy);
  wr16(0xFAB5 + (uint32_t)cx, (uint16_t)(yLo - 0x0100));
  wr16(0xFAB7 + (uint32_t)cx, (uint16_t)(car_word(0xFAB7, cy) - (yLo >= 0x0100 ? 0 : 1)));
  const bool notBelow = car_word(0xFAB5, cx) >= car_word(0xFAC1, cx);
  if((uint16_t)(car_word(0xFAB7, cx) - car_word(0xFAC3, cx) - (notBelow ? 0 : 1)) & 0x8000) {  // never run
    wr16(0xFAB5 + (uint32_t)cx, car_word(0xFAC1, cx));
    wr16(0xFAB7 + (uint32_t)cx, car_word(0xFAC3, cx));
  }
  const uint16_t oldLo = rd16(DP + 0x1C), newLo = car_word(0xFAB5, cx);
  wr16(DP + 0x1C, (uint16_t)(oldLo - newLo));
  const uint16_t oldHi = rd16(DP + 0x1E), newHi = car_word(0xFAB7, cx);
  uint16_t acc = (uint16_t)(oldHi - newHi - (oldLo >= newLo ? 0 : 1));
  bool overflow = sub_overflow(oldHi, newHi, acc);
  wr16(DP + 0x1E, acc);
  const uint16_t player = rd16(DP + 0x18);
  acc = rd16(0x0244 + (uint32_t)player);
  if(!(acc & 0x0001)) {
    const uint16_t lostLo = rd16(DP + 0x1C), lostHi = rd16(DP + 0x1E);
    const uint16_t lo = rd16(0x0225 + (uint32_t)player);
    wr16(0x0225 + (uint32_t)player, (uint16_t)(lo - lostLo));
    const uint16_t hi = rd16(0x0227 + (uint32_t)player);
    acc = (uint16_t)(hi - lostHi - (lo >= lostLo ? 0 : 1));
    overflow = sub_overflow(hi, lostHi, acc);
    wr16(0x0227 + (uint32_t)player, acc);
  }
  *e = (VsCarsExit){.a = acc, .n = (pulledX & 0x8000) != 0, .z = pulledX == 0, .c = true, .v = overflow};
}

// $81:F70B, the two routines' shared tail (X and Y pushed by them): D+$14/$16 = X/Y, D+$18/$1A =
// their cars' words at $81:FDEE (by the record / $35, the CPU divider), D+$1C/$1E = the 32-bit
// gap of their $FAC1/$FAC3. When that gap is under $100 ahead, car X is put $40 to the side of
// car Y, away by their $FAD5 words, and the same to $0206 + D+$18 when that is a player's (not
// negative): C clear. Otherwise the cars collide (cars_collide): C set. Ends with X and Y pulled
// (their N Z of X).
static void cars_push_apart(uint16_t x, uint16_t y, uint16_t pulledX, VsCarsExit* e) {
  wr16(DP + 0x14, x);
  wr16(DP + 0x16, y);
  wr16(DP + 0x18, bus_rd16(0x81FDEE + 2u * (uint32_t)(uint16_t)(x / 0x35)));
  wr16(DP + 0x1A, bus_rd16(0x81FDEE + 2u * (uint32_t)(uint16_t)(y / 0x35)));
  const uint16_t loY = car_word(0xFAC1, y), loX = car_word(0xFAC1, x);
  const uint16_t lo = (uint16_t)(loY - loX);
  const uint16_t hi = (uint16_t)(car_word(0xFAC3, y) - car_word(0xFAC3, x) - (loY < loX ? 1 : 0));
  wr16(DP + 0x1C, lo);
  wr16(DP + 0x1E, hi);
  if(!((uint16_t)(hi - (lo >= 0x100 ? 0 : 1)) & 0x8000)) {
    cars_collide(pulledX, e);
    return;
  }
  const uint16_t side = car_word(0xFABB, y);
  uint16_t v;
  bool overflow;
  if((uint16_t)(car_word(0xFAD5, y) - car_word(0xFAD5, x)) & 0x8000) {
    v = (uint16_t)(side + 0x40);
    overflow = add_overflow(side, 0x40, v);
  } else {
    v = (uint16_t)(side - 0x40);
    overflow = sub_overflow(side, 0x40, v);
  }
  wr16(0xFABB + (uint32_t)x, v);
  const uint16_t slot = rd16(DP + 0x18);
  if(!(slot & 0x8000)) wr16((uint16_t)(0x0206 + slot), v);
  *e = (VsCarsExit){.a = v, .n = (pulledX & 0x8000) != 0, .z = pulledX == 0, .c = false, .v = overflow};
}

// TG3000_ROUTINE($81:F6AE) vs_cars_tie
// Two cars (X, Y) whose high distance words are equal. When their low words ($FAB5) are $100 or
// more apart, $013C = 0 (C clear). Else $013C counts up and D+$14 takes it; then, walking the
// ring from Y ($FACD) while the next car is less than $100 ahead of X and D+$14 counts down: a
// car less than $40 sideways from X is pushed apart (the shared tail); the walk ends with C set
// at a car $100 or more ahead, or with C clear when the count runs out (never in the movies).
VsCarsExit vs_cars_tie(uint16_t x, uint16_t y) {
  const uint16_t dY = car_word(0xFAB5, y), dX = car_word(0xFAB5, x);
  uint16_t d = (uint16_t)(dY - dX);
  bool overflow = sub_overflow(dY, dX, d);
  if(d >= 0x100) {
    wr16(DP + 0x3C, 0);
    const uint16_t t = (uint16_t)(d - 0x100);
    return (VsCarsExit){.a = d, .n = (t & 0x8000) != 0, .z = t == 0, .c = false, .v = overflow};
  }
  const uint16_t count = (uint16_t)(rd16(DP + 0x3C) + 1);
  wr16(DP + 0x3C, count);
  wr16(DP + 0x14, count);
  const uint16_t originalX = x;
  for(;;) {
    uint16_t gap;
    if(cars_close_sideways(x, y, &gap)) {
      VsCarsExit e;
      cars_push_apart(x, y, originalX, &e);
      return e;
    }
    y = car_word(0xFACD, y);
    const uint16_t nY = car_word(0xFAB5, y);
    d = (uint16_t)(nY - dX);
    overflow = sub_overflow(nY, dX, d);
    const VsCarsExit end = {.a = d, .n = (originalX & 0x8000) != 0, .z = originalX == 0, .c = d >= 0x100, .v = overflow};
    if(d >= 0x100) return end;
    const uint16_t left = (uint16_t)(rd16(DP + 0x14) - 1);  // (never run: a 16-bit DEC)
    wr16(DP + 0x14, left);
    if(left == 0) return end;
  }
}

// TG3000_ROUTINE($81:F6F2) vs_cars_apart
// Two cars (X, Y) $40 or more apart sideways: C clear (A the gap, the flags of its CMP with $40 or
// $FFC0). Closer (never in the movies): the shared tail pushes them apart.
VsCarsExit vs_cars_apart(uint16_t x, uint16_t y) {
  const uint16_t sY = car_word(0xFABB, y), sX = car_word(0xFABB, x);
  uint16_t gap;
  if(cars_close_sideways(x, y, &gap)) {
    VsCarsExit e;
    cars_push_apart(x, y, x, &e);
    return e;
  }
  const uint16_t t = (uint16_t)(gap - ((gap & 0x8000) ? 0xFFC0 : 0x40));
  return (VsCarsExit){.a = gap, .n = (t & 0x8000) != 0, .z = t == 0, .c = false, .v = sub_overflow(sY, sX, gap)};
}

// One of the order's compares: car Y (the slot above) against car X by distance (the CMP of the
// low words, the SBC of the high). Equal high words: vs_cars_tie, no swap. Else $013C = 0, and
// when Y is behind, a swap unless vs_cars_apart leaves C set.
static bool vs_order_swap(uint16_t x, uint16_t y) {
  const uint16_t borrow = car_word(0xFAB5, y) < car_word(0xFAB5, x) ? 1 : 0;
  const uint16_t hi = (uint16_t)(car_word(0xFAB7, y) - car_word(0xFAB7, x) - borrow);
  if(hi == 0) {
    (void)vs_cars_tie(x, y);
    return false;
  }
  wr16(DP + 0x3C, 0);
  if(!(hi & 0x8000)) return false;
  return !vs_cars_apart(x, y).c;
}

// The order's slots, last place first: $1E70, $1E6E, $1E6C, $1E6A.
static uint16_t order_at(int k) { return rd16((uint32_t)(0x1E6A + 2 * k)); }
static void order_set(int k, uint16_t car) { wr16((uint32_t)(0x1E6A + 2 * k), car); }

// TG3000_ROUTINE($81:F4AF) vs_cars_order
// (drafted by a delegated agent from the listing; its two never-run lap paths decoded and
// completed here.) Each frame, for the VS race's four cars:
// - the ring of next pointers ($FACD) relinked in the order, last place first;
// - each car's side copied to $FAD5, its distance saved to $FAC1/$FAC3, $FAC7 cleared, and its
//   handler run (cars_handler);
// - an unrolled insertion sort of the order (pairs from the last up, a swapped car compared on
//   down), by vs_order_swap;
// - the laps: the cars at the back that are past the lap length ($019A, in the $FAB6 word, the
//   distance / 256) lose a lap and go to the front of the order; else a first place below 0
//   (and those after it) gains one and goes to the back (never run). The order turns once a car,
//   but only three times for all four. Either way the sort runs again. Ends when no car moved.
VsCarsOrderExit vs_cars_order(void) {
  const uint16_t p0 = order_at(0), p1 = order_at(1), p2 = order_at(2), p3 = order_at(3);
  wr16(0xFACD + (uint32_t)p3, p0);
  wr16(0xFACD + (uint32_t)p2, p3);
  wr16(0xFACD + (uint32_t)p1, p2);
  wr16(0xFACD + (uint32_t)p0, p1);
  for(uint32_t c = 0; c < 4 * 0x35; c += 0x35) {
    wr16(0xFAD5 + c, rd16(0xFABB + c));
    wr16(0xFAC1 + c, rd16(0xFAB5 + c));
    wr16(0xFAC3 + c, rd16(0xFAB7 + c));
    wr16(0xFAC7 + c, 0);
    cars_handler((uint16_t)c);
  }
  for(;;) {  // $81:F549
    wr16(DP + 0x3C, 0);
    int k = 2;  // the pair (k, k + 1)
    uint16_t x = order_at(2);
    while(k >= 0) {
      const uint16_t y = order_at(k + 1);
      if(vs_order_swap(x, y)) {
        order_set(k, y);
        order_set(k + 1, x);
        vs_cars_ring();
        if(k < 2) {  // the car moved up is compared again one slot higher (X kept)
          k++;
          continue;
        }
      }
      k--;
      if(k >= 0) x = order_at(k);
    }
    // the laps ($81:F5CC)
    const uint16_t lap = rd16(0x019A);
    int moved = 0;
    bool overflow = false;
    for(int s = 3; s >= 0; s--) {
      const uint32_t at = 0xFAB6 + (uint32_t)order_at(s);
      const uint16_t w = rd16(at), r = (uint16_t)(w - lap);
      overflow = sub_overflow(w, lap, r);
      if(w < lap) break;
      wr16(at, r);
      if(s > 0) moved++;  // (the original counts no fourth: all four past the lap turn the order 3 times)
    }
    if(moved > 0) {  // $81:F60B: each to the front
      for(int i = 0; i < moved; i++) {
        const uint16_t t = order_at(3);
        order_set(3, order_at(2));
        order_set(2, order_at(1));
        order_set(1, order_at(0));
        order_set(0, t);
      }
      continue;
    }
    const uint16_t first = rd16(0xFAB6 + (uint32_t)order_at(0));
    if(!(first & 0x8000)) return (VsCarsOrderExit){.a = first, .v = overflow};
    for(int s = 0; s < 4; s++) {  // $81:F634 (never run): below 0, a lap back, to the back
      const uint32_t at = 0xFAB6 + (uint32_t)order_at(s);
      if(s > 0 && !(rd16(at) & 0x8000)) break;
      wr16(at, (uint16_t)(rd16(at) + lap));
      if(s < 3) moved++;  // (likewise: no count for the fourth)
    }
    for(int i = 0; i < moved; i++) {
      const uint16_t t = order_at(0);
      order_set(0, order_at(1));
      order_set(1, order_at(2));
      order_set(2, order_at(3));
      order_set(3, t);
    }
  }
}

// v, unless v - limit is not negative (by sign, the original's BMI): then the limit.
static uint16_t cap_signed(uint16_t v, uint16_t limit) { return ((uint16_t)(v - limit) & 0x8000) ? v : limit; }

// One player's block of vs_car_bands: its record, its car's low distance word ($FAB4 + car),
// its speed limit, its speed and band words at $7E:4E2D.. / $4E35...
typedef struct {
  uint16_t rec, carLo, limit, speed, band;
} VsBandPlayer;
static void vs_car_band(const VsBandPlayer* p) {
  const uint32_t product = (uint32_t)dsp4_multiply((int16_t)rd16((uint16_t)(p->rec + 0x13)), 0x0300);
  wr16(DP + 0x14, (uint16_t)((product >> 8) & 0xFF));  // scratch: the product's byte 1
  wr16(p->speed, cap_signed((uint16_t)((uint16_t)(product >> 8) - 0x0B00), p->limit));  // its bits 8-23
  const uint16_t flags = rd16((uint16_t)(p->rec + 0x44));
  if(!(flags & 1)) return;
  // the high word of the 32-bit difference car - player ($FAB4/$FAB6 - +$24/+$26), at least 0
  const uint32_t a = ((uint32_t)rd16((uint32_t)p->carLo + 2) << 16) | rd16(p->carLo);
  const uint32_t b = ((uint32_t)rd16((uint16_t)(p->rec + 0x26)) << 16) | rd16((uint16_t)(p->rec + 0x24));
  uint16_t far = (uint16_t)((a - b) >> 16);
  if(far & 0x8000) far = 0;  // (never run)
  wr16(DP + 0x14, far);
  uint16_t near = (uint16_t)(0x58 - far);
  if(near & 0x8000) near = 0;  // (never run)
  wr16(p->band, (uint16_t)(near & 0xF8));
  if(!(flags & 4)) return;
  uint16_t w = (uint16_t)(rd16(p->speed) + 0x0B00);  // (never run from here)
  if(w & 0x8000) w = 0;
  uint16_t d = (uint16_t)(rd16((uint32_t)p->carLo + 2) - rd16((uint16_t)(p->rec + 0x26)));
  if(d & 0x8000) d = 0;
  d = (uint16_t)(d + 0x10);
  if(d >= 0xD0) d = 0xD0;
  const uint16_t q = (uint16_t)(w / (uint8_t)d);  // the CPU's 16 / 8 divider
  wr16(p->speed, cap_signed((uint16_t)((uint16_t)(q << 4) - 0x0B00), p->limit));
}

// TG3000_ROUTINE($82:9806) vs_car_bands
// (drafted by a delegated agent; its never-run parts decoded here.) func_82AC52 for the VS race's
// four players in turn (records $0200, $02B3, $0262, $0315; cars 0, $35, $6A, $9F): the speed word
// from the DSP-4 product of the player's +$13 and $0300 (bits 8-23, less $0B00), capped at the
// player's limit ($3100, $2E00, $3100, $2D80); then, when its +$44 bit 0 is set, the band word
// ($58 less the car's lead over the player in its high distance word, at least 0, rounded down to
// 8), and with bit 2 the speed again from the lead (the CPU divider). The one-player race's has the
// limit $3000 and its band from twice the lead.
void vs_car_bands(void) {
  static const VsBandPlayer players[4] = {
    {0x0200, 0xFAB4, 0x3100, 0x4E2D, 0x4E35},
    {0x02B3, 0xFAE9, 0x2E00, 0x4E2F, 0x4E37},
    {0x0262, 0xFB1E, 0x3100, 0x4E31, 0x4E39},
    {0x0315, 0xFB53, 0x2D80, 0x4E33, 0x4E3B},
  };
  for(int i = 0; i < 4; i++) vs_car_band(&players[i]);
}

// One player's block of vs_hud_digits: its record's words, the HUD words it writes, and its
// message's state.
typedef struct {
  uint16_t flags, near, speed, level;    // its record's +$44, +$13, +$10, +$40
  uint16_t bar, hundreds, tens, ones;    // the HUD words written (D-page offsets in WRAM)
  uint16_t offset, base;                 // the offset word, and the base added to it
  uint16_t word, slot, buf, line, tail;  // $7E:4E3D.., its message slot, its line, $7E:4E45.., $7E:4E4D..
} VsHudPlayer;

static uint16_t hud_digit_tile(uint16_t i) { return bus_rd16(0x839F54 + (uint32_t)(uint16_t)(2 * i)); }  // DB $83
static uint16_t product_high(uint16_t a, uint16_t b) { return (uint16_t)((uint32_t)dsp4_multiply((int16_t)a, (int16_t)b) >> 16); }

static void vs_hud_player(const VsHudPlayer* p) {
  if(!(rd16(p->flags) & 0x0004)) {  // (bit 2 set skips to the message: never run)
    uint16_t a = product_high(rd16(p->near), 0x0160);
    if(a & 0x8000) a = (uint16_t)(0 - a);  // (never run)
    if(a >= 0x000C) a = 0x000C;
    wr16(p->bar, a);
    wr16(p->hundreds, bus_rd16(0x839F52));  // the blank
    wr16(p->tens, bus_rd16(0x839F52));
    uint16_t s = rd16(p->speed);
    if(s & 0x8000) s = (uint16_t)(0 - s);  // (never run)
    a = product_high((uint16_t)(s << 3), 0x4000);
    if(a >= 10) {
      if(a >= 100) {  // the CPU divider, an 8-bit divisor
        wr16(p->hundreds, hud_digit_tile((uint16_t)(a / 100)));
        a = (uint16_t)(a % 100);
      }
      wr16(p->tens, hud_digit_tile((uint16_t)(a / 10)));
      a = (uint16_t)(a % 10);
    }
    wr16(p->ones, hud_digit_tile(a));
    uint16_t v = (uint16_t)(0x55 - product_high(rd16(p->level), rd16(0x019E)));
    if(v & 0x8000) v = 0;  // (never run)
    wr16(p->offset, (uint16_t)(v + p->base));
  }
  wr16(DP + 0x14, rd16(p->word));
  wr16(DP + 0x16, rd16(p->slot));
  wr16(DP + 0x18, p->buf);
  wr16(DP + 0x1A, rd16(p->line));
  wr16(DP + 0x1C, rd16(p->tail));
  wr16(DP + 0x3A, 0x0C00);
  vs_message_update();
  wr16(p->word, rd16(DP + 0x14));
  wr16(p->slot, rd16(DP + 0x16));
  wr16(p->line, rd16(DP + 0x1A));
  wr16(p->tail, rd16(DP + 0x1C));
}

// TG3000_ROUTINE($82:9C39) vs_hud_digits
// (drafted by a delegated agent; its never-run parts decoded here.) The VS race's HUD for the
// four players in turn (records $0200, $02B3, $0262, $0315), unless the record's +$44 bit 2 is set:
// the bar (the high word of +$13 times $0160, at most $0C), the speed's three digit tiles
// ($83:9F54; blank $83:9F52 for leading zeros; from the high word of 8 x +$10 times $4000, on the
// CPU divider), the offset word ($55 less the high word of +$40 times $019E, at least 0, plus the
// view's base). Then, always, the player's message, updated in the VS font (vs_message_update)
// through the D-page words.
void vs_hud_digits(void) {
  static const VsHudPlayer players[4] = {
    {0x0244, 0x0213, 0x0210, 0x0240, 0x003E, 0x0042, 0x0044, 0x0046, 0x00A4, 0x57A1, 0x4E3D, 0x0233, 0x4E61, 0x4E45, 0x4E4D},
    {0x02F7, 0x02C6, 0x02C3, 0x02F3, 0x0040, 0x004E, 0x0050, 0x0052, 0x00AC, 0x57A1, 0x4E3F, 0x02E6, 0x4E95, 0x4E47, 0x4E4F},
    {0x02A6, 0x0275, 0x0272, 0x02A2, 0x0064, 0x0048, 0x004A, 0x004C, 0x00A6, 0x57F9, 0x4E41, 0x0295, 0x4EC9, 0x4E49, 0x4E51},
    {0x0359, 0x0328, 0x0325, 0x0355, 0x0068, 0x0054, 0x0056, 0x0058, 0x00AE, 0x57F9, 0x4E43, 0x0348, 0x4EFD, 0x4E4B, 0x4E53},
  };
  for(int i = 0; i < 4; i++) vs_hud_player(&players[i]);
}

// TG3000_ROUTINE($82:B902) vs_track_objects_stream
// (drafted by a delegated agent; its never-run spots decoded here.) The VS race's
// track_objects_stream for one of the four views, set up by its caller: D+$36 the player's
// distance, D+$38 the list index (kept as it moves), D+$3A the view's VRAM word base, D+$3C its
// block in the slot tables ($7E:45EC / $7E:495C, $DC bytes a view), D+$34 a bias added to the
// slots stored. `first` goes to the block's first word. Then, while the player has reached the
// record at $7E:5B91 + the index (5 bytes each: start, slot, kind):
// - kind 0 (never run): the block's first word = 2 x slot + the bias;
// - else the kind's graphic is uploaded to VRAM at the base + 32 x slot (upload_vram_now), and
//   its table entry (the high byte picks the table, the low byte the entry) = 2 x slot + bias.
// Unlike the one-player routine, a kind with no graphic (a zero table entry) is not skipped. The
// original's check that the VRAM word is within $1000-$2FFF does nothing: its fail branch is a NOP.
void vs_track_objects_stream(uint16_t first) {
  wr16(0x45EC + (uint32_t)rd16(DP + 0x3C), first);
  uint16_t x = rd16(DP + 0x38);
  if(rd16(DP + 0x36) < rd16(0x5B91 + (uint32_t)x)) return;  // (never run)
  for(;;) {
    const uint16_t kind = rd16(0x5B94 + (uint32_t)x), slot = rd8(0x5B93 + (uint32_t)x);
    if(kind == 0) {  // (never run)
      wr16(0x45EC + (uint32_t)rd16(DP + 0x3C), (uint16_t)((slot << 1) + rd16(DP + 0x34)));
    } else {
      const bool high = (kind & 0xFF00) != 0;
      const uint16_t t = bus_rd16((high ? 0x86AC36 : 0x86AE36) + (uint32_t)(uint16_t)(kind << 1));
      wr16(DP + 0x14, bus_rd16((high ? 0x860022 : 0x86001E) + (uint32_t)t));
      wr16(DP + 0x16, bus_rd16((high ? 0x860024 : 0x860020) + (uint32_t)t));
      const uint16_t base = bus_rd16(0x860000 + (uint32_t)t);
      wr16(DP + 0x14, (uint16_t)(rd16(0x6591 + (uint32_t)base) + rd16(DP + 0x14)));
      wr16(DP + 0x18, (uint16_t)((slot << 5) + rd16(DP + 0x3A)));
      wr16(DP + 0x1A, (uint16_t)(slot << 1));
      wr16((high ? 0x495C : 0x45EC) + (uint32_t)(uint16_t)(((kind & 0x00FF) << 1) + rd16(DP + 0x3C)),
           (uint16_t)(rd16(DP + 0x1A) + rd16(DP + 0x34)));
      const uint16_t src = rd16(DP + 0x14);
      wr16(DP + 0x14, 0x007F);
      upload_vram_now(rd16(DP + 0x18), 0x7F, src, rd16(DP + 0x16));
    }
    x = (uint16_t)(x + 5);
    wr16(DP + 0x38, x);
    if(rd16(DP + 0x36) < rd16(0x5B91 + (uint32_t)x)) return;
  }
}

// The four views' parameters for vs_track_objects_stream: the slot-table block, the VRAM base,
// the player's distance word (its record's +$26), the bias, the block's first word.
static void vs_track_objects_view(int view) {
  static const struct { uint16_t block, vram, distance, bias, first; } views[4] = {
    {0x0000, 0x0000, 0x0226, 0x0000, 0x0100},
    {0x00DC, 0x0800, 0x02D9, 0x0080, 0x0180},
    {0x01B8, 0x1000, 0x0288, 0x0000, 0x0100},
    {0x0294, 0x1800, 0x033B, 0x0080, 0x0180},
  };
  const uint16_t index = (uint16_t)(0x01A0 + 4 * view);  // D+$A0, D+$A4, D+$A8, D+$AC
  wr16(DP + 0x3C, views[view].block);
  wr16(DP + 0x3A, views[view].vram);
  wr16(DP + 0x36, rd16(views[view].distance));
  wr16(DP + 0x34, views[view].bias);
  wr16(DP + 0x38, rd16(index));
  vs_track_objects_stream(views[view].first);
  wr16(index, rd16(DP + 0x38));
}

// TG3000_ROUTINE($82:B87A) vs_track_objects_view0
void vs_track_objects_view0(void) { vs_track_objects_view(0); }
// TG3000_ROUTINE($82:B89C) vs_track_objects_view1
void vs_track_objects_view1(void) { vs_track_objects_view(1); }
// TG3000_ROUTINE($82:B8BF) vs_track_objects_view2
void vs_track_objects_view2(void) { vs_track_objects_view(2); }
// TG3000_ROUTINE($82:B8DF) vs_track_objects_view3
void vs_track_objects_view3(void) { vs_track_objects_view(3); }

// TG3000_ROUTINE($82:9AF4) vs_places_update
// For the four players in turn (records $0200, $02B3, $0262, $0315; indices 4 to 1): the cars
// ahead of it (vs_cars_ahead). When that differs from its last count ($1ED2 + 2 a player), the
// count is kept and the place's message (6 + the count) requested in the player's slot.
void vs_places_update(void) {
  static const uint16_t recs[4] = {0x0200, 0x02B3, 0x0262, 0x0315};
  static void (*const request[4])(uint16_t) = {race_message_request_0233, race_message_request_02e6,
                                               race_message_request_0295, race_message_request_0348};
  for(int i = 0; i < 4; i++) {
    uint16_t a;
    bool c, v;
    const uint16_t ahead = vs_cars_ahead(recs[i], (uint16_t)(4 - i), &a, &c, &v);
    const uint16_t last = (uint16_t)(0x1ED2 + 2 * i);
    if(ahead == rd16(last)) continue;
    wr16(last, ahead);
    request[i]((uint16_t)(ahead + 6));
  }
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

// TG3000_ROUTINE($82:BA12) vs_race_reset
// (drafted by a delegated agent from the listing; checked here.) The VS race's state reset,
// race_state_reset's counterpart: $0200-$0365 cleared; the four players' words and records'
// fields; the objects' indices; the lap time ($019E: $B0 << 16 over the DSP-4 product of $019A
// and $1F06, a 16-step restoring divide); track_palettes_load; the four cars' records
// (vs_grid_setup), their sides and start places; CGRAM from colour $C1, five rows of 30 table
// bytes ($85:A118) and the last one twice (80 colours: the index wraps past $FF).
void vs_race_reset(void) {
  for(uint16_t x = 0x0166; x != 0; x = (uint16_t)(x - 2)) wr16((uint16_t)(0x01FE + x), 0);
  wr8(0x024E, 4);
  wr8(0x0301, 4);
  wr8(0x02B0, 4);
  wr8(0x0363, 4);
  static const uint16_t z1[] = {0x0231, 0x02E4, 0x0293, 0x0346, 0x0216, 0x02C9, 0x0278, 0x032B};
  for(unsigned i = 0; i < sizeof z1 / sizeof z1[0]; i++) wr16(z1[i], 0);
  wr16(0x02A4, 0x0020);
  wr16(0x1F32, (uint16_t)(rd16(0x1F32) | 0x0080));
  static const uint16_t z2[] = {0x00B4, 0x00B6, 0x00B8, 0x022D, 0x022F, 0x02E0, 0x02E2, 0x028F, 0x0291, 0x0342,
                                0x0344, 0x0221, 0x02D4, 0x0283, 0x0336, 0x4E3D, 0x4E3F, 0x4E41, 0x4E43};
  for(unsigned i = 0; i < sizeof z2 / sizeof z2[0]; i++) wr16(z2[i], 0);
  static const uint16_t slots[4] = {0x0233, 0x02E6, 0x0295, 0x0348};
  for(int i = 0; i < 4; i++) wr16(slots[i], 1);
  wr16(0x1ED0, 0);
  static const uint16_t flags[4] = {0x0244, 0x02F7, 0x02A6, 0x0359};
  for(int i = 0; i < 4; i++) wr16(flags[i], 1);
  for(int i = 0; i < 4; i++) wr16((uint16_t)(0x1ED2 + 2 * i), (uint16_t)i);
  static const uint16_t counters[4] = {0x0235, 0x02E8, 0x0297, 0x034A};
  for(int i = 0; i < 4; i++) wr16(counters[i], 0xFFFF);
  static const uint16_t z3[] = {0x021F, 0x02D2, 0x0281, 0x0334, 0x021A, 0x02CD, 0x027C, 0x032F, 0x00BE, 0x01A0, 0x01A4,
                                0x01A8, 0x01AC, 0x00BE, 0x00C0, 0x00BC, 0x0486, 0x00C4, 0x00C6, 0x00C2, 0x050E, 0x0176,
                                0x017A, 0x0208, 0x02BB, 0x026A, 0x031D, 0x0200, 0x0202, 0x02B3, 0x02B5, 0x0262, 0x0264,
                                0x0315, 0x0317, 0x0225, 0x0227, 0x02D8, 0x02DA, 0x0287, 0x0289, 0x033A, 0x033C};
  for(unsigned i = 0; i < sizeof z3 / sizeof z3[0]; i++) wr16(z3[i], 0);
  wr16(0x1F28, bus_rd16(0x85A33A + 8));
  wr16(0x1F2A, bus_rd16(0x85A33C + 8));
  wr16(0x0226, 0x0020);
  static const uint16_t z4[] = {0x023E, 0x02F1, 0x0210, 0x02C3, 0x0272, 0x0325, 0x0253, 0x0306,
                                0x0223, 0x02D6, 0x0285, 0x0338, 0x0240, 0x02F3, 0x02A2, 0x0355};
  for(unsigned i = 0; i < sizeof z4 / sizeof z4[0]; i++) wr16(z4[i], 0);
  wr16(0x00A4, 0x57F6);
  wr16(0x00AC, 0x57F6);
  wr16(0x00A6, 0x584E);
  wr16(0x00AE, 0x584E);
  // the lap time: $B0 << 16 over the product's low word (16 restoring steps, as the original)
  const uint16_t d = (uint16_t)dsp4_multiply((int16_t)rd16(0x019A), (int16_t)rd16(0x1F06));
  wr16(DP + 0x14, d);
  uint16_t a = 0x00B0, q = rd16(DP + 0x16);
  for(int i = 0; i < 16; i++) {
    uint16_t bit = 0;
    if(a >= d) {
      a = (uint16_t)(a - d);
      bit = 1;
    }
    q = (uint16_t)((q << 1) | bit);
    a = (uint16_t)(a << 1);
  }
  wr16(DP + 0x16, q);
  wr16(0x019E, q);
  static const uint16_t z5[] = {0x003E, 0x0040, 0x0064, 0x0068, 0x00FE, 0x0100, 0x1E02, 0x1E0A, 0x1E12, 0x1E1A};
  for(unsigned i = 0; i < sizeof z5 / sizeof z5[0]; i++) wr16(z5[i], 0);
  track_palettes_load(DP);
  vs_grid_setup(DP);
  static const struct { uint16_t car, place, side0, side1, v0, v1; } cars[4] = {
    {0x0000, 0x0226, 0x0204, 0x0206, 0x0000, 0xFF40}, {0x0035, 0x02D9, 0x02B7, 0x02B9, 0x0000, 0xFFC0},
    {0x006A, 0x0288, 0x0266, 0x0268, 0x0000, 0x0040}, {0x009F, 0x033B, 0x0319, 0x031B, 0x0000, 0x00C0},
  };
  for(int i = 0; i < 4; i++) {
    wr16(cars[i].side0, cars[i].v0);
    wr16(cars[i].side1, cars[i].v1);
  }
  for(int i = 0; i < 4; i++) {
    wr16(cars[i].place, (uint16_t)(rd16(0xFAB6 + (uint32_t)cars[i].car) - 0x0030));
    wr16(0xFAB9 + (uint32_t)cars[i].car, rd16(cars[i].side0));
    wr16(0xFABB + (uint32_t)cars[i].car, rd16(cars[i].side1));
  }
  // one SEC, then four SBC #$20 (the borrow runs on from one to the next)
  static const uint16_t from[4] = {0x0226, 0x02D9, 0x0288, 0x033B}, to[4] = {0x023E, 0x02F1, 0x02A0, 0x0353};
  uint32_t borrow = 0;
  for(int i = 0; i < 4; i++) {
    const uint32_t r = (uint32_t)rd16(from[i]) - 0x20u - borrow;
    wr16(to[i], (uint16_t)r);
    borrow = (r >> 31) & 1u;
  }
  uint16_t x = 0;
  uint8_t colour = 0xC1;
  for(int row = 0; row < 5; row++) {
    uint8_t b[32];
    for(int i = 0; i < 30; i++) b[i] = bus_rd8(0x85A118 + x++);
    b[30] = b[31] = b[29];
    for(int k = 0; k < 16; k++, colour++) video_set_color(colour, (uint16_t)(b[2 * k] | b[2 * k + 1] << 8));
  }
  wr8(DP + 0x14, 0);
  wr8(DP + 0x16, (uint8_t)(rd8(DP + 0x16) + 10));
  static const uint16_t z6[4] = {0x023A, 0x02ED, 0x029C, 0x034F};
  for(int i = 0; i < 4; i++) wr16(z6[i], 0x0300);
  wr16(0x1F08, 0);
}

// TG3000_ROUTINE($82:92BA) vs_race_load
// The VS race setup's first piece, entered from the race wrapper's mode table ($82:8418), up to
// the test of $047C at $82:932D: the screen image; the track number moved on by $2F (to the VS
// tracks: a 16-bit add, its carry reaches $047C); the track's settings; vs_race_reset; then the
// graphics unpacked to VRAM and to $7F:0000, the backdrop, 32 colours from $8E:8000 and the
// track's tiles. The rest of the setup waits on routines not ported yet.
// TG3000_ROUTINE($82:932D) vs_race_sound
void vs_race_sound(void) {
  if(rd16(0x047C) & 0x8000) return;
  wait_03e0();
  sound_load_vs(DP);
  wr8(SOUND_VOLUME, 0x7F);
  wr8(0x045F, 0);
}

void vs_race_load(void) {
  video_load_screen_long(0x839EEA);
  wr16(0x047B, (uint16_t)(rd16(0x047B) + 0x002F));
  track_settings_load();
  vs_race_reset();
  decompress_to_vram(DP, 0x9B, 0x8125, 0x0000);
  uint16_t end;
  decompress(DP, 0x9B, 0x8080, 0x7F, 0x0000, &end);
  track_backdrop_load(DP);
  upload_cgram_now(0x00, 0x8E, 0x8000, 0x0040);
  decompress_to_vram(DP, 0x87, 0x8E04, 0x0E40);
  track_tiles_load(rd16(0x047B));
  decompress_to_vram(DP, 0x9B, 0x8080, 0x7C00);
  decompress_to_vram(DP, 0x87, 0x8C10, 0x57A0);
}

// TG3000_ROUTINE($82:9348) vs_race_track
// The VS race setup's track piece, from $82:9348 (where the $047C test lands) up to the VS road
// rows at $82:941F: the clocks and counters cleared (D = $0100: $3E and $013E are both
// cleared; the overlapping words at $005A-$0060 in the original's order); the track
// (track_setup_load, track_rows_build) and its objects in the four views; four announcements
// (func_82AD12), each followed by a frame's wait; 79 colours from $7E:6A35 to CGRAM $71 and
// colour 0 from $7E:6A33; the sizes of DMA slots 1-3 cleared. Its rows are cut at the call to
// track_setup_load (port-trap-1): the head, that call's own row, the tail.
void vs_race_track(void) {
  vs_race_track_head();
  track_setup_load();
  vs_race_track_tail();
}

// (vs_race_track's row runs this head alone)
// The clocks and counters cleared, up to the JSR to track_setup_load ($82:9391).
void vs_race_track_head(void) {
  wr16(0x002E, 0x0010);
  wr16(0x0030, 0x0000);
  wr16(0x003E, 0);
  wr16(0x0040, 0);
  wr16(DP + 0x3E, 0);
  wr16(DP + 0x40, 0);
  wr16(0x005A, 0);
  wr16(0x005C, 0);
  wr16(0x005D, 0);
  wr16(0x005F, 0);
  wr16(DP + 0x35, 0);
  wr16(DP + 0xE8, 0);
  wr16(0x0060, 3);
  wr16(0x0062, 3);
  static const uint8_t dp_words[] = {0x56, 0x58, 0x09, 0x0B, 0x5A, 0x0D, 0x4C, 0x4E, 0x50, 0x52, 0x56, 0x58, 0x54};
  for(unsigned i = 0; i < sizeof dp_words; i++) wr16((uint16_t)(DP + dp_words[i]), 0);
}

// TG3000_ROUTINE($82:9394) vs_race_track_tail
// After track_setup_load: the track's rows, its objects in the four views, the announcements
// with their waits, the palette, the DMA slots' sizes.
void vs_race_track_tail(void) {
  track_rows_build();
  vs_track_objects_view0();
  vs_track_objects_view1();
  vs_track_objects_view2();
  vs_track_objects_view3();
  static const uint16_t announce[4][2] = {{0x0010, 0x4000}, {0x0011, 0x1000}, {0x0012, 0x0400}, {0x0013, 0x0100}};
  for(int i = 0; i < 4; i++) {
    func_82AD12(announce[i][0], announce[i][1]);
    wait_frames(1);
  }
  video_upload_cgram(0x71, 0x7E6A35, 0x009E);
  video_set_color(0x00, (uint16_t)(bus_rd8(0x7E6A33) | bus_rd8(0x7E6A34) << 8));
  dma_stream_size(1, 0);
  dma_stream_size(2, 0);
  dma_stream_size(3, 0);
}

// The VS race's road rows, one copy per player ($85:BF78, $85:C316, $85:C6C2, $85:CA18):
// road_rows_build's counterpart without the segments, curves, edges and track events. Each
// copy has its player's record and tables (below). The paths a copy never runs are traps,
// by the flags in `holes` (the others' code is not taken for them unchecked).
enum {
  VS_HOLE_FLAG_CLEAR = 1, VS_HOLE_LOW_KIND = 2, VS_HOLE_LIST2 = 4, VS_HOLE_NEXT_OBJ = 8,
  VS_HOLE_LAP_WRAP = 16, VS_HOLE_KIND_SCAN = 32, VS_HOLE_TILT = 64,
};
typedef struct {
  uint16_t rec, tab, sum, height;    // the player's record, its row tables, sideways sum, height
  uint8_t objCur, segCur;            // the D+ cursors of its object stream and segments (0: none)
  uint16_t spawnFlag, gfx, baseLow;  // a spawned object's $F733, $F633 add, $F833 base (high kind: +$0370)
  uint8_t f933Low, f933High;         // $F933: 0 lane << 1, 1 ((lane + $40) & $FF) << 1, 2 (lane + $40) << 1
  uint16_t side0, side1;             // the $1EF8-style words (0: none)
  uint16_t kindOut;                  // the road kind out ($0142/$0144; 0: no road-kind block)
  unsigned holes;                    // the copy's never-run paths
} VsRoadView;

static const VsRoadView vs_road_views[4] = {
  {0x0200, 0x3AEC, 0x01CE, 0x1E1E, 0xA0, 0xA2, 0x007F, 0x0000, 0x45EC, 0, 0, 0x1EF8, 0x1EFA, 0x0142,
   VS_HOLE_LIST2 | VS_HOLE_KIND_SCAN},
  {0x02B3, 0x406C, 0x01D2, 0x1E20, 0xA4, 0xA6, 0x017F, 0x0800, 0x46C8, 1, 2, 0x1F02, 0x1F04, 0x0144,
   VS_HOLE_LIST2 | VS_HOLE_NEXT_OBJ | VS_HOLE_KIND_SCAN},
  {0x0262, 0x3DEC, 0x01D6, 0x1E22, 0xA8, 0x00, 0x027F, 0x1000, 0x47A4, 0, 0, 0, 0, 0, VS_HOLE_LIST2},
  {0x0315, 0x436C, 0x01DA, 0x1E24, 0xAC, 0xAE, 0x037F, 0x1800, 0x4880, 1, 1, 0, 0, 0, VS_HOLE_LIST2},
};

static uint16_t vs_object_spawn(const VsRoadView* v, uint16_t ox, uint16_t y, uint16_t kind2, bool high) {
  const uint32_t t = bus_rd16((high ? 0x86AC36 : 0x86AE36) + kind2);
  const uint32_t field = high ? 0x22 : 0x1E;
  wr16(0xF533 + (uint32_t)y, bus_rd16(0x860000 + field + t));
  wr16(0xF433 + (uint32_t)y, bus_rd16(0x860002 + field + t));
  wr16(0xF733 + (uint32_t)y, v->spawnFlag);
  wr16(0xF533 + (uint32_t)y, (uint16_t)(rd16(0x6591 + (uint32_t)bus_rd16(0x860000 + t)) + rd16(0xF533 + (uint32_t)y)));
  const uint16_t lane = rd8(0x5B93 + (uint32_t)ox);
  wr16(0xF633 + (uint32_t)y, (uint16_t)((lane << 5) + v->gfx));
  const uint8_t m = high ? v->f933High : v->f933Low;
  wr16(0xF933 + (uint32_t)y, m == 0 ? (uint16_t)(lane << 1)
                             : m == 1 ? (uint16_t)(((lane + 0x40) & 0xFF) << 1) : (uint16_t)((lane + 0x40) << 1));
  wr16(0xF833 + (uint32_t)y, (uint16_t)((rd8(0x5B94 + (uint32_t)ox) << 1) + v->baseLow + (high ? 0x0370 : 0)));
  func_85CDAF(y);
  return (uint16_t)((y + 2) & 0x00FF);
}

static void vs_road_rows_one(const VsRoadView* v) {
  const uint32_t rec = v->rec, pos = rec + 0x24, row = rec + 0x26, frac = rec + 0x3C, prev = rec + 0x3E;
  const uint32_t acc = rec + 0x40, flag = rec + 0x44, xtab = (uint32_t)v->tab + 2;
  // the sideways place over the rows crossed, by DSP-4 products (as road_rows_build)
  uint32_t x = (uint16_t)(rd16(prev) << 1) & 0x003E;
  uint16_t crossed = (uint16_t)(rd16(row) - rd16(prev));
  if(crossed != 0) {
    const uint16_t back = (uint16_t)(0 - rd16(frac));
    uint32_t p = (uint32_t)dsp4_multiply((int16_t)((back >> 8) & 0x00FF), (int16_t)rd16(xtab + x));
    wr16(DP + 0x14, (uint16_t)((p << 8) & 0xFF00));
    wr16(DP + 0x16, (uint16_t)((p >> 8) & 0x00FF));
    uint16_t a = (uint16_t)(p >> 8);
    wr16(v->sum, a);
    x += 2;
    if(--crossed != 0) {
      uint32_t c = rd16(frac) == 0;  // the carry of the SBC above
      do {
        const uint32_t sum = (uint32_t)a + rd16(xtab + x) + c;
        a = (uint16_t)sum;
        c = sum >> 16;
        x += 2;
      } while(--crossed != 0);
      wr16(v->sum, a);
    }
    p = (uint32_t)dsp4_multiply((int16_t)(uint16_t)((rd16(pos) & 0xFF00) >> 1), (int16_t)(uint16_t)(rd16(xtab + x) << 1));
    wr16(v->sum, (uint16_t)((uint16_t)(p >> 16) + rd16(v->sum)));
  } else {
    const uint32_t p = (uint32_t)dsp4_multiply((int16_t)(uint16_t)((uint16_t)((rd16(pos) & 0xFF00) - rd16(frac)) >> 1),
                                               (int16_t)(uint16_t)(rd16(xtab + x) << 1));
    wr16(v->sum, (uint16_t)(p >> 16));
  }
  uint16_t r16 = (uint16_t)(rd16(prev) + 0x0020);
  if(r16 >= rd16(0x019A)) r16 = (uint16_t)(r16 - rd16(0x019A));
  wr16(DP + 0x16, r16);
  wr16(frac, (uint16_t)(rd16(pos) & 0xFF00));
  const uint16_t rows = (uint16_t)(rd16(row) - rd16(prev));
  if(rows != 0) {
    wr16(DP + 0x14, rows);
    if(!(rd16(flag) & 1)) {
      if(v->holes & VS_HOLE_FLAG_CLEAR) tg3000_unported("$85:C05E: the rows accumulator, flag bit 0 clear");
      wr16(acc, (uint16_t)(rows + rd16(acc)));
    }
    if(v->side0) {
      x = (uint16_t)(rd16(prev) << 1) & 0x003E;
      wr16(v->side0, (uint16_t)(rd16(0x3E6E + x) - rd16(0x3E6C + x)));
      wr16(v->side1, (uint16_t)(rd16(0x3DEE + x) - rd16(0x3DEC + x)));
    }
    do {  // each new row, at D+$16
      const uint16_t d16 = rd16(DP + 0x16);
      wr16(DP + 0x18, (uint16_t)((uint16_t)dsp4_multiply((int16_t)((uint16_t)(d16 << 2) >> 8), 0x000A) + rd16(DP + 0x98)));
      const uint32_t entry = rd16(DP + 0x18);
      const uint32_t y = (uint16_t)(d16 << 1) & 0x003E;
      wr16(DP + 0x1A, (uint16_t)y);
      wr16(DP + 0x1C, (uint16_t)(d16 & 0x003E));
      const uint32_t T = v->tab;
      uint32_t at = (uint16_t)(rd16(DP + 0x1C) + bus_rd16(0x840000 + entry) + (d16 >> 15));
      wr16(T + y, bus_rd16(0x850000 + at));
      wr16(T + 0x40 + y, bus_rd16(0x850000 + at));
      at = (uint16_t)(rd16(DP + 0x1C) + bus_rd16(0x840002 + entry));
      const uint16_t up = (uint16_t)(bus_rd16(0x850000 + at) - rd16(v->height));
      wr16(T + 0x80 + y, up);
      wr16(T + 0xC0 + y, up);
      if((y & 0x0006) == 0x0006) wr16(v->height, bus_rd16(0x850000 + at));
      uint16_t a = (y & 0x0002) ? (uint16_t)(rd16(DP + 0x1C) | 0x0001) : rd16(DP + 0x1C);
      a = (uint16_t)(a << 1);
      if(!(a & 0x0040)) at = (uint16_t)(a + bus_rd16(0x840006 + entry));
      else at = (uint16_t)((a & 0x003F) + bus_rd16(0x840008 + entry));
      wr16(T + 0x200 + y, rd16(0x5391 + at));
      wr16(T + 0x240 + y, rd16(0x5391 + at));
      const uint32_t list = bus_rd16(0x840004 + entry);
      if(bus_rd16(0x850002 + list) < rd16(DP + 0x1A) && (v->holes & VS_HOLE_LIST2))
        tg3000_unported("$85:C13A and its copies: a row list's second pair");
      wr16(T + 0x100 + y, bus_rd16(0x850000 + list));
      wr16(T + 0x140 + y, bus_rd16(0x850000 + list));
      wr16(DP + 0x16, (uint16_t)(rd16(DP + 0x16) + 1));
      if(rd16(DP + 0x16) == rd16(0x019A)) {
        wr16(DP + 0x16, 0);
        if(v->segCur) wr16(DP + v->segCur, 0);
      }
      wr16(DP + 0x14, (uint16_t)(rd16(DP + 0x14) - 1));
    } while(rd16(DP + 0x14) != 0);
    // the road objects whose place came up
    uint16_t ox = rd16(DP + v->objCur);
    if(rd16(row) >= rd16(0x5B91 + (uint32_t)ox)) {
      uint16_t slot = rd16(0x0100);
      for(;;) {
        const uint16_t k = rd16(0x5B94 + (uint32_t)ox);
        const bool high = (k & 0xFF00) != 0;
        if(!high && (v->holes & VS_HOLE_LOW_KIND)) tg3000_unported("$85:C18A: a low-kind object");
        slot = vs_object_spawn(v, ox, slot, (uint16_t)(k << 1), high);
        ox = (uint16_t)(ox + 5);
        wr16(DP + v->objCur, ox);
        if(rd16(row) < rd16(0x5B91 + (uint32_t)ox)) break;
        if(v->holes & VS_HOLE_NEXT_OBJ) tg3000_unported("$85:C25E and its copy: a second object in one frame");
      }
      wr16(0x0100, slot);
    }
    uint16_t cur = rd16(row);
    if(cur >= rd16(0x019A)) {  // the lap's wrap
      if(v->holes & VS_HOLE_LAP_WRAP) tg3000_unported("$85:C26D: the lap's wrap");
      cur = (uint16_t)(cur - rd16(0x019A));
      wr16(DP + v->objCur, 0);
    }
    wr16(row, cur);
    wr16(prev, cur);
    if(v->kindOut) {  // the road kind at the player: only before the first segment
      wr16(rec + 0x60, 0);
      if(rd16(row) >= bus_rd16(0x840000 + rd16(DP + 0x9C)) && (v->holes & VS_HOLE_KIND_SCAN))
        tg3000_unported("$85:C28D and its copy: the road-kind scan");
      wr16(v->kindOut, (uint16_t)(rd16(rec + 0x60) & 0x00FF));
    }
  }
  // the tilt from the heights a row ahead and three behind (func_82EA49 = $85:CD78)
  uint16_t a = (uint16_t)(rd16(row) + 1);
  if(a >= rd16(0x019A)) {
    if(v->holes & VS_HOLE_TILT) tg3000_unported("$85:C2B6: the tilt's row ahead wraps");
    a = (uint16_t)(a - rd16(0x019A));
  }
  wr16(DP + 0x14, func_82EA49(a));
  a = rd16(row);
  if(a < 3) {
    if(v->holes & VS_HOLE_TILT) tg3000_unported("$85:C2C7: the tilt's row behind wraps");
    a = (uint16_t)(a - 3 + rd16(0x019A));
  } else {
    a = (uint16_t)(a - 3);
  }
  wr16(DP + 0x16, func_82EA49(a));
  const uint16_t d16 = rd16(DP + 0x16);
  const uint32_t p = (uint32_t)dsp4_multiply((int16_t)(uint16_t)(rd16(DP + 0x14) - d16),
                                             (int16_t)(uint16_t)((rd16(rec + 0x25) + 0x0100) & 0x03FF));
  const uint16_t r = (uint16_t)((uint16_t)(d16 << 2) | ((d16 & 0x8000) ? 3 : 0));
  uint16_t s = (uint16_t)(((r >> 8) | (r << 8)) & 0x03FF);
  if(s & 0x0080) {
    if(v->holes & VS_HOLE_TILT) tg3000_unported("$85:C2FD: the tilt's sign extension");
    s |= 0xFF00;
  }
  wr16(rec + 0x37, (uint16_t)-(uint16_t)((uint16_t)(s + (uint16_t)(p >> 16)) + 9));
}

// TG3000_ROUTINE($85:BF78) vs_road_rows_view0
void vs_road_rows_view0(void) { vs_road_rows_one(&vs_road_views[0]); }
// TG3000_ROUTINE($85:C316) vs_road_rows_view1
void vs_road_rows_view1(void) { vs_road_rows_one(&vs_road_views[1]); }
// TG3000_ROUTINE($85:C6C2) vs_road_rows_view2
void vs_road_rows_view2(void) { vs_road_rows_one(&vs_road_views[2]); }
// TG3000_ROUTINE($85:CA18) vs_road_rows_view3
void vs_road_rows_view3(void) { vs_road_rows_one(&vs_road_views[3]); }

// TG3000_ROUTINE($82:DF91) vs_road_rows
// The four players' road rows in turn (the VS race's road_rows_build_call).
void vs_road_rows(void) {
  vs_road_rows_view0();
  vs_road_rows_view1();
  vs_road_rows_view2();
  vs_road_rows_view3();
}

// TG3000_ROUTINE($82:9422) vs_race_sums_clear
// The VS race setup's piece between the road rows and the VS road driver ($85:A471, not ported):
// the words at $01CC-$01DA (the four players' sideways sums and the words below them) and the
// bytes $00D4-$00DF cleared, in the original's order.
void vs_race_sums_clear(void) {
  static const uint16_t words[] = {0x01CE, 0x01CC, 0x01D2, 0x01D0, 0x01D6, 0x01D4, 0x01DA, 0x01D8,
                                   0x00D4, 0x00D6, 0x00D7, 0x00D9, 0x00DA, 0x00DC, 0x00DD, 0x00DF};
  for(unsigned i = 0; i < sizeof words / sizeof words[0]; i++) wr16(words[i], 0);
}

// TG3000_ROUTINE($82:9456) vs_race_start
// The VS race setup's last piece, after the VS road driver, up to the race loop ($82:94B2): the
// cars' bands; the NMI ($83:9777), IRQ ($83:9721) and IRQ trampoline ($83:8B8E) vectors; then
// the interrupts on (CLI); the race's counters; a frame's wait; $1ECE = $8004. With bit 15 of
// $047C set (never run; decoded from the ROM), the four players' $0233-style words = $43.
void vs_race_start(void) {
  vs_car_bands();
  wr16(0x0008, 0x9777);
  wr16(0x0001, 0x9721);
  wr16(0x0005, 0x8B8E);
  wr8(0x0003, 0x83);
  wr8(0x0007, 0x83);  // then CLI
  wr16(0x00BA, 0);
  wr16(0x1E9E, 1);
  wr16(0x014A, 0);
  wr16(0x0148, 0x0F00);
  wr16(0x0146, 0x0080);
  wait_frames(1);
  wr16(0x1ECE, 0x8004);
  if(rd16(0x047C) & 0x8000) {
    static const uint16_t words[4] = {0x0233, 0x02E6, 0x0295, 0x0348};
    for(int i = 0; i < 4; i++) wr16(words[i], 0x0043);
  }
}

// The countdown's message for the four players: A from the time left before (x) and now (y).
static uint16_t vs_countdown_message(uint16_t x, uint16_t y) {
  uint16_t a = 0;
  if(!((uint16_t)(x - 0x0168) & 0x8000) && ((uint16_t)(y - 0x0168) & 0x8000)) a = 0x0002;  // (never run)
  if(!((uint16_t)(x - 0x00F0) & 0x8000) && ((uint16_t)(y - 0x00F0) & 0x8000)) a = 0x0003;
  if(!((uint16_t)(x - 0x0078) & 0x8000) && ((uint16_t)(y - 0x0078) & 0x8000)) a = 0x0004;
  if(!((uint16_t)(x - 0x0002) & 0x8000) && ((uint16_t)(y - 0x0002) & 0x8000)) {
    wr16(0x1ECE, 0x0001);
    a = 0x0005;
  }
  return a;
}

// TG3000_ROUTINE($82:94B2) vs_race_frame_body
// One frame of the VS race loop, from its head up to its wait ($82:95E8): the HUD's digits; the
// start's countdown (its messages to the four players: 3, 2, 1, go; or the demo's $3F-$42); the
// cars linked, moved, ordered and banded; the records' sides from the cars; the road rows, the
// steps' shares, the places; the road projected and the four views' sprites; then the frame
// flagged for the NMI. The race's end (the fade-out after $1F08 = 4) is ported from the ROM's
// bytes (never run: the movie races on); the way out of the loop ($82:9642) is a trap.
void vs_race_frame_body(void) {
  vs_hud_digits();
  const uint16_t count = rd16(0x1ECE);
  if(count & 0x8000) {  // the demo's messages
    if(!(rd16(0x047C) & 0x8000)) {
      wr16(0x0233, 0x003F);
      wr16(0x02E6, 0x0040);
      wr16(0x0295, 0x0041);
      wr16(0x0348, 0x0042);
    }
  } else if(count != 0) {
    uint16_t left = (uint16_t)(count - rd16(0x1E9E));
    if(left & 0x8000) left = 0;
    wr16(0x1ECE, left);
    const uint16_t m = vs_countdown_message(count, left);
    wr16(0x0233, m);
    wr16(0x02E6, m);
    wr16(0x0295, m);
    wr16(0x0348, m);
  }
  vs_cars_link();
  vs_cars_frame();
  vs_cars_order();
  vs_car_bands();
  static const uint16_t recs[4] = {0x0200, 0x02B3, 0x0262, 0x0315};
  for(int i = 0; i < 4; i++) {
    wr16(recs[i], rd16(recs[i] + 4));
    wr16(recs[i] + 2, rd16(recs[i] + 6));
  }
  vs_road_rows();
  vs_steps_share();
  vs_places_update();
  const uint16_t base = rd16(0x00B4) ? 0x0220 : 0x0000;
  wr16(DP + 0x86, base);
  wr16(DP + 0x88, base);
  wr16(DP + 0x8E, base);
  vs_road_project();
  vs_sprites_view0();
  vs_sprites_view1();
  oam_high_load_vs();
  vs_sprites_view2();
  vs_sprites_view3();
  oam_high_load2();
  wr16(0x00B8, (uint16_t)(rd16(0x00B8) | 0x0001));
  if(rd16(0x1F08) == 0x0004) {  // the race over: a pause, then the fade-out (never run)
    uint16_t t = rd16(0x1ED0);
    if(t == 0) {
      t = 0x7F10;
      wr16(0x1ED0, t);
    }
    if(!(t & 0x8000)) {
      t = (uint16_t)(rd16(0x1ED0) + rd16(0x1E9E));
      wr16(0x1ED0, t);
      if(t & 0x8000) {
        wr16(0x0148, 0);
        wr16(0x0146, 0xFF80);
        if(!(rd16(0x047C) & 0x8000)) wr16(0x03E0, 0xFFFF);
      }
    }
  }
  if(rd16(0x0146) == 0 && !(rd16(0x014A) & 0x0F00)) tg3000_unported("$82:9642: the VS race loop's way out");
}

// TG3000_ROUTINE($82:95E8) vs_race_frame_tail
// The VS race frame's rest: the wait for the NMI to take the frame; the frames it took ($00BA)
// kept for the next one ($1E9E); the demo's end on any button and the Start test (both ported
// from the ROM's bytes, never run; the VS pause, $82:9657, is a trap).
void vs_race_frame_tail(void) {
  while(rd16(0x00B8) != 0) platform_wait_vblank();
  const uint16_t frames = rd16(0x00BA);
  if(rd16(0x047C) & 0x8000) {  // the demo: any button ends it (never run)
    if((rd16(0x000C) | rd16(0x0010) | rd16(0x0014) | rd16(0x0018)) & 0xFFF0) {
      wr16(0x047D, (uint16_t)(rd16(0x047D) & ~0x007F));
      wr16(0x0148, 0);
      wr16(0x0146, 0xFF80);
    }
  } else {
    static const uint16_t pads[4] = {0x000E, 0x0012, 0x0016, 0x001A};
    bool start = false;
    for(int i = 0; i < 4 && !start; i++) {
      start = (rd16(pads[i]) & 0x1000) != 0;
      wr16(pads[i], (uint16_t)(rd16(pads[i]) & ~0x1000));
    }
    if(start && rd16(0x014A) == 0x0F00) tg3000_unported("$82:9657: the VS race paused");
  }
  wr16(0x00BA, 0);
  wr16(0x1E9E, frames);
}

// One frame of the VS race loop ($82:94B2 to its jump back).
void vs_race_frame(void) {
  vs_race_frame_body();
  vs_race_frame_tail();
}

void race_mode_vs(void) {
  vs_race_load();
  vs_race_sound();
  vs_race_track();
  vs_road_rows();
  vs_race_sums_clear();
  vs_road_project();
  vs_race_start();
  for(;;) vs_race_frame();
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

// The VS race's player_frame_setup, one routine a player (vs-race-7). Each sets the frame's car:
// - $1EBE its view's bit, D+$14 twice its choice byte, $1EB2 the choice's entry at $86:DD75;
// - the record's +$2B (func_83EA9B of twice +$1F plus the word at $86:DAD9 + twice the entry's
//   low 7 bits), the engine and tyre words (vs_car_parts), $1EB8 the word at $86:DD45;
// - $1ECA the player's number - 1, $1EBA its record's offset, $1EBC twice its +$26 (low 5
//   bits) plus its view's base, $1E9C cleared.
// Then, when the record's +$44 bit 2 is clear and the player's bit of $03DC is set, the
// controls into $1E9C from its pad (held, then pressed, with the controls' masks at $039D or
// $03CC); otherwise player_road_limits for the record.
typedef struct {
  uint16_t view, choice, rec, number, base, mask, pad, keys;
} VsPlayer;
static const VsPlayer vsPlayers[4] = {
  {0x8000, 0x0418, 0x0200, 0x0000, 0x0000, 0x0010, 0x000C, 0x039D},
  {0x2000, 0x0419, 0x02B3, 0x0035, 0x0580, 0x0020, 0x0010, 0x03CC},
  {0x0800, 0x041A, 0x0262, 0x006A, 0x0300, 0x0040, 0x0014, 0x039D},
  {0x0200, 0x041B, 0x0315, 0x009F, 0x0880, 0x0080, 0x0018, 0x03CC},
};
static void vs_player_frame_setup(int n) {
  const VsPlayer* p = &vsPlayers[n];
  wr16(0x1EBE, p->view);
  const uint16_t i = (uint16_t)((rd16(p->choice) & 0x00FF) << 1);
  wr16(DP + 0x14, i);
  const uint16_t t = bus_rd8(0x86DD75 + (uint32_t)i);
  wr16(0x1EB2, t);
  const uint32_t x = (uint32_t)((t & 0x7F) << 1);
  wr16((uint16_t)(p->rec + 0x2B), func_83EA9B((uint16_t)((rd16((uint16_t)(p->rec + 0x1F)) << 1) + bus_rd16(0x86DAD9 + x))));
  vs_car_parts(DP);
  wr16(0x1EB8, bus_rd16(0x86DD45));
  wr16(0x1ECA, (uint16_t)n);
  wr16(0x1EBA, p->number);
  wr16(0x1EBC, (uint16_t)(((rd16((uint16_t)(p->rec + 0x26)) << 1) & 0x003E) + p->base));
  wr16(0x1E9C, 0);
  if(!(rd16((uint16_t)(p->rec + 0x44)) & 0x0004) && (rd16(0x03DC) & p->mask)) {  // never in the movies
    const uint16_t held = rd16(p->pad);
    if(held & rd16(p->keys)) wr16(0x1E9C, (uint16_t)(rd16(0x1E9C) | 0x0001));
    if(held & rd16((uint16_t)(p->keys + 2))) wr16(0x1E9C, (uint16_t)(rd16(0x1E9C) | 0x0002));
    if(held & 0x0200) wr16(0x1E9C, (uint16_t)(rd16(0x1E9C) | 0x0004));
    if(held & 0x0100) wr16(0x1E9C, (uint16_t)(rd16(0x1E9C) | 0x0008));
    const uint16_t key = rd16((uint16_t)(p->keys + 4)), pressed = rd16((uint16_t)(p->pad + 2));
    wr16((uint16_t)(p->pad + 2), (uint16_t)(pressed & ~key));
    if(pressed & key) wr16(0x1E9C, (uint16_t)(rd16(0x1E9C) | 0x0010));
    return;
  }
  player_road_limits(p->rec);
}

// TG3000_ROUTINE($83:E597) vs_player1_frame_setup
void vs_player1_frame_setup(void) { vs_player_frame_setup(0); }
// TG3000_ROUTINE($83:E64D) vs_player2_frame_setup
void vs_player2_frame_setup(void) { vs_player_frame_setup(1); }
// TG3000_ROUTINE($83:E706) vs_player3_frame_setup
void vs_player3_frame_setup(void) { vs_player_frame_setup(2); }
// TG3000_ROUTINE($83:E7BF) vs_player4_frame_setup
void vs_player4_frame_setup(void) { vs_player_frame_setup(3); }

// TG3000_ROUTINE($83:E28C) vs_cars_frame
void vs_cars_frame(void) {
  const uint32_t at = 0x4CDD + (uint32_t)(uint16_t)(rd16(0x1E9E) << 1);  // bank $7E
  wr16(at, (uint16_t)(rd16(at) + 1));
  for(int n = 0; n < 4; n++) {
    vs_player_frame_setup(n);
    player_car_frame(vsPlayers[n].rec);
    car_track_step(vsPlayers[n].rec);
  }
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
