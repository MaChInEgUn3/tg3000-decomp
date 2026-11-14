// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef TG3000_FRAME_H
#define TG3000_FRAME_H

#include <stdint.h>

// Incremented once per frame by the NMI handler.
#define FRAME_COUNTER 0x0024

// Wait until the frame counter has advanced by `n` (n = 0 waits a full 65,536 frames,
// as the original does). Returns the counter value it waited for.
uint16_t wait_frames(uint16_t n);

#endif
