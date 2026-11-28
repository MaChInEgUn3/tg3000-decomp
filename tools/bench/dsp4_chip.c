// SPDX-License-Identifier: GPL-3.0-or-later
#include "dsp4_chip.h"

#include <string.h>

#include "dsp4.h"

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

// The chip's rounding in place of src/dsp4.c's global for one native call; the global is put
// back after, so the chip and the code using the global do not disturb each other.
static void dsp4_round_enter(const Dsp4Chip* c, int16_t saved[2]) {
  dsp4_round_get(&saved[0], &saved[1]);
  dsp4_round_set(c->edge2, c->edge3);
}
static void dsp4_round_leave(Dsp4Chip* c, const int16_t saved[2]) {
  dsp4_round_get(&c->edge2, &c->edge3);
  dsp4_round_set(saved[0], saved[1]);
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
    int16_t e[4], saved[2];
    dsp4_round_enter(c, saved);
    dsp4_edges(dsp4_word(c->in), dsp4_word(c->in + 2), dsp4_word(c->in + 4), e);
    dsp4_round_leave(c, saved);  // the copy takes edges 2 and 3
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
    // (dsp4-01u: $0003 sets the road's rounding; dsp4-01v: the list's slivers then move the second
    // word, which this model, without the list, does not follow)
    if(c->cmd == 0x03) c->edge2 = c->edge3 = 0x2121;
    const uint8_t* e = c->in + (c->cmd == 0x03 ? 4 : 0);
    uint16_t w[2] = {0, 0};
    dsp4_put(c->out, dsp4_sprite(dsp4_word(e), dsp4_word(e + 2), dsp4_word(e + 4), w));
    dsp4_put(c->out + 2, w[0]);
    dsp4_put(c->out + 4, w[1]);
    break;
  }
  }
}

void dsp4_chip_reset(Dsp4Chip* c) {
  memset(c, 0, sizeof *c);
  c->edge2 = c->edge3 = 0;  // (dsp4-01u: the power-on rounding)
}

void dsp4_chip_abort(Dsp4Chip* c) {
  const int16_t e2 = c->edge2, e3 = c->edge3;
  const unsigned long u = c->unmodelled;
  static Dsp4Chip keep;  // (the list followed lasts across commands: dsp4-01w)
  keep.follow = c->follow;
  dsp4_chip_reset(c);
  c->edge2 = e2;
  c->edge3 = e3;
  c->unmodelled = u;
  c->follow = keep.follow;
}

