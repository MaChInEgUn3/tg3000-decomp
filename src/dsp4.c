// SPDX-License-Identifier: GPL-3.0-or-later
#include "dsp4.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

int32_t dsp4_multiply(int16_t a, int16_t b) { return (int32_t)a * b; }

// The width in nibble j (0: bits 15-12 ... 3: bits 3-0), signed, times 48.
static int16_t dsp4_width(uint16_t row, int j) {
  int n = (row >> (12 - 4 * j)) & 0xF;
  if(n >= 8) n -= 16;
  return (int16_t)(n * 48);
}

// the road's rounding, as the chip keeps it (dsp4-01r): the last $0A's edges 2 and 3
// dsp4-01u: at power-on the rounding is 0 (300 of 300 fresh firmware chips; a road leaves $2000)
static int16_t dsp4_round[2] = {0, 0};
void dsp4_round_get(int16_t* edge2, int16_t* edge3) {
  *edge2 = dsp4_round[0];
  *edge3 = dsp4_round[1];
}
void dsp4_round_set(int16_t edge2, int16_t edge3) {
  dsp4_round[0] = edge2;
  dsp4_round[1] = edge3;
}

void dsp4_edges(uint16_t t, uint16_t near, uint16_t far, int16_t out[4]) {
  for(int j = 0; j < 4; j++) {
    int32_t a = dsp4_width(near, j), b = dsp4_width(far, j);
    int32_t p = (b - a) * (int32_t)t;
    int32_t step = p >= 0 ? p / 32768 : -((-p + 32767) / 32768);  // floor(p / 2^15)
    out[j] = (int16_t)(a + step);
  }
  dsp4_round_set(out[2], out[3]);
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
  r->m = (int16_t)head[2];      // lines are drawn below word 2, the line limit
  r->yPrev = (int16_t)head[1];  // the camera's line, where the first segment starts
  r->endA = s16x(head[4]) - s16x(head[5]) - (s16x(head[1]) - s16x(head[2]));
  r->endB = s16x(head[8]) + s16x(head[7]);
  r->round[0] = (uint16_t)(((uint16_t)edge2 >> 2) & 0x3FFF);
  r->round[1] = (uint16_t)(((uint16_t)edge3 >> 2) & 0x3FFF);
  r->index = head[9];
  r->row = 0;
  r->fromState = false;
}

void dsp4_road_start(Dsp4Road* r, const uint16_t head[15]) {
  dsp4_road_begin(r, head, dsp4_round[0], dsp4_round[1]);
  r->fromState = true;
}

