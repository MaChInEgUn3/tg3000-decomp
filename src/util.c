// SPDX-License-Identifier: GPL-3.0-or-later
#include "util.h"

#include "tg3000.h"

// TG3000_ROUTINE($8A:B15E) lsr_n
uint16_t lsr_n(uint16_t value, uint16_t count, bool* carry) {
  bool out = true;
  for(; count != 0; count--) {
    out = value & 1;
    value >>= 1;
  }
  *carry = out;
  return value;
}

// TG3000_ROUTINE($8A:A25D) take_slot
//
// The original's counter is at D+$8C; its callers then fill per-object tables at
// $0500/$0600/$0800 with the returned index.
uint16_t take_slot(uint16_t counter) {
  uint16_t slot = rd16(counter);
  wr16(counter, (uint16_t)(slot + 2));
  return slot;
}
