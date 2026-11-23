// SPDX-License-Identifier: GPL-3.0-or-later
// The shop and the screens between races (bank $80, $80:98xx-$80:B2xx).
#ifndef TG3000_SHOP_H
#define TG3000_SHOP_H

#include <stdbool.h>
#include <stdint.h>

// The shop's lists and the car's bytes they set (+ the car's offset D+$94: 0, or $2F for
// player 2). The names are the shop's own (its menu) and those of public cheat lists
// (docs/DECISIONS.md "Names from public cheat lists"):
//   list 0 Paintshop ($0392, the colour), 1 Engine ($0393), 2 Tires ($0394), 3 Gearbox
//   ($0395), 4 Armor ($0396), 5 Boost ($0397); then the specials, whose byte is the row + 1:
//   6 Jump ($0398), 7 Attractor ($0399), 8 Warp ($039A), 10 'P' ($039B), 9 Infra-Red ($039C).
// One of the car's six part levels ($0392-$0397 + the car's offset), plus 1.
uint16_t shop_part_level_2(uint16_t car);  // $80:9F23 ($0392)
uint16_t shop_part_level_3(uint16_t car);  // $80:9F0A ($0393)
uint16_t shop_part_level_4(uint16_t car);  // $80:9F0F ($0394)
uint16_t shop_part_level_5(uint16_t car);  // $80:9F14 ($0395)
uint16_t shop_part_level_6(uint16_t car);  // $80:9F19 ($0396)
uint16_t shop_part_level_7(uint16_t car);  // $80:9F1E ($0397)
// Lists 6-10's levels as they are (the row + 1): $0398-$039C + the car's offset.
uint16_t shop_part_level_8(uint16_t car);  // $80:9F30 ($0398, list 6)
uint16_t shop_part_level_9(uint16_t car);  // $80:9F2C ($0399, list 7)
uint16_t shop_part_level_a(uint16_t car);  // $80:9F28 ($039A, list 8)
uint16_t shop_part_level_b(uint16_t car);  // $80:9F38 ($039B, list 10; never run)
uint16_t shop_part_level_c(uint16_t car);  // $80:9F34 ($039C, list 9)

