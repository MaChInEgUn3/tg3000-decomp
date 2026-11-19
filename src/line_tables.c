// SPDX-License-Identifier: GPL-3.0-or-later
#include "line_tables.h"

#include "platform.h"
#include "tg3000.h"

// Direct-page variables (D = $0100).
#define V32 (DP + 0x32)
#define BACK (DP + 0x34)   // back-buffer offset into the tables
#define FRONT (DP + 0x36)  // front-buffer offset
#define V38 (DP + 0x38)    // inverse_scale's result
#define V3A (DP + 0x3A)
#define V3C (DP + 0x3C)
#define T14 (DP + 0x14)
#define T16 (DP + 0x16)
#define T18 (DP + 0x18)

static uint16_t swap_bytes(uint16_t v) { return (uint16_t)((v << 8) | (v >> 8)); }

// One 3-byte entry: the original stores a word at +0 (whose high byte the next store
// overwrites) and a word at +1, so both stores are kept in that order.
static void entry(uint16_t table, uint16_t* x, uint16_t count, uint16_t value) {
  wr16(table + *x, count);
  wr16(table + *x + 1, value);
  *x = (uint16_t)(*x + 3);
}

// TG3000_ROUTINE($80:F745) func_80F745
uint16_t func_80F745(void) {
  uint16_t x = rd16(BACK);
  uint16_t v3a = rd16(V3A);
  if(v3a == 0) {
    wr16(LINES_A + x, 1);
    wr16(LINES_B + x, 1);
    wr16(LINES_A + x + 1, 0x00FF);
    wr16(LINES_B + x + 1, 0x00FF);
    wr16(LINES_A + x + 3, 0);
    wr16(LINES_B + x + 3, 0);
  } else {
    // table A
    wr16(T18, rd16(V38) & 0xFF00);
    uint16_t a = (uint16_t)((v3a & 0xFF00) - rd16(T18));
    if(a != 0) {
      a = swap_bytes(a);
      if(a >= 0x80) {
        uint16_t hi = rd16(V3C) & 0xFF00;
        wr16(T14, hi);
        entry(LINES_A, &x, (uint16_t)(a - 0x7F), (uint16_t)(swap_bytes(hi) | hi));
        a = 0x7F;
      }
      uint16_t hi = rd16(V3C) & 0xFF00;
      wr16(T14, hi);
      wr16(T16, (uint16_t)(swap_bytes(hi) | hi));
      entry(LINES_A, &x, a, rd16(T16));
    }
    wr16(T14, 0x8081);
    uint16_t h38 = rd16(V38) & 0xFF00;
    if(h38 != 0) {
      uint16_t t = (uint16_t)((h38 + rd16(V3C)) & 0xFF00);
      wr16(T14, t);
      uint16_t u = (uint16_t)((t - (uint16_t)(h38 << 1)) & 0xFF00);
      wr16(T14, (uint16_t)(swap_bytes(u) | t));
      entry(LINES_A, &x, 1, rd16(T14));
      uint16_t n = (uint16_t)((h38 >> 8) - 1);
      if(n != 0) entry(LINES_A, &x, n, rd16(T14));
    }
    entry(LINES_A, &x, 1, 0xFF00);
    if(h38 != 0) {
      uint16_t n = (uint16_t)(h38 - 0x0100);
      if(n != 0) entry(LINES_A, &x, swap_bytes(n), rd16(T14));
      entry(LINES_A, &x, 1, rd16(T14));
    }
    entry(LINES_A, &x, 1, rd16(T16));
    wr16(LINES_A + x, 0);

    // table B
    x = rd16(BACK);
    wr16(T16, rd16(V38) & 0xFF00);
    a = (uint16_t)(((swap_bytes((uint16_t)((v3a & 0xFF00) - rd16(T16))) & 0x00FF)) + 1);
    if(a >= 0x80) {
      entry(LINES_B, &x, (uint16_t)(a - 0x7F), 0x00FF);
      a = 0x7F;
    }
    entry(LINES_B, &x, a, 0x00FF);
    wr16(T16, rd16(V32) & 0xFF00);
    if(rd16(T16) != 0) {  // never seen in the movies so far
      entry(LINES_B, &x, swap_bytes(rd16(T16)), (uint16_t)(rd16(T14) + 0xFF01));
      entry(LINES_B, &x, 1, 0x00FF);
      wr16(DP + 0x17, (uint16_t)(rd16(DP + 0x17) + 1));
    }
    wr16(T16, swap_bytes(rd16(T16)));
    uint16_t lines = rd16(V38) >> 8;
    if(lines != 0) {
      uint16_t n = (uint16_t)(((lines - 1) << 1) + 1 - rd16(T16));
      if(n != 0) {
        entry(LINES_B, &x, n, (uint16_t)(rd16(T14) + 0xFF01));
        entry(LINES_B, &x, 1, 0x00FF);
      }
    }
    wr16(LINES_B + x, 0);
  }
  // swap the buffers
  uint16_t back = rd16(BACK);
  wr16(BACK, rd16(FRONT));
  wr16(FRONT, back);
  return back;
}

// TG3000_ROUTINE($80:FA5B) window_curve_build
void window_curve_build(void) {
  const uint8_t r = rd8(DP + 0x42);
  wr16(DP + 0x14, 0);
  wr16(DP + 0x16, 0);
  wr16(DP + 0x17, r ? (uint16_t)(0xA000 / r) : 0xFFFF);  // the CPU divider
  uint16_t y = 0x00B8;
  for(;;) {
    const uint16_t x = rd16(DP + 0x15);
    const uint8_t lo = bus_rd8(0x81DA4C + x), hi = bus_rd8(0x81DACC + x);
    const int16_t m7a = (int16_t)(lo | hi << 8);
    video_set_m7a(m7a);
    video_set_m7b((int16_t)(r << 8 | hi));  // the latch holds the high byte just written
    const uint8_t t = (uint8_t)((uint32_t)((int32_t)m7a * (int8_t)r) >> 8);
    const uint8_t left = (uint8_t)(0x7F - t);
    wr8(0x1501 + (uint32_t)y, (left & 0x80) ? 0 : left);
    wr8(0x1502 + (uint32_t)y, (0x7E + t > 0xFF) ? 0xFF : (uint8_t)(0x7E + t));
    const uint16_t p = (uint16_t)(rd16(DP + 0x14) + rd16(DP + 0x17));
    wr16(DP + 0x14, p);
    y = (uint16_t)(y - 2);
    if(y == 0) break;
    if(p >= 0x8000) {
      for(; y != 0; y = (uint16_t)(y - 2)) wr16(0x1501 + (uint32_t)y, 0x00FF);
      break;
    }
  }
  for(uint16_t x = 0x00B6, to = 0x00BA; x != 0; x = (uint16_t)(x - 2), to = (uint16_t)(to + 2))
    wr16(0x1502 + (uint32_t)to, rd16(0x1501 + (uint32_t)x));
  wr8(0x1500, 0xDD);
  wr8(0x15BB, 0xDD);
  wr8(0x1674, 0);
}
