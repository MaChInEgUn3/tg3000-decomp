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
#include "dsp4_bridge.h"

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
  unsigned long entries, samples, pass, fail, interrupted, abandoned, fuzzPass, fuzzFail, fuzzRequested;
  bool contractBroken;
  Contract contracts[MAX_CONTRACTS];
  int contractCount;
  bool starts[MAX_EXTENT], visited[MAX_EXTENT];
  char firstFailure[400];
  // flags: compared separately; a mismatch only fails the routine if the flag is live
  unsigned long watched, live[4], dead[4], flagMismatch[4];
  char firstFlagFailure[4][160];
  // the original's PPU field writes in live samples: scanlines, how many while the picture
  // is being drawn (line < 225, screen on), BG modes, HDMA targets at the time, and how
  // many samples changed a field's value
  int ppuLineMin, ppuLineMax, ppuDotMin, ppuDotMax;
  bool ppuLines;
  unsigned long ppuWrites, ppuDrawn, ppuSamplesChanged;
  unsigned long foreignStack;  // passing samples that waited through a switch to another task's stack
  uint8_t ppuModes;
  uint8_t ppuHdma[32];  // B-bus addresses ($21xx) of HDMA channels active at the writes
} Stats;

static Snes* snes;
static const Routine* table;
static int tableCount;
static Stats* stats;

// Samples in progress, innermost last: a registered routine called from inside another
// one's sample gets its own sample too (it would otherwise never be sampled once its
// callers are registered).
#define MAX_DEPTH 4
#define MAX_EFFECTS 64
typedef struct { int kind; uint32_t target, src, size; } Effect;

// PPU fields the C sets through platform calls (src/platform.h): the original's register
// writes are compared by end state, as the bench's PPU decoded them (which fields were
// written, and their final values). Bits of each field that the PPU keeps:
static const uint16_t ppuFieldMask[PF_COUNT] = {
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x1FFF, 0x1FFF, 0x1FFF, 0x1FFF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x1F, 0x1F, 0x1F};
static const char* const ppuFieldName[PF_COUNT] = {
  "M7A", "M7B", "M7C", "M7D", "M7X", "M7Y", "M7HOFS", "M7VOFS", "BG1HOFS", "BG1VOFS", "BG2HOFS", "BG2VOFS",
  "BG3HOFS", "BG3VOFS", "COLDATA red", "COLDATA green", "COLDATA blue"};

// Pure byte registers: their state is the last byte written.
static const uint16_t byteRegs[] = {0x212C, 0x212D, 0x2123, 0x2124, 0x2125, 0x2130, 0x2131,
                                    0x4200, 0x4207, 0x4208, 0x4209, 0x420A,
                                    0x2101, 0x2105, 0x2107, 0x2108, 0x2109, 0x210A, 0x210B, 0x210C};
#define N_BYTE_REGS ((int)(sizeof byteRegs / sizeof byteRegs[0]))
static int byteRegIndex(uint32_t adr) {
  if(((adr >> 16) & 0x7F) >= 0x40) return -1;
  for(int i = 0; i < N_BYTE_REGS; i++) if(byteRegs[i] == (adr & 0xFFFF)) return i;
  return -1;
}

