// SPDX-License-Identifier: GPL-3.0-or-later
// Small helpers the original keeps as subroutines.
#ifndef TG3000_UTIL_H
#define TG3000_UTIL_H

#include <stdbool.h>
#include <stdint.h>

// Shift `value` right by `count` bits, one at a time as the original does. `carry`
// receives the last bit shifted out; with `count` = 0 the original leaves the carry set
// (from its CPY #0), and so does this.
uint16_t lsr_n(uint16_t value, uint16_t count, bool* carry);

// Return the 16-bit index stored at `counter` and advance it by 2 (one word slot).
uint16_t take_slot(uint16_t counter);

#endif
