// SPDX-License-Identifier: GPL-3.0-or-later
#include "sound.h"

#include "platform.h"
#include "tg3000.h"

// TG3000_ROUTINE($86:DFAE) sound_fade_step
uint8_t sound_fade_step(void) {
  uint16_t fade = rd16(SOUND_FADE);
  uint8_t v = (uint8_t)((uint8_t)(fade << 1) + rd8(SOUND_VOLUME));
  if((v & 0x80) != 0) {  // past the end: up to $7F, down to 0
    wr8(SOUND_FADE, 0);
    wr8(SOUND_FADE + 1, 0);
    v = (fade & 0x8000) ? 0x00 : 0x7F;
  }
  wr8(SOUND_VOLUME, v);
  return v;
}

// TG3000_ROUTINE($86:DFFB) sound_send_message
void sound_send_message(void) {
  uint8_t msg[16] = {
    rd8(0x045C), rd8(0x045D), rd8(0x045F), (uint8_t)((video_is_pal() ? 0x02 : 0) | rd8(SOUND_FLAGS) | 0x80),
    rd8(0x0461), rd8(0x045E), rd8(0x0460), (uint8_t)(rd8(0x0462) & 0x7F),
    rd8(0x0464), rd8(0x0467), rd8(0x0466), (uint8_t)(rd8(SOUND_REQUEST_A) | 0x80),
    rd8(0x0469), rd8(0x046C), rd8(0x046B), (uint8_t)(rd8(SOUND_REQUEST_B) & 0x7F),
  };
  sound_send_frame(msg);
  wr8(SOUND_REQUEST_A, 0);
  wr8(SOUND_REQUEST_B, 0);
  wr8(0x045F, 0xFF);
}

// TG3000_ROUTINE($86:DFE1) sound_frame (long)
bool sound_frame(void) {
  sound_fade_step();
  if(!(rd16(SOUND_ENABLE) & 0x8000) || !sound_is_ready()) return false;
  sound_send_message();
  return true;
}
