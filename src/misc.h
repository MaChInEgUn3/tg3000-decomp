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

#endif
