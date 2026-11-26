// SPDX-License-Identifier: GPL-3.0-or-later
// dsp4-01p: src/dsp4.c's road (/bin/bash1) on dsp4_road_probe's specs, printed in the probe's format,
// so the two can be compared line for line (tools/dsp4_road_check.py). Leading "E T NEAR FAR"
// and "M A B" run dsp4_edges and dsp4_multiply; the road's first rounding comes from the last
// E's edges 2 and 3 (000 without one, as the fresh chip).
//   build/bench/dsp4_road_native < spec
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dsp4.h"
int main(void) {
  char line[65536];
  while(fgets(line, sizeof line, stdin)) {
    char* p = line; unsigned w[4096]; int nw = 0; int16_t e2 = (int16_t)0x8000, e3 = (int16_t)0x8000;
    for(;;) {
      while(*p == ' ') p++;
      if(*p == 'E') { unsigned t, a, b; int k; sscanf(p + 1, "%x %x %x%n", &t, &a, &b, &k); p += 1 + k;
        int16_t o[4]; dsp4_edges((uint16_t)t, (uint16_t)a, (uint16_t)b, o); e2 = o[2]; e3 = o[3];
        for(int i = 0; i < 4; i++) printf("%04X ", (uint16_t)o[i]);
        continue; }
      if(*p == 'M') { unsigned a, b; int k; sscanf(p + 1, "%x %x%n", &a, &b, &k); p += 1 + k;
        int32_t q = dsp4_multiply((int16_t)a, (int16_t)b); printf("%04X %04X ", (uint16_t)q, (uint16_t)(q >> 16)); continue; }
      break;
    }
    if(p != line) printf("# ");
    while(*p == ' ') p++;
    const int r7 = p[0] == 'R' && p[1] == '7';  // dsp4-07a: the road command $07
    if(r7) p += 2;
    char* tok = strtok(p, " \n");
    while(tok) { w[nw++] = (unsigned)strtoul(tok, NULL, 16); tok = strtok(NULL, " \n"); }
    if(r7) {
      uint16_t head11[11];
      for(int i = 0; i < 11; i++) head11[i] = (uint16_t)w[i];
      dsp4_round_set(e2, e3);
      Dsp4Road r;
      dsp4_road2_start(&r, head11);
      for(int k = 0; 11 + 6 * k < nw; k++) {
        uint16_t words[6], out[2];
        static uint16_t lines[512][3];
        for(int i = 0; i < 6; i++) words[i] = (uint16_t)w[11 + 6 * k + i];
        const unsigned c = dsp4_road2_row(&r, words, out, lines, 512);
        printf("%s%04X %04X %u:", k ? " | " : "", out[0], out[1], c);
        for(unsigned i = 0; i < c && i < 512; i++) printf(" %04X,%04X,%04X", lines[i][0], lines[i][1], lines[i][2]);
      }
      printf("\n");
      continue;
    }
    uint16_t head[15]; for(int i = 0; i < 15; i++) head[i] = (uint16_t)w[i];
    Dsp4Road r; dsp4_road_begin(&r, head, e2, e3);
    for(int k = 0; 18 + 4 * k < nw; k++) {
      uint16_t first[3] = {(uint16_t)w[15], (uint16_t)w[16], (uint16_t)w[17]};
      uint16_t tr[3] = {(uint16_t)w[18 + 4 * k + 1], (uint16_t)w[18 + 4 * k + 2], (uint16_t)w[18 + 4 * k + 3]};
      uint16_t out[4]; static uint16_t lines[512][3];
      unsigned c = dsp4_road_row(&r, (uint16_t)w[18 + 4 * k], k == 0 ? first : NULL, tr, out, lines, 512);
      printf("%s%04X %04X %04X %04X %u:", k ? " | " : "", out[0], out[1], out[2], out[3], c);
      for(unsigned i = 0; i < c && i < 512; i++) printf(" %04X,%04X,%04X", lines[i][0], lines[i][1], lines[i][2]);
    }
    printf("\n");
  }
  return 0;
}
