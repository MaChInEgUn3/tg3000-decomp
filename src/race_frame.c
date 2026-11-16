// SPDX-License-Identifier: GPL-3.0-or-later
#include "race_frame.h"

#include "platform.h"
#include "tg3000.h"

// TG3000_ROUTINE($83:8644) race_upload_queue_drain
//
// Sends queued transfers while the frame's DMA budget lasts. Each whole transfer costs its
// size plus 100. A transfer larger than what is left sends that much (the budget), the
// entry keeps the rest for the next frame (its size shrinks, source and VRAM address
// advance), and the budget becomes 0. Returns whether it wrote a flag word (the original
// then leaves its data bank at $83; the C has no data bank).
bool race_upload_queue_drain(void) {
  bool flagged = false;
  int16_t budget = (int16_t)rd16(RACE_DMA_BUDGET);
  uint16_t x = rd16(RACE_QUEUE_TAIL);
  if(budget <= 0 || x == rd16(RACE_QUEUE_HEAD)) return flagged;
  do {
    if(!(rd16(RACE_QUEUE_BANK + x) & 0x8000)) {
      budget = (int16_t)rd16(RACE_DMA_BUDGET);
      if(budget <= 0) break;
      uint16_t size = rd16(RACE_QUEUE_SIZE + x);
      if((int16_t)(budget - size) < 0) {  // only part of it fits
        uint16_t part = (uint16_t)budget;
        uint16_t src = rd16(RACE_QUEUE_SRC + x), dest = rd16(RACE_QUEUE_DEST + x);
        wr16(0x003C, (uint16_t)(part >> 1));  // the words it sends (scratch)
        wr16(RACE_QUEUE_SIZE + x, (uint16_t)(size - part));
        wr16(RACE_QUEUE_SRC + x, (uint16_t)(src + part));
        wr16(RACE_QUEUE_DEST + x, (uint16_t)(dest + (part >> 1)));
        video_upload_vram(dest, (uint32_t)rd8(RACE_QUEUE_BANK + x) << 16 | src, part ? part : 0x10000);
        wr16(RACE_DMA_BUDGET, 0);
        wr16(RACE_QUEUE_TAIL, x);
        return flagged;
      }
      wr16(RACE_DMA_BUDGET, (uint16_t)(budget - size - 100));
      video_upload_vram(rd16(RACE_QUEUE_DEST + x), (uint32_t)rd8(RACE_QUEUE_BANK + x) << 16 | rd16(RACE_QUEUE_SRC + x),
                        size ? size : 0x10000);
      uint16_t flag = rd16(RACE_QUEUE_FLAG_ADR + x);
      if(flag) {
        wr16(flag, rd16(RACE_QUEUE_FLAG_VAL + x));  // DB = $7E
        flagged = true;
      }
    }
    x = (uint16_t)((x + 2) & 0xFF);
  } while(x != rd16(RACE_QUEUE_HEAD));
  wr16(RACE_QUEUE_TAIL, x);
  return flagged;
}

// TG3000_ROUTINE($83:96D1) race_upload_queue_drain_long (long)
bool race_upload_queue_drain_long(void) { return race_upload_queue_drain(); }

// TG3000_ROUTINE($83:8989) func_838989
//
// Unless the game is paused ($0026 bit 4), adds the signed word $01CC to the 24-bit value
// at $00D4. Then writes its upper word + $3C to the two words at $1536 and $1556 (indexed
// by $00E2); when $1F32 is negative, the first gets that word less an eighth instead.
void func_838989(void) {
  uint16_t x = rd16(0x00E2);
  if(!(rd16(0x0026) & 0x0010)) {
    uint16_t step = rd16(0x01CC);
    uint32_t sum = (uint32_t)rd16(0x00D4) + step;
    wr16(0x00D4, (uint16_t)sum);
    wr8(0x00D6, (uint8_t)(rd8(0x00D6) + ((step & 0x8000) ? 0xFF : 0x00) + (sum >> 16)));
  }
  uint16_t pos = rd16(0x00D5);
  wr16((uint16_t)(0x1536 + x), (uint16_t)(pos + 0x3C));
  wr16((uint16_t)(0x1556 + x), (uint16_t)(pos + 0x3C));
  if(rd16(0x1F32) & 0x8000) wr16((uint16_t)(0x1536 + x), (uint16_t)(pos - (pos >> 3)));
}

// TG3000_ROUTINE($83:96D5) func_838989_long (long)
void func_838989_long(void) { func_838989(); }

// TG3000_ROUTINE($85:9147) race_cycle_blocks
//
// Five groups of 64 bytes at $7E:2000-$7E:213F, each eight blocks of 8 bytes (four
// words). Every block becomes four copies of the next block's first word; the last block
// gets the first block's. The race NMI runs it every fourth frame.
void race_cycle_blocks(void) {
  for(uint32_t group = 0x2000; group < 0x2140; group += 0x40) {
    uint16_t first = rd16(group);
    for(uint32_t k = 0; k < 8; k++) {
      uint16_t next = k < 7 ? rd16(group + 8 * (k + 1)) : first;
      for(uint32_t w = 0; w < 8; w += 2) wr16(group + 8 * k + w, next);
    }
  }
}
