// SPDX-License-Identifier: GPL-3.0-or-later
#include "misc.h"

#include "tg3000.h"
#include "util.h"

// TG3000_ROUTINE($81:F251) func_81F251
void func_81F251(void) {}

// TG3000_ROUTINE($8A:8001) clear_dp_scratch (long call)
void clear_dp_scratch(void) {
  for(int i = 0x7E; i >= 0; i -= 2) wr16(DP + 0x14 + i, 0);
}

#define ORDER_LIST 0x1E6A   // 20 words
#define ORDER_LAST 0x1E90
#define ORDER_NEXT 0xFACD   // in bank $7E

// TG3000_ROUTINE($81:F1FC) func_81F1FC
uint16_t func_81F1FC(void) {
  uint16_t index = rd16(ORDER_LIST);
  for(uint16_t i = 0; i < 0x26; i += 2) {
    uint16_t value = rd16(ORDER_LIST + 2 + i);
    wr16(ORDER_NEXT + index, value);
    index = value;
  }
  uint16_t first = rd16(ORDER_LIST);
  wr16(ORDER_NEXT + rd16(ORDER_LAST), first);
  return first;
}

// TG3000_ROUTINE($8A:A216) func_8AA216
void func_8AA216(uint16_t entry) {
  uint16_t slot = take_slot(DP + 0x8C);
  wr16(0x0500 + slot, 0xFFFF);
  wr16(0x0600 + slot, rd16(0x0D60 + entry));
  wr16(0x0700 + slot, rd16(0x0D68 + entry));
  wr16(0x0800 + slot, 0x3002);
  slot = take_slot(DP + 0x8C);
  wr16(0x0500 + slot, 0xFFFF);
  wr16(0x0600 + slot, rd16(0x0D60 + entry));
  wr16(0x0700 + slot, (uint16_t)(rd16(0x0D68 + entry) + 2 * rd16(DP + 0x4C)));
  wr16(0x0800 + slot, 0xB002);
  wr16(0x0D60 + entry, (uint16_t)(rd16(0x0D60 + entry) + 8));
}

// TG3000_ROUTINE($8A:92CF) func_8A92CF
uint16_t func_8A92CF(uint16_t slot) {
  uint16_t x = rd16(DP + 0x7A), y = rd16(DP + 0x7C);
  wr16(0x06C0 + slot, x);
  wr16(0x06C2 + slot, (uint16_t)(x + 0x10));
  wr16(0x07C0 + slot, y);
  wr16(0x07C2 + slot, y);
  wr16(0x05C0 + slot, 1);
  wr16(0x05C2 + slot, 1);
  wr16(0x08C0 + slot, 0x04A0);
  wr16(0x08C2 + slot, 0x04A2);
  return (uint16_t)(slot + 4);
}

// TG3000_ROUTINE($8A:B909) tilemap_offset
uint16_t tilemap_offset(void) {
  uint16_t column = (uint16_t)(rd16(DP + 0x36) << 1);
  wr16(DP + 0x36, column);
  return (uint16_t)((rd16(DP + 0x38) << 6) + column);
}

// TG3000_ROUTINE($8A:B700) func_8AB700
void func_8AB700(void) {
  int16_t i = (int16_t)(rd16(DP + 0x3C) - 10);
  if(i < 0) return;
  wr16(0x0D04 + (uint16_t)i, (rd16(DP + 0x5A) & 1) ? 4 : 0);
}

// TG3000_ROUTINE($8A:A264) step_toward: $0D80,x towards $0D88,x
// TG3000_ROUTINE($8A:A27D) step_toward: $0D70,x towards $0D78,x
// TG3000_ROUTINE($8A:A296) step_toward: $0D68,x towards $0D90,x
uint16_t step_toward(uint16_t value, uint16_t target, uint16_t steps, uint16_t* last_diff) {
  for(;;) {
    uint16_t diff = (uint16_t)(rd16(target) - rd16(value));
    *last_diff = diff;
    if(diff == 0) return steps;
    wr16(value, (uint16_t)(rd16(value) + ((int16_t)diff < 0 ? -1 : 1)));
    if(--steps == 0) return 0;
  }
}
