// SPDX-License-Identifier: GPL-3.0-or-later
#include "decompress.h"

#include "platform.h"
#include "tg3000.h"

typedef struct {
  uint32_t in, out;  // bank << 16 of the input and output
  uint16_t x, y;     // input and output offsets
} Stream;

static uint8_t in8(const Stream* s, uint16_t at) { return bus_rd8(s->in | at); }
static uint16_t in16(const Stream* s, uint16_t at) { return (uint16_t)(in8(s, at) | (in8(s, (uint16_t)(at + 1)) << 8)); }
// The output is read as words through DB = its bank and an index (LDA $0000,X): the 65816
// adds the index in 24 bits, so a word read at offset $FFFF takes its high byte from the
// next bank's $0000 (bank $7F + 1 = $80: low WRAM). A copy from before the output's start
// does this (title_scene_load's pictures). Writes stay in the bank: no output the game
// unpacks runs across $FFFF, and that case is not modelled.
static uint8_t out8(const Stream* s, uint16_t at) { return bus_rd8(s->out | at); }
static uint16_t out16(const Stream* s, uint16_t at) {
  return (uint16_t)(out8(s, at) | (bus_rd8((s->out | at) + 1) << 8));
}
static void put8(const Stream* s, uint16_t at, uint8_t v) { bus_wr8(s->out | at, v); }
static void put16(const Stream* s, uint16_t at, uint16_t v) {
  put8(s, at, (uint8_t)v);
  put8(s, (uint16_t)(at + 1), (uint8_t)(v >> 8));
}

// A word of the output read as LDA disp,X with X = `at` - disp: the 24-bit sum crosses into
// the next bank (low WRAM) when `at` wrapped past $FFFF but X did not.
static uint16_t out16_disp(const Stream* s, uint16_t at, uint16_t disp) {
  const uint32_t a = (s->out | (uint16_t)(at - disp)) + disp;
  return (uint16_t)(bus_rd8(a) | (bus_rd8(a + 1) << 8));
}

// Copy `len` (4-260) bytes inside the output from `from` to the current position, moving
// words: at offsets 0, 2, 4, ... and, for an odd length, the last two words overlap by a
// byte (... L-5, L-3, L-2). This is the order the original's unrolled code uses. The short
// copies ($C0-$DF, `disp`) read their last word as LDA $0002,X (an odd length: the last two
// as $0002,X and $0003,X), X stepping by INX up to there; the long one's loop steps X to
// every word.
static void copy_back_words(Stream* s, uint16_t from, uint16_t len, bool disp) {
  const uint16_t body = (uint16_t)(len - (len & 1 ? 3 : 2));
  uint16_t i = 0;
  for(; i < body; i += 2) put16(s, (uint16_t)(s->y + i), out16(s, (uint16_t)(from + i)));
  if(len & 1) {
    put16(s, (uint16_t)(s->y + len - 3), disp ? out16_disp(s, (uint16_t)(from + len - 3), 2) : out16(s, (uint16_t)(from + len - 3)));
    put16(s, (uint16_t)(s->y + len - 2), disp ? out16_disp(s, (uint16_t)(from + len - 2), 3) : out16(s, (uint16_t)(from + len - 2)));
  } else {
    put16(s, (uint16_t)(s->y + len - 2), disp ? out16_disp(s, (uint16_t)(from + len - 2), 2) : out16(s, (uint16_t)(from + len - 2)));
  }
  s->y = (uint16_t)(s->y + len);
}
static void copy_back(Stream* s, uint16_t from, uint16_t len) { copy_back_words(s, from, len, false); }

// The long copy ($E0-$FF) orders an odd length differently: 5 bytes are words at 0 and 2
// plus a byte at 4; longer odd lengths are a word at 0 advancing one byte, then words at
// 1, 3, 5, ... L-2. Even lengths are words at 0, 2, ... L-2 as above.
static void copy_back_long(Stream* s, uint16_t from, uint16_t len) {
  if(!(len & 1)) {
    copy_back(s, from, len);
    return;
  }
  if(len == 5) {
    put16(s, s->y, out16(s, from));
    put16(s, (uint16_t)(s->y + 2), out16(s, (uint16_t)(from + 2)));
    put8(s, (uint16_t)(s->y + 4), out8(s, (uint16_t)(from + 4)));
  } else {
    put16(s, s->y, out16(s, from));
    for(uint16_t i = 1; i + 1 < len; i += 2) put16(s, (uint16_t)(s->y + i), out16(s, (uint16_t)(from + i)));
  }
  s->y = (uint16_t)(s->y + len);
}