typedef struct {
  int idx;
  Regs entryRegs;
  uint16_t entrySp, minSp, retPc, retSp;
  uint8_t retBank;
  uint32_t interruptsAtEntry, insns;
  bool hwAccess, mathWritten, leftExtent, setupPending, vramAdrPending, effectOverflow;
  bool vramLowSet;  // a $2118 byte waiting for its $2119 half
  uint8_t vramLow;
  bool cgLowSet, cgAdrPending;  // a $2122 byte waiting for its second one; $2121 set, unused
  uint8_t cgLow;
  // reads of input registers, in order (served to the C by difftest_input)
  struct { uint16_t reg; uint8_t val; } inputs[256];
  int nInputs;
  bool inputOverflow;
  uint8_t dsp4State;  // where the routine is in a DSP-4 multiply (0 = idle; see dsp4Step)
  uint32_t hwAccessAdr, leftExtentPc;
  bool foreignStack;  // a handler it recorded switched to another stack (two-task screens)
  const char* hwWhy;
  Effect effects[MAX_EFFECTS];  // hardware effects of the original, from its DMA kicks
  int effectCount;
  // PPU fields written (bit per PF_*), and those whose last write combined a latch byte
  // written before the sample (the result depends on state the C does not get)
  uint32_t ppuWritten, ppuStale;
  uint16_t ppuStart[PF_COUNT], ppuEnd[PF_COUNT];
  uint32_t ppuPending;  // fields written by the last instruction: read back at the next one
  uint32_t regWritten;  // byte registers written (bit per byteRegs entry) and their last values
  uint8_t regVal[32];
  bool m7Latch, scrollLatch, scrollLatch2;  // latch bytes written inside the sample
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
static int irqBaseDepth;  // samples from this depth on began inside the handler being recorded
// A recorded handler that loads SP (TCS/TXS) far from its own frame has switched to another
// task's stack (the two-task screens): its stack use is not the waiting sample's, so it
// does not widen that sample's stack window. A routine's own stack stays within
// STACK_REACH bytes below its entry; a row that loads SP outside that fails.
#define STACK_REACH 0x100
static bool irqForeign;
static uint8_t lastOp;  // the opcode of the instruction just executed
static int accessBase;    // the first sample a hardware access belongs to
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
static bool cEffectOverflow;
static void* cSample;  // the sample the C is being compared with
static bool cInputUsed[256], cInputMissing;
static uint16_t cMissingReg;


// Input registers whose values the C receives through platform calls (difftest_input):
// the APU ports, $213F (the PPU status with the 50/60 Hz flag), the joypad registers
// ($4212 status, with $4213 that a 16-bit BIT of it reads too; $4218-$421F automatic
// reads; $4016/$4017 serial lines).
static bool isInputReg(uint32_t adr) {
  if(((adr >> 16) & 0x7F) >= 0x40) return false;
  uint16_t a = adr & 0xFFFF;
  return a == 0x213F || (a >= 0x2140 && a <= 0x217F) || a == 0x4212 || a == 0x4213 || (a >= 0x4218 && a <= 0x421F) ||
         a == 0x4016 || a == 0x4017;
}

// Writes that drive the joypad protocol (the latch strobe $4016, the I/O line $4201): the
// platform's input calls stand for them.
static bool isJoypadProtocolWrite(uint32_t adr) {
  if(((adr >> 16) & 0x7F) >= 0x40) return false;
  uint16_t a = adr & 0xFFFF;
  return a == 0x4016 || a == 0x4201;
}
static bool collectingC;

static uint8_t inputFor(void* p, uint16_t reg) {
  Sample* smp = p;
  for(int i = 0; i < smp->nInputs; i++) {
    if(smp->inputs[i].reg != reg || cInputUsed[i]) continue;
    cInputUsed[i] = true;
    return smp->inputs[i].val;
  }
  if(!cInputMissing) cMissingReg = reg;
  cInputMissing = true;
  return 0;
}

int difftest_input_peek(uint16_t reg, int n) {
  Sample* smp = cSample;
  if(smp == NULL) return -1;
  for(int i = 0; i < smp->nInputs; i++) if(smp->inputs[i].reg == reg && n-- == 0) return smp->inputs[i].val;
  return -1;
}

uint8_t difftest_input(uint16_t reg) {
  if(!collectingC || cSample == NULL) return 0;
  return inputFor(cSample, reg);
}

void difftest_effect(int kind, uint32_t target, uint32_t src, uint32_t size) {
  if(!collectingC) return;
  if(cEffectCount >= MAX_EFFECTS) { cEffectOverflow = true; return; }
  cEffects[cEffectCount++] = (Effect){kind, target, src, size};
}

static void vramWordWrite(uint32_t adr, uint8_t val);
static void cgramWordWrite(uint32_t adr, uint8_t val);
static void mode7ProductRead(uint32_t adr);

static uint32_t cPpuWritten;
static uint16_t cPpu[PF_COUNT];
static uint32_t cRegWritten;
static uint8_t cRegVal[32];

void difftest_reg(uint16_t adr, uint8_t value) {
  if(!collectingC) return;
  int i = byteRegIndex(adr);
  if(i < 0) return;
  cRegWritten |= 1u << i;
  cRegVal[i] = value;
}

void difftest_ppu_field(int field, uint16_t value) {
  if(!collectingC) return;
  cPpuWritten |= 1u << field;
  cPpu[field] = value;
}

static uint16_t ppuFieldValue(int field) {
  const Ppu* ppu = snes->ppu;
  if(field <= PF_M7V) return (uint16_t)ppu->m7matrix[field];
  if(field == PF_FIXR) return ppu->fixedColorR;
  if(field == PF_FIXG) return ppu->fixedColorG;
  if(field == PF_FIXB) return ppu->fixedColorB;
  const BgLayer* bg = &ppu->bgLayer[(field - PF_BG1H) / 2];  // BG1-BG3
  return (field - PF_BG1H) % 2 == 0 ? bg->hScroll : bg->vScroll;
}

// A write to a PPU register the C models as fields: $210D/$210E (BG1 scroll, also the
// Mode 7 offset), $210F-$2112 (BG2, BG3 scroll) and $211B-$2120 (Mode 7 matrix and centre). Each is written twice, low
// byte then high, through latches shared between registers (LakeSnes's ppu_write).
static bool ppuFieldWrite(uint32_t adr, uint8_t val) {
  if(((adr >> 16) & 0x7F) >= 0x40) return false;
  uint16_t a = adr & 0xFFFF;
  if(a == 0x2132) {  // the fixed colour: each channel whose select bit is set takes the value
    for(int k = accessBase; k < depth; k++) {
      Sample* smp = &samples[k];
      uint32_t f = 0;
      if(val & 0x20) f |= 1u << PF_FIXR;
      if(val & 0x40) f |= 1u << PF_FIXG;
      if(val & 0x80) f |= 1u << PF_FIXB;
      smp->ppuWritten |= f;
      smp->ppuStale &= ~f;
      smp->ppuPending |= f;
    }
    return true;
  }
  bool scroll = a >= 0x210D && a <= 0x2112, m7 = a == 0x210D || a == 0x210E || (a >= 0x211B && a <= 0x2120);
  if(!scroll && !m7) return false;
  bool hofs = scroll && (a & 1);  // $210D/$210F: horizontal; $210E/$2110: vertical
  for(int k = accessBase; k < depth; k++) {
    Sample* smp = &samples[k];
    uint32_t fresh = 0, stale = 0;
    if(m7) {
      int f = a == 0x210D ? PF_M7H : a == 0x210E ? PF_M7V : a - 0x211B;
      *(smp->m7Latch ? &fresh : &stale) |= 1u << f;
      smp->m7Latch = true;
    }
    if(scroll) {  // BGnHOFS takes both scroll latches, BGnVOFS the first one
      int f = PF_BG1H + (a - 0x210D);
      bool ok = smp->scrollLatch && (!hofs || smp->scrollLatch2);
      *(ok ? &fresh : &stale) |= 1u << f;
      smp->scrollLatch = true;
      if(hofs) smp->scrollLatch2 = true;
    }
    smp->ppuWritten |= fresh | stale;
    smp->ppuStale = (smp->ppuStale & ~fresh) | stale;
    smp->ppuPending |= fresh | stale;  // the field's value once this write lands
    if(!fuzzing && cpu_interruptCount == smp->interruptsAtEntry) {  // not a handler's write
      Stats* s = &stats[smp->idx];
      int line = snes->vPos;
      int dot = snes->hPos;
      if(!s->ppuLines || dot < s->ppuDotMin) s->ppuDotMin = dot;
      if(!s->ppuLines || dot > s->ppuDotMax) s->ppuDotMax = dot;
      if(!s->ppuLines || line < s->ppuLineMin) s->ppuLineMin = line;
      if(!s->ppuLines || line > s->ppuLineMax) s->ppuLineMax = line;
      s->ppuLines = true;
      const Ppu* ppu = snes->ppu;
      s->ppuWrites++;
      if(line < 225 && !ppu->forcedBlank && ppu->brightness != 0) s->ppuDrawn++;
      s->ppuModes |= (uint8_t)(1u << (ppu->mode & 7));
      for(int c = 0; c < 8; c++) {
        const DmaChannel* ch = &snes->dma->channel[c];
        if(ch->hdmaActive && !ch->terminated) s->ppuHdma[ch->bAdr >> 3] |= (uint8_t)(1u << (ch->bAdr & 7));
      }
    }
  }
  return true;
}

static const char* effectName(int kind) {
  return kind == EFFECT_VRAM ? "VRAM upload" : kind == EFFECT_CGRAM ? "CGRAM upload" :
         kind == EFFECT_VRAM_FILL ? "VRAM fill" : kind == EFFECT_VRAM_WORD ? "VRAM word" :
         kind == EFFECT_APU ? "APU port write" : kind == EFFECT_CGRAM_WORD ? "CGRAM colour" : "other DMA";
}

// A write of `val` to MDMAEN ($420B): one effect per channel started, read from the
// channel's registers and the PPU's address state as they are at the kick, for every
// sample in progress (an inner routine's upload is also its caller's).
static uint8_t peekByte(uint32_t adr);

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
    } else if(!ch->fromB && ch->fixed && ch->bAdr == 0x18 && ch->mode == 1 && ppu->vramIncrementOnHigh &&
              ppu->vramIncrement == 1 && ppu->vramRemapMode == 0) {
      // a fixed source writes the same byte to both halves of every word
      e = (Effect){EFFECT_VRAM_FILL, ppu->vramPointer, peekByte(src), size};
    }
    for(int k = accessBase; k < depth; k++) {
      Sample* smp = &samples[k];
      if(smp->effectCount < MAX_EFFECTS) smp->effects[smp->effectCount++] = e;
      else smp->effectOverflow = true;
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
            cpu->e || cpu->mf, cpu->e || cpu->xf, cpu->e, cpu->sp};
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

