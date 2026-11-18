// SPDX-License-Identifier: GPL-3.0-or-later
// TG3000 bench: runs the original ROM headless as the ground truth for the C port.
// It is a measuring instrument (it emulates); nothing here is linked into the game.
//
//   bench ROM [frames] [--movie FILE] [--dump DIR] [--every N] [--dsp4-log FILE] [--hashes FILE]
//
// A "frame" is one full video frame: the bench steps the CPU until LakeSnes's frame
// counter changes (start of the next frame), so frame N of a movie is always video frame N.
// --hashes writes one line per step: "<step> <video frame counter> <FNV-1a 64 of the 128 KB WRAM>".
// --coverage FILE: trace execution and OR-merge the per-ROM-byte flags into FILE
// (see coverage.h); RAM-executed addresses go to FILE.ram.
// --autopilot FROM: from step FROM on, drive the race: hold X (accelerate) and steer with
// L/R to keep the car's lateral position ($7E:0202, signed) near the road centre.
// Replaces the movie's buttons from that step. --record FILE writes every step's buttons
// as a movie, so an autopilot run can be replayed without the autopilot.
// --wram-dump FILE: append the 128 KB WRAM after every step (for finding variables).
// --difftest RESULTS: run the per-routine differential test for every routine in
// routines.c during the run, then --fuzz N synthetic samples each; write RESULTS
// (map/verified.txt). Exit status 4 if any routine fails. --difftest-negative swaps in
// deliberately wrong C, which must fail.
// --census FILE: for every routine entered with JSR/JSL (and every interrupt handler):
// entries, interrupted entries, hardware classes it touches itself, D/DB/widths, callees and
// indirect-jump targets (map/census.txt).
// --load-state FILE: start from a save state (frame numbers in the movie stay relative to
// the bench's own loop, starting at 0). --save-state FILE: write one at the end.
// --state-test N: after the run, save a state, run N more frames, reload the state, run the
// same N frames again; the WRAM hashes and the final full save states must be identical.
//
// Movie file: one "<frame> <buttons>" per line, the buttons held from that frame on
// (player 1). Buttons: B Y s(elect) S(tart) U D L R A X l r, or "." for none.
// Lines starting with '#' are comments.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snes.h"
#include "bench_log.h"
#include "png.h"
#include "coverage.h"
#include "difftest.h"
#include "census.h"
#include "hdma.h"
#include "apu.h"

static Snes* snes;
static FILE* dsp4Log;
static unsigned long dsp4Counts[2][2];  // [write][data]
static unsigned long dsp4WritesByFrame[1 << 16];

void bench_log_dsp4(bool write, bool data, uint8_t bank, uint16_t adr, uint8_t val) {
  dsp4Counts[write][data]++;
  if(write) dsp4WritesByFrame[snes->frames & 0xFFFF]++;
  if(dsp4Log != NULL) {
    // frame, cpu PB:PC (points into the accessing instruction), access, address, value
    fprintf(dsp4Log, "%u %02X:%04X %c%c %02X:%04X %02X\n", snes->frames, snes->cpu->k, snes->cpu->pc,
      write ? 'W' : 'R', data ? 'D' : 'S', bank, adr, val);
  }
}

static uint8_t* readFile(const char* path, int* size) {
  FILE* f = fopen(path, "rb");
  if(f == NULL) return NULL;
  fseek(f, 0, SEEK_END);
  *size = (int)ftell(f);
  rewind(f);
  uint8_t* buf = malloc(*size);
  if(fread(buf, 1, *size, f) != (size_t)*size) { free(buf); buf = NULL; }
  fclose(f);
  return buf;
}

// One video frame: run until the frame counter advances. (snes_runFrame stops at the start
// of vblank and, in a run, once covered two frames in one call; this cannot.)
static void benchRunFrame(void) {
  const uint32_t frame = snes->frames;
  while(snes->frames == frame) cpu_runOpcode(snes->cpu);
}

