// SPDX-License-Identifier: GPL-3.0-or-later
// Top Gear 3000 — readable C of the game logic.
//
// Game state lives in one flat array at the original WRAM offsets, so the bench can
// compare it byte for byte with the original game. Always access it through the
// helpers below (explicit little-endian), never by casting pointers: the code must
// also build on big-endian CPUs such as the 68000.
#ifndef TG3000_H
#define TG3000_H

#include <stdbool.h>
#include <stdint.h>

#define TG3000_WRAM_SIZE 0x20000

extern uint8_t g_ram[TG3000_WRAM_SIZE];  // $7E:0000-$7F:FFFF
extern const uint8_t* g_rom;              // the 1 MB ROM image, as file offsets

static inline uint8_t rd8(uint32_t adr) { return g_ram[adr & (TG3000_WRAM_SIZE - 1)]; }

static inline uint16_t rd16(uint32_t adr) {
  return (uint16_t)(rd8(adr) | (rd8(adr + 1) << 8));
}

static inline void wr8(uint32_t adr, uint8_t val) { g_ram[adr & (TG3000_WRAM_SIZE - 1)] = val; }

static inline void wr16(uint32_t adr, uint16_t val) {
  wr8(adr, (uint8_t)val);
  wr8(adr + 1, (uint8_t)(val >> 8));
}

static inline uint32_t rd32(uint32_t adr) { return rd16(adr) | ((uint32_t)rd16(adr + 2) << 16); }

static inline void wr32(uint32_t adr, uint32_t val) {
  wr16(adr, (uint16_t)val);
  wr16(adr + 2, (uint16_t)(val >> 16));
}

#endif
