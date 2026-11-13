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
#include "cpu.h"
#include "routines.h"
#include "tg3000.h"

uint8_t g_ram[TG3000_WRAM_SIZE];
const uint8_t* g_rom;

#define MAX_SAMPLES 4000     // compared live samples per routine
#define SAMPLE_STRIDE 3      // take one entry in N
#define MAX_EXTENT 0x400
#define MAX_CONTRACTS 8
#define MAX_INSNS 200000     // a sample that runs longer is abandoned

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
static bool hwAccess;
static uint32_t hwAccessAdr;

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

static void onAccess(uint32_t adr, uint8_t val, bool write) {
  (void)val;
  if(active < 0 || hwAccess) return;
  if(isWram(adr) || (!write && isRom(adr))) return;
  hwAccess = true;
  hwAccessAdr = adr;
}

// Instruction starts of a routine's extent, by linear decode with the contract's widths.
static void findStarts(const Routine* r, Stats* s) {
  uint32_t off = (((r->entry >> 16) & 0x7F) << 15) | (r->entry & 0x7FFF);
  uint32_t n = r->end - r->entry;
  for(uint32_t i = 0; i < n && i < MAX_EXTENT;) {
    s->starts[i] = true;
    i += coverage_insn_length(g_rom[off + i], !r->needM16, !r->needX16);
  }
}

static void noteContract(Stats* s, const Routine* r, const Regs* in) {
  Contract c = {in->d, in->db, in->m8, in->x8, in->e};
  if(in->e || (r->needM16 && in->m8) || (r->needX16 && in->x8)) s->contractBroken = true;
  for(int i = 0; i < s->contractCount; i++) {
    Contract* o = &s->contracts[i];
    if(o->d == c.d && o->db == c.db && o->m8 == c.m8 && o->x8 == c.x8 && o->e == c.e) return;
  }
  if(s->contractCount < MAX_CONTRACTS) s->contracts[s->contractCount++] = c;
}

// Run the C from the recorded entry state and compare with the recorded result.
static bool compareWithC(int idx, const Regs* after, char* why, size_t whyLen) {
  const Routine* r = &table[idx];
  memcpy(g_ram, entryRam, sizeof g_ram);
  Regs out = entryRegs;
  r->call(&entryRegs, &out);
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
  CHECK(a, r->compareA) CHECK(x, r->compareX) CHECK(y, r->compareY) CHECK(c, r->compareC)
  CHECK(n, true) CHECK(z, true) CHECK(v, true) CHECK(d, true) CHECK(db, true)
#undef CHECK
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
  if(hwAccess) {
    snprintf(why, sizeof why, "hardware access at $%06X (not supported by this routine's C)", hwAccessAdr);
    ok = false;
  } else {
    ok = compareWithC(idx, &after, why, sizeof why);
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
  hwAccess = false;
  noteContract(&stats[idx], r, &entryRegs);
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
    } else if(++insnsInSample > MAX_INSNS) {
      stats[active].abandoned++;
      active = -1;
    }
    return;
  }
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
void difftest_fuzz(int iterations) {
  uint32_t seed = 0x13579BDF;
  fuzzing = true;
  snes->nmiEnabled = false;
  snes->hIrqEnabled = snes->vIrqEnabled = false;
  for(int i = 0; i < tableCount; i++) {
    const Routine* r = &table[i];
    Stats* s = &stats[i];
    if(r->fuzzDpLen == 0 || s->contractCount == 0) continue;
    for(int k = 0; k < iterations; k++) {
      Contract c = s->contracts[k % s->contractCount];
      Cpu* cpu = snes->cpu;
      cpu->nmiWanted = cpu->irqWanted = cpu->intWanted = false;
      cpu->i = true;
      cpu->e = false;
      cpu->mf = c.m8;
      cpu->xf = c.x8;
      cpu->dp = c.d;
      cpu->db = c.db;
      seed = seed * 1103515245u + 12345u; cpu->a = seed >> 16;
      seed = seed * 1103515245u + 12345u; cpu->x = c.x8 ? (seed >> 24) : (seed >> 16);
      seed = seed * 1103515245u + 12345u; cpu->y = c.x8 ? (seed >> 24) : (seed >> 16);
      for(int b = 0; b < r->fuzzDpLen; b++) {
        seed = seed * 1103515245u + 12345u;
        snes->ram[(uint16_t)(c.d + r->fuzzDp + b)] = seed >> 24;
      }
      // sentinel return to $8000 of the entry bank (a BRA * trap), stack at $1F00
      cpu->sp = 0x1F00;
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

static uint64_t fileHash(const char* path) {
  FILE* f = fopen(path, "rb");
  if(f == NULL) return 0;
  uint64_t h = 0xcbf29ce484222325ull;
  int ch;
  while((ch = fgetc(f)) != EOF) { h ^= (uint8_t)ch; h *= 0x100000001b3ull; }
  fclose(f);
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
  if(out != NULL) fprintf(out, "# entry name source fnv1a64(source) live fuzz paths status  -- written by the bench (--difftest)\n");
  for(int i = 0; i < tableCount; i++) {
    const Routine* r = &table[i];
    Stats* s = &stats[i];
    int need = 0, seen = 0;
    for(int k = 0; k < MAX_EXTENT; k++) if(s->starts[k]) { need++; if(s->visited[k]) seen++; }
    bool reached = s->entries > 0 || s->fuzzPass + s->fuzzFail > 0;
    bool ok = s->fail == 0 && s->fuzzFail == 0 && (s->pass + s->fuzzPass) > 0 && seen == need && !s->contractBroken;
    const char* status = !reached ? "unreached" : ok ? "verified" : "failed";
    if(reached) allOk &= ok;  // a routine this run never reached is reported, not failed
    printf("difftest %-16s $%06X: %s  live %lu/%lu pass (%lu entries, %lu interrupted, %lu abandoned), fuzz %lu/%lu, paths %d/%d%s\n",
      r->name, r->entry, !reached ? "UNREACHED" : ok ? "PASS" : "FAIL", s->pass, s->samples, s->entries, s->interrupted, s->abandoned,
      s->fuzzPass, s->fuzzPass + s->fuzzFail, seen, need, s->contractBroken ? ", ENTRY CONTRACT BROKEN" : "");
    for(int c = 0; c < s->contractCount; c++) {
      Contract* k = &s->contracts[c];
      printf("    entry state: D=%04X DB=%02X %s %s%s\n", k->d, k->db, k->m8 ? "m8" : "m16", k->x8 ? "x8" : "x16", k->e ? " emu" : "");
    }
    if(s->firstFailure[0]) printf("    first failure: %s\n", s->firstFailure);
    bool keptRow = false;
    for(int k = 0; k < keptCount; k++) {
      unsigned entry;
      if(sscanf(kept[k], "%x", &entry) == 1 && entry == r->entry) {
        if(out != NULL) fputs(kept[k], out);
        keptRow = true;
      }
    }
    if(out != NULL && !keptRow) {
      fprintf(out, "%06X %s %s %016llx %lu %lu %d/%d %s\n", r->entry, r->name, r->source,
        (unsigned long long)fileHash(r->source), s->pass, s->fuzzPass, seen, need, status);
    }
  }
  if(out != NULL) fclose(out);
  return allOk;
}