// ---- dsp4-01w: the sprite list, followed from the writes: its words as the race's list sends
// them ($0005, $000B and 3 words, $0009 and its stream, multiplies, $0006 the end), on the chip's
// own list. Anything else ends the following. ----
enum { FL_OFF, FL_CMD, FL_ARGS, FL_OBJ0, FL_OBJ1, FL_HEAD, FL_TAIL, FL_PART };
static Dsp4Sprites flGlobal;
static int16_t flRound[2];
static void dsp4_follow_enter(Dsp4Chip* c) {
  dsp4_sprites_get(&flGlobal);
  dsp4_round_get(&flRound[0], &flRound[1]);
  dsp4_sprites_set(&c->follow.list);
}
static void dsp4_follow_leave(Dsp4Chip* c) {
  dsp4_sprites_get(&c->follow.list);
  dsp4_sprites_set(&flGlobal);
  dsp4_round_set(flRound[0], flRound[1]);
}
static void dsp4_follow_open(Dsp4Chip* c) {
  dsp4_follow_enter(c);
  dsp4_list_open();
  dsp4_follow_leave(c);
  c->follow.state = FL_CMD;
  c->follow.half = false;
}
static void dsp4_follow_word(Dsp4Chip* c, uint16_t w) {
  __typeof__(c->follow)* f = &c->follow;
  switch(f->state) {
  case FL_CMD:
    f->got = 0;
    f->kind = (uint8_t)w;
    if(w == 0x0003) dsp4_follow_open(c);
    else if(w == 0x0005) {
      dsp4_follow_enter(c);
      dsp4_list_sizes();
      dsp4_follow_leave(c);
    } else if(w == 0x000B || w == 0x0009 || w == 0x0000) {
      f->need = w == 0x000B ? 3 : w == 0x0009 ? 7 : 2;
      f->state = FL_ARGS;
    } else f->state = FL_OFF;  // $0006 (the list's end) or a command not the list's
    return;
  case FL_ARGS:
    f->args[f->got++] = w;
    if(f->got < f->need) return;
    f->state = FL_CMD;
    if(f->kind == 0x0B) {
      uint16_t out[2];
      dsp4_follow_enter(c);
      (void)dsp4_list_sprite(f->args[0], f->args[1], f->args[2], out);
      dsp4_follow_leave(c);
    } else if(f->kind == 0x09) {
      dsp4_follow_enter(c);
      dsp4_objects_begin(f->args);
      dsp4_follow_leave(c);
      f->state = FL_OBJ0;
    }
    return;
  case FL_OBJ0:
    f->args[0] = w;
    f->state = FL_OBJ1;
    return;
  case FL_OBJ1:
    f->args[1] = w;
    if(f->args[0] == 0 && w == 0x8000) {
      dsp4_follow_enter(c);
      dsp4_objects_end();
      const uint16_t left = dsp4_list_rounding();
      dsp4_follow_leave(c);
      c->edge3 = (int16_t)left;  // (the road reads lines 127/128's budget left)
      f->state = FL_CMD;
    } else if(w == 0) {
      dsp4_follow_enter(c);
      dsp4_objects_slot(f->args[0]);
      dsp4_follow_leave(c);
      f->state = FL_OBJ0;
    } else {
      f->kind = (w & 0x8000) ? 2 : 1;
      f->need = f->kind == 2 ? 9 : 7;
      f->got = 2;
      f->state = FL_HEAD;
    }
    return;
  case FL_HEAD:
    f->args[f->got++] = w;
    if(f->got < f->need) return;
    f->lists = 0;
    f->got = 0;
    dsp4_follow_enter(c);
    if(f->kind == 2) (void)dsp4_objects_car(f->args);
    else dsp4_objects_scenery(f->args);
    dsp4_follow_leave(c);
    f->state = f->kind == 2 ? FL_TAIL : FL_PART;
    return;
  case FL_TAIL:
    f->args[f->got++] = w;
    if(f->got < 2) return;
    dsp4_follow_enter(c);
    dsp4_objects_car_tail(f->args[0], f->args[1]);
    dsp4_follow_leave(c);
    f->got = 0;
    f->state = FL_PART;
    return;
  case FL_PART:
    if(f->got == 0 && w == 0) {
      dsp4_follow_enter(c);
      dsp4_objects_list_end();
      dsp4_follow_leave(c);
      if(++f->lists == 2) f->state = FL_OBJ0;
      return;
    }
    f->args[f->got++] = w;
    if(f->got < 3) return;
    {
      uint16_t out[2][2];
      dsp4_follow_enter(c);
      (void)dsp4_objects_part(f->args[0], f->args[1], f->args[2], out);
      dsp4_follow_leave(c);
    }
    f->got = 0;
    return;
  }
}
static void dsp4_follow_byte(Dsp4Chip* c, uint8_t v) {
  if(!c->follow.half) {
    c->follow.lo = v;
    c->follow.half = true;
    return;
  }
  c->follow.half = false;
  dsp4_follow_word(c, (uint16_t)(c->follow.lo | v << 8));
}

// ---- the streams: phase 5 takes the header, phase 6 a row's words (its first word $8000
// ends the command, $8001 is a road edge, not had), phase 7 gives an answer ----
static unsigned dsp4_stream_head(uint8_t cmd) { return cmd == 0x01 ? 30 : cmd == 0x07 ? 22 : 90; }
static unsigned dsp4_stream_row(const Dsp4Chip* c) { return c->cmd == 0x01 ? (c->roadFirst ? 14 : 8) : c->cmd == 0x07 ? 12 : 18; }

static uint16_t dsp4_lines[DSP4_CHIP_LINES][3];
static uint16_t dsp4_runs[2][DSP4_CHIP_LINES][2];

static void dsp4_stream_answered(Dsp4Chip* c, unsigned len, bool unmodelled) {
  c->streamLen = (uint16_t)len;
  c->phase = 7;
  c->count = 0;
  if(unmodelled) c->unmodelled++;
}

// a row's count word and its lines at p (the count says what is kept); returns the bytes
static unsigned dsp4_put_lines(uint8_t* p, unsigned n) {
  const unsigned k = n < DSP4_CHIP_LINES ? n : DSP4_CHIP_LINES;
  dsp4_put(p, (uint16_t)k);
  for(unsigned i = 0; i < k; i++)
    for(int j = 0; j < 3; j++) dsp4_put(p + 2 + 6 * i + 2 * j, dsp4_lines[i][j]);
  return 2 + 6 * k;
}

