// SPDX-License-Identifier: GPL-3.0-or-later
#include "hud.h"

#include "misc.h"
#include "platform.h"
#include "text.h"
#include "tg3000.h"

// TG3000_ROUTINE($80:DA5F) draw_meter
//
// The original multiplies on the PPU: matrix A = the value, then one byte $0A to matrix B
// (whose low byte is then the latch, the value's high byte), and reads bits 8-23 of the
// product. The two registers keep those values, so the C sets them the same way.
uint16_t draw_meter(uint16_t dp) {
  int16_t value = (int16_t)rd16(dp + METER_VALUE);
  video_set_m7a(value);
  video_set_m7b((int16_t)(0x0A00 | (uint8_t)((uint16_t)value >> 8)));
  uint16_t length = (uint16_t)(((int32_t)value * 10) >> 8) + 2;
  wr16(dp + METER_SCRATCH, length);
  uint16_t left = METER_TILES, vram = METER_VRAM;
  for(uint16_t full = length >> 3; full != 0; full--, left--) video_write_vram_word(vram++, METER_FULL);
  if(length & 7) {
    video_write_vram_word(vram++, (uint16_t)(METER_PART + (length & 7)));
    left--;
  }
  for(; left != 0; left--) video_write_vram_word(vram++, METER_EMPTY);
  return length;
}

// a light's sprite at slot y: lit (the light's own sprite) or a plain tile at its place
static void race_light_lit(uint16_t y, uint16_t n) {
  const uint32_t s = y, x = rom16(0x81CB2C + (uint32_t)(uint16_t)(n << 1));
  wr16(0x0900 + s, rom16(0x81CB44 + x));
  wr16(0x0902 + s, rom16(0x81CB46 + x));
  wr16(0x0B01 + s, n ? 0x0100 : 0x0000);
}

static void race_light_tile(uint16_t y, uint16_t n, uint16_t tile) {
  const uint32_t s = y;
  wr16(0x0900 + s, (uint16_t)(rom16(0x81CB14 + (uint32_t)(uint16_t)(n << 1)) + 0x3068));
  wr16(0x0902 + s, tile);
  wr16(0x0B01 + s, 0);
}

// TG3000_ROUTINE($80:DAED) race_lights_draw
void race_lights_draw(void) {
  wr16(DP + 0x14, 0);
  uint16_t y = rd16(DP + 0x74);
  do {
    uint16_t n = rd16(DP + 0x14), lit = rd16(0x0368);
    if(n < lit) {
      race_light_lit(y, n);
    } else if(n != lit) {
      race_light_tile(y, n, 0x25C2);
    } else if(rd16(DP + 0x3C) == 0) {
      race_light_lit(y, n);
    } else if(rd16(DP + 0x26) != 0) {
      race_light_tile(y, n, 0x25C3);
    } else {
      uint16_t count = rd16(DP + 0x3C);
      wr16(DP + 0x3C, (uint16_t)(count - 1));
      if(rd16(DP + 0x3C) == 0) {
        uint16_t keep = rd16(DP + 0x14);
        func_80D6F3(rom16(0x81CB14 + (uint32_t)(uint16_t)(n << 1)));
        wr16(DP + 0x14, keep);
        count = 0;
      }
      if(count & 0x0010) race_light_tile(y, n, 0x25C3);
      else race_light_lit(y, n);
    }
    y = (uint16_t)(y + 4);
    wr16(DP + 0x14, (uint16_t)(rd16(DP + 0x14) + 1));
  } while(rd16(DP + 0x14) != 0x000C);
  wr16(DP + 0x74, y);
}

// D+$14 = 0 - D+$22 plus `x`; whether that lies on screen ($100 and on, or below `low`,
// is off, each judged by the sign of the difference).
static bool hud_on_screen(uint16_t x, uint16_t low) {
  const uint16_t at = (uint16_t)(x - rd16(DP + 0x22));
  wr16(DP + 0x14, at);
  return ((uint16_t)(at - 0x0100) & 0x8000) && !((uint16_t)(at - low) & 0x8000);
}

// Bits 8-23 of `m7a` times the signed byte `b` on the PPU multiplier (matrix B gets `b`
// over the latch, the high byte of `m7a` just written); the low byte in *low.
static uint16_t hud_product(uint16_t m7a, uint8_t b, uint8_t* low) {
  video_set_m7a((int16_t)m7a);
  video_set_m7b((int16_t)(b << 8 | m7a >> 8));
  const uint32_t p = (uint32_t)((int32_t)(int16_t)m7a * (int8_t)b);
  *low = (uint8_t)p;
  return (uint16_t)(p >> 8);
}

// One OAM column value from an entry's +$06 word: its high byte with bit 7 of the low byte
// rounded in, less 8, the high byte a sign.
static uint16_t hud_column(uint16_t w) {
  const uint16_t m = (uint16_t)(w & 0xFF80);
  uint16_t a = (uint16_t)(m << 1 | (m >= 0x8000));
  a = (uint16_t)((uint16_t)(a << 8 | a >> 8) - 8);
  return (a & 0xFF00) ? (uint16_t)(a | 0xFF00) : a;
}

