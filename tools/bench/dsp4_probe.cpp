// SPDX-License-Identifier: GPL-3.0-or-later
// dsp4-11a: src/dsp4.c's commands against the firmware DSP-4 (harness-27's chip), on chosen
// inputs: build/bench/dsp4_probe ext/firmware/dsp4.rom [cases]. Outputs only (what the chip
// answers), never its memory.
//   $11 (dsp4_row): four edges in, one row word out.
//   $0A (dsp4_edges): t, near, far in; four edges out.
#include <cstdio>
#include <cstdlib>
#include <cstdint>
extern "C" {
#include "dsp4.h"
bool dsp4_lle_load(const char* path);
void dsp4_lle_command(uint16_t cmd, const uint16_t* in, int nin, uint16_t* out, int nout);
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
  std::printf("%ld cases: $11 %ld differ, $0A %ld differ\n", cases, bad11, bad0a);
  return bad11 || bad0a ? 1 : 0;
}
