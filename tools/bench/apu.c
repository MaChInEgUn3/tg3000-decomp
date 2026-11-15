// SPDX-License-Identifier: GPL-3.0-or-later
// APU port measurement: for every instruction that reads or writes $2140-$2143, the port,
// direction, how many times, in how many frames, the most writes it made in one frame,
// and up to 8 distinct values; plus, per frame, how many port writes there were in total
// (a histogram), which tells a per-frame message from a bulk upload.
#include "apu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t pc, frame;

#define MAX_SITES 512
typedef struct {
  uint32_t pc;
  uint8_t port, write;
  uint32_t count, frames, lastFrame, inFrame, maxInFrame;
  uint8_t values[8];
  int nValues;
  bool moreValues;
} Site;
static Site sites[MAX_SITES];
static int nSites;
static uint32_t writesThisFrame;
static uint32_t histogram[8];  // frames with 0, 1-16, 17-64, 65-256, 257-1024, 1025-4096, 4097-16384, more writes
static uint32_t frames;

void apu_start(Snes* snes) { (void)snes; }

void apu_on_insn(Cpu* cpu) {
  uint8_t bank = cpu->k;
  if(bank < 0x7E) bank |= 0x80;
  pc = (uint32_t)bank << 16 | cpu->pc;
}

void apu_on_access(uint32_t adr, uint8_t val, bool write) {
  if(((adr >> 16) & 0x7F) >= 0x40) return;
  uint16_t a = adr & 0xFFFF;
  if(a < 0x2140 || a > 0x217F) return;  // $2140-$2143 and their mirrors
  uint8_t port = a & 3;
  if(write) writesThisFrame++;
  Site* s = NULL;
  for(int i = 0; i < nSites; i++)
    if(sites[i].pc == pc && sites[i].port == port && sites[i].write == write) { s = &sites[i]; break; }
  if(s == NULL) {
    if(nSites >= MAX_SITES) return;
    s = &sites[nSites++];
    *s = (Site){pc, port, write, 0, 0, UINT32_MAX, 0, 0, {0}, 0, false};
  }
  s->count++;
  if(s->lastFrame != frame) { s->frames++; s->lastFrame = frame; s->inFrame = 0; }
  if(++s->inFrame > s->maxInFrame) s->maxInFrame = s->inFrame;
  bool seen = false;
  for(int i = 0; i < s->nValues; i++) if(s->values[i] == val) seen = true;
  if(!seen) { if(s->nValues < 8) s->values[s->nValues++] = val; else s->moreValues = true; }
}

void apu_on_frame(int f) {
  uint32_t w = writesThisFrame;
  int b = w == 0 ? 0 : w <= 16 ? 1 : w <= 64 ? 2 : w <= 256 ? 3 : w <= 1024 ? 4 : w <= 4096 ? 5 : w <= 16384 ? 6 : 7;
  histogram[b]++;
  writesThisFrame = 0;
  frames++;
  frame = (uint32_t)f + 1;
}

static int bySite(const void* a, const void* b) {
  const Site* x = a; const Site* y = b;
  return x->pc != y->pc ? (x->pc < y->pc ? -1 : 1) : x->port - y->port;
}

bool apu_save(const char* path) {
  FILE* f = fopen(path, "w");
  if(f == NULL) { perror(path); return false; }
  fprintf(f, "# APU port report over %u frames -- written by the bench (--apu-report)\n", frames);
  static const char* const bucket[8] = {"0", "1-16", "17-64", "65-256", "257-1024", "1025-4096", "4097-16384", ">16384"};
  fprintf(f, "\n# frames by number of port writes\n");
  for(int i = 0; i < 8; i++) if(histogram[i]) fprintf(f, "%10s writes: %u frames\n", bucket[i], histogram[i]);
  qsort(sites, (size_t)nSites, sizeof *sites, bySite);
  fprintf(f, "\n# access sites (%d%s): pc port dir count frames max-per-frame values\n", nSites, nSites >= MAX_SITES ? ", list full" : "");
  for(int i = 0; i < nSites; i++) {
    Site* s = &sites[i];
    fprintf(f, "$%06X $214%u %s x%u in %u frames (max %u/frame):", s->pc, s->port, s->write ? "write" : "read ", s->count,
      s->frames, s->maxInFrame);
    for(int k = 0; k < s->nValues; k++) fprintf(f, " %02X", s->values[k]);
    fprintf(f, "%s\n", s->moreValues ? " ..." : "");
  }
  fclose(f);
  return true;
}
