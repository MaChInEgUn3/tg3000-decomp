// SPDX-License-Identifier: GPL-3.0-or-later
// Routines whose purpose is not known yet are named func_<address> until it is.
#ifndef TG3000_MISC_H
#define TG3000_MISC_H

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

#endif