// The DSP-4 at $30-$3F:8000 (data below $C000, status above). The C calls the chip's
// commands as native functions (src/dsp4.c), so a routine's DSP-4 traffic is its own work
// when it is made of whole commands the C has: so far command $0000, the multiply (the
// word $0000, two 16-bit operands, then the 32-bit product's 4 bytes, each read or skipped
// by a write — the chip's behaviour, checked on the real firmware). Status reads are
// free. Anything else, or a multiply left unfinished, is a hardware access the C cannot
// reproduce. Returns whether the access fits.
static bool isDsp4(uint32_t adr) {
  uint8_t bank = (adr >> 16) & 0x7F;
  return bank >= 0x30 && bank < 0x40 && (adr & 0xFFFF) >= 0x8000;
}

static bool dsp4Step(Sample* smp, uint32_t adr, uint8_t val, bool write) {
  if((adr & 0xFFFF) >= 0xC000) return !write;          // status: reads only
  uint8_t st = smp->dsp4State;
  if(st >= 6) {  // the product's 4 bytes: a read takes one, a write skips one (as the chip does)
    smp->dsp4State = st == 9 ? 0 : (uint8_t)(st + 1);
    return true;
  }
  if(!write) return false;                             // reading before the operands are in
  if(st < 2 && val != 0) return false;                 // the command word must be $0000
  smp->dsp4State = (uint8_t)(st + 1);
  return true;
}

