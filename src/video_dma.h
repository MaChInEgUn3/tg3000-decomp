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

#endif
