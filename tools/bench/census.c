// SPDX-License-Identifier: GPL-3.0-or-later
// Routine census: for every routine entered with JSR/JSL (and every interrupt handler),
// what the movie shows it doing. A shadow call stack attributes each hardware access to
// the innermost routine running; a frame ends when the stack pointer climbs back above
// its entry value.
#include "census.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "snes.h"

#define MAX_ROUTINES 4096
#define MAX_FRAMES 64
#define MAX_EDGES 48
#define MAX_STATES 6

enum {
  HW_MATH = 1, HW_PPU = 2, HW_DMA = 4, HW_HDMA = 8, HW_APU = 16, HW_IO = 32, HW_DSP4 = 64, HW_OTHER = 128,
};
static const char* hwNames[8] = {"math", "ppu", "dma", "hdma", "apu", "io", "dsp4", "other"};

typedef struct { uint16_t d; uint8_t db; bool m8, x8; } State;

typedef struct {
  uint32_t entry;
  bool isInterrupt;
  unsigned long entries, interrupted;
  uint8_t hw;
  State states[MAX_STATES];
  int stateCount;
  uint32_t callees[MAX_EDGES];
  int calleeCount;
  uint32_t indirect[MAX_EDGES];
  int indirectCount;
} Routine;

typedef struct { int routine; uint16_t sp; bool interrupted; } Frame;

static Snes* snes;
static const uint8_t* rom;
static Routine routines[MAX_ROUTINES];
static int routineCount;
static Frame frames[MAX_FRAMES];
static int depth;
static bool pendingCall, pendingIndirect;
static uint32_t lastInterrupts;

// ROM addresses go to the $80-$FF mirror; WRAM (code copied there, e.g. the interrupt
// trampolines at $00:0000/$00:0004) keeps its own address.
static uint32_t norm(uint32_t adr) {
  uint8_t bank = adr >> 16;
  bool rom = (bank & 0x7F) < 0x7E && ((adr & 0xFFFF) >= 0x8000 || (bank & 0x7F) >= 0x40);
  if(rom) bank |= 0x80;
  return ((uint32_t)bank << 16) | (adr & 0xFFFF);
}

static uint8_t peek(uint32_t adr) {
  uint8_t bank = (adr >> 16) & 0x7F;
  if(bank < 0x7E && ((adr & 0xFFFF) >= 0x8000 || bank >= 0x40)) return rom[((bank << 15) | (adr & 0x7FFF)) & 0xFFFFF];
  if(bank == 0x7E || bank == 0x7F) return snes->ram[((bank - 0x7E) << 16) | (adr & 0xFFFF)];
  if((adr & 0xFFFF) < 0x2000) return snes->ram[adr & 0x1FFF];
  return 0xEA;
}

static int findRoutine(uint32_t entry, bool isInterrupt) {
  for(int i = 0; i < routineCount; i++) if(routines[i].entry == entry) return i;
  if(routineCount >= MAX_ROUTINES) return -1;
  routines[routineCount] = (Routine){.entry = entry, .isInterrupt = isInterrupt};
  return routineCount++;
}

static void addEdge(uint32_t* list, int* count, uint32_t target) {
  for(int i = 0; i < *count; i++) if(list[i] == target) return;
  if(*count < MAX_EDGES) list[(*count)++] = target;
}

static void enter(const Cpu* cpu, uint32_t entry, bool isInterrupt) {
  int r = findRoutine(entry, isInterrupt);
  if(r < 0 || depth >= MAX_FRAMES) return;
  // an interrupt handler is not a callee of the routine it interrupted
  if(depth > 0 && !isInterrupt) addEdge(routines[frames[depth - 1].routine].callees, &routines[frames[depth - 1].routine].calleeCount, entry);
  Routine* rt = &routines[r];
  rt->entries++;
  State st = {cpu->dp, cpu->db, cpu->e || cpu->mf, cpu->e || cpu->xf};
  bool seen = false;
  for(int i = 0; i < rt->stateCount; i++) {
    State* o = &rt->states[i];
    if(o->d == st.d && o->db == st.db && o->m8 == st.m8 && o->x8 == st.x8) seen = true;
  }
  if(!seen && rt->stateCount < MAX_STATES) rt->states[rt->stateCount++] = st;
  frames[depth++] = (Frame){r, cpu->sp, false};
}

