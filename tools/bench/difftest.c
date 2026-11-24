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
bool dsp4_bridge_firmware_on(void);  // (tools/bench/dsp4_bridge.cpp, harness-29)

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
#define SAMPLE_STRIDE 3      // take one entry in N...
#define SAMPLE_ALL_FIRST 32  // ...after the first entries, which are all taken: a routine the
                             // movies enter only a few times gets every entry compared
#define MAX_EXTENT 0x1000   // a row's extent in bytes (findStarts refuses a longer one)
#define MAX_CONTRACTS 8
#define MAX_INSNS 4000000    // a sample that runs longer is abandoned
#define WATCH_CAP 400        // flag liveness: instructions watched after a return
#define WATCH_CAP_HANDLER 20000  // after a prologue's end: its target runs to its RTI (which writes all flags)
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
  unsigned long hidden;  // entries inside a handler a waiting sample records (not sampled)
  unsigned long replayPass, replayFail, replayRequested;  // interrupted entries run again (see Replay)
  bool contractBroken;
  Contract contracts[MAX_CONTRACTS];
  int contractCount;
  bool starts[MAX_EXTENT], visited[MAX_EXTENT];
  char firstFailure[400];
  // flags: compared separately; a mismatch only fails the routine if the flag is live
  unsigned long watched, live[4], dead[4], flagMismatch[4];
  // harness-17: watches of a cut (a sample that ended at endAt, inside a routine) that found
  // the flag written in straight-line code after it; at every watched cut: dead there
  unsigned long straightDead[4];
  char firstFlagFailure[4][160];
  // the original's PPU field writes in live samples: scanlines, how many while the picture
  // is being drawn (line < 225, screen on), BG modes, HDMA targets at the time, and how
  // many samples changed a field's value
  int ppuLineMin, ppuLineMax, ppuDotMin, ppuDotMax;
  bool ppuLines;
  unsigned long ppuWrites, ppuDrawn, ppuSamplesChanged;
  unsigned long foreignStack;  // passing samples that waited through a switch to another task's stack
  // harness-15: waiting samples an interrupt stopped outside a wait (a lag frame), failing
  // ones (not a failure while some sample without lag passes), and passes without lag
  unsigned long lagFail, cleanPass;
  char firstLagFailure[400];
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
#define MAX_DEPTH 8
#define MAX_EFFECTS 512  // track_objects_stream's replay on the password movie makes 158
// harness-18: a row with a budget (maxInsns: a screen's frame loop, thousands of frames)
// keeps more effects and input reads; the buffers live outside the sample, one per depth
#define MAX_EFFECTS_LONG 65536
#define MAX_INPUTS 256
#define MAX_INPUTS_LONG (1 << 19)  // (serial pad reads count every bit: ~70 a frame)
typedef struct { uint16_t reg; uint8_t val; uint32_t frame; } InputRead;  // frame: cpu_interruptCount
// `vram`: for a raw DMA to VMDATA ($2118/$2119), the VRAM state it wrote with (vramPack:
// the word pointer and VMAIN's fields), 0 otherwise.
typedef struct { int kind; uint32_t target, src, size, vram; } Effect;

// PPU fields the C sets through platform calls (src/platform.h): the original's register
// writes are compared by end state, as the bench's PPU decoded them (which fields were
// written, and their final values). Bits of each field that the PPU keeps:
static const uint16_t ppuFieldMask[PF_COUNT] = {
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x1FFF, 0x1FFF, 0x1FFF, 0x1FFF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x1F, 0x1F, 0x1F};
static const char* const ppuFieldName[PF_COUNT] = {
  "M7A", "M7B", "M7C", "M7D", "M7X", "M7Y", "M7HOFS", "M7VOFS", "BG1HOFS", "BG1VOFS", "BG2HOFS", "BG2VOFS",
  "BG3HOFS", "BG3VOFS", "BG4HOFS", "BG4VOFS", "COLDATA red", "COLDATA green", "COLDATA blue"};

// Pure byte registers: their state is the last byte written.
static const uint16_t byteRegs[] = {0x212C, 0x212D, 0x2123, 0x2124, 0x2125, 0x2130, 0x2131,
                                    0x4200, 0x4207, 0x4208, 0x4209, 0x420A,
                                    0x2101, 0x2105, 0x2107, 0x2108, 0x2109, 0x210A, 0x210B, 0x210C,
                                    0x2100, 0x2102, 0x2103, 0x2115, 0x420C,
                                    0x2106, 0x211A, 0x2126, 0x2127, 0x2128, 0x2129, 0x212A, 0x212B,
                                    0x212E, 0x212F, 0x2133, 0x420D,
                                    0x2181, 0x2182, 0x2183};  // (harness-14: the WRAM port's address)
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
  uint16_t altRetPc, altRetSp;  // mayLeaveCaller: the caller's own return
  bool leftCaller;
  uint8_t retBank;
  uint32_t interruptsAtEntry, insns;
  bool hwAccess, mathWritten, leftExtent, setupPending, vramAdrPending, effectOverflow;
  bool vramLowSet;  // a $2118 byte waiting for its $2119 half
  uint8_t vramLow;
  bool cgLowSet, cgAdrPending;  // a $2122 byte waiting for its second one; $2121 set, unused
  uint16_t vramAdr;  // the address bytes last written to $2116/$2117, and $2121
  uint8_t cgAdr;
  uint8_t cgLow;
  // reads of input registers, in order (served to the C by difftest_input)
  InputRead* inputs;  // (sampleInputs[depth]; inputCap of them)
  int nInputs, inputCap;
  bool inputOverflow;
  // where the routine is in a DSP-4 command (see dsp4Step): phase 0 idle, 1 half of the
  // command word, 2 parameters, 3 results; the command and the bytes of the phase so far
  uint8_t dsp4Phase, dsp4Cmd, dsp4Count;
  uint16_t dsp4Flag;  // the result's first word so far (dsp4-03c)
  // a taped row's DSP-4 data-port bytes, in order, in tapes[depth] (the C consumes them:
  // difftest_dsp4_byte); kept out of the sample, which every entry clears
  int nTape;
  bool tapeOverflow;
  uint32_t hwAccessAdr, leftExtentPc;
  bool foreignStack;  // a handler it recorded switched to another stack (two-task screens)
  const char* hwWhy;
  Effect* effects;  // hardware effects of the original, from its DMA kicks (sampleEffects[depth]; effectCap)
  int effectCap;
  int effectCount;
  // PPU fields written (bit per PF_*), and those whose last write combined a latch byte
  // written before the sample (the result depends on state the C does not get)
  uint32_t ppuWritten, ppuStale;
  uint16_t ppuStart[PF_COUNT], ppuEnd[PF_COUNT];
  uint32_t ppuPending;  // fields written by the last instruction: read back at the next one
  uint64_t regWritten;  // byte registers written (bit per byteRegs entry) and their last values
  uint8_t regVal[64];
  // DMA channels: registers written in the sample (bits 0-7 for $43x0-$43x7), those left
  // set up after the channel's last kick (0-4 and 7: the sizes are a kick's own), and the
  // values; the CGRAM address at entry (a stream kick records the one at the kick)
  uint8_t chanWritten[8], chanPrepared[8], chanVal[8][8];
  uint16_t cgPtrEntry;
  uint32_t vramEntry;  // the VRAM pointer and VMAIN at entry (vramPack)
  bool m7Latch, scrollLatch, scrollLatch2;  // latch bytes written inside the sample
  int replaySlot;  // the entry state kept in case an interrupt drops the sample, or -1
  bool lagged;  // harness-15: an interrupt it recorded stopped code outside a wait (lagPc)
  uint32_t lagPc;
} Sample;

// harness-15: the waits, as the original's polling loops: an interrupt whose last instruction
// before it lies in one of these ranges (from the poll's target read to its branch) was
// waited for. Elsewhere it stopped the routine's own work (a decompression, a clear, the
// body of a frame loop): a lag frame, which a port that waits in step with the frame does
// not have. Such a sample can differ in its frame count; see finishSample.
static const struct { uint32_t lo, hi; } waitSites[] = {
  {0x808374, 0x80837A},  // wait_frames: ADC $0024 .. BNE
  {0x82841F, 0x828425},  // wait_frames_82
  {0x8ABA91, 0x8ABA99},  // wait_frames_long
  {0x8A9881, 0x8A9889},  // wait_frames_8a9880
  {0x8A8BB7, 0x8A8BBF},  // wait_frames_8a8bb6
  {0x80B953, 0x80B95A},  // wait_flag_76: TSB $76 .. BEQ
  {0x84A4CD, 0x84A4D1},  // wait_03e0
  {0x8ABAE9, 0x8ABAEF},  // fade_out_wait: STA $0196 .. BNE
  {0x808FCB, 0x808FCD},  // shop_item_down: CMP $4E / BCS
  {0x808FF3, 0x808FF5},  // shop_item_up
  {0x809B04, 0x809B08},  // shop_item_slide_row: $8C = $8E
  {0x809B99, 0x809B9D},  // shop_item_slide_list_in
  {0x80B10C, 0x80B115},  // shop_panel_clear
  {0x8AC0E5, 0x8AC0EB},  // title_card_show: STA $0196 .. the $019A poll
  {0x80B95E, 0x80B960},  // queue_unpacked_upload: the $64 poll
  {0x808CE1, 0x808CE3},  // the $64 polls around a wait_frames ($80:8CE1, $80:8CEB)
  {0x808CEB, 0x808CED},
  {0x808EFB, 0x808EFF},  // shop_item_screen: the $8C/$8E poll, D+$4E up to 4, the typed poll, $0024 past D+$4E
  {0x808F32, 0x808F34},
  {0x808F46, 0x808F48},
  {0x808F5F, 0x808F62},
};
static bool atWaitSite(uint32_t pc) {
  for(size_t i = 0; i < sizeof waitSites / sizeof waitSites[0]; i++)
    if(pc >= waitSites[i].lo && pc <= waitSites[i].hi) return true;
  return false;
}
static bool visitedAtStart[MAX_DEPTH][MAX_EXTENT];  // a waiting row's paths before the sample
static uint32_t prevInsnPc;  // the instruction before the current one (a handler's first: the stopped one)

