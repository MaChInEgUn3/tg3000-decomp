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
