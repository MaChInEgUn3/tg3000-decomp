// SPDX-License-Identifier: GPL-3.0-or-later
// HDMA measurement: per frame, the configuration of every channel enabled in $420C
// (B-bus target, mode, indirect, table address); the instruction addresses that write
// $420C, the channel registers $43x0-$43xA and $2105 (BG mode: in vblank or while
// drawing, and whether a write while drawing changes the mode); and, for tables in WRAM,
// which instructions last wrote their bytes, walking each table to its terminator. CPU
// writes only: a table filled by DMA shows no writer. On the two-task screens (IRQ
// $80:BC9B) it also logs where each task was switched out and the WRAM and registers
// each task touches.
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
static Set taskRegs;
static Set handlers, handlerWrites, handlerCalls, kickInherits, tableRaces, sharedTargets, dispatches, switchOuts;

// Interrupt handlers: the vectors jump to trampolines at $0000 (NMI) and $0004 (IRQ) in
// WRAM, which JML to the current handler. One record per handler instance in progress.
typedef struct {
  bool active;
  uint32_t target, entrySp, insns;
  uint32_t interrupted;  // the PC the interrupt was taken at (from its frame)
  uint8_t written[8];  // $43x0-$43x7 written by this instance (bit per register)
} Handler;
static Handler current[2];  // [0] NMI, [1] IRQ (an IRQ inside the NMI gets its own)
static int handlerDepth;
static uint32_t prevPc;
static bool afterCall, afterIndirect;
static uint8_t* tableByte;  // bytes of enabled tables and their indirect data, last frame

// Two-task screens (the IRQ trampoline points at $80:BC9B): outside the handlers, the
// running task is told by its stack (task 2's stack is seeded at $1D8C). Per WRAM byte:
// which task wrote it (bits 0-1), which read it (bits 2-3), and the first PC of each.
static Cpu* cpuNow;
static uint8_t* taskUse;
static uint32_t* taskPc;  // [w * 2 + task - 1]: first PC of that task touching w
static uint16_t taskSpMin[3] = {0, 0xFFFF, 0xFFFF}, taskSpMax[3];
static int runningTask(void) {
  if(handlerDepth > 0 || cpuNow == NULL) return 0;
  if(snes->ram[0x0005] != 0x9B || snes->ram[0x0006] != 0xBC) return 0;
  return cpuNow->sp >= 0x1E00 ? 1 : 2;
}

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
  tableByte = calloc(0x20000, 1);
  taskUse = calloc(0x20000, 1);
  taskPc = calloc(0x40000, sizeof *taskPc);
}

static int wramIndex(uint32_t adr);

static uint8_t peek(uint32_t adr) {  // ROM or WRAM, without side effects (else 0)
  int w = wramIndex(adr);
  if(w >= 0) return snes->ram[w];
  uint8_t bank = (adr >> 16) & 0x7F;
  if(bank < 0x7E && ((adr & 0xFFFF) >= 0x8000 || bank >= 0x40))
    return snes->cart->rom[((uint32_t)bank << 15 | (adr & 0x7FFF)) & 0xFFFFF];
  return 0;
}

