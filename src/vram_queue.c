// SPDX-License-Identifier: GPL-3.0-or-later
#include "vram_queue.h"

#include "tg3000.h"

// TG3000_ROUTINE($80:83FC) vram_queue_push
//
// The original takes size in A, dest in Y, src in X and the bank word from the direct
// page ($14). It does not check for a full queue: a 33rd transfer overwrites the oldest.
uint16_t vram_queue_push(uint16_t size, uint16_t dest, uint16_t src, uint16_t src_bank) {
  uint16_t slot = rd16(VRAM_QUEUE_HEAD);
  wr16(VRAM_QUEUE_SIZE + slot, size);
  wr16(VRAM_QUEUE_DEST + slot, dest);
  wr16(VRAM_QUEUE_SRC + slot, src);
  wr16(VRAM_QUEUE_BANK + slot, src_bank);
  uint16_t head = (uint16_t)((slot + 2) & 0x3F);
  wr16(VRAM_QUEUE_HEAD, head);
  return head;
}
