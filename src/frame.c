// SPDX-License-Identifier: GPL-3.0-or-later
#include "frame.h"

#include "misc.h"
#include "shop.h"
#include "video_dma.h"
#include "vram_queue.h"
#include "pads.h"
#include "platform.h"
#include "sound.h"
#include "tg3000.h"

// TG3000_ROUTINE($80:8373) wait_frames
// TG3000_ROUTINE($8A:BA90) wait_frames (long-call copy in bank $8A)
// TG3000_ROUTINE($82:841E) wait_frames (copy in bank $82)
// TG3000_ROUTINE($8A:8BB6) wait_frames (another long-call copy in bank $8A)
// TG3000_ROUTINE($8A:9880) wait_frames (and another)
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

// TG3000_ROUTINE($80:8807) nmi_prologue
//
// The NMI outside the race, up to its dispatch: the original saves the registers, sets
// DB = $81 and D = 0, and ends by jumping through $0008 to the current target (each has
// its own row; in C the caller calls the target after this). The brightness fade
// ($019A towards $0198 by $0196) and the display; the palette ($81:0500, 512 bytes) and
// the sprite table ($81:0900, 544 bytes) when their flags in $0026 ask, each costing
// $280 of the frame's VRAM budget (4 KB); then the VRAM queue within the rest.
void nmi_prologue(void) {
  wr16(VRAM_QUEUE_BUDGET, 0x1000);
  uint16_t step = rd16(0x0196);
  if(step) {  // fade $019A towards $0198 by `step`, stopping at it
    uint16_t level = (uint16_t)(rd16(0x019A) + step);
    wr16(0x019A, level);
    uint16_t past = (uint16_t)(level - rd16(0x0198));
    if(past == 0 || !((past ^ step) & 0x8000)) {
      wr16(0x019A, rd16(0x0198));
      wr16(0x0196, 0);
    }
  }
  uint8_t brightness = rd8(0x019B);
  video_set_display(brightness ? brightness : 0x80);  // brightness 0: forced blank
  uint16_t flags = rd16(0x0026);
  wr16(0x0026, (uint16_t)(flags & ~0x0001));
  if(flags & 0x0001) {
    video_upload_cgram(0, 0x810500, 0x0200);
    wr16(VRAM_QUEUE_BUDGET, (uint16_t)(rd16(VRAM_QUEUE_BUDGET) - 0x0280));
  }
  flags = rd16(0x0026);
  wr16(0x0026, (uint16_t)(flags & ~0x0002));
  if(flags & 0x0002) {
    video_set_oam_address(0x0000);
    video_upload_oam(0x0000, 0x810900, 0x0220);
    video_set_oam_address(0x0000);
    wr16(VRAM_QUEUE_BUDGET, (uint16_t)(rd16(VRAM_QUEUE_BUDGET) - 0x0280));
  }
  vram_queue_drain();
}

// TG3000_ROUTINE($85:8377) nmi_race_prologue
//
// The race NMI up to its dispatch through $0008 (to $85:838A or $85:83CE): the original
// only saves the registers and sets D = 0, DB = $85, so there is nothing to do in C.
void nmi_race_prologue(void) {}

// TG3000_ROUTINE($83:9721) nmi_vs_prologue
// The VS race's NMI up to its dispatch through $0008 ($83:9777): the original saves the
// registers, sets D = 0 and DB = $83 (A 16-bit; X and Y as they were). Nothing for the C.
void nmi_vs_prologue(void) {}

// TG3000_ROUTINE($83:9734) nmi_vs_tail
// The VS race's NMI tail: nmi_race_tail's (a short JSR to pads_read for its JSL). Only the
// full frame $83:9777 jumps here.
void nmi_vs_tail(void) { nmi_race_tail(); }

// TG3000_ROUTINE($85:838A) nmi_race_tail
//
// The race NMI's tail, run on every race frame: the race NMI $85:8377 dispatches through
// $0008 either here or to the full frame $85:83CE, which ends by jumping here. The
// brightness fade, the display, sound, pads, and the frame counters. The original enables
// IRQs here (CLI), so the race's raster IRQs may start before it returns.
void nmi_race_tail(void) {
  uint16_t step = rd16(0x0146);
  if(step) {  // fade $014A towards $0148 by `step`, stopping at it
    uint16_t level = (uint16_t)(rd16(0x014A) + step);
    wr16(0x014A, level);
    uint16_t past = (uint16_t)(level - rd16(0x0148));
    if(past == 0 || !((past ^ step) & 0x8000)) {
      wr16(0x014A, rd16(0x0148));
      wr16(0x0146, 0);
    }
  }
  uint8_t brightness = rd8(0x014B);
  video_set_display(brightness ? brightness : 0x80);  // brightness 0: forced blank
  sound_frame();
  pads_read_long_2();
  wr16(FRAME_COUNTER, (uint16_t)(rd16(FRAME_COUNTER) + 1));
  wr16(0x00BA, (uint16_t)(rd16(0x00BA) + 1));
}

