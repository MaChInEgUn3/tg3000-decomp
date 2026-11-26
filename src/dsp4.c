// SPDX-License-Identifier: GPL-3.0-or-later
#include "dsp4.h"
#include <stdbool.h>
#include <string.h>

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

// The commands the chip has: parameter bytes in, answer bytes out; flagEnds: a first answer
// word of 0 is the whole answer (a hidden sprite).
static const struct { uint8_t cmd, in, out; bool flagEnds; } dsp4_commands[] = {
  {0x00, 4, 4, false},   // dsp4_multiply
  {0x0A, 6, 8, false},   // dsp4_edges
  {0x11, 8, 2, false},   // dsp4_row
  {0x03, 10, 6, true},   // a sprite list's first entry: a word, then $000B and the entry
  {0x0B, 6, 6, true},    // dsp4_sprite
};

static int dsp4_command_index(uint8_t cmd) {
  for(int i = 0; i < (int)(sizeof dsp4_commands / sizeof dsp4_commands[0]); i++) if(dsp4_commands[i].cmd == cmd) return i;
  return -1;
}

static uint16_t dsp4_word(const uint8_t* p) { return (uint16_t)(p[0] | p[1] << 8); }

static void dsp4_put(uint8_t* p, uint16_t w) {
  p[0] = (uint8_t)w;
  p[1] = (uint8_t)(w >> 8);
}

static void dsp4_answer(Dsp4Chip* c) {
  memset(c->out, 0, sizeof c->out);
  switch(c->cmd) {
  case 0x00: {
    const uint32_t p = (uint32_t)dsp4_multiply((int16_t)dsp4_word(c->in), (int16_t)dsp4_word(c->in + 2));
    dsp4_put(c->out, (uint16_t)p);
    dsp4_put(c->out + 2, (uint16_t)(p >> 16));
    break;
  }
  case 0x0A: {
    int16_t e[4];
    c->roadBit = (dsp4_word(c->in + 2) & 0x80) != 0;
    dsp4_edges(dsp4_word(c->in), dsp4_word(c->in + 2), dsp4_word(c->in + 4), e);
    for(int j = 0; j < 4; j++) dsp4_put(c->out + 2 * j, (uint16_t)e[j]);
    break;
  }
  case 0x11: {
    int16_t e[4];
    for(int j = 0; j < 4; j++) e[j] = (int16_t)dsp4_word(c->in + 2 * j);
    dsp4_put(c->out, dsp4_row(e));
    break;
  }
  default: {  // $03 (its entry after two words) and $0B
    const uint8_t* e = c->in + (c->cmd == 0x03 ? 4 : 0);
    uint16_t w[2] = {0, 0};
    dsp4_put(c->out, dsp4_sprite(dsp4_word(e), dsp4_word(e + 2), dsp4_word(e + 4), w));
    dsp4_put(c->out + 2, w[0]);
    dsp4_put(c->out + 4, w[1]);
    break;
  }
  }
}

void dsp4_chip_reset(Dsp4Chip* c) { memset(c, 0, sizeof *c); }

bool dsp4_chip_lost(const Dsp4Chip* c) { return c->phase == 4; }

void dsp4_chip_write(Dsp4Chip* c, uint8_t v) {
  switch(c->phase) {
  case 0:
    c->cmd = v;
    c->phase = 1;
    return;
  case 1: {
    const int i = dsp4_command_index(c->cmd);
    if(v != 0 || i < 0) {
      c->phase = 4;
      return;
    }
    c->phase = 2;
    c->count = 0;
    c->inSize = dsp4_commands[i].in;
    c->outSize = dsp4_commands[i].out;
    c->flagEnds = dsp4_commands[i].flagEnds;
    return;
  }
  case 2:
    c->in[c->count++] = v;
    if(c->count == c->inSize) {
      dsp4_answer(c);
      c->phase = 3;
      c->count = 0;
    }
    return;
  case 3:
    if(c->cmd != 0x00) {  // only the multiply's answer may be skipped
      c->phase = 4;
      return;
    }
    (void)dsp4_chip_read(c);
    return;
  default:
    return;
  }
}

uint8_t dsp4_chip_read(Dsp4Chip* c) {
  if(c->phase == 0) return 0xFF;  // idle: the game's start-up waits for $FFFF ($80:8088)
  if(c->phase != 3) {
    c->phase = 4;
    return 0;
  }
  const uint8_t v = c->out[c->count++];
  const bool hidden = c->flagEnds && c->count == 2 && dsp4_word(c->out) == 0;
  if(c->count == c->outSize || hidden) {
    c->phase = 0;
    c->count = 0;
  }
  return v;
}