void difftest_on_access(uint32_t adr, uint8_t val, bool write) {
  // a handler recorded for a waiting sample: its hardware is its own, and only the samples
  // that began inside it see it
  accessBase = (irqOwner >= 0 && irqActive) ? irqBaseDepth : 0;
  if(depth <= accessBase) return;
  if(isDsp4(adr)) {  // before the ROM test: $30:8000 reads look like ROM reads to it
    for(int k = accessBase; k < depth; k++) {
      Sample* smp = &samples[k];
      if(smp->hwAccess || dsp4Step(smp, adr, val, write)) continue;
      smp->hwWhy = "DSP-4 traffic other than whole multiplies";
      smp->hwAccess = true;
      smp->hwAccessAdr = adr;
    }
    return;
  }
  if(isWram(adr) || (!write && isRom(adr))) return;
  if(write && ppuFieldWrite(adr, val)) return;
  if(write && ((adr >> 16) & 0x7F) < 0x40 && (adr & 0xFFFF) == 0x2122) { cgramWordWrite(adr, val); return; }
  int bri = write ? byteRegIndex(adr) : -1;
  if(bri >= 0) {
    for(int k = accessBase; k < depth; k++) {
      samples[k].regWritten |= 1u << bri;
      samples[k].regVal[bri] = val;
    }
    return;
  }
  if(write && isJoypadProtocolWrite(adr)) return;
  uint16_t lo = adr & 0xFFFF;
  bool apuPort = lo >= 0x2140 && lo <= 0x217F && ((adr >> 16) & 0x7F) < 0x40;
  if((write && apuPort) || (!write && isInputReg(adr))) {
    // APU port writes are effects (port, value) in order; input reads are recorded
    for(int k = accessBase; k < depth; k++) {
      Sample* smp = &samples[k];
      if(write) {
        if(smp->effectCount < MAX_EFFECTS) smp->effects[smp->effectCount++] = (Effect){EFFECT_APU, (adr & 3), val, 1};
        else smp->effectOverflow = true;
      } else {
        // a value repeated by a polling loop is one input: only changes are recorded
        uint16_t reg = apuPort ? (uint16_t)(0x2140 + (adr & 3)) : lo;
        int last = -1;
        for(int i = smp->nInputs - 1; i >= 0 && last < 0; i--) if(smp->inputs[i].reg == reg) last = i;
        // serial joypad reads are bits: every one counts
        if(reg != 0x4016 && reg != 0x4017 && last >= 0 && smp->inputs[last].val == val) continue;
        if(smp->nInputs < 256) {
          smp->inputs[smp->nInputs].reg = reg;
          smp->inputs[smp->nInputs++].val = val;
        } else smp->inputOverflow = true;
      }
    }
    return;
  }
  if(write && ((adr >> 16) & 0x7F) < 0x40 && (adr & 0xFFFF) == 0x420B) {
    recordKick(val);
    for(int k = accessBase; k < depth; k++) samples[k].setupPending = samples[k].vramAdrPending = samples[k].cgAdrPending = false;
    return;
  }
  if(((adr >> 16) & 0x7F) < 0x40 && (write ? ((adr & 0xFFFF) == 0x2118 || (adr & 0xFFFF) == 0x2119)
                                             : ((adr & 0xFFFF) >= 0x2134 && (adr & 0xFFFF) <= 0x2136))) {
    if(write) vramWordWrite(adr, val);
    else mode7ProductRead(adr);
    return;
  }
  for(int k = accessBase; k < depth; k++) {
    Sample* smp = &samples[k];
    if(smp->hwAccess) continue;
    uint16_t lo16 = adr & 0xFFFF;
    if(!write && table[smp->idx].rti && ((adr >> 16) & 0x7F) < 0x40 && lo16 >= 0x4210 && lo16 <= 0x4212) continue;
    if(isMathReg(adr, write)) {
      if(write) { smp->mathWritten = true; continue; }
      if(smp->mathWritten) continue;
      smp->hwWhy = "reads a multiply/divide result it did not start";
    } else if(write && isUploadSetup(adr)) {
      uint16_t a16 = adr & 0xFFFF;
      if(a16 == 0x2116 || a16 == 0x2117) smp->vramAdrPending = true;  // a DMA or a word write uses it
      else if(a16 == 0x2121) smp->cgAdrPending = true;                // a DMA or a colour write uses it
      else smp->setupPending = true;
      continue;
    } else {
      smp->hwWhy = "hardware access (not supported by this routine's C)";
    }
    smp->hwAccess = true;
    smp->hwAccessAdr = adr;
  }
}

