// SPDX-License-Identifier: GPL-3.0-or-later
// Per-routine differential test (see difftest.h).
//
// Live samples: when the CPU reaches a registered entry, WRAM and the registers are
// copied; the original runs on to its return (an interrupt inside discards the sample,
// a hardware access fails it, since the C cannot reproduce it yet); then the C runs from
// the copy and WRAM (minus the stack bytes the routine pushed), registers and flags must
// match. Fuzz samples do the same from synthetic inputs, run on the interpreter after the
// movie, so every branch is reached. A routine passes only if every instruction start in
// its extent ran in a compared sample.
#include "difftest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coverage.h"
#include "opflags.h"
#include "cpu.h"
#include "dma.h"
#include "ppu.h"
#include "routines.h"
#include "tg3000.h"

uint8_t g_ram[TG3000_WRAM_SIZE];
const uint8_t* g_rom;

#define MAX_SAMPLES 4000     // compared live samples per routine
#define SAMPLE_STRIDE 3      // take one entry in N
#define MAX_EXTENT 0x400
#define MAX_CONTRACTS 8
#define MAX_INSNS 200000     // a sample that runs longer is abandoned
#define WATCH_CAP 400        // flag liveness: instructions watched after a return
#define MIN_WATCHED 100      // fewer watched returns than this: every flag counts as live

enum { FN = 8, FV = 4, FZ = 2, FC = 1 };
static const char flagNames[4] = {'N', 'V', 'Z', 'C'};
static const uint8_t flagBits[4] = {FN, FV, FZ, FC};

typedef struct {
  uint16_t d;
  uint8_t db;
  bool m8, x8, e;
} Contract;

typedef struct {
  unsigned long entries, samples, pass, fail, interrupted, abandoned, fuzzPass, fuzzFail;
  bool contractBroken;
  Contract contracts[MAX_CONTRACTS];
  int contractCount;
  bool starts[MAX_EXTENT], visited[MAX_EXTENT];
  char firstFailure[256];
  // flags: compared separately; a mismatch only fails the routine if the flag is live
  unsigned long watched, live[4], dead[4], flagMismatch[4];
  char firstFlagFailure[4][160];
} Stats;

static Snes* snes;
static const Routine* table;
static int tableCount;
static Stats* stats;

// the sample in progress
static int active = -1;
static bool fuzzing;
static uint8_t entryRam[TG3000_WRAM_SIZE], afterRam[TG3000_WRAM_SIZE];
static Regs entryRegs;
static uint16_t entrySp, minSp, retPc;
static uint8_t retBank;
static uint32_t interruptsAtEntry, insnsInSample;
static bool hwAccess, mathWritten, leftExtent;
static uint32_t hwAccessAdr, leftExtentPc;
static const char* hwWhy;

// hardware effects of the sample: from the original's DMA kicks, and from the C's calls
#define MAX_EFFECTS 32
typedef struct { int kind; uint32_t target, src, size; } Effect;
static Effect origEffects[MAX_EFFECTS], cEffects[MAX_EFFECTS];
static int origEffectCount, cEffectCount;
static bool collectingC;

void difftest_effect(int kind, uint32_t target, uint32_t src, uint32_t size) {
  if(!collectingC || cEffectCount >= MAX_EFFECTS) return;
  cEffects[cEffectCount++] = (Effect){kind, target, src, size};
}

static const char* effectName(int kind) {
  return kind == EFFECT_VRAM ? "VRAM upload" : kind == EFFECT_CGRAM ? "CGRAM upload" : "other DMA";
}