// TG3000_ROUTINE($82:FB70) decompress
uint16_t decompress(uint16_t dp, uint8_t src_bank, uint16_t src, uint8_t dst_bank, uint16_t dst, uint16_t* src_end) {
  Stream s = {(uint32_t)src_bank << 16, (uint32_t)dst_bank << 16, src, dst};
  wr8(dp + 0x02, dst_bank);
  wr16(dp + 0x00, 0);
  for(;;) {
    wr16(dp + 0x04, s.x);
    uint8_t cmd = in8(&s, s.x);
    if(cmd < 0x40) {  // literal bytes
      s.x++;
      if(cmd == 0) break;
      uint16_t n = (uint16_t)(cmd >> 1);
      wr16(dp + 0x08, n);
      if(cmd & 1) {
        if(n == 0) {
          put8(&s, s.y, in8(&s, s.x));
          s.x++, s.y++;
          continue;
        }
        put16(&s, s.y, in16(&s, s.x));  // a word, advancing one byte
        s.x++, s.y++;
      }
      for(; n; n--) {
        put16(&s, s.y, in16(&s, s.x));
        s.x += 2, s.y += 2;
        wr16(dp + 0x08, (uint16_t)(n - 1));
      }
    } else if(cmd < 0x50) {  // byte progression
      uint16_t n = (uint16_t)((cmd & 0x0F) + 1);
      s.x++;
      uint8_t last = out8(&s, (uint16_t)(s.y - 1));
      uint8_t step = (uint8_t)(last - out8(&s, (uint16_t)(s.y - 2)));
      wr8(dp + 0x0A, step);
      for(uint16_t i = 0; i < n + 2; i++) {
        last = (uint8_t)(last + step);
        put8(&s, s.y++, last);
      }
    } else if(cmd < 0x60) {  // word progression
      uint16_t n = (uint16_t)((cmd & 0x0F) + 1);
      s.x++;
      uint16_t last = out16(&s, (uint16_t)(s.y - 2));
      uint16_t step = (uint16_t)(last - out16(&s, (uint16_t)(s.y - 4)));
      wr16(dp + 0x0A, step);
      for(uint16_t i = 0; i < n + 1; i++) {  // one fewer than the byte version
        last = (uint16_t)(last + step);
        put16(&s, s.y, last);
        s.y += 2;
      }
    } else if(cmd < 0x70) {  // repeat the last byte
      s.x++;
      uint8_t b = out8(&s, (uint16_t)(s.y - 1));
      uint16_t count = (uint16_t)((cmd & 0x0F) + 3);
      for(; count >= 2; count -= 2) {
        put16(&s, s.y, (uint16_t)(b | (b << 8)));
        s.y += 2;
      }
      if(count) put8(&s, s.y++, b);
    } else if(cmd < 0x80) {  // repeat the last word
      s.x++;
      uint16_t w = out16(&s, (uint16_t)(s.y - 2));
      for(uint16_t i = 0; i < (uint16_t)((cmd & 0x0F) + 2); i++) {
        put16(&s, s.y, w);
        s.y += 2;
      }
    } else if(cmd < 0xC0) {  // 3 bytes from close behind
      s.x++;
      wr16(dp + 0x0A, s.y);
      uint16_t back = (uint16_t)(s.y - ((cmd & 0x3F) + 2));
      uint16_t hi = out16(&s, back);
      uint16_t lo = out16(&s, (uint16_t)(back - 1));
      put16(&s, s.y, lo);
      put16(&s, (uint16_t)(s.y + 1), hi);
      s.y = (uint16_t)(s.y + 3);
    } else if(cmd < 0xE0) {  // 4-11 bytes from up to ~1 KB behind
      uint16_t from = (uint16_t)(s.y - ((((cmd & 3) << 8) | in8(&s, (uint16_t)(s.x + 1))) + 3));
      wr16(dp + 0x0A, from);
      s.x += 2;
      wr16(dp + 0x04, s.x);
      copy_back_words(&s, from, (uint16_t)(4 + ((cmd >> 2) & 7)), true);
    } else {  // 5-260 bytes from up to ~8 KB behind
      uint16_t from = (uint16_t)(s.y - ((((cmd & 0x1F) << 8) | in8(&s, (uint16_t)(s.x + 1))) + 3));
      wr16(dp + 0x0A, from);
      uint16_t len = (uint16_t)(in8(&s, (uint16_t)(s.x + 2)) + 1);
      s.x += 3;
      wr16(dp + 0x04, s.x);
      wr16(dp + 0x08, 0);
      copy_back_long(&s, from, (uint16_t)(len + 4));
    }
  }
  wr16(dp + 0x0A, s.y);
  *src_end = s.x;
  return (uint16_t)(s.y - dst);
}

// TG3000_ROUTINE($80:FBA0) decompress_near
uint16_t decompress_near(uint16_t dp, uint8_t src_bank, uint16_t src, uint8_t dst_bank, uint16_t dst, uint16_t* src_end) {
  return decompress(dp, src_bank, src, dst_bank, dst, src_end);
}

// TG3000_ROUTINE($80:B95D) queue_unpacked_upload
uint16_t queue_unpacked_upload(uint16_t dp, uint8_t src_bank, uint16_t src, uint16_t vram, uint16_t* src_end) {
  while(rd16(dp + 0x64) != 0) platform_wait_vblank();
  uint16_t len = decompress_near(dp, src_bank, src, 0x7F, rd16(dp + 0x5E), src_end);
  wr16(dp + 0x6A, vram);
  wr16(dp + 0x66, rd16(dp + 0x5E));
  wr16(dp + 0x68, 0x007F);
  wr16(dp + 0x64, len);
  return len;
}
