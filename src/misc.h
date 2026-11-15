// SPDX-License-Identifier: GPL-3.0-or-later
// Routines whose purpose is not known yet are named func_<address> until it is.
#ifndef TG3000_MISC_H
#define TG3000_MISC_H

#include <stdbool.h>
#include <stdint.h>

// $81:F251: an empty routine (a lone RTS) that 19 places call.
void func_81F251(void);

// Clear the direct-page scratch area $0114-$0193 (64 words).
void clear_dp_scratch(void);

// Link the 20-entry order list into "next" pointers: starting from the index in the
// first entry, each entry's value is stored at $FACD + the previous value, and the list
// is closed by storing the first entry at $FACD + the index in $1E90. Most likely the
// race order (20 cars). Returns the first entry.
uint16_t func_81F1FC(void);

// Two objects from the table entry at byte offset `entry` ($0D60 x, $0D68 y): slots come
// from take_slot; the second sits 2 * $014C lower. Advances the entry's x by 8.
void func_8AA216(uint16_t entry);

// A 2-tile sprite at byte offset `slot` of the tables $05C0/$06C0/$07C0/$08C0: x from
// $017A (and x + 16), y from $017C, tiles $04A0/$04A2. Returns slot + 4.
uint16_t func_8A92CF(uint16_t slot);

// Offset of a cell in a 32-column map of words: row ($0138) * 64 + column ($0136) * 2.
// The original doubles the column in place ($0136 <<= 1). Returns the offset.
uint16_t tilemap_offset(void);

// If $013C >= 10: set the word at $0D04 + ($013C - 10) to 4 when $015A is odd, else 0.
// (With $013C < 10 the original returns with Y still on the stack, i.e. through a bogus
// return address; no movie reaches that, and this C does nothing then.)
void func_8AB700(void);

// Move the word at `value` one step towards the word at `target`, up to `steps` times
// (the original's Y), stopping early once they are equal. Returns the steps left;
// `last_diff` receives the last target - value the original computed (its A).
uint16_t step_toward(uint16_t value, uint16_t target, uint16_t steps, uint16_t* last_diff);

// Append byte `b` to the buffer at $7F:7D00 (index $0158, then +1) and count it in the
// byte $0160.
void byte_buffer_push(uint8_t b);

// byte_buffer_push of a space ($20).
void byte_buffer_space(void);

// Push spaces until the byte count $0160 (as a word, with $0161) equals `count`.
void byte_buffer_pad(uint16_t count);

// Push the end marker $80 and reset the count word $0160.
void byte_buffer_end(void);

// Push `value` in decimal as ASCII digits, without leading zeros ("0" for zero). Leaves
// $015A = 1 (a digit was pushed; untouched for zero) and $015C = 0 (the remainder).
void byte_buffer_decimal(uint16_t value);

// $01FB = $11 / $13.
void func_80E4C5(void);
void func_80E4DB(void);

// $01CC = x, byte $01CE = a, $01E3 = 0, $01E1 = 9.
void func_80B58D(uint8_t a, uint16_t x);

// Shift the 48-bit number D+$46 : D+$44 : `low` right by 4 (D+$44 and D+$46 keep the
// result) and return its low word shifted right by 12 more (lsr_n; *carry = the last bit
// out): bits 16-19 of the original number.
uint16_t func_8ABFBC(uint16_t low, bool* carry);

// $0142 = the five BCD digits of $0144 (four nibbles, low first) and $0146 (its low
// nibble) as a binary number (weights 1, 10, 100, 1000, 10000 at $8A:CABA), taking the
// digits off with func_8ABFBC (which shifts $0144-$0146 right by 4 each time). Returns
// the A it leaves: the last sum, or 0 if the last digit was 0.
uint16_t bcd_to_binary(uint16_t a);

// If the byte `b` is not zero: store it at $1B30 + x (an 8-byte ring) and return
// (x + 1) & 7; else return x.
uint16_t func_80899F(uint8_t b, uint16_t x);

// Shift the inverse of bit 0 of `a` into the word $0156 (from the right) and add it to
// the word $0158. Returns the new $0158.
uint16_t func_8AA022(uint16_t a);

// Set to $FFFF each of the seven words $01CF-$01DB that equals $01DD.
void func_80B7BB(void);

// byte_buffer_decimal of $015E, after a space if it is below 10 (signed): two columns.
void byte_buffer_decimal2(void);