static Sample samples[MAX_DEPTH];
static Effect sampleEffects[MAX_DEPTH][MAX_EFFECTS_LONG];
static InputRead sampleInputs[MAX_DEPTH][MAX_INPUTS_LONG];

// Replays. A live sample an interrupt lands in is dropped (the handler changes memory the C
// does not model). While a routine has no completed live sample yet, its entry states
// (WRAM, PPU, DMA, registers) are kept, at most REPLAYS_PER_ROUTINE; the ones whose sample
// an interrupt dropped are run again in the fuzz phase, from that exact state with the
// interrupts off, and compared like any sample. They are counted apart ("replayed"), not
// as live samples: the original did not run them that way in the movie.
#define MAX_REPLAYS 64
#define REPLAYS_PER_ROUTINE 2
typedef struct {
  bool used, kept;  // taken by a sample; kept: its sample was dropped by an interrupt
  int idx;
  Regs regs;
  bool decimal;
  uint8_t ram[TG3000_WRAM_SIZE];
  Ppu ppu;
  Dma dma;
  // harness-29: with the firmware DSP-4 (harness-27), its state at the entry, so that a
  // replay finds the chip where the sample did (mid-command); the clean-room chip is reset
  uint8_t dsp4[8192];
  int dsp4Size;
} Replay;
static Replay* replays;
static bool replaying;

static uint8_t sampleRam[MAX_DEPTH][TG3000_WRAM_SIZE];  // WRAM at each sample's entry
#define MAX_TAPE 16384
static struct { uint8_t val; bool read; } tapes[MAX_DEPTH][MAX_TAPE];  // DSP-4 tapes (dsp4Tape rows)
static int depth;
static bool fuzzing;
static uint8_t afterRam[TG3000_WRAM_SIZE];

// Interrupts inside the sample of a routine that waits for them (at most one such sample
// at a time): the WRAM bytes each handler changed (harness-16: the changes, not two whole
// copies of WRAM), replayed in order by platform_wait_vblank() on the C side.
#define MAX_IRQS 8192       // interrupts a row with a budget (maxInsns) records in one sample
#define MAX_IRQS_DEFAULT 64 // the others: past it a recording stops, and the routines its
                            // handlers call are sampled again (raising it for every row hid
                            // two rows only reached from handlers inside wait_flag_76)
static uint8_t irqPre[TG3000_WRAM_SIZE];  // WRAM as the handler being recorded began
typedef struct { uint32_t adr; uint8_t val; } IrqByte;
static IrqByte* irqBytes;                    // every recorded handler's changes, in order
static size_t irqBytesUsed, irqBytesCap;
static size_t irqFirst[MAX_IRQS + 1];        // handler i's changes: [irqFirst[i], irqFirst[i + 1])
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
  for(size_t k = irqFirst[i]; k < irqFirst[i + 1]; k++) g_ram[irqBytes[k].adr] = irqBytes[k].val;
}

// The handler just recorded returned: keep the bytes it changed. A frame's interrupt (it moved
// the frame counter $0024: the NMI) that stopped the routine outside a wait makes the sample
// lagged (harness-15); a raster IRQ does not (harness-22: it is part of every frame of a
// split screen, and no frame is lost to it).
static uint32_t irqStoppedPc;
static void recordIrqEnd(void) {
  const bool frame = irqPre[0x24] != snes->ram[0x24] || irqPre[0x25] != snes->ram[0x25];
  if(frame && !atWaitSite(irqStoppedPc) && !samples[irqOwner].lagged) {
    samples[irqOwner].lagged = true;
    samples[irqOwner].lagPc = irqStoppedPc;
  }
  if(irqCount == 0) irqBytesUsed = 0;
  irqFirst[irqCount] = irqBytesUsed;
  for(uint32_t a = 0; a < TG3000_WRAM_SIZE; a++) {
    if(irqPre[a] == snes->ram[a]) continue;
    if(irqBytesUsed == irqBytesCap) {
      irqBytesCap = irqBytesCap ? irqBytesCap * 2 : 1u << 16;
      irqBytes = realloc(irqBytes, irqBytesCap * sizeof *irqBytes);
      if(irqBytes == NULL) { fprintf(stderr, "difftest: out of memory for interrupts\n"); exit(1); }
    }
    irqBytes[irqBytesUsed++] = (IrqByte){a, snes->ram[a]};
  }
  irqFirst[++irqCount] = irqBytesUsed;
}

void difftest_replay_interrupt(void) {
  if(irqReplayed >= irqLimit) {  // the C wants a frame the original never waited for
    cWaitedTooLong = true;
    longjmp(cAbort, 1);
  }
  applyIrq(irqReplayed++);
}

// The C's platform calls, recorded while it runs.
static Effect cEffects[MAX_EFFECTS_LONG];
static int cEffectCap;  // the compared sample's
static int cEffectCount;
static bool cEffectOverflow;
static void* cSample;  // the sample the C is being compared with
static uint8_t cChanClaim[8], cChanVal[8][8];
static uint32_t cStreamSize[8];
static int32_t cCgPtr;  // the C's CGRAM address (-1: the one at entry)
// an address the C set and nothing used yet (-1: none), compared with the original's
// $2116/$2117 or $2121 left written but unused
static int32_t cVramAdrClaim, cCgAdrClaim;
static int32_t cVramPtr;  // the C's VRAM pointer after a raw VMDATA DMA (-1: not moved)
static uint64_t cRegWritten;  // (defined with the byte registers below)
static uint8_t cRegVal[64];

// The VRAM state a raw VMDATA DMA writes with: the word pointer, and VMAIN as the PPU keeps
// it (increment after the high byte, the step 1/32/128, the remap mode), flagged present.
static uint32_t vramPack(uint16_t ptr, bool onHigh, uint16_t inc, uint8_t remap) {
  const uint32_t code = inc == 1 ? 0 : inc == 32 ? 1 : 2;
  return 0x80000000u | ptr | (uint32_t)onHigh << 16 | code << 17 | (uint32_t)(remap & 3) << 19;
}
static uint32_t vramNow(void) {
  const Ppu* p = snes->ppu;
  return vramPack(p->vramPointer, p->vramIncrementOnHigh, p->vramIncrement, p->vramRemapMode);
}
static bool cInputUsed[MAX_INPUTS_LONG], cInputMissing;
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

// The C's place on the sample's DSP-4 tape, and its first mismatch.
static int cTapePos;
static char cTapeWhy[160];