void census_on_insn(Cpu* cpu) {
  while(depth > 0 && cpu->sp > frames[depth - 1].sp) {
    depth--;
    if(frames[depth].interrupted) routines[frames[depth].routine].interrupted++;
  }
  uint32_t pc = norm(((uint32_t)cpu->k << 16) | cpu->pc);
  if(cpu_interruptCount != lastInterrupts) {
    lastInterrupts = cpu_interruptCount;
    for(int i = 0; i < depth; i++) frames[i].interrupted = true;
    enter(cpu, pc, true);
    pendingCall = pendingIndirect = false;
  } else if(pendingCall) {
    enter(cpu, pc, false);
  } else if(pendingIndirect && depth > 0) {
    Routine* rt = &routines[frames[depth - 1].routine];
    addEdge(rt->indirect, &rt->indirectCount, pc);
  }
  pendingCall = pendingIndirect = false;
  uint8_t op = peek(((uint32_t)cpu->k << 16) | cpu->pc);
  if(op == 0x20 || op == 0x22 || op == 0xFC) pendingCall = true;          // JSR abs, JSL, JSR (abs,X)
  if(op == 0x6C || op == 0x7C || op == 0xDC) pendingIndirect = true;      // JMP (abs), JMP (abs,X), JML [abs]
}

static int hwClass(uint32_t adr) {
  uint8_t bank = (adr >> 16) & 0x7F;
  uint16_t a = adr & 0xFFFF;
  if(bank >= 0x30 && bank < 0x40 && a >= 0x8000) return -1;  // ROM read through the DSP-4 mapping
  if(bank >= 0x40) return 0;
  if(a < 0x2000 || a >= 0x8000) return 0;
  if(a >= 0x4202 && a <= 0x4206) return HW_MATH;
  if(a >= 0x4214 && a <= 0x4217) return HW_MATH;
  if(a >= 0x2140 && a <= 0x2143) return HW_APU;
  if(a >= 0x2100 && a <= 0x213F) return HW_PPU;
  if(a == 0x420B || (a >= 0x4300 && a <= 0x437F)) return HW_DMA;
  if(a == 0x420C) return HW_HDMA;
  if(a >= 0x4016 && a <= 0x421F) return HW_IO;
  return HW_OTHER;
}

void census_on_access(uint32_t adr, bool write) {
  (void)write;
  if(depth == 0) return;
  uint8_t bank = (adr >> 16) & 0x7F;
  int c = (bank >= 0x30 && bank < 0x40 && (adr & 0xFFFF) >= 0x8000) ? HW_DSP4 : hwClass(adr);
  if(c > 0) routines[frames[depth - 1].routine].hw |= (uint8_t)c;
}

void census_start(Snes* s, const uint8_t* r) {
  snes = s;
  rom = r;
  lastInterrupts = cpu_interruptCount;
}

static int byEntry(const void* a, const void* b) {
  uint32_t x = ((const Routine*)a)->entry, y = ((const Routine*)b)->entry;
  return x < y ? -1 : x > y;
}

bool census_save(const char* path) {
  FILE* f = fopen(path, "w");
  if(f == NULL) { perror(path); return false; }
  qsort(routines, routineCount, sizeof routines[0], byEntry);
  fprintf(f, "# Routine census from the bench (--census); one routine per line:\n"
             "# entry kind entries interrupted hw=<classes> states=<D/DB/widths,...> calls=<entries> indirect=<targets>\n");
  for(int i = 0; i < routineCount; i++) {
    Routine* r = &routines[i];
    char hw[64] = "";
    for(int b = 0; b < 8; b++) if(r->hw & (1 << b)) { if(hw[0]) strcat(hw, ","); strcat(hw, hwNames[b]); }
    fprintf(f, "%06X %s %lu %lu hw=%s states=", r->entry, r->isInterrupt ? "irq" : "sub", r->entries, r->interrupted, hw[0] ? hw : "-");
    for(int k = 0; k < r->stateCount; k++) {
      State* s = &r->states[k];
      fprintf(f, "%s%04X/%02X/%s%s", k ? "," : "", s->d, s->db, s->m8 ? "m8" : "m16", s->x8 ? "x8" : "x16");
    }
    fprintf(f, " calls=");
    for(int k = 0; k < r->calleeCount; k++) fprintf(f, "%s%06X", k ? "," : "", r->callees[k]);
    if(r->calleeCount == 0) fprintf(f, "-");
    fprintf(f, " indirect=");
    for(int k = 0; k < r->indirectCount; k++) fprintf(f, "%s%06X", k ? "," : "", r->indirect[k]);
    if(r->indirectCount == 0) fprintf(f, "-");
    fprintf(f, "\n");
  }
  return fclose(f) == 0;
}
