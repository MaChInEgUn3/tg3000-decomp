// SPDX-License-Identifier: GPL-3.0-or-later
#include "frame.h"

#include "misc.h"
#include "pads.h"
#include "platform.h"
#include "sound.h"
#include "tg3000.h"

// TG3000_ROUTINE($80:8373) wait_frames
// TG3000_ROUTINE($8A:BA90) wait_frames (long-call copy in bank $8A)
// TG3000_ROUTINE($82:841E) wait_frames (copy in bank $82)
uint16_t wait_frames(uint16_t n) {
  uint16_t target = (uint16_t)(rd16(FRAME_COUNTER) + n);
  while(rd16(FRAME_COUNTER) != target) platform_wait_vblank();
  return target;
}

// TG3000_ROUTINE($80:B950) wait_flag_76
uint16_t wait_flag_76(uint16_t dp) {
  wr16(dp + 0x76, (uint16_t)(rd16(dp + 0x76) | 0x0010));
  while(!(rd16(dp + 0x76) & 0x0008)) platform_wait_vblank();
  return rd16(dp + 0x76);
}

// TG3000_ROUTINE($8A:B212) wait_one_frame
void wait_one_frame(void) { wait_frames(1); }

// TG3000_ROUTINE($85:95BD) irq_ack_only
//
// The original reads $4210 (BIT) to acknowledge; the platform acknowledges for itself.
void irq_ack_only(void) {}

// TG3000_ROUTINE($80:8934) nmi_tail
//
// The original then restores the registers its NMI prologue saved and returns (RTI).
void nmi_tail(void) {
  if(!(rd16(0x0026) & 0x0004)) pads_read_long();
  func_80894E();
  sound_frame();
  wr16(FRAME_COUNTER, (uint16_t)(rd16(FRAME_COUNTER) + 1));
}
