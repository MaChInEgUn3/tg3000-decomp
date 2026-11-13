// SPDX-License-Identifier: GPL-3.0-or-later
#include "scale.h"

#include "tg3000.h"

// TG3000_ROUTINE($80:F709) inverse_scale
//
// For divisors above $100 the quotient is floor(65536 / divisor); below that it no longer
// fits in 8 bits and the original's truncation is kept as is. The multiply by 42 ($2A) is
// done on the CPU's hardware multiplier in the original.
uint16_t inverse_scale(uint16_t dp) {
  uint16_t divisor = rd16(dp + SCALE_DIVISOR);
  wr16(dp + SCALE_QUOTIENT, 0x0100);
  if(divisor == 0x0100) {
    wr16(dp + SCALE_RESULT, 0x2A00);
    return 0x2A00;
  }
  uint16_t remainder = 0x0200, quotient = 0;
  for(int bit = 0; bit < 8; bit++) {
    int take = remainder >= divisor;
    if(take) remainder = (uint16_t)(remainder - divisor);
    quotient = (uint16_t)((quotient << 1) | take);
    remainder = (uint16_t)(remainder << 1);
  }
  wr16(dp + SCALE_QUOTIENT, quotient);
  uint16_t result = (uint16_t)((quotient & 0xFF) * 0x2A);
  wr16(dp + SCALE_RESULT, result);
  return result;
}
