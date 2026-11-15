// SPDX-License-Identifier: GPL-3.0-or-later
// The bench's implementation of src/platform.h: it does not draw anything, it records each
// call as an effect for the differential test to compare with the original's DMA.
#include "platform.h"

#include "difftest.h"

void video_upload_vram(uint16_t vram_word, uint32_t src, uint32_t size) {
  difftest_effect(EFFECT_VRAM, vram_word, src, size);
}

void video_fill_vram(uint16_t vram_word, uint8_t value, uint32_t size) {
  difftest_effect(EFFECT_VRAM_FILL, vram_word, value, size);
}

void video_write_vram_word(uint16_t vram_word, uint16_t value) {
  difftest_effect(EFFECT_VRAM_WORD, vram_word, value, 1);
}

void video_upload_cgram(uint8_t index, uint32_t src, uint32_t size) {
  difftest_effect(EFFECT_CGRAM, index, src, size);
}

// PPU registers: recorded as the fields the bench's PPU decodes from the same writes.
void video_set_m7a(int16_t value) { difftest_ppu_field(PF_M7A, (uint16_t)value); }
void video_set_m7b(int16_t value) { difftest_ppu_field(PF_M7B, (uint16_t)value); }
void video_set_m7c(int16_t value) { difftest_ppu_field(PF_M7C, (uint16_t)value); }
void video_set_m7d(int16_t value) { difftest_ppu_field(PF_M7D, (uint16_t)value); }
void video_set_m7x(int16_t value) { difftest_ppu_field(PF_M7X, (uint16_t)value); }
void video_set_m7y(int16_t value) { difftest_ppu_field(PF_M7Y, (uint16_t)value); }
void video_set_bg2_hofs(uint16_t value) { difftest_ppu_field(PF_BG2H, value); }
void video_set_bg2_vofs(uint16_t value) { difftest_ppu_field(PF_BG2V, value); }

void video_set_bg1_hofs(uint16_t value) {
  difftest_ppu_field(PF_BG1H, value);
  difftest_ppu_field(PF_M7H, value);
}

void video_set_bg1_vofs(uint16_t value) {
  difftest_ppu_field(PF_BG1V, value);
  difftest_ppu_field(PF_M7V, value);
}

// Sound: the message as the original's sixteen port writes, in its order; the handshake
// it waits on is the platform's.
void sound_send_frame(const uint8_t msg[16]) {
  static const uint8_t port[4] = {1, 2, 3, 0};
  for(int i = 0; i < 16; i++) difftest_effect(EFFECT_APU, port[i & 3], msg[i], 1);
}

// Inputs come from what the original read (difftest_input replays it in order).
bool sound_is_ready(void) { return difftest_input(0x2141) == 0x77; }
bool video_is_pal(void) { return (difftest_input(0x213F) & 0x10) != 0; }

// The bench replays, in order, the WRAM changes each interrupt made while the original
// routine was waiting (recorded by the differential test).
void platform_wait_vblank(void) { difftest_replay_interrupt(); }
