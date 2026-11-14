// SPDX-License-Identifier: GPL-3.0-or-later
#include "stars.h"

#include <stddef.h>

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
