// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef TG3000_VRAM_QUEUE_H
#define TG3000_VRAM_QUEUE_H

#include <stdint.h>

// The VRAM transfer queue: a ring of 32 transfers kept in four parallel tables of words,
// indexed by a byte offset (even, $00-$3E). The NMI handler ($80:88B9) drains it with DMA
// channel 0 into VRAM, splitting transfers to stay within a per-frame byte budget.
#define VRAM_QUEUE_SIZE 0x1900  // bytes to transfer
#define VRAM_QUEUE_SRC 0x1940   // source address, low 16 bits
#define VRAM_QUEUE_BANK 0x1980  // source bank in the low byte (the high byte is stored too)
#define VRAM_QUEUE_DEST 0x19C0  // VRAM word address
#define VRAM_QUEUE_HEAD 0x0028  // where the next transfer goes
#define VRAM_QUEUE_TAIL 0x002A  // next transfer the NMI handler sends

// Queue a transfer of `size` bytes from `src_bank`:`src` to VRAM word address `dest`.
// Returns the new head.
uint16_t vram_queue_push(uint16_t size, uint16_t dest, uint16_t src, uint16_t src_bank);

#endif
