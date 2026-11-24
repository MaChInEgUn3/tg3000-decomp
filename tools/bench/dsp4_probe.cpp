// SPDX-License-Identifier: GPL-3.0-or-later
// dsp4-11a: src/dsp4.c's commands against the firmware DSP-4 (harness-27's chip), on chosen
// inputs: build/bench/dsp4_probe ext/firmware/dsp4.rom [cases]. Outputs only (what the chip
// answers), never its memory.
//   $11 (dsp4_row): four edges in, one row word out.
//   $0A (dsp4_edges): t, near, far in; four edges out.
//   $03/$0B (dsp4_sprite, dsp4-03b): as the game sends them ($82:CBD4, $82:D8C6): $0003,
//   $0005, then each entry $000B, x, y, attribute; out a flag, then (when it is not 0) two
//   words. Single entries on a fresh list, every y in -512..511: the flag and the words
//   against dsp4_sprite (dsp4-03c). The firmware's per-row count is only reported.
#include <cstdio>
#include <cstdlib>
#include <cstdint>
extern "C" {
#include "dsp4.h"
bool dsp4_lle_load(const char* path);
void dsp4_lle_command(uint16_t cmd, const uint16_t* in, int nin, uint16_t* out, int nout);
uint8_t dsp4_lle_read(bool data);
void dsp4_lle_write(bool data, uint8_t v);
}

static uint16_t dsp4_lle_word(void) {  // one word out, low byte first
  const uint8_t lo = dsp4_lle_read(true);
  return uint16_t(lo | dsp4_lle_read(true) << 8);
}
static void dsp4_lle_word_in(uint16_t v) {
  dsp4_lle_write(true, uint8_t(v));
  dsp4_lle_write(true, uint8_t(v >> 8));
}
static uint32_t seed = 12345;
static uint32_t rnd(void) { seed = seed * 1103515245u + 12345u; return seed >> 8; }

int main(int argc, char** argv) {
  if(argc < 2 || !dsp4_lle_load(argv[1])) { std::fprintf(stderr, "usage: %s dsp4.rom [cases]\n", argv[0]); return 2; }
  const long cases = argc > 2 ? std::atol(argv[2]) : 100000;
  long bad11 = 0, bad0a = 0;
  for(long k = 0; k < cases; k++) {
    // $11: edges, small (the game's range) or any
    uint16_t e[4];
    const int span = argc > 3 ? std::atoi(argv[3]) : 0x400;  // edges in [-span, span)
    for(int j = 0; j < 4; j++) e[j] = uint16_t(int16_t((int)(rnd() % (2 * span)) - span));
    uint16_t fw;
    dsp4_lle_command(0x0011, e, 4, &fw, 1);
    const int16_t se[4] = {int16_t(e[0]), int16_t(e[1]), int16_t(e[2]), int16_t(e[3])};
    const uint16_t c = dsp4_row(se);
    if(fw != c && bad11++ < 12)
      std::printf("$11 edges %d %d %d %d: firmware %04X, C %04X\n", se[0], se[1], se[2], se[3], fw, c);
    // $0A: t, near, far
    uint16_t in[3] = {uint16_t(rnd() & 0x7FFF), uint16_t(rnd()), uint16_t(rnd())};
    uint16_t fo[4];
    dsp4_lle_command(0x000A, in, 3, fo, 4);
    int16_t co[4];
    dsp4_edges(in[0], in[1], in[2], co);
    bool same = true;
    for(int j = 0; j < 4; j++) same = same && fo[j] == uint16_t(co[j]);
    if(!same && bad0a++ < 12)
      std::printf("$0A t %04X near %04X far %04X: firmware %04X %04X %04X %04X, C %04X %04X %04X %04X\n", in[0], in[1], in[2],
                  fo[0], fo[1], fo[2], fo[3], uint16_t(co[0]), uint16_t(co[1]), uint16_t(co[2]), uint16_t(co[3]));
  }
  {
    // dsp4-03b: single entries on a fresh list (the chip reset first), every y in
    // -512..511, x and the attribute at random
    long shown = 0, hiddenRows = 0, badWords = 0, badFlags = 0;
    int prevShown = -1, start = 0;
    std::printf("$03/$0B single entries, the firmware's flag by y:");
    for(int y = -512; y <= 512; y++) {
      int s = -2;
      if(y < 512) {
        const uint16_t x = uint16_t(rnd()), attr = uint16_t(rnd());
        const uint16_t in[5] = {0x0005, 0x000B, x, uint16_t(y), attr};
        uint16_t fo[3] = {0, 0, 0};
        dsp4_lle_command(0x0003, in, 5, fo, 1);
        s = fo[0] != 0;
        uint16_t mine[2];
        const uint16_t flag = dsp4_sprite(x, uint16_t(y), attr, mine);
        if(flag != fo[0] && badFlags++ < 12) std::printf("\n  y %d: firmware flag %04X, C %04X", y, fo[0], flag);
        if(s) {
          fo[1] = dsp4_lle_word();
          fo[2] = dsp4_lle_word();
          shown++;
          if((fo[0] != 1 || fo[1] != mine[0] || fo[2] != mine[1]) && badWords++ < 12)
            std::printf("\n  y %d: firmware %04X %04X %04X, C 0001 %04X %04X", y, fo[0], fo[1], fo[2], mine[0], mine[1]);
        } else hiddenRows++;
      }
      if(s != prevShown) {
        if(prevShown >= 0) std::printf(" [%d..%d] %s", start, y - 1, prevShown ? "shown" : "hidden");
        prevShown = s;
        start = y;
      }
    }
    std::printf("\n  %ld shown (their words: %ld differ), %ld hidden; flags: %ld differ from dsp4_sprite\n", shown, badWords,
                hiddenRows, badFlags);
    // and one list of many entries on one row: the firmware's count per row
    const uint16_t head[1] = {0x0005};
    uint16_t none[1];
    dsp4_lle_command(0x0003, head, 1, none, 0);
    int firstHidden = -1;
    for(int n = 0; n < 64 && firstHidden < 0; n++) {
      const uint16_t e[4] = {0x000B, 300, 50, 0x30FD};
      for(int i = 0; i < 4; i++) dsp4_lle_word_in(e[i]);
      if(dsp4_lle_word() == 0) firstHidden = n + 1;
      else { dsp4_lle_word(); dsp4_lle_word(); }
    }
    std::printf("  one list, every entry at y 50: the firmware hides entry %d on (a count per row)\n", firstHidden);
    if(badWords || badFlags) return 1;
  }
  std::printf("%ld cases: $11 %ld differ, $0A %ld differ\n", cases, bad11, bad0a);
  return bad11 || bad0a ? 1 : 0;
}
