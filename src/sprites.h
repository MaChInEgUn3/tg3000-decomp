// SPDX-License-Identifier: GPL-3.0-or-later
// The sprite table: 128 entries of 4 bytes (x, y, tile, attributes) that become OAM.
#ifndef TG3000_SPRITES_H
#define TG3000_SPRITES_H

#include <stdint.h>

// Finish a frame's sprite table at `table` (WRAM, 128 entries of 4 bytes): pack the two
// extra bits of each entry (bit 7 of byte 1 and bit 0 of byte 2, both shifted out of
// their bytes) into the 32-byte OAM high table written over the start of `table`
// (entry 4j..4j+3 -> byte j, two bits each, first entry in the low bits). Then the
// entries from slot D+$74 up to the previous frame's end D+$72 are hidden (x/y = $E0).
// `dp` is the caller's direct page. ($81:8101 takes the table at $0200 + D+$6C; $81:80F6
// the one at $0B00.)
void oam_high_table(uint16_t dp, uint16_t table);

// End of frame with the single table at $0B00: oam_high_table, then D+$72 = D+$74 (this
// frame's end) and D+$74 = 0.
void oam_finish_0b00(uint16_t dp);

// End of frame with two sprite buffers: D+$6C is the one just built (its table at
// D+$6C + $200), D+$6E the other. Packs the built one (oam_high_table), keeps the ends of
// the last two frames (D+$72 = D+$70, D+$70 = D+$74), starts the next frame in the other
// buffer (D+$74 = D+$6E - $0900) and swaps D+$6C and D+$6E.
void oam_finish(uint16_t dp);

// The same two, reached by a near call (JSR) from bank $80.
void oam_finish_0b00_near(uint16_t dp);
void oam_finish_near(uint16_t dp);

// $8A:8E1B (long; D = $0100, DB = $8A): turn the 128-slot object table into sprites. A
// slot is in use when its word at $0500 is not 0; $0600 holds x (9 bits), $0700 y (skipped
// when negative or $F0 and over), $0800 the tile word. Each used slot takes the next
// sprite entry at $0900 + D+$2A (x, y bytes, then the tile word) and its two bits in the
// high table at $0B00 + D+$28: the x high bit (D+$2C mask) when x >= $100, and the size bit
// (D+$2E mask) when $0500 is not negative. Entries left over from the last frame (up to
// D+$30) are hidden ($E000), and D+$30 = the new end. Returns it.
uint16_t oam_from_objects(void);

#endif
