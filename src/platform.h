// SPDX-License-Identifier: GPL-3.0-or-later
// Services the game logic needs from the platform. Each port implements them natively
// (no emulated hardware); the bench implements them by recording their effect so the
// differential test can compare it with what the original did through the hardware.
//
// Addresses of data in the game's memory are 24-bit SNES bus addresses (bank:address):
// LoROM ROM or WRAM ($7E/$7F, or $0000-$1FFF of banks $00-$3F/$80-$BF).
#ifndef TG3000_PLATFORM_H
#define TG3000_PLATFORM_H

#include <stdint.h>

// Copy `size` bytes (1-65536) from `src` into video RAM starting at word address
// `vram_word`, low byte then high byte of each word, one word after another.
// (The original does this with DMA in mode 1 to $2118 with VMAIN = $80.)
void video_upload_vram(uint16_t vram_word, uint32_t src, uint32_t size);

// Fill `size` bytes (1-65536) of video RAM from word address `vram_word` with `value`
// (both bytes of every word). (The original does this with a fixed-source DMA to
// $2118/$2119, VMAIN = $80.)
void video_fill_vram(uint16_t vram_word, uint8_t value, uint32_t size);

// Write one word to video RAM at word address `vram_word`. (The original writes $2118 then
// $2119 with VMAIN = $80; the console takes it only in vblank or under forced blank, which
// is when the game does it.)
void video_write_vram_word(uint16_t vram_word, uint16_t value);

// Copy `size` bytes (1-65536) from `src` into the palette starting at colour `index`;
// each colour is two bytes, low byte first (BGR555).
void video_upload_cgram(uint8_t index, uint32_t src, uint32_t size);

// PPU registers, set as the original writes them (both bytes, low first); the console
// latches them and they take effect on the next lines it draws (see docs/DECISIONS.md,
// "PPU registers").
//
// Mode 7 matrix parameters A-D ($211B-$211E): signed 8.8 fixed point.
void video_set_m7a(int16_t value);
void video_set_m7b(int16_t value);
void video_set_m7c(int16_t value);
void video_set_m7d(int16_t value);

// Mode 7 centre X / Y ($211F / $2120): 13 bits, signed.
void video_set_m7x(int16_t value);
void video_set_m7y(int16_t value);

// BG1 horizontal / vertical scroll ($210D / $210E). The console keeps 10 bits as the BG1
// scroll and, from the same write, 13 bits (signed) as the Mode 7 offset.
void video_set_bg1_hofs(uint16_t value);
void video_set_bg1_vofs(uint16_t value);

// BG2 horizontal / vertical scroll ($210F / $2110): 10 bits.
void video_set_bg2_hofs(uint16_t value);
void video_set_bg2_vofs(uint16_t value);

// Return after the next interrupt has run. On the console the NMI and IRQ handlers do
// the frame's work; they are game code too (ported to C like the rest), so a port
// presents the frame here and then runs the game's NMI code, and runs its IRQ code at
// the line the game asked for (see docs/DECISIONS.md, "HDMA, measured"). Game code that
// waits for "the next frame" calls this in a loop until its own frame counter has moved on.
void platform_wait_vblank(void);

#endif
