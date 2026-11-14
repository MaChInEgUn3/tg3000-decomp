// SPDX-License-Identifier: GPL-3.0-or-later
// The star table at $0F00, in two layouts: 3-word entries (x, y, depth) for the intro's
// 3D starfield, and 4-word entries (speed, y, tile, x) for scrolling star layers. (Named
// from what the code does; the drawing side is not ported yet.)
#ifndef TG3000_STARS_H
#define TG3000_STARS_H

#include <stdint.h>

#define STAR_X 0x0F00      // + slot: signed, random
#define STAR_Y 0x0F02      // + slot
#define STAR_DEPTH 0x0F04  // + slot

// Put the star at byte offset `slot` at a random x/y (each half the range, sign kept)
// and at the far depth $7F00. Returns $7F00.
uint16_t star_spawn(uint16_t slot);

// Give the star at `slot` a random y below $DC00, in whole units (low byte 0).
uint16_t star_random_y(uint16_t slot);

// Fill the 4-word layout with four layers of stars: 2 at speed $100, 10 at $80, 24 at
// $40 and 28 at $20, each at a random y and x with that layer's tile. Returns the end
// offset ($200).
uint16_t star_layers_init(void);

#endif
