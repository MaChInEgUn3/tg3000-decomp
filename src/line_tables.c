// SPDX-License-Identifier: GPL-3.0-or-later
#include "line_tables.h"

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
