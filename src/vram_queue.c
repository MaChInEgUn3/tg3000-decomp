// SPDX-License-Identifier: GPL-3.0-or-later
#include "vram_queue.h"

#include "platform.h"
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

// $80:88B9, inside nmi_prologue's row. A transfer of size 0, or a split when the budget is
// already 0, sends 65536 bytes (the DMA size register's 0), as the original does.
void vram_queue_drain(void) {
  uint16_t x = rd16(VRAM_QUEUE_TAIL);
  while(x != rd16(VRAM_QUEUE_HEAD)) {
    uint16_t dest = rd16(VRAM_QUEUE_DEST + x), src = rd16(VRAM_QUEUE_SRC + x), size = rd16(VRAM_QUEUE_SIZE + x);
    uint16_t budget = rd16(VRAM_QUEUE_BUDGET);
    uint32_t bank = (uint32_t)rd8(VRAM_QUEUE_BANK + x) << 16;
    if(size > budget) {  // only part of it fits
      wr16(VRAM_QUEUE_SIZE + x, (uint16_t)(size - budget));
      wr16(VRAM_QUEUE_SRC + x, (uint16_t)(src + budget));
      wr16(VRAM_QUEUE_DEST + x, (uint16_t)(dest + (budget >> 1)));
      video_upload_vram(dest, bank | src, budget ? budget : 0x10000);
      wr16(VRAM_QUEUE_BUDGET, 0);
      return;
    }
    video_upload_vram(dest, bank | src, size ? size : 0x10000);
    wr16(VRAM_QUEUE_BUDGET, (uint16_t)(budget - size));
    x = (uint16_t)((x + 2) & 0x3F);
    wr16(VRAM_QUEUE_TAIL, x);
  }
}
