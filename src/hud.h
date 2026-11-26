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

// $80:DAED: twelve lights as sprites from slot D+$74, light n at the place of $81:CB14 +
// 2n. Those below $0368 are lit: their sprite from $81:CB44/$81:CB46 (by $81:CB2C), high
// table $00 for the first, $01 for the rest. Light $0368 is the one under way: lit while
// D+$3C is 0; else, while D+$26 is 0, D+$3C counts down (at 0, func_80D6F3 steers to
// its place) and it blinks by bit 4 of the count (tile $25C3 when off); with D+$26 set
// it is off. The rest are unlit (tile $25C2).
void race_lights_draw(uint16_t dp);

// $80:D823: the race HUD, one frame (D = $0100):
// - the two window shapes (window_shape_a with D+$37:D+$35's bytes, window_shape_b with
//   D+$24) and, when 0 - D+$22 is on screen, the sprite group $81:C272;
// - the readouts, in attribute $2200: the shown speed D+$28 (with a point), the same times
//   $4D * 2 / 256 (the other unit, rounded by the product's bit 6; the PPU multiplier),
//   D+$35 * $63 / 256 and D+$37 * $41 / 128 (the PPU multiplier), and three names from the
//   tables at $86:CEA7 (by $0368), $86:CEBF and $86:D0A7 (by $0444), centred;
// - race_lights_draw; the shown speed moves toward D+$32 by $10 at most (down: never run);
// - the sprite groups of the list at $0700 (10-byte entries; the second is drawn twice, as
//   the original's loop does) that are on screen, and of the first entry (never run);
// - the OAM from the slot $0174 on: two 2-tile sprites, then one-tile ones, from the
//   8-byte entries at $0F00 (x byte at +$06 rounded to a signed column less 8), until the
//   table or the OAM ends; $0174 = the slot after.
void race_hud_sprites(uint16_t dp);

#endif
