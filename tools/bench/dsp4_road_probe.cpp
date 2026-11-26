// SPDX-License-Identifier: GPL-3.0-or-later
// dsp4-01f (port request A): the road command $01 on chosen inputs, sent to the firmware DSP-4
// (harness-27's chip) as road_rows_project sends it ($85:B3D7, with D+$E4 negative: the
// objects copied, none sent). Outputs only: what the chip answers.
//   build/bench/dsp4_road_probe ext/firmware/dsp4.rom [--edges NEAR] < spec
// --edges NEAR (hex): before each case, a $0A with that near row (t = 0, far = NEAR), its
// answer read, as the game's frame does before the road (dsp4-01c's bit 7). A case line may
// instead start with its own commands, "E T NEAR FAR" ($0A), "M A B" ($00) and "B X Y ATTR"
// ($0B, a sprite) in any order;
// their answers are printed first, then '#'.
// The spec, one case a line: 15 header words, the first row's 3 words, then per row its
// height and 3 track words (all hex). Per case it prints one line: per row the 4 answers,
// the count and the count lines' 3 words, rows separated by '|'.
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
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
  if(argc < 2 || !dsp4_lle_load(argv[1])) { std::fprintf(stderr, "usage: %s dsp4.rom [--edges NEAR] < spec\n", argv[0]); return 2; }
  const bool edges = argc >= 4 && std::strcmp(argv[2], "--edges") == 0;
  const uint16_t near = edges ? uint16_t(std::strtoul(argv[3], nullptr, 16)) : 0;
  std::string line;
  while(std::getline(std::cin, line)) {
    std::istringstream in(line);
    std::vector<unsigned> w;
    unsigned v;
    // leading commands, each a letter and its hex words: "E T NEAR FAR" ($0A, 4 answers) or
    // "M A B" ($00, 2 answers); their answers are printed, then '#'
    std::vector<std::vector<unsigned>> pre;
    while(true) {
      in >> std::ws;
      const int c = in.peek();
      if(c != 'E' && c != 'M' && c != 'B') break;
      in.get();
      std::vector<unsigned> p{unsigned(c)};
      for(int i = 0; i < (c == 'M' ? 2 : 3); i++) { in >> std::hex >> v; p.push_back(v); }
      pre.push_back(p);
    }
    // dsp4-08b: "R8": the window command $08, 45 header words, then rows of 9 words.
    // dsp4-07a: "R7" first: the road command $07 instead, 11 header words, then per row its
    // height and 5 words; per row it prints 2 answers, the count and the lines
    bool r7 = false, r8 = false;
    in >> std::ws;
    if(in.peek() == 'R') {
      std::string tag;
      in >> tag;
      r7 = tag == "R7";
      r8 = tag == "R8";
    }
    while(in >> std::hex >> v) w.push_back(v);
    if(r8 ? (w.size() < 45 || (w.size() - 45) % 9 != 0)
          : r7 ? (w.size() < 11 || (w.size() - 11) % 6 != 0) : (w.size() < 18 || (w.size() - 18) % 4 != 0)) {
      std::printf("bad case\n");
      continue;
    }
    dsp4_lle_reset();
    if(!pre.empty()) {
      for(const auto& p : pre) {
        wr(p[0] == 'E' ? 0x000A : p[0] == 'B' ? 0x000B : 0x0000);
        for(size_t i = 1; i < p.size(); i++) wr(uint16_t(p[i]));
        if(p[0] == 'B') {  // a sprite: its flag, then two words when shown
          const uint16_t f = rd();
          std::printf("%04X ", f);
          if(f) for(int i = 0; i < 2; i++) std::printf("%04X ", rd());
          continue;
        }
        for(int i = 0; i < (p[0] == 'E' ? 4 : 2); i++) std::printf("%04X ", rd());
      }
      std::printf("# ");
    } else if(edges) {
      wr(0x000A);
      wr(0x0000);
      wr(near);
      wr(near);
      for(int i = 0; i < 4; i++) (void)rd();
    }
    if(r8) {  // dsp4-08b: the window command $08: 45 header words, then rows of 9 words; prints the
              // first answer, per row its two runs (count: address,word ...), then the last answer
      wr(0x0008);
      for(int i = 0; i < 45; i++) wr(uint16_t(w[i]));
      std::printf("%04X", rd());
      for(size_t r = 0; 45 + 9 * r < w.size(); r++) {
        for(int i = 0; i < 9; i++) wr(uint16_t(w[45 + 9 * r + i]));
        std::printf(" |");
        for(int run = 0; run < 2; run++) {
          const uint16_t n = rd();
          std::printf(" %u:", n);
          for(unsigned k = 0; k < n && k < 512; k++) {
            const uint16_t a = rd(), b = rd();
            std::printf(" %04X,%04X", a, b);
          }
        }
      }
      wr(0x8000);
      std::printf(" | %04X\n", rd());
      continue;
    }
    if(r7) {  // dsp4-07a: the road command $07
      wr(0x0007);
      for(int i = 0; i < 11; i++) wr(uint16_t(w[i]));
      for(size_t r = 0; 11 + 6 * r < w.size(); r++) {
        for(int i = 0; i < 6; i++) wr(uint16_t(w[11 + 6 * r + i]));
        uint16_t out[2];
        for(int i = 0; i < 2; i++) out[i] = rd();
        const uint16_t count = rd();
        std::printf("%s%04X %04X %u:", r ? " | " : "", out[0], out[1], count);
        for(unsigned k = 0; k < count && k < 512; k++) {
          const uint16_t a = rd(), b = rd(), c = rd();
          std::printf(" %04X,%04X,%04X", a, b, c);
        }
      }
      wr(0x8000);
      std::printf("\n");
      continue;
    }
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
