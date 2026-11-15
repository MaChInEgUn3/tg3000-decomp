// SPDX-License-Identifier: GPL-3.0-or-later
// Immediate uploads to video memory (outside the VRAM transfer queue).
#include "video_dma.h"

#include "decompress.h"
#include "line_tables.h"
#include "scale.h"
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

// TG3000_ROUTINE($80:8381) decompress_to_vram
// TG3000_ROUTINE($80:837D) decompress_to_vram (long call)
uint16_t decompress_to_vram(uint16_t dp, uint8_t src_bank, uint16_t src, uint16_t vram_word) {
  uint16_t end;
  uint16_t len = decompress_near(dp, src_bank, src, 0x7F, 0x0000, &end);
  video_upload_vram(vram_word, 0x7F0000, len ? len : 0x10000);
  return end;
}

// TG3000_ROUTINE($8A:8DEC) clear_vram (long call)
void clear_vram(void) {
  video_fill_vram(0x0000, rom8(0x8A8E19), 0x10000);  // the fill byte is kept in the ROM
}

// Upload part of a picture built in WRAM, then step the frame counter $013E and rebuild
// the line tables (inverse_scale, func_80F745).
static uint16_t upload_half_sized(uint16_t vram_word, uint32_t src, uint32_t size) {
  video_upload_vram(vram_word, src, size);
  wr16(DP + 0x3E, (uint16_t)(rd16(DP + 0x3E) + 1));
  inverse_scale(DP);
  return func_80F745();
}

// TG3000_ROUTINE($80:F3C4) func_80F3C4
uint16_t func_80F3C4(void) { return upload_half_sized(0x3100, 0x7E2000, 0x04A0); }

// TG3000_ROUTINE($80:F3F4) func_80F3F4
uint16_t func_80F3F4(void) { return upload_half_sized(0x3350, 0x7E24A0, 0x04A0); }

// TG3000_ROUTINE($80:F4AD) upload_slice: slice 0 (and $80:F4B9, $80:F4C5, ... every 12
// bytes up to $80:F561 for slices 1-15; they share the tail at $80:F56D)
uint16_t upload_slice(int slice) {
  return upload_half_sized((uint16_t)(slice * 0x030A), 0x7E3040 + (uint32_t)slice * 0x0614, 0x0614);
}

// TG3000_ROUTINE($8A:B994) func_8AB994
uint16_t func_8AB994(uint16_t dp) {
  decompress_to_vram(dp, 0x88, 0xFEC3, 0x0800);
  return decompress_to_vram(dp, 0x88, 0xF87F, 0x1000);
}
