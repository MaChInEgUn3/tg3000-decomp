// SPDX-License-Identifier: GPL-3.0-or-later
// dsp4-01f (port request A): the road command $01 on chosen inputs, sent to the firmware DSP-4
// (harness-27's chip) as road_rows_project sends it ($85:B3D7, with D+$E4 negative: the
// objects copied, none sent). Outputs only: what the chip answers.
//   build/bench/dsp4_road_probe ext/firmware/dsp4.rom < spec
// The spec, one case a line: 15 header words, the first row's 3 words, then per row its
// height and 3 track words (all hex). Per case it prints one line: per row the 4 answers,
// the count and the count lines' 3 words, rows separated by '|'.
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>
#include <sstream>
#include <string>
#include <iostream>
extern "C" {
bool dsp4_lle_load(const char* path);
void dsp4_lle_reset(void);
uint8_t dsp4_lle_read(bool data);
void dsp4_lle_write(bool data, uint8_t v);
}

static uint16_t rd(void) {
  const uint8_t lo = dsp4_lle_read(true);
  return uint16_t(lo | dsp4_lle_read(true) << 8);
}
static void wr(uint16_t v) {
  dsp4_lle_write(true, uint8_t(v));
  dsp4_lle_write(true, uint8_t(v >> 8));
}

int main(int argc, char** argv) {
  if(argc < 2 || !dsp4_lle_load(argv[1])) { std::fprintf(stderr, "usage: %s dsp4.rom < spec\n", argv[0]); return 2; }
  std::string line;
  while(std::getline(std::cin, line)) {
    std::istringstream in(line);
    std::vector<unsigned> w;
    unsigned v;
    while(in >> std::hex >> v) w.push_back(v);
    if(w.size() < 18 || (w.size() - 18) % 4 != 0) { std::printf("bad case\n"); continue; }
    dsp4_lle_reset();
    wr(0x0001);
    for(int i = 0; i < 15; i++) wr(uint16_t(w[i]));
    for(size_t r = 0; 18 + 4 * r < w.size(); r++) {
      const unsigned* row = &w[18 + 4 * r];
      wr(uint16_t(row[0]));
      if(r == 0) for(int i = 15; i < 18; i++) wr(uint16_t(w[i]));
      for(int i = 1; i < 4; i++) wr(uint16_t(row[i]));
      uint16_t out[4];
      for(int i = 0; i < 4; i++) out[i] = rd();
      const uint16_t count = rd();
      std::printf("%s%04X %04X %04X %04X %u:", r ? " | " : "", out[0], out[1], out[2], out[3], count);
      for(unsigned k = 0; k < count && k < 512; k++) {
        const uint16_t a = rd(), b = rd(), c = rd();
        std::printf(" %04X,%04X,%04X", a, b, c);
      }
    }
    wr(0x8000);
    std::printf("\n");
  }
  return 0;
}