// Copy the player's money ($038E/$0390 + $0194) to $52/$54, mark $56 = $FFFF and set
// $76 bit 2. ($80:993E)
void shop_money_take(void);
// $80:9957: the money after the current item's price (decimal SBC, the item's price at
// $81:A890/$81:A88E + its word at $81:A89B) to D+$52/D+$54. The leading digits that neither
// the money nor that result has, or that stay the same, are marked settled (D+$58, then the
// low word's zeros in D+$56, a path no run takes); D+$5A = 4. D+$7A/$7C: money | result;
// D+$7E/$80 the last digit compared. Returns Y at the exit (the digits left unchecked).
uint16_t shop_money_after_price(void);
// The original's carry and overflow at the exits of the two below (for the bench; the game's
// callers reload the flags). shop_money_roll_flags leaves `v` as given when no SBC/ADC ran.
typedef struct { bool c, v; } ShopMoneyFlags;
uint16_t shop_money_after_price_flags(ShopMoneyFlags* f);
// $80:99D1: one step of the money rolling down to D+$52/D+$54. The settled digits from the
// top are passed over; at the first other one, D+$5A counts down: at 0 that digit takes its
// new value and settles (D+$5A = 4), else it and every digit below roll down by one (0 to
// 9). D+$7A/$7C the digit mask, D+$84 the digits left. Returns the original's A at its RTS.
uint16_t shop_money_roll(void);
uint16_t shop_money_roll_flags(ShopMoneyFlags* f);
// $80:9ABB: D+$52/D+$54 become the player's money ($038E/$0390 + D+$94).
void shop_money_store(void);
// $80:990D: the money plus the price of row `row` of the current list (a part sold back) to
// D+$52/D+$54, D+$76 bit 2 cleared, then the settled digits as shop_money_after_price's.
// Returns Y at the exit.
uint16_t shop_money_after_refund(uint16_t row);
// The same, with the flags its shared tail leaves (as shop_money_after_price_flags).
uint16_t shop_money_after_refund_flags(uint16_t row, ShopMoneyFlags* f);
// $80:9053/$9100/$91AE/$925C/$95C0: buying a part of the car from lists 1-5 (the level bytes
// $0393-$0397 + D+$94; bit 7: bought here). Already owned: a sound and the icon. Else the
// money is taken and, if it covers the price, the panel rises with the money rolling, the
// level byte becomes the row | $80, the upload is queued, and the panel falls with the money
// rolling to its new value; else shop_flag_fb. A part bought here before is sold back first.
// The panel rises to $78 for list 1, $B4 for the others.
// Which way shop_buy_part went: the part already owned, the money short (a refund first: a
// path the shop-buy movie runs once), or bought.
typedef enum { SHOP_BUY_OWNED, SHOP_BUY_SHORT, SHOP_BUY_REFUNDED_SHORT, SHOP_BUY_BOUGHT } ShopBuyEnd;
ShopBuyEnd shop_buy_part(uint16_t field, uint16_t height, bool price_first, bool settle, ShopMoneyFlags* f);
void shop_buy_list1(void);
void shop_buy_list2(void);
void shop_buy_list3(void);
void shop_buy_list4(void);
void shop_buy_list5(void);
// Lists 6-8 (shop_buy_list6-8, $80:94DE, $80:93FC, $80:931A): the same buying, with the level
// byte at $0398/$0399/$039A + D+$94 holding the row + 1 (0: nothing bought yet; bit 7: bought
// here). The first purchase skips the panel's rise: it is set at $B4 at once. A level held
// from elsewhere (bit 7 clear) is not sold back.
ShopBuyEnd shop_buy_upgrade(uint16_t field, ShopMoneyFlags* f);
void shop_buy_list6(void);
void shop_buy_list7(void);
void shop_buy_list8(void);
// Lists 9 and 10 ($80:966E, $80:9750): the same, with $039C and $039B. No run reaches list
// 10's (its one entry is at $81:AE20).
void shop_buy_list9(void);
void shop_buy_list10(void);
// $80:9039: the current item's buying handler is run: the word at $81:A89F in the item's
// entry (list D+$48, row D+$4A; shop_buy_handler returns it).
uint16_t shop_buy_handler(void);
void shop_buy_dispatch(void);
// $80:930A: list 0, the car's colour: $0392 + D+$94 = the row (a byte), then func_80E4DB.
void shop_buy_colour(void);

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
// - shop_item_view_g ($80:ABF6, list 0): two groups (fields 0 and 2).
void shop_item_view_g(uint16_t entry);

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
// Lists 6-10 the same, with their level bytes ($0398, $0399, $039A, $039C, $039B: the row + 1)
// and level 0 queueing nothing ($80:A121 list 6, $80:A0E0 list 7, $80:A09F list 8, $80:A162
// list 9, $80:A1A3 list 10); list 0 ($80:A096) a fixed icon ($7E:4F6A) and no upload.
void shop_item_icon_0(bool upload);
void shop_item_icon_6(bool upload);
void shop_item_icon_7(bool upload);
void shop_item_icon_8(bool upload);
void shop_item_icon_9(bool upload);
void shop_item_icon_10(bool upload);

// The second item dispatch: the current item's handler (by the table at $81:A8A3, list
// D+$48, row D+$4A), with the carry clear ($80:9F40: its icon) or set ($80:9C0B: queue its
// panel upload). The handlers of lists 0 and 6-8 run only here: list 0 ($80:A096) a fixed
// icon and no upload; lists 6-8 ($80:A121, $80:A0E0, $80:A09F) as lists 1-5 on $0398-$039A,
// but a level-0 part queues no upload.
void shop_item_dispatch_icon(void);
void shop_item_dispatch_upload(void);

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
// $80:B149: the shop panel's frame (the callback at D+$EC, set by shop_panel_blank). While
// the icon animates (D+$44 negative, counting down to $8000) its frame's six words ($7E:4EDA
// on, by D+$44 * 2) go to $01EE/$01F0/$0182/$0186/$0188/$018E + D+$C4, and, while the text
// types, the icon, the level marks and the labels are redrawn. Once the text is typed (D+$8C
// = D+$8E): a typewriter step, the panel's static (random tiles in its map at $7F:00C4 and
// random words at $7F:0710, + D+$5E), both uploaded (func_80B537), D+$4E counts.
void shop_panel_frame(void);

