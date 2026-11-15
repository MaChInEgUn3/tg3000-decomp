// SPDX-License-Identifier: GPL-3.0-or-later
#include "hud.h"

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