// TG3000_ROUTINE($80:D823) race_hud_sprites
void race_hud_sprites(void) {
  window_shape_a((uint16_t)(rd8(DP + 0x37) << 8 | rd8(DP + 0x35)), 0x0000);
  window_shape_b(rd16(DP + 0x24), 0x0100);
  if(hud_on_screen(0, 0xFFF0)) {
    wr16(DP + 0x16, 0x0098);
    wr16(DP + 0x18, 0);
    sprite_group(DP, 0x81C272);
  }
  wr16(DP + 0x14, 0x2200);
  sprite_number_point_right((uint16_t)binary_to_bcd(rd16(DP + 0x28)), 0x4846);
  wr16(DP + 0x14, 0x2200);
  uint8_t low;
  uint16_t a = hud_product(rd16(DP + 0x28), 0x4D, &low);
  wr8(DP + 0x17, (uint8_t)(low << 1));
  a = (uint16_t)(a << 1 | (low >> 7));
  const uint16_t w16 = rd16(DP + 0x16);
  wr16(DP + 0x16, (uint16_t)(w16 << 1));
  a = (uint16_t)(a + rd16(DP + 0x28) + (w16 >> 15));
  sprite_number_point_right((uint16_t)binary_to_bcd(a), 0x5046);
  wr16(DP + 0x14, 0x2200);
  sprite_number(DP, binary_to_bcd(hud_product((uint16_t)(rd8(DP + 0x35) << 8), 0x63, &low)), 0x2CD2);
  wr16(DP + 0x14, 0x2200);
  a = hud_product((uint16_t)(rd8(DP + 0x37) << 8), 0x41, &low);
  sprite_number_right(binary_to_bcd((uint16_t)(a << 1 | (low >> 7))), 0x7662);
  wr16(DP + 0x14, 0x2200);
  sprite_text_centered(0x860000 | bus_rd16(0x86CEA7 + (uint32_t)(uint16_t)(rd16(0x0368) << 1)), 0xBDA8);
  wr16(DP + 0x14, 0x2200);
  sprite_text_centered(0x860000 | bus_rd16(0x86CEBF + (uint32_t)(uint16_t)(rd16(0x0444) << 1)), 0xCDA8);
  wr16(DP + 0x14, 0x2200);
  sprite_text_centered(0x860000 | bus_rd16(0x86D0A7 + (uint32_t)(uint16_t)(rd16(0x0444) << 1)), 0x6832);
  race_lights_draw();
  uint16_t d = (uint16_t)(rd16(DP + 0x32) - rd16(DP + 0x28));
  if(!((uint16_t)(d - 0x0010) & 0x8000)) d = 0x0010;
  else if((uint16_t)(d - 0xFFF0) & 0x8000) d = 0xFFF0;  // never run
  wr16(DP + 0x28, (uint16_t)(d + rd16(DP + 0x28)));
  wr16(DP + 0x1C, 0);
  if(rd16(0x070A) != 0) {
    uint16_t x = 0x000A;  // the first pass and the second both use entry $0A
    do {
      if(hud_on_screen(rd16(0x0706 + (uint32_t)x), 0xFFE0)) {
        wr16(DP + 0x16, 0x0098);
        wr16(DP + 0x18, rd16(0x0708 + (uint32_t)x));
        sprite_group(DP, 0x810000 | rd16(0x0704 + (uint32_t)x));
      }
      wr16(DP + 0x1C, (uint16_t)(rd16(DP + 0x1C) + 0x000A));
      x = rd16(DP + 0x1C);
    } while(rd16(0x0700 + (uint32_t)x) != 0);
  }
  if(rd16(0x0700) != 0 && hud_on_screen(rd16(0x0706), 0xFFD0)) {  // never run
    wr16(DP + 0x16, 0x0098);
    wr16(DP + 0x18, 0x0800);
    sprite_group(DP, 0x81C2F2);
  }
  uint16_t y = rd16(0x0174), x = 0;
  for(; x != 0x0010 && y < 0x01FC; x = (uint16_t)(x + 8), y = (uint16_t)(y + 8)) {
    const uint16_t c = hud_column(rd16(0x0F06 + (uint32_t)x));
    wr16(0x0B00 + (uint32_t)y, c);
    wr16(0x0B04 + (uint32_t)y, c);
    const uint16_t pos = (uint16_t)((c & 0x00FF) | rd16(0x0F02 + (uint32_t)x));
    wr16(0x0900 + (uint32_t)y, pos);
    wr16(0x0904 + (uint32_t)y, (uint16_t)(pos + 0x0800));
    wr16(0x0902 + (uint32_t)y, rd16(0x0F04 + (uint32_t)x));
    wr16(0x0906 + (uint32_t)y, (uint16_t)(rd16(0x0F04 + (uint32_t)x) + 1));
    wr16(0x0B02 + (uint32_t)y, 0);
    wr16(0x0B06 + (uint32_t)y, 0);
  }
  for(; x != 0x0200 && y < 0x0200; x = (uint16_t)(x + 8), y = (uint16_t)(y + 4)) {
    const uint16_t c = hud_column(rd16(0x0F06 + (uint32_t)x));
    wr16(0x0B00 + (uint32_t)y, c);
    wr16(0x0900 + (uint32_t)y, (uint16_t)((c & 0x00FF) | rd16(0x0F02 + (uint32_t)x)));
    wr16(0x0902 + (uint32_t)y, rd16(0x0F04 + (uint32_t)x));
    wr16(0x0B02 + (uint32_t)y, 0);
  }
  wr16(0x0174, y);
}