void hdma_on_insn(Cpu* cpu) {
  cpuNow = cpu;
  {
    int t = runningTask();
    if(t) {
      if(cpu->sp < taskSpMin[t]) taskSpMin[t] = cpu->sp;
      if(cpu->sp > taskSpMax[t]) taskSpMax[t] = cpu->sp;
    }
  }
  uint8_t bank = cpu->k;
  if(bank < 0x7E) bank |= 0x80;
  pc = (uint32_t)bank << 16 | cpu->pc;
  // the first instruction after a trampoline is the handler's
  if(prevPc == 0x800000 || prevPc == 0x800004) {
    if(handlerDepth == 2) { current[0] = current[1]; handlerDepth = 1; }  // never blocks a new entry
    Handler* h = &current[handlerDepth++];
    uint16_t sp = cpu->sp;
    uint8_t pb = snes->ram[(uint16_t)(sp + 4)];
    if(pb < 0x7E) pb |= 0x80;
    uint32_t from = (uint32_t)pb << 16 | (uint16_t)(snes->ram[(uint16_t)(sp + 2)] | snes->ram[(uint16_t)(sp + 3)] << 8);
    *h = (Handler){true, pc, (uint32_t)(uint16_t)(cpu->sp + (cpu->e ? 3 : 4)), 0, from, {0}};
    note(&handlers, pc, prevPc == 0x800000 ? 0 : 1, 0, 0);
    // the two-task screens: the mid-frame IRQ $80:BC9B switches task 1 out
    if(pc == 0x80BC9B) note(&switchOuts, 1, from, 0, 0);
  }
  if(afterCall && handlerDepth > 0) note(&handlerCalls, current[handlerDepth - 1].target, pc, 0, 0);
  if(afterIndirect && handlerDepth > 0) {
    note(&dispatches, current[handlerDepth - 1].target, pc, prevPc, 0);
    // ... and the NMI target $80:B97B switches task 2 out
    if(pc == 0x80B97B) note(&switchOuts, 2, current[handlerDepth - 1].interrupted, 0, 0);
  }
  afterCall = afterIndirect = false;
  if(handlerDepth > 0) {
    Handler* h = &current[handlerDepth - 1];
    h->insns++;
    uint8_t opcode = peek(((uint32_t)cpu->k << 16) | cpu->pc);
    if(opcode == 0x20 || opcode == 0x22 || opcode == 0xFC) afterCall = true;  // JSR, JSL, JSR (abs,X)
    if(opcode == 0x6C || opcode == 0x7C || opcode == 0xDC || opcode == 0xFC) afterIndirect = true;
    if(opcode == 0x40) {  // RTI: the innermost handler instance ends (also when it switched stacks)
      Handler* t = &current[handlerDepth - 1];
      note(&handlers, t->target, 2, t->insns, 0);  // size: one record per distinct length
      t->active = false;
      handlerDepth--;
    }
  }
  prevPc = pc;
}

static int wramIndex(uint32_t adr) {
  uint8_t bank = adr >> 16;
  if(bank == 0x7E || bank == 0x7F) return (int)(((uint32_t)(bank - 0x7E) << 16) | (adr & 0xFFFF));
  if((bank & 0x7F) < 0x40 && (adr & 0xFFFF) < 0x2000) return (int)(adr & 0x1FFF);
  return -1;
}

void hdma_on_access(uint32_t adr, uint8_t val, bool write) {
  int t = runningTask();
  if(t) {
    int u = wramIndex(adr);
    if(u >= 0) {
      uint8_t bit = (uint8_t)(1u << ((t - 1) + (write ? 0 : 2)));
      if(!(taskUse[u] & (0x5u << (t - 1)))) taskPc[u * 2 + t - 1] = pc;
      taskUse[u] |= bit;
    } else if(((adr >> 16) & 0x7F) < 0x40) {
      uint16_t a = adr & 0xFFFF;
      if((a >= 0x2100 && a < 0x2200) || (a >= 0x4200 && a < 0x4380)) note(&taskRegs, (uint32_t)t, a, write, pc);
    }
  }
  if(!write) return;
  int w = wramIndex(adr);
  if(w >= 0) {
    writer[w] = pc;
    if(tableByte[w] && snes->vPos < 225) note(&tableRaces, (uint32_t)w, pc, tableByte[w] - 1, 0);
    return;
  }
  if(((adr >> 16) & 0x7F) >= 0x40) return;
  uint16_t a = adr & 0xFFFF;
  if(handlerDepth > 0) {
    Handler* h = &current[handlerDepth - 1];
    if((a >= 0x2100 && a < 0x2200) || (a >= 0x4200 && a < 0x4380)) note(&handlerWrites, h->target, a, 0, 0);
    if(a >= 0x4300 && a <= 0x437F && (a & 0xF) < 8) h->written[(a >> 4) & 7] |= (uint8_t)(1u << (a & 0xF));
    if(a == 0x420B) {  // a DMA kick: which of the used registers did this instance not set?
      for(int c = 0; c < 8; c++) {
        if(!(val & (1 << c))) continue;
        uint8_t inherited = (uint8_t)(~h->written[c] & 0x7F);  // $43x0-$43x6
        note(&kickInherits, h->target, (uint32_t)c, inherited, pc);
      }
    }
  }
  if(a == 0x420C) note(&enableWrites, pc, val, 0, 0);
  else if(a >= 0x4300 && a <= 0x437F && (a & 0xF) <= 0xA) note(&regWrites, (a >> 4) & 7, a & 0xF, pc, 0);
  else if(a == 0x2105) {
    // 0 = in vblank, 1 = while drawing with the same mode, 2 = while drawing, a new mode
    uint32_t when = snes->vPos >= 225 ? 0 : (val & 7) == (snes->ppu->mode & 7) ? 1 : 2;
    note(&modeWrites, val, pc, when, 0);
  }
}

