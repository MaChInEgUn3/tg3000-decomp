// SPDX-License-Identifier: GPL-3.0-or-later
// Execution coverage for the code/data map. One flag byte per ROM byte, OR-merged into a
// file across runs, plus the set of PB:PC addresses executed outside ROM (code in RAM).
#include "coverage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "cart.h"

#define ROM_SIZE 0x100000

static uint8_t flags[ROM_SIZE];
static uint8_t ramExec[1 << 21];  // bit per 24-bit address would be 2 MB; byte map of bank:pc>>3
static uint64_t insnCount, ramInsnCount;

// 65816 instruction length for an opcode; -1 = accumulator-sized immediate, -2 = index-sized.
static const int8_t opLength[256] = {
  // 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
     2, 2, 2, 2, 2, 2, 2, 2, 1,-1, 1, 1, 3, 3, 3, 4,  // 0
     2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,  // 1
     3, 2, 4, 2, 2, 2, 2, 2, 1,-1, 1, 1, 3, 3, 3, 4,  // 2
     2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,  // 3
     1, 2, 2, 2, 3, 2, 2, 2, 1,-1, 1, 1, 3, 3, 3, 4,  // 4
     2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4,  // 5
     1, 2, 3, 2, 2, 2, 2, 2, 1,-1, 1, 1, 3, 3, 3, 4,  // 6
     2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,  // 7
     2, 2, 3, 2, 2, 2, 2, 2, 1,-1, 1, 1, 3, 3, 3, 4,  // 8
     2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,  // 9
    -2, 2,-2, 2, 2, 2, 2, 2, 1,-1, 1, 1, 3, 3, 3, 4,  // A
     2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,  // B
    -2, 2, 2, 2, 2, 2, 2, 2, 1,-1, 1, 1, 3, 3, 3, 4,  // C
     2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,  // D
    -2, 2, 2, 2, 2, 2, 2, 2, 1,-1, 1, 1, 3, 3, 3, 4,  // E
     2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,  // F
};

int coverage_insn_length(uint8_t opcode, bool m8, bool x8) {
  int n = opLength[opcode];
  if(n == -1) return m8 ? 2 : 3;
  if(n == -2) return x8 ? 2 : 3;
  return n;
}

// LoROM: ROM is $8000-$FFFF in banks $00-$7D/$80-$FF, and all of $40-$7D/$C0-$FF.
static int32_t romOffset(uint8_t bank, uint16_t adr) {
  uint8_t b = bank & 0x7F;
  if(b >= 0x7E) return -1;  // WRAM banks 7E/7F
  if(adr < 0x8000 && b < 0x40) return -1;
  return (int32_t)(((uint32_t)b << 15) | (adr & 0x7FFF)) & (ROM_SIZE - 1);
}

static Cpu* traced;
static uint32_t prevPc, firstRamEntryFrom = 0xFFFFFFFF, firstRamAt;

static void onInsn(Cpu* cpu) {
  insnCount++;
  int32_t off = romOffset(cpu->k, cpu->pc);
  if(off < 0) {
    ramInsnCount++;
    uint32_t a = ((uint32_t)cpu->k << 16) | cpu->pc;
    ramExec[a >> 3] |= 1 << (a & 7);
    if(firstRamEntryFrom == 0xFFFFFFFF) { firstRamEntryFrom = prevPc; firstRamAt = a; }
    prevPc = a;
    return;
  }
  prevPc = ((uint32_t)cpu->k << 16) | cpu->pc;
  bool e = cpu->e, m8 = e || cpu->mf, x8 = e || cpu->xf;
  uint8_t opcode = cart_peekRom(off);
  int len = coverage_insn_length(opcode, m8, x8);
  uint8_t state = m8 ? (x8 ? COV_M8X8 : COV_M8X16) : (x8 ? COV_M16X8 : COV_M16X16);
  flags[off] |= COV_START | COV_CODE | state | (e ? COV_EMU : 0);
  for(int i = 1; i < len; i++) {
    // operand bytes stay in the same bank (PC wraps within the bank)
    int32_t o = romOffset(cpu->k, (uint16_t)(cpu->pc + i));
    if(o >= 0) flags[o] |= COV_CODE;
  }
}

static void onRomRead(uint32_t offset) {
  flags[offset & (ROM_SIZE - 1)] |= COV_READ;
}

static const uint8_t* romData;

uint8_t cart_peekRom(uint32_t offset) { return romData[offset & (ROM_SIZE - 1)]; }

void coverage_start(Cpu* cpu, const uint8_t* rom) {
  traced = cpu;
  romData = rom;
  cpu_traceHook = onInsn;
  cart_romReadHook = onRomRead;
}

bool coverage_merge_save(const char* path) {
  // OR with what earlier runs found, then write back
  FILE* f = fopen(path, "rb");
  if(f != NULL) {
    static uint8_t old[ROM_SIZE];
    if(fread(old, 1, ROM_SIZE, f) == ROM_SIZE) {
      for(int i = 0; i < ROM_SIZE; i++) flags[i] |= old[i];
    }
    fclose(f);
  }
  f = fopen(path, "wb");
  if(f == NULL) { perror(path); return false; }
  bool ok = fwrite(flags, 1, ROM_SIZE, f) == ROM_SIZE;
  ok = (fclose(f) == 0) && ok;

  // RAM-executed addresses, merged as text: one "BB:AAAA" per line
  char ramPath[1024];
  snprintf(ramPath, sizeof ramPath, "%s.ram", path);
  f = fopen(ramPath, "r");
  if(f != NULL) {
    unsigned b, a;
    while(fscanf(f, "%x:%x", &b, &a) == 2) {
      uint32_t x = (b << 16) | a;
      ramExec[x >> 3] |= 1 << (x & 7);
    }
    fclose(f);
  }
  f = fopen(ramPath, "w");
  if(f == NULL) { perror(ramPath); return false; }
  for(uint32_t x = 0; x < (1u << 24); x++) {
    if(ramExec[x >> 3] & (1 << (x & 7))) fprintf(f, "%02X:%04X\n", x >> 16, x & 0xFFFF);
  }
  fclose(f);
  return ok;
}

void coverage_report(void) {
  int code = 0, starts = 0, read = 0;
  for(int i = 0; i < ROM_SIZE; i++) {
    if(flags[i] & COV_CODE) code++;
    if(flags[i] & COV_START) starts++;
    if((flags[i] & COV_READ) && !(flags[i] & COV_CODE)) read++;
  }
  printf("coverage (this run): %llu instructions (%llu outside ROM); %d code bytes, %d instruction starts, %d data bytes read\n",
    (unsigned long long)insnCount, (unsigned long long)ramInsnCount, code, starts, read);
  if(firstRamEntryFrom != 0xFFFFFFFF)
    printf("first execution outside ROM: %02X:%04X, reached from %02X:%04X\n", firstRamAt >> 16, firstRamAt & 0xFFFF,
      firstRamEntryFrom >> 16, firstRamEntryFrom & 0xFFFF);
}
