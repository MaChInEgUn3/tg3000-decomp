// SPDX-License-Identifier: GPL-3.0-or-later
#include "scroll_tables.h"

#include "tg3000.h"

#define SCROLL_BAND (DP + 0x14)  // the line count left for the rest of the table
#define SCROLL_MARK (DP + 0x16)

typedef struct {
  uint16_t head;      // $00D0 / $00D2: where the table starts (built downwards from it)
  uint16_t extra;     // $0242 / $1EF2: a negative value adds a band at the top
  uint16_t index;     // $0164 / $0168
  uint32_t heights;   // $7E:4CFE / $7E:4D4A: a byte per index
  uint16_t base;      // $1E3E / $1E40
  uint16_t add_base;  // $0796 / $0E56
  uint16_t add_e0;    // $1536 / $1556
  uint16_t out;       // $1538 / $1558
} Half;

// one 3-byte entry at y: the count word, then the pointer word over its high byte
static uint16_t entry(uint16_t y, uint16_t count, uint16_t pointer) {
  wr16(y, count);
  wr16((uint16_t)(y + 1), pointer);
  return (uint16_t)(y - 3);
}

static uint16_t negative(uint16_t v) { return v & 0x8000; }

static uint16_t build_half(const Half* h) {
  uint16_t y = rd16(h->head);
  wr16(y, 0);  // the terminator
  y = (uint16_t)(y - 3);
  wr16(SCROLL_BAND, 0x00D8);
  uint16_t a = rd16(h->extra);
  if(negative(a)) {  // never in a recorded run: checked by the fuzz
    a = (uint16_t)-a;
    if(!negative((uint16_t)(a - 0xD8))) a = 0x00D8;
    if(a >= 0x80) {
      a = (uint16_t)(a - 0x7F);
      wr16(SCROLL_BAND, (uint16_t)(rd16(SCROLL_BAND) - a));
      y = entry(y, a, 0xA5D8);
      a = 0x007F;
    }
    wr16(SCROLL_BAND, (uint16_t)(rd16(SCROLL_BAND) - a));
    y = entry(y, a, 0xA5D8);
  }
  wr16(SCROLL_MARK, rd16(SCROLL_BAND));
  a = bus_rd8(h->heights + rd16(h->index));
  if(a == 0) a = 0x00DC;  // never in a recorded run
  a = (uint16_t)(a - 4);
  if(negative(a)) a = 0;  // never in a recorded run
  if(negative((uint16_t)(a - rd16(SCROLL_BAND)))) {
    wr16(SCROLL_BAND, a);
    uint16_t lines = (uint16_t)(rd16(SCROLL_MARK) - a);
    if(lines != 0) {
      uint16_t p = (uint16_t)((uint16_t)(0xD7 - a) << 2);
      y = entry(y, (uint16_t)(lines | 0x80), (uint16_t)(rd16(h->base) - p + h->add_base));
    }
  }
  a = (uint16_t)(rd16(0x0237) + 0xB0);
  if(negative(a)) a = 0;  // never in a recorded run
  wr16(SCROLL_MARK, a);
  if(negative((uint16_t)(a - rd16(SCROLL_BAND)))) {
    uint16_t lines = (uint16_t)-(uint16_t)(a - rd16(SCROLL_BAND));
    wr16(SCROLL_BAND, a);
    y = entry(y, (uint16_t)(lines | 0x80), (uint16_t)((uint16_t)(a << 2) + 0xA968));
  }
  a = (uint16_t)(rd16(0x0237) + 0x70);
  if(negative(a)) a = 0;  // never in a recorded run
  wr16(SCROLL_MARK, a);
  if(negative((uint16_t)(a - rd16(SCROLL_BAND)))) {
    uint16_t lines = (uint16_t)-(uint16_t)(a - rd16(SCROLL_BAND));
    y = entry(y, lines, (uint16_t)(rd16(0x00E0) + h->add_e0));
    wr16(SCROLL_BAND, a);
  }
  a = rd16(SCROLL_BAND);
  if(a != 0) {
    if(a >= 0x80) {
      y = entry(y, (uint16_t)((uint16_t)(a - 0x7F) | 0x80), 0xA7D8);
      a = 0x007F;
    }
    y = entry(y, (uint16_t)(a | 0x80), 0xA5DC);
  }
  wr16(y, 4);
  wr16(h->head, y);
  uint16_t v = (uint16_t)(0xBB - rd16(0x0237) - 0x70);
  wr16((uint16_t)(h->out + rd16(0x00E0)), v);
  return v;
}

static void rotate3(uint16_t a, uint16_t b, uint16_t c, uint16_t head) {
  // c <- b, b <- a, a <- old c; head = the new c
  uint16_t old_c = rd16(c);
  wr16(c, rd16(b));
  wr16(head, rd16(c));
  wr16(b, rd16(a));
  wr16(a, old_c);
}

// TG3000_ROUTINE($85:BC20) scroll_tables_build (long)
//
// Ten short stretches of it run in no recorded movie (a negative $0242/$1EF2, a zero
// height byte, bands below the top); the fuzz covers them.
uint16_t scroll_tables_build(void) {
  uint16_t e0 = rd16(0x00E0);
  wr16(0x00E0, rd16(0x00E4));
  wr16(0x00E4, e0);
  rotate3(0x1E2E, 0x1E32, 0x1E36, 0x00D0);
  rotate3(0x1E30, 0x1E34, 0x1E38, 0x00D2);
  static const Half halves[2] = {
    {0x00D0, 0x0242, 0x0164, 0x7E4CFE, 0x1E3E, 0x0796, 0x1536, 0x1538},
    {0x00D2, 0x1EF2, 0x0168, 0x7E4D4A, 0x1E40, 0x0E56, 0x1556, 0x1558},
  };
  build_half(&halves[0]);
  return build_half(&halves[1]);
}
