// SPDX-License-Identifier: GPL-3.0-or-later
// TG3000 bench: runs the original ROM headless as the ground truth for the C port.
// It is a measuring instrument (it emulates); nothing here is linked into the game.
//
//   bench ROM [frames] [--dsp4-log FILE]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snes.h"
#include "bench_log.h"

static Snes* snes;
static FILE* dsp4Log;
static unsigned long dsp4Counts[2][2];  // [write][data]

void bench_log_dsp4(bool write, bool data, uint8_t bank, uint16_t adr, uint8_t val) {
  dsp4Counts[write][data]++;
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

int main(int argc, char** argv) {
  if(argc < 2) {
    fprintf(stderr, "usage: %s ROM [frames] [--dsp4-log FILE]\n", argv[0]);
    return 2;
  }
  const char* romPath = argv[1];
  int frames = 600;
  for(int i = 2; i < argc; i++) {
    if(strcmp(argv[i], "--dsp4-log") == 0 && i + 1 < argc) {
      dsp4Log = fopen(argv[++i], "w");
      if(dsp4Log == NULL) { perror(argv[i]); return 1; }
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

  for(int i = 0; i < frames; i++) snes_runFrame(snes);

  printf("frames=%u dsp4 data: %lu reads %lu writes; status: %lu reads %lu writes\n", snes->frames,
    dsp4Counts[0][1], dsp4Counts[1][1], dsp4Counts[0][0], dsp4Counts[1][0]);
  if(dsp4Log != NULL) fclose(dsp4Log);
  snes_free(snes);
  free(rom);
  return 0;
}
