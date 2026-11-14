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

// Copy `size` bytes (1-65536) from `src` into the palette starting at colour `index`;
// each colour is two bytes, low byte first (BGR555).
void video_upload_cgram(uint8_t index, uint32_t src, uint32_t size);

// Return after the next interrupt has run: the platform does the frame's work (what the
// original's NMI and IRQ handlers do) and then comes back. Game code that waits for
// "the next frame" calls this in a loop until its own frame counter has moved on.
void platform_wait_vblank(void);

#endif
