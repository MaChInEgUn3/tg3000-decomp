// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef TG3000_FRAME_H
#define TG3000_FRAME_H

#include <stdint.h>

// Incremented once per frame by the NMI handler.
#define FRAME_COUNTER 0x0024

// Wait until the frame counter has advanced by `n` (n = 0 waits a full 65,536 frames,
// as the original does). Returns the counter value it waited for.
uint16_t wait_frames(uint16_t n);

// Set bit 4 of D+$76 and wait until an interrupt sets its bit 3. Returns D+$76.
uint16_t wait_flag_76(uint16_t dp);

// wait_frames(1), the caller's A kept.
void wait_one_frame(void);

// Interrupt handlers (docs/DECISIONS.md, "HDMA, measured"): called by the platform where the
// console takes the interrupt.

// IRQ $85:95BD: acknowledges the interrupt and returns; nothing else.
void irq_ack_only(void);

// $80:8934, the NMI's common tail (most of its dispatch targets end here): read the pads
// unless bit 2 of $0026 is set, step the sound request queue (func_80894E), send the
// sound message (sound_frame), count the frame (FRAME_COUNTER).
void nmi_tail(void);

#endif
