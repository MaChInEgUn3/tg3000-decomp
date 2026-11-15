// SPDX-License-Identifier: GPL-3.0-or-later
// HDMA measurement: per frame, the configuration of every channel enabled in $420C
// (B-bus target, mode, indirect, table address); the instruction addresses that write
// $420C, the channel registers $43x0-$43xA and $2105 (BG mode, with the scanline); and,
// for tables in WRAM, which instructions last wrote their first 64 bytes. CPU writes only:
// a table filled by DMA shows no writer.
#include "hdma.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dma.h"
#include "ppu.h"

static Snes* snes;
static uint32_t pc;                       // the instruction being executed ($80+ mirror for ROM)
static uint32_t* writer;                  // last CPU writer of each WRAM byte
static uint32_t frames;

#define MAX_ITEMS 8192  // per list; a power of two (open addressing)
typedef struct { uint32_t key[4]; uint32_t count; uint32_t first, last; } Item;
typedef struct { Item* items; int32_t* slots; int n; bool full; } Set;

static Set configs, regWrites, enableWrites, modeWrites, tableWriters;

static uint32_t hashKey(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
  uint32_t h = 2166136261u;
  uint32_t k[4] = {a, b, c, d};
  for(int i = 0; i < 4; i++) h = (h ^ k[i]) * 16777619u;
  return h;
}

static void note(Set* s, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
  if(s->items == NULL) {
    s->items = calloc(MAX_ITEMS, sizeof *s->items);
    s->slots = malloc(2 * MAX_ITEMS * sizeof *s->slots);
    for(int i = 0; i < 2 * MAX_ITEMS; i++) s->slots[i] = -1;
  }
  for(uint32_t h = hashKey(a, b, c, d) & (2 * MAX_ITEMS - 1);; h = (h + 1) & (2 * MAX_ITEMS - 1)) {
    int32_t i = s->slots[h];
    if(i < 0) {
      if(s->n >= MAX_ITEMS) { s->full = true; return; }
      s->items[s->n] = (Item){{a, b, c, d}, 1, frames, frames};
      s->slots[h] = s->n++;
      return;
    }
    Item* it = &s->items[i];
    if(it->key[0] == a && it->key[1] == b && it->key[2] == c && it->key[3] == d) {
      it->count++;
      it->last = frames;
      return;
    }
  }
}

void hdma_start(Snes* s) {
  snes = s;
  writer = calloc(0x20000, sizeof *writer);
}

void hdma_on_insn(Cpu* cpu) {
  uint8_t bank = cpu->k;
  if(bank < 0x7E) bank |= 0x80;
  pc = (uint32_t)bank << 16 | cpu->pc;
}

static int wramIndex(uint32_t adr) {
  uint8_t bank = adr >> 16;
  if(bank == 0x7E || bank == 0x7F) return (int)(((uint32_t)(bank - 0x7E) << 16) | (adr & 0xFFFF));
  if((bank & 0x7F) < 0x40 && (adr & 0xFFFF) < 0x2000) return (int)(adr & 0x1FFF);
  return -1;
}

void hdma_on_access(uint32_t adr, uint8_t val, bool write) {
  if(!write) return;
  int w = wramIndex(adr);
  if(w >= 0) { writer[w] = pc; return; }
  if(((adr >> 16) & 0x7F) >= 0x40) return;
  uint16_t a = adr & 0xFFFF;
  if(a == 0x420C) note(&enableWrites, pc, val, 0, 0);
  else if(a >= 0x4300 && a <= 0x437F && (a & 0xF) <= 0xA) note(&regWrites, (a >> 4) & 7, a & 0xF, pc, 0);
  else if(a == 0x2105) note(&modeWrites, val, pc, snes->vPos >= 225 ? 1 : 0, 0);
}

void hdma_on_frame(int frame) {
  frames = (uint32_t)frame;
  for(int c = 0; c < 8; c++) {
    DmaChannel* ch = &snes->dma->channel[c];
    if(!ch->hdmaActive) continue;
    uint32_t table = (uint32_t)ch->aBank << 16 | ch->aAdr;
    note(&configs, (uint32_t)c, (uint32_t)ch->bAdr << 8 | ch->mode, ch->indirect ? (0x100u | ch->indBank) : 0, table);
    int w = wramIndex(table);
    if(w < 0) continue;
    for(int k = 0; k < 64; k++) {
      uint32_t who = writer[(w + k) & 0x1FFFF];
      if(who) note(&tableWriters, (uint32_t)c, table, who, 0);
    }
  }
}

static void dump(FILE* f, const char* title, Set* s, void (*row)(FILE*, Item*)) {
  fprintf(f, "\n# %s (%d%s)\n", title, s->n, s->full ? ", list full" : "");
  for(int i = 0; i < s->n; i++) row(f, &s->items[i]);
}

static void rowConfig(FILE* f, Item* it) {
  fprintf(f, "ch%u -> $21%02X mode %u %s table $%06X  frames %u (%u..%u)\n", it->key[0], it->key[1] >> 8,
    it->key[1] & 7, it->key[2] ? "indirect" : "direct", it->key[3], it->count, it->first, it->last);
  if(it->key[2]) fprintf(f, "    indirect data bank $%02X\n", it->key[2] & 0xFF);
}
static void rowReg(FILE* f, Item* it) { fprintf(f, "ch%u $43%u%X by $%06X x%u\n", it->key[0], it->key[0], it->key[1], it->key[2], it->count); }
static void rowEnable(FILE* f, Item* it) { fprintf(f, "$420C = $%02X by $%06X x%u (frames %u..%u)\n", it->key[1], it->key[0], it->count, it->first, it->last); }
static void rowMode(FILE* f, Item* it) {
  fprintf(f, "$2105 = $%02X by $%06X %s x%u (frames %u..%u)\n", it->key[0], it->key[1], it->key[2] ? "in vblank" : "while drawing",
    it->count, it->first, it->last);
}
static void rowWriter(FILE* f, Item* it) { fprintf(f, "ch%u table $%06X written by $%06X (%u frame-bytes)\n", it->key[0], it->key[1], it->key[2], it->count); }

bool hdma_save(const char* path) {
  FILE* f = fopen(path, "w");
  if(f == NULL) { perror(path); return false; }
  fprintf(f, "# HDMA report over %u frames -- written by the bench (--hdma-report)\n", frames + 1);
  dump(f, "channel configurations seen at the end of a frame", &configs, rowConfig);
  dump(f, "$420C (HDMA enable) writers", &enableWrites, rowEnable);
  dump(f, "channel register writers", &regWrites, rowReg);
  dump(f, "$2105 (BG mode) writers", &modeWrites, rowMode);
  dump(f, "last CPU writers of the first 64 bytes of WRAM tables", &tableWriters, rowWriter);
  fclose(f);
  return true;
}