static uint64_t wramHash(void) {
  uint64_t h = 0xcbf29ce484222325ull;
  for(size_t i = 0; i < sizeof snes->ram; i++) { h ^= snes->ram[i]; h *= 0x100000001b3ull; }
  return h;
}

// ---- input movie ----

typedef struct { int frame; uint16_t buttons; } MovieEvent;
static MovieEvent* movie;
static int movieLen;

static const char buttonChars[] = "BYsSUDLRAXlr";  // LakeSnes button indices 0..11

static bool loadMovie(const char* path) {
  FILE* f = fopen(path, "r");
  if(f == NULL) { perror(path); return false; }
  char line[256];
  int cap = 0;
  while(fgets(line, sizeof line, f)) {
    if(line[0] == '#' || line[0] == '\n') continue;
    int frame;
    char buttons[64] = ".";
    if(sscanf(line, "%d %63s", &frame, buttons) < 1) continue;
    uint16_t mask = 0;
    for(char* c = buttons; *c; c++) {
      const char* p = strchr(buttonChars, *c);
      if(p != NULL && *c != '\0') mask |= 1 << (p - buttonChars);
    }
    if(movieLen == cap) { cap = cap ? cap * 2 : 64; movie = realloc(movie, cap * sizeof *movie); }
    movie[movieLen++] = (MovieEvent){frame, mask};
  }
  fclose(f);
  return true;
}

static uint16_t heldButtons;

static void setButtons(uint16_t mask) {
  heldButtons = mask;
  for(int b = 0; b < 12; b++) snes_setButtonState(snes, 1, b, (mask >> b) & 1);
}

static void applyMovie(int frame) {
  for(int i = 0; i < movieLen; i++) {
    if(movie[i].frame == frame) setButtons(movie[i].buttons);
  }
}

// Race autopilot (see --autopilot). Button bits follow buttonChars.
enum { BTN_L_DPAD = 1 << 6, BTN_R_DPAD = 1 << 7, BTN_A = 1 << 8, BTN_X = 1 << 9 };

// In a race the DSP-4 gets commands every frame; elsewhere (league table, track
// description, shop) the autopilot loops "Down x6, A", which leaves the shop through EXIT
// and confirms the other screens.
static unsigned long dsp4WritesAtLastStep;
static int menuClock;

static void autopilot(void) {
  unsigned long writes = dsp4Counts[1][1];
  bool racing = writes != dsp4WritesAtLastStep;
  dsp4WritesAtLastStep = writes;
  if(racing) {
    menuClock = 0;
    int16_t lateral = (int16_t)(snes->ram[0x202] | (snes->ram[0x203] << 8));
    uint16_t mask = BTN_X;
    if(lateral > 12) mask |= BTN_L_DPAD;
    else if(lateral < -12) mask |= BTN_R_DPAD;
    setButtons(mask);
    return;
  }
  int t = menuClock++ % 240;
  bool down = t < 120 && t % 20 < 6;
  bool a = t >= 150 && t < 156;
  setButtons(down ? (1 << 5) : a ? BTN_A : 0);
}

static bool coverageOn, difftestOn, censusOn, hdmaOn, apuOn;

static void benchTrace(Cpu* cpu) {
  if(coverageOn) coverage_on_insn(cpu);
  if(censusOn) census_on_insn(cpu);
  if(difftestOn) difftest_on_insn(cpu);
  if(hdmaOn) hdma_on_insn(cpu);
  if(apuOn) apu_on_insn(cpu);
}

static void benchAccess(uint32_t adr, uint8_t val, bool write) {
  if(censusOn) census_on_access(adr, write);
  if(difftestOn) difftest_on_access(adr, val, write);
  if(hdmaOn) hdma_on_access(adr, val, write);
  if(apuOn) apu_on_access(adr, val, write);
}

