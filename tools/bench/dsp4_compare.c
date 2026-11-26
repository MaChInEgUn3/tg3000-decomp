// SPDX-License-Identifier: GPL-3.0-or-later
// dsp4-chip-1 (port request A): src/dsp4.c's chip against a recording of the firmware chip.
//   build/bench/dsp4_compare tape.bin sites.txt   (sites: python3 -I tools/dsp4_sites.py)
// The tape is what the game wrote to the DSP-4 and read from it (5-byte records: PC low byte
// first, the value, 1 for a read), from bench --dsp4-tape-out with --dsp4-firmware, or from
// refbench --dsp4-tape. Every write goes to the C chip; every read is compared with the byte
// the firmware gave. Outputs only: the tape holds what the game sent and got.
// Per command: its runs, the bytes read and how many differ, and the first PC where one did.
// A command the C chip does not have yet leaves it lost: the tape's bytes are then counted
// against that command until the next command word the chip has. A restart is taken only at
// a place where the listing writes that word as a constant (sites.txt: the game's code, not
// the chip's), right after a read (the last command's answer), and where the records ahead
// have that command's shape exactly: its word (low byte, then 0, at
// one PC), its parameters (writes), its answer (reads; the multiply's low word may be skipped
// by writes; a sprite's first word of 0 ends it), then a write or the tape's end. Exit status
// 1 when a command the chip has differs.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dsp4_chip.h"

typedef struct { unsigned long runs, read, bad, lostBytes; unsigned firstBadPc; } Count;
static Count counts[257];

static int known(unsigned v) { return v == 0x00 || v == 0x01 || v == 0x0A || v == 0x11 || v == 0x03 || v == 0x0B; }

#define SITES 4096
static unsigned sitePc[SITES], siteCmd[SITES];
static int nSites;
static int isSite(unsigned pc, unsigned cmd) {
  for(int i = 0; i < nSites; i++) if(sitePc[i] == pc && siteCmd[i] == cmd) return 1;
  return 0;
}

static const unsigned char* t;
static long n;
static unsigned pcAt(long i) { return (unsigned)(t[5 * i] | t[5 * i + 1] << 8 | t[5 * i + 2] << 16); }
static int isRead(long i) { return t[5 * i + 4] & 1; }
static unsigned valAt(long i) { return t[5 * i + 3]; }

// whether a command the chip has starts at record i (see above)
static int shapeAt(long i) {
  const unsigned c = valAt(i);
  if(!known(c) || !isSite(pcAt(i), c) || (i > 0 && !isRead(i - 1)) || i + 1 >= n || isRead(i) || isRead(i + 1) || valAt(i + 1) != 0 || pcAt(i + 1) != pcAt(i)) return 0;
  long k = i + 2;
  if(c == 0x01) {  // the road: its header's 15 words (the rows follow as they come)
    for(int j = 0; j < 30; j++, k++) if(k >= n || isRead(k)) return 0;
    return 1;
  }
  const int in = c == 0x00 ? 4 : c == 0x0A ? 6 : c == 0x11 ? 8 : c == 0x03 ? 10 : 6;
  for(int j = 0; j < in; j++, k++) if(k >= n || isRead(k)) return 0;
  if(c == 0x00) {
    for(int j = 0; j < 4; j++, k++) if(k >= n || (j >= 2 && !isRead(k))) return 0;
  } else if(c == 0x0A || c == 0x11) {
    const int out = c == 0x0A ? 8 : 2;
    for(int j = 0; j < out; j++, k++) if(k >= n || !isRead(k)) return 0;
  } else {
    if(k + 1 >= n || !isRead(k) || !isRead(k + 1)) return 0;
    const int hidden = valAt(k) == 0 && valAt(k + 1) == 0;
    k += 2;
    if(!hidden) for(int j = 0; j < 4; j++, k++) if(k >= n || !isRead(k)) return 0;
  }
  return k >= n || !isRead(k);
}

int main(int argc, char** argv) {
  if(argc < 3) { fprintf(stderr, "usage: %s tape.bin sites.txt\n", argv[0]); return 2; }
  FILE* sf = fopen(argv[2], "r");
  if(sf == NULL) { perror(argv[2]); return 2; }
  while(nSites < SITES && fscanf(sf, "%x %x", &sitePc[nSites], &siteCmd[nSites]) == 2) nSites++;
  fclose(sf);
  FILE* f = fopen(argv[1], "rb");
  if(f == NULL) { perror(argv[1]); return 2; }
  fseek(f, 0, SEEK_END);
  const long size = ftell(f);
  fseek(f, 0, SEEK_SET);
  unsigned char* buf = malloc((size_t)size);
  if(buf == NULL || fread(buf, 1, (size_t)size, f) != (size_t)size) return 2;
  fclose(f);
  t = buf;
  n = size / 5;
  Dsp4Chip chip;
  dsp4_chip_reset(&chip);
  unsigned lostCmd = 0;
  unsigned long resyncs = 0;
  for(long i = 0; i < n; i++) {
    const unsigned char* r = t + 5 * i;
    const unsigned pc = pcAt(i), v = r[3];
    const int read = r[4] & 1;
    if(dsp4_chip_lost(&chip)) {
      if(shapeAt(i)) {
        dsp4_chip_reset(&chip);
        resyncs++;
      } else {
        counts[lostCmd].lostBytes++;
        continue;
      }
    }
    if(read) {
      const unsigned cmd = chip.phase == 0 ? 0x100 : chip.cmd;  // 0x100: an idle read
      const unsigned got = dsp4_chip_read(&chip);
      if(dsp4_chip_lost(&chip)) { lostCmd = cmd; counts[lostCmd].lostBytes++; continue; }
      counts[cmd].read++;
      if(got != v) {
        if(counts[cmd].bad++ == 0) counts[cmd].firstBadPc = pc;
      }
      continue;
    }
    if(chip.phase == 0) counts[v].runs++;
    const unsigned cmd = chip.phase == 0 ? v : chip.cmd;
    dsp4_chip_write(&chip, (unsigned char)v);
    if(dsp4_chip_lost(&chip)) { lostCmd = cmd; counts[lostCmd].lostBytes++; }
  }
  printf("%ld records; %lu restarts after a command the chip does not have\n", n, resyncs);
  printf("cmd   runs        bytes read   differ  first PC  | bytes while lost\n");
  int fail = 0;
  for(int c = 0; c < 257; c++) {
    const Count* k = &counts[c];
    if(k->runs == 0 && k->read == 0 && k->lostBytes == 0) continue;
    if(c == 0x100) printf("idle"); else printf("$%02X ", c);
    printf("%10lu %14lu %8lu  ", k->runs, k->read, k->bad);
    if(k->bad) printf("$%06X", k->firstBadPc); else printf("   -   ");
    printf("  | %lu%s\n", k->lostBytes, c == 0x100 || known((unsigned)c) ? "" : "  (not in the C chip yet)");
    if((c == 0x100 || known((unsigned)c)) && k->bad) fail = 1;
  }
  free(buf);
  return fail;
}
