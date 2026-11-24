// SPDX-License-Identifier: GPL-3.0-or-later
#include "dsp4.h"

int32_t dsp4_multiply(int16_t a, int16_t b) { return (int32_t)a * b; }

// The width in nibble j (0: bits 15-12 ... 3: bits 3-0), signed, times 48.
static int16_t dsp4_width(uint16_t row, int j) {
  int n = (row >> (12 - 4 * j)) & 0xF;
  if(n >= 8) n -= 16;
  return (int16_t)(n * 48);
}

void dsp4_edges(uint16_t t, uint16_t near, uint16_t far, int16_t out[4]) {
  for(int j = 0; j < 4; j++) {
    int32_t a = dsp4_width(near, j), b = dsp4_width(far, j);
    int32_t p = (b - a) * (int32_t)t;
    int32_t step = p >= 0 ? p / 32768 : -((-p + 32767) / 32768);  // floor(p / 2^15)
    out[j] = (int16_t)(a + step);
  }
}

// (dsp4-11a: measured on the firmware, every edge alike: the edge times 1364 / 65536, rounded
// down, not a division by 48: 48 gives 0 and 49 gives 1, -48 gives -1 and -49 gives -2, and
// from about 1,000 on the steps drift from 48 by one now and then.)
uint16_t dsp4_row(const int16_t edge[4]) {
  uint16_t row = 0;
  for(int j = 0; j < 4; j++) {
    const int32_t p = (int32_t)edge[j] * 1364;
    const int32_t q = p >= 0 ? p / 65536 : -((-p + 65535) / 65536);  // floor(p / 2^16)
    row = (uint16_t)((row << 4) | (q & 0xF));
  }
  return row;
}

uint16_t dsp4_sprite(uint16_t x, uint16_t y, uint16_t attr, uint16_t out[2]) {
  out[0] = (uint16_t)((x & 0x00FF) | (y & 0x00FF) << 8);
  out[1] = attr;
  return 1;
}
