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

// Unpack the stream at `src_bank`:`src` (the game's compression, see decompress) to
// $7F:0000 and copy the result to video RAM from word `vram_word`. Returns the address
// just past the stream. (The original takes the source bank in A's low byte and ORs A with
// $7F00 for the destination bank; the game's high byte is always below $80, i.e. $7F.)
uint16_t decompress_to_vram(uint16_t dp, uint8_t src_bank, uint16_t src, uint16_t vram_word);

// decompress_to_vram of $88:FEC3 to VRAM $0800 and of $88:F87F to VRAM $1000. Returns the
// second one's result.
uint16_t func_8AB994(uint16_t dp);

// Fill all of video RAM with the byte at $8A:8E19 (both bytes of every word).
void clear_vram(void);

// Upload the first ($7E:2000 to VRAM $3100) or second ($7E:24A0 to $3350) 1184-byte half
// of a picture, then $013E += 1, inverse_scale and func_80F745 (whose result is returned).
uint16_t func_80F3C4(void);
uint16_t func_80F3F4(void);

// Upload slice `slice` (0-15) of a picture built from $7E:3040: 1556 bytes from
// $7E:3040 + slice * $614 to VRAM word slice * $30A; then as func_80F3C4 ($013E += 1, line
// tables). Returns func_80F745's result. (16 entries in the original, $80:F4AD + 12 * slice.)
uint16_t upload_slice(int slice);

#endif
