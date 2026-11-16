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

void video_set_bg3_hofs(uint16_t value) { difftest_ppu_field(PF_BG3H, value); }

// Byte registers: recorded as the bytes the original writes.
void video_set_main_screen(uint8_t layers) { difftest_reg(0x212C, layers); }
void video_set_sub_screen(uint8_t layers) { difftest_reg(0x212D, layers); }
void video_set_color_math(uint8_t value) { difftest_reg(0x2131, value); }

void video_set_color_math_control(uint8_t value) { difftest_reg(0x2130, value); }
void video_set_window_bg12(uint8_t value) { difftest_reg(0x2123, value); }
void video_set_window_bg34(uint8_t value) { difftest_reg(0x2124, value); }
void video_set_window_obj(uint8_t value) { difftest_reg(0x2125, value); }
void platform_set_interrupts(uint8_t value) { difftest_reg(0x4200, value); }

void video_set_fixed_color(uint8_t value) {
  if(value & 0x20) difftest_ppu_field(PF_FIXR, value & 0x1F);
  if(value & 0x40) difftest_ppu_field(PF_FIXG, value & 0x1F);
  if(value & 0x80) difftest_ppu_field(PF_FIXB, value & 0x1F);
}

void video_set_color(uint8_t index, uint16_t bgr) { difftest_effect(EFFECT_CGRAM_WORD, index, bgr, 1); }

void platform_set_irq_v(uint16_t line) {
  difftest_reg(0x4209, (uint8_t)line);
  difftest_reg(0x420A, (uint8_t)(line >> 8));
}

void platform_set_irq_h(uint16_t dot) {
  difftest_reg(0x4207, (uint8_t)dot);
  difftest_reg(0x4208, (uint8_t)(dot >> 8));
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

// Controllers: from the original's reads of the joypad registers, in order.
uint16_t input_pad1(void) { return (uint16_t)(difftest_input(0x4218) | difftest_input(0x4219) << 8); }
bool input_port1_data(void) { return difftest_input(0x4016) & 1; }

void input_multitap_pair(int pair, uint16_t* d0, uint16_t* d1, uint8_t* tail) {
  (void)pair;  // the original's reads already carry which pair it was
  uint16_t a = 0, b = 0;
  for(int i = 0; i < 16; i++) {
    uint8_t v = difftest_input(0x4017);
    a = (uint16_t)(a << 1 | (v & 1));
    b = (uint16_t)(b << 1 | ((v >> 1) & 1));
  }
  *d0 = a;
  *d1 = b;
  *tail = difftest_input(0x4017) & 3;
}

// The bench replays, in order, the WRAM changes each interrupt made while the original
// routine was waiting (recorded by the differential test).
void platform_wait_vblank(void) { difftest_replay_interrupt(); }