// A CPU write to VMDATA ($2118 low, $2119 high): with VMAIN = $80 (increment by one word
// after the high byte, no remapping) each pair is one word written at the VRAM address
// (the hook runs before the PPU takes the write, so the pointer is this word's). The
// console only stores it in vblank or under forced blank; a word written while the screen
// is drawn is lost there, which the C (no scanlines) cannot reproduce.
static void vramWordWrite(uint32_t adr, uint8_t val) {
  const Ppu* ppu = snes->ppu;
  bool mode = ppu->vramIncrementOnHigh && ppu->vramIncrement == 1 && ppu->vramRemapMode == 0;
  bool high = (adr & 0xFFFF) == 0x2119;
  for(int k = accessBase; k < depth; k++) {
    Sample* smp = &samples[k];
    if(smp->hwAccess) continue;
    const char* why = NULL;
    if(!mode) why = "VRAM data written under a VMAIN mode the C does not model";
    else if(high && !smp->vramLowSet) why = "VRAM high byte written without its low byte";
    else if(high && !ppu->forcedBlank && !snes->inVblank) why = "VRAM word written while the screen is drawn (the console drops it)";
    if(why) {
      smp->hwWhy = why;
      smp->hwAccess = true;
      smp->hwAccessAdr = adr;
      continue;
    }
    if(!high) {
      smp->vramLow = val;
      smp->vramLowSet = true;
      continue;
    }
    smp->vramLowSet = false;
    smp->vramAdrPending = false;
    if(smp->effectCount < MAX_EFFECTS)
      smp->effects[smp->effectCount++] = (Effect){EFFECT_VRAM_WORD, ppu->vramPointer, (uint32_t)(smp->vramLow | val << 8), 1};
    else smp->effectOverflow = true;
  }
}

// A CPU write to CGDATA ($2122): two of them are one colour, stored at the CGRAM address on
// the second (the hook runs before the PPU takes it, so the pointer is that colour's). An
// effect (index, colour). The first byte must have been written inside the sample.
static void cgramWordWrite(uint32_t adr, uint8_t val) {
  const Ppu* ppu = snes->ppu;
  for(int k = accessBase; k < depth; k++) {
    Sample* smp = &samples[k];
    if(smp->hwAccess) continue;
    if(!ppu->cgramSecondWrite) {
      smp->cgLow = val;
      smp->cgLowSet = true;
      continue;
    }
    if(!smp->cgLowSet) {
      smp->hwWhy = "a colour completed from a byte written before the routine";
      smp->hwAccess = true;
      smp->hwAccessAdr = adr;
      continue;
    }
    smp->cgLowSet = false;
    smp->cgAdrPending = false;
    if(smp->effectCount < MAX_EFFECTS)
      smp->effects[smp->effectCount++] = (Effect){EFFECT_CGRAM_WORD, ppu->cgramPointer, (uint32_t)(smp->cgLow | val << 8), 1};
    else smp->effectOverflow = true;
  }
}

