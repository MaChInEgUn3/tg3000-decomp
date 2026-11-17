// SPDX-License-Identifier: GPL-3.0-or-later
// The shop and the screens between races (bank $80, $80:98xx-$80:B2xx).
#ifndef TG3000_SHOP_H
#define TG3000_SHOP_H

#include <stdint.h>

// One of the car's six part levels ($0392-$0397 + the car's offset), plus 1.
uint16_t shop_part_level_2(uint16_t car);  // $80:9F23 ($0392)
uint16_t shop_part_level_3(uint16_t car);  // $80:9F0A ($0393)
uint16_t shop_part_level_4(uint16_t car);  // $80:9F0F ($0394)
uint16_t shop_part_level_5(uint16_t car);  // $80:9F14 ($0395)
uint16_t shop_part_level_6(uint16_t car);  // $80:9F19 ($0396)
uint16_t shop_part_level_7(uint16_t car);  // $80:9F1E ($0397)

// Copy the player's money ($038E/$0390 + $0194) to $52/$54, mark $56 = $FFFF and set
// $76 bit 2. ($80:993E)
void shop_money_take(void);

// $01FB = $12. ($80:E4D0, long)
void shop_flag_fb(void);

// Queue the shop panel's VRAM upload: $03C0 bytes from $7F:$5E to VRAM $5C + $0400.
// Returns func_80B537's result. ($80:9DBC)
uint16_t shop_panel_upload(void);

// The current item's picture: func_80B58D with $81 and the item's table entry. ($80:9BF2)
void shop_item_picture(void);

// Copy the current item's text block (from bank $89) to $0100 + $01C4. ($80:B096)
void shop_item_text(void);

// Four sprite entries of a 2x2 block at OAM slot `slot` (byte offset): tile words from
// $7E's low byte | $16 (+$0800, +$1000, +$1400), positions `pos`, attributes $7E, high
// bits 0. Returns slot + $10. ($80:A7E4)
uint16_t shop_sprite_block(uint16_t pos, uint16_t slot);

// A bar's cap: four sprites in a column at the slot D+$74 (y = D+$80 + D+$14 - 1, the
// tile row ($51 - D+$7A + D+$E8) << 8 | y; tiles D+$7E x3 and D+$7E + 1), then D+$74 += $10.
// Returns the new slot. ($80:AAF1)
uint16_t shop_bar_cap(void);

// A bar's three-sprite column at the slot D+$74 (y = D+$14 + D+$80; tile row
// ((($6C - D+$7A - D+$7E) & $FF) << 8) + D+$E7, kept in D+$82; tiles D+$84, then
// D+$84 + 1 twice), then D+$74 += $0C. Returns the new slot. ($80:A9BD)
uint16_t shop_bar_column(void);

// The shop's level bars, from the stage D+$3E (in half steps) and the bar table at $81:A896
// (D+$7C picks it; 4-byte entries: start, length):
// - shop_bar_levels ($80:A403): six 2-sprite marks, each at its entry's level (the stage
//   less the start, at most the length), then six words of the tile table at $81:A882
//   (by the level, at most 7) stored at $81:A876's offsets + D+$C4;
// - shop_bar_segments ($80:A4B8): the bar of the stage past $10, in segments of 6 drawn
//   as 3-sprite columns (one level on the half segment at $1B); nothing when the stage
//   fills it;
// - shop_bar_segments_long ($80:A87D): the same up to $24, without the half segment.
void shop_bar_levels(void);
void shop_bar_segments(void);
void shop_bar_segments_long(void);

#endif