static void recordStep(FILE* f, int step, uint16_t* last) {
  if(f == NULL || (step > 0 && heldButtons == *last)) return;
  char buf[16] = ".";
  int n = 0;
  for(int b = 0; b < 12; b++) if(heldButtons & (1 << b)) buf[n++] = buttonChars[b];
  buf[n ? n : 1] = '\0';
  fprintf(f, "%d %s\n", step, buf);
  *last = heldButtons;
}

// ---- coverage explorer (--explore): from the end of the run, segment after segment, try
// several random button sequences from the same saved state and keep the one that starts
// the most instructions not seen yet (nor in --explore-known, a coverage file of what other
// movies already reach); its buttons go to --record, so the result replays as a movie ----

typedef struct { int offset; uint16_t buttons; } ExploreEvent;
static uint32_t exploreSeed = 0x2545F491u;
static uint32_t exploreRandom(void) {
  exploreSeed ^= exploreSeed << 13;
  exploreSeed ^= exploreSeed >> 17;
  exploreSeed ^= exploreSeed << 5;
  return exploreSeed;
}

// Taps of one button (held 3 frames), with gaps: menus take a press per few frames.
static int exploreCandidate(ExploreEvent* ev, int cap, int len) {
  static const uint16_t pick[] = {1 << 4, 1 << 5, 1 << 6, 1 << 7, 1 << 8, 1 << 8, 1 << 0, 1 << 3, 1 << 9, 1 << 1};
  int n = 0, t = 0;
  while(n + 2 < cap) {
    t += 6 + (int)(exploreRandom() % 24);
    if(t + 3 >= len) break;
    ev[n++] = (ExploreEvent){t, pick[exploreRandom() % (sizeof pick / sizeof pick[0])]};
    ev[n++] = (ExploreEvent){t + 3, 0};
    t += 3;
  }
  return n;
}

// Runs a candidate; returns false when the game stopped (WRAM unchanged over its last
// 120 frames: a hang, such as a mode that waits for a multitap the bench does not have).
static bool exploreRun(const ExploreEvent* ev, int n, int len) {
  int k = 0;
  setButtons(0);
  uint64_t last = 0;
  int still = 0;
  for(int f = 0; f < len; f++) {
    while(k < n && ev[k].offset == f) setButtons(ev[k++].buttons);
    benchRunFrame();
    uint64_t h = wramHash();
    still = h == last ? still + 1 : 0;
    last = h;
  }
  return still < 120;
}

// --explore-want: CPU addresses whose first execution is worth `EXPLORE_WANT_BONUS` starts
// each (steers the explorer to code a movie still misses, such as one shop item's view)
#define EXPLORE_WANT_BONUS 100000
static uint32_t exploreWant[16];
static int exploreWantCount;
static int exploreWantScore(const uint8_t* base) {
  int bonus = 0;
  for(int i = 0; i < exploreWantCount; i++) {
    uint32_t a = exploreWant[i];
    uint32_t off = ((a >> 16) & 0x7F) << 15 | (a & 0x7FFF);
    if(!(base[off] & COV_START) && coverage_flag(off) & COV_START) bonus += EXPLORE_WANT_BONUS;
  }
  return bonus;
}

// --explore-goal ADDR=VAL: a WRAM word that should hold VAL; each one holding it at the end
// of a candidate is worth `EXPLORE_GOAL_BONUS` starts (partial progress towards a screen)
#define EXPLORE_GOAL_BONUS 1000
static uint32_t exploreGoalAdr[8];
static uint16_t exploreGoalVal[8];
static int exploreGoalCount;
static int exploreGoalScore(void) {
  int bonus = 0;
  for(int i = 0; i < exploreGoalCount; i++) {
    uint32_t a = exploreGoalAdr[i] & 0x1FFFF;
    if((uint16_t)(snes->ram[a] | snes->ram[(a + 1) & 0x1FFFF] << 8) == exploreGoalVal[i]) bonus += EXPLORE_GOAL_BONUS;
  }
  return bonus;
}