static void dsp4_stream_begin(Dsp4Chip* c) {
  uint16_t head[45];
  const unsigned n = dsp4_stream_head(c->cmd) / 2;
  for(unsigned i = 0; i < n; i++) head[i] = dsp4_word(c->streamIn + 2 * i);
  int16_t saved[2];
  uint16_t first = 0;
  dsp4_round_enter(c, saved);
  if(c->cmd == 0x01) dsp4_road_start(&c->road, head);
  else if(c->cmd == 0x07) dsp4_road2_start(&c->road, head);
  else first = dsp4_window_begin(&c->window, head);
  dsp4_round_leave(c, saved);
  // (dsp4.h) measured for the camera's line at or above the line limit; below it, a row can
  // divide by 0 in dsp4.c (its segment 0 with lines to draw)
  if(c->cmd == 0x08) c->outside = (int16_t)head[38] < (int16_t)head[24] || (int16_t)head[40] < (int16_t)head[26];
  else c->outside = (int16_t)head[1] < (int16_t)head[2];
  c->roadFirst = true;
  c->count = 0;
  if(c->cmd == 0x08) {
    dsp4_put(c->streamOut, first);
    dsp4_stream_answered(c, 2, false);
  } else c->phase = 6;
}

static void dsp4_road_answer(Dsp4Chip* c) {
  const uint8_t* w = c->streamIn;
  uint16_t first[3], track[3], out[4] = {0, 0, 0, 0};
  const uint8_t* tr = w + (c->roadFirst ? 8 : 2);
  for(int i = 0; i < 3; i++) {
    first[i] = dsp4_word(w + 2 + 2 * i);
    track[i] = dsp4_word(tr + 2 * i);
  }
  unsigned n = 0;
  bool unmodelled = c->outside;
  if(!c->outside) {
    const int prev = c->road.yPrev;  // the previous row's line (the camera's, for the first)
    int16_t saved[2];
    dsp4_round_enter(c, saved);
    n = dsp4_road_row(&c->road, dsp4_word(w), c->roadFirst ? first : NULL, track, out, dsp4_lines, DSP4_CHIP_LINES);
    dsp4_round_leave(c, saved);
    unmodelled = prev - c->road.yPrev >= 88 || n > DSP4_CHIP_LINES;
  }
  c->roadFirst = false;
  for(int i = 0; i < 4; i++) dsp4_put(c->streamOut + 2 * i, out[i]);
  dsp4_stream_answered(c, 8 + dsp4_put_lines(c->streamOut + 8, n), unmodelled);
}

static void dsp4_road2_answer(Dsp4Chip* c) {
  uint16_t words[6], out[2] = {0, 0};
  for(int i = 0; i < 6; i++) words[i] = dsp4_word(c->streamIn + 2 * i);
  unsigned n = 0;
  bool unmodelled = c->outside;
  if(!c->outside) {
    const int prev = c->road.yPrev;
    int16_t saved[2];
    dsp4_round_enter(c, saved);
    n = dsp4_road2_row(&c->road, words, out, dsp4_lines, DSP4_CHIP_LINES);
    dsp4_round_leave(c, saved);
    unmodelled = prev - c->road.yPrev >= 88 || n > DSP4_CHIP_LINES;
  }
  for(int i = 0; i < 2; i++) dsp4_put(c->streamOut + 2 * i, out[i]);
  dsp4_stream_answered(c, 4 + dsp4_put_lines(c->streamOut + 4, n), unmodelled);
}

static void dsp4_window_answer(Dsp4Chip* c) {
  uint16_t words[9];
  for(int i = 0; i < 9; i++) words[i] = dsp4_word(c->streamIn + 2 * i);
  unsigned n[2] = {0, 0};
  bool unmodelled = c->outside || words[2] != words[4];  // (dsp4.h) measured for equal lines only
  if(!c->outside) {
    uint16_t (*runs[2])[2] = {dsp4_runs[0], dsp4_runs[1]};
    int16_t saved[2];
    dsp4_round_enter(c, saved);  // (the windows do not use the rounding; kept alike)
    dsp4_window_row(&c->window, words, runs, n, DSP4_CHIP_LINES);
    dsp4_round_leave(c, saved);
  }
  unsigned at = 0;
  for(int k = 0; k < 2; k++) {
    if(n[k] > DSP4_CHIP_LINES) unmodelled = true;
    const unsigned kept = n[k] < DSP4_CHIP_LINES ? n[k] : DSP4_CHIP_LINES;
    dsp4_put(c->streamOut + at, (uint16_t)kept);
    at += 2;
    for(unsigned i = 0; i < kept; i++, at += 4) {
      dsp4_put(c->streamOut + at, dsp4_runs[k][i][0]);
      dsp4_put(c->streamOut + at + 2, dsp4_runs[k][i][1]);
    }
  }
  dsp4_stream_answered(c, at, unmodelled);
}

