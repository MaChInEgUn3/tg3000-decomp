// SPDX-License-Identifier: GPL-3.0-or-later
#include "scale.h"

#include "platform.h"
#include "tg3000.h"

// TG3000_ROUTINE($80:F709) inverse_scale
//
// For divisors above $100 the quotient is floor(65536 / divisor); below that it no longer
// fits in 8 bits and the original's truncation is kept as is. The multiply by 42 ($2A) is
// done on the CPU's hardware multiplier in the original.
uint16_t inverse_scale(uint16_t dp) {
  uint16_t divisor = rd16(dp + SCALE_DIVISOR);
  wr16(dp + SCALE_QUOTIENT, 0x0100);
  if(divisor == 0x0100) {
    wr16(dp + SCALE_RESULT, 0x2A00);
    return 0x2A00;
  }
  uint16_t remainder = 0x0200, quotient = 0;
  for(int bit = 0; bit < 8; bit++) {
    int take = remainder >= divisor;
    if(take) remainder = (uint16_t)(remainder - divisor);
    quotient = (uint16_t)((quotient << 1) | take);
    remainder = (uint16_t)(remainder << 1);
  }
  wr16(dp + SCALE_QUOTIENT, quotient);
  uint16_t result = (uint16_t)((quotient & 0xFF) * 0x2A);
  wr16(dp + SCALE_RESULT, result);
  return result;
}

// TG3000_ROUTINE($80:F38D) mode7_zoom_view
//
// The offsets are computed a byte at a time in the original; the high byte is the borrow
// of the low one, so they are the 16-bit differences.
uint16_t mode7_zoom_view(uint16_t dp) {
  int16_t scale = (int16_t)rd16(dp + ZOOM_SCALE);
  video_set_m7a(scale);
  video_set_m7d(scale);
  uint16_t h = (uint16_t)(0x80 - rd8(dp + ZOOM_CENTRE_X));
  uint16_t v = (uint16_t)(0x6F - rd8(dp + ZOOM_CENTRE_Y));
  video_set_bg1_hofs(h);
  video_set_bg1_vofs(v);
  return v;
}

// TG3000_ROUTINE($8A:8B0A) mode7_view
//
// The path for a set bit continues at $8A:8B9D, past the routine's RTS; no recorded run
// takes it (it reads as data in the map), the fuzz does.
uint16_t mode7_view(uint16_t dp) {
  uint16_t b = rd16(dp + VIEW_LINE_B), a = rd16(dp + VIEW_LINE_A);
  for(int i = 0; i < 3; i++) {
    wr16(0x0D01 + 3 * i, b);
    wr16(0x0D21 + 3 * i, a);
  }
  video_set_m7a((int16_t)rd16(dp + VIEW_MATRIX));
  video_set_m7b((int16_t)rd16(dp + VIEW_MATRIX + 2));
  video_set_m7c((int16_t)rd16(dp + VIEW_MATRIX + 4));
  video_set_m7d((int16_t)rd16(dp + VIEW_MATRIX + 6));
  video_set_m7x(0x0080);
  video_set_m7y(0x0040);
  uint16_t scroll = rd16(dp + VIEW_SCROLL);
  video_set_bg2_hofs(scroll);
  wr16(0x0D2A, (uint16_t)((scroll + 0x0200) << 1));
  uint16_t scene = (uint16_t)(rd16(VIEW_SCENE) << 3);
  bool follow = bus_rd16(0x8A9351 + scene) & 0x0004;  // a long read: the index may cross into bank $8B
  uint16_t y = rd16(dp + VIEW_BG2_Y);
  bool keep = !((uint16_t)(y + 0x40) & 0x8000);
  if(!follow) video_set_bg2_vofs(0x0080);
  if(keep) {
    wr16(0x0D0A, y);
    if(follow) video_set_bg2_vofs(y);
  }
  return y;
}
