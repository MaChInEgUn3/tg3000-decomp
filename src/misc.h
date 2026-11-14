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

#endif