void hdma_on_frame(int frame) {
  frames = (uint32_t)frame;
  memset(tableByte, 0, 0x20000);
  for(int c = 0; c < 8; c++) {
    DmaChannel* a = &snes->dma->channel[c];
    if(!a->hdmaActive) continue;
    for(int d = c + 1; d < 8; d++) {
      DmaChannel* b = &snes->dma->channel[d];
      if(b->hdmaActive && b->bAdr == a->bAdr) note(&sharedTargets, (uint32_t)c, (uint32_t)d, a->bAdr, 0);
    }
  }
  for(int c = 0; c < 8; c++) {
    DmaChannel* ch = &snes->dma->channel[c];
    if(!ch->hdmaActive) continue;
    uint32_t table = (uint32_t)ch->aBank << 16 | ch->aAdr;
    note(&configs, (uint32_t)c, (uint32_t)ch->bAdr << 8 | ch->mode, ch->indirect ? (0x100u | ch->indBank) : 0, table);
    int w = wramIndex(table);
    if(w < 0) continue;
    // bytes per line for the transfer mode (direct tables), 2 for an indirect pointer
    static const int unit[8] = {1, 2, 2, 4, 4, 4, 2, 4};
    int u = ch->indirect ? 2 : unit[ch->mode & 7];
    for(int k = 0, guard = 0; guard < 128; guard++) {
      uint8_t count = snes->ram[(w + k) & 0x1FFFF];
      int len = 1 + (count == 0 ? 0 : ch->indirect ? 2 : (count & 0x80) ? (count & 0x7F) * u : u);
      for(int b = 0; b < len; b++) {
        uint32_t who = writer[(w + k + b) & 0x1FFFF];
        if(who) note(&tableWriters, (uint32_t)c, table, who, 0);
        tableByte[(w + k + b) & 0x1FFFF] = (uint8_t)(c + 1);
      }
      if(ch->indirect && count != 0) {  // the data the entry points at, if in WRAM
        uint16_t ptr = (uint16_t)(snes->ram[(w + k + 1) & 0x1FFFF] | snes->ram[(w + k + 2) & 0x1FFFF] << 8);
        int d = wramIndex((uint32_t)ch->indBank << 16 | ptr);
        int n = (count & 0x80) ? (count & 0x7F) * unit[ch->mode & 7] : unit[ch->mode & 7];
        if(d >= 0) for(int b = 0; b < n; b++) tableByte[(d + b) & 0x1FFFF] = (uint8_t)(c + 1);
      }
      if(count == 0) break;
      k += len;
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
  static const char* const when[3] = {"in vblank", "while drawing, same mode", "while drawing, NEW MODE"};
  fprintf(f, "$2105 = $%02X by $%06X %s x%u (frames %u..%u)\n", it->key[0], it->key[1], when[it->key[2]],
    it->count, it->first, it->last);
}
static void rowHandler(FILE* f, Item* it) {
  if(it->key[1] == 2) fprintf(f, "  $%06X ran %u instructions x%u\n", it->key[0], it->key[2], it->count);
  else fprintf(f, "%s handler $%06X entered x%u (frames %u..%u)\n", it->key[1] ? "IRQ" : "NMI", it->key[0], it->count, it->first, it->last);
}
static void rowHandlerWrite(FILE* f, Item* it) { fprintf(f, "$%06X writes $%04X x%u\n", it->key[0], it->key[1], it->count); }
static void rowHandlerCall(FILE* f, Item* it) { fprintf(f, "$%06X calls $%06X x%u\n", it->key[0], it->key[1], it->count); }
static void rowInherit(FILE* f, Item* it) {
  fprintf(f, "$%06X kick at $%06X: ch%u inherits $43%u0-6 mask %02X (bit n = $43%un not written by this instance) x%u\n",
    it->key[0], it->key[3], it->key[1], it->key[1], it->key[2], it->key[1], it->count);
}
static void rowRace(FILE* f, Item* it) {
  fprintf(f, "WRAM $%05X (ch%u table or data) written while drawing by $%06X x%u (frames %u..%u)\n", it->key[0], it->key[2],
    it->key[1], it->count, it->first, it->last);
}
static void rowDispatch(FILE* f, Item* it) {
  fprintf(f, "$%06X jumps indirectly at $%06X to $%06X x%u (frames %u..%u)\n", it->key[0], it->key[2], it->key[1], it->count,
    it->first, it->last);
}
static void rowSwitch(FILE* f, Item* it) {
  fprintf(f, "task %u switched out at $%06X x%u (frames %u..%u)\n", it->key[0], it->key[1], it->count, it->first, it->last);
}
static void rowTaskReg(FILE* f, Item* it) {
  fprintf(f, "task %u %s $%04X at $%06X x%u\n", it->key[0], it->key[2] ? "writes" : "reads ", it->key[1], it->key[3], it->count);
}
static void rowShared(FILE* f, Item* it) { fprintf(f, "ch%u and ch%u both target $21%02X x%u\n", it->key[0], it->key[1], it->key[2], it->count); }
static void rowWriter(FILE* f, Item* it) { fprintf(f, "ch%u table $%06X written by $%06X (%u frame-bytes)\n", it->key[0], it->key[1], it->key[2], it->count); }

// WRAM both tasks touch, one of them writing: merged ranges, stacks left out.
static void dumpTaskShared(FILE* f) {
  fprintf(f, "\n# two-task screens: stacks task 1 $%04X-$%04X, task 2 $%04X-$%04X\n",
          taskSpMin[1], taskSpMax[1], taskSpMin[2], taskSpMax[2]);
  fprintf(f, "# WRAM both tasks use, at least one writing (W1/W2 written, R1/R2 read; first PC per task)\n");
  uint32_t touched[16] = {0};
  for(int w = 0; w < 0x20000; w++) touched[taskUse[w]]++;
  fprintf(f, "# bytes by use pattern (bit0 W1, bit1 W2, bit2 R1, bit3 R2):");
  for(int k = 1; k < 16; k++) fprintf(f, " %X:%u", k, touched[k]);
  fprintf(f, "\n");
  int start = -1;
  uint8_t kind = 0;
  for(int w = 0; w <= 0x20000; w++) {
    uint8_t u = w < 0x20000 ? taskUse[w] : 0;
    bool stack = w < 0x2000 && ((w >= taskSpMin[1] && w <= taskSpMax[1] + 3) || (w >= taskSpMin[2] && w <= taskSpMax[2] + 3));
    bool both = (u & 0x5) && (u & 0xA) && (u & 0x3) && !stack;
    if(both && start >= 0 && u == kind) continue;
    if(start >= 0) {
      fprintf(f, "$%s:%04X-$%04X %s%s%s%s  task1 $%06X task2 $%06X\n", start >= 0x10000 ? "7F" : "7E", start & 0xFFFF,
              (w - 1) & 0xFFFF, kind & 1 ? "W1 " : "", kind & 2 ? "W2 " : "", kind & 4 ? "R1 " : "", kind & 8 ? "R2 " : "",
              taskPc[start * 2], taskPc[start * 2 + 1]);
      start = -1;
    }
    if(both) { start = w; kind = u; }
  }
}

bool hdma_save(const char* path) {
  FILE* f = fopen(path, "w");
  if(f == NULL) { perror(path); return false; }
  fprintf(f, "# HDMA report over %u frames -- written by the bench (--hdma-report)\n", frames + 1);
  dump(f, "channel configurations seen at the end of a frame", &configs, rowConfig);
  dump(f, "$420C (HDMA enable) writers", &enableWrites, rowEnable);
  dump(f, "channel register writers", &regWrites, rowReg);
  dump(f, "$2105 (BG mode) writers", &modeWrites, rowMode);
  dump(f, "last CPU writers of the bytes of WRAM tables (to the terminator)", &tableWriters, rowWriter);
  dump(f, "interrupt handlers (entered through the $0000/$0004 trampolines) and their lengths", &handlers, rowHandler);
  dump(f, "indirect jumps inside handlers (their dispatch)", &dispatches, rowDispatch);
  dump(f, "hardware registers each handler writes", &handlerWrites, rowHandlerWrite);
  dump(f, "routines each handler calls (JSR/JSL)", &handlerCalls, rowHandlerCall);
  dump(f, "DMA kicks inside handlers: registers the kick uses but the instance did not write", &kickInherits, rowInherit);
  dump(f, "CPU writes while drawing into an enabled table or its indirect data", &tableRaces, rowRace);
  dump(f, "enabled channels sharing a target", &sharedTargets, rowShared);
  dump(f, "two-task screens: where each task was when the interrupt switched it out", &switchOuts, rowSwitch);
  dumpTaskShared(f);
  dump(f, "two-task screens: hardware registers a task touches (task, register, write, PC)", &taskRegs, rowTaskReg);
  fclose(f);
  return true;
}
