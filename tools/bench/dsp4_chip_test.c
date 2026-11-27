// SPDX-License-Identifier: GPL-3.0-or-later
// Dsp4Chip (tools/bench/dsp4_chip.c) against src/dsp4.c called directly: a $0A, a $01, a $0A,
// a $07 and a $08, byte by byte, then the unmodelled answers ($01 segment of 88+ lines, $08
// rows with two lines, $01's $8001). The global rounding is set to a sentinel first and must
// not change across any of the chip's calls.
//   cc -std=c99 -Wall -Wextra -Isrc -Itools/bench tools/bench/dsp4_chip_test.c tools/bench/dsp4_chip.c src/dsp4.c
#include <stdio.h>
#include <string.h>

#include "dsp4.h"
#include "dsp4_chip.h"

static const int16_t S2 = 0x1234, S3 = -0x2345;  // the sentinel in the global
static unsigned long checks, fails;

static void check(int ok, const char* what) {
  checks++;
  if(!ok) {
    fails++;
    printf("FAIL: %s\n", what);
  }
}
static void checkGlobal(void) {
  int16_t a, b;
  dsp4_round_get(&a, &b);
  check(a == S2 && b == S3, "the global rounding changed");
}

static Dsp4Chip chip;
static void put(uint16_t w) {
  dsp4_chip_write(&chip, (uint8_t)w);
  checkGlobal();
  dsp4_chip_write(&chip, (uint8_t)(w >> 8));
  checkGlobal();
  check(!dsp4_chip_lost(&chip), "the chip is lost");
}
static uint16_t get(void) {
  const uint8_t lo = dsp4_chip_read(&chip);
  checkGlobal();
  const uint8_t hi = dsp4_chip_read(&chip);
  checkGlobal();
  return (uint16_t)(lo | hi << 8);
}
static void expect(uint16_t want, const char* what) { check(get() == want, what); }

// the reference's rounding (the chip's, as dsp4.c's global would hold it), swapped in per call
static int16_t ref2 = (int16_t)0x8000, ref3 = (int16_t)0x8000;
static void refEnter(void) { dsp4_round_set(ref2, ref3); }
static void refLeave(void) {
  dsp4_round_get(&ref2, &ref3);
  dsp4_round_set(S2, S3);
}

static void testEdges(uint16_t t, uint16_t near, uint16_t far) {
  put(0x000A);
  put(t);
  put(near);
  put(far);
  int16_t e[4];
  refEnter();
  dsp4_edges(t, near, far, e);
  refLeave();
  for(int j = 0; j < 4; j++) expect((uint16_t)e[j], "$0A edge");
  check(dsp4_chip_idle(&chip), "$0A: idle after");
  check(chip.edge2 == e[2] && chip.edge3 == e[3], "$0A: the chip's rounding is edges 2 and 3");
}

static uint16_t lines[DSP4_CHIP_LINES][3];

// a road's rows: heights h, the first row's words, track words per row
static unsigned long testRoad(const uint16_t head[15], const uint16_t* h, int rows, const uint16_t firstW[3],
                              const uint16_t (*track)[3], int expectDraw) {
  unsigned long total = 0;
  unsigned maxBytes = 0;
  put(0x0001);
  for(int i = 0; i < 15; i++) put(head[i]);
  // reference: dsp4_road_begin with the chip's rounding given outright (not via the global)
  Dsp4Road r;
  dsp4_road_begin(&r, head, chip.edge2, chip.edge3);
  int drew = 0;
  for(int row = 0; row < rows; row++) {
    put(h[row]);
    if(row == 0)
      for(int i = 0; i < 3; i++) put(firstW[i]);
    for(int i = 0; i < 3; i++) put(track[row][i]);
    uint16_t out[4];
    refEnter();
    const unsigned n = dsp4_road_row(&r, h[row], row == 0 ? firstW : NULL, track[row], out, lines, DSP4_CHIP_LINES);
    refLeave();
    check(dsp4_chip_pending(&chip) == 10 + 6 * n, "$01: the answer's length");
    if(10 + 6 * n > maxBytes) maxBytes = 10 + 6 * n;
    for(int i = 0; i < 4; i++) expect(out[i], "$01 row word");
    expect((uint16_t)n, "$01 count");
    for(unsigned k = 0; k < n; k++)
      for(int j = 0; j < 3; j++) expect(lines[k][j], "$01 line word");
    if(n && !drew) {
      drew = 1;
      check(chip.edge2 == (int16_t)0x8000 && chip.edge3 == (int16_t)0x8000, "$01: the first row that draws leaves $8000");
    }
    total += n;
  }
  check(drew == expectDraw, "$01 drew");
  put(0x8000);
  check(dsp4_chip_idle(&chip), "$01: idle after $8000");
  printf("$01: %d rows, %lu lines, longest answer %u bytes\n", rows, total, maxBytes);
  return total;
}

