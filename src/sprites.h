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

#endif