// A write of `val` to MDMAEN ($420B): one effect per channel started, read from the
// channel's registers and the PPU's address state as they are at the kick.
static void recordKick(uint8_t val) {
  for(int c = 0; c < 8; c++) {
    if(!(val & (1 << c)) || origEffectCount >= MAX_EFFECTS) continue;
    DmaChannel* ch = &snes->dma->channel[c];
    Ppu* ppu = snes->ppu;
    uint32_t src = ((uint32_t)ch->aBank << 16) | ch->aAdr;
    uint32_t size = ch->size ? ch->size : 0x10000;
    bool plain = !ch->fromB && !ch->fixed && !ch->decrement;
    Effect e = {EFFECT_RAW_DMA, ((uint32_t)ch->bAdr << 8) | ch->mode, src, size};
    if(plain && ch->bAdr == 0x18 && ch->mode == 1 && ppu->vramIncrementOnHigh && ppu->vramIncrement == 1 &&
       ppu->vramRemapMode == 0) {
      e = (Effect){EFFECT_VRAM, ppu->vramPointer, src, size};
    } else if(plain && ch->bAdr == 0x22 && (ch->mode == 0 || ch->mode == 2) && !ppu->cgramSecondWrite) {
      e = (Effect){EFFECT_CGRAM, ppu->cgramPointer, src, size};
    }
    origEffects[origEffectCount++] = e;
  }
}

// Registers a DMA upload is set up through; on their own they have no visible effect, the
// kick turns them into one.
static bool isUploadSetup(uint32_t adr) {
  if(((adr >> 16) & 0x7F) >= 0x40) return false;
  uint16_t a = adr & 0xFFFF;
  return (a >= 0x4300 && a <= 0x437F) || a == 0x2116 || a == 0x2117 || a == 0x2121;
}

// flag-liveness watch after a live sample returns
static int watching = -1;
static uint8_t watchUnresolved;
static int watchCount;
static uint32_t watchInterrupts;
static int32_t watchPausedUntilSp = -1;

static uint32_t norm(uint32_t adr) {  // ROM banks $00-$3F/$40-$7D -> $80+ mirror
  uint8_t bank = adr >> 16;
  if(bank < 0x7E) bank |= 0x80;
  return ((uint32_t)bank << 16) | (adr & 0xFFFF);
}

static Regs readRegs(const Cpu* cpu) {
  Regs r = {cpu->a, cpu->x, cpu->y, cpu->dp, cpu->db, cpu->n, cpu->v, cpu->z, cpu->c,
            cpu->e || cpu->mf, cpu->e || cpu->xf, cpu->e};
  return r;
}

static bool isWram(uint32_t adr) {
  uint8_t bank = adr >> 16;
  if(bank == 0x7E || bank == 0x7F) return true;
  return (bank & 0x7F) < 0x40 && (adr & 0xFFFF) < 0x2000;
}

static bool isRom(uint32_t adr) {
  uint8_t bank = (adr >> 16) & 0x7F;
  return bank < 0x7E && ((adr & 0xFFFF) >= 0x8000 || bank >= 0x40);
}

// CPU multiply/divide unit ($4202-$4206 operands, $4214-$4217 results): the C computes
// natively, so these accesses are part of the routine's own work, provided the result it
// reads was started inside the routine.
static bool isMathReg(uint32_t adr, bool write) {
  if(((adr >> 16) & 0x7F) >= 0x40) return false;
  uint16_t a = adr & 0xFFFF;
  return write ? (a >= 0x4202 && a <= 0x4206) : (a >= 0x4214 && a <= 0x4217);
}

static void onAccess(uint32_t adr, uint8_t val, bool write) {
  if(active < 0 || hwAccess) return;
  if(isWram(adr) || (!write && isRom(adr))) return;
  if(isMathReg(adr, write)) {
    if(write) { mathWritten = true; return; }
    if(mathWritten) return;
    hwWhy = "reads a multiply/divide result it did not start";
  } else if(write && isUploadSetup(adr)) {
    return;
  } else if(write && ((adr >> 16) & 0x7F) < 0x40 && (adr & 0xFFFF) == 0x420B) {
    recordKick(val);
    return;
  } else {
    hwWhy = "hardware access (not supported by this routine's C)";
  }
  hwAccess = true;
  hwAccessAdr = adr;
}

