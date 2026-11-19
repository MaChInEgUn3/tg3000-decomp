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

// $80:DACF: every star (8-byte slots at $0F00, 64 of them) moves: position ($0F06) minus
// speed ($0F00), wrapping by + $8800 when it passes 0.
void star_layers_move(void);

// $80:F950: the title's 128 stars drawn as sprites (D = $0100; A = the step toward the
// viewer, kept in D+$16; X's low byte the multiplier, kept in D+$18). Each star (6 bytes
// from $0F00: x, y, depth, speed byte at +5) is projected with the CPU's divider: screen x
// = $80 + (x + speed * multiplier) / speed, y = $5C + (y - speed * multiplier) / speed
// (a quotient taken from the magnitude, then added or subtracted; the divider gives $FFFF
// for 0). Off screen (x past a byte, y $E0 or more), or once the depth less the step falls
// below $100, the star is spawned again (star_spawn) and redone; else its OAM entry at
// $0900 + D+$74 (x, y; tile $81:D692 + (speed & $7C) | D+$80; $0B01 cleared) and its new
// depth are written. D+$74 moves on by 4 a star.
void stars_draw(uint16_t step, uint16_t multiplier);
// $8A:9773: the same in bank $8A (tiles from $86:F46B; its star_spawn copy).
void stars_draw_8a(uint16_t step, uint16_t multiplier);

#endif
