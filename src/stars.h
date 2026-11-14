// SPDX-License-Identifier: GPL-3.0-or-later
// A table of 3-word entries at $0F00 (x, y, depth), filled with random positions: most
// likely the starfield of the intro and menus. (Named from what the code does; the
// drawing side is not ported yet.)
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

#endif