// Queue the request bytes $01FB, $01FC, $02FB, $02FC (non-zero ones; func_80899F into the
// ring at $1B30, write index $1B24) and clear them; then, if the ring is not empty, take
// one (read index $1B26): $0463 = it | $FF00, $0465 = $FF00, $0467 = 0 (byte).
void func_80894E(void);

// The 8 decimal digits of `value` as BCD, ones in the low nibble, by dividing by 10 on
// the CPU divider; left in D+$16 (low word) and D+$18 (high word). Returns it.
uint32_t binary_to_bcd(uint16_t value);

// $0156 = $0158 = 0, then func_8AA022 of the words $0018, $0014, $0010, $000C in turn.
// Returns the final $0158.
uint16_t func_8AA005(void);

// 2a + $B990 when 2a < $40, else 2a + $2400.
uint16_t func_80E0C8(uint16_t a);

// Hide every sprite: fill the 512-byte table at $0900 with $E0 and reset the slot
// indices $0172 and $0174.
void oam_clear(void);

// oam_clear, by a long call.
void oam_clear_long(void);

// Two sprites at the slot D+$74 (then advanced by 8): tile $3892 at (D+$EE + D+$E7 + $5A,
// $08) and tile $3890 at (D+$E7 + $5C, $0A), both with $0100 in the extra table at $0B01;
// D+$EE counts down to 0 first. Returns the new D+$74.
uint16_t func_808FFC(void);

// For each non-zero word p of the zero-terminated table at `table` (24-bit; the original
// reads it through DB), func_8AB680 of the list at p + 4 in the same bank. D+$4A = the
// table's address.
void func_8AA4BE(uint32_t table);

// $014A = ($0136 >> 2) + $0108.
uint16_t func_8A8232(void);

// Unless bit 15 of $047C is set: $0463 = a, $0465 = x, $0467 = 0 (byte; $0466 is then
// overwritten by x's high byte).
void func_82AD12(uint16_t a, uint16_t x);

// $0142 in BCD into $0144 (four digits, ones lowest) and $0146 (the ten-thousands), by
// repeated subtraction of the weights at $8A:CABA; $0142 ends as 0.
void binary_to_bcd5(void);

// An empty routine (RTS).
void func_82F380(void);
// Another one.
void func_82FB1C(void);

// Shift the 32-bit numbers $0390:$038E and $03BF:$03BD left (func_8AC03D) or right
// (func_8AC024) by 12 bits, one bit at a time. Return the last bit shifted out of the
// second number.
bool func_8AC03D(void);
bool func_8AC024(void);

// Append the words (a, x, y, D+$14) to the four tables $1900, $1940, $19C0, $1980 at
// index D+$8C, then advance it: D+$8C = ((index + 2) & $3F) + D+$8A (32 entries in a
// ring, D+$8A choosing the ring). Returns the new index.
uint16_t func_80B537(uint16_t dp, uint16_t a, uint16_t x, uint16_t y);

// One step of a two-column wipe: through the long pointer at $0170 (+ $0172 bank), set
// the word at columns $0176 and $0174 of 28 rows ($40 bytes apart) to tile $3D, counting
// changed cells in $0178; then the columns move apart ($0176 += 2, $0174 -= 2).
void func_8AB942(void);

// Clear the object tables' first column ($0500-$05F7) and the slot counter $018C.
void clear_object_slots(void);

// $0D90 = $013C * 8 - $1C and $0D78 = $015A * 8 (targets for step_toward_0d68/0d70).
void func_8AA469(void);

// Skip `n` zero-terminated strings starting at bus address `list`, returning the
// address of the next one (also left in $0144). The original steps over each terminator
// with an 8-bit increment of the pointer's low byte, so a terminator at $xxFF wraps
// within the page; that quirk is kept.
uint16_t string_list_nth(uint32_t list, uint16_t n);

// Shift the 32-bit value at $0118 (high word) : $011A (low word) left by 4 and return
// the 4 bits shifted out (a nibble reader).
uint16_t take_nibble(void);

// A request ($0463 = a, $0465 = x, $0467 = 0) that is refused while the word at $047C has
// bit 15 set (returns false, nothing stored) or while a cooldown runs: with the direct
// page `dp`'s word $23 non-zero, it is reduced by $1E9E; if still >= 0 the request is
// refused (returns true), else the cooldown restarts from y and the request is stored.
// Returns true when refused (the original's carry).
bool func_83FE9A(uint16_t dp, uint16_t a, uint16_t x, uint16_t y);

