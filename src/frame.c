// SPDX-License-Identifier: GPL-3.0-or-later
#include "frame.h"

#include "platform.h"
#include "tg3000.h"

// TG3000_ROUTINE($80:8373) wait_frames
// TG3000_ROUTINE($8A:BA90) wait_frames (long-call copy in bank $8A)
uint16_t wait_frames(uint16_t n) {
  uint16_t target = (uint16_t)(rd16(FRAME_COUNTER) + n);
  while(rd16(FRAME_COUNTER) != target) platform_wait_vblank();
  return target;
}
