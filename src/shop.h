// SPDX-License-Identifier: GPL-3.0-or-later
// The shop and the screens between races (bank $80, $80:98xx-$80:B2xx).
#ifndef TG3000_SHOP_H
#define TG3000_SHOP_H

#include <stdbool.h>
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

// A double bar ($80:A91E): its 2-sprite frame at the slot D+$74, then two
// shop_bar_column bars for the bar entry's first two parts (levels capped as in
// shop_bar_levels; tiles $231E and $2320, y offsets $2C and $33).
void shop_bar_double(void);

// The gauge ($80:AA59): the sprite group at $7E:3438 ($7E:33DA at stage $10), then caps
// (shop_bar_cap) for the stage past $10 (one partial, then full ones to $38) and past $0C.
void shop_gauge(void);

// The current item's level marks and price ($80:A26E): for stages $10-$17, two words of
// $7E:4DCA to $01A2/$01A4 + D+$C4; then the item's 32-bit price (BCD) as a number centred
// at $5C2E + D+$E7 with sprite base $3A00 (D+$14 and D+$16 kept). Returns
// sprite_number_centered's shift.
uint16_t shop_item_price(void);

// The item screens' handlers, run by the dispatch at $80:A251 (D+$16 in A, the item's bar
// table index in Y): each draws its bars and sprite groups ($7E:xxxx groups from the
// bar entry's words) and then advances the stage D+$3E (stopping at $7FFF):
// - shop_item_view_a ($80:A2CB): level marks, segments, two groups;
// - shop_item_view_b ($80:A831): a double bar, long segments, two groups;
// - shop_item_view_c ($80:AA15): a group, the gauge, a group.
void shop_item_view_a(uint16_t entry);
void shop_item_view_b(uint16_t entry);
void shop_item_view_c(uint16_t entry);
// - shop_item_view_d ($80:A717): a group, a bar of 2x2 sprite blocks (shop_sprite_block,
//   tile $2316; the part under way $2316 + (stage / 2) & 7), two groups;
// - shop_item_view_e ($80:AB54): three groups, then six words of the wave tables at
//   $7E:4E1A-$7E:4EBA (by D+$40, back and forth over 16 words) to $0122-$012E + D+$C4,
//   and D+$40 + 1.
void shop_item_view_d(uint16_t entry);
void shop_item_view_e(uint16_t entry);

// The item's icon ($80:A1E4): the sprite group `group` at (D+$42, D+$E8), palette $0E00
// while D+$44 < 8, $2E00 after; then, while D+$44 is not negative, its colours: two words
// of $7E:4EDA/$7E:4EFA (and below step 8 four of $7E:4F1A-$7E:4F4A) to the palette copy
// at D+$C4; D+$44 counts on, holding at bit 15 after step 15 (or step 7 unless D+$76 is
// negative).
void shop_item_icon_draw(uint16_t group);

// The second item dispatch's handlers, one per list (the part level byte of D+$94 + $0393
// .. $0397). Carry clear: the icon of that level (shop_item_icon_draw); carry set: queue the
// level's panel upload (func_80B537). ($80:9F5B list 1, $80:9F9A list 2, $80:9FD9 list 3,
// $80:A057 list 4, $80:A018 list 5)
void shop_item_icon_1(bool upload);
void shop_item_icon_2(bool upload);
void shop_item_icon_3(bool upload);
void shop_item_icon_4(bool upload);
void shop_item_icon_5(bool upload);

// The cursor and the level marks ($80:9E49): D+$EE eases one pixel a frame towards the
// cursor row (D+$4A * 16 + $14); two cursor sprites (tiles $3892, $3890), then one mark per
// level of list D+$48 (the byte at $036E + list), the part's current level (the list's
// getter of $80:9EEA, low 7 bits) drawn with tile + $20; slots from D+$74.
void shop_level_marks(void);

// The shop panel's map buffer in $7F, from D+$5E:
// - shop_panel_blank ($80:B0C0): 16 rows of 8 words (row stride $20) at $7F:0700 set to
//   $FFFF; D+$4E = 0 and the frame hook D+$EC = $B149;
// - shop_panel_clear ($80:B10C): with the frame hook D+$EC = $B94F, wait until the upload
//   queue is empty (D+$8C = D+$8E), then 9 rows of 9 words (row stride $40) at $7F:00C4
//   set to 0.
void shop_panel_blank(void);
void shop_panel_clear(void);

#endif
