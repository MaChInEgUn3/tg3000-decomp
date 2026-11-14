// SPDX-License-Identifier: GPL-3.0-or-later
// Per-routine differential test (see difftest.h).
//
// Live samples: when the CPU reaches a registered entry (also inside another routine's
// sample, up to MAX_DEPTH deep), WRAM and the registers are copied; the original runs on to its return (an interrupt inside discards the sample,
// a hardware access fails it, since the C cannot reproduce it yet); then the C runs from
// the copy and WRAM (minus the stack bytes the routine pushed), registers and flags must
// match. Fuzz samples do the same from synthetic inputs, run on the interpreter after the
// movie, so every branch is reached. A routine passes only if every instruction start in
// its extent ran in a compared sample.
#include "difftest.h"

#include <setjmp.h>
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
#define MAX_EXTENT 0x600
#define MAX_CONTRACTS 8
#define MAX_INSNS 4000000    // a sample that runs longer is abandoned
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
  char firstFailure[400];
  // flags: compared separately; a mismatch only fails the routine if the flag is live
  unsigned long watched, live[4], dead[4], flagMismatch[4];
  char firstFlagFailure[4][160];
} Stats;

static Snes* snes;
static const Routine* table;
static int tableCount;
static Stats* stats;

// Samples in progress, innermost last: a registered routine called from inside another
// one's sample gets its own sample too (it would otherwise never be sampled once its
// callers are registered).
#define MAX_DEPTH 4
#define MAX_EFFECTS 32
typedef struct { int kind; uint32_t target, src, size; } Effect;

typedef struct {
  int idx;
  Regs entryRegs;
  uint16_t entrySp, minSp, retPc;
  uint8_t retBank;
  uint32_t interruptsAtEntry, insns;
  bool hwAccess, mathWritten, leftExtent, setupPending;
  uint32_t hwAccessAdr, leftExtentPc;
  const char* hwWhy;
  Effect effects[MAX_EFFECTS];  // hardware effects of the original, from its DMA kicks
  int effectCount;
} Sample;

static Sample samples[MAX_DEPTH];
static uint8_t sampleRam[MAX_DEPTH][TG3000_WRAM_SIZE];  // WRAM at each sample's entry
static int depth;
static bool fuzzing;
static uint8_t afterRam[TG3000_WRAM_SIZE];

// Interrupts inside the sample of a routine that waits for them (at most one such sample
// at a time): each one's WRAM before and after the handler, replayed in order by
// platform_wait_vblank() on the C side.
#define MAX_IRQS 64
static uint8_t irqPre[MAX_IRQS][TG3000_WRAM_SIZE], irqPost[MAX_IRQS][TG3000_WRAM_SIZE];
static int irqOwner = -1;  // depth of the sample that records them
static int irqCount, irqReplayed, irqLimit;
static bool irqActive, irqOverflow;
static uint16_t irqReturnSp;
static uint32_t irqSeen;
static jmp_buf cAbort;
static bool cWaitedTooLong;

static void applyIrq(int i) {
  for(uint32_t a = 0; a < TG3000_WRAM_SIZE; a++) {
    if(irqPre[i][a] != irqPost[i][a]) g_ram[a] = irqPost[i][a];
  }
}

void difftest_replay_interrupt(void) {
  if(irqReplayed >= irqLimit) {  // the C wants a frame the original never waited for
    cWaitedTooLong = true;
    longjmp(cAbort, 1);
  }
  applyIrq(irqReplayed++);
}

// The C's platform calls, recorded while it runs.
static Effect cEffects[MAX_EFFECTS];
static int cEffectCount;
static bool collectingC;

void difftest_effect(int kind, uint32_t target, uint32_t src, uint32_t size) {
  if(!collectingC || cEffectCount >= MAX_EFFECTS) return;
  cEffects[cEffectCount++] = (Effect){kind, target, src, size};
}

static const char* effectName(int kind) {
  return kind == EFFECT_VRAM ? "VRAM upload" : kind == EFFECT_CGRAM ? "CGRAM upload" : "other DMA";
}

// A write of `val` to MDMAEN ($420B): one effect per channel started, read from the
// channel's registers and the PPU's address state as they are at the kick, for every
// sample in progress (an inner routine's upload is also its caller's).
static void recordKick(uint8_t val) {
  for(int c = 0; c < 8; c++) {
    if(!(val & (1 << c))) continue;
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
    for(int k = 0; k < depth; k++) {
      Sample* smp = &samples[k];
      if(smp->effectCount < MAX_EFFECTS) smp->effects[smp->effectCount++] = e;
    }
  }
}