// Draw the sprite frame of the table entry at byte offset `entry`: first its width
// ($0D70), y ($0D68) and x offset ($0D80) each step towards their targets ($0D78, $0D90,
// $0D88) by up to $017E, $017C and $017A steps. The frame is three rows $014C apart,
// centred on x ($0D60) + the offset: a left column (tiles $3000/$3001/$B000), one
// func_8AA216 column per 8 pixels of width, and a right column (the left one flipped,
// $7000/$7001/$F000). $0148 = width / 2; x, y and width are left as they were.
void sprite_frame(uint16_t entry);

// Show or hide seven objects by the mask word for $0158 (table $8A:9FF9): object i
// (offsets at $8A:9FE9) gets bit 7 of its $1516 flags cleared when mask bit i is set,
// set otherwise. $0158 is 0-5 in the game.
void func_8A9FA1(void);

// Fill the 32x64 map of words at $7F:0000 with tile $003D (a blank).
void clear_tilemap_7f(void);

// $0192 += $0DCA.
void func_8ABC32(void);

// Clear the 16 words $0DAC-$0DCB.
void func_8ABC7F(void);

// For entry `x`: the byte at $7E:8CB3 + x as the top 8 bits of a signed 12-bit value
// (sign-extended, times 16) minus the byte at $7E:8CB5 + x; that byte, doubled, is left
// in $011E. Returns the difference.
uint16_t func_82EC09(uint16_t x);

// Wipe the map at $7F:0000 with tile $3D from both edges inwards: 16 steps of
// func_8AB942, waiting a frame after each step that changed a cell. Returns the frame
// the last wait ended on, or 0 when the last step changed nothing (the original's A).
uint16_t wipe_columns(void);

// $01FB = $11 (or $13 for func_8AA49E) and $0D7E = $0A8C.
void func_8AA48D(void);
void func_8AA49E(void);

// Set the first word of each of the 128 entries of $0900 (4 bytes each) to $E000.
void clear_sprites(void);

// Load entry `i` (a byte offset) of six parallel word tables in bank $86 ($DAF5, $DB47,
// $DB99, $DBEB, $DC3D, $DC8F; 41 entries each) into $1EA0, $1EA2, $1EA4, $1EA6, $1EAA and
// $1EAC. Returns the $1EA0 value.
uint16_t func_83EA9B(uint16_t i);

// The 16 words $0DAC-$0DCB are a 256-bit number, $0DAC the most significant word.
// func_8ABB5C shifts it right `count` times (1-65536; 0 means 65536): the first bit in at
// the top is *carry, the next ones are taken from the top of $0146 (which fills with 1s);
// returns the bits shifted out at the bottom (last one in bit 0) and leaves the last
// $0146 bit taken in *carry.
uint16_t func_8ABB5C(uint16_t count, bool* carry);

// Shift the 256-bit number left `count` times (0 means 65536), feeding in the bits of
// `bits` from bit 0 up; after each shift $0184 += 1 and func_8ABC32 ($0192 += $0DCA).
void func_8ABBF4(uint16_t bits, uint16_t count);

// The 16-bit sibling of take_nibble: A = the top 4 bits of $011A, which is shifted left 4.
uint16_t take_nibble16(void);

// Blocks of 2 rows x 5 words in a map at $7E:230C (rows $50 bytes apart).
// func_82B78D: from the entry at $7E:4F31 + ((A >> 2) & ~3) (two bytes), $0156 = byte 0
// - 2 and $0158 = byte 1 - 2 (both 8-bit). Returns $0158.
uint16_t func_82B78D(uint16_t a);
// func_82B7B0: AND the block at row $0158, column $0156 (offset 2 * $0158 + the word at
// $83:8C65 + ($0156 & ~1)) with one of 8 ten-word masks in bank $83 (pointer table at
// $83:8C55, chosen by $0156 & 7).
void func_82B7B0(void);
// func_82B838: copy the block at byte offset `at` from $7E:217C to $7E:230C. Returns the
// last word copied.
uint16_t func_82B838(uint16_t at);

// Write a small HDMA-style table at bus address `table` (DB:X in the original; bank $7F in
// the game): 3-byte entries (line count, word) — $31 lines of $00FF; if A's high byte H
// is not 0, H lines of (L | L << 8) with L = A's low byte + $69; 1 line of (L << 8 | $69);
// $4D - H lines of $00FF; then the byte $B4. Also leaves A in $0114, L in $0116 and H in
// $0118. Returns the address of the final byte.
uint32_t func_80DC5A(uint32_t table, uint16_t a);