// TG3000_ROUTINE($80:FBD1) nmi_target_80FBD1
void nmi_target_80FBD1(void) {
  func_8A8000();
  nmi_tail();
}

// TG3000_ROUTINE($80:FC14) nmi_target_80FC14
// The results screen's NMI target (port request 8): $0026 |= 2, func_8A987F (empty), then the
// common tail. The original jumps to $8A:968E for its body and back to $80:FC18.
void nmi_target_80FC14(void) {
  wr16(0x0026, (uint16_t)(rd16(0x0026) | 0x0002));
  func_8A987F();
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

// One task's upload ring ($1900 size, $1940 source, $1980 bank, $19C0 VRAM word) from its
// tail to its head; `ring` is the index's base ($0000 or $0100). Returns the budget left.
static uint16_t menu_ring_drain(uint32_t tail_at, uint32_t head_at, uint16_t ring, uint16_t budget) {
  uint16_t x = rd16(tail_at);
  while(x != rd16(head_at)) {
    const uint16_t size = rd16(0x1900 + (uint32_t)x), vram = rd16(0x19C0 + (uint32_t)x), src = rd16(0x1940 + (uint32_t)x);
    const uint32_t bank = (uint32_t)rd8(0x1980 + (uint32_t)x) << 16;
    if(size > budget) {  // only part of it: the entry keeps the rest
      wr16(0x1900 + (uint32_t)x, (uint16_t)(size - budget));
      wr16(0x1940 + (uint32_t)x, (uint16_t)(src + budget));
      wr16(0x19C0 + (uint32_t)x, (uint16_t)(vram + (budget >> 1)));
      video_upload_vram(vram, bank | src, budget ? budget : 0x10000);
      return 0;
    }
    video_upload_vram(vram, bank | src, size ? size : 0x10000);
    budget = (uint16_t)(budget - size);
    x = (uint16_t)(((x + 2) & 0x003F) + ring);
    wr16(tail_at, x);
  }
  return budget;
}

// A vertical scroll less the screen's offset, in the original's byte order: the low byte
// less $01C2's, the high byte less $01C4's with that borrow.
static uint16_t menu_vofs(uint32_t at) {
  const uint8_t lo = rd8(at), sub_lo = rd8(0x01C2);
  const uint8_t hi = (uint8_t)(rd8(at + 1) - rd8(0x01C4) - (lo < sub_lo));
  return (uint16_t)((uint16_t)(hi << 8) | (uint8_t)(lo - sub_lo));
}

// TG3000_ROUTINE($80:B97B) nmi_menu_frame
void nmi_menu_frame(void) {
  video_set_display(0x80);
  uint16_t budget = 0x0C00;
  const uint16_t step = rd16(0x0296);
  if(step) {  // as nmi_prologue's fade
    const uint16_t level = (uint16_t)(rd16(0x029A) + step);
    wr16(0x029A, level);
    const uint16_t past = (uint16_t)(level - rd16(0x0298));
    if(past == 0 || !((past ^ step) & 0x8000)) {
      wr16(0x029A, rd16(0x0298));
      wr16(0x0296, 0);
    }
  }
  for(uint32_t p = 0; p < 2; p++) {  // each pad's new presses
    const uint16_t now = rd16(p ? 0x0010 : 0x000C), was = rd16(p ? 0x020C : 0x010C);
    wr16(p ? 0x020C : 0x010C, now);
    wr16(p ? 0x020E : 0x010E, (uint16_t)(((now ^ was) | rd16(p ? 0x020E : 0x010E)) & now));
  }
  const uint16_t odd = (uint16_t)(rd16(FRAME_COUNTER) & 1);
  wr16(0x0290, odd);
  wr16(0x0190, (uint16_t)(odd ^ 1));
  if(odd) budget = menu_ring_drain(0x028E, 0x028C, 0x0100, budget);
  else budget = menu_ring_drain(0x018E, 0x018C, 0x0000, budget);
  wr16(0x002C, budget);
  video_set_bg1_hofs(rd16(0x019C));
  video_set_bg1_vofs(menu_vofs(0x019E));
  video_set_bg2_hofs(rd16(0x01A0));
  video_set_bg2_vofs(menu_vofs(0x01A2));
  video_set_bg3_hofs(rd16(0x01A4));
  video_set_bg3_vofs(menu_vofs(0x01A6));
  video_set_bg_mode(rd8(0x01A8));
  video_set_window_obj(rd8(0x01A9));
  video_set_fixed_color((uint8_t)(rd8(0x01AA) | 0x20));
  video_set_fixed_color((uint8_t)(rd8(0x01AB) | 0x40));
  video_set_fixed_color((uint8_t)(rd8(0x01AC) | 0x80));
  video_set_obj_select(rd8(0x01AD));
  for(int edge = 0; edge < 4; edge++) video_set_window_position(edge, rd8(0x01AE + (uint32_t)edge));
  video_set_window_logic_bg(rd8(0x01B2));
  video_set_window_logic_obj(rd8(0x01B3));
  for(int bg = 1; bg <= 4; bg++) video_set_bg_map(bg, rd8(0x01B3 + (uint32_t)bg));
  video_set_bg_tiles(0, rd8(0x01B8));
  video_set_bg_tiles(1, rd8(0x01B9));
  video_set_window_bg12(rd8(0x01BA));
  video_set_window_bg34(rd8(0x01BB));
  video_set_main_screen(rd8(0x01BC));
  video_set_sub_screen(rd8(0x01BD));
  video_set_main_window_mask(rd8(0x01BE));
  video_set_sub_window_mask(rd8(0x01BF));
  video_set_color_math_control(rd8(0x01C0));
  video_set_color_math(rd8(0x01C1));
  video_upload_cgram(0, 0x810500, 0x0200);
  video_set_oam_address(0x0000);
  video_upload_oam(0x0000, 0x810000 | rd16(0x016E), 0x0220);
  video_set_oam_address(0x0000);
  platform_set_irq_v(0x006C);
  platform_set_irq_h(0x0010);
  platform_set_interrupts(0xB1);
  wr16(0x0005, 0xBC9B);  // the IRQ trampoline's target: $80:BC9B, the second task's frame
  upload_slot_drain(0x0100);
  const uint8_t brightness = rd8(0x019B);
  video_set_display(brightness ? brightness : 0x80);
}

// A task's large upload slot (its D+$64 bytes left, D+$66/$68 the source, D+$6A the VRAM word):
// $1C00 bytes a frame (port request D: task 1's in nmi_menu_frame, task 2's in its IRQ's tail).
void upload_slot_drain(uint16_t dp) {
  const uint16_t left = rd16((uint16_t)(dp + 0x64));
  if(!left) return;
  const uint16_t vram = rd16((uint16_t)(dp + 0x6A)), src = rd16((uint16_t)(dp + 0x66));
  uint16_t size = left;
  if(left >= 0x1C00) {
    wr16((uint16_t)(dp + 0x64), (uint16_t)(left - 0x1C00));
    const uint32_t moved = (uint32_t)src + 0x1C00;
    wr16((uint16_t)(dp + 0x66), (uint16_t)moved);
    wr16((uint16_t)(dp + 0x68), (uint16_t)(rd16((uint16_t)(dp + 0x68)) + (moved > 0xFFFF)));
    wr16((uint16_t)(dp + 0x6A), (uint16_t)(vram + 0x0E00));
    size = 0x1C00;
  } else {
    wr16((uint16_t)(dp + 0x64), 0);
  }
  video_upload_vram(vram, (uint32_t)rd8((uint16_t)(dp + 0x68)) << 16 | src, size);  // the bank as it is now
}

// A task's per-frame hook, the word at its D+$EC (called after the task switch): $80:B94F does
// nothing; $80:B149 is the shop panel's frame; $80:C5E0 is the menu bar's line window.
void task_frame_hook(uint16_t dp) {
  const uint16_t hook = rd16((uint16_t)(dp + 0xEC));
  if(hook == 0xB94F) return;
  if(hook == 0xB149) {  // the shop panel's frame (set by task 1's shop code)
    if(dp != DP) tg3000_unported("$80:B149: the shop panel's frame in task 2");
    shop_panel_frame();
    return;
  }
  if(hook == 0xC5E0) {  // the menu bar: its lines, then its step by D+$4E ($80:C636), then its end
    switch(menu_bar_lines(dp) >> 1) {
    case 0: break;
    case 1: menu_bar_open_step(dp); break;
    case 2: menu_bar_close_step(dp); break;
    case 3: menu_bar_open_colours_step(dp); break;
    case 4:
      if(dp != DP) tg3000_unported("$80:C68D: the controls menu's bar in task 2");
      menu_bar_open_controls_step();
      break;
    case 5:
      if(dp != DP) tg3000_unported("$80:C6A2: the menu panel's bar in task 2");
      menu_bar_open_panel_step();
      break;
    default: tg3000_unported("$80:C636: a menu bar step past its table"); break;
    }
    menu_bar_lines_end(dp);
    return;
  }
  tg3000_unported("a task's frame hook other than $80:B94F, $80:B149 and $80:C5E0");
}

// TG3000_ROUTINE($80:BC60) nmi_menu_frame_tail
// After the switch to task 1 (the stack's: a port's scheduler, port request D): when both tasks
// wait at their end (bit 4 of $0176 and $0276), both are released (bit 3); then, once vblank is
// over, task 1's hook and the common tail.
void nmi_menu_frame_tail(void) {
  if(rd16(0x0176) & rd16(0x0276) & 0x0010) {
    wr16(0x0176, (uint16_t)(rd16(0x0176) | 0x0008));
    wr16(0x0276, (uint16_t)(rd16(0x0276) | 0x0008));
  }
  task_frame_hook(0x0100);
}

// TG3000_ROUTINE($80:BDCD) irq_menu_frame_tail
// After the switch to task 2: its large upload slot, then its hook.
void irq_menu_frame_tail(void) {
  upload_slot_drain(0x0200);
  task_frame_hook(0x0200);
}

// TG3000_ROUTINE($80:BC9B) irq_menu_frame
//
// The original waits (WAI) for the line interrupt twice before its writes; a port applies
// them from the next line (as irq_split_bottom).
void irq_menu_frame(void) {
  const uint8_t brightness = rd8(0x029B) ? rd8(0x029B) : 0x80;
  platform_set_wram_address(0x00029C);
  platform_set_irq_h(0x010A);
  platform_set_interrupts(0x91);
  video_set_display(0x80);
  video_set_bg1_hofs(rd16(0x029C));
  video_set_bg1_vofs((uint16_t)(rd16(0x029E) - 0x74));
  video_set_bg2_hofs(rd16(0x02A0));
  video_set_bg2_vofs((uint16_t)(rd16(0x02A2) - 0x74));
  video_set_bg3_hofs(rd16(0x02A4));
  video_set_bg3_vofs((uint16_t)(rd16(0x02A6) - 0x74));
  video_set_bg_mode(rd8(0x02A8));
  video_set_window_obj(rd8(0x02A9));
  video_set_fixed_color((uint8_t)(rd8(0x02AA) | 0x20));
  video_set_fixed_color((uint8_t)(rd8(0x02AB) | 0x40));
  video_set_fixed_color((uint8_t)(rd8(0x02AC) | 0x80));
  video_set_obj_select(rd8(0x02AD));
  video_set_cgram_address(0x00);
  for(int edge = 0; edge < 4; edge++) video_set_window_position(edge, rd8(0x02AE + (uint32_t)edge));
  video_set_window_logic_bg(rd8(0x01B2));  // the first task's (the ROM reads $01B2 here)
  video_set_window_logic_obj(rd8(0x01B3));
  for(int bg = 1; bg <= 4; bg++) video_set_bg_map(bg, rd8(0x02B3 + (uint32_t)bg));
  video_set_bg_tiles(0, rd8(0x02B8));
  video_set_bg_tiles(1, rd8(0x02B9));
  video_set_window_bg12(rd8(0x02BA));
  video_set_window_bg34(rd8(0x02BB));
  video_set_main_screen(rd8(0x02BC));
  video_set_sub_screen(rd8(0x02BD));
  video_set_main_window_mask(rd8(0x02BE));
  video_set_sub_window_mask(rd8(0x02BF));
  video_set_color_math_control(rd8(0x02C0));
  video_set_color_math(rd8(0x02C1));
  video_set_oam_address(0x0000);
  platform_set_irq_v_high(0x00);  // a 16-bit store at $420A: its high byte kicks the sprites
  video_upload_oam(0x0000, 0x810000 | rd16(0x026E), 0x0220);
  video_upload_cgram(0, 0x810700, 0x0200);
  platform_set_irq_h(0x010A);
  video_set_display(brightness);
  platform_set_interrupts(0xB1);
}

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

// TG3000_ROUTINE($80:80E4) interrupt_vectors_init
void interrupt_vectors_init(void) {
  wr8(0x0000, 0x5C);  // JML
  wr8(0x0004, 0x5C);
  wr8(0x0003, 0x80);
  wr8(0x0007, 0x80);
  wr16(0x0001, 0x8807);
  wr16(0x0005, 0x89B2);
  wr16(0x0008, 0x8934);
  wr16(0x0026, 0);
  wr16(0x019A, 0);
  wr16(0x002A, 0);
  wr16(0x0028, 0);
}

// TG3000_ROUTINE($84:A4CD) wait_03e0
uint16_t wait_03e0(void) {
  while(rd16(0x03E0) != 0) platform_wait_vblank();
  return 0;
}
