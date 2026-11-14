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

// The direct page the game runs with: every routine entry seen so far had D = $0100, so
// the original's "direct page $14" is WRAM $0114. (The bench checks D on every entry.)
#define DP 0x0100

// Read the ROM through a 24-bit bus address (LoROM: bank & $7F selects a 32 KB half).
static inline uint8_t rom8(uint32_t bus) {
  return g_rom[(((bus >> 16) & 0x7F) << 15) | (bus & 0x7FFF)];
}

static inline uint16_t rom16(uint32_t bus) {
  return (uint16_t)(rom8(bus) | (rom8((bus & 0xFF0000) | ((bus + 1) & 0xFFFF)) << 8));
}

// Read/write through a 24-bit bus address, for the original's long and DB-relative
// pointers: $7E/$7F and $0000-$1FFF of banks $00-$3F/$80-$BF are WRAM; other reads come
// from ROM. (No pointer the game follows here points at hardware registers.)
static inline int bus_wram(uint32_t bus, uint32_t* index) {
  uint8_t bank = (uint8_t)(bus >> 16);
  uint16_t adr = (uint16_t)bus;
  if(bank == 0x7E || bank == 0x7F) { *index = ((uint32_t)(bank - 0x7E) << 16) | adr; return 1; }
  if((bank & 0x7F) < 0x40 && adr < 0x2000) { *index = adr; return 1; }
  return 0;
}

static inline uint8_t bus_rd8(uint32_t bus) {
  uint32_t i;
  return bus_wram(bus, &i) ? g_ram[i] : rom8(bus);
}

static inline uint16_t bus_rd16(uint32_t bus) {
  return (uint16_t)(bus_rd8(bus) | (bus_rd8(bus + 1) << 8));
}

static inline void bus_wr8(uint32_t bus, uint8_t val) {
  uint32_t i;
  if(bus_wram(bus, &i)) wr8(i, val);
}

static inline void bus_wr16(uint32_t bus, uint16_t val) {
  uint32_t i;
  if(bus_wram(bus, &i)) wr16(i, val);
}

static inline uint32_t rd32(uint32_t adr) { return rd16(adr) | ((uint32_t)rd16(adr + 2) << 16); }

static inline void wr32(uint32_t adr, uint32_t val) {
  wr16(adr, (uint16_t)val);
  wr16(adr + 2, (uint16_t)(val >> 16));
}

#endif