// Take a free slot (word 0 in $7F:7FBC + slot, slots 0..$0166 step 2) whose kind
// ($041C + slot) is $0168; if there is none, lower $0168 and look again, down to kind 0.
// The slot taken is marked $FFFF and stored in *slot. Returns false when even kind 0 has
// no free slot (*slot = $0166 then).
bool func_8A9BB5(uint16_t* slot);

// $0130 = 0, the 128 words at $0500 cleared, then clear_sprites.
void func_8A8EEF(void);

// $0154 += $10, $0156 -= 1.
void func_8A9BAA(void);

// $0168 = the largest of the 20 slot kinds at $041C (largest by the sign of the
// difference, as the original's BMI does), starting from 0. Returns it.
uint16_t func_8A9BE2(void);

// For each 8-byte record of the list at bus address `list` (the original's A + 4, bank DB)
// until a zero first word: the word at $1516 + (word 4) = word 6. $0160 follows the
// record pointer.
void func_8AB680(uint32_t list);

// Overlap test for the entry at $011A of the $7E:8CB3 table (D = `dp`, $0200 in the
// game): its 12-bit signed value (the byte at $8CB3, times 16) goes to $0128 and, minus
// D+$06, must lie within the kind's limits ($83:80CC below, $83:80CE above; kind offset
// $011C); then -(high nibble of the byte at $8CB5) * 8 minus $012C must lie within
// $83:826C/$83:826E. "Within" is judged by the sign of each difference, as the original's
// BPL/BMI do. Returns true when both hold (the original's carry).
bool func_83FC05(uint16_t dp);

// The object hit test with the position in `a` (D = `dp`, $0200 in the game): unless $0120
// has bit 15 set, a - D+$06 and $012A - $012C must lie within the kind's limits (as in
// func_83FC05). On a hit: clears bit 3 of D+$7C, requests effect $FF23 for $1EBE through
// func_83FE9A, adds (D+$0F / 2) * $1F30 / 32768 to D+$5A (DSP-4 multiply; saturating;
// skipped when $1F31 has bit 15 set), sets D+$1A to 2 (behind, $0128 < 0) or 3 if it was 0
// and D+$10 >= $60, D+$1C to $1000 or $F000, and halves the signed 32-bit D+$18:D+$16.
// Returns whether it hit.
bool func_83FD8A(uint16_t dp, uint16_t a);

// $83:FDCD: always "no hit" (its test was disabled: a BRA skips it).
bool func_83FDCD(void);

// The word in bank $85 for A: row (A >> 6) & $FF times 10 (DSP-4 multiply) plus $0198 (also
// left in $0118) picks an entry of a table at $84:0002; that entry plus (A & $3E) (and the
// first add's carry) is the offset in bank $85.
uint16_t func_82EA49(uint16_t a);

// Two linear interpolations at the fraction $FAB5,x / 256 ($0114): between the words at
// $34EC,y and $34EE,y (minus $FABB,x, into $0116) and between $36EC,y and $36EE,y (minus
// $0C, into $0118; returned). Bank $7E; each uses a DSP-4 multiply's high word.
uint16_t func_82AB3F(uint16_t x, uint16_t y);

// Targets for $028B/$028D: unless $0281 is negative (then $0118 = -$30, $0116 = 0),
// func_82AB3F at ($0281, $0283), then $0116 = -(2 * $0116 * $0293 >> 16) and $0118 =
// (2 * $0118 * $0293 >> 16) - $33 (DSP-4 multiplies). $028B and $028D then move towards
// $0116 and $0118 by at most $1E9E * 4 (judged by sign, as the original). Returns $028D.
uint16_t func_82A9B7(void);

// $7E:4E2D = (bits 8-23 of $0213 * $300, a DSP-4 multiply) - $B00, at most $3000 (by
// sign). If $0244 bit 0: $7E:4E35 = ($B0 - 2 * the high word of the 32-bit distance
// $7E:FAB4 - $0224, both at least 0) & $F8. If also bit 2: $7E:4E2D = ((its value + $B00,
// at least 0) / (high word of $FAB6 - $0226, at least 0, + $10, at most $D0)) * 16 - $B00,
// at most $3000 (the division on the CPU's divider). $0114 is left as scratch.
void func_82AC52(void);

#endif