// Registers a DMA upload is set up through; on their own they have no visible effect, the
// kick turns them into one.
static bool isUploadSetup(uint32_t adr) {
  if(((adr >> 16) & 0x7F) >= 0x40) return false;
  uint16_t a = adr & 0xFFFF;
  return (a >= 0x4300 && a <= 0x437F) || a == 0x2116 || a == 0x2117 || a == 0x2121;
}

// Flag liveness: after a live sample of a routine returns, its watch follows the caller
// until each flag is read (live) or overwritten (dead). One watch per routine.
typedef struct {
  bool active;
  uint8_t unresolved;
  int count;
  uint32_t interrupts;
  int32_t pausedUntilSp;
} Watch;
static Watch* watches;

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

void difftest_on_access(uint32_t adr, uint8_t val, bool write) {
  if(depth == 0 || irqActive) return;  // a handler's hardware is its own
  if(isWram(adr) || (!write && isRom(adr))) return;
  if(write && ((adr >> 16) & 0x7F) < 0x40 && (adr & 0xFFFF) == 0x420B) {
    recordKick(val);
    for(int k = 0; k < depth; k++) samples[k].setupPending = false;
    return;
  }
  for(int k = 0; k < depth; k++) {
    Sample* smp = &samples[k];
    if(smp->hwAccess) continue;
    if(isMathReg(adr, write)) {
      if(write) { smp->mathWritten = true; continue; }
      if(smp->mathWritten) continue;
      smp->hwWhy = "reads a multiply/divide result it did not start";
    } else if(write && isUploadSetup(adr)) {
      smp->setupPending = true;
      continue;
    } else {
      smp->hwWhy = "hardware access (not supported by this routine's C)";
    }
    smp->hwAccess = true;
    smp->hwAccessAdr = adr;
  }
}

