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

#define OBJ_USED 0x0500
#define OBJ_X 0x0600
#define OBJ_Y 0x0700
#define OBJ_TILE 0x0800
#define OAM_HIGH 0x0B00
#define OBJ_DP 0x0100  // D during it

static uint16_t rotate2(uint16_t at, bool* carry) {  // ASL then ROL of a word in WRAM
  uint16_t v = rd16(at);
  bool c1 = v >> 15;
  v = (uint16_t)(v << 1);
  *carry = v >> 15;
  v = (uint16_t)(v << 1 | c1);
  wr16(at, v);
  return v;
}

// TG3000_ROUTINE($8A:8E1B) oam_from_objects (long)
uint16_t oam_from_objects(void) {
  for(uint16_t i = 0; i < 0x20; i = (uint16_t)(i + 2)) wr16((uint16_t)(OAM_HIGH + i), 0);
  wr16(OBJ_DP + 0x28, 0);
  wr16(OBJ_DP + 0x2A, 0);
  wr16(OBJ_DP + 0x2C, 1);
  wr16(OBJ_DP + 0x2E, 2);
  for(uint16_t x = 0; x != 0x100; x = (uint16_t)(x + 2)) {
    if(rd16((uint16_t)(OBJ_USED + x)) == 0) continue;
    wr16(OBJ_DP + 0x32, (uint16_t)(rd16((uint16_t)(OBJ_X + x)) & 0x01FF));
    uint16_t y = rd16((uint16_t)(OBJ_Y + x));
    if(y & 0x8000) continue;
    if(!((uint16_t)(y - 0xF0) & 0x8000)) continue;  // CMP #$F0 / BPL
    wr16(OBJ_DP + 0x26, y);
    uint16_t hi = (uint16_t)(OAM_HIGH + rd16(OBJ_DP + 0x28));
    if(rd16(OBJ_DP + 0x32) & 0xFF00) wr16(hi, (uint16_t)(rd16(hi) | rd16(OBJ_DP + 0x2C)));
    if(!(rd16((uint16_t)(OBJ_USED + x)) & 0x8000)) wr16(hi, (uint16_t)(rd16(hi) | rd16(OBJ_DP + 0x2E)));
    uint16_t s = rd16(OBJ_DP + 0x2A);
    wr16((uint16_t)(0x0902 + s), rd16((uint16_t)(OBJ_TILE + x)));
    wr8((uint16_t)(0x0900 + s), rd8(OBJ_DP + 0x32));
    wr8((uint16_t)(0x0901 + s), rd8(OBJ_DP + 0x26));
    wr16(OBJ_DP + 0x2A, (uint16_t)(s + 4));
    bool c;
    rotate2(OBJ_DP + 0x2E, &c);
    rotate2(OBJ_DP + 0x2C, &c);
    if(c) {  // eight sprites done: the next high-table word
      wr16(OBJ_DP + 0x2C, 1);
      wr16(OBJ_DP + 0x2E, 2);
      wr16(OBJ_DP + 0x28, (uint16_t)(rd16(OBJ_DP + 0x28) + 2));
    }
  }
  uint16_t end = rd16(OBJ_DP + 0x2A), last = rd16(OBJ_DP + 0x30);
  if(end != last && ((uint16_t)(end - last) & 0x8000))  // fewer than last time: hide the rest
    for(uint16_t s = end; s != last; s = (uint16_t)(s + 4)) wr16((uint16_t)(0x0900 + s), 0xE000);
  wr16(OBJ_DP + 0x30, end);
  return end;
}