// Instruction starts of a routine's extent, by linear decode with the contract's widths.
// REP/SEP change the widths as the decode goes (the routines are straight-line enough for
// a linear sweep).
static void findStarts(const Routine* r, Stats* s) {
  uint32_t off = (((r->entry >> 16) & 0x7F) << 15) | (r->entry & 0x7FFF);
  uint32_t n = r->end - r->entry;
  bool m8 = !r->needM16, x8 = !r->needX16;
  for(uint32_t i = 0; i < n && i < MAX_EXTENT;) {
    s->starts[i] = true;
    uint8_t op = g_rom[off + i], arg = g_rom[off + i + 1];
    i += coverage_insn_length(op, m8, x8);
    if(op == 0xC2) { if(arg & 0x20) m8 = false; if(arg & 0x10) x8 = false; }
    if(op == 0xE2) { if(arg & 0x20) m8 = true; if(arg & 0x10) x8 = true; }
  }
}

static void noteContract(Stats* s, const Routine* r, const Regs* in) {
  Contract c = {in->d, in->db, in->m8, in->x8, in->e};
  if(in->e || (r->needM16 && in->m8) || (r->needX16 && in->x8)) s->contractBroken = true;
  if(r->lowWramDb && !((in->db & 0x7F) < 0x40 || in->db == 0x7E)) s->contractBroken = true;
  if(r->fixedD && in->d != DP) s->contractBroken = true;
  if(r->db != 0 && in->db != r->db) s->contractBroken = true;
  for(int i = 0; i < s->contractCount; i++) {
    Contract* o = &s->contracts[i];
    if(o->d == c.d && o->db == c.db && o->m8 == c.m8 && o->x8 == c.x8 && o->e == c.e) return;
  }
  if(s->contractCount < MAX_CONTRACTS) s->contracts[s->contractCount++] = c;
}

