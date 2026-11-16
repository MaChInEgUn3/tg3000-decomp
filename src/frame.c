// SPDX-License-Identifier: GPL-3.0-or-later
#include "frame.h"

#include "misc.h"
#include "video_dma.h"
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

// TG3000_ROUTINE($80:FBD1) nmi_target_80FBD1
void nmi_target_80FBD1(void) {
  func_8A8000();
  nmi_tail();
}

// TG3000_ROUTINE($80:FC5F) nmi_target_80FC5F
void nmi_target_80FC5F(void) {
  wr16(0x0026, (uint16_t)(rd16(0x0026) | 0x0002));
  func_8AA050();
  nmi_tail();
}

// TG3000_ROUTINE($80:E48A) irq_split_bottom
//
// The original waits (WAI) for the next interrupt, at the dot it has just set, before its
// register writes, so that they land in that line's horizontal blank; a port applies them
// from the line after this one (docs/DECISIONS.md, "Raster interrupts").
void irq_split_bottom(void) {
  platform_set_irq_h(0x0100);
  video_set_bg2_hofs(0);
  video_set_bg1_hofs(0);
  video_set_bg3_hofs(0);
  video_set_main_screen(0x17);
  video_set_sub_screen(0x00);
  video_set_color_math(0x05);
}

// TG3000_ROUTINE($80:E3F7) irq_split_top
void irq_split_top(void) {
  if(rd16(0x017A) < 0x64) {
    platform_set_irq_h(0x0100);
    uint16_t scroll = rd16(0x0120);  // (after the WAI; see irq_split_bottom)
    video_set_bg2_hofs(scroll);
    video_set_bg1_hofs(scroll);
    video_set_bg3_hofs(scroll);
    video_set_main_screen(0x17);
    video_set_sub_screen(0x10);
    video_set_color_math(0x05);
    video_set_color(0, 0x7CC6);
    platform_set_interrupts(0xB1);
    platform_set_irq_v(0x00B3);
    platform_set_irq_h(0x0010);
    wr16(0x0006, 0x8000);  // the IRQ trampoline's JML target: $80:E48A
    wr16(0x0005, 0xE48A);
  }
  video_set_fixed_color(0xE0);
  video_set_color_math_control(0x02);
  video_set_window_bg12(0xAA);
  video_set_window_bg34(0x0A);
  video_set_window_obj(0x0A);
}

// TG3000_ROUTINE($80:E28C) nmi_target_80E28C
//
// The table writes are words that overlap byte-wise (an entry is a count byte and a word);
// they are made in the original's order.
void nmi_target_80E28C(void) {
  uint16_t v = (uint16_t)(rd16(0x017A) - 4);
  if(v & 0x8000) v = 0;
  wr16(0x017A, v);
  uint16_t x = 0;
  bool tail_rows = true;  // the rows at $80:E311
  if(v == 0) {
    wr16(0x1B00, 0x0072); wr16(0x0D00, 0x0072); wr16(0x0D28, 0x0072);
    wr16(0x1B01, 0xFFFF); wr16(0x0D01, 0x0009); wr16(0x0D29, 0x0017);
    x = 3;
  } else {
    wr16(0x1B00, v); wr16(0x0D00, v); wr16(0x0D28, v);
    wr16(0x1B01, (uint16_t)(0xFFFF - v)); wr16(0x0D01, 0x0001); wr16(0x0D29, 0x0000);
    uint16_t rest = (uint16_t)(0x72 - v);
    if(rest == 0) {
      tail_rows = false;
    } else {
      wr16(0x1B03, rest); wr16(0x0D03, rest); wr16(0x0D2B, rest);
      wr16(0x1B04, 0xFFFF); wr16(0x0D04, 0x0009); wr16(0x0D2C, 0x0017);
      x = 6;
    }
  }
  if(tail_rows) {
    uint16_t a = (uint16_t)(0x72 - v);
    if(a >= 0x6E) a = 0;
    wr16((uint16_t)(0x1B00 + x), a); wr16((uint16_t)(0x0D00 + x), a); wr16((uint16_t)(0x0D28 + x), a);
    wr16((uint16_t)(0x1B01 + x), 0xFFFF); wr16((uint16_t)(0x0D01 + x), 0x0009); wr16((uint16_t)(0x0D29 + x), 0x0017);
  }
  wr16((uint16_t)(0x1B03 + x), 1); wr16((uint16_t)(0x0D03 + x), 1); wr16((uint16_t)(0x0D2B + x), 1);
  wr16((uint16_t)(0x1B04 + x), (uint16_t)(0xFFFB + v));
  wr16((uint16_t)(0x0D04 + x), 1); wr16((uint16_t)(0x0D2C + x), 0);
  wr16((uint16_t)(0x1B06 + x), 0); wr16((uint16_t)(0x0D06 + x), 0); wr16((uint16_t)(0x0D2E + x), 0);
  wr16(0x0D14, 0x0072); wr16(0x0D15, 0);
  wr16(0x0D17, (uint16_t)(0x72 - v)); wr16(0x0D18, 0);
  wr16(0x0D1A, 1); wr16(0x0D1B, 0); wr16(0x0D1D, 0);
  platform_set_interrupts(0xB1);
  platform_set_irq_v(0x007E);
  platform_set_irq_h(0x0010);
  wr16(0x0006, 0x8000);  // the IRQ trampoline's JML target: $80:E3F7 (irq_split_top)
  wr16(0x0005, 0xE3F7);
  video_set_bg1_hofs(0);
  video_set_bg2_hofs(0);
  video_set_bg3_hofs(0);
  video_set_color(0, 0x1BE6);
  video_set_color_math(0x17);
  video_set_fixed_color(rd8(0x017C));
  video_set_fixed_color(rd8(0x017D));
  video_set_color_math_control(0x12);
  video_set_window_bg12(0x22);
  video_set_window_bg34(0x02);
  video_set_window_obj(0x82);
  video_set_main_screen(0x17);
  video_set_sub_screen(0x00);
  nmi_tail();
}

