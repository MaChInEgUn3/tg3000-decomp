// SPDX-License-Identifier: GPL-3.0-or-later
#include "stars.h"

#include <stddef.h>

#include "platform.h"
#include "rng.h"
#include "tg3000.h"

// Arithmetic shift right by one: the original's CMP #$8000 / ROR A.
static uint16_t half_signed(uint16_t v) { return (uint16_t)((v >> 1) | (v & 0x8000)); }

// TG3000_ROUTINE($80:FA3F) star_spawn
// TG3000_ROUTINE($8A:9863) star_spawn (copy in bank $8A, calling its own rng_next copy)
uint16_t star_spawn(uint16_t slot) {
  wr16(STAR_X + slot, half_signed(rng_next(RNG_STATE, NULL)));
  uint16_t r = rng_next(RNG_STATE, NULL);
  wr16(STAR_Y + slot, half_signed((uint16_t)((r << 8) | (r >> 8))));  // XBA: bytes swapped
  wr16(STAR_DEPTH + slot, 0x7F00);
  return 0x7F00;
}

// TG3000_ROUTINE($80:E166) star_random_y
uint16_t star_random_y(uint16_t slot) {
  uint16_t r;
  do r = rng_next(RNG_STATE, NULL); while(r >= 0xDC00);
  r &= 0xFF00;
  wr16(STAR_Y + slot, r);
  return r;
}

// TG3000_ROUTINE($80:E0D6) star_layers_init
uint16_t star_layers_init(void) {
  static const struct { uint16_t speed, tile, end; } layers[4] = {
    {0x0100, 0x018C, 0x0010}, {0x0080, 0x018E, 0x0060}, {0x0040, 0x018F, 0x0120}, {0x0020, 0x019A, 0x0200},
  };
  uint16_t slot = 0;
  for(int i = 0; i < 4; i++) {
    do {
      wr16(0x0F00 + slot, layers[i].speed);
      star_random_y(slot);
      wr16(0x0F04 + slot, layers[i].tile);
      wr16(0x0F06 + slot, rng_next(RNG_STATE, NULL) & 0x7FFF);
      slot += 8;
    } while(slot != layers[i].end);
  }
  return slot;
}

// TG3000_ROUTINE($80:DACF) star_layers_move
void star_layers_move(void) {
  for(uint32_t x = 0; x < 0x0200; x += 8) {
    uint16_t pos = rd16(0x0F06 + x), speed = rd16(0x0F00 + x);
    uint16_t moved = (uint16_t)(pos - speed);
    if(pos < speed) moved = (uint16_t)(moved + 0x8800);
    wr16(0x0F06 + x, moved);
  }
}

// The CPU's 16 / 8 divider: $FFFF for a divisor of 0.
static uint16_t cpu_divide(uint16_t n, uint8_t d) { return d ? (uint16_t)(n / d) : 0xFFFF; }

// One axis: `base` plus or minus the quotient of the value's magnitude by the speed.
static uint16_t star_axis(uint16_t base, uint16_t v, uint8_t speed) {
  if(v & 0x8000) return (uint16_t)(base - cpu_divide((uint16_t)-v, speed));
  return (uint16_t)(base + cpu_divide(v, speed));
}

static void stars_draw_using(uint16_t step, uint16_t multiplier, uint32_t tiles, uint16_t (*spawn)(uint16_t)) {
  wr16(DP + 0x18, multiplier);
  wr16(DP + 0x16, step);
  uint16_t y = 0, x = rd16(DP + 0x74);
  for(;;) {
    const uint8_t speed = rd8(0x0F05 + (uint32_t)y);
    wr16(DP + 0x19, (uint16_t)(speed * rd8(DP + 0x18)));  // the CPU multiplier
    const uint16_t sx = star_axis(0x0080, (uint16_t)(rd16(DP + 0x19) + rd16(0x0F00 + (uint32_t)y)), speed);
    if(sx & 0xFF00) {
      spawn(y);
      continue;
    }
    wr16(DP + 0x14, sx);
    const uint16_t sy = star_axis(0x005C, (uint16_t)(rd16(0x0F02 + (uint32_t)y) - rd16(DP + 0x19)), speed);
    if(sy >= 0x00E0) {
      spawn(y);
      continue;
    }
    wr16(0x0900 + (uint32_t)x, (uint16_t)(((sy & 0x00FF) << 8) | rd16(DP + 0x14)));
    wr16(0x0902 + (uint32_t)x, (uint16_t)(bus_rd16(tiles + (speed & 0x7C)) | rd16(DP + 0x80)));
    wr16(0x0B01 + (uint32_t)x, 0);
    const uint16_t depth = (uint16_t)(rd16(0x0F04 + (uint32_t)y) - rd16(DP + 0x16));
    if(depth < 0x0100) {
      spawn(y);
      continue;
    }
    wr16(0x0F04 + (uint32_t)y, depth);
    x = (uint16_t)(x + 4);
    y = (uint16_t)(y + 6);
    if(y == 0x0300) break;
  }
  wr16(DP + 0x74, x);
}

// TG3000_ROUTINE($80:F950) stars_draw
void stars_draw(uint16_t step, uint16_t multiplier) { stars_draw_using(step, multiplier, 0x81D692, star_spawn); }

// TG3000_ROUTINE($8A:9773) stars_draw_8a
void stars_draw_8a(uint16_t step, uint16_t multiplier) {
  stars_draw_using(step, multiplier, 0x86F46B, star_spawn);  // star_spawn_8a is the same C
}

// d times the signed byte `r` on the PPU multiplier: matrix A = d, B = r over the latch
// (the high byte of d, just written); returns bits 8-23 of the product.
static uint16_t star_scale(uint16_t d, uint8_t r) {
  video_set_m7a((int16_t)d);
  video_set_m7b((int16_t)(r << 8 | d >> 8));
  return (uint16_t)((uint32_t)((int32_t)(int16_t)d * (int8_t)r) >> 8);
}

// TG3000_ROUTINE($8A:971B) stars_scatter_8a
void stars_scatter_8a(void) {
  wr16(DP + 0x14, 0x2000);
  for(uint32_t x = 0; x < 0x300; x += 6) {
    const uint16_t d = rd16(DP + 0x14);
    wr16(STAR_X + x, star_scale(d, (uint8_t)rng_next(RNG_STATE, NULL)));
    wr16(STAR_Y + x, star_scale(d, (uint8_t)(rng_next(RNG_STATE, NULL) >> 8)));
    wr16(STAR_DEPTH + x, d);
    wr16(DP + 0x14, (uint16_t)(d + 0x00BE));
  }
}
