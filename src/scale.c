// SPDX-License-Identifier: GPL-3.0-or-later
#include "scale.h"

#include "line_tables.h"
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

// TG3000_ROUTINE($80:F226) mode7_zoom_step
uint16_t mode7_zoom_step(uint16_t dp) {
  mode7_zoom_view(dp);
  wr16(dp + 0x40, (uint16_t)(rd16(dp + 0x40) + 1));
  if(rd16(dp + 0x40) >= 0x0320) {
    uint16_t v = (uint16_t)(rd16(dp + 0x7C) - 0x0010);
    wr16(dp + 0x7C, (v & 0x8000) ? 0 : v);
  }
  inverse_scale(dp);
  return func_80F745();
}

// TG3000_ROUTINE($80:F2A0) title_zoom_pan
uint16_t title_zoom_pan(bool* done) {
  if(rd16(DP + 0x30) != 0x000F) wr16(DP + 0x30, (uint16_t)(rd16(DP + 0x30) + 1));
  *done = (rd16(DP + 0x3C) & 0xFF00) == 0xC000 && (rd16(DP + 0x3A) & 0xFF00) == 0x1C00;
  if(*done) {
    wr16(DP + 0x3E, (uint16_t)(rd16(DP + 0x3E) + 1));
    return 0;
  }
  uint16_t at = (uint16_t)(rd16(DP + 0x3C) + rd16(DP + 0x40));
  wr16(DP + 0x3C, at);
  if(at >= 0xF000) wr16(DP + 0x40, 0xF800);
  else if(at < 0x1000) wr16(DP + 0x40, 0x0800);
  wr16(DP + 0x3A, (uint16_t)(rd16(DP + 0x3A) - 0x0080));
  inverse_scale(DP);
  return func_80F745();
}

// TG3000_ROUTINE($80:F2EE) title_zoom_cycle
uint16_t title_zoom_cycle(void) {
  uint16_t n = rd16(DP + 0x30);
  if(n == 0x001F) n = 0x000E;
  wr16(DP + 0x30, (uint16_t)(n + 1));
  inverse_scale(DP);
  return func_80F745();
}

// TG3000_ROUTINE($80:F595) title_zoom_drop
uint16_t title_zoom_drop(void) {
  uint16_t y = (uint16_t)(rd16(DP + 0x7E) - 1);
  if((uint16_t)(y - 0xFFB8) & 0x8000) {
    wr16(DP + 0x3E, (uint16_t)(rd16(DP + 0x3E) + 1));
    y = 0xFFB8;
  }
  wr16(DP + 0x7E, y);
  for(uint32_t x = 0; x <= 0x16; x += 2) {
    wr16(0x10214 + x, 0x1001);
    wr16(0x10254 + x, 0x1002);
  }
  inverse_scale(DP);
  return func_80F745();
}

// TG3000_ROUTINE($80:F5BF) title_zoom_count
uint16_t title_zoom_count(void) {
  const uint16_t n = (uint16_t)(rd16(DP + 0x30) - 1);
  wr16(DP + 0x30, n);
  if(n == 0) wr16(DP + 0x3E, (uint16_t)(rd16(DP + 0x3E) + 1));
  inverse_scale(DP);
  return func_80F745();
}

// An arithmetic shift right (the original's CMP #$8000, ROR).
static uint16_t asr16(uint16_t v) { return (uint16_t)((v >> 1) | (v & 0x8000)); }

// TG3000_ROUTINE($80:F303) title_zoom_settle
uint16_t title_zoom_settle(void) {
  mode7_zoom_view(DP);
  uint16_t t = asr16((uint16_t)(rd16(DP + 0x7A) - 0x00E2));
  if(t >= 0x0200) t = 0x0200;
  wr16(DP + 0x7C, t);
  uint16_t v = (uint16_t)(rd16(DP + 0x7A) - asr16(asr16(t)));
  if((uint16_t)(v - 0x0100) & 0x8000) {
    wr16(DP + 0x7C, 0);
    v = 0x0100;
  }
  wr16(DP + 0x7A, v);
  uint16_t s = (uint16_t)(rd16(DP + 0x86) >> 3);
  if(s >= 0x00D0) s = 0x00D0;
  uint16_t w = (uint16_t)(rd16(DP + 0x86) - (uint16_t)(s + 1));
  if(w & 0x8000) w = 0;
  wr16(DP + 0x86, w);
  const uint16_t p = (uint16_t)(0x80 * (uint8_t)(w >> 8));  // the CPU multiplier
  wr16(DP + 0x3C, (uint16_t)((0x4000 + p) << 1));
  wr16(DP + 0x3A, (uint16_t)((0x2E00 - p) << 1));
  uint16_t n = rd16(DP + 0x30);
  if(n != 0x000F) {
    if(n == 0x001F) n = 0x000E;
    wr16(DP + 0x30, (uint16_t)(n + 1));
  }
  inverse_scale(DP);
  return func_80F745();
}

// TG3000_ROUTINE($80:F247) title_stars_spread
uint16_t title_stars_spread(bool carry) {
  wr16(DP + 0x7C, 0);
  wr16(DP + 0x86, 0x4000);
  wr16(DP + 0x18, 0x0040);
  for(uint32_t y = 0; y < 0x80 * 6; y += 6) {
    const uint16_t p = (uint16_t)(rd8(0x0F05 + y) * rd8(DP + 0x18));  // the CPU multiplier
    wr16(DP + 0x19, p);
    const uint32_t sum = (uint32_t)p + rd16(0x0F02 + y) + (carry ? 1u : 0u);
    wr16(0x0F02 + y, (uint16_t)sum);
    wr16(0x0F00 + y, (uint16_t)(rd16(0x0F00 + y) - rd16(DP + 0x19)));
    carry = false;  // the ADC #6 of the next index never carries
  }
  wr16(DP + 0x3E, (uint16_t)(rd16(DP + 0x3E) + 1));
  wr16(DP + 0x3A, 0x9B00);
  wr16(DP + 0x3C, 0x1000);
  wr16(DP + 0x40, 0x0800);
  inverse_scale(DP);
  return func_80F745();
}

// TG3000_ROUTINE($80:F424) title_mode7_setup
uint16_t title_mode7_setup(void) {
  for(uint16_t w = 0; w < 16; w++) video_write_vram_word(w, 0);
  video_upload_vram(0x3C00, 0x7E2940, 0x0800);
  video_set_bg_map(1, 0x3C);
  video_set_bg1_hofs(0);
  video_set_bg1_vofs(0x0003);
  dma_hdma_set_table(1, 0xD719);
  dma_hdma_set_table(3, 0xD727);
  wr16(DP + 0x80, 0x2000);
  wr16(DP + 0x3E, (uint16_t)(rd16(DP + 0x3E) + 1));
  inverse_scale(DP);
  return func_80F745();
}
