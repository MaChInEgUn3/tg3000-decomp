// SPDX-License-Identifier: GPL-3.0-or-later
// Immediate uploads to video memory (outside the VRAM transfer queue).
#include "video_dma.h"

#include "platform.h"
#include "tg3000.h"

// TG3000_ROUTINE($80:83AD) upload_vram_now (long call)
//
// The original takes size in A, source in X with its bank at D+$14, and the VRAM word
// address in Y, and starts DMA channel 0 right away (VMAIN must already be $80).
void upload_vram_now(uint16_t vram_word, uint8_t src_bank, uint16_t src, uint16_t size) {
  video_upload_vram(vram_word, ((uint32_t)src_bank << 16) | src, size ? size : 0x10000);
}

// TG3000_ROUTINE($80:83CB) upload_cgram_now (long call)
//
// The original takes the colour index in A's low byte and the source bank in its high
// byte, the source in X and the size in Y.
void upload_cgram_now(uint8_t index, uint8_t src_bank, uint16_t src, uint16_t size) {
  video_upload_cgram(index, ((uint32_t)src_bank << 16) | src, size ? size : 0x10000);
}

// TG3000_ROUTINE($8A:BA7D) upload_7f_to_3000: 2 KB from $7F:0000 to VRAM $3000.
void upload_7f_to_3000(void) {
  wr16(DP + 0x14, 0x007F);
  upload_vram_now(0x3000, 0x7F, 0x0000, 0x0800);
}

static void upload_tiles(uint32_t table, uint16_t entry, uint16_t offset, uint16_t extra, uint16_t vram_word) {
  wr16(DP + 0x16, offset);
  wr16(DP + 0x14, 0x007F);
  uint16_t src = (uint16_t)(rom16(table + entry) + extra + offset);
  upload_vram_now(vram_word, 0x7F, src, 0x0200);
}

#define TILE_OFFSETS 0x8384B1  // word table in ROM (the routines run with DB = $83)

// TG3000_ROUTINE($85:8245) upload_tiles_3300
void upload_tiles_3300(uint16_t entry, uint16_t offset) { upload_tiles(TILE_OFFSETS, entry, offset, 0, 0x3300); }

// TG3000_ROUTINE($85:8260) upload_tiles_3700
void upload_tiles_3700(uint16_t entry, uint16_t offset) { upload_tiles(TILE_OFFSETS, entry, offset, 0, 0x3700); }

// TG3000_ROUTINE($85:827B) upload_tiles_7700
void upload_tiles_7700(uint16_t entry, uint16_t offset) { upload_tiles(TILE_OFFSETS, entry, offset, 0, 0x7700); }

// TG3000_ROUTINE($85:8296) upload_tiles_7b00
void upload_tiles_7b00(uint16_t entry, uint16_t offset) { upload_tiles(TILE_OFFSETS, entry, offset, 0, 0x7B00); }

// Two more variants at $8A:8D81 / $8A:8DA0 (+$20, VRAM $C280 / $C680) are not ported until
// a movie runs them and shows their data bank.
