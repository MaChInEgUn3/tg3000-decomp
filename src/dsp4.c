// SPDX-License-Identifier: GPL-3.0-or-later
#include "dsp4.h"
#include <stdbool.h>
#include <stddef.h>

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

// (dsp4-03c: whether the entry is shown, measured on the firmware for a single entry on a
// fresh list: by y alone. Below 0 only -7..-1; from 0 on, rows 0-232 and 234 of every 512.)
static bool dsp4_sprite_row_shown(int16_t y) {
  if(y < 0) return y >= -7;
  const int row = y & 0x01FF;
  return row <= 232 || row == 234;
}

uint16_t dsp4_sprite(uint16_t x, uint16_t y, uint16_t attr, uint16_t out[2]) {
  if(!dsp4_sprite_row_shown((int16_t)y)) return 0;
  out[0] = (uint16_t)((x & 0x00FF) | (y & 0x00FF) << 8);
  out[1] = attr;
  return 1;
}

// ---- the road, $0001 ----
static int32_t s16x(uint16_t v) { return (int16_t)v; }
static int32_t floor_shift(int64_t v, int k) { return (int32_t)(v >= 0 ? v >> k : -((-v + (1LL << k) - 1) >> k)); }

void dsp4_road_begin(Dsp4Road* r, const uint16_t head[15], int16_t edge2, int16_t edge3) {
  for(int i = 0; i < 15; i++) r->head[i] = head[i];
  r->x = (int32_t)((uint32_t)s16x(head[7]) << 16 | head[6]);
  r->g = (int32_t)((uint32_t)s16x(head[14]) << 16 | head[13]);
  r->e = (int32_t)((uint32_t)s16x(head[1]) << 16);
  r->s = (int32_t)((uint32_t)s16x(head[12]) << 16 | head[11]);
  r->m = (int16_t)head[1];
  r->yPrev = (int16_t)head[1];
  r->endA = s16x(head[4]) - s16x(head[5]);
  r->endB = s16x(head[8]) + s16x(head[7]);
  r->round[0] = (uint16_t)(((uint16_t)edge2 >> 2) & 0x3FFF);
  r->round[1] = (uint16_t)(((uint16_t)edge3 >> 2) & 0x3FFF);
  r->index = head[9];
  r->row = 0;
}

unsigned dsp4_road_row(Dsp4Road* r, uint16_t h, const uint16_t first[3], const uint16_t track[3], uint16_t out[4],
                       uint16_t (*lines)[3], unsigned max) {
  if(r->row == 0 && first != NULL) {  // the first row's words: the sideways place's fine part, the height's
    r->x += (int32_t)((uint32_t)s16x(first[2]) << 16 | first[1]);
    r->e += (int32_t)((uint32_t)s16x(first[0]) << 8);
  }
  const int32_t xi = floor_shift(r->x, 16), ei = floor_shift(r->e, 16);
  const int32_t p = floor_shift((int64_t)xi * h, 15), y = floor_shift((int64_t)ei * h, 15);
  out[0] = (uint16_t)xi;
  out[1] = (uint16_t)p;
  out[2] = (uint16_t)ei;
  out[3] = (uint16_t)y;
  const int32_t c = s16x(r->head[4]) - s16x(r->head[5]);
  const int32_t endA = c + floor_shift((int64_t)s16x(r->head[10]) * h, 15) - s16x(r->head[10]) + (s16x(r->head[1]) - y);
  const int32_t endB = s16x(r->head[8]) + p;
  const int32_t cnt = r->m - y > 0 ? r->m - y : 0;
  if(cnt > 0) {
    const int32_t n = r->yPrev - y;
    const int32_t rec = 16384 / n;
    const int32_t j0 = r->yPrev - (r->m < r->yPrev ? r->m : r->yPrev);
    const int32_t dA = endA - r->endA - n, dB = endB - r->endB;
    for(int32_t k = 0; k < cnt; k++) {
      const int32_t j = j0 + k;
      const int32_t a = r->endA + j + floor_shift((int64_t)j * dA * rec + r->round[0], 14) + s16x(track[2]);
      int32_t b = r->endB + floor_shift((int64_t)j * dB * rec + r->round[1], 14);
      if(b < -128) b = -128;
      if(b > 896) b = 896;
      if((unsigned)k < max) {
        lines[k][0] = r->index;
        lines[k][1] = (uint16_t)a;
        lines[k][2] = (uint16_t)b;
      }
      r->index = (uint16_t)(r->index - 4);
    }
  }
  r->endA = endA;
  r->endB = endB;
  r->yPrev = (int16_t)y;
  if(y < r->m) r->m = (int16_t)y;
  if(cnt > 0) r->round[0] = r->round[1] = 0x2000;  // the last $0A's rounding: the first row that draws
  r->g += (int32_t)((uint32_t)s16x(track[1]) << 8);
  r->s += (int32_t)((uint32_t)s16x(track[0]) << 8);
  r->x += r->g;
  r->e += r->s;
  r->row++;
  return (unsigned)cnt;
}
