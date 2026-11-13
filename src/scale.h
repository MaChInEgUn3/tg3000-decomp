// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef TG3000_SCALE_H
#define TG3000_SCALE_H

#include <stdint.h>

// The routine works on direct-page variables; `dp` is the direct page it runs with.
#define SCALE_DIVISOR 0x7A   // input (word)
#define SCALE_QUOTIENT 0x14  // scratch: the 8-bit quotient (word)
#define SCALE_RESULT 0x38    // output (word)

// result = 42 * quotient, where quotient = 8 bits of 65536 / divisor (restoring division
// that starts from $200 and keeps the 16-bit overflow of the original). A divisor of
// exactly $100 short-cuts to $2A00. Returns the result.
uint16_t inverse_scale(uint16_t dp);

#endif