// a row's lines, from the previous row's ends to this one's (screen x p, line y), shared by the
// roads $01 and $07 (dsp4-01n/o, dsp4-07a); `add` goes on every $0798 word
static unsigned dsp4_road_lines(Dsp4Road* r, uint16_t h, int32_t p, int32_t y, uint16_t add, uint16_t (*lines)[3],
                                unsigned max) {
  const int32_t c = s16x(r->head[4]) - s16x(r->head[5]);
  const int32_t endA = c + floor_shift((int64_t)s16x(r->head[10]) * h, 15) - s16x(r->head[10]) + (s16x(r->head[2]) - y);
  const int32_t endB = s16x(r->head[8]) + p;
  const int32_t cnt = r->m - y > 0 ? r->m - y : 0;
  if(cnt > 0) {
    const int32_t n = r->yPrev - y;
    const int32_t rec = 16384 / n;
    const int32_t j0 = r->yPrev - (r->m < r->yPrev ? r->m : r->yPrev);
    const int32_t dA = endA - r->endA - n, dB = endB - r->endB;
    for(int32_t k = 0; k < cnt; k++) {
      const int32_t j = j0 + k;
      const int32_t a = r->endA + j + floor_shift((int64_t)j * dA * rec + r->round[0], 14) + s16x(add);
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
  if(cnt > 0) {  // the last $0A's rounding: the first row that draws uses it up
    r->round[0] = r->round[1] = 0x2000;
    if(r->fromState) dsp4_round_set((int16_t)0x8000, (int16_t)0x8000);
    r->fromState = false;
  }
  r->row++;
  return (unsigned)cnt;
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
  const unsigned cnt = dsp4_road_lines(r, h, p, y, track[2], lines, max);
  r->g += (int32_t)((uint32_t)s16x(track[1]) << 8);
  r->s += (int32_t)((uint32_t)s16x(track[0]) << 8);
  r->x += r->g;
  r->e += r->s;
  return cnt;
}

void dsp4_road2_start(Dsp4Road* r, const uint16_t head[11]) {
  uint16_t h15[15] = {0};
  for(int i = 0; i < 11; i++) h15[i] = head[i];
  dsp4_road_start(r, h15);
}

unsigned dsp4_road2_row(Dsp4Road* r, const uint16_t words[6], uint16_t out[2], uint16_t (*lines)[3], unsigned max) {
  const uint16_t h = words[0];
  const int32_t y = s16x(words[1]) + floor_shift((int64_t)s16x(words[2]) * h, 15);
  const int32_t p = s16x(words[3]) + floor_shift((int64_t)s16x(words[4]) * h, 15);
  out[0] = (uint16_t)p;
  out[1] = (uint16_t)y;
  return dsp4_road_lines(r, h, p, y, words[5], lines, max);
}

// ---- the road's windows, $0008 ----
// dsp4-08h: the chip clamps a 16-bit value: a window edge past 16 bits wraps first (measured on
// the firmware: a player far off the road, x near -16,700, on the 60k movie)
static int32_t dsp4_clamp_byte(int32_t v) {
  const int32_t w = (int16_t)(uint16_t)v;
  return w < 0 ? 0 : w > 0xFF ? 0xFF : w;
}

uint16_t dsp4_window_begin(Dsp4Window* w, const uint16_t head[45]) {
  w->x = s16x(head[39]);
  w->base = 0x80 - w->x - 1;
  for(int k = 0; k < 2; k++) {
    const int32_t right = s16x(head[41 + 2 * k]), left = s16x(head[42 + 2 * k]);  // the camera row's edge words
    w->prevL[k] = w->base + floor_shift((int64_t)left * 0x7FFF, 15);
    w->prevR[k] = w->base + 2 + floor_shift((int64_t)right * 0x7FFF, 15);
    w->yPrev[k] = (int16_t)head[38 + 2 * k];
    w->m[k] = (int16_t)head[24 + 2 * k];
    w->addr[k] = head[20 + 2 * k];
  }
  return (uint16_t)(dsp4_clamp_byte(w->prevL[0]) << 8 | dsp4_clamp_byte(w->prevR[0]));
}

void dsp4_window_row(Dsp4Window* w, const uint16_t words[9], uint16_t (*runs[2])[2], unsigned n[2], unsigned max) {
  const uint16_t h = words[0];
  const int64_t side = (int64_t)(w->x - s16x(words[3])) * 32768;
  for(int k = 0; k < 2; k++) {
    const int32_t y = s16x(words[2 + 2 * k]);
    const int32_t endL = w->base + floor_shift(side + (int64_t)s16x(words[6 + 2 * k]) * h, 15);
    const int32_t endR = w->base + 2 + floor_shift(side + (int64_t)s16x(words[5 + 2 * k]) * h, 15);
    const int32_t cnt = w->m[k] - y > 0 ? w->m[k] - y : 0;
    n[k] = 0;
    if(cnt > 0) {
      const int32_t span = w->yPrev[k] - y;
      const int32_t rec = 16384 / span;
      const int32_t j0 = w->yPrev[k] - (w->m[k] < w->yPrev[k] ? w->m[k] : w->yPrev[k]);
      for(int32_t c = 0; c < cnt; c++) {
        const int32_t j = j0 + c + 1;
        const int32_t left = w->prevL[k] + floor_shift((int64_t)j * (endL - w->prevL[k]) * rec, 14);
        const int32_t right = w->prevR[k] + floor_shift((int64_t)j * (endR - w->prevR[k]) * rec, 14);
        if((unsigned)c < max) {
          runs[k][c][0] = w->addr[k];
          runs[k][c][1] = (uint16_t)(dsp4_clamp_byte(left) << 8 | dsp4_clamp_byte(right));
        }
        w->addr[k] = (uint16_t)(w->addr[k] - 4);
      }
      n[k] = (unsigned)cnt;
    }
    w->prevL[k] = endL;
    w->prevR[k] = endR;
    w->yPrev[k] = (int16_t)y;
    if(y < w->m[k]) w->m[k] = (int16_t)y;
  }
}

uint16_t dsp4_window_end(const Dsp4Window* w) {
  (void)w;
  return 0;
}

// ---- the sprite list (dsp4-09l) ----
static Dsp4Sprites dsp4_sp;

void dsp4_sprites_get(Dsp4Sprites* out) { *out = dsp4_sp; }
void dsp4_sprites_set(const Dsp4Sprites* in) { dsp4_sp = *in; }

void dsp4_list_open(void) {
  const bool sizes = dsp4_sp.sizes;  // ($0005 lasts past $0003)
  memset(&dsp4_sp, 0, sizeof dsp4_sp);
  dsp4_sp.sizes = sizes;
  // dsp4-01u: $0003 also sets the road's rounding to $848 (as a $0A's edges 2 and 3 of $2120 would;
  // measured on the firmware: chosen roads after a $0003, and race1's roads before any $0A)
  dsp4_round_set(0x2120, 0x2120);
}

void dsp4_list_sizes(void) { dsp4_sp.sizes = true; }

// an entry into the high table: its ninth x bit, its size bit (when $0005 was sent)
static void dsp4_list_high(int32_t x, bool large) {
  if(dsp4_sp.entries < 128) {
    const unsigned bits = (x < 0 || x > 0xFF ? 1u : 0u) | (large && dsp4_sp.sizes ? 2u : 0u);
    dsp4_sp.high[dsp4_sp.entries >> 2] |= (uint8_t)(bits << (2 * (dsp4_sp.entries & 3)));
  }
  dsp4_sp.entries++;
}

// an entry's lines walked from the top: false at the first one that would pass 33 slivers
// (the lines before stay counted, as on the chip)
static bool dsp4_list_lines(int32_t y, bool large) {
  const unsigned size = large ? 16 : 8, k = large ? 2 : 1;
  for(unsigned i = 0; i < size; i++) {
    uint8_t* line = &dsp4_sp.lines[(uint8_t)(y + (int32_t)i)];
    if(*line + k > 33) return false;
    *line = (uint8_t)(*line + k);
  }
  return true;
}

uint16_t dsp4_list_sprite(uint16_t x, uint16_t y, uint16_t attr, uint16_t out[2]) {
  const uint16_t flag = dsp4_sprite(x, y, attr, out);
  if(flag) {
    (void)dsp4_list_lines((int16_t)y, false);
    dsp4_list_high((int16_t)x, false);
  }
  return flag;
}

void dsp4_objects_begin(const uint16_t pre[7]) {
  for(int i = 0; i < 7; i++) dsp4_sp.pre[i] = (int16_t)pre[i];
  dsp4_sp.crestSet = false;
  dsp4_sp.kind = 0;
}

static void dsp4_objects_crest(uint16_t line) {
  const int16_t c = (int16_t)(dsp4_sp.pre[1] + (int16_t)line);
  if(!dsp4_sp.crestSet || c < dsp4_sp.crest) dsp4_sp.crest = c;
  dsp4_sp.crestSet = true;
}

void dsp4_objects_slot(uint16_t line) {
  dsp4_objects_crest(line);
  dsp4_sp.kind = 0;
}

void dsp4_objects_scenery(const uint16_t head[7]) {
  // the crest's line, h, the road's x, the object's line, its sideways place, its height, attribute
  dsp4_objects_crest(head[0]);
  const int32_t h = head[1];
  dsp4_sp.x0 = (int16_t)(dsp4_sp.pre[0] - s16x(head[2]) + floor_shift((int64_t)s16x(head[4]) * h, 15));
  dsp4_sp.y0 = (int16_t)(dsp4_sp.pre[1] + s16x(head[3]) + floor_shift((int64_t)s16x(head[5]) * h, 15));
  dsp4_sp.attr = head[6];
  dsp4_sp.kind = 1;
  dsp4_sp.list = 0;
}

static int32_t dsp4_byte_lerp(int32_t a, int32_t b, uint16_t d) { return b + floor_shift((int64_t)(a - b) * (d >> 8), 8); }

uint16_t dsp4_objects_car(const uint16_t head[9]) {
  // the crest's line, $9000, the distance, two rows' lines, two rows' x, h, the sideways place
  dsp4_objects_crest(head[0]);
  const uint16_t d = head[2];
  const int32_t h = head[7];
  const uint16_t answer = (uint16_t)(s16x(head[8]) - dsp4_byte_lerp(s16x(head[5]), s16x(head[6]), d));
  const int32_t line = dsp4_byte_lerp(s16x(head[3]), s16x(head[4]), d);
  dsp4_sp.x0 = (int16_t)(dsp4_sp.pre[0] + floor_shift((int64_t)(int16_t)answer * h, 15));
  dsp4_sp.y0 = (int16_t)(dsp4_sp.pre[1] + floor_shift((int64_t)line * h, 15));
  dsp4_sp.h = (uint16_t)h;
  dsp4_sp.attr = 0;
  dsp4_sp.kind = 2;
  dsp4_sp.list = 0;
  return answer;
}

void dsp4_objects_car_tail(uint16_t height, uint16_t attr) {
  dsp4_sp.y0 = (int16_t)(dsp4_sp.y0 + floor_shift((int64_t)s16x(height) * dsp4_sp.h, 15));
  dsp4_sp.attr = attr;
}

unsigned dsp4_objects_part(uint16_t w, uint16_t a, uint16_t b, uint16_t out[2][2]) {
  if(dsp4_sp.kind == 0) return 0;  // (parts with no object: not sent by the game)
  const bool large = dsp4_sp.list == 0;
  const int32_t size = large ? 16 : 8;
  const int32_t x = dsp4_sp.x0 + s16x(b), y = dsp4_sp.y0 + s16x(a);
  const int16_t* p = dsp4_sp.pre;
  // the screen's edges (the stream's words 3 to 6), then the crest above the part's top
  if(x + size <= p[3] || x > p[4] || y + size <= p[5] || y > p[6]) return 0;
  const int32_t c = dsp4_sp.crestSet ? dsp4_sp.crest : 0x7FFF;
  if(c < y) return 0;
  unsigned n = 0;
  if(c < y + size) {  // the crest cuts the part: a mask (tile $EE) on the crest's line first
    if(!dsp4_list_lines(c, large)) return 0;
    out[n][0] = (uint16_t)((x & 0xFF) | (c & 0xFF) << 8);
    out[n][1] = 0x00EE;
    dsp4_list_high(x, large);
    n++;
  }
  if(!dsp4_list_lines(y, large)) return n;  // (a mask already given stays)
  out[n][0] = (uint16_t)((x & 0xFF) | (y & 0xFF) << 8);
  out[n][1] = (uint16_t)(dsp4_sp.attr + w);
  dsp4_list_high(x, large);
  return n + 1;
}

void dsp4_objects_list_end(void) {
  if(dsp4_sp.list == 0) dsp4_sp.list = 1;
  else dsp4_sp.kind = 0;
}

void dsp4_objects_end(void) { dsp4_sp.kind = 0; }

void dsp4_high_table(uint16_t out[16]) {
  for(int i = 0; i < 16; i++) out[i] = (uint16_t)(dsp4_sp.high[2 * i] | dsp4_sp.high[2 * i + 1] << 8);
}