// Run the C from the recorded entry state and compare with the recorded result.
static bool compareWithC(int idx, const Regs* after, char* why, size_t whyLen, uint8_t* flagDiff) {
  const Routine* r = &table[idx];
  memcpy(g_ram, entryRam, sizeof g_ram);
  Regs out = entryRegs;
  cEffectCount = 0;
  collectingC = true;
  r->call(&entryRegs, &out);
  collectingC = false;
  for(int i = 0; i < origEffectCount || i < cEffectCount; i++) {
    if(i >= origEffectCount || i >= cEffectCount) {
      snprintf(why, whyLen, "%d hardware effect(s) in the original, %d from the C", origEffectCount, cEffectCount);
      return false;
    }
    Effect* o = &origEffects[i];
    Effect* c = &cEffects[i];
    if(o->kind != c->kind || o->target != c->target || o->src != c->src || o->size != c->size) {
      snprintf(why, whyLen, "effect %d: original %s $%X <- $%06X x%u, C %s $%X <- $%06X x%u", i,
        effectName(o->kind), o->target, o->src, o->size, effectName(c->kind), c->target, c->src, c->size);
      return false;
    }
  }
  for(uint32_t i = 0; i < TG3000_WRAM_SIZE; i++) {
    if(i > minSp && i <= entrySp) continue;  // bytes the routine pushed below its entry SP
    if(g_ram[i] != afterRam[i]) {
      snprintf(why, whyLen, "WRAM $%05X: original %02X, C %02X", i, afterRam[i], g_ram[i]);
      return false;
    }
  }
#define CHECK(field, enabled) \
  if((enabled) && out.field != after->field) { \
    snprintf(why, whyLen, #field ": original %X, C %X", (unsigned)after->field, (unsigned)out.field); \
    return false; \
  }
  CHECK(a, r->compareA) CHECK(x, r->compareX) CHECK(y, r->compareY) CHECK(d, true) CHECK(db, true)
#undef CHECK
  *flagDiff = (out.n != after->n ? FN : 0) | (out.v != after->v ? FV : 0) |
              (out.z != after->z ? FZ : 0) | (out.c != after->c ? FC : 0);
  return true;
}

static void finishSample(const Cpu* cpu) {
  int idx = active;
  Stats* s = &stats[idx];
  active = -1;
  memcpy(afterRam, snes->ram, sizeof afterRam);
  Regs after = readRegs(cpu);
  if(cpu_interruptCount != interruptsAtEntry) { s->interrupted++; return; }
  char why[200] = "";
  bool ok;
  uint8_t flagDiff = 0;
  if(hwAccess) {
    snprintf(why, sizeof why, "%s at $%06X", hwWhy, hwAccessAdr);
    ok = false;
  } else if(leftExtent) {
    snprintf(why, sizeof why, "ran outside its declared extent at $%06X", leftExtentPc);
    ok = false;
  } else {
    ok = compareWithC(idx, &after, why, sizeof why, &flagDiff);
  }
  for(int f = 0; f < 4; f++) {
    if(!(flagDiff & flagBits[f])) continue;
    s->flagMismatch[f]++;
    if(s->firstFlagFailure[f][0] == '\0') {
      snprintf(s->firstFlagFailure[f], sizeof s->firstFlagFailure[f], "%s%c: original %d, C %d (D=%04X A=%04X)",
        fuzzing ? "fuzz " : "", flagNames[f], (int)((f == 0 ? after.n : f == 1 ? after.v : f == 2 ? after.z : after.c)),
        (int)!((f == 0 ? after.n : f == 1 ? after.v : f == 2 ? after.z : after.c)), entryRegs.d, entryRegs.a);
    }
  }
  if(!fuzzing && watching < 0) {  // watch which flags the caller goes on to read
    watching = idx;
    watchUnresolved = FN | FV | FZ | FC;
    watchCount = 0;
    watchInterrupts = cpu_interruptCount;
    watchPausedUntilSp = -1;
    s->watched++;
  }
  if(fuzzing) { if(ok) s->fuzzPass++; else s->fuzzFail++; }
  else { s->samples++; if(ok) s->pass++; else s->fail++; }
  if(!ok && s->firstFailure[0] == '\0') {
    snprintf(s->firstFailure, sizeof s->firstFailure, "%s%s at D=%04X A=%04X: %s",
      fuzzing ? "fuzz " : "", table[idx].name, entryRegs.d, entryRegs.a, why);
  }
}

static void startSample(int idx, const Cpu* cpu) {
  const Routine* r = &table[idx];
  active = idx;
  memcpy(entryRam, snes->ram, sizeof entryRam);
  entryRegs = readRegs(cpu);
  entrySp = minSp = cpu->sp;
  uint16_t sp = cpu->sp;
  retPc = (uint16_t)((snes->ram[(uint16_t)(sp + 1)] | (snes->ram[(uint16_t)(sp + 2)] << 8)) + 1);
  retBank = r->rtl ? snes->ram[(uint16_t)(sp + 3)] : cpu->k;
  interruptsAtEntry = cpu_interruptCount;
  insnsInSample = 0;
  hwAccess = mathWritten = leftExtent = false;
  origEffectCount = 0;
  noteContract(&stats[idx], r, &entryRegs);
}

static uint8_t peekByte(uint32_t adr) {
  if(isRom(adr)) return g_rom[(((adr >> 16) & 0x7F) << 15 | (adr & 0x7FFF)) & 0xFFFFF];
  if(isWram(adr)) {
    uint8_t bank = adr >> 16;
    return snes->ram[(bank == 0x7F ? 0x10000 : 0) | (bank == 0x7E || bank == 0x7F ? (adr & 0xFFFF) : (adr & 0x1FFF))];
  }
  return 0xEA;  // unknown: treat as NOP (reads and writes no flag)
}

static void resolveFlags(uint8_t mask, bool live) {
  Stats* s = &stats[watching];
  for(int f = 0; f < 4; f++) {
    if(!(watchUnresolved & mask & flagBits[f])) continue;
    if(live) s->live[f]++; else s->dead[f]++;
    watchUnresolved &= ~flagBits[f];
  }
  if(watchUnresolved == 0) watching = -1;
}

// One instruction of the caller after a live sample returned: which flags does it read
// before writing them? Interrupt handlers are skipped (they restore P with RTI).
static void watchStep(const Cpu* cpu) {
  if(cpu_interruptCount != watchInterrupts) {
    watchInterrupts = cpu_interruptCount;
    if(watchPausedUntilSp < 0) watchPausedUntilSp = (uint16_t)(cpu->sp + (cpu->e ? 3 : 4));
  }
  if(watchPausedUntilSp >= 0) {
    if(cpu->sp != (uint16_t)watchPausedUntilSp) return;
    watchPausedUntilSp = -1;
  }
  if(++watchCount > WATCH_CAP) { resolveFlags(FN | FV | FZ | FC, true); return; }
  uint32_t pc = ((uint32_t)cpu->k << 16) | cpu->pc;
  uint8_t op = peekByte(pc);
  uint8_t reads = opFlagsRead[op], writes = opFlagsWritten[op];
  if(op == 0xC2 || op == 0xE2) {
    uint8_t p = peekByte((pc & 0xFF0000) | (uint16_t)(pc + 1));
    writes = (p & 0x80 ? FN : 0) | (p & 0x40 ? FV : 0) | (p & 0x02 ? FZ : 0) | (p & 0x01 ? FC : 0);
  }
  if(reads) resolveFlags(reads, true);
  if(watching >= 0 && writes) resolveFlags(writes, false);
}

void difftest_on_insn(Cpu* cpu) {
  uint32_t pc = norm(((uint32_t)cpu->k << 16) | cpu->pc);
  if(active >= 0) {
    const Routine* r = &table[active];
    if(pc >= r->entry && pc < r->end) stats[active].visited[pc - r->entry] = true;
    if(cpu->sp < minSp) minSp = cpu->sp;
    uint16_t back = r->rtl ? 3 : 2;
    if(cpu->pc == retPc && cpu->k == retBank && cpu->sp == (uint16_t)(entrySp + back)) {
      finishSample(cpu);
    } else {
      // at the routine's own stack depth, every instruction must lie inside its extent
      if(cpu->sp == entrySp && (pc < r->entry || pc >= r->end) && !leftExtent) {
        leftExtent = true;
        leftExtentPc = pc;
      }
      if(++insnsInSample > MAX_INSNS) {
        stats[active].abandoned++;
        active = -1;
      }
      if(watching >= 0) watchStep(cpu);
      return;
    }
  }
  if(watching >= 0) watchStep(cpu);
  if(fuzzing) return;
  for(int i = 0; i < tableCount; i++) {
    if(table[i].entry != pc) continue;
    Stats* s = &stats[i];
    if(s->entries++ % SAMPLE_STRIDE != 0 || s->samples >= MAX_SAMPLES) return;
    if(cpu->sp >= 0x2000) return;  // stack outside WRAM: not handled
    startSample(i, cpu);
    stats[i].visited[0] = true;
    return;
  }
}

void difftest_init(Snes* s, const uint8_t* rom, bool negative) {
  snes = s;
  g_rom = rom;
  table = negative ? negativeRoutines : routines;
  tableCount = negative ? negativeRoutineCount : routineCount;
  stats = calloc(tableCount, sizeof *stats);
  for(int i = 0; i < tableCount; i++) findStarts(&table[i], &stats[i]);
  snes_cpuAccessHook = onAccess;
}

// Fuzz: synthetic inputs, run on the interpreter from the entry, interrupts disabled.
static uint32_t fuzzSeed;
static uint32_t fuzzRandom(void) {
  fuzzSeed = fuzzSeed * 1103515245u + 12345u;
  return (fuzzSeed >> 16) | ((fuzzSeed * 1103515245u + 12345u) & 0xFFFF0000u);
}

void difftest_fuzz(int iterations) {
  fuzzSeed = 0x13579BDF;
  fuzzing = true;
  snes->nmiEnabled = false;
  snes->hIrqEnabled = snes->vIrqEnabled = false;
  for(int i = 0; i < tableCount; i++) {
    const Routine* r = &table[i];
    Stats* s = &stats[i];
    if(r->fuzz == NULL || s->contractCount == 0) continue;
    for(int k = 0; k < iterations; k++) {
      Contract c = s->contracts[k % s->contractCount];
      Regs in = {(uint16_t)fuzzRandom(), (uint16_t)fuzzRandom(), (uint16_t)fuzzRandom(), c.d, c.db,
                 fuzzRandom() & 1, fuzzRandom() & 1, fuzzRandom() & 1, fuzzRandom() & 1, c.m8, c.x8, false};
      if(in.x8) { in.x &= 0xFF; in.y &= 0xFF; }
      r->fuzz(&in, snes->ram, fuzzRandom);
      Cpu* cpu = snes->cpu;
      cpu->nmiWanted = cpu->irqWanted = cpu->intWanted = false;
      cpu->i = true;
      cpu->e = false;
      cpu->mf = in.m8; cpu->xf = in.x8;
      cpu->a = in.a; cpu->x = in.x; cpu->y = in.y; cpu->dp = in.d; cpu->db = in.db;
      cpu->n = in.n; cpu->v = in.v; cpu->z = in.z; cpu->c = in.c;
      // sentinel return to $8000 of the entry bank (a BRA * trap), stack at $1F00
      if(r->rtl) {
        snes->ram[0x1F00] = r->entry >> 16;
        snes->ram[0x1EFF] = 0x7F; snes->ram[0x1EFE] = 0xFF;
        cpu->sp = 0x1EFD;
      } else {
        snes->ram[0x1F00] = 0x7F; snes->ram[0x1EFF] = 0xFF;
        cpu->sp = 0x1EFE;
      }
      cpu->k = r->entry >> 16;
      cpu->pc = r->entry & 0xFFFF;
      startSample(i, cpu);
      s->visited[0] = true;
      for(int n = 0; n < MAX_INSNS && active >= 0; n++) {
        cpu_runOpcode(cpu);  // the bench's trace hook calls difftest_on_insn
      }
      if(active >= 0) { s->abandoned++; active = -1; }
    }
  }
  fuzzing = false;
}

// What a "verified" row vouches for: the routine's C source, the registry (adapters,
// extents, contracts) and this harness. A change to any of them makes the row stale.
static uint64_t verificationHash(const char* source) {
  const char* files[4] = {source, "tools/bench/routines.c", "tools/bench/difftest.c", "tools/bench/platform_bench.c"};
  uint64_t h = 0xcbf29ce484222325ull;
  for(int i = 0; i < 4; i++) {
    FILE* f = fopen(files[i], "rb");
    if(f == NULL) return 0;
    int ch;
    while((ch = fgetc(f)) != EOF) { h ^= (uint8_t)ch; h *= 0x100000001b3ull; }
    fclose(f);
  }
  return h;
}

bool difftest_report(const char* resultsPath) {
  bool allOk = true;
  // keep earlier rows for routines this run did not reach
  char kept[64][256];
  int keptCount = 0;
  if(resultsPath != NULL) {
    FILE* in = fopen(resultsPath, "r");
    char line[256];
    while(in != NULL && fgets(line, sizeof line, in) && keptCount < 64) {
      unsigned entry;
      if(line[0] == '#' || sscanf(line, "%x", &entry) != 1) continue;
      for(int i = 0; i < tableCount; i++) {
        if(table[i].entry == entry && stats[i].entries == 0 && stats[i].fuzzPass + stats[i].fuzzFail == 0) {
          snprintf(kept[keptCount++], sizeof kept[0], "%s", line);
        }
      }
    }
    if(in != NULL) fclose(in);
  }
  FILE* out = NULL;
  if(resultsPath != NULL) out = fopen(resultsPath, "w");
  if(out != NULL) fprintf(out, "# entry name source fnv1a64(source+routines.c+difftest.c+platform_bench.c) live fuzz paths status live-flags  -- written by the bench (--difftest)\n");
  for(int i = 0; i < tableCount; i++) {
    const Routine* r = &table[i];
    Stats* s = &stats[i];
    int need = 0, seen = 0;
    for(int k = 0; k < MAX_EXTENT; k++) if(s->starts[k]) { need++; if(s->visited[k]) seen++; }
    bool reached = s->entries > 0 || s->fuzzPass + s->fuzzFail > 0;
    char liveFlags[5] = "", flagNote[64] = "";
    bool flagsOk = true;
    for(int f = 0, n = 0; f < 4; f++) {
      bool live = s->live[f] > 0 || s->watched < MIN_WATCHED;
      if(live) liveFlags[n++] = flagNames[f];
      if(live && s->flagMismatch[f] > 0) flagsOk = false;
    }
    if(liveFlags[0] == '\0') strcpy(liveFlags, "-");
    snprintf(flagNote, sizeof flagNote, "flags live %s (of %lu returns watched)", liveFlags, s->watched);
    bool ok = flagsOk && s->fail == 0 && s->fuzzFail == 0 && (s->pass + s->fuzzPass) > 0 && seen == need && !s->contractBroken;
    const char* status = !reached ? "unreached" : ok ? "verified" : "failed";
    if(reached) allOk &= ok;  // a routine this run never reached is reported, not failed
    printf("difftest %-16s $%06X: %s  live %lu/%lu pass (%lu entries, %lu interrupted, %lu abandoned), fuzz %lu/%lu, paths %d/%d%s\n",
      r->name, r->entry, !reached ? "UNREACHED" : ok ? "PASS" : "FAIL", s->pass, s->samples, s->entries, s->interrupted, s->abandoned,
      s->fuzzPass, s->fuzzPass + s->fuzzFail, seen, need, s->contractBroken ? ", ENTRY CONTRACT BROKEN" : "");
    for(int c = 0; c < s->contractCount; c++) {
      Contract* k = &s->contracts[c];
      printf("    entry state: D=%04X DB=%02X %s %s%s\n", k->d, k->db, k->m8 ? "m8" : "m16", k->x8 ? "x8" : "x16", k->e ? " emu" : "");
    }
    printf("    %s; mismatches N%lu V%lu Z%lu C%lu\n", flagNote, s->flagMismatch[0], s->flagMismatch[1], s->flagMismatch[2], s->flagMismatch[3]);
    if(s->firstFailure[0]) printf("    first failure: %s\n", s->firstFailure);
    for(int f = 0; f < 4; f++) {
      bool live = s->live[f] > 0 || s->watched < MIN_WATCHED;
      if(live && s->firstFlagFailure[f][0]) printf("    first live-flag failure: %s\n", s->firstFlagFailure[f]);
    }
    bool keptRow = false;
    for(int k = 0; k < keptCount; k++) {
      unsigned entry;
      if(sscanf(kept[k], "%x", &entry) == 1 && entry == r->entry) {
        if(out != NULL) fputs(kept[k], out);
        keptRow = true;
      }
    }
    if(out != NULL && !keptRow) {
      fprintf(out, "%06X %s %s %016llx %lu %lu %d/%d %s live-flags:%s\n", r->entry, r->name, r->source,
        (unsigned long long)verificationHash(r->source), s->pass, s->fuzzPass, seen, need, status, liveFlags);
    }
  }
  if(out != NULL) fclose(out);
  return allOk;
}
