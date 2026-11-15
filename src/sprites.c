// SPDX-License-Identifier: GPL-3.0-or-later
#include "sprites.h"

#include "tg3000.h"

#define SPRITE_COUNT 128

// TG3000_ROUTINE($81:8101) oam_high_table (the table at $0200 + D+$6C)
// TG3000_ROUTINE($81:80F6) oam_high_table (the table at $0B00)
void oam_high_table(uint16_t dp, uint16_t table) {
  for(int j = 0; j < SPRITE_COUNT / 4; j++) {
    uint8_t packed = 0;
    for(int k = 0; k < 4; k++) {
      uint16_t e = (uint16_t)(table + 4 * (4 * j + k));
      uint8_t b1 = rd8((uint16_t)(e + 1)), b2 = rd8((uint16_t)(e + 2));
      packed = (uint8_t)((packed >> 1) | (b1 & 0x80));          // bit 7 of byte 1
      packed = (uint8_t)((packed >> 1) | ((b2 & 1) << 7));      // bit 0 of byte 2
      wr8((uint16_t)(e + 1), (uint8_t)(b1 << 1));
      wr8((uint16_t)(e + 2), (uint8_t)(b2 >> 1));
    }
    wr8((uint16_t)(table + j), packed);
  }
  // hide what is left of last frame's sprites; slots are multiples of 4 (the original
  // counts (last - end) / 4 down, and would run 65,536 times for a gap under 4)
  uint16_t end = rd16(dp + 0x74), last = rd16(dp + 0x72);
  if(end < last) {
    uint16_t n = (uint16_t)(last - end) >> 2;
    for(uint16_t i = 0, s = end; i < n; i++, s = (uint16_t)(s + 4)) wr16(0x0900 + s, 0xE0E0);
  }
}

// TG3000_ROUTINE($81:80D0) oam_finish_0b00 (long call)
void oam_finish_0b00(uint16_t dp) {
  oam_high_table(dp, 0x0B00);
  wr16(dp + 0x72, rd16(dp + 0x74));  // this frame's end becomes "last frame's"
  wr16(dp + 0x74, 0);
}

// TG3000_ROUTINE($81:80DA) oam_finish (long call)
void oam_finish(uint16_t dp) {
  uint16_t built = rd16(dp + 0x6C), other = rd16(dp + 0x6E);
  oam_high_table(dp, (uint16_t)(built + 0x200));
  wr16(dp + 0x72, rd16(dp + 0x70));
  wr16(dp + 0x70, rd16(dp + 0x74));
  wr16(dp + 0x74, (uint16_t)(other - 0x0900));  // the next frame fills the other buffer
  wr16(dp + 0x6C, other);
  wr16(dp + 0x6E, built);
}

// TG3000_ROUTINE($80:89D1) oam_finish_0b00_near
void oam_finish_0b00_near(uint16_t dp) { oam_finish_0b00(dp); }

// TG3000_ROUTINE($80:89D6) oam_finish_near
void oam_finish_near(uint16_t dp) { oam_finish(dp); }
