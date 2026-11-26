// SPDX-License-Identifier: GPL-3.0-or-later
// Per-routine differential test: the original routine runs first (recorded from entry to
// return), then the C version runs from the same entry state, and WRAM, registers and
// flags must match. See docs/DECISIONS.md ("Differential test").
#ifndef BENCH_DIFFTEST_H
#define BENCH_DIFFTEST_H

#include <stdbool.h>
#include <stdint.h>

#include "snes.h"

typedef struct {
  uint16_t a, x, y, d;
  uint8_t db;
  bool n, v, z, c, m8, x8, e;
  uint16_t sp;  // at entry (an interrupt handler's adapter reads the pushed P through it)
  bool leftCaller;  // returned to its caller's caller (PLA, RTS): see mayLeaveCaller
} Regs;

// A ported routine: where the original lives and how to call the C for it.
typedef struct {
  const char* name;
  uint32_t entry;          // CPU address, $80-$FF mirror
  uint32_t end;            // extent [entry, end): instruction starts in it must all be covered
  bool rtl;                // returns with RTL (else RTS)
  bool rti;                // an interrupt handler entered at its vector target: returns with
                           // RTI to the pushed PC (its reads of $4210-$4212, the interrupt
                           // acknowledges, are free)
  uint8_t rtiFrame;        // for a handler's tail: bytes its prologue pushed above the
                           // interrupt frame (D, Y, X, A, DB = 9 for the common NMI prologue)
  uint32_t alsoIn[2][2];   // [start, end) ranges the routine may continue into at its own
                           // stack depth: a tail jump into another (verified) routine
  uint32_t vramReadAt;     // harness-9: the PC of a dead read of VMDATA ($2139/$213A, 16-bit):
                           // only its step of the VRAM address matters, which the PPU's
                           // pointer (the one VRAM word writes are recorded at) takes; at
                           // that PC, for every row, the read is not a hardware access
  uint32_t vramDeadAt[4];  // harness-30: more dead VMDATA read sites, as vramReadAt
  uint32_t vramLiveAt[4];  // harness-30: live VMDATA read sites (a 16-bit LDA $2139): the words
                           // the original reads there are kept, in order, and served to the C
                           // (video_read_vram_word), which must read as many
  uint32_t dmaReadAt[4];   // harness-31: sites of a 16-bit read of a DMA register ($43xx): the
                           // words are kept in order and served to the C (dma_hdma_get_table)
  uint32_t endAt;          // a handler's prologue: the sample ends when it reaches this
                           // instruction (its dispatch, an indirect jump to the target, which
                           // has its own row); D and DB are compared there
  uint32_t endAt2;         // harness-23: a second cut (a loop part whose branch back to the
                           // loop's head and whose way out both reach a wait)
  const char* source;      // src/ file holding the C, hashed into the results
  bool needM16, needX16;   // entry contract: register widths the C assumes (else either)
  // Run the C from `in` (g_ram already holds the entry WRAM); fill the outputs in `out`
  // (start: a copy of `in`).
  void (*call)(const Regs* in, Regs* out);
  bool compareA, compareX, compareY;  // which registers are outputs (flags: by liveness)
  bool lowWramDb;          // absolute ($0000-$1FFF) accesses through DB must hit low WRAM
  bool fixedD;             // the C assumes D = DP ($0100)
  uint8_t db;              // the C assumes this data bank (0 = any)
  // Fuzz: given registers already randomised and D/DB/widths from a real entry, make the
  // inputs valid for this routine (WRAM through `ram`, registers through `r`). NULL = no fuzz.
  void (*fuzz)(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
  // The routine waits for interrupts: the ones that land inside a sample are recorded
  // (their WRAM changes) and replayed when the C calls platform_wait_vblank(), instead
  // of discarding the sample. Their hardware accesses belong to the handler.
  bool waitsForInterrupts;
  // The routine may also end with PLA, RTS: dropping its own return address, it returns
  // from its caller. That return ends the sample too; the adapter tells which one the C
  // took through out->leftCaller, compared with the original's.
  bool mayLeaveCaller;
  // Data inside the extent (jump tables): [start, end) ranges skipped when listing its
  // instruction starts.
  uint32_t holes[12][2];
  // Addresses where the linear decode must switch to 8-bit A (code reached by a branch
  // from an 8-bit section after a REP that the sweep would otherwise carry over).
  uint32_t m8At[2];
  // The most instructions a live sample may run (0: MAX_INSNS), honoured only for a routine
  // that waits for interrupts (a title card waiting a few hundred frames). A budget only:
  // what is compared does not change.
  uint32_t maxInsns;
  // harness-25: the long effect log (MAX_EFFECTS_LONG) for a routine that does not wait but
  // writes more than MAX_EFFECTS hardware effects (a tilemap written word by word).
  bool manyEffects;
  bool emuEntry;           // harness-39: entered in emulation mode (the reset vector's code)
  // The fuzz's stack: the highest byte of the sentinel frame (0: $1F00, the fuzz's usual
  // place). A routine whose callees write the game's words $1EEE-$1EFD needs the game's own
  // stack instead (measured from its live samples' entry SP).
  uint16_t fuzzStack;
  // The routine streams DSP-4 commands src/dsp4.c does not have: its data-port bytes are
  // recorded in order (its tape) instead of being checked as whole commands, and the C's
  // port calls (dsp4_port_write16/read16) must reproduce the writes in place and take the
  // reads exactly, the whole tape. Its fuzz scripts the chip's answers (dsp4_bridge_script).
  bool dsp4Tape;
  // harness-34: how many entry states to keep for replay while no live sample has completed
  // (0: REPLAYS_PER_ROUTINE), one entry in REPLAY_SPREAD. For a routine the interrupts land in
  // nearly every time.
  int replays;
  // [start, end) code ranges of a taped row whose DSP-4 bytes are whole commands of
  // src/dsp4.c (its native ones: a prologue before its stream). They stay off every tape,
  // as do the bytes of any untaped row's extent, and are judged as an untaped row's.
  uint32_t dsp4Native[2][2];
} Routine;

// Hardware effects, in canonical form: what the original did through DMA and what the C
// asked the platform for (tools/bench/platform_bench.c) must be the same list.
enum { EFFECT_VRAM = 1, EFFECT_CGRAM = 2, EFFECT_RAW_DMA = 3, EFFECT_VRAM_FILL = 4, EFFECT_VRAM_WORD = 5, EFFECT_APU = 6,
       EFFECT_CGRAM_WORD = 7, EFFECT_OAM = 8, EFFECT_STREAM = 9,
       EFFECT_STREAM_SOURCE = 10 };  // harness-32: a stream re-pointed in the sample (channel | written bits << 8, mode | target << 8, source)
void difftest_effect(int kind, uint32_t target, uint32_t src, uint32_t size);
void difftest_replay_interrupt(void);

// Inputs: the values the original read from an input register ($2140-$2143, $213F, the
// joypad registers), served
// to the C in order (the first unread value of that register). Returns 0 and fails the
// sample if the original read no more of it.
uint8_t difftest_input(uint16_t reg);
// The C's DSP-4 data-port byte (platform_bench.c): a write is checked against the sample's
// tape in place; a read returns the tape's byte. Outside a taped row any use fails.
uint8_t difftest_dsp4_byte(uint8_t value, bool write);
// harness-30: the next word the original read at its live VMDATA read sites (vramLiveAt)
uint16_t difftest_vram_read(void);
// harness-31: the next word the original read at its DMA register read sites (dmaReadAt)
uint16_t difftest_reg_read(void);
// The bench chip's scripted answers for a taped row's fuzz case (dsp4_bridge.cpp): status
// always ready, data reads from `words` (low byte first), 0 past the end; writes are not
// fed to the chip. Cleared by dsp4_bridge_reset.
void dsp4_bridge_script(const uint16_t* words, int n);
// The n-th value (from 0) the original read from `reg` in the sample being compared, for
// adapters that model registers left by input reads (-1 if there is none).
int difftest_input_peek(uint16_t reg, int n);

// PPU fields (Mode 7 matrix and offsets, BG1/BG2 scroll) set by the C, compared by end state.
enum { PF_M7A, PF_M7B, PF_M7C, PF_M7D, PF_M7X, PF_M7Y, PF_M7H, PF_M7V, PF_BG1H, PF_BG1V, PF_BG2H, PF_BG2V, PF_BG3H, PF_BG3V, PF_BG4H, PF_BG4V, PF_FIXR, PF_FIXG, PF_FIXB, PF_COUNT };
void difftest_ppu_field(int field, uint16_t value);

// Registers whose state is the last byte written (layer enables, windows, colour math,
// interrupt enable and timers): compared by which were written and their last values.
void difftest_reg(uint16_t adr, uint8_t value);  // platform_wait_vblank() on the bench

// DMA channels as slots (tools/bench/platform_bench.c): the C claims the channel registers
// a routine leaves set up (after the channel's last kick: HDMA setups, streams), and kicks
// streams set up before the routine (a kick that uses registers the routine did not write).
void difftest_dma_claim(int slot, int reg, uint8_t value);  // reg 0-4 or 7 of $43x0-$43x7
void difftest_stream_size(int slot, uint32_t size);
void difftest_stream_kick(uint8_t mask);
void difftest_cgram_address(uint8_t index);
void difftest_vram_address(uint16_t word);

void difftest_init(Snes* snes, const uint8_t* rom, bool negative);
// glue-1: use the composed rows' table (before difftest_init).
void difftest_composed(void);
// harness-28: rows (comma-separated names) whose fuzz cases are skipped on this movie.
void difftest_fuzz_skip(const char* names);
// Before difftest_init: test only the rows named (comma-separated), to iterate on a few
// quickly. Live counts then differ (another row no longer owns the interrupts it waits
// through); tools/verify_all.sh always runs every row.
void difftest_only(const char* names);
// Before difftest_init: test only slice k of n (contiguous rows, k from 0), so a movie's
// rows can run as n processes in parallel; concatenated in k order the results keep the
// registry's order (tools/verify_all.sh).
void difftest_shard(int k, int n);
void difftest_on_insn(Cpu* cpu);              // from the CPU trace hook
void difftest_on_access(uint32_t adr, uint8_t val, bool write);  // from the CPU access hook
void difftest_fuzz(int iterations);          // at the end of a run
bool difftest_report(const char* resultsPath);  // prints a summary; false if any routine failed

#endif
