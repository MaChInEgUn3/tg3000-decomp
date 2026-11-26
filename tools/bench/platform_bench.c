// SPDX-License-Identifier: GPL-3.0-or-later
// The bench's implementation of src/platform.h: it does not draw anything, it records each
// call as an effect for the differential test to compare with the original's DMA.
#include "platform.h"

#include <stdio.h>

#include "difftest.h"
#include "tg3000.h"

// port request 2: the trap names the code hit (the bench then stops, as a port may)
void platform_unported(const char* what) { fprintf(stderr, "unported: %s\n", what); }

void video_upload_vram(uint16_t vram_word, uint32_t src, uint32_t size) {
  difftest_effect(EFFECT_VRAM, vram_word, src, size);
}

void video_fill_vram(uint16_t vram_word, uint8_t value, uint32_t size) {
  difftest_effect(EFFECT_VRAM_FILL, vram_word, value, size);
}

// port request J: a note for a port; the words themselves are compared
void video_note_vs_panels(uint16_t vram_word, uint32_t rows) {
  (void)vram_word;
  (void)rows;
}

// port request I: a note for a port; the uploads themselves are compared
void video_note_source(uint32_t rom, uint16_t vram_word) {
  (void)rom;
  (void)vram_word;
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

void video_set_m7_vofs(uint16_t value) { difftest_ppu_field(PF_M7V, value); }

void video_set_bg1_vofs(uint16_t value) {
  difftest_ppu_field(PF_BG1V, value);
  difftest_ppu_field(PF_M7V, value);
}

void video_set_bg3_hofs(uint16_t value) { difftest_ppu_field(PF_BG3H, value); }
void video_set_bg3_vofs(uint16_t value) { difftest_ppu_field(PF_BG3V, value); }
void video_set_bg4_hofs(uint16_t value) { difftest_ppu_field(PF_BG4H, value); }
void video_set_bg4_vofs(uint16_t value) { difftest_ppu_field(PF_BG4V, value); }
void video_set_mosaic(uint8_t value) { difftest_reg(0x2106, value); }
void video_set_m7_select(uint8_t value) { difftest_reg(0x211A, value); }
void video_set_window_position(int edge, uint8_t value) { difftest_reg((uint16_t)(0x2126 + edge), value); }
void video_set_window_logic_bg(uint8_t value) { difftest_reg(0x212A, value); }
void video_set_window_logic_obj(uint8_t value) { difftest_reg(0x212B, value); }
void video_set_main_window_mask(uint8_t value) { difftest_reg(0x212E, value); }
void video_set_sub_window_mask(uint8_t value) { difftest_reg(0x212F, value); }
void video_set_screen_init(uint8_t value) { difftest_reg(0x2133, value); }
void platform_set_rom_speed(uint8_t value) { difftest_reg(0x420D, value); }
void video_set_vram_address(uint16_t word) { difftest_vram_address(word); }
void video_set_bg_mode(uint8_t value) { difftest_reg(0x2105, value); }
void video_set_bg_map(int bg, uint8_t value) { difftest_reg((uint16_t)(0x2106 + bg), value); }
void video_set_bg_tiles(int pair, uint8_t value) { difftest_reg((uint16_t)(0x210B + pair), value); }
void video_set_obj_select(uint8_t value) { difftest_reg(0x2101, value); }
void video_set_display(uint8_t value) { difftest_reg(0x2100, value); }
void video_set_vram_increment(uint8_t value) { difftest_reg(0x2115, value); }

// DMA channels as slots: the registers a setup leaves, claimed byte by byte.
static void claimSetup(int slot, uint8_t mode, uint8_t target, uint32_t adr) {
  difftest_dma_claim(slot, 0, mode);
  difftest_dma_claim(slot, 1, target);
  difftest_dma_claim(slot, 2, (uint8_t)adr);
  difftest_dma_claim(slot, 3, (uint8_t)(adr >> 8));
  difftest_dma_claim(slot, 4, (uint8_t)(adr >> 16));
}
void dma_hdma_setup(int slot, uint8_t mode, uint8_t target, uint32_t table) { claimSetup(slot, mode, target, table); }
void dma_hdma_set_indirect_bank(int slot, uint8_t bank) { difftest_dma_claim(slot, 7, bank); }
void dma_hdma_set_mode(int slot, uint8_t mode, uint8_t target) {
  difftest_dma_claim(slot, 0, mode);
  difftest_dma_claim(slot, 1, target);
}
void dma_hdma_set_bank(int slot, uint8_t bank) { difftest_dma_claim(slot, 4, bank); }
void dma_hdma_set_table(int slot, uint16_t table) {
  difftest_dma_claim(slot, 2, (uint8_t)table);
  difftest_dma_claim(slot, 3, (uint8_t)(table >> 8));
}
void video_hdma_enable(uint8_t slots) { difftest_reg(0x420C, slots); }
void dma_stream_setup(int slot, uint8_t mode, uint8_t target, uint32_t src) { claimSetup(slot, mode, target, src); }
void dma_stream_size(int slot, uint32_t size) { difftest_stream_size(slot, size); }
void dma_stream_set_source(int slot, uint16_t src) {
  difftest_dma_claim(slot, 2, (uint8_t)src);
  difftest_dma_claim(slot, 3, (uint8_t)(src >> 8));
}
void dma_stream_kick(uint8_t slots) { difftest_stream_kick(slots); }
void video_set_cgram_address(uint8_t index) { difftest_cgram_address(index); }
void platform_set_irq_h_low(uint8_t dot) { difftest_reg(0x4207, dot); }
void platform_set_irq_v_low(uint8_t line) { difftest_reg(0x4209, line); }
void platform_set_irq_v_high(uint8_t line) { difftest_reg(0x420A, line); }
void platform_set_wram_address(uint32_t address) {
  difftest_reg(0x2181, (uint8_t)address);
  difftest_reg(0x2182, (uint8_t)(address >> 8));
  difftest_reg(0x2183, (uint8_t)(address >> 16));
}
void video_set_oam_address(uint16_t value) {
  difftest_reg(0x2102, (uint8_t)value);
  difftest_reg(0x2103, (uint8_t)(value >> 8));
}
void video_upload_oam(uint16_t oam_word, uint32_t src, uint32_t size) { difftest_effect(EFFECT_OAM, oam_word, src, size); }

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

// The start-up loader ($86:E096): each block's header (destination, "more", the kick) and
// its bytes, as the original's port writes; its waits are the platform's (their reads are not
// recorded, harness-38). The kick is $CC first, then the last count + 4, never 0.
static uint8_t bootKick;
static void bootHeader(uint16_t dest, uint8_t more) {
  difftest_effect(EFFECT_APU, 2, dest & 0xFF, 1);
  difftest_effect(EFFECT_APU, 3, dest >> 8, 1);
  difftest_effect(EFFECT_APU, 1, more, 1);
  difftest_effect(EFFECT_APU, 0, bootKick, 1);
}
void sound_boot_begin(void) { bootKick = 0xCC; }
void sound_boot_block(uint16_t spc_dest, uint32_t src, uint16_t size) {
  bootHeader(spc_dest, 1);
  for(uint32_t i = 0; i < size; i++) {
    difftest_effect(EFFECT_APU, 0, i & 0xFF, 1);
    difftest_effect(EFFECT_APU, 1, rd8(src + i), 1);
  }
  bootKick = (uint8_t)(size + 4);
  if(bootKick == 0) bootKick = 4;
}
void sound_boot_start(uint16_t spc_entry) { bootHeader(spc_entry, 0); }
void sound_clear_ports(void) {
  for(int p = 0; p < 4; p++) difftest_effect(EFFECT_APU, p, 0, 1);
}

// A set's uploads ($86:E0F0, $86:E2B4): the driver's protocol as the original's port writes.
// $AB on port 3 to start; then per block its size and its destination (ports 0-1), the bytes
// three at a time (ports 0-2; the last one or two alone), each step flagged by toggling port 3;
// at the end a size of 0, $BC, and the ports cleared. Its waits are the platform's (harness-38).
static uint8_t uploadToggle;
static struct { uint8_t slot; uint16_t dest, loop; } pendingSamples[64];
static int pendingCount;
static void uploadStep(void) {
  uploadToggle ^= 0xFF;
  difftest_effect(EFFECT_APU, 3, uploadToggle, 1);
}
static void uploadBytes(uint16_t spc_dest, uint32_t src, uint16_t size) {
  difftest_effect(EFFECT_APU, 0, size & 0xFF, 1);
  difftest_effect(EFFECT_APU, 1, size >> 8, 1);
  uploadStep();
  difftest_effect(EFFECT_APU, 0, spc_dest & 0xFF, 1);
  difftest_effect(EFFECT_APU, 1, spc_dest >> 8, 1);
  uploadStep();
  for(uint32_t i = 0; i < size;) {
    for(int p = 0; p < 3 && i < size; p++, i++) difftest_effect(EFFECT_APU, p, bus_rd8(src + i), 1);
    uploadStep();
  }
}
void sound_upload_begin(void) {
  difftest_effect(EFFECT_APU, 3, 0xAB, 1);
  uploadToggle = 0;
}
// The directory: each sample sent before it must sit in its slot with its loop point (a
// mismatch adds a write the original never made).
void sound_upload_block(uint16_t spc_dest, uint32_t src, uint16_t size) {
  for(int i = 0; i < pendingCount; i++) {
    uint32_t o = pendingSamples[i].slot * 4u;
    if(o + 4 > size || bus_rd16(src + o) != pendingSamples[i].dest || bus_rd16(src + o + 2) != pendingSamples[i].loop)
      difftest_effect(EFFECT_APU, 7, 0xEE, 1);
  }
  pendingCount = 0;
  uploadBytes(spc_dest, src, size);
}
void sound_upload_sample(uint8_t slot, uint16_t spc_dest, uint32_t src, uint16_t size, uint16_t spc_loop) {
  if(pendingCount < 64) pendingSamples[pendingCount++] = (typeof(pendingSamples[0])){slot, spc_dest, spc_loop};
  uploadBytes(spc_dest, src, size);
}
void sound_upload_end(void) {
  difftest_effect(EFFECT_APU, 0, 0, 1);
  difftest_effect(EFFECT_APU, 1, 0, 1);
  difftest_effect(EFFECT_APU, 3, uploadToggle ^ 0xFF, 1);
  difftest_effect(EFFECT_APU, 3, 0xBC, 1);
  for(int p = 0; p < 4; p++) difftest_effect(EFFECT_APU, p, 0, 1);
}

// harness-39: the start-up's waits are the platform's (the original's polls of $4212 and of the
// DSP-4's data port there are neither inputs nor hardware the C must reproduce)
void platform_wait_vblank_start(void) {}
void platform_wait_dsp4_start(void) {}

// port request D: the two-task screens' tasks are verified as rows (either D); no scheduler here
void platform_task_split(void (*task)(uint16_t dp)) { (void)task; }
void platform_task_join(void) {}

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

// The DSP-4 port: the differential test checks the C's bytes against the original's in
// place and serves the original's reads (its tape).
void dsp4_port_write16(uint16_t value) {
  difftest_dsp4_byte((uint8_t)value, true);
  difftest_dsp4_byte((uint8_t)(value >> 8), true);
}
uint16_t dsp4_port_read16(void) {
  const uint8_t lo = difftest_dsp4_byte(0, false);
  return (uint16_t)(lo | difftest_dsp4_byte(0, false) << 8);
}

