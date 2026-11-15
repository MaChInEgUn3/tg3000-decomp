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

void video_upload_cgram(uint8_t index, uint32_t src, uint32_t size) {
  difftest_effect(EFFECT_CGRAM, index, src, size);
}

// PPU registers: recorded as the fields the bench's PPU decodes from the same writes.
void video_set_m7a(int16_t value) { difftest_ppu_field(PF_M7A, (uint16_t)value); }
void video_set_m7d(int16_t value) { difftest_ppu_field(PF_M7D, (uint16_t)value); }

void video_set_bg1_hofs(uint16_t value) {
  difftest_ppu_field(PF_BG1H, value);
  difftest_ppu_field(PF_M7H, value);
}

void video_set_bg1_vofs(uint16_t value) {
  difftest_ppu_field(PF_BG1V, value);
  difftest_ppu_field(PF_M7V, value);
}

// The bench replays, in order, the WRAM changes each interrupt made while the original
// routine was waiting (recorded by the differential test).
void platform_wait_vblank(void) { difftest_replay_interrupt(); }