// The list cursor's range check ($80:B359): (D+$48 * 8) in the high byte, swapped with
// its low byte, plus D+$4A; true (carry set) above $1800 or negative.
bool shop_list_locked(void);

// $80:B2F2: list `x` / 2's name ($80:B371 + x: a string in bank $80), centred at row D+$22
// (+ D+$E7) while the row is within $1000-$57FF (signed); the row moves on by $800 and x by
// 2. At x = $20 that is all; a list whose count ($036E + x / 2) is 0 ends the run there:
// D+$F0 = x / 2 and x = $1E. Returns the new x.
uint16_t shop_list_label(uint16_t x);
// Step the list cursor D+$48 down ($80:B333, not below 0) or up ($80:B345, not past D+$F0)
// unless locked; a step clears D+$4E and plays sound $11 (func_80E4C5).
void shop_list_prev(void);
void shop_list_next(void);
// - shop_item_view_f ($80:A333): the spinning ring (shop_ring), a group, segments
//   (tile $2338), two groups.
void shop_item_view_f(uint16_t entry);

// One sprite of the spinning ring at slot `slot`: its frame (bits 8-11 of `angle`) picks a
// 6-byte entry (tile, x, y) of $7E:4D0A (front) or $7E:4D6A (back), placed from D+$18 (y)
// and D+$1A (row). Returns slot + 4. ($80:A69F, $80:A6DB)
uint16_t shop_ring_piece_front(uint16_t angle, uint16_t slot);
uint16_t shop_ring_piece_back(uint16_t angle, uint16_t slot);

// The spinning ring ($80:A579): two pieces (front at angle D+$40, back at D+$40 + $400; the
// four-piece ring is $80:A5F0) around a centre
// sprite (tile $2526) at D+$14 + $2B, row $5E - D+$7A + D+$E8; the pieces with bit 11
// set first (behind), the others after; then D+$40 += $80.
void shop_ring(void);

// The views of lists 6-8 (only the item dispatch reaches them):
// - shop_item_view_h ($80:AEBE, list 6): a group, the level blocks (shop_level_blocks,
//   tile $257B), the frame of $81:BD44 by D+$40's high byte, the group $7E:46BA; D+$40
//   + $40, wrapping at $1E00;
// - shop_item_view_i ($80:AE4E, list 7): a group, the group $7E:4686, the frame of
//   $81:BD26; D+$40 + $40, wrapping at $0F00;
// - shop_item_view_j ($80:AC37, list 8): a group, the frame of $81:BD06 (by bits 8-11 of
//   D+$40), the gauge (shop_gauge_lines), four palette words of $7E:4F5A + (bits 8-9 of
//   D+$40) * 2 to $0110 + D+$C4; D+$40 + $40, D+$46 - 1.
// Each ends with the stage step.
void shop_item_view_h(uint16_t entry);
void shop_item_view_i(uint16_t entry);
void shop_item_view_j(uint16_t entry);

// $80:AF36: a row of 2x2 blocks filling with the stage: (D+$3E / 2 - $10) / 6 blocks full,
// the next at tile D+$84 + 2 * the remainder; 7 blocks, the last half a block over.
// Returns true when all seven blocks are full with three or more left over: nothing drawn.
bool shop_level_blocks(void);

// $80:ACB6: the gauge: four sprites (tile $032F) at D+$14 + $40, then lines of
// sprites from the strings at $7E:44BA-$7E:44E0 (shop_gauge_line); twelve lines in all.
void shop_gauge_lines(void);