int main(void) {
  dsp4_chip_reset(&chip);
  dsp4_round_set(S2, S3);
  check(chip.edge2 == (int16_t)0x8000 && chip.edge3 == (int16_t)0x8000, "power-on rounding");

  // $0A: two different rows (widths 0 and 2 negative, 1 and 3 positive)
  testEdges(0x2A80, 0xE3D2, 0xC5A7);
  printf("$0A: edges ok, rounding %04X %04X\n", (uint16_t)chip.edge2, (uint16_t)chip.edge3);

  // $01: as road_rows_project sends it (camera line $33 + $50, limit $33); one segment of 46
  // lines (an answer over 255 bytes)
  const uint16_t head[15] = {0, 0x0083, 0x0033, 0xFF5B, 0x00BF, 0x00DC, 0x8000, 0x0002, 0x0180, 0x035C,
                             0x00C0, 0x4000, 0xFFFF, 0x2000, 0x0000};
  const uint16_t h[] = {0x7F00, 0x3400, 0x0800, 0x0600, 0x0400, 0x0200, 0x0100, 0x0080};
  const uint16_t firstW[3] = {0x0120, 0x4000, 0x0001};
  const uint16_t track[8][3] = {{0x0010, 0x0020, 0x0004}, {0xFFF0, 0x0030, 0xFFFC}, {0x0008, 0xFFE0, 0x0000},
                                {0x0000, 0x0010, 0x0002}, {0x0004, 0x0000, 0x0000}, {0xFFFC, 0x0008, 0x0001},
                                {0x0000, 0x0000, 0x0000}, {0x0002, 0xFFF8, 0x0000}};
  testRoad(head, h, 8, firstW, track, 1);
  check(dsp4_chip_unmodelled(&chip) == 0, "$01: all modelled");

  // a $0A again, then $07 (rows: h, line and offset, x and offset, a word added)
  testEdges(0x5D00, 0xF1E3, 0xD2B4);
  {
    const uint16_t head7[11] = {0, 0x0060, 0x0033, 0xFF5B, 0x00BF, 0x00DC, 0x4000, 0x0010, 0x0180, 0x050C, 0x00C0};
    const uint16_t rows7[5][6] = {{0x7F00, 0x0048, 0x0000, 0x0090, 0x0000, 0x0002},
                                  {0x6000, 0x0030, 0x0010, 0x0088, 0xFFF0, 0x0000},
                                  {0x5000, 0x0024, 0xFFF8, 0x0080, 0x0020, 0xFFFE},
                                  {0x3000, 0x0018, 0x0000, 0x0070, 0x0000, 0x0001},
                                  {0x2000, 0x0008, 0x0004, 0x0060, 0x0008, 0x0000}};
    put(0x0007);
    for(int i = 0; i < 11; i++) put(head7[i]);
    Dsp4Road r;
    refEnter();
    dsp4_road2_start(&r, head7);
    refLeave();
    unsigned long total = 0;
    for(int row = 0; row < 5; row++) {
      for(int i = 0; i < 6; i++) put(rows7[row][i]);
      uint16_t out[2];
      refEnter();
      const unsigned n = dsp4_road2_row(&r, rows7[row], out, lines, DSP4_CHIP_LINES);
      refLeave();
      check(dsp4_chip_pending(&chip) == 6 + 6 * n, "$07: the answer's length");
      for(int i = 0; i < 2; i++) expect(out[i], "$07 row word");
      expect((uint16_t)n, "$07 count");
      for(unsigned k = 0; k < n; k++)
        for(int j = 0; j < 3; j++) expect(lines[k][j], "$07 line word");
      total += n;
      check(chip.edge2 == ref2 && chip.edge3 == ref3, "$07: the chip's rounding follows dsp4.c's");
    }
    put(0x8000);
    check(dsp4_chip_idle(&chip), "$07: idle after $8000");
    check(ref2 == (int16_t)0x8000 && total > 0, "$07 drew and used up the rounding");
    check(dsp4_chip_unmodelled(&chip) == 0, "$07: all modelled");
    printf("$07: 5 rows, %lu lines\n", total);
  }

  // $08: a header as road_rows_stream sends it; rows with both lines equal, then one with two
  // lines (unmodelled, but still answered by dsp4.c)
  {
    uint16_t head8[45] = {0x00FF, 0x00FF, 0x00FF, 0x00FF, 0, 0, 0, 0, 0x00A6, 0x00A6, 0x00A6, 0x00A6,
                          0x00A5, 0x00A5, 0x00A7, 0x00A7, 0x0081, 0x007F, 0x0081, 0x007F,
                          0x34E8, 0x34E8, 0x34EA, 0x34EA, 0x0033, 0x0033, 0x0033, 0x0033,
                          0xFF5B, 0xFF5B, 0xFF5B, 0xFF5B, 0x2FC8, 0x0034, 0xFF5C, 0x0035, 0x7FFF,
                          0x0003, 0x0033, 0x0040, 0x0033, 0x0012, 0xFFE0, 0x0012, 0xFFE0};
    const uint16_t rows8[4][9] = {{0x7F00, 0x0050, 0x0030, 0x0044, 0x0030, 0x0020, 0xFFD0, 0x0020, 0xFFD0},
                                  {0x6000, 0x0050, 0x002A, 0x0048, 0x002A, 0x0030, 0xFFC0, 0x0030, 0xFFC0},
                                  {0x4000, 0x0050, 0x0020, 0x0038, 0x0020, 0x0028, 0xFFB8, 0x0028, 0xFFB8},
                                  {0x3000, 0x0050, 0x0010, 0x0030, 0x0018, 0x0018, 0xFFA0, 0x0010, 0xFFA8}};
    put(0x0008);
    for(int i = 0; i < 45; i++) put(head8[i]);
    Dsp4Window w;
    expect(dsp4_window_begin(&w, head8), "$08 first word");
    static uint16_t runs0[DSP4_CHIP_LINES][2], runs1[DSP4_CHIP_LINES][2];
    uint16_t (*runs[2])[2] = {runs0, runs1};
    unsigned long total = 0;
    for(int row = 0; row < 4; row++) {
      for(int i = 0; i < 9; i++) put(rows8[row][i]);
      unsigned n[2];
      dsp4_window_row(&w, rows8[row], runs, n, DSP4_CHIP_LINES);
      for(int k = 0; k < 2; k++) {
        expect((uint16_t)n[k], "$08 count");
        for(unsigned i = 0; i < n[k]; i++) {
          expect(runs[k][i][0], "$08 address");
          expect(runs[k][i][1], "$08 window");
        }
        total += n[k];
      }
      check(dsp4_chip_unmodelled(&chip) == (row == 3 ? 1u : 0u), "$08: unmodelled only for two lines");
    }
    put(0x8000);
    expect(dsp4_window_end(&w), "$08 last word");
    check(dsp4_chip_idle(&chip), "$08: idle after");
    printf("$08: 4 rows, %lu pairs\n", total);
  }

  // the other unmodelled answers: a $01 segment of 88+ lines, then $01's $8001
  {
    const unsigned long before = dsp4_chip_unmodelled(&chip);
    const uint16_t headFar[15] = {0, 0x0100, 0x0033, 0xFF5B, 0x00BF, 0x00DC, 0, 0, 0x0180, 0x035C, 0x00C0, 0, 0, 0, 0};
    const uint16_t hf[2] = {0x7F00, 0x2800};  // lines 254, then 80: 174 lines, none drawn
    const uint16_t zero[3] = {0, 0, 0};
    const uint16_t trackFar[2][3] = {{0, 0, 0}, {0, 0, 0}};
    testRoad(headFar, hf, 2, zero, trackFar, 0);
    check(dsp4_chip_unmodelled(&chip) == before + 1, "$01: an 88+ line segment is unmodelled");
    put(0x0001);
    for(int i = 0; i < 15; i++) put(headFar[i]);
    dsp4_chip_write(&chip, 0x01);
    dsp4_chip_write(&chip, 0x80);
    check(dsp4_chip_lost(&chip) && dsp4_chip_unmodelled(&chip) == before + 2, "$01: $8001 is unmodelled, the chip lost");
    dsp4_chip_abort(&chip);
    check(dsp4_chip_idle(&chip) && dsp4_chip_unmodelled(&chip) == before + 2, "abort: idle, count kept");
  }

  checkGlobal();
  printf("%lu checks, %lu failed; unmodelled %lu\n", checks, fails, dsp4_chip_unmodelled(&chip));
  return fails != 0;
}
