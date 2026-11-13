// SPDX-License-Identifier: GPL-3.0-or-later
#include "rng.h"

#include <stddef.h>

#include "tg3000.h"

// TG3000_ROUTINE($80:87E9) rng_next
// TG3000_ROUTINE($8A:988C) rng_next (identical copy in bank $8A)
//
// A Galois-style LFSR, shifted left: if the top bit was clear, the shifted state is
// XORed with $1D872B45; if it was set, the state is only shifted. (The usual form XORs
// when the outgoing bit is set; this game does the opposite.)
uint16_t rng_next(uint16_t state, bool* carry) {
  uint32_t s = rd32(state);
  bool top = (s >> 31) & 1;
  s <<= 1;
  if(!top) s ^= 0x1D872B45;
  wr32(state, s);
  if(carry != NULL) *carry = top;
  return (uint16_t)s;
}
