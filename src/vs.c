// SPDX-License-Identifier: GPL-3.0-or-later
#include "vs.h"

#include <stdbool.h>

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
