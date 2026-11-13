// SPDX-License-Identifier: GPL-3.0-or-later
// TG3000 bench: runs the original ROM headless as the ground truth for the C port.
// It is a measuring instrument (it emulates); nothing here is linked into the game.
//
//   bench ROM [frames] [--movie FILE] [--dump DIR] [--every N] [--dsp4-log FILE] [--hashes FILE]
//
// A "frame" is one full video frame: the bench steps the CPU until LakeSnes's frame
// counter changes (start of the next frame), so frame N of a movie is always video frame N.
// --hashes writes one line per frame: "<frame> <FNV-1a 64 of the 128 KB WRAM>".
// --coverage FILE: trace execution and OR-merge the per-ROM-byte flags into FILE
// (see coverage.h); RAM-executed addresses go to FILE.ram.
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

static void applyMovie(int frame) {
  for(int i = 0; i < movieLen; i++) {
    if(movie[i].frame != frame) continue;
    for(int b = 0; b < 12; b++) snes_setButtonState(snes, 1, b, (movie[i].buttons >> b) & 1);
  }
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
    fprintf(stderr, "usage: %s ROM [frames] [--movie FILE] [--dump DIR] [--every N] [--dsp4-log FILE] [--hashes FILE] [--coverage FILE] [--state-test N]\n", argv[0]);
    return 2;
  }
  const char* romPath = argv[1];
  const char* dumpDir = NULL;
  FILE* hashes = NULL;
  int stateTest = 0;
  const char* coveragePath = NULL;
  int frames = 600, every = 60;
  for(int i = 2; i < argc; i++) {
    if(strcmp(argv[i], "--dsp4-log") == 0 && i + 1 < argc) {
      dsp4Log = fopen(argv[++i], "w");
      if(dsp4Log == NULL) { perror(argv[i]); return 1; }
    } else if(strcmp(argv[i], "--hashes") == 0 && i + 1 < argc) {
      hashes = fopen(argv[++i], "w");
      if(hashes == NULL) { perror(argv[i]); return 1; }
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
  if(coveragePath != NULL) coverage_start(snes->cpu, snes->cart->rom);

  for(int i = 0; i < frames; i++) {
    applyMovie(i);
    benchRunFrame();
    if(hashes != NULL) fprintf(hashes, "%d %016llx\n", i + 1, (unsigned long long)wramHash());
    if(dumpDir != NULL && (i + 1) % every == 0) dumpFrame(dumpDir);
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

  if(coveragePath != NULL) {
    coverage_report();
    if(!coverage_merge_save(coveragePath)) status = 1;
  }

  int firstWrite = -1;
  for(int i = 0; i < (int)snes->frames && i < (1 << 16); i++) if(dsp4WritesByFrame[i]) { firstWrite = i; break; }
  printf("frames=%u dsp4 data: %lu reads %lu writes; status: %lu reads %lu writes; first write frame %d\n",
    snes->frames, dsp4Counts[0][1], dsp4Counts[1][1], dsp4Counts[0][0], dsp4Counts[1][0], firstWrite);
  if(dsp4Log != NULL) fclose(dsp4Log);
  if(hashes != NULL) fclose(hashes);
  snes_free(snes);
  free(rom);
  free(movie);
  return status;
}
