// SPDX-License-Identifier: GPL-3.0-or-later
// dsp4-09c: replay tapes (refbench/bench --dsp4-tape-out records: PC, value, 1 for a read) into
// the firmware DSP-4, each on a chip just powered on, and compare every byte read with the
// tape's. Outputs only: the chip is driven as the game drives it.
//   build/bench/dsp4_tape_run ext/firmware/dsp4.rom FROM tape[@FROM]...
// (a tape's own @FROM overrides FROM). A record whose direction byte is 2 reads one $08 run
// (a count word, then 4 bytes a pair), however long the chip makes it; it is not compared.
// A record with direction 3 reads one $09 part's answer (a flag, then per 1 two words and another
// flag), however long; dumped, not compared.
// dsp4-01v: direction 4 reads one $01 row's answer (4 words, a count, 3 words a line); dumped as
// dsp4_road_native prints it (rows separated by '|'), and nothing else is dumped then.
// TAPE_RUN_SHOW=N prints each tape's first N differing reads (record, PC, tape and chip byte).
// TAPE_RUN_DUMP=1 prints instead each tape's bytes read from FROM on, in hex, on one line. Per tape one line: its reads at record FROM or later that differ, of how many, the first
// (its record and PC), then the differing reads by PC (at most 8). Reads before FROM are not
// compared (they set the chip up: a frame's commands before the one measured).
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>
extern "C" {
bool dsp4_lle_load(const char* path);
void dsp4_lle_reset(void);
uint8_t dsp4_lle_read(bool data);
void dsp4_lle_write(bool data, uint8_t v);
int dsp4_lle_state_size(void);
void dsp4_lle_save_state(uint8_t* out, int size);
void dsp4_lle_load_state(const uint8_t* in, int size);
}

int main(int argc, char** argv) {
  if(argc < 4 || !dsp4_lle_load(argv[1])) { std::fprintf(stderr, "usage: %s dsp4.rom FROM tape...\n", argv[0]); return 2; }
  const long from0 = std::strtol(argv[2], nullptr, 10);
  const int size = dsp4_lle_state_size();
  std::vector<uint8_t> power(size);
  dsp4_lle_save_state(power.data(), size);
  for(int t = 3; t < argc; t++) {
    std::string path = argv[t];
    long from = from0;
    const size_t at = path.rfind('@');
    if(at != std::string::npos) {
      from = std::strtol(path.c_str() + at + 1, nullptr, 10);
      path.resize(at);
    }
    FILE* f = std::fopen(path.c_str(), "rb");
    if(f == nullptr) { std::printf("%s: cannot open\n", path.c_str()); continue; }
    dsp4_lle_load_state(power.data(), size);
    uint8_t b[5];
    long k = 0, n = 0, bad = 0, first = -1;
    uint32_t firstPc = 0;
    std::map<uint32_t, long> byPc;
    long show = std::getenv("TAPE_RUN_SHOW") ? std::strtol(std::getenv("TAPE_RUN_SHOW"), nullptr, 10) : 0;
    const bool dump = std::getenv("TAPE_RUN_DUMP") != nullptr;
    std::string dumped;
    while(std::fread(b, 1, 5, f) == 5) {
      const uint32_t pc = uint32_t(b[0] | b[1] << 8 | b[2] << 16);
      if(b[4] == 3) {  // dsp4-09i: a $09 part's answer: a flag, then per 1 an entry (2 words) and another flag
        for(;;) {
          const uint16_t flag = uint16_t(dsp4_lle_read(true) | dsp4_lle_read(true) << 8);
          if(dump) { char t[8]; std::snprintf(t, sizeof t, "%02X%02X", flag & 0xFF, flag >> 8); dumped += t; }
          if(flag == 0) break;
          for(int q = 0; q < 4; q++) {
            const uint8_t v = dsp4_lle_read(true);
            if(dump) { char t[4]; std::snprintf(t, sizeof t, "%02X", v); dumped += t; }
          }
        }
      } else if(b[4] == 4) {  // dsp4-01v: one $01 row's answer: 4 words, a count, 3 words a line; dumped
        uint16_t w[5];
        for(int q = 0; q < 5; q++) w[q] = uint16_t(dsp4_lle_read(true) | dsp4_lle_read(true) << 8);
        if(dump) {
          char t[40];
          std::snprintf(t, sizeof t, "%s%04X %04X %04X %04X %u:", dumped.empty() ? "" : " | ", w[0], w[1], w[2], w[3], w[4]);
          dumped += t;
        }
        for(unsigned q = 0; q < w[4] && q < 512; q++) {
          uint16_t l[3];
          for(int i = 0; i < 3; i++) l[i] = uint16_t(dsp4_lle_read(true) | dsp4_lle_read(true) << 8);
          if(dump) { char t[20]; std::snprintf(t, sizeof t, " %04X,%04X,%04X", l[0], l[1], l[2]); dumped += t; }
        }
      } else if(b[4] == 2) {  // an $08 run, its length the chip's: a count word, then 4 bytes a pair (not compared)
        const uint16_t cnt = uint16_t(dsp4_lle_read(true) | dsp4_lle_read(true) << 8);
        for(unsigned q = 0; q < 4u * cnt; q++) (void)dsp4_lle_read(true);
      } else if(!(b[4] & 1)) dsp4_lle_write(true, b[3]);
      else {
        const uint8_t v = dsp4_lle_read(true);
        if(k >= from) {
          if(dump) { char t[4]; std::snprintf(t, sizeof t, "%02X", v); dumped += t; }
          n++;
          if(v != b[3]) {
            if(show > 0) { std::printf("  r%ld $%06X tape %02X chip %02X\n", k, pc, b[3], v); show--; }
            bad++;
            byPc[pc]++;
            if(first < 0) { first = k; firstPc = pc; }
          }
        }
      }
      k++;
    }
    std::fclose(f);
    if(dump) { std::printf("%s: %s\n", path.c_str(), dumped.c_str()); continue; }
    std::printf("%s: %ld of %ld differ", path.c_str(), bad, n);
    if(first >= 0) std::printf(" (first at record %ld, $%06X)", first, firstPc);
    int shown = 0;
    for(const auto& p : byPc) if(shown++ < 8) std::printf(" $%06X:%ld", p.first, p.second);
    std::printf("\n");
  }
  return 0;
}
