// SPDX-License-Identifier: GPL-3.0-or-later
#include "pads.h"

#include "platform.h"
#include "tg3000.h"

static void pad_update(uint16_t at, uint16_t now) {
  uint16_t old = rd16(at);
  wr16(at, now);
  wr16((uint16_t)(at + 2), (uint16_t)((rd16((uint16_t)(at + 2)) | (uint16_t)(old ^ now)) & now));
}

// TG3000_ROUTINE($83:87C1) pads_read_pair
//
// The 17th read's lines, inverted, go into bit 0 of each pad: a pad that is not there
// leaves a signature bit set and reads as absent.
void pads_read_pair(int pair) {
  uint16_t d0, d1;
  uint8_t tail;
  input_multitap_pair(pair, &d0, &d1, &tail);
  wr16(PAD_SCRATCH, (uint16_t)(d0 | ((tail & 1) ^ 1)));
  wr16(PAD_SCRATCH + 2, (uint16_t)(d1 | (((tail >> 1) & 1) ^ 1)));
}

// TG3000_ROUTINE($83:86FF) pads_read
//
// The original first waits for the console's automatic read to finish ($4212 bit 0) and
// strobes the latch ($4016) between the reads; the platform's input calls stand for both.
// The test for a non-standard multitap pad A ($83:878F) runs in no recorded movie.
void pads_read(void) {
  pad_update(PAD_BASE, input_pad1());
  if(!input_port1_data()) wr8(PAD_BASE, (uint8_t)(rd8(PAD_BASE) | 1));
  for(int pair = 0; pair < 2; pair++) {
    pads_read_pair(pair);
    pad_update((uint16_t)(PAD_BASE + 4 + 8 * pair), rd16(PAD_SCRATCH));
    pad_update((uint16_t)(PAD_BASE + 8 + 8 * pair), rd16(PAD_SCRATCH + 2));
  }
  for(uint16_t at = PAD_BASE; at <= PAD_BASE + 0x10; at = (uint16_t)(at + 4)) {
    if(rd16(at) & 0x000F) {
      wr16(at, 1);
      wr16((uint16_t)(at + 2), 1);
    }
  }
}

// TG3000_ROUTINE($83:86FB) pads_read_long (long)
void pads_read_long(void) { pads_read(); }

// TG3000_ROUTINE($83:96CD) pads_read_long_2 (long): a second copy of pads_read_long
void pads_read_long_2(void) { pads_read(); }