#define BAND_DP 0x0100  // D while $80:E175 runs
#define BAND_TABLE 0x1B00

// TG3000_ROUTINE($80:E175) nmi_target_80E175
//
// As in $80:E28C, the table is written as overlapping words in the original's order.
void nmi_target_80E175(void) {
  if(rd16(BAND_DP + 0x16) == 0) {
    uint16_t h = rd16(BAND_DP + 0x14), x = 0;
    wr16(BAND_TABLE + x, (uint16_t)(h + 0x38)); x = 1;
    wr16(BAND_TABLE + x, (uint16_t)(0xFFDF - h)); x = 3;
    uint16_t a = (uint16_t)(0x34 - h);
    if(a != 0 && !(a & 0x8000)) {  // the band is still opening above line $34
      wr16((uint16_t)(BAND_TABLE + x), a); x = 4;
      wr16((uint16_t)(BAND_TABLE + x), 0x001F); x = 6;
      wr16((uint16_t)(BAND_TABLE + x), 0x0088);
      for(uint16_t k = 0, p = 0xFFEB; k < 8; k++, p--) wr16((uint16_t)(BAND_TABLE + 1 + 2 * k + x), p);
      wr16((uint16_t)(BAND_TABLE + 0x11 + x), (uint16_t)(0x38 - h));
      wr16((uint16_t)(BAND_TABLE + 0x12 + x), 0xFF6B);
      wr16((uint16_t)(BAND_TABLE + 0x14 + x), 0x0001);
      wr16((uint16_t)(BAND_TABLE + 0x15 + x), (uint16_t)(0xFFAB + h));
      wr16((uint16_t)(BAND_TABLE + 0x17 + x), 0);
    } else {
      a = (uint16_t)(a + 8);
      if(a != 0) {  // a repeat run of a lines, each pointing one entry lower
        wr16(BAND_DP + 0x18, x);
        wr16((uint16_t)(BAND_TABLE + x), (uint16_t)(a | 0x80)); x++;
        uint16_t p = (uint16_t)((a & 0x7F) + 0xFFE3);
        for(uint16_t n = a; n != 0; n--, p--, x = (uint16_t)(x + 2)) wr16((uint16_t)(BAND_TABLE + x), p);
      }
      a = (uint16_t)(0x38 - h);
      if(a == 0 || (a & 0x8000)) {  // shorten that run instead
        wr16(BAND_DP + 0x1A, a);
        wr16(BAND_DP + 0x16, (uint16_t)(a << 1));
        uint16_t next = (uint16_t)(x + rd16(BAND_DP + 0x16));
        uint16_t at = (uint16_t)(BAND_TABLE + rd16(BAND_DP + 0x18));
        uint8_t count = (uint8_t)(rd8(at) + (uint8_t)a);
        wr8(at, count);
        if(count == 0x80) next = (uint16_t)((next & 0xFF00) | (uint8_t)(next - 1));  // DEC of its low byte on the stack
        x = next;
      } else {
        wr16((uint16_t)(BAND_TABLE + x), a); x++;
        wr16((uint16_t)(BAND_TABLE + x), 0xFF6B); x = (uint16_t)(x + 2);
      }
      wr16((uint16_t)(BAND_TABLE + x), 0x0001);
      wr16((uint16_t)(BAND_TABLE + 1 + x), (uint16_t)(0xFFAB + h));
      wr16((uint16_t)(BAND_TABLE + 3 + x), 0);
    }
    if(h == 0x3A) {
      wr16(BAND_DP + 0x16, 1);
    } else {
      h = (uint16_t)(h + 4);
      if(h >= 0x3A) h = 0x3A;
      wr16(BAND_DP + 0x14, h);
      wr16(BAND_DP + 0x16, 0);
    }
  }
  nmi_tail();
}

// TG3000_ROUTINE($80:D784) nmi_target_80D784
void nmi_target_80D784(void) {
  for(int i = 0; i < 7; i++) wr8((uint16_t)(0x1B00 + i), rom8(0x81C207 + (uint32_t)i));  // MVN
  video_upload_vram(0x0000, 0x7F0000, 0x0800);
  video_upload_vram(0x0800, 0x7F0800, 0x0800);
  video_set_main_screen(0x02);
  video_set_bg_map(2, 0x03);
  video_set_bg_map(1, 0x51);
  video_set_bg_map(3, 0x59);
  video_set_bg_mode(0x09);
  video_set_obj_select(0x03);
  video_set_bg_tiles(1, 0x03);
  video_set_bg3_hofs(0);
  video_set_bg3_vofs(0xFFFF);
  video_set_color_math_control(0x02);
  video_set_color(2, 0x0000);
  nmi_tail();
}