// Instruction starts of a routine's extent, by linear decode with the contract's widths.
// REP/SEP change the widths as the decode goes (the routines are straight-line enough for
// a linear sweep).
static void findStarts(const Routine* r, Stats* s) {
  uint32_t off = (((r->entry >> 16) & 0x7F) << 15) | (r->entry & 0x7FFF);
  uint32_t n = r->end - r->entry;
  bool m8 = !r->needM16, x8 = !r->needX16;
  for(uint32_t i = 0; i < n && i < MAX_EXTENT;) {
    bool skipped = false;
    for(int h = 0; h < 4; h++) {
      if(r->holes[h][1] && r->entry + i >= r->holes[h][0] && r->entry + i < r->holes[h][1]) {
        i = r->holes[h][1] - r->entry;
        skipped = true;
      }
    }
    if(skipped) continue;
    for(int h = 0; h < 2; h++) if(r->m8At[h] == r->entry + i) m8 = true;
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

// Run the C from sample k's entry state and compare with the original's result.
static bool compareWithC(int k, const Regs* after, char* why, size_t whyLen, uint8_t* flagDiff) {
  Sample* smp = &samples[k];
  const Routine* r = &table[smp->idx];
  memcpy(g_ram, sampleRam[k], sizeof g_ram);
  Regs out = smp->entryRegs;
  cEffectCount = 0;
  collectingC = true;
  irqReplayed = 0;
  irqLimit = irqOwner == k ? irqCount : 0;
  cWaitedTooLong = false;
  if(setjmp(cAbort) == 0) r->call(&smp->entryRegs, &out);
  collectingC = false;
  if(cWaitedTooLong) {
    snprintf(why, whyLen, "the C waited for more interrupts than the original's %d", irqLimit);
    return false;
  }
  while(irqReplayed < irqLimit) applyIrq(irqReplayed++);  // interrupts after the C's last wait
  for(int i = 0; i < smp->effectCount || i < cEffectCount; i++) {
    if(i >= smp->effectCount || i >= cEffectCount) {
      snprintf(why, whyLen, "%d hardware effect(s) in the original, %d from the C", smp->effectCount, cEffectCount);
      return false;
    }
    Effect* o = &smp->effects[i];
    Effect* c = &cEffects[i];
    if(o->kind != c->kind || o->target != c->target || o->src != c->src || o->size != c->size) {
      snprintf(why, whyLen, "effect %d: original %s $%X <- $%06X x%u, C %s $%X <- $%06X x%u", i,
        effectName(o->kind), o->target, o->src, o->size, effectName(c->kind), c->target, c->src, c->size);
      return false;
    }
  }
  int diffs = 0, shown = 0;
  size_t used = 0;
  for(uint32_t i = 0; i < TG3000_WRAM_SIZE; i++) {
    if(i > smp->minSp && i <= smp->entrySp) continue;  // bytes pushed below the entry SP
    if(g_ram[i] == afterRam[i]) continue;
    if(diffs++ == 0) used = (size_t)snprintf(why, whyLen, "WRAM");
    if(shown < 4 && used < whyLen) {
      used += (size_t)snprintf(why + used, whyLen - used, " $%05X: original %02X, C %02X;", i, afterRam[i], g_ram[i]);
      shown++;
    }
  }
  if(diffs) {
    if(used < whyLen) snprintf(why + used, whyLen - used, " %d byte(s) differ", diffs);
    return false;
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

// Sample k (the innermost) has returned: compare, record, start its flag watch.
static void finishSample(int k, const Cpu* cpu) {
  Sample* smp = &samples[k];
  int idx = smp->idx;
  Stats* s = &stats[idx];
  bool owner = irqOwner == k;
  depth = k;
  if(owner) irqOwner = -1;
  memcpy(afterRam, snes->ram, sizeof afterRam);
  Regs after = readRegs(cpu);
  if(table[idx].waitsForInterrupts) {
    if(!owner || irqOverflow || irqActive) { s->abandoned++; return; }
  } else if(cpu_interruptCount != smp->interruptsAtEntry) {
    s->interrupted++;
    return;
  }
  char why[320] = "";
  bool ok;
  uint8_t flagDiff = 0;
  if(smp->hwAccess) {
    snprintf(why, sizeof why, "%s at $%06X", smp->hwWhy, smp->hwAccessAdr);
    ok = false;
  } else if(smp->setupPending) {
    snprintf(why, sizeof why, "DMA/PPU address set up but no DMA started afterwards (not modelled)");
    ok = false;
  } else if(smp->leftExtent) {
    snprintf(why, sizeof why, "ran outside its declared extent at $%06X", smp->leftExtentPc);
    ok = false;
  } else {
    // the replay reads the recorded interrupts; an outer waiting sample keeps recording
    int outerOwner = irqOwner;
    if(owner) irqOwner = k;
    ok = compareWithC(k, &after, why, sizeof why, &flagDiff);
    irqOwner = owner ? -1 : outerOwner;
  }
  for(int f = 0; f < 4; f++) {
    if(!(flagDiff & flagBits[f])) continue;
    s->flagMismatch[f]++;
    if(s->firstFlagFailure[f][0] == '\0') {
      bool orig = f == 0 ? after.n : f == 1 ? after.v : f == 2 ? after.z : after.c;
      snprintf(s->firstFlagFailure[f], sizeof s->firstFlagFailure[f], "%s%c: original %d, C %d (D=%04X A=%04X)",
        fuzzing ? "fuzz " : "", flagNames[f], (int)orig, (int)!orig, smp->entryRegs.d, smp->entryRegs.a);
    }
  }
  Watch* w = &watches[idx];
  if(!fuzzing && !w->active) {  // watch which flags the caller goes on to read
    *w = (Watch){true, FN | FV | FZ | FC, 0, cpu_interruptCount, -1};
    s->watched++;
  }
  if(fuzzing) { if(ok) s->fuzzPass++; else s->fuzzFail++; }
  else { s->samples++; if(ok) s->pass++; else s->fail++; }
  if(!ok && s->firstFailure[0] == '\0') {
    snprintf(s->firstFailure, sizeof s->firstFailure, "%s%s at D=%04X A=%04X X=%04X Y=%04X: %s",
      fuzzing ? "fuzz " : "", table[idx].name, smp->entryRegs.d, smp->entryRegs.a, smp->entryRegs.x,
      smp->entryRegs.y, why);
  }
}

static void startSample(int idx, const Cpu* cpu) {
  const Routine* r = &table[idx];
  int k = depth++;
  Sample* smp = &samples[k];
  memcpy(sampleRam[k], snes->ram, TG3000_WRAM_SIZE);
  uint16_t sp = cpu->sp;
  *smp = (Sample){
    .idx = idx, .entryRegs = readRegs(cpu), .entrySp = sp, .minSp = sp,
    .retPc = (uint16_t)((snes->ram[(uint16_t)(sp + 1)] | (snes->ram[(uint16_t)(sp + 2)] << 8)) + 1),
    .retBank = r->rtl ? snes->ram[(uint16_t)(sp + 3)] : cpu->k,
    .interruptsAtEntry = cpu_interruptCount,
  };
  // A waiting routine records the interrupts it waits through, unless an outer waiting
  // sample already does (a wipe that waits through wait_frames): the outer one keeps them
  // and the inner one goes unsampled. (An owner index at or past k is stale.)
  if(r->waitsForInterrupts && (irqOwner < 0 || irqOwner >= k)) {
    irqOwner = k;
    irqCount = 0;
    irqActive = irqOverflow = false;
    irqSeen = cpu_interruptCount;
  }
}

static uint8_t peekByte(uint32_t adr) {
  if(isRom(adr)) return g_rom[(((adr >> 16) & 0x7F) << 15 | (adr & 0x7FFF)) & 0xFFFFF];
  if(isWram(adr)) {
    uint8_t bank = adr >> 16;
    return snes->ram[(bank == 0x7F ? 0x10000 : 0) | (bank == 0x7E || bank == 0x7F ? (adr & 0xFFFF) : (adr & 0x1FFF))];
  }
  return 0xEA;  // unknown: treat as NOP (reads and writes no flag)
}

static void resolveFlags(int idx, uint8_t mask, bool live) {
  Watch* w = &watches[idx];
  Stats* s = &stats[idx];
  for(int f = 0; f < 4; f++) {
    if(!(w->unresolved & mask & flagBits[f])) continue;
    if(live) s->live[f]++; else s->dead[f]++;
    w->unresolved &= ~flagBits[f];
  }
  if(w->unresolved == 0) w->active = false;
}

// One instruction after a live sample of routine idx returned: which flags does it read
// before writing them? Interrupt handlers are skipped (they restore P with RTI).
static void watchStep(int idx, const Cpu* cpu, uint8_t reads, uint8_t writes) {
  Watch* w = &watches[idx];
  if(cpu_interruptCount != w->interrupts) {
    w->interrupts = cpu_interruptCount;
    if(w->pausedUntilSp < 0) w->pausedUntilSp = (uint16_t)(cpu->sp + (cpu->e ? 3 : 4));
  }
  if(w->pausedUntilSp >= 0) {
    if(cpu->sp != (uint16_t)w->pausedUntilSp) return;
    w->pausedUntilSp = -1;
  }
  if(++w->count > WATCH_CAP) { resolveFlags(idx, FN | FV | FZ | FC, true); return; }
  if(reads) resolveFlags(idx, reads, true);
  if(w->active && writes) resolveFlags(idx, writes, false);
}

static void watchAll(const Cpu* cpu) {
  uint32_t pc = ((uint32_t)cpu->k << 16) | cpu->pc;
  uint8_t op = peekByte(pc);
  uint8_t reads = opFlagsRead[op], writes = opFlagsWritten[op];
  if(op == 0xC2 || op == 0xE2) {
    uint8_t p = peekByte((pc & 0xFF0000) | (uint16_t)(pc + 1));
    writes = (p & 0x80 ? FN : 0) | (p & 0x40 ? FV : 0) | (p & 0x02 ? FZ : 0) | (p & 0x01 ? FC : 0);
  }
  for(int i = 0; i < tableCount; i++) {
    if(watches[i].active) watchStep(i, cpu, reads, writes);
  }
}

// Every entry of a registered routine counts and is checked against its contract; one in
// SAMPLE_STRIDE becomes a compared sample, nested inside other samples if need be.
static void checkEntries(Cpu* cpu, uint32_t pc) {
  for(int i = 0; i < tableCount; i++) {
    if(table[i].entry != pc) continue;
    Stats* s = &stats[i];
    Regs in = readRegs(cpu);
    noteContract(s, &table[i], &in);
    if(s->entries++ % SAMPLE_STRIDE != 0 || s->samples >= MAX_SAMPLES || depth >= MAX_DEPTH) return;
    if(table[i].waitsForInterrupts && irqOwner >= 0) return;  // one interrupt recorder at a time
    // an entry reached in a stack frame that is already being sampled is a branch back to
    // the entry (a loop) or a tail jump, not a call: it would steal the real sample's return
    for(int k = 0; k < depth; k++) if(samples[k].entrySp == cpu->sp) return;
    if(cpu->sp >= 0x2000) return;  // stack outside WRAM: not handled
    startSample(i, cpu);
    s->visited[0] = true;
    return;
  }
}

void difftest_on_insn(Cpu* cpu) {
  uint32_t pc = norm(((uint32_t)cpu->k << 16) | cpu->pc);
  if(irqOwner >= 0) {
    if(irqActive && cpu->sp == irqReturnSp) {  // the handler has returned (RTI)
      memcpy(irqPost[irqCount++], snes->ram, TG3000_WRAM_SIZE);
      irqActive = false;
    }
    if(cpu_interruptCount != irqSeen) {  // this instruction is a handler's first
      irqSeen = cpu_interruptCount;
      if(!irqActive) {
        if(irqCount >= MAX_IRQS) irqOverflow = true;
        else {
          memcpy(irqPre[irqCount], snes->ram, TG3000_WRAM_SIZE);
          irqActive = true;
          irqReturnSp = (uint16_t)(cpu->sp + (cpu->e ? 3 : 4));
        }
      }
    }
    if(irqActive) {
      for(int k = 0; k < depth; k++) if(cpu->sp < samples[k].minSp) samples[k].minSp = cpu->sp;
      return;
    }
  }
  // the innermost sample that has returned (normally the innermost one)
  for(int k = depth - 1; k >= 0; k--) {
    Sample* smp = &samples[k];
    uint16_t back = table[smp->idx].rtl ? 3 : 2;
    if(cpu->pc == smp->retPc && cpu->k == smp->retBank && cpu->sp == (uint16_t)(smp->entrySp + back)) {
      for(int j = depth - 1; j > k; j--) stats[samples[j].idx].abandoned++;  // left without returning
      depth = k + 1;
      finishSample(k, cpu);
      break;
    }
  }
  for(int k = 0; k < depth; k++) {
    Sample* smp = &samples[k];
    const Routine* r = &table[smp->idx];
    if(pc >= r->entry && pc < r->end) stats[smp->idx].visited[pc - r->entry] = true;
    if(cpu->sp < smp->minSp) smp->minSp = cpu->sp;
    // at the routine's own stack depth, every instruction must lie inside its extent
    if(cpu->sp == smp->entrySp && (pc < r->entry || pc >= r->end) && !smp->leftExtent) {
      smp->leftExtent = true;
      smp->leftExtentPc = pc;
    }
    if(++smp->insns > MAX_INSNS) {
      for(int j = depth - 1; j >= k; j--) stats[samples[j].idx].abandoned++;
      if(irqOwner >= k) irqOwner = -1;
      depth = k;
      break;
    }
  }
  watchAll(cpu);
  if(!fuzzing) checkEntries(cpu, pc);
}

void difftest_init(Snes* s, const uint8_t* rom, bool negative) {
  snes = s;
  g_rom = rom;
  table = negative ? negativeRoutines : routines;
  tableCount = negative ? negativeRoutineCount : routineCount;
  stats = calloc(tableCount, sizeof *stats);
  watches = calloc(tableCount, sizeof *watches);
  for(int i = 0; i < tableCount; i++) findStarts(&table[i], &stats[i]);
}

// Fuzz: synthetic inputs, run on the interpreter from the entry, interrupts disabled.
static uint32_t fuzzSeed;
static uint32_t fuzzRandom(void) {
  fuzzSeed = fuzzSeed * 1103515245u + 12345u;
  return (fuzzSeed >> 16) | ((fuzzSeed * 1103515245u + 12345u) & 0xFFFF0000u);
}

void difftest_fuzz(int iterations) {
  fuzzing = true;
  depth = 0;
  irqOwner = -1;
  snes->nmiEnabled = false;
  snes->hIrqEnabled = snes->vIrqEnabled = false;
  for(int i = 0; i < tableCount; i++) {
    const Routine* r = &table[i];
    Stats* s = &stats[i];
    if(r->fuzz == NULL || s->contractCount == 0) continue;
    // Each routine's inputs depend only on its own entry, not on its place in the
    // registry: adding a routine must not change what the others are tested with.
    fuzzSeed = 0x13579BDFu ^ (r->entry * 2654435761u);
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
      for(int n = 0; n < MAX_INSNS && depth > 0; n++) {
        cpu_runOpcode(cpu);  // the bench's trace hook calls difftest_on_insn
      }
      if(depth > 0) { s->abandoned++; depth = 0; irqOwner = -1; }
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