static void explore(int segments, int cands, int len, int start, FILE* record, const uint8_t* known) {
  int stSize = snes_saveState(snes, NULL);
  uint8_t* state = malloc(stSize);
  uint8_t* bestState = malloc(stSize);
  static uint8_t base[1 << 20], bestCov[1 << 20];
  enum { CAP = 512 };
  static ExploreEvent ev[CAP], bestEv[CAP];
  int total = 0;
  for(int s = 0; s < segments; s++) {
    snes_saveState(snes, state);
    coverage_snapshot(base);
    int best = -1, bestN = 0;
    for(int c = 0; c < cands; c++) {
      if(c > 0) {
        snes_loadState(snes, state, stSize);
        coverage_restore(base);
      }
      int n = exploreCandidate(ev, CAP, len);
      bool alive = exploreRun(ev, n, len);
      int score = alive ? coverage_new_starts(base, known) + exploreWantScore(base) + exploreGoalScore() : -1;
      if(score > best) {
        best = score;
        bestN = n;
        memcpy(bestEv, ev, n * sizeof *ev);
        snes_saveState(snes, bestState);
        coverage_snapshot(bestCov);
      }
    }
    if(best < 0) {  // every candidate froze the game: stop here
      snes_loadState(snes, state, stSize);
      coverage_restore(base);
      fprintf(stderr, "explore: segment %d: every candidate froze the game; stopping\n", s);
      break;
    }
    snes_loadState(snes, bestState, stSize);
    coverage_restore(bestCov);
    total += best;
    int at = start + s * len;
    if(record != NULL) {
      if(s == 0) fprintf(record, "%d .\n", at);
      for(int k = 0; k < bestN; k++) {
        char buf[16] = ".";
        int m = 0;
        for(int b = 0; b < 12; b++) if(bestEv[k].buttons & (1 << b)) buf[m++] = buttonChars[b];
        buf[m ? m : 1] = '\0';
        fprintf(record, "%d %s\n", at + bestEv[k].offset, buf);
      }
      fflush(record);
    }
    fprintf(stderr, "explore: segment %d (frames %d-%d): %d new instruction starts (total %d)\n", s, at, at + len - 1,
      best, total);
  }
  free(state);
  free(bestState);
}

// ---- frame dumps ----

static uint8_t argb[512 * 480 * 4];
static uint8_t rgb[256 * 224 * 3];

static void dumpFrame(const char* dir) {
  snes_setPixels(snes, argb);
  // 512x480 ARGB (B,G,R,A in memory) -> 256x224 RGB: every other pixel of every other line,
  // skipping the 16 blank lines on top (non-overscan picture).
  for(int y = 0; y < 224; y++) {
    for(int x = 0; x < 256; x++) {
      const uint8_t* s = &argb[((16 + y * 2) * 512 + x * 2) * 4];
      uint8_t* d = &rgb[(y * 256 + x) * 3];
      d[0] = s[2]; d[1] = s[1]; d[2] = s[0];
    }
  }
  char path[512];
  snprintf(path, sizeof path, "%s/frame_%05u.png", dir, snes->frames);
  if(!png_write_rgb(path, rgb, 256, 224)) perror(path);
}

