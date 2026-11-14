// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef TG3000_VIDEO_DMA_H
#define TG3000_VIDEO_DMA_H

#include <stdint.h>

// Upload `size` bytes (0 = 64 KB, as the DMA size register) from `src_bank`:`src` to VRAM
// word address `vram_word`, now.
void upload_vram_now(uint16_t vram_word, uint8_t src_bank, uint16_t src, uint16_t size);

// Upload `size` bytes (0 = 64 KB) from `src_bank`:`src` to the palette from colour
// `index`, now.
void upload_cgram_now(uint8_t index, uint8_t src_bank, uint16_t src, uint16_t size);

// 2 KB from the WRAM buffer at $7F:0000 to VRAM $3000. Leaves $14 = $7F.
void upload_7f_to_3000(void);

// Tile uploads from the WRAM buffer at $7F (graphics are unpacked there first): the
// source is a word from the offset table at $83:84B1 (indexed by `entry`, a byte offset)
// plus `offset`; 512 bytes go to the VRAM word address in the name. They leave
// $14 = $7F and $16 = `offset`.
void upload_tiles_3300(uint16_t entry, uint16_t offset);
void upload_tiles_3700(uint16_t entry, uint16_t offset);
void upload_tiles_7700(uint16_t entry, uint16_t offset);
void upload_tiles_7b00(uint16_t entry, uint16_t offset);

#endif