// A read of the Mode 7 product ($2134-$2136: matrix A times the high byte of B): the C
// computes it natively, so the read is the routine's own work provided both operands were
// written inside the sample, from bytes written inside it.
static void mode7ProductRead(uint32_t adr) {
  uint32_t need = 1u << PF_M7A | 1u << PF_M7B;
  for(int k = accessBase; k < depth; k++) {
    Sample* smp = &samples[k];
    if(smp->hwAccess || ((smp->ppuWritten & need) == need && !(smp->ppuStale & need))) continue;
    smp->hwWhy = "reads a Mode 7 product it did not set up";
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
  cEffectOverflow = false;
  cRegWritten = 0;
  cSample = smp;
  memset(cInputUsed, 0, sizeof cInputUsed);
  cInputMissing = false;
  cPpuWritten = 0;
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
  if(cInputMissing) {
    snprintf(why, whyLen, "the C read $%04X more times than the original", cMissingReg);
    return false;
  }
  if(cEffectOverflow) {
    snprintf(why, whyLen, "the C made more than %d hardware effects", MAX_EFFECTS);
    return false;
  }
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
  if(cPpuWritten != smp->ppuWritten) {
    snprintf(why, whyLen, "PPU fields written: original %05X, C %05X (bits: M7A..M7VOFS, BG1HOFS..BG3VOFS, COLDATA R G B)",
      (unsigned)smp->ppuWritten, (unsigned)cPpuWritten);
    return false;
  }
  for(int f = 0; f < PF_COUNT; f++) {
    if(!(smp->ppuWritten & (1u << f))) continue;
    uint16_t o = smp->ppuEnd[f] & ppuFieldMask[f], c = cPpu[f] & ppuFieldMask[f];
    if(o != c) {
      snprintf(why, whyLen, "PPU %s: original %04X, C %04X", ppuFieldName[f], o, c);
      return false;
    }
  }
  if(cRegWritten != smp->regWritten) {
    snprintf(why, whyLen, "byte registers written: original %05X, C %05X", (unsigned)smp->regWritten, (unsigned)cRegWritten);
    return false;
  }
  for(int i = 0; i < N_BYTE_REGS; i++) {
    if((smp->regWritten & (1u << i)) && smp->regVal[i] != cRegVal[i]) {
      snprintf(why, whyLen, "$%04X: original %02X, C %02X", byteRegs[i], smp->regVal[i], cRegVal[i]);
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
  for(int f = 0; f < PF_COUNT; f++) if(smp->ppuPending & (1u << f)) smp->ppuEnd[f] = ppuFieldValue(f);
  smp->ppuPending = 0;
  if(!fuzzing && cpu_interruptCount == smp->interruptsAtEntry) {
    for(int f = 0; f < PF_COUNT; f++) {
      if((smp->ppuWritten & (1u << f)) && ((smp->ppuEnd[f] ^ smp->ppuStart[f]) & ppuFieldMask[f])) {
        s->ppuSamplesChanged++;
        break;
      }
    }
  }
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
  if(!smp->hwAccess && smp->dsp4State != 0) {
    static char unfinished[96];
    snprintf(unfinished, sizeof unfinished, "a DSP-4 multiply left unfinished (%d of 6 bytes written, %d of 4 read)",
      smp->dsp4State < 6 ? smp->dsp4State : 6, smp->dsp4State < 6 ? 0 : smp->dsp4State - 6);
    smp->hwAccess = true;
    smp->hwWhy = unfinished;
    smp->hwAccessAdr = 0x308000;
  }
  if(smp->hwAccess) {
    snprintf(why, sizeof why, "%s at $%06X", smp->hwWhy, smp->hwAccessAdr);
    ok = false;
  } else if(smp->ppuStale) {
    snprintf(why, sizeof why, "a PPU register write completed a latch byte written before the routine (fields %05X)",
      (unsigned)smp->ppuStale);
    ok = false;
  } else if(smp->inputOverflow) {
    snprintf(why, sizeof why, "more than 256 input reads");
    ok = false;
  } else if(smp->effectOverflow) {
    snprintf(why, sizeof why, "more than %d hardware effects", MAX_EFFECTS);
    ok = false;
  } else if(smp->vramLowSet) {
    snprintf(why, sizeof why, "VRAM low byte written without its high byte (not modelled)");
    ok = false;
  } else if(smp->cgLowSet) {
    snprintf(why, sizeof why, "a colour's first byte written without its second (not modelled)");
    ok = false;
  } else if(smp->setupPending || smp->vramAdrPending || smp->cgAdrPending) {
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
  else { s->samples++; if(ok) s->pass++; else s->fail++; if(ok && smp->foreignStack) s->foreignStack++; }
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
    .retSp = (uint16_t)(sp + (r->rtl ? 3 : 2)),
    .interruptsAtEntry = cpu_interruptCount,
  };
  if(r->rti) {  // the interrupt frame (P, PC low, PC high, PB) above what the prologue pushed
    uint16_t f = (uint16_t)(sp + r->rtiFrame);
    smp->retPc = (uint16_t)(snes->ram[(uint16_t)(f + 2)] | (snes->ram[(uint16_t)(f + 3)] << 8));
    smp->retBank = snes->ram[(uint16_t)(f + 4)];
    smp->retSp = (uint16_t)(f + 4);
  }
  for(int f = 0; f < PF_COUNT; f++) smp->ppuStart[f] = ppuFieldValue(f);
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
static void checkEntries(Cpu* cpu, uint32_t pc, bool inHandler) {
  for(int i = 0; i < tableCount; i++) {
    if(table[i].entry != pc) continue;
    if(inHandler && !table[i].rti) return;  // inside a recorded handler: only handler rows
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
          irqForeign = false;
          irqBaseDepth = depth;
          irqReturnSp = (uint16_t)(cpu->sp + (cpu->e ? 3 : 4));
        }
      }
    }
  }
  // inside a handler while a waiting sample records it, only the handler's own samples
  // (interrupt-handler rows) go on; the others just see its stack use
  bool inHandler = irqOwner >= 0 && irqActive;
  int base = inHandler ? irqBaseDepth : 0;
  bool loadedSp = lastOp == 0x1B || lastOp == 0x9A;  // TCS, TXS
  if(inHandler && loadedSp)
    irqForeign = cpu->sp > irqReturnSp || (int)irqReturnSp - (int)cpu->sp > STACK_REACH;
  if(inHandler && !irqForeign) for(int k = 0; k < base && k < depth; k++) if(cpu->sp < samples[k].minSp) samples[k].minSp = cpu->sp;
  if(inHandler && irqForeign && irqOwner >= 0 && irqOwner < depth) samples[irqOwner].foreignStack = true;
  // fields written by the previous instruction have landed: keep their values now (HDMA may
  // rewrite them later in the frame)
  for(int k = 0; k < depth; k++) {
    Sample* smp = &samples[k];
    for(int f = 0; smp->ppuPending && f < PF_COUNT; f++)
      if(smp->ppuPending & (1u << f)) { smp->ppuEnd[f] = ppuFieldValue(f); smp->ppuPending &= ~(1u << f); }
  }
  // the innermost sample that has returned (normally the innermost one)
  for(int k = depth - 1; k >= base; k--) {
    Sample* smp = &samples[k];
    if(cpu->pc == smp->retPc && cpu->k == smp->retBank && cpu->sp == smp->retSp) {
      for(int j = depth - 1; j > k; j--) stats[samples[j].idx].abandoned++;  // left without returning
      depth = k + 1;
      finishSample(k, cpu);
      break;
    }
  }
  for(int k = base; k < depth; k++) {
    Sample* smp = &samples[k];
    const Routine* r = &table[smp->idx];
    // (inside an interrupt that landed in the sample, the stack is the handler's: such a
    // sample is not compared, or, for a waiting routine, the recording covers it)
    if(loadedSp && cpu_interruptCount == smp->interruptsAtEntry && (cpu->sp > smp->retSp || (int)smp->entrySp - (int)cpu->sp > STACK_REACH)) {
      // it switched stacks: fail it (and the samples inside it) now, it will not return here
      Stats* s = &stats[smp->idx];
      if(fuzzing) s->fuzzFail++; else { s->samples++; s->fail++; }
      if(s->firstFailure[0] == '\0')
        snprintf(s->firstFailure, sizeof s->firstFailure, "%s%s loaded SP = $%04X (entry SP $%04X) before $%06X: an undeclared stack switch",
          fuzzing ? "fuzz " : "", r->name, cpu->sp, smp->entrySp, pc);
      for(int j = depth - 1; j > k; j--) stats[samples[j].idx].abandoned++;
      if(irqOwner >= k) irqOwner = -1;
      depth = k;
      break;
    }
    if(pc >= r->entry && pc < r->end) stats[smp->idx].visited[pc - r->entry] = true;
    if(cpu->sp < smp->minSp) smp->minSp = cpu->sp;
    // at the routine's own stack depth, every instruction must lie inside its extent (or a
    // range it tail-jumps into)
    bool inside = pc >= r->entry && pc < r->end;
    for(int t = 0; t < 2; t++) if(r->alsoIn[t][1] && pc >= r->alsoIn[t][0] && pc < r->alsoIn[t][1]) inside = true;
    if(cpu->sp == smp->entrySp && !inside && !smp->leftExtent) {
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
  if(!inHandler) watchAll(cpu);
  if(!fuzzing) checkEntries(cpu, pc, inHandler);
  lastOp = peekByte(((uint32_t)cpu->k << 16) | cpu->pc);
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
  // Every fuzz case starts from the same RAM (as the live run left it): what one case
  // leaves behind must not change another's.
  static uint8_t base[TG3000_WRAM_SIZE];
  memcpy(base, snes->ram, sizeof base);
  for(int i = 0; i < tableCount; i++) {
    const Routine* r = &table[i];
    Stats* s = &stats[i];
    if(r->fuzz == NULL || s->contractCount == 0) continue;
    // Each routine's inputs depend only on its own entry, not on its place in the
    // registry: adding a routine must not change what the others are tested with.
    fuzzSeed = 0x13579BDFu ^ (r->entry * 2654435761u);
    for(int k = 0; k < iterations; k++) {
      // each case starts clean: the same WRAM, and no interrupt or HDMA that an earlier
      // case (or the game) left enabled can run during it
      memcpy(snes->ram, base, sizeof base);
      snes->nmiEnabled = false;
      snes->hIrqEnabled = snes->vIrqEnabled = false;
      for(int ch = 0; ch < 8; ch++) snes->dma->channel[ch].hdmaActive = false;
      dsp4_bridge_reset();  // and the DSP-4 waits for a command
      // the controllers hold random buttons (their serial reads; signature bits included)
      snes->input1->currentState = (uint16_t)fuzzRandom();
      snes->input2->currentState = (uint16_t)fuzzRandom();
      // the screen blanked, as when the game writes VRAM itself: the console takes VRAM data
      // at any line then (the live samples check the game's own timing)
      snes->ppu->forcedBlank = true;
      snes->dma->hdmaInitRequested = snes->dma->hdmaRunRequested = false;
      s->fuzzRequested++;
      Contract c = s->contracts[k % s->contractCount];
      Regs in = {(uint16_t)fuzzRandom(), (uint16_t)fuzzRandom(), (uint16_t)fuzzRandom(), c.d, c.db,
                 fuzzRandom() & 1, fuzzRandom() & 1, fuzzRandom() & 1, fuzzRandom() & 1, c.m8, c.x8, false};
      if(in.x8) { in.x &= 0xFF; in.y &= 0xFF; }
      r->fuzz(&in, snes->ram, fuzzRandom);
      Cpu* cpu = snes->cpu;
      cpu->nmiWanted = cpu->irqWanted = cpu->intWanted = false;
      cpu->i = true;
      cpu->d = false;  // binary mode (an earlier case's RTI may have restored a P with D set)
      cpu->e = false;
      cpu->mf = in.m8; cpu->xf = in.x8;
      cpu->a = in.a; cpu->x = in.x; cpu->y = in.y; cpu->dp = in.d; cpu->db = in.db;
      cpu->n = in.n; cpu->v = in.v; cpu->z = in.z; cpu->c = in.c;
      // sentinel return to $8000 of the entry bank (a BRA * trap), stack at $1F00
      if(r->rti) {  // an interrupt frame: PB, PC = $8000, a random P; then what the prologue pushed
        snes->ram[0x1F00] = r->entry >> 16;
        snes->ram[0x1EFF] = 0x80; snes->ram[0x1EFE] = 0x00;
        snes->ram[0x1EFD] = (uint8_t)(fuzzRandom() & ~0x08);  // decimal mode off
        for(int b = 0; b < r->rtiFrame; b++) snes->ram[0x1EFC - b] = (uint8_t)fuzzRandom();
        cpu->sp = (uint16_t)(0x1EFC - r->rtiFrame);
      } else if(r->rtl) {
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
    // a fuzz case that was interrupted or abandoned was neither compared nor passed: the
    // harness lost it, and a lost case may hide a deviation (e.g. an input on which the
    // original runs away), so it fails the routine
    unsigned long fuzzLost = s->fuzzRequested - (s->fuzzPass + s->fuzzFail);
    bool ok = flagsOk && s->fail == 0 && s->fuzzFail == 0 && fuzzLost == 0 && (s->pass + s->fuzzPass) > 0 &&
              seen == need && !s->contractBroken;
    const char* status = !reached ? "unreached" : ok ? "verified" : "failed";
    if(reached) allOk &= ok;  // a routine this run never reached is reported, not failed
    printf("difftest %-16s $%06X: %s  live %lu/%lu pass (%lu entries, %lu interrupted, %lu abandoned), fuzz %lu/%lu (%lu lost), paths %d/%d%s\n",
      r->name, r->entry, !reached ? "UNREACHED" : ok ? "PASS" : "FAIL", s->pass, s->samples, s->entries, s->interrupted, s->abandoned,
      s->fuzzPass, s->fuzzPass + s->fuzzFail, fuzzLost, seen, need, s->contractBroken ? ", ENTRY CONTRACT BROKEN" : "");
    for(int c = 0; c < s->contractCount; c++) {
      Contract* k = &s->contracts[c];
      printf("    entry state: D=%04X DB=%02X %s %s%s\n", k->d, k->db, k->m8 ? "m8" : "m16", k->x8 ? "x8" : "x16", k->e ? " emu" : "");
    }
    printf("    %s; mismatches N%lu V%lu Z%lu C%lu\n", flagNote, s->flagMismatch[0], s->flagMismatch[1], s->flagMismatch[2], s->flagMismatch[3]);
    if(s->firstFailure[0]) printf("    first failure: %s\n", s->firstFailure);
    if(s->foreignStack)
      printf("    %lu passing sample(s) waited through a switch to the other task's stack (left out of the stack window)\n", s->foreignStack);
    if(s->ppuLines) {
      printf("    PPU field writes on scanlines %d..%d, h %d..%d (live samples): %lu of %lu while drawing, BG modes", s->ppuLineMin,
        s->ppuLineMax, s->ppuDotMin, s->ppuDotMax, s->ppuDrawn, s->ppuWrites);
      for(int m = 0; m < 8; m++) if(s->ppuModes & (1u << m)) printf(" %d", m);
      printf(", HDMA to");
      bool any = false;
      for(int b = 0; b < 256; b++) if(s->ppuHdma[b >> 3] & (1u << (b & 7))) { printf(" $21%02X", b); any = true; }
      printf("%s; %lu sample(s) changed a field\n", any ? "" : " none", s->ppuSamplesChanged);
    }
    if(reached && seen < need) {  // which instruction starts no sample reached
      printf("    paths not reached:");
      for(int k = 0, shown = 0; k < MAX_EXTENT && shown < 8; k++)
        if(s->starts[k] && !s->visited[k]) { printf(" $%06X", (unsigned)(r->entry + k)); shown++; }
      printf("\n");
    }
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