int main(int argc, char** argv) {
  if(argc < 2) {
    fprintf(stderr, "usage: %s ROM [frames] [--movie FILE] [--dump DIR] [--every N] [--dsp4-log FILE] [--hashes FILE] [--coverage FILE] [--load-state FILE] [--save-state FILE] [--wram-dump FILE] [--autopilot STEP] [--record FILE] [--difftest RESULTS] [--difftest-negative] [--fuzz N] [--census FILE] [--hdma-report FILE] [--apu-report FILE] [--state-test N]\n", argv[0]);
    return 2;
  }
  const char* romPath = argv[1];
  const char* dumpDir = NULL;
  FILE* hashes = NULL;
  int stateTest = 0;
  const char* coveragePath = NULL;
  const char* loadStatePath = NULL;
  FILE* wramDump = NULL;
  FILE* record = NULL;
  int autopilotFrom = -1;
  int exploreSegments = 0, exploreCands = 8, exploreLen = 600;
  const char* exploreKnownPath = NULL;
  const char* difftestResults = NULL;
  const char* censusPath = NULL;
  const char* hdmaPath = NULL;
  const char* apuPath = NULL;
  bool difftestNegative = false;
  int fuzzIterations = 0;
  const char* saveStatePath = NULL;
  int frames = 600, every = 60;
  for(int i = 2; i < argc; i++) {
    if(strcmp(argv[i], "--dsp4-log") == 0 && i + 1 < argc) {
      dsp4Log = fopen(argv[++i], "w");
      if(dsp4Log == NULL) { perror(argv[i]); return 1; }
    } else if(strcmp(argv[i], "--hashes") == 0 && i + 1 < argc) {
      hashes = fopen(argv[++i], "w");
      if(hashes == NULL) { perror(argv[i]); return 1; }
    } else if(strcmp(argv[i], "--difftest") == 0 && i + 1 < argc) {
      difftestResults = argv[++i];
    } else if(strcmp(argv[i], "--census") == 0 && i + 1 < argc) {
      censusPath = argv[++i];
    } else if(strcmp(argv[i], "--hdma-report") == 0 && i + 1 < argc) {
      hdmaPath = argv[++i];
    } else if(strcmp(argv[i], "--apu-report") == 0 && i + 1 < argc) {
      apuPath = argv[++i];
    } else if(strcmp(argv[i], "--difftest-shard") == 0 && i + 1 < argc) {
      int k = 0, n = 0;
      if(sscanf(argv[++i], "%d/%d", &k, &n) != 2 || n < 1 || k < 0 || k >= n) {
        fprintf(stderr, "--difftest-shard K/N with 0 <= K < N\n");
        return 1;
      }
      difftest_shard(k, n);
    } else if(strcmp(argv[i], "--difftest-only") == 0 && i + 1 < argc) {
      difftest_only(argv[++i]);
    } else if(strcmp(argv[i], "--difftest-negative") == 0) {
      difftestNegative = true;
    } else if(strcmp(argv[i], "--fuzz") == 0 && i + 1 < argc) {
      fuzzIterations = atoi(argv[++i]);
    } else if(strcmp(argv[i], "--explore") == 0 && i + 1 < argc) {
      exploreSegments = atoi(argv[++i]);
    } else if(strcmp(argv[i], "--explore-cands") == 0 && i + 1 < argc) {
      exploreCands = atoi(argv[++i]);
    } else if(strcmp(argv[i], "--explore-len") == 0 && i + 1 < argc) {
      exploreLen = atoi(argv[++i]);
    } else if(strcmp(argv[i], "--explore-known") == 0 && i + 1 < argc) {
      exploreKnownPath = argv[++i];
    } else if(strcmp(argv[i], "--explore-want") == 0 && i + 1 < argc) {
      if(exploreWantCount < 16) exploreWant[exploreWantCount++] = (uint32_t)strtoul(argv[++i], NULL, 16);
      else ++i;
    } else if(strcmp(argv[i], "--explore-goal") == 0 && i + 1 < argc) {
      unsigned adr = 0, val = 0;
      if(sscanf(argv[++i], "%x=%x", &adr, &val) == 2 && exploreGoalCount < 8) {
        exploreGoalAdr[exploreGoalCount] = adr;
        exploreGoalVal[exploreGoalCount++] = (uint16_t)val;
      }
    } else if(strcmp(argv[i], "--explore-seed") == 0 && i + 1 < argc) {
      exploreSeed = (uint32_t)strtoul(argv[++i], NULL, 0);
    } else if(strcmp(argv[i], "--autopilot") == 0 && i + 1 < argc) {
      autopilotFrom = atoi(argv[++i]);
    } else if(strcmp(argv[i], "--record") == 0 && i + 1 < argc) {
      record = fopen(argv[++i], "w");
      if(record == NULL) { perror(argv[i]); return 1; }
    } else if(strcmp(argv[i], "--wram-dump") == 0 && i + 1 < argc) {
      wramDump = fopen(argv[++i], "wb");
      if(wramDump == NULL) { perror(argv[i]); return 1; }
    } else if(strcmp(argv[i], "--load-state") == 0 && i + 1 < argc) {
      loadStatePath = argv[++i];
    } else if(strcmp(argv[i], "--save-state") == 0 && i + 1 < argc) {
      saveStatePath = argv[++i];
    } else if(strcmp(argv[i], "--coverage") == 0 && i + 1 < argc) {
      coveragePath = argv[++i];
    } else if(strcmp(argv[i], "--state-test") == 0 && i + 1 < argc) {
      stateTest = atoi(argv[++i]);
    } else if(strcmp(argv[i], "--movie") == 0 && i + 1 < argc) {
      if(!loadMovie(argv[++i])) return 1;
    } else if(strcmp(argv[i], "--dump") == 0 && i + 1 < argc) {
      dumpDir = argv[++i];
    } else if(strcmp(argv[i], "--every") == 0 && i + 1 < argc) {
      every = atoi(argv[++i]);
      if(every < 1) every = 1;
    } else {
      frames = atoi(argv[i]);
    }
  }
  int size = 0;
  uint8_t* rom = readFile(romPath, &size);
  if(rom == NULL) { perror(romPath); return 1; }

  snes = snes_init();
  if(!snes_loadRom(snes, rom, size)) { fprintf(stderr, "ROM did not load\n"); return 1; }
  if(snes->cart->type != 5) { fprintf(stderr, "ROM was not detected as DSP-4 (type %d)\n", snes->cart->type); return 1; }
  if(snes->cart->romSize != 0x100000) { fprintf(stderr, "expected a 1 MB ROM\n"); return 1; }
  if(coveragePath != NULL) { coverage_start(snes->cpu, snes->cart->rom); coverageOn = true; }
  if(difftestResults != NULL || difftestNegative) { difftest_init(snes, snes->cart->rom, difftestNegative); difftestOn = true; }
  if(censusPath != NULL) { census_start(snes, snes->cart->rom); censusOn = true; }
  if(hdmaPath != NULL) { hdma_start(snes); hdmaOn = true; }
  if(apuPath != NULL) { apu_start(snes); apuOn = true; }
  if(coverageOn || difftestOn || censusOn || hdmaOn || apuOn) cpu_traceHook = benchTrace;
  if(difftestOn || censusOn || hdmaOn || apuOn) snes_cpuAccessHook = benchAccess;
  if(loadStatePath != NULL) {
    int stSize = 0;
    uint8_t* st = readFile(loadStatePath, &stSize);
    if(st == NULL || !snes_loadState(snes, st, stSize)) { fprintf(stderr, "could not load state %s\n", loadStatePath); return 1; }
    free(st);
  }

  uint16_t lastRecorded = 0;
  for(int i = 0; i < frames; i++) {
    applyMovie(i);
    if(autopilotFrom >= 0 && i >= autopilotFrom) autopilot();
    recordStep(record, i, &lastRecorded);
    benchRunFrame();
    if(hdmaOn) hdma_on_frame(i);
    if(apuOn) apu_on_frame(i);
    if(wramDump != NULL) fwrite(snes->ram, 1, sizeof snes->ram, wramDump);
    if(hashes != NULL) fprintf(hashes, "%d %u %016llx\n", i + 1, snes->frames, (unsigned long long)wramHash());
    if(dumpDir != NULL && (i + 1) % every == 0) dumpFrame(dumpDir);
  }

  if(exploreSegments > 0) {
    if(!coverageOn) { fprintf(stderr, "--explore needs --coverage FILE\n"); return 2; }
    uint8_t* known = NULL;
    if(exploreKnownPath != NULL) {
      int ksize = 0;
      known = readFile(exploreKnownPath, &ksize);
      if(known == NULL || ksize != (1 << 20)) { fprintf(stderr, "bad --explore-known %s\n", exploreKnownPath); return 2; }
    }
    explore(exploreSegments, exploreCands, exploreLen, frames, record, known);
    free(known);
  }

  int status = 0;
  if(stateTest > 0) {
    int size = snes_saveState(snes, NULL);
    uint8_t* saved = malloc(size);
    uint8_t* endA = malloc(size);
    uint8_t* endB = malloc(size);
    uint64_t* hashA = malloc(stateTest * sizeof *hashA);
    snes_saveState(snes, saved);
    for(int k = 0; k < stateTest; k++) { applyMovie(frames + k); benchRunFrame(); hashA[k] = wramHash(); }
    int sizeA = snes_saveState(snes, endA);
    if(!snes_loadState(snes, saved, size)) { fprintf(stderr, "state-test: load rejected\n"); return 1; }
    int firstDiff = -1;
    for(int k = 0; k < stateTest; k++) {
      applyMovie(frames + k);
      benchRunFrame();
      if(firstDiff < 0 && wramHash() != hashA[k]) firstDiff = k;
    }
    int sizeB = snes_saveState(snes, endB);
    bool same = firstDiff < 0 && sizeA == sizeB && memcmp(endA, endB, sizeA) == 0;
    printf("state-test: %d frames after reload: %s (state %d bytes%s)\n", stateTest,
      same ? "IDENTICAL" : "DIFFERENT", size, firstDiff >= 0 ? ", WRAM diverged" : "");
    if(!same) status = 3;
    free(saved); free(endA); free(endB); free(hashA);
  }

  if(saveStatePath != NULL) {
    int stSize = snes_saveState(snes, NULL);
    uint8_t* st = malloc(stSize);
    snes_saveState(snes, st);
    FILE* f = fopen(saveStatePath, "wb");
    if(f == NULL || fwrite(st, 1, stSize, f) != (size_t)stSize) { perror(saveStatePath); status = 1; }
    if(f != NULL) fclose(f);
    free(st);
  }

  if(hdmaPath != NULL) {
    hdmaOn = false;
    if(!hdma_save(hdmaPath)) status = 1;
  }
  if(apuPath != NULL) {
    apuOn = false;
    if(!apu_save(apuPath)) status = 1;
  }

  if(censusPath != NULL) {
    censusOn = false;
    if(!census_save(censusPath)) status = 1;
  }

  if(coveragePath != NULL) {
    coverage_report();
    if(!coverage_merge_save(coveragePath)) status = 1;
  }

  // last: fuzzing runs the interpreter on synthetic state, so it must come after the save
  // state and the coverage file have captured the real run
  if(difftestOn) {
    coverageOn = false;
    if(fuzzIterations > 0) difftest_fuzz(fuzzIterations);
    if(!difftest_report(difftestNegative ? NULL : difftestResults)) status = 4;
  }

  int firstWrite = -1;
  for(int i = 0; i < (int)snes->frames && i < (1 << 16); i++) if(dsp4WritesByFrame[i]) { firstWrite = i; break; }
  printf("frames=%u dsp4 data: %lu reads %lu writes; status: %lu reads %lu writes; first write frame %d\n",
    snes->frames, dsp4Counts[0][1], dsp4Counts[1][1], dsp4Counts[0][0], dsp4Counts[1][0], firstWrite);
  if(dsp4Log != NULL) fclose(dsp4Log);
  if(hashes != NULL) fclose(hashes);
  if(wramDump != NULL) fclose(wramDump);
  if(record != NULL) fclose(record);
  snes_free(snes);
  free(rom);
  free(movie);
  return status;
}
