// SPDX-License-Identifier: GPL-3.0-or-later
#include "hud.h"

#include "misc.h"
#include "platform.h"
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