// a row's first word: $8000 ends the command ($08 then answers one word); $8001 in a road is an
// edge, which the native code does not have
static bool dsp4_stream_mark(Dsp4Chip* c) {
  const uint16_t w = dsp4_word(c->streamIn);
  if(w == 0x8000) {
    if(c->cmd == 0x08) {
      dsp4_put(c->streamOut, dsp4_window_end(&c->window));
      c->streamEnd = true;
      dsp4_stream_answered(c, 2, false);
    } else {
      c->phase = 0;
      c->count = 0;
    }
    return true;
  }
  if(w == 0x8001 && c->cmd != 0x08) {
    c->unmodelled++;
    c->phase = 4;
    return true;
  }
  return false;
}

bool dsp4_chip_lost(const Dsp4Chip* c) { return c->phase == 4; }
bool dsp4_chip_idle(const Dsp4Chip* c) { return c->phase == 0; }
unsigned long dsp4_chip_unmodelled(const Dsp4Chip* c) { return c->unmodelled; }

unsigned dsp4_chip_pending(const Dsp4Chip* c) {
  if(c->phase == 7) return (unsigned)(c->streamLen - c->count);
  if(c->phase != 3) return 0;
  if(c->flagEnds && c->count < 2 && dsp4_word(c->out) == 0) return 2u - c->count;
  return (unsigned)(c->outSize - c->count);
}

static void dsp4_chip_step(Dsp4Chip* c, uint8_t v);
void dsp4_chip_write(Dsp4Chip* c, uint8_t v) {
  // dsp4-01w: the list follows the writes; a $0003 command word (the chip's own framing: not a
  // 3 inside data) opens it
  const bool following = c->follow.state != FL_OFF;
  const uint8_t phase = c->phase;
  dsp4_chip_step(c, v);
  if(following) dsp4_follow_byte(c, v);
  else if(phase == 1 && c->phase == 2 && c->cmd == 0x03) dsp4_follow_open(c);
}

// A command starting on the other chip: dropped to idle, its word's low byte (already written,
// so already followed) replayed to the framing alone, then its high byte written
void dsp4_chip_start(Dsp4Chip* c, uint8_t lo, uint8_t hi) {
  dsp4_chip_abort(c);
  dsp4_chip_step(c, lo);
  dsp4_chip_write(c, hi);
}

static void dsp4_chip_step(Dsp4Chip* c, uint8_t v) {
  switch(c->phase) {
  case 0:
    c->cmd = v;
    c->phase = 1;
    return;
  case 1: {
    if(v == 0 && (c->cmd == 0x01 || c->cmd == 0x07 || c->cmd == 0x08)) {  // a stream
      c->phase = 5;
      c->count = 0;
      c->streamEnd = false;
      return;
    }
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
  case 5:
    c->streamIn[c->count++] = v;
    if(c->count == dsp4_stream_head(c->cmd)) dsp4_stream_begin(c);
    return;
  case 6:
    c->streamIn[c->count++] = v;
    if(c->count == 2 && dsp4_stream_mark(c)) return;
    if(c->count == dsp4_stream_row(c)) {
      if(c->cmd == 0x01) dsp4_road_answer(c);
      else if(c->cmd == 0x07) dsp4_road2_answer(c);
      else dsp4_window_answer(c);
    }
    return;
  default:  // lost, or a write in a stream's answer
    c->phase = 4;
    return;
  }
}

uint8_t dsp4_chip_read(Dsp4Chip* c) {
  if(c->phase == 0) return 0xFF;  // idle: the game's start-up waits for $FFFF ($80:8088)
  if(c->phase == 7) {
    const uint8_t v = c->streamOut[c->count++];
    if(c->count == c->streamLen) {
      c->phase = c->streamEnd ? 0 : 6;
      c->count = 0;
    }
    return v;
  }
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
