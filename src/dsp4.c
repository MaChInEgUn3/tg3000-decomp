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
