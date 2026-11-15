// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef TG3000_HUD_H
#define TG3000_HUD_H

#include <stdint.h>

#define METER_VALUE 0x28    // input (word, signed): the meter's value
#define METER_SCRATCH 0x14  // output (word): the bar's length in eighths of a tile

#define METER_VRAM 0x00C2   // the bar's first tile in the tilemap
#define METER_TILES 10
#define METER_FULL 0x0847   // tilemap entries: a full tile, an empty one, and
#define METER_EMPTY 0x0848  // METER_PART + n for a tile filled n eighths (1-7)
#define METER_PART 0x0840

// Draw a 10-tile bar at tilemap word METER_VRAM: its length in eighths is
// value * 10 / 256 + 2 (the product computed on the PPU's multiplier in the original),
// as full tiles, then one partial tile, then empty ones up to ten. Lengths past 80 run
// over the ten tiles as the original does (its tile count goes negative and the empty
// fill runs on until it wraps). Returns the length.
uint16_t draw_meter(uint16_t dp);

#endif