// Each register's reads are served in order: a cursor per register (harness-18: a long row
// has hundreds of thousands, so no scan from the start).
typedef struct { uint16_t reg; int pos; } InputCursor;
static InputCursor cInputPos[32];
static int cInputPosCount;
static uint8_t inputFor(void* p, uint16_t reg) {
  Sample* smp = p;
  int c = 0;
  while(c < cInputPosCount && cInputPos[c].reg != reg) c++;
  if(c == cInputPosCount && cInputPosCount < 32) cInputPos[cInputPosCount++] = (InputCursor){reg, 0};
  for(int i = c < 32 ? cInputPos[c].pos : 0; i < smp->nInputs; i++) {
    if(smp->inputs[i].reg != reg || cInputUsed[i]) continue;
    cInputUsed[i] = true;
    if(c < 32) cInputPos[c].pos = i + 1;
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

// The native sites (harness-5): code whose DSP-4 bytes are whole commands of src/dsp4.c, not
// any tape's. Every untaped row's extent (with its tail ranges) and the ranges a taped row
// declares (dsp4Native), from the whole registry (not the rows this run tests), by the PC of
// the accessing instruction. A bitmap over the LoROM halves of banks $00-$7F.
static uint8_t nativeSite[0x80][0x8000 / 8];
static uint32_t insnPc;  // the instruction running (set by difftest_on_insn)
static void markNative(uint32_t from, uint32_t to) {
  for(uint32_t a = from; a < to; a++) {
    const uint32_t bank = (a >> 16) & 0x7F, adr = a & 0xFFFF;
    if(adr >= 0x8000) nativeSite[bank][(adr - 0x8000) >> 3] |= (uint8_t)(1u << (adr & 7));
  }
}
static void buildNativeSites(void) {
  memset(nativeSite, 0, sizeof nativeSite);
  for(int i = 0; i < routineCount; i++) {
    const Routine* r = &routines[i];
    if(!r->dsp4Tape) {
      markNative(r->entry, r->end);
      for(int t = 0; t < 2; t++) if(r->alsoIn[t][1]) markNative(r->alsoIn[t][0], r->alsoIn[t][1]);
    }
    for(int t = 0; t < 2; t++) if(r->dsp4Native[t][1]) markNative(r->dsp4Native[t][0], r->dsp4Native[t][1]);
  }
}
// harness-9: the PCs of the registry's declared dead VMDATA reads (vramReadAt)
static bool vramReadSite(uint32_t pc) {
  for(int i = 0; i < routineCount; i++) if(routines[i].vramReadAt && routines[i].vramReadAt == pc) return true;
  return false;
}
static bool nativeAt(uint32_t pc) {
  const uint32_t bank = (pc >> 16) & 0x7F, adr = pc & 0xFFFF;
  return adr >= 0x8000 && (nativeSite[bank][(adr - 0x8000) >> 3] >> (adr & 7) & 1);
}
// For the bridge: in a fuzz case, a native site's bytes go to the real stand-in, not the script
bool difftest_dsp4_native_now(void) { return fuzzing && nativeAt(insnPc); }

uint8_t difftest_dsp4_byte(uint8_t value, bool write) {
  if(!collectingC || cSample == NULL) return 0;
  Sample* smp = cSample;
  const int k = (int)(smp - samples);
  const int pos = cTapePos++;
  if(cTapeWhy[0] != '\0') return 0;
  if(!table[smp->idx].dsp4Tape) {
    snprintf(cTapeWhy, sizeof cTapeWhy, "the C used the DSP-4 port in a row without a tape");
    return 0;
  }
  if(pos >= smp->nTape) {
    snprintf(cTapeWhy, sizeof cTapeWhy, "the C %s DSP-4 byte %d, past the tape's %d", write ? "wrote" : "read", pos, smp->nTape);
    return 0;
  }
  if(tapes[k][pos].read == write) {
    snprintf(cTapeWhy, sizeof cTapeWhy, "DSP-4 tape byte %d: the original %s, the C %s", pos,
      tapes[k][pos].read ? "read" : "wrote", write ? "wrote" : "read");
    return 0;
  }
  if(write && tapes[k][pos].val != value) {
    snprintf(cTapeWhy, sizeof cTapeWhy, "DSP-4 tape byte %d: the original wrote $%02X, the C $%02X", pos, tapes[k][pos].val, value);
    return 0;
  }
  return tapes[k][pos].val;
}

void difftest_dma_claim(int slot, int reg, uint8_t value) {
  if(!collectingC || slot < 0 || slot > 7 || reg < 0 || reg > 7) return;
  cChanClaim[slot] |= (uint8_t)(1u << reg);
  cChanVal[slot][reg] = value;
}
void difftest_stream_size(int slot, uint32_t size) { if(collectingC && slot >= 0 && slot < 8) cStreamSize[slot] = size; }
void difftest_cgram_address(uint8_t index) {
  if(!collectingC) return;
  cCgPtr = index;
  cCgAdrClaim = index;
}
void difftest_vram_address(uint16_t word) { if(collectingC) cVramAdrClaim = word; }

// A stream kick, packed: target = mask | CGRAM address << 8; src = the sizes of slots 0-3
// (a byte each), size = those of slots 4-7. Sizes above 255 are not modelled.
static Effect streamEffect(uint8_t mask, uint16_t cgPtr, const uint32_t* sizes) {
  uint32_t lo = 0, hi = 0;
  for(int c = 0; c < 8; c++) {
    if(!(mask & (1 << c))) continue;
    uint32_t s = sizes[c] > 255 ? 255 : sizes[c];
    if(c < 4) lo |= s << (8 * c); else hi |= s << (8 * (c - 4));
  }
  return (Effect){EFFECT_STREAM, (uint32_t)mask | (uint32_t)cgPtr << 8, lo, hi};
}

static uint8_t peekByte(uint32_t adr);

// Slots the C set up in this sample (mode, target, source claimed) are sent like the
// original's uploads, decoded the same way; the others are stream kicks. Either way a
// kicked slot's setup is used up (no longer left set up).
void difftest_stream_kick(uint8_t mask) {
  if(!collectingC) return;
  // the CGRAM address is used only by slots to $2121/$2122: those the C set up here by
  // their claimed target, the others by the target the routine left them with
  for(int c = 0; c < 8; c++) {
    if(!(mask & (1 << c))) continue;
    uint8_t target = (cChanClaim[c] & 0x02) ? cChanVal[c][1] : snes->dma->channel[c].bAdr;
    if(target == 0x21 || target == 0x22) cCgAdrClaim = -1;
  }
  uint16_t ptr = cCgPtr >= 0 ? (uint16_t)cCgPtr : ((Sample*)cSample)->cgPtrEntry;
  uint8_t stream = 0;
  for(int c = 0; c < 8; c++) {
    if(!(mask & (1 << c))) continue;
    if((cChanClaim[c] & 0x1F) != 0x1F) { stream |= (uint8_t)(1 << c); continue; }
    uint8_t dmap = cChanVal[c][0], target = cChanVal[c][1];
    uint32_t src = (uint32_t)cChanVal[c][4] << 16 | (uint32_t)cChanVal[c][3] << 8 | cChanVal[c][2];
    uint32_t size = cStreamSize[c] ? cStreamSize[c] : 0x10000;
    bool fromB = dmap & 0x80, fixed = dmap & 0x08, decrement = dmap & 0x10;
    Effect e = {EFFECT_RAW_DMA, ((uint32_t)target << 8) | (dmap & 7), src, size};
    if(!fromB && !fixed && !decrement && target == 0x22 && ((dmap & 7) == 0 || (dmap & 7) == 2))
      e = (Effect){EFFECT_CGRAM, ptr, src, size};
    if(e.kind == EFFECT_RAW_DMA && !fromB && (target == 0x18 || target == 0x19)) {
      // the VRAM state as the C left it: its address, else where its last such DMA ended,
      // else the entry's; VMAIN as the C wrote it, else the entry's
      const Sample* sm = (const Sample*)cSample;
      uint32_t st = sm->vramEntry;
      bool onHigh = (st >> 16) & 1;
      uint16_t inc = ((st >> 17) & 3) == 0 ? 1 : ((st >> 17) & 3) == 1 ? 32 : 128;
      uint8_t remap = (uint8_t)((st >> 19) & 3);
      for(int i = 0; i < N_BYTE_REGS; i++) {
        if(byteRegs[i] != 0x2115 || !(cRegWritten & (1ull << i))) continue;
        const uint8_t v = cRegVal[i];
        onHigh = (v & 0x80) != 0;
        inc = (v & 3) == 0 ? 1 : (v & 3) == 1 ? 32 : 128;
        remap = (uint8_t)((v >> 2) & 3);
      }
      const uint16_t at = cVramAdrClaim >= 0 ? (uint16_t)cVramAdrClaim : cVramPtr >= 0 ? (uint16_t)cVramPtr : (uint16_t)st;
      e.vram = vramPack(at, onHigh, inc, remap);
      // the pointer moves on with each write to the byte that increments it
      uint32_t steps = 0;
      // (harness-21: modes 2 and 6, one register written twice per unit, step as mode 0)
      if((dmap & 7) == 0 || (dmap & 7) == 2 || (dmap & 7) == 6) steps = ((target == 0x19) == onHigh) ? size : 0;
      else if((dmap & 7) == 1 && target == 0x18) steps = onHigh ? size / 2 : (size + 1) / 2;
      cVramPtr = (uint16_t)(at + steps * inc);
      cVramAdrClaim = -1;
    }
    if(!fromB && target == 0x22) ptr = (uint16_t)((ptr + size / 2) & 0xFF);
    else if(!fromB && target == 0x21) ptr = peekByte(fixed ? src : src + size - 1);
    if(cEffectCount >= cEffectCap) { cEffectOverflow = true; return; }
    cEffects[cEffectCount++] = e;
    cChanClaim[c] = 0;
  }
  cCgPtr = ptr;
  if(!stream) return;
  if(cEffectCount >= cEffectCap) { cEffectOverflow = true; return; }
  cEffects[cEffectCount++] = streamEffect(stream, ptr, cStreamSize);
}

void difftest_effect(int kind, uint32_t target, uint32_t src, uint32_t size) {
  if(!collectingC) return;
  if(kind == EFFECT_VRAM || kind == EFFECT_VRAM_FILL || kind == EFFECT_VRAM_WORD) cVramAdrClaim = -1;
  if(kind == EFFECT_CGRAM || kind == EFFECT_CGRAM_WORD) cCgAdrClaim = -1;
  if(cEffectCount >= cEffectCap) { cEffectOverflow = true; return; }
  cEffects[cEffectCount++] = (Effect){kind, target, src, size};
}

static void vramWordWrite(uint32_t adr, uint8_t val);
static void cgramWordWrite(uint32_t adr, uint8_t val);
static void mode7ProductRead(uint32_t adr);

static uint32_t cPpuWritten;
static uint16_t cPpu[PF_COUNT];
static uint64_t cRegWritten;
static uint8_t cRegVal[64];

void difftest_reg(uint16_t adr, uint8_t value) {
  if(!collectingC) return;
  int i = byteRegIndex(adr);
  if(i < 0) return;
  cRegWritten |= 1ull << i;
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
  bool scroll = a >= 0x210D && a <= 0x2114, m7 = a == 0x210D || a == 0x210E || (a >= 0x211B && a <= 0x2120);
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
         kind == EFFECT_APU ? "APU port write" : kind == EFFECT_CGRAM_WORD ? "CGRAM colour" :
         kind == EFFECT_OAM ? "OAM upload" : kind == EFFECT_STREAM ? "stream kick (mask | CGRAM address << 8, sizes)" :
         "other DMA";
}

// A write of `val` to MDMAEN ($420B): one effect per channel started, read from the
// channel's registers and the PPU's address state as they are at the kick, for every
// sample in progress (an inner routine's upload is also its caller's).
static uint8_t peekByte(uint32_t adr);

// The CGRAM address the kick model expects after a kick (-1: none), checked against the
// PPU at the first instruction after the transfers ran: the model that decodes the
// original's effects must agree with the hardware.
static int32_t cgPtrAfterKick = -1;

static void checkCgPtrAfterKick(void) {
  if(cgPtrAfterKick < 0 || snes->dma->dmaState != 0) return;  // once the kick's transfers have run
  if((uint16_t)cgPtrAfterKick != snes->ppu->cgramPointer) {
    static char msg[96];
    snprintf(msg, sizeof msg, "the CGRAM address after a kick: model %02X, PPU %02X", (unsigned)cgPtrAfterKick,
      snes->ppu->cgramPointer);
    for(int k = 0; k < depth; k++) {
      if(samples[k].hwAccess) continue;
      samples[k].hwAccess = true;
      samples[k].hwWhy = msg;
      samples[k].hwAccessAdr = 0x420B;
    }
  }
  cgPtrAfterKick = -1;
}

static void recordKick(uint8_t val) {
  Effect plainEffect[8];
  uint32_t sizes[8] = {0};
  // the channels of one kick run in order: the CGRAM address moves on with each colour
  // written, and a byte sent to $2121 sets it
  uint16_t cgPtr = snes->ppu->cgramPointer;
  for(int c = 0; c < 8; c++) {
    if(!(val & (1 << c))) continue;
    DmaChannel* ch = &snes->dma->channel[c];
    Ppu* ppu = snes->ppu;
    uint32_t src = ((uint32_t)ch->aBank << 16) | ch->aAdr;
    uint32_t size = ch->size ? ch->size : 0x10000;
    sizes[c] = size;
    bool plain = !ch->fromB && !ch->fixed && !ch->decrement;
    Effect e = {EFFECT_RAW_DMA, ((uint32_t)ch->bAdr << 8) | ch->mode, src, size};
    if(plain && ch->bAdr == 0x18 && ch->mode == 1 && ppu->vramIncrementOnHigh && ppu->vramIncrement == 1 &&
       ppu->vramRemapMode == 0) {
      e = (Effect){EFFECT_VRAM, ppu->vramPointer, src, size};
    } else if(plain && ch->bAdr == 0x22 && (ch->mode == 0 || ch->mode == 2) && !ppu->cgramSecondWrite) {
      e = (Effect){EFFECT_CGRAM, cgPtr, src, size};
    } else if(plain && ch->bAdr == 0x04 && (ch->mode == 0 || ch->mode == 2) && !ppu->oamSecondWrite) {
      // the sprite table, from the OAM word address (bit 8: the high table) set at the kick
      e = (Effect){EFFECT_OAM, (uint32_t)(ppu->oamInHigh ? 0x100 : 0) | ppu->oamAdr, src, size};
    } else if(!ch->fromB && ch->fixed && ch->bAdr == 0x18 && ch->mode == 1 && ppu->vramIncrementOnHigh &&
              ppu->vramIncrement == 1 && ppu->vramRemapMode == 0) {
      // a fixed source writes the same byte to both halves of every word
      e = (Effect){EFFECT_VRAM_FILL, ppu->vramPointer, peekByte(src), size};
    }
    if(e.kind == EFFECT_RAW_DMA && !ch->fromB && (ch->bAdr == 0x18 || ch->bAdr == 0x19)) e.vram = vramNow();
    plainEffect[c] = e;
    if(!ch->fromB && ch->bAdr == 0x22) cgPtr = (uint16_t)((cgPtr + size / 2) & 0xFF);
    else if(!ch->fromB && ch->bAdr == 0x21) cgPtr = peekByte(ch->fixed ? src : src + size - 1);
  }
  cgPtrAfterKick = cgPtr;
  for(int k = accessBase; k < depth; k++) {
    Sample* smp = &samples[k];
    // per channel: set up in this sample (mode, target, source: an upload), or wholly
    // before it (a stream kick: the platform keeps the source going)
    uint8_t stream = 0, plainMask = 0;
    for(int c = 0; c < 8; c++) {
      if(!(val & (1 << c))) continue;
      uint8_t w = smp->chanWritten[c] & 0x1F;
      if(w == 0x1F) plainMask |= (uint8_t)(1 << c);
      else if(w == 0) stream |= (uint8_t)(1 << c);
      else if(!smp->hwAccess) {
        smp->hwAccess = true;
        smp->hwWhy = "a DMA kick on a channel set up partly before the routine (not modelled)";
        smp->hwAccessAdr = 0x420B;
      }
      smp->chanPrepared[c] = 0;
    }
    if(stream && plainMask && !smp->hwAccess) {
      smp->hwAccess = true;
      smp->hwWhy = "one kick of streams and uploads together (not modelled)";
      smp->hwAccessAdr = 0x420B;
    }
    for(int c = 0; c < 8; c++) {
      if(!(plainMask & (1 << c))) continue;
      if(smp->effectCount < smp->effectCap) smp->effects[smp->effectCount++] = plainEffect[c];
      else smp->effectOverflow = true;
    }
    if(stream) {
      for(int c = 0; c < 8; c++) {
        if((stream & (1 << c)) && sizes[c] > 255 && !smp->hwAccess) {
          smp->hwAccess = true;
          smp->hwWhy = "a stream kick of more than 255 bytes (not modelled)";
          smp->hwAccessAdr = 0x420B;
        }
      }
      if(smp->effectCount < smp->effectCap) smp->effects[smp->effectCount++] = streamEffect(stream, snes->ppu->cgramPointer, sizes);
      else smp->effectOverflow = true;
    }
  }
}

// Registers a DMA upload is set up through; on their own they have no visible effect, the
// kick turns them into one.
static bool isUploadSetup(uint32_t adr) {
  if(((adr >> 16) & 0x7F) >= 0x40) return false;
  uint16_t a = adr & 0xFFFF;
  return a == 0x2116 || a == 0x2117 || a == 0x2121;
}

// Flag liveness: after a live sample of a routine returns, its watch follows the caller
// until each flag is read (live) or overwritten (dead). One watch per routine.
typedef struct {
  bool active;
  uint8_t unresolved;
  int count;
  uint32_t interrupts;
  int32_t pausedUntilSp;
  // PHP ... PLP pairs entered since the return: inside one, a flag written is shadowed
  // (the PLP brings the watched value back), not dead; the matching PLP (by SP) restores
  // the shadow mask the PHP saw
  uint8_t shadow;
  int phpDepth;
  struct { uint16_t sp; uint8_t shadow; } php[4];
  bool straight;  // harness-17: a cut's watch still in the straight-line code after it
} Watch;
static Watch* watches;
static void watchStart(int idx);  // (below, with the active-watch list)

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
// when it is made of whole commands the C has (the table below: the command word, its
// parameter bytes, its result bytes; each checked on the real firmware with refbench).
// The multiply's results may also be skipped by a write (the chip's behaviour, measured);
// other commands' results must be read. Status reads are free. Anything else, or a command
// left unfinished, is a hardware access the C cannot reproduce. Returns whether the access
// fits.
// flagEnds: a first result word of 0 is the whole result (dsp4-03c: a hidden sprite).
static const struct { uint8_t cmd, in, out; bool writeSkips, flagEnds; } dsp4Commands[] = {
  {0x00, 4, 4, true, false},   // dsp4_multiply
  {0x0A, 6, 8, false, false},  // dsp4_edges
  {0x11, 8, 2, false, false},  // dsp4_row
  {0x03, 10, 6, false, true},  // dsp4_sprite: its word, then a sprite entry
  {0x0B, 6, 6, false, true},   // dsp4_sprite
};
static bool isDsp4(uint32_t adr) {
  uint8_t bank = (adr >> 16) & 0x7F;
  return bank >= 0x30 && bank < 0x40 && (adr & 0xFFFF) >= 0x8000;
}

// harness-20: no device answers here ($2000-$20FF, $2200-$3FFF, $4400-$7FFF of the system
// banks, outside the DSP-4's): a write is lost, as on the console; the C drops it too
// (bus_wr16). Reads (open bus) stay hardware accesses.
static bool isUnmapped(uint32_t adr) {
  const uint8_t bank = (adr >> 16) & 0x7F;
  const uint16_t lo = adr & 0xFFFF;
  if(bank >= 0x40) return false;
  return (lo >= 0x2000 && lo < 0x2100) || (lo >= 0x2200 && lo < 0x4000) || (lo >= 0x4400 && lo < 0x8000);
}

static int dsp4CommandIndex(uint8_t cmd) {
  for(int i = 0; i < (int)(sizeof dsp4Commands / sizeof dsp4Commands[0]); i++) if(dsp4Commands[i].cmd == cmd) return i;
  return -1;
}

static bool dsp4Step(Sample* smp, uint32_t adr, uint8_t val, bool write) {
  if((adr & 0xFFFF) >= 0xC000) return !write;          // status: reads only
  switch(smp->dsp4Phase) {
  case 0:  // the command word's low byte
    if(!write || dsp4CommandIndex(val) < 0) return false;
    smp->dsp4Cmd = val;
    smp->dsp4Phase = 1;
    break;
  case 1:  // its high byte: 0
    if(!write || val != 0) return false;
    smp->dsp4Phase = 2;
    smp->dsp4Count = 0;
    break;
  case 2: {  // the parameters
    if(!write) return false;
    const int i = dsp4CommandIndex(smp->dsp4Cmd);
    if(++smp->dsp4Count == dsp4Commands[i].in) { smp->dsp4Phase = 3; smp->dsp4Count = 0; smp->dsp4Flag = 0; }
    break;
  }
  default: {  // the results: a read takes one (a write skips one, for the multiply)
    const int i = dsp4CommandIndex(smp->dsp4Cmd);
    if(write && !dsp4Commands[i].writeSkips) return false;
    if(smp->dsp4Count < 2) smp->dsp4Flag |= (uint16_t)(val << (8 * smp->dsp4Count));
    if(++smp->dsp4Count == dsp4Commands[i].out || (dsp4Commands[i].flagEnds && smp->dsp4Count == 2 && smp->dsp4Flag == 0)) {
      smp->dsp4Phase = 0;
      smp->dsp4Count = 0;
    }
    break;
  }
  }
  return true;
}

void difftest_on_access(uint32_t adr, uint8_t val, bool write) {
  // a CGRAM access right after a kick: the check must see the address before it moves
  if(cgPtrAfterKick >= 0 && ((adr >> 16) & 0x7F) < 0x40 && ((adr & 0xFFFF) == 0x2121 || (adr & 0xFFFF) == 0x2122 ||
                             (adr & 0xFFFF) == 0x213B))
    checkCgPtrAfterKick();
  // a handler recorded for a waiting sample: its hardware is its own, and only the samples
  // that began inside it see it
  accessBase = (irqOwner >= 0 && irqActive) ? irqBaseDepth : 0;
  if(depth <= accessBase) return;
  if(isDsp4(adr)) {  // before the ROM test: $30:8000 reads look like ROM reads to it
    for(int k = accessBase; k < depth; k++) {
      Sample* smp = &samples[k];
      if(table[smp->idx].dsp4Tape && !nativeAt(insnPc)) {  // its tape: the data bytes in order; status reads free
        if((adr & 0xFFFF) >= 0xC000) {
          if(!write) continue;
        } else if(smp->nTape < MAX_TAPE) {
          tapes[k][smp->nTape].val = val;
          tapes[k][smp->nTape++].read = !write;
          continue;
        } else {
          smp->tapeOverflow = true;
          continue;
        }
      }
      if(smp->hwAccess || dsp4Step(smp, adr, val, write)) continue;
      smp->hwWhy = "DSP-4 traffic other than whole commands of src/dsp4.c";
      smp->hwAccess = true;
      smp->hwAccessAdr = adr;
    }
    return;
  }
  if(write && isUnmapped(adr)) return;  // (harness-20)
  if(isWram(adr) || (!write && isRom(adr))) return;
  if(write && ppuFieldWrite(adr, val)) return;
  if(write && ((adr >> 16) & 0x7F) < 0x40 && (adr & 0xFFFF) == 0x2122) { cgramWordWrite(adr, val); return; }
  int bri = write ? byteRegIndex(adr) : -1;
  if(bri >= 0) {
    for(int k = accessBase; k < depth; k++) {
      samples[k].regWritten |= 1ull << bri;
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
        if(smp->effectCount < smp->effectCap) smp->effects[smp->effectCount++] = (Effect){EFFECT_APU, (adr & 3), val, 1};
        else smp->effectOverflow = true;
      } else {
        // a value repeated by a polling loop is one input: only changes are recorded, and
        // (harness-18) a repeat after an interrupt, a frame's own read of it
        uint16_t reg = apuPort ? (uint16_t)(0x2140 + (adr & 3)) : lo;
        int last = -1;
        for(int i = smp->nInputs - 1; i >= 0 && last < 0; i--) if(smp->inputs[i].reg == reg) last = i;
        // serial joypad reads are bits: every one counts
        if(reg != 0x4016 && reg != 0x4017 && last >= 0 && smp->inputs[last].val == val &&
           smp->inputs[last].frame == cpu_interruptCount) continue;
        if(smp->nInputs < smp->inputCap) smp->inputs[smp->nInputs++] = (InputRead){reg, val, cpu_interruptCount};
        else smp->inputOverflow = true;
      }
    }
    return;
  }
  if(((adr >> 16) & 0x7F) < 0x40 && lo >= 0x4300 && lo <= 0x437F) {  // a DMA channel register
    int c = (lo >> 4) & 7, r = lo & 0xF;
    for(int k = accessBase; k < depth; k++) {
      Sample* smp = &samples[k];
      if(!write || r >= 8) {
        if(!smp->hwAccess) {
          smp->hwAccess = true;
          smp->hwWhy = write ? "writes a channel's HDMA counters ($43x8-$43xF)" : "reads a DMA channel register";
          smp->hwAccessAdr = adr;
        }
        continue;
      }
      smp->chanWritten[c] |= (uint8_t)(1u << r);
      if(r != 5 && r != 6) {
        smp->chanPrepared[c] |= (uint8_t)(1u << r);
        smp->chanVal[c][r] = val;
      }
    }
    return;
  }
  if(write && ((adr >> 16) & 0x7F) < 0x40 && (adr & 0xFFFF) == 0x420B) {
    recordKick(val);
    // a kick uses the VRAM address only through a channel to $2118/$2119, the CGRAM address
    // only through one to $2121/$2122
    bool vram = false, cg = false;
    for(int c = 0; c < 8; c++) {
      if(!(val & (1 << c))) continue;
      uint8_t b = snes->dma->channel[c].bAdr;
      vram |= b == 0x18 || b == 0x19;
      cg |= b == 0x21 || b == 0x22;
    }
    for(int k = accessBase; k < depth; k++) {
      samples[k].setupPending = false;
      if(vram) samples[k].vramAdrPending = false;
      if(cg) samples[k].cgAdrPending = false;
    }
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
    // harness-14: a read of the WRAM port ($2180) is a WRAM read at its address ($2181-$2183,
    // compared as byte registers), which the C reads directly
    if(!write && ((adr >> 16) & 0x7F) < 0x40 && lo16 == 0x2180) continue;
    if(isMathReg(adr, write)) {
      if(write) { smp->mathWritten = true; continue; }
      if(smp->mathWritten) continue;
      smp->hwWhy = "reads a multiply/divide result it did not start";
    } else if(write && isUploadSetup(adr)) {
      uint16_t a16 = adr & 0xFFFF;
      if(a16 == 0x2116 || a16 == 0x2117) {  // a DMA or a word write uses it
        smp->vramAdrPending = true;
        smp->vramAdr = a16 == 0x2116 ? (uint16_t)((smp->vramAdr & 0xFF00) | val) : (uint16_t)((smp->vramAdr & 0x00FF) | val << 8);
      } else if(a16 == 0x2121) {  // a DMA or a colour write uses it
        smp->cgAdrPending = true;
        smp->cgAdr = val;
      }
      else smp->setupPending = true;
      continue;
    } else if(!write && (lo16 == 0x2139 || lo16 == 0x213A) && vramReadSite(insnPc)) {
      continue;  // harness-9: a declared dead VMDATA read; the PPU's pointer takes its step
    } else {
      smp->hwWhy = "hardware access (not supported by this routine's C)";
    }
    smp->hwAccess = true;
    smp->hwAccessAdr = adr;
  }
}

// A CPU write to VMDATA ($2118 low, $2119 high): with VMAIN = $80 or $81 (increment by one
// word or by 32 words, a column, after the high byte, no remapping) each pair is one word
// written at the VRAM address
// (the hook runs before the PPU takes the write, so the pointer is this word's). The
// console only stores it in vblank or under forced blank; a word written while the screen
// is drawn is lost there, which the C (no scanlines) cannot reproduce.
static void vramWordWrite(uint32_t adr, uint8_t val) {
  const Ppu* ppu = snes->ppu;
  bool mode = ppu->vramIncrementOnHigh && (ppu->vramIncrement == 1 || ppu->vramIncrement == 32) && ppu->vramRemapMode == 0;
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
    if(smp->effectCount < smp->effectCap)
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
    if(smp->effectCount < smp->effectCap)
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
  if(n > MAX_EXTENT) {  // its paths and visits would not fit (they were cut off, and overran)
    fprintf(stderr, "difftest: %s's extent ($%06X-$%06X) is longer than MAX_EXTENT ($%X)\n", r->name, r->entry,
            r->end, MAX_EXTENT);
    exit(2);
  }
  bool m8 = !r->needM16, x8 = !r->needX16;
  for(uint32_t i = 0; i < n;) {
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
  memset(cChanClaim, 0, sizeof cChanClaim);
  memset(cStreamSize, 0, sizeof cStreamSize);
  cCgPtr = -1;
  cVramAdrClaim = cCgAdrClaim = -1;
  cVramPtr = -1;
  cSample = smp;
  cEffectCap = smp->effectCap;
  memset(cInputUsed, 0, (size_t)smp->nInputs * sizeof cInputUsed[0]);
  cInputPosCount = 0;
  cInputMissing = false;
  cTapePos = 0;
  cTapeWhy[0] = '\0';
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
  if(cTapeWhy[0] != '\0') {
    snprintf(why, whyLen, "%s", cTapeWhy);
    return false;
  }
  if(r->dsp4Tape && smp->tapeOverflow) {
    snprintf(why, whyLen, "the original's DSP-4 tape overflowed (%d bytes)", smp->nTape);
    return false;
  }
  if(r->dsp4Tape && cTapePos != smp->nTape) {
    snprintf(why, whyLen, "the C used %d of the DSP-4 tape's %d bytes", cTapePos, smp->nTape);
    return false;
  }
  if(cEffectOverflow) {
    snprintf(why, whyLen, "the C made more than %d hardware effects", cEffectCap);
    return false;
  }
  for(int i = 0; i < smp->effectCount || i < cEffectCount; i++) {
    if(i >= smp->effectCount || i >= cEffectCount) {
      snprintf(why, whyLen, "%d hardware effect(s) in the original, %d from the C", smp->effectCount, cEffectCount);
      return false;
    }
    Effect* o = &smp->effects[i];
    Effect* c = &cEffects[i];
    if(o->kind != c->kind || o->target != c->target || o->src != c->src || o->size != c->size || o->vram != c->vram) {
      snprintf(why, whyLen, "effect %d: original %s $%X <- $%06X x%u (VRAM %06X), C %s $%X <- $%06X x%u (VRAM %06X)", i,
        effectName(o->kind), o->target, o->src, o->size, o->vram, effectName(c->kind), c->target, c->src, c->size, c->vram);
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
    snprintf(why, whyLen, "byte registers written: original %llX, C %llX", (unsigned long long)smp->regWritten,
      (unsigned long long)cRegWritten);
    return false;
  }
  for(int i = 0; i < N_BYTE_REGS; i++) {
    if((smp->regWritten & (1ull << i)) && smp->regVal[i] != cRegVal[i]) {
      snprintf(why, whyLen, "$%04X: original %02X, C %02X", byteRegs[i], smp->regVal[i], cRegVal[i]);
      return false;
    }
  }
  if(smp->vramAdrPending != (cVramAdrClaim >= 0) || (smp->vramAdrPending && smp->vramAdr != (uint16_t)cVramAdrClaim)) {
    snprintf(why, whyLen, "VRAM address left set: original %s $%04X, C %s $%04X", smp->vramAdrPending ? "yes" : "no",
      smp->vramAdr, cVramAdrClaim >= 0 ? "yes" : "no", (unsigned)(cVramAdrClaim & 0xFFFF));
    return false;
  }
  if(smp->cgAdrPending != (cCgAdrClaim >= 0) || (smp->cgAdrPending && smp->cgAdr != (uint8_t)cCgAdrClaim)) {
    snprintf(why, whyLen, "CGRAM address left set: original %s $%02X, C %s $%02X", smp->cgAdrPending ? "yes" : "no",
      smp->cgAdr, cCgAdrClaim >= 0 ? "yes" : "no", (unsigned)(cCgAdrClaim & 0xFF));
    return false;
  }
  for(int c = 0; c < 8; c++) {
    if(smp->chanPrepared[c] != cChanClaim[c]) {
      snprintf(why, whyLen, "DMA channel %d left set up: original registers %02X, C %02X (bits: $43x0-$43x7)", c,
        smp->chanPrepared[c], cChanClaim[c]);
      return false;
    }
    for(int r = 0; r < 8; r++) {
      if((smp->chanPrepared[c] & (1u << r)) && smp->chanVal[c][r] != cChanVal[c][r]) {
        snprintf(why, whyLen, "$43%d%d: original %02X, C %02X", c, r, smp->chanVal[c][r], cChanVal[c][r]);
        return false;
      }
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
  CHECK(leftCaller, r->mayLeaveCaller)
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
  after.leftCaller = smp->leftCaller;
  if(table[idx].waitsForInterrupts) {
    if(!owner || irqOverflow || irqActive) { s->abandoned++; return; }
  } else if(cpu_interruptCount != smp->interruptsAtEntry) {
    s->interrupted++;
    if(smp->replaySlot >= 0) replays[smp->replaySlot].kept = true;  // run it again later
    // not compared, but its caller's use of the flags is the same (harness-6): watch it
    Watch* w = &watches[idx];
    if(!fuzzing && !w->active) {
      watchStart(idx);
      s->watched++;
    }
    return;
  }
  char why[320] = "";
  bool ok;
  uint8_t flagDiff = 0;
  if(!smp->hwAccess && smp->dsp4Phase != 0) {
    static char unfinished[96];
    snprintf(unfinished, sizeof unfinished, "a DSP-4 command $%02X left unfinished (phase %d, %d bytes)", smp->dsp4Cmd,
      smp->dsp4Phase, smp->dsp4Count);
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
    snprintf(why, sizeof why, "more than %d input reads", smp->inputCap);
    ok = false;
  } else if(smp->effectOverflow) {
    snprintf(why, sizeof why, "more than %d hardware effects", smp->effectCap);
    ok = false;
  } else if(smp->vramLowSet) {
    snprintf(why, sizeof why, "VRAM low byte written without its high byte (not modelled)");
    ok = false;
  } else if(smp->cgLowSet) {
    snprintf(why, sizeof why, "a colour's first byte written without its second (not modelled)");
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
  // harness-15: a lagged sample that fails is set apart (its own count; its paths and flags
  // not credited); it fails the routine only when no sample without lag passed
  if(!ok && smp->lagged && !fuzzing && !replaying) {
    s->lagFail++;
    memcpy(s->visited, visitedAtStart[k], MAX_EXTENT);
    if(s->firstLagFailure[0] == '\0')
      snprintf(s->firstLagFailure, sizeof s->firstLagFailure, "%s at D=%04X A=%04X (lag frame at $%06X): %s", table[idx].name,
        smp->entryRegs.d, smp->entryRegs.a, smp->lagPc, why);
    return;
  }
  if(ok && !smp->lagged && !fuzzing && !replaying) s->cleanPass++;
  for(int f = 0; f < 4; f++) {
    if(!(flagDiff & flagBits[f])) continue;
    s->flagMismatch[f]++;
    if(s->firstFlagFailure[f][0] == '\0') {
      bool orig = f == 0 ? after.n : f == 1 ? after.v : f == 2 ? after.z : after.c;
      snprintf(s->firstFlagFailure[f], sizeof s->firstFlagFailure[f], "%s%c: original %d, C %d (D=%04X A=%04X)",
        replaying ? "replay " : fuzzing ? "fuzz " : "", flagNames[f], (int)orig, (int)!orig, smp->entryRegs.d,
        smp->entryRegs.a);
    }
  }
  Watch* w = &watches[idx];
  if(!fuzzing && !w->active) {  // watch which flags the caller goes on to read
    watchStart(idx);
    s->watched++;
  }
  if(replaying) { if(ok) s->replayPass++; else s->replayFail++; }
  else if(fuzzing) { if(ok) s->fuzzPass++; else s->fuzzFail++; }
  else { s->samples++; if(ok) s->pass++; else s->fail++; if(ok && smp->foreignStack) s->foreignStack++; }
  if(!ok && s->firstFailure[0] == '\0') {
    snprintf(s->firstFailure, sizeof s->firstFailure, "%s%s at D=%04X A=%04X X=%04X Y=%04X: %s",
      replaying ? "replay " : fuzzing ? "fuzz " : "", table[idx].name, smp->entryRegs.d, smp->entryRegs.a, smp->entryRegs.x,
      smp->entryRegs.y, why);
  }
}

// A slot for an entry state: free, or taken by a sample that is no longer running and was
// not dropped by an interrupt. -1 when none (or the routine has its share).
static int replaySlotFor(int idx) {
  if(replays == NULL) replays = calloc(MAX_REPLAYS, sizeof *replays);
  int mine = 0, slot = -1;
  for(int q = 0; q < MAX_REPLAYS; q++) {
    Replay* p = &replays[q];
    bool active = false;
    for(int k = 0; k < depth; k++) if(samples[k].replaySlot == q) active = true;
    if(p->used && (p->kept || active)) { if(p->idx == idx) mine++; continue; }
    if(slot < 0) slot = q;
  }
  return mine < REPLAYS_PER_ROUTINE ? slot : -1;
}

static void startSample(int idx, const Cpu* cpu) {
  const Routine* r = &table[idx];
  int slot = -1;
  if(!fuzzing && !r->waitsForInterrupts && !r->rti && stats[idx].samples == 0) slot = replaySlotFor(idx);
  int k = depth++;
  Sample* smp = &samples[k];
  memcpy(sampleRam[k], snes->ram, TG3000_WRAM_SIZE);
  if(r->waitsForInterrupts) memcpy(visitedAtStart[k], stats[idx].visited, MAX_EXTENT);  // (harness-15)
  uint16_t sp = cpu->sp;
  *smp = (Sample){
    .idx = idx, .entryRegs = readRegs(cpu), .entrySp = sp, .minSp = sp,
    .retPc = (uint16_t)((snes->ram[(uint16_t)(sp + 1)] | (snes->ram[(uint16_t)(sp + 2)] << 8)) + 1),
    .retBank = r->rtl ? snes->ram[(uint16_t)(sp + 3)] : cpu->k,
    .retSp = (uint16_t)(sp + (r->rtl ? 3 : 2)),
    .interruptsAtEntry = cpu_interruptCount,
  };
  smp->replaySlot = slot;
  smp->effects = sampleEffects[k];
  smp->inputs = sampleInputs[k];
  smp->effectCap = (r->manyEffects || (r->waitsForInterrupts && r->maxInsns)) ? MAX_EFFECTS_LONG : MAX_EFFECTS;
  smp->inputCap = (r->waitsForInterrupts && r->maxInsns) ? MAX_INPUTS_LONG : MAX_INPUTS;
  smp->vramEntry = vramNow();
  if(slot >= 0) {
    Replay* p = &replays[slot];
    p->used = true;
    p->kept = false;
    p->idx = idx;
    p->regs = smp->entryRegs;
    p->regs.sp = sp;
    p->decimal = cpu->d;
    memcpy(p->ram, snes->ram, TG3000_WRAM_SIZE);
    memcpy(&p->ppu, snes->ppu, sizeof p->ppu);
    memcpy(&p->dma, snes->dma, sizeof p->dma);
    p->dsp4Size = 0;
    if(dsp4_bridge_firmware_on() && dsp4_bridge_state_size() <= (int)sizeof p->dsp4) {
      p->dsp4Size = dsp4_bridge_state_size();
      dsp4_bridge_save_state(p->dsp4);
    }
  }
  if(r->mayLeaveCaller) {  // the caller's return address, above this routine's
    smp->altRetPc = (uint16_t)((snes->ram[(uint16_t)(sp + 3)] | (snes->ram[(uint16_t)(sp + 4)] << 8)) + 1);
    smp->altRetSp = (uint16_t)(sp + 4);
  }
  if(r->rti) {  // the interrupt frame (P, PC low, PC high, PB) above what the prologue pushed
    uint16_t f = (uint16_t)(sp + r->rtiFrame);
    smp->retPc = (uint16_t)(snes->ram[(uint16_t)(f + 2)] | (snes->ram[(uint16_t)(f + 3)] << 8));
    smp->retBank = snes->ram[(uint16_t)(f + 4)];
    smp->retSp = (uint16_t)(f + 4);
  }
  for(int f = 0; f < PF_COUNT; f++) smp->ppuStart[f] = ppuFieldValue(f);
  smp->cgPtrEntry = snes->ppu->cgramPointer;
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

// The rows whose watch is active (harness-11: watchAll steps these, not the whole table). A
// row is added when its watch starts; it leaves the list when watchAll finds it inactive.
static int* activeWatches;
static int activeWatchCount;
static void watchStart(int idx) {
  watches[idx] = (Watch){true, FN | FV | FZ | FC, 0, cpu_interruptCount, -1};
  // harness-17: a sample that ended at its endAt (not at a return): the code after the cut
  // is the same every time, so up to its first branch one watch has seen all of it
  watches[idx].straight = (table[idx].endAt != 0 && insnPc == norm(table[idx].endAt)) ||
                          (table[idx].endAt2 != 0 && insnPc == norm(table[idx].endAt2));
  activeWatches[activeWatchCount++] = idx;
}

static void resolveFlags(int idx, uint8_t mask, bool live) {
  Watch* w = &watches[idx];
  Stats* s = &stats[idx];
  for(int f = 0; f < 4; f++) {
    if(!(w->unresolved & mask & flagBits[f])) continue;
    if(live) s->live[f]++; else s->dead[f]++;
    if(!live && w->straight) s->straightDead[f]++;
    w->unresolved &= ~flagBits[f];
  }
  if(w->unresolved == 0) w->active = false;
}

// One instruction after a live sample of routine idx returned: which flags does it read
// before writing them? Interrupt handlers are skipped (they restore P with RTI).
static void watchStep(int idx, const Cpu* cpu, uint8_t op, uint8_t reads, uint8_t writes) {
  Watch* w = &watches[idx];
  if(cpu_interruptCount != w->interrupts) {
    w->interrupts = cpu_interruptCount;
    w->straight = false;
    if(w->pausedUntilSp < 0) w->pausedUntilSp = (uint16_t)(cpu->sp + (cpu->e ? 3 : 4));
  }
  if(w->pausedUntilSp >= 0) {
    if(cpu->sp != (uint16_t)w->pausedUntilSp) return;
    w->pausedUntilSp = -1;
  }
  if(++w->count > (table[idx].endAt ? WATCH_CAP_HANDLER : WATCH_CAP)) { resolveFlags(idx, FN | FV | FZ | FC, true); return; }
  if((op == 0x08 || op == 0x28)) w->straight = false;  // (harness-17: PHP, PLP end the straight code)
  if(op == 0x08 && w->phpDepth < 4) {  // PHP: the flags are saved, not read
    w->php[w->phpDepth].sp = cpu->sp;
    w->php[w->phpDepth++].shadow = w->shadow;
    return;
  }
  if(op == 0x28 && w->phpDepth > 0 && cpu->sp == (uint16_t)(w->php[w->phpDepth - 1].sp - 1)) {
    w->shadow = w->php[--w->phpDepth].shadow;  // PLP of that PHP: the saved flags are back
    return;
  }
  reads &= (uint8_t)~w->shadow;  // a shadowed flag holds a value written since the PHP
  if(reads) resolveFlags(idx, reads, true);
  if(w->phpDepth > 0) w->shadow |= writes;
  else if(w->active && writes) resolveFlags(idx, writes, false);
  // the straight-line code ends at a conditional branch, an indirect jump, a return, a
  // break, PHP or PLP (the flags a later path sees depend on the state)
  switch(op) {
    case 0x10: case 0x30: case 0x50: case 0x70: case 0x90: case 0xB0: case 0xD0: case 0xF0:
    case 0x6C: case 0x7C: case 0xDC: case 0xFC: case 0x60: case 0x6B: case 0x40: case 0x00: case 0x02:
    case 0x08: case 0x28:
      w->straight = false;
      break;
    default:
      break;
  }
}

// Inside a handler that a waiting sample records, only the watches of rows that end inside
// the handler (endAt: a prologue's flags flow into its target, up to the RTI) go on.
static void watchAll(const Cpu* cpu, bool inHandler) {
  uint32_t pc = ((uint32_t)cpu->k << 16) | cpu->pc;
  uint8_t op = peekByte(pc);
  uint8_t reads = opFlagsRead[op], writes = opFlagsWritten[op];
  if(op == 0xC2 || op == 0xE2) {
    uint8_t p = peekByte((pc & 0xFF0000) | (uint16_t)(pc + 1));
    writes = (p & 0x80 ? FN : 0) | (p & 0x40 ? FV : 0) | (p & 0x02 ? FZ : 0) | (p & 0x01 ? FC : 0);
  }
  for(int j = 0; j < activeWatchCount;) {
    const int i = activeWatches[j];
    if(watches[i].active && (!inHandler || table[i].endAt)) watchStep(i, cpu, op, reads, writes);
    if(!watches[i].active) activeWatches[j] = activeWatches[--activeWatchCount];  // done: off the list
    else j++;
  }
}

// Every entry of a registered routine counts and is checked against its contract; the first
// SAMPLE_ALL_FIRST and then one in SAMPLE_STRIDE become compared samples, nested inside
// other samples if need be.
// One bit per 24-bit PC: set where some row has its entry (harness-11: checkEntries scans the
// table only there).
static uint8_t* entryBits;

static void checkEntries(Cpu* cpu, uint32_t pc, bool inHandler) {
  if(!(entryBits[(pc & 0xFFFFFF) >> 3] & (1u << (pc & 7)))) return;
  for(int i = 0; i < tableCount; i++) {
    if(table[i].entry != pc) continue;
    if(inHandler && !table[i].rti) {  // inside a recorded handler: only handler rows
      stats[i].hidden++;
      return;
    }
    Stats* s = &stats[i];
    Regs in = readRegs(cpu);
    noteContract(s, &table[i], &in);
    unsigned long e = s->entries++;
    if((e >= SAMPLE_ALL_FIRST && e % SAMPLE_STRIDE != 0) || s->samples >= MAX_SAMPLES || depth >= MAX_DEPTH) return;
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
  insnPc = pc;
  const uint32_t stoppedPc = prevInsnPc;
  prevInsnPc = pc;
  // fuzz and replay cases run with interrupts off, even after the routine itself writes
  // $4200 (harness-13: an NMI the routine enabled lost a case; the write itself is still
  // compared, as a byte register)
  if(fuzzing || replaying) {
    snes->nmiEnabled = snes->hIrqEnabled = snes->vIrqEnabled = false;
    cpu->nmiWanted = cpu->irqWanted = cpu->intWanted = false;  // (a $4200 write in vblank raises one at once)
  }
  checkCgPtrAfterKick();
  if(irqOwner >= 0) {
    if(irqActive && cpu->sp == irqReturnSp) {  // the handler has returned (RTI)
      recordIrqEnd();
      irqActive = false;
    }
    if(cpu_interruptCount != irqSeen) {  // this instruction is a handler's first
      irqSeen = cpu_interruptCount;
      if(!irqActive) {
        if(irqCount >= (table[samples[irqOwner].idx].maxInsns ? MAX_IRQS : MAX_IRQS_DEFAULT)) irqOverflow = true;
        else {
          memcpy(irqPre, snes->ram, TG3000_WRAM_SIZE);
          irqStoppedPc = stoppedPc;  // (the lag test waits for its RTI: harness-22)
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
  // the innermost sample that has returned (normally the innermost one); then the next one
  // out may end on the same instruction too: an endAt that an inner sample returns to
  // (harness-7: race_frame_draw's tail jump comes right after func_82A9B7's RTS)
  for(int k = depth - 1; k >= base; k--) {
    Sample* smp = &samples[k];
    bool ended = ((table[smp->idx].endAt != 0 && pc == table[smp->idx].endAt) ||
                  (table[smp->idx].endAt2 != 0 && pc == table[smp->idx].endAt2)) && k == depth - 1;
    bool left = table[smp->idx].mayLeaveCaller && cpu->pc == smp->altRetPc && cpu->k == smp->retBank &&
                cpu->sp == smp->altRetSp;
    if(left) smp->leftCaller = true;
    if(ended || left || (cpu->pc == smp->retPc && cpu->k == smp->retBank && cpu->sp == smp->retSp)) {
      for(int j = depth - 1; j > k; j--) stats[samples[j].idx].abandoned++;  // left without returning
      depth = k + 1;
      finishSample(k, cpu);
      if(k == 0 || (table[samples[k - 1].idx].endAt != pc && table[samples[k - 1].idx].endAt2 != pc)) break;
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
    if(++smp->insns > ((r->waitsForInterrupts && r->maxInsns) ? r->maxInsns : MAX_INSNS)) {
      for(int j = depth - 1; j >= k; j--) stats[samples[j].idx].abandoned++;
      if(irqOwner >= k) irqOwner = -1;
      depth = k;
      break;
    }
  }
  watchAll(cpu, inHandler);
  if(!fuzzing) checkEntries(cpu, pc, inHandler);
  lastOp = peekByte(((uint32_t)cpu->k << 16) | cpu->pc);
}

static const char* onlyNames;
void difftest_only(const char* names) { onlyNames = names; }
static int shardK, shardN;
void difftest_shard(int k, int n) {
  shardK = k;
  shardN = n;
}

static bool named(const char* list, const char* name) {
  size_t n = strlen(name);
  for(const char* p = list; *p;) {
    const char* e = strchr(p, ',');
    size_t len = e ? (size_t)(e - p) : strlen(p);
    if(len == n && strncmp(p, name, n) == 0) return true;
    if(!e) break;
    p = e + 1;
  }
  return false;
}

// glue-1: the composed rows' table instead of the main one (bench --difftest-composed)
static bool useComposed;
void difftest_composed(void) { useComposed = true; }

void difftest_init(Snes* s, const uint8_t* rom, bool negative) {
  snes = s;
  g_rom = rom;
  table = negative ? negativeRoutines : useComposed ? composedRoutines : routines;
  tableCount = negative ? negativeRoutineCount : useComposed ? composedRoutineCount : routineCount;
  if(onlyNames != NULL) {  // the named rows only
    Routine* kept = calloc((size_t)tableCount, sizeof *kept);
    int n = 0;
    for(int i = 0; i < tableCount; i++) if(named(onlyNames, table[i].name)) kept[n++] = table[i];
    table = kept;
    tableCount = n;
  }
  if(shardN > 1) {  // rows [k * count / n, (k + 1) * count / n)
    int from = shardK * tableCount / shardN, to = (shardK + 1) * tableCount / shardN;
    table += from;
    tableCount = to - from;
  }
  stats = calloc(tableCount, sizeof *stats);
  watches = calloc(tableCount, sizeof *watches);
  activeWatches = calloc((size_t)tableCount + 1, sizeof *activeWatches);
  activeWatchCount = 0;
  entryBits = calloc(1u << 21, 1);
  for(int i = 0; i < tableCount; i++) entryBits[(table[i].entry & 0xFFFFFF) >> 3] |= (uint8_t)(1u << (table[i].entry & 7));
  buildNativeSites();
  for(int i = 0; i < tableCount; i++) findStarts(&table[i], &stats[i]);
}

// harness-28: rows not fuzzed on this movie (bench --fuzz-skip): their fuzz needs a base this
// movie's last RAM is not (the VS race ends in a race; the VS screen's frame needs its own)
static const char* fuzzSkipNames;
void difftest_fuzz_skip(const char* names) { fuzzSkipNames = names; }

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
  // and the same PPU and DMA state: a case that writes PPU registers (video_load_screen
  // writes all of them) must not set the stage for the next routine's cases
  static Ppu ppuBase;
  static Dma dmaBase;
  memcpy(&ppuBase, snes->ppu, sizeof ppuBase);
  memcpy(&dmaBase, snes->dma, sizeof dmaBase);
  for(int i = 0; i < tableCount; i++) {
    const Routine* r = &table[i];
    Stats* s = &stats[i];
    for(int q = 0; replays != NULL && q < MAX_REPLAYS; q++) {
      Replay* p = &replays[q];
      if(!p->used || !p->kept || p->idx != i) continue;
      memcpy(snes->ram, p->ram, sizeof p->ram);
      memcpy(snes->ppu, &p->ppu, sizeof p->ppu);
      memcpy(snes->dma, &p->dma, sizeof p->dma);
      snes->nmiEnabled = false;
      snes->hIrqEnabled = snes->vIrqEnabled = false;
      for(int ch = 0; ch < 8; ch++) snes->dma->channel[ch].hdmaActive = false;
      snes->dma->hdmaInitRequested = snes->dma->hdmaRunRequested = false;
      if(p->dsp4Size > 0) dsp4_bridge_load_state(p->dsp4, p->dsp4Size);  // (harness-29)
      else dsp4_bridge_reset();
      s->replayRequested++;
      Cpu* cpu = snes->cpu;
      cpu->nmiWanted = cpu->irqWanted = cpu->intWanted = false;
      cpu->i = true;
      cpu->d = p->decimal;
      cpu->e = p->regs.e;
      cpu->mf = p->regs.m8; cpu->xf = p->regs.x8;
      cpu->a = p->regs.a; cpu->x = p->regs.x; cpu->y = p->regs.y; cpu->dp = p->regs.d; cpu->db = p->regs.db;
      cpu->n = p->regs.n; cpu->v = p->regs.v; cpu->z = p->regs.z; cpu->c = p->regs.c;
      cpu->sp = p->regs.sp;  // the real stack: the sample ends at the real return
      cpu->k = r->entry >> 16;
      cpu->pc = r->entry & 0xFFFF;
      replaying = true;
      startSample(i, cpu);
      s->visited[0] = true;
      for(int n = 0; n < MAX_INSNS && depth > 0; n++) cpu_runOpcode(cpu);
      if(depth > 0) { depth = 0; irqOwner = -1; }  // lost: counted against it below
      replaying = false;
    }
    if(r->fuzz == NULL || s->contractCount == 0) continue;
    if(fuzzSkipNames != NULL && named(fuzzSkipNames, r->name)) continue;  // (harness-28)
    // Each routine's inputs depend only on its own entry, not on its place in the
    // registry: adding a routine must not change what the others are tested with.
    fuzzSeed = 0x13579BDFu ^ (r->entry * 2654435761u);
    for(int k = 0; k < iterations; k++) {
      // each case starts clean: the same WRAM, and no interrupt or HDMA that an earlier
      // case (or the game) left enabled can run during it
      memcpy(snes->ram, base, sizeof base);
      memcpy(snes->ppu, &ppuBase, sizeof ppuBase);
      memcpy(snes->dma, &dmaBase, sizeof dmaBase);
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
      // sentinel return to $8000 of the entry bank (a BRA * trap), stack at $1F00 (or the
      // row's fuzzStack)
      const uint16_t top = r->fuzzStack ? r->fuzzStack : 0x1F00;
      if(r->rti) {  // an interrupt frame: PB, PC = $8000, a random P; then what the prologue pushed
        snes->ram[top] = r->entry >> 16;
        snes->ram[top - 1] = 0x80; snes->ram[top - 2] = 0x00;
        snes->ram[top - 3] = (uint8_t)(fuzzRandom() & ~0x08);  // decimal mode off
        for(int b = 0; b < r->rtiFrame; b++) snes->ram[top - 4 - b] = (uint8_t)fuzzRandom();
        cpu->sp = (uint16_t)(top - 4 - r->rtiFrame);
      } else if(r->rtl) {
        snes->ram[top] = r->entry >> 16;
        snes->ram[top - 1] = 0x7F; snes->ram[top - 2] = 0xFF;
        cpu->sp = (uint16_t)(top - 3);
      } else {
        snes->ram[top] = 0x7F; snes->ram[top - 1] = 0xFF;
        cpu->sp = (uint16_t)(top - 2);
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

static unsigned long fuzzLostOf(const Stats* s) { return s->fuzzRequested - (s->fuzzPass + s->fuzzFail); }

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
      bool live = s->live[f] > 0 || (s->watched < MIN_WATCHED && !(s->watched > 0 && s->straightDead[f] == s->watched));
      if(live) liveFlags[n++] = flagNames[f];
      if(live && s->flagMismatch[f] > 0) flagsOk = false;
    }
    if(liveFlags[0] == '\0') strcpy(liveFlags, "-");
    snprintf(flagNote, sizeof flagNote, "flags live %s (of %lu returns watched)", liveFlags, s->watched);
    // a fuzz case that was interrupted or abandoned was neither compared nor passed: the
    // harness lost it, and a lost case may hide a deviation (e.g. an input on which the
    // original runs away), so it fails the routine
    unsigned long fuzzLost = s->fuzzRequested - (s->fuzzPass + s->fuzzFail);
    unsigned long replayLost = s->replayRequested - (s->replayPass + s->replayFail);
    bool ok = flagsOk && s->fail == 0 && s->fuzzFail == 0 && fuzzLost == 0 && s->replayFail == 0 && replayLost == 0 &&
              (s->pass + s->fuzzPass + s->replayPass) > 0 && seen == need && !s->contractBroken &&
              (s->lagFail == 0 || s->cleanPass > 0);  // harness-15
    const char* status = !reached ? "unreached" : ok ? "verified" : "failed";
    if(reached) allOk &= ok;  // a routine this run never reached is reported, not failed
    printf("difftest %-16s $%06X: %s  live %lu/%lu pass (%lu entries, %lu interrupted, %lu abandoned), fuzz %lu/%lu (%lu lost), paths %d/%d%s\n",
      r->name, r->entry, !reached ? "UNREACHED" : ok ? "PASS" : "FAIL", s->pass, s->samples, s->entries, s->interrupted, s->abandoned,
      s->fuzzPass, s->fuzzPass + s->fuzzFail, fuzzLost, seen, need, s->contractBroken ? ", ENTRY CONTRACT BROKEN" : "");
    if(s->replayRequested)
      printf("    replayed %lu/%lu interrupted entries from their entry state (%lu lost)\n", s->replayPass,
        s->replayPass + s->replayFail, replayLost);
    for(int c = 0; c < s->contractCount; c++) {
      Contract* k = &s->contracts[c];
      printf("    entry state: D=%04X DB=%02X %s %s%s\n", k->d, k->db, k->m8 ? "m8" : "m16", k->x8 ? "x8" : "x16", k->e ? " emu" : "");
    }
    printf("    %s; mismatches N%lu V%lu Z%lu C%lu\n", flagNote, s->flagMismatch[0], s->flagMismatch[1], s->flagMismatch[2], s->flagMismatch[3]);
    if(s->firstFailure[0]) printf("    first failure: %s\n", s->firstFailure);
    if(s->lagFail)
      printf("    %lu lagged sample(s) failed, set apart (harness-15; %lu passed without lag): %s\n", s->lagFail, s->cleanPass,
        s->firstLagFailure);
    if(s->hidden)
      printf("    %lu entries inside interrupt handlers a waiting sample recorded (not sampled)\n", s->hidden);
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
      bool live = s->live[f] > 0 || (s->watched < MIN_WATCHED && !(s->watched > 0 && s->straightDead[f] == s->watched));
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
      // harness-8: this movie's raw evidence, for tools/merge_verified.py to pool over the
      // movies: the returns watched, the flags seen read (before the MIN_WATCHED rule), the
      // flags with a compared mismatch, a hard failure (anything but the flag and path
      // rules), the cases compared, and the paths visited (one bit per instruction start,
      // in order, as hex)
      char readSet[5] = "", mismSet[5] = "", sdeadSet[5] = "", vis[MAX_EXTENT / 4 + 2] = "";
      for(int f = 0, nr = 0, nm = 0, nd = 0; f < 4; f++) {
        if(s->live[f] > 0) readSet[nr++] = flagNames[f];
        if(s->flagMismatch[f] > 0) mismSet[nm++] = flagNames[f];
        if(s->watched > 0 && s->straightDead[f] == s->watched) sdeadSet[nd++] = flagNames[f];  // (harness-17)
      }
      if(readSet[0] == '\0') strcpy(readSet, "-");
      if(mismSet[0] == '\0') strcpy(mismSet, "-");
      if(sdeadSet[0] == '\0') strcpy(sdeadSet, "-");
      {
        int bit = 0, nib = 0, at = 0;
        for(int k = 0; k < MAX_EXTENT; k++) {
          if(!s->starts[k]) continue;
          if(s->visited[k]) nib |= 8 >> bit;
          if(++bit == 4) { vis[at++] = "0123456789abcdef"[nib]; bit = nib = 0; }
        }
        if(bit) vis[at++] = "0123456789abcdef"[nib];
        if(at == 0) vis[at++] = '-';
        vis[at] = '\0';
      }
      const bool hard = s->fail > 0 || s->fuzzFail > 0 || fuzzLostOf(s) > 0 || s->replayFail > 0 ||
                        (s->replayRequested - (s->replayPass + s->replayFail)) > 0 || s->contractBroken;
      fprintf(out, "%06X %s %s %016llx %lu %lu %d/%d %s live-flags:%s replayed:%lu watched:%lu read:%s mism:%s hard:%d compared:%lu vis:%s lagfail:%lu cleanpass:%lu sdead:%s\n",
        r->entry, r->name, r->source, (unsigned long long)verificationHash(r->source), s->pass, s->fuzzPass, seen, need,
        status, liveFlags, s->replayPass, s->watched, readSet, mismSet, hard ? 1 : 0, s->pass + s->fuzzPass + s->replayPass, vis, s->lagFail, s->cleanPass, sdeadSet);
    }
  }
  if(out != NULL) fclose(out);
  return allOk;
}