// $80:ADD9: one line of the gauge: starting column ((y + D+$46 (15 bits) * `speed`) >> 5
// & $7F) - $20, the tiles of the zero-ended string at $7E:`string`, eight pixels apart,
// up to column $48; row D+$7E. The PPU multiplies: matrix A = D+$46 & $7FFF, matrix B =
// `speed` in its high byte.
void shop_gauge_line(uint8_t speed, uint16_t y, uint16_t string);

// The item dispatch ($80:A251): the current item's view by the table at $81:A897 (list
// D+$48, row D+$4A), called with its bar entry ($81:A8A1).
void shop_item_dispatch_view(void);

// The item's panel ($80:9C28): by the table at $81:A899 (list, row), one handler per list
// ($80:9C47 ... $80:9D9E) queues one or two uploads (func_80B537, priority $7F) of the
// item's pictures: VRAM D+$5C + a fixed offset per handler, from D+$5E + $0900 + the
// picture's offset, with its size; the pictures' (offset, size) pairs are at $81:A892 +
// the item's word at $81:A89B (the first pair) and + 4 (the second).
void shop_item_dispatch_panel(void);
// Its handlers ($80:9C47 list 1, $80:9C85 list 2, $80:9CC3 list 3, $80:9D02 list 5, $80:9D41
// list 4, $80:9D80 list 0, $80:9D9E lists 6-10): one or two picture uploads each, from the
// pairs at $81:A892 + `pictures` (Y).
void shop_item_panel_0(uint16_t pictures);
void shop_item_panel_1(uint16_t pictures);
void shop_item_panel_2(uint16_t pictures);
void shop_item_panel_3(uint16_t pictures);
void shop_item_panel_4(uint16_t pictures);
void shop_item_panel_5(uint16_t pictures);
void shop_item_panel_6(uint16_t pictures);

// The item screen's frame ($80:9847): with the upload queue empty, the next typewriter
// step (text_type_step) and the panel's upload; then the price at (16, 24), the item's
// view, its icon, the level marks, the labels and the sprite table's end.
void shop_item_redraw(void);

// The item changes, sliding six pixels a frame (each frame: wait for it, the typewriter
// and panel when the queue is empty, then the view, icon, marks, labels and sprites at
// (D+$14, D+$16)); between the two halves, once the upload queue has drained, the new item
// (row D+$4C) takes over: stage, spin and timer cleared, its panel and its text.
// - shop_item_slide_row ($80:9AC8): the old item up and out (y $18 down to -$46), the new
//   one in from the left (x -$3E up to $10);
// - shop_item_slide_list ($80:9B5D): the old item out to the left (x $10 down to -$3E),
//   then shop_item_slide_list_in;
// - shop_item_slide_list_in ($80:9B99): the new item, down from the top (y -$36 to $18).
void shop_item_slide_row(void);
void shop_item_slide_list(void);
void shop_item_slide_list_in(void);

// The player moves down ($80:8FA1) or up ($80:8FD4) the item list: the next item slides in
// (down: out to the left, shop_item_slide_list; up: out the top, shop_item_slide_row) with
// sound $11 (up plays it even at the top). Past the end, the list is left: the icon stops
// (D+$44 bit 15), the panel blanks, and once D+$4E has passed 4 (the frame hook counts it)
// the panel clears; the routine then returns from its caller too (true).
bool shop_item_down(void);
bool shop_item_up(void);

// $80:9832: the item screen runs (a frame each: wait, shop_item_redraw) with D+$76 bit 15
// set and the icon's step D+$44 restarted (bit 15 clear) until the icon has finished.
void shop_item_show_icon(void);

// $80:98C4: can the player pay? The money D+$54:$52 (BCD) against the item's price
// ($81:A88E:$81:A890 + the item's word at $81:A89B); if short, the price flashes for 16
// frames (two colours from $7E:4DDA/$7E:4DFA to $01C2/$01C4 + D+$C4, the screen running)
// and false.
bool shop_item_affordable(void);

// $80:9870: the icon's colours step back down (D+$44 counts down to $8007), the screen
// running; then D+$76 bit 15 clears.
void shop_item_icon_settle(void);

#endif
