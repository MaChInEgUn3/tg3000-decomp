// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef TG3000_RNG_H
#define TG3000_RNG_H

#include <stdbool.h>
#include <stdint.h>

// The 32-bit state sits in the direct page at D+$10. Every call seen so far ran with
// D = $0100 (state at $0110-$0113), including the long-call wrapper at $80:87DF, which
// sets D = $0100 itself.
#define RNG_STATE 0x0110

// Advance the generator whose state is at `state` and return its new low word.
// `carry`, if not NULL, receives the bit shifted out (the old bit 31), which the original
// leaves in the carry flag. The bench measured it: unused after a direct call, but read by
// callers of the long entry.
uint16_t rng_next(uint16_t state, bool* carry);

// The long-call entry other banks use: rng_next on the state at RNG_STATE.
uint16_t rng_next_long(bool* carry);

#endif
