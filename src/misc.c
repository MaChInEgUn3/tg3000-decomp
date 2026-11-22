// SPDX-License-Identifier: GPL-3.0-or-later
#include "misc.h"

#include "decompress.h"
#include "dsp4.h"
#include "frame.h"
#include "line_tables.h"
#include "pads.h"
#include "rng.h"
#include "platform.h"
#include "scroll_tables.h"
#include "sprites.h"
#include "text.h"
#include "video_dma.h"
#include "tg3000.h"
#include "track.h"
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

// TG3000_ROUTINE($8A:9C19) byte_buffer_push (8-bit A)
void byte_buffer_push(uint8_t b) {
  uint16_t i = rd16(DP + 0x58);
  wr8(0x17D00 + i, b);
  wr16(DP + 0x58, (uint16_t)(i + 1));
  wr8(DP + 0x60, (uint8_t)(rd8(DP + 0x60) + 1));
}

// TG3000_ROUTINE($8A:B942) func_8AB942
void func_8AB942(void) {
  uint16_t base = rd16(DP + 0x70);
  for(int row = 0; row < 0x1C; row++) {
    uint32_t ptr = ((uint32_t)rd8(DP + 0x72) << 16) | rd16(DP + 0x70);
    uint16_t cols[2] = {rd16(DP + 0x76), rd16(DP + 0x74)};
    for(int k = 0; k < 2; k++) {
      if(bus_rd16(ptr + cols[k]) != 0x003D) wr16(DP + 0x78, (uint16_t)(rd16(DP + 0x78) + 1));
      bus_wr16(ptr + cols[k], 0x003D);
    }
    wr16(DP + 0x70, (uint16_t)(rd16(DP + 0x70) + 0x40));
  }
  wr16(DP + 0x70, base);
  wr16(DP + 0x76, (uint16_t)(rd16(DP + 0x76) + 2));
  wr16(DP + 0x74, (uint16_t)(rd16(DP + 0x74) - 2));
}

// TG3000_ROUTINE($8A:A47E) clear_object_slots
void clear_object_slots(void) {
  wr16(DP + 0x8C, 0);
  for(int i = 0xF6; i >= 0; i -= 2) wr16(0x0500 + i, 0);
}

// TG3000_ROUTINE($8A:A469) func_8AA469
void func_8AA469(void) {
  wr16(0x0D90, (uint16_t)((rd16(DP + 0x3C) << 3) - 0x1C));
  wr16(0x0D78, (uint16_t)(rd16(DP + 0x5A) << 3));
}

// TG3000_ROUTINE($8A:A4CF) string_list_nth
uint16_t string_list_nth(uint32_t list, uint16_t n) {
  uint32_t bank = list & 0xFF0000;
  wr16(DP + 0x44, (uint16_t)list);
  for(; n; n--) {
    while(bus_rd8(bank | rd16(DP + 0x44)) != 0) wr16(DP + 0x44, (uint16_t)(rd16(DP + 0x44) + 1));
    wr8(DP + 0x44, (uint8_t)(rd8(DP + 0x44) + 1));  // 8-bit: no carry into $0145
  }
  return rd16(DP + 0x44);
}

// TG3000_ROUTINE($80:8542) take_nibble
uint16_t take_nibble(void) {
  uint32_t v = ((uint32_t)rd16(DP + 0x18) << 16) | rd16(DP + 0x1A);
  wr16(DP + 0x18, (uint16_t)(v >> 12));
  wr16(DP + 0x1A, (uint16_t)(v << 4));
  return (uint16_t)(v >> 28);
}

// TG3000_ROUTINE($83:FE9A) func_83FE9A
bool func_83FE9A(uint16_t dp, uint16_t a, uint16_t x, uint16_t y) {
  if(rd16(0x047C) & 0x8000) return false;
  if(rd16(dp + 0x23) != 0) {
    uint16_t left = (uint16_t)(rd16(dp + 0x23) - rd16(0x1E9E));
    wr16(dp + 0x23, left);
    if(!(left & 0x8000)) return true;
    wr16(dp + 0x23, y);
  }
  wr16(0x0466, 0);
  wr16(0x0465, x);
  wr16(0x0463, a);
  return false;
}

// One piece of a sprite frame: a slot from take_slot with tile/attributes `tile` at
// (`x`, `y`), $0500 = $FFFF.
static void frame_piece(uint16_t tile, uint16_t x, uint16_t y) {
  uint16_t slot = take_slot(DP + 0x8C);
  wr16(0x0500 + slot, 0xFFFF);
  wr16(0x0800 + slot, tile);
  wr16(0x0600 + slot, x);
  wr16(0x0700 + slot, y);
}

// TG3000_ROUTINE($8A:A0F9) sprite_frame
void sprite_frame(uint16_t entry) {
  uint16_t diff;
  step_toward(0x0D70 + entry, 0x0D78 + entry, rd16(DP + 0x7E), &diff);
  step_toward(0x0D68 + entry, 0x0D90 + entry, rd16(DP + 0x7C), &diff);
  step_toward(0x0D80 + entry, 0x0D88 + entry, rd16(DP + 0x7A), &diff);
  uint16_t x = rd16(0x0D60 + entry), y = rd16(0x0D68 + entry), width = rd16(0x0D70 + entry);
  uint16_t row = rd16(DP + 0x4C);
  wr16(DP + 0x48, width >> 1);
  uint16_t left = (uint16_t)(x - (width >> 1) + rd16(0x0D80 + entry));
  wr16(0x0D60 + entry, left);
  frame_piece(0x3000, left, y);  // left column: corner, side, corner flipped vertically
  frame_piece(0x3001, left, (uint16_t)(y + row));
  frame_piece(0xB000, left, (uint16_t)(y + 2 * row));
  wr16(0x0D60 + entry, (uint16_t)(left + 8));
  while(rd16(0x0D70 + entry) >= 8) {  // middle columns, 8 pixels each
    wr16(0x0D70 + entry, (uint16_t)(rd16(0x0D70 + entry) - 8));
    func_8AA216(entry);
  }
  uint16_t right = (uint16_t)(rd16(0x0D70 + entry) + rd16(0x0D60 + entry) - 8);
  wr16(0x0D60 + entry, right);
  frame_piece(0x7000, right, y);  // right column: the left one flipped horizontally
  frame_piece(0x7001, right, (uint16_t)(y + row));
  frame_piece(0xF000, right, (uint16_t)(y + 2 * row));
  wr16(0x0D70 + entry, width);
  wr16(0x0D68 + entry, y);
  wr16(0x0D60 + entry, x);
}

#define MASK_OBJECTS 0x8A9FE9  // object offsets into $1516, $FFFF-terminated
#define MASK_TABLE   0x8A9FF9  // one mask word per value of $0158

// TG3000_ROUTINE($8A:9FA1) func_8A9FA1
void func_8A9FA1(void) {
  uint16_t mask = rom16(MASK_TABLE + 2 * rd16(DP + 0x58));
  for(uint16_t i = 0;; i += 2, mask >>= 1) {
    uint16_t object = rom16(MASK_OBJECTS + i);
    if(object == 0xFFFF) return;
    uint16_t flags = rd16(0x1516 + object);
    wr16(0x1516 + object, (mask & 1) ? (uint16_t)(flags & 0xFF7F) : (uint16_t)(flags | 0x0080));
  }
}

// TG3000_ROUTINE($8A:B981) clear_tilemap_7f
void clear_tilemap_7f(void) {
  for(uint32_t i = 0; i < 0x1000; i += 2) wr16(0x10000 + i, 0x003D);
}

// TG3000_ROUTINE($8A:BC32) func_8ABC32
void func_8ABC32(void) { wr16(DP + 0x92, (uint16_t)(rd16(DP + 0x92) + rd16(0x0DCA))); }

// TG3000_ROUTINE($8A:BC7F) func_8ABC7F
void func_8ABC7F(void) {
  for(uint16_t i = 0; i < 0x20; i += 2) wr16(0x0DAC + i, 0);
}

// TG3000_ROUTINE($82:EC09) func_82EC09
uint16_t func_82EC09(uint16_t x) {
  uint16_t b = rd8(0x8CB5 + (uint32_t)x);
  uint16_t a = (uint16_t)(rd8(0x8CB3 + (uint32_t)x) << 4);
  if(a & 0x0800) a |= 0xF000;  // the byte is the top of a signed 12-bit value
  wr16(DP + 0x1E, (uint16_t)(b << 1));
  return (uint16_t)(a - b);
}

// TG3000_ROUTINE($8A:B91A) wipe_columns
uint16_t wipe_columns(void) {
  wr16(DP + 0x70, 0x0000);  // the map at $7F:0000
  wr16(DP + 0x72, 0x007F);
  wr16(DP + 0x76, 0);       // from the left edge and the right edge inwards
  wr16(DP + 0x74, 0x003E);
  uint16_t last = 0;
  for(int step = 0; step < 16; step++) {
    wr16(DP + 0x78, 0);
    func_8AB942();
    last = 0;
    if(rd16(DP + 0x78) != 0) last = wait_frames(1);
  }
  return last;
}

static void set_mode_fb(uint8_t mode) {
  wr8(DP + 0xFB, mode);
  wr16(0x0D7E, 0x0A8C);
}

// TG3000_ROUTINE($8A:A48D) func_8AA48D
void func_8AA48D(void) { set_mode_fb(0x11); }

// TG3000_ROUTINE($8A:A49E) func_8AA49E
void func_8AA49E(void) { set_mode_fb(0x13); }

// TG3000_ROUTINE($8A:8F02) clear_sprites (long call)
void clear_sprites(void) {
  for(uint16_t i = 0; i < 0x200; i += 4) wr16(0x0900 + i, 0xE000);
}

// TG3000_ROUTINE($83:EA9B) func_83EA9B
uint16_t func_83EA9B(uint16_t i) {
  wr16(0x1EA2, rom16(0x86DB47 + i));
  wr16(0x1EA4, rom16(0x86DB99 + i));
  wr16(0x1EA6, rom16(0x86DBEB + i));
  wr16(0x1EAA, rom16(0x86DC3D + i));
  wr16(0x1EAC, rom16(0x86DC8F + i));
  uint16_t first = rom16(0x86DAF5 + i);
  wr16(0x1EA0, first);
  return first;
}

#define BIG_NUMBER 0x0DAC  // 16 words, $0DAC the most significant: a 256-bit number
#define BIG_WORDS 16

// TG3000_ROUTINE($8A:BB5C) func_8ABB5C
uint16_t func_8ABB5C(uint16_t count, bool* carry) {
  uint16_t out = 0;
  bool in = *carry;
  do {
    for(int w = 0; w < BIG_WORDS; w++) {  // shift right, `in` entering at the top
      uint16_t v = rd16(BIG_NUMBER + 2 * w);
      wr16(BIG_NUMBER + 2 * w, (uint16_t)((v >> 1) | (in ? 0x8000 : 0)));
      in = v & 1;
    }
    out = (uint16_t)((out << 1) | in);  // the bit that fell off the bottom
    uint16_t feed = rd16(DP + 0x46);
    in = (feed & 0x8000) != 0;  // the next bit in comes from $0146, which fills with 1s
    wr16(DP + 0x46, (uint16_t)((feed << 1) | 1));
  } while(--count != 0);
  *carry = in;
  return out;
}

// TG3000_ROUTINE($8A:BBF4) func_8ABBF4
void func_8ABBF4(uint16_t bits, uint16_t count) {
  do {
    bool in = bits & 1;
    bits >>= 1;
    for(int w = BIG_WORDS - 1; w >= 0; w--) {  // shift left, `in` entering at the bottom
      uint16_t v = rd16(BIG_NUMBER + 2 * w);
      wr16(BIG_NUMBER + 2 * w, (uint16_t)((v << 1) | in));
      in = (v & 0x8000) != 0;
    }
    wr16(DP + 0x84, (uint16_t)(rd16(DP + 0x84) + 1));
    func_8ABC32();
  } while(--count != 0);
}

// TG3000_ROUTINE($80:85CA) take_nibble16
uint16_t take_nibble16(void) {
  uint16_t v = rd16(DP + 0x1A);
  wr16(DP + 0x1A, (uint16_t)(v << 4));
  return (uint16_t)(v >> 12);
}

#define BLOCK_MAP 0x230C      // bank $7E: blocks of 2 rows x 5 words, rows $50 bytes apart
#define BLOCK_SOURCE 0x217C   // bank $7E: the same layout, copied in by func_82B838
#define BLOCK_ROW 0x50
#define BLOCK_COLUMN 0x0156   // set by func_82B78D
#define BLOCK_ROW_INDEX 0x0158

// TG3000_ROUTINE($82:B78D) func_82B78D
uint16_t func_82B78D(uint16_t a) {
  uint32_t at = 0x4F31 + (uint32_t)((uint16_t)(a >> 2) & 0xFFFC);
  wr16(BLOCK_COLUMN, (uint16_t)((rd16(at) - 2) & 0xFF));
  uint16_t row = (uint16_t)((rd16(at + 1) - 2) & 0xFF);
  wr16(BLOCK_ROW_INDEX, row);
  return row;
}

// TG3000_ROUTINE($82:B7B0) func_82B7B0
void func_82B7B0(void) {
  uint16_t column = rd16(BLOCK_COLUMN), row = rd16(BLOCK_ROW_INDEX);
  uint16_t at = (uint16_t)(2 * row + rom16(0x838C65 + (column & 0xFFFE)));
  uint16_t mask = rom16(0x838C55 + 2 * (column & 7));  // one of 8 masks, in bank $83
  for(int i = 0; i < 10; i++) {
    uint32_t cell = BLOCK_MAP + (uint32_t)at + (i < 5 ? 2 * i : BLOCK_ROW + 2 * (i - 5));
    wr16(cell, (uint16_t)(rd16(cell) & bus_rd16(0x830000 + (uint32_t)mask + 2 * i)));
  }
}

// TG3000_ROUTINE($82:B838) func_82B838
uint16_t func_82B838(uint16_t at) {
  uint16_t last = 0;
  for(int i = 0; i < 10; i++) {
    uint16_t off = (uint16_t)(at + (i < 5 ? 2 * i : BLOCK_ROW + 2 * (i - 5)));
    last = rd16(BLOCK_SOURCE + (uint32_t)off);
    wr16(BLOCK_MAP + (uint32_t)off, last);
  }
  return last;
}

// TG3000_ROUTINE($80:DC5A) func_80DC5A
uint32_t func_80DC5A(uint32_t table, uint16_t a) {
  // entries of 3 bytes (line count, word); each written as two overlapping words, as the
  // original does
  wr16(DP + 0x14, a);
  uint16_t low = (uint16_t)((a & 0xFF) + 0x69), high = (uint16_t)(a >> 8);
  wr16(DP + 0x16, low);
  wr16(DP + 0x18, high);
  bus_wr16(table, 0xFF31);
  bus_wr16(table + 1, 0x00FF);
  table += 3;
  if(high != 0) {
    bus_wr16(table, high);
    bus_wr16(table + 1, (uint16_t)(((low << 8) | (low >> 8)) | low));
    table += 3;
  }
  bus_wr16(table, 0x0001);
  bus_wr16(table + 1, (uint16_t)(((low << 8) | (low >> 8)) | 0x69));
  table += 3;
  bus_wr16(table, (uint16_t)(0x4D - high));
  bus_wr16(table + 1, 0x00FF);
  table += 3;
  bus_wr16(table, 0x00B4);
  return table;
}

#define SLOT_USED 0x17FBC   // $7F:7FBC, one word per slot: 0 free, $FFFF taken
#define SLOT_KIND 0x81041C  // the slots' kinds (low WRAM $041C through bank $81)

// TG3000_ROUTINE($8A:9BB5) func_8A9BB5
bool func_8A9BB5(uint16_t* slot) {
  for(;;) {
    uint16_t want = rd16(DP + 0x68), end = rd16(DP + 0x66);
    for(uint16_t x = 0; x != end; x = (uint16_t)(x + 2)) {
      if(rd16(SLOT_USED + (uint32_t)x) == 0 && bus_rd16(SLOT_KIND + x) == want) {
        wr16(SLOT_USED + (uint32_t)x, 0xFFFF);
        *slot = x;
        return true;
      }
    }
    if(want == 0) {
      *slot = end;
      return false;
    }
    wr16(DP + 0x68, (uint16_t)(want - 1));  // settle for a lower kind
  }
}

// TG3000_ROUTINE($8A:8EEF) func_8A8EEF (long call)
void func_8A8EEF(void) {
  wr16(DP + 0x30, 0);
  for(uint16_t i = 0; i < 0x100; i += 2) wr16(0x0500 + i, 0);
  clear_sprites();
}

// TG3000_ROUTINE($8A:9BAA) func_8A9BAA
void func_8A9BAA(void) {
  wr16(DP + 0x54, (uint16_t)(rd16(DP + 0x54) + 0x10));
  wr16(DP + 0x56, (uint16_t)(rd16(DP + 0x56) - 1));
}

// TG3000_ROUTINE($8A:9BE2) func_8A9BE2
uint16_t func_8A9BE2(void) {
  wr16(DP + 0x68, 0);
  for(int x = 0x26; x >= 0; x -= 2) {
    uint16_t kind = bus_rd16(SLOT_KIND + (uint32_t)x);
    // the original keeps the larger one by the sign of kind - best (BMI), not a true
    // signed compare
    if(!((uint16_t)(kind - rd16(DP + 0x68)) & 0x8000)) wr16(DP + 0x68, kind);
  }
  return rd16(DP + 0x68);
}

// TG3000_ROUTINE($8A:B680) func_8AB680
void func_8AB680(uint32_t list) {
  uint32_t bank = list & 0xFF0000;
  uint16_t at = (uint16_t)list;
  wr16(DP + 0x60, at);
  while(bus_rd16(bank + at) != 0) {
    uint16_t x = bus_rd16(bank + at + 4);
    bus_wr16(bank + 0x1516 + x, bus_rd16(bank + at + 6));
    at = (uint16_t)(at + 8);
    wr16(DP + 0x60, at);
  }
}

// TG3000_ROUTINE($83:FC05) func_83FC05
bool func_83FC05(uint16_t dp) {
  uint16_t x = rd16(0x011A), kind = rd16(0x011C);
  uint16_t v = (uint16_t)(rd8(0x8CB3 + (uint32_t)x) << 4);
  if(v & 0x0800) v |= 0xF000;  // a signed 12-bit value
  wr16(0x0128, v);
  // the original tests the sign of each difference (BPL/BMI), not a signed compare
  uint16_t d = (uint16_t)(v - rd16(dp + 0x06));
  if(!(d & 0x8000)) {
    if(!((uint16_t)(d - bus_rd16(0x8380CE + kind)) & 0x8000)) return false;
  } else {
    if((uint16_t)(d - bus_rd16(0x8380CC + kind)) & 0x8000) return false;
  }
  uint16_t h = (uint16_t)(-(uint16_t)((rd8(0x8CB5 + (uint32_t)x) & 0xF0) >> 1));
  uint16_t e = (uint16_t)(h - bus_rd16(0x81012C));
  if(!(e & 0x8000)) return ((uint16_t)(e - bus_rd16(0x83826E + kind)) & 0x8000) != 0;
  return !((uint16_t)(e - bus_rd16(0x83826C + kind)) & 0x8000);
}

// TG3000_ROUTINE($83:FD8A) func_83FD8A
bool func_83FD8A(uint16_t dp, uint16_t a) {
  if(rd16(0x0120) & 0x8000) return false;
  uint16_t kind = rd16(0x011C);
  // the same limits as func_83FC05, judged by the sign of each difference
  uint16_t d = (uint16_t)(a - rd16(dp + 0x06));
  if(!(d & 0x8000)) {
    if(!((uint16_t)(d - bus_rd16(0x8380CE + kind)) & 0x8000)) return false;
  } else if((uint16_t)(d - bus_rd16(0x8380CC + kind)) & 0x8000) return false;
  uint16_t e = (uint16_t)(rd16(0x012A) - bus_rd16(0x81012C));
  if(!(e & 0x8000)) {
    if(!((uint16_t)(e - bus_rd16(0x83826E + kind)) & 0x8000)) return false;
  } else if((uint16_t)(e - bus_rd16(0x83826C + kind)) & 0x8000) return false;

  // a hit
  wr16(dp + 0x7C, (uint16_t)(rd16(dp + 0x7C) & ~0x0008));
  func_83FE9A(dp, 0xFF23, rd16(0x1EBE), 0);
  if(!(rd16(0x1F31) & 0x8000)) {
    // D+$5A += (D+$0F / 2) * $1F30 / 32768, saturating at $FFFF (the product's bits 15-30,
    // with bit 31 as the carry in)
    uint32_t p = (uint32_t)dsp4_multiply((int16_t)(rd16(dp + 0x0F) >> 1), (int16_t)rd16(0x1F30));
    uint16_t top = (uint16_t)((p >> 15) & 0xFFFF);
    uint32_t sum = (uint32_t)top + rd16(dp + 0x5A) + (p >> 31);
    wr16(dp + 0x5A, sum > 0xFFFF ? 0xFFFF : (uint16_t)sum);
  }
  bool behind = (rd16(0x0128) & 0x8000) != 0;
  if(!((uint16_t)(rd16(dp + 0x10) - 0x60) & 0x8000) && rd16(dp + 0x1A) == 0) wr16(dp + 0x1A, behind ? 2 : 3);
  wr16(dp + 0x1C, behind ? 0x1000 : 0xF000);
  uint32_t v = ((uint32_t)rd16(dp + 0x18) << 16) | rd16(dp + 0x16);  // halved, keeping the sign
  v = (v >> 1) | (v & 0x80000000u);
  wr16(dp + 0x18, (uint16_t)(v >> 16));
  wr16(dp + 0x16, (uint16_t)v);
  return true;
}

// TG3000_ROUTINE($83:FDCD) func_83FDCD
bool func_83FDCD(void) { return false; }

// TG3000_ROUTINE($82:EA49) func_82EA49
uint16_t func_82EA49(uint16_t a) {
  uint16_t row = (uint16_t)((a >> 6) & 0xFF);
  uint16_t lo = (uint16_t)dsp4_multiply((int16_t)row, 10);  // only the product's low word is used
  uint32_t at = (uint32_t)lo + rd16(DP + 0x98);
  wr16(DP + 0x18, (uint16_t)at);
  // the carry out of that add goes into the next one
  uint32_t entry = (uint32_t)(a & 0x3E) + bus_rd16(0x840002 + (uint32_t)(uint16_t)at) + (at >> 16);
  return bus_rd16(0x850000 + (uint32_t)(uint16_t)entry);
}

// The high word of start + (end - start) * 2 * frac / 65536 (a DSP-4 multiply whose low
// word the original skips by writing to the chip).
static uint16_t lerp(uint16_t start, uint16_t end, uint16_t frac) {
  uint16_t span = (uint16_t)((uint16_t)(end - start) << 1);
  uint16_t high = (uint16_t)((uint32_t)dsp4_multiply((int16_t)frac, (int16_t)span) >> 16);
  return (uint16_t)(start + high);
}

// TG3000_ROUTINE($82:AB3F) func_82AB3F
uint16_t func_82AB3F(uint16_t x, uint16_t y) {
  uint16_t frac = (uint16_t)((rd16(0xFAB4 + (uint32_t)x) & 0xFF00) >> 1);  // the byte $FAB5,x / 256, as 1.15
  wr16(DP + 0x14, frac);
  uint16_t a = lerp(rd16(0x34EC + (uint32_t)y), rd16(0x34EE + (uint32_t)y), frac);
  wr16(DP + 0x16, (uint16_t)(a - rd16(0xFABB + (uint32_t)x)));
  uint16_t b = lerp(rd16(0x36EC + (uint32_t)y), rd16(0x36EE + (uint32_t)y), frac);
  wr16(DP + 0x18, (uint16_t)(b - 0x0C));
  return rd16(DP + 0x18);
}

// Move `cur` towards `target` by at most `step`; "past the target" is judged by the sign of
// each difference (BMI/BPL), as the original does.
static uint16_t approach(uint16_t cur, uint16_t target, uint16_t step) {
  if((uint16_t)(target - cur) & 0x8000) {
    uint16_t t = (uint16_t)(cur - step);
    return ((uint16_t)(t - target) & 0x8000) ? target : t;
  }
  uint16_t t = (uint16_t)(cur + step);
  return ((uint16_t)(t - target) & 0x8000) ? t : target;
}

// TG3000_ROUTINE($82:A9B7) func_82A9B7
uint16_t func_82A9B7(void) {
  if(rd16(0x0281) & 0x8000) {
    wr16(DP + 0x18, 0xFFD0);
    wr16(DP + 0x16, 0);
  } else {
    func_82AB3F(rd16(0x0281), rd16(0x0283));
    int16_t scale = (int16_t)rd16(0x0293);
    uint16_t h1 = (uint16_t)((uint32_t)dsp4_multiply((int16_t)(rd16(DP + 0x16) << 1), scale) >> 16);
    wr16(DP + 0x16, (uint16_t)-h1);
    uint16_t h2 = (uint16_t)((uint32_t)dsp4_multiply((int16_t)(rd16(DP + 0x18) << 1), scale) >> 16);
    wr16(DP + 0x18, (uint16_t)(h2 - 0x33));
  }
  uint16_t step = (uint16_t)(rd16(0x1E9E) << 2);
  wr16(0x028B, approach(rd16(0x028B), rd16(DP + 0x16), step));
  wr16(0x028D, approach(rd16(0x028D), rd16(DP + 0x18), step));
  return rd16(0x028D);
}

// v unless v - $3000 is non-negative (by sign, as the original's BMI), then $3000.
static uint16_t cap_3000(uint16_t v) { return ((uint16_t)(v - 0x3000) & 0x8000) ? v : 0x3000; }

// TG3000_ROUTINE($82:AC52) func_82AC52
void func_82AC52(void) {
  uint32_t p = (uint32_t)dsp4_multiply((int16_t)rd16(0x0213), 0x0300);
  wr16(DP + 0x14, (uint16_t)((p >> 8) & 0xFF));  // scratch: the product's byte 1
  wr16(0x4E2D, cap_3000((uint16_t)((uint16_t)(p >> 8) - 0x0B00)));  // bits 8-23 of the product
  uint16_t flags = rd16(0x0244);
  if(!(flags & 1)) return;
  // the high word of the 32-bit difference $7E:FAB4/$FAB6 - $0224/$0226, at least 0
  uint32_t a = ((uint32_t)rd16(0xFAB6) << 16) | rd16(0xFAB4), b = ((uint32_t)rd16(0x0226) << 16) | rd16(0x0224);
  uint16_t far = (uint16_t)((a - b) >> 16);
  if(far & 0x8000) far = 0;
  wr16(DP + 0x14, (uint16_t)(far << 1));
  uint16_t near = (uint16_t)(0xB0 - (uint16_t)(far << 1));
  if(near & 0x8000) near = 0;
  wr16(0x4E35, (uint16_t)(near & 0xF8));
  if(!(flags & 4)) return;
  uint16_t w = (uint16_t)(rd16(0x4E2D) + 0x0B00);
  if(w & 0x8000) w = 0;
  uint16_t d = (uint16_t)(rd16(0xFAB6) - rd16(0x0226));
  if(d & 0x8000) d = 0;
  d = (uint16_t)(d + 0x10);
  if(d >= 0xD0) d = 0xD0;
  uint16_t q = (uint16_t)(w / (uint8_t)d);  // the CPU's 16 / 8 divider
  wr16(0x4E2D, cap_3000((uint16_t)((uint16_t)(q << 4) - 0x0B00)));
}

// TG3000_ROUTINE($8A:9A99) byte_buffer_space
void byte_buffer_space(void) { byte_buffer_push(0x20); }

// TG3000_ROUTINE($80:B537) func_80B537
uint16_t func_80B537(uint16_t dp, uint16_t a, uint16_t x, uint16_t y) {
  uint16_t i = rd16(dp + 0x8C);
  wr16(0x1900 + i, a);
  wr16(0x19C0 + i, y);
  wr16(0x1940 + i, x);
  wr16(0x1980 + i, rd16(dp + 0x14));
  uint16_t next = (uint16_t)(((i + 2) & 0x3F) + rd16(dp + 0x8A));
  wr16(dp + 0x8C, next);
  return next;
}

// TG3000_ROUTINE($8A:99B4) byte_buffer_pad
void byte_buffer_pad(uint16_t count) {
  while(rd16(DP + 0x60) != count) byte_buffer_space();
}

// TG3000_ROUTINE($8A:99D6) byte_buffer_end
void byte_buffer_end(void) {
  byte_buffer_push(0x80);
  wr16(DP + 0x60, 0);
}

#define DECIMAL_POWERS 0x8A9D83  // 10000, 1000, 100, 10, 1

// TG3000_ROUTINE($8A:9A33) byte_buffer_decimal
void byte_buffer_decimal(uint16_t value) {
  if(value == 0) {
    byte_buffer_push('0');
    return;
  }
  wr16(DP + 0x5A, 0);  // 0 while only leading zeros have been met
  uint16_t rest = value;
  for(uint16_t i = 0; i != 10; i = (uint16_t)(i + 2)) {
    uint16_t power = rom16(DECIMAL_POWERS + i);
    uint8_t digit = '0';
    for(; rest >= power; rest = (uint16_t)(rest - power)) digit++;
    wr16(DP + 0x5C, rest);
    if(rd16(DP + 0x5A) == 0) {
      if(digit == '0') continue;
      wr16(DP + 0x5A, 1);
    }
    byte_buffer_push(digit);
  }
}

// TG3000_ROUTINE($80:E4C5) func_80E4C5
void func_80E4C5(void) { wr8(DP + 0xFB, 0x11); }

// TG3000_ROUTINE($80:E4DB) func_80E4DB
void func_80E4DB(void) { wr8(DP + 0xFB, 0x13); }

// TG3000_ROUTINE($80:B58D) func_80B58D
void func_80B58D(uint8_t a, uint16_t x) {
  wr16(DP + 0xCC, x);
  wr8(DP + 0xCE, a);
  wr16(DP + 0xE3, 0);
  wr16(DP + 0xE1, 9);
}

// TG3000_ROUTINE($8A:BFBC) func_8ABFBC
uint16_t func_8ABFBC(uint16_t low, bool* carry) {
  uint16_t high = rd16(DP + 0x46), mid = rd16(DP + 0x44);
  for(int i = 0; i < 4; i++) {  // LSR high, ROR mid, ROR low
    low = (uint16_t)(low >> 1 | (mid & 1) << 15);
    mid = (uint16_t)(mid >> 1 | (high & 1) << 15);
    high >>= 1;
  }
  wr16(DP + 0x46, high);
  wr16(DP + 0x44, mid);
  return lsr_n(low, 12, carry);
}

#define BCD_WEIGHTS 0x8ACABA  // 1, 10, 100, 1000, 10000

// TG3000_ROUTINE($8A:BF9E) bcd_to_binary
uint16_t bcd_to_binary(uint16_t a) {
  uint16_t sum = 0;
  wr16(DP + 0x42, 0);
  for(uint16_t i = 0; i != 10; i = (uint16_t)(i + 2)) {
    bool carry;
    uint16_t digit = func_8ABFBC(a, &carry);
    a = digit;
    for(; digit != 0; digit--) {
      sum = (uint16_t)(sum + rom16(BCD_WEIGHTS + i));
      wr16(DP + 0x42, sum);
      a = sum;
    }
  }
  return a;
}

// TG3000_ROUTINE($80:899F) func_80899F
uint16_t func_80899F(uint8_t b, uint16_t x) {
  if(b == 0) return x;
  wr8(0x1B30 + x, b);
  return (uint16_t)((x + 1) & 7);
}

// TG3000_ROUTINE($8A:A022) func_8AA022
uint16_t func_8AA022(uint16_t a) {
  uint16_t bit = (uint16_t)((a ^ 1) & 1);
  wr16(DP + 0x56, (uint16_t)(rd16(DP + 0x56) << 1 | bit));
  uint16_t sum = (uint16_t)(rd16(DP + 0x58) + bit);
  wr16(DP + 0x58, sum);
  return sum;
}

// TG3000_ROUTINE($80:B7BB) func_80B7BB
void func_80B7BB(void) {
  uint16_t v = rd16(DP + 0xDD);
  for(uint16_t x = 14; x != 0; x = (uint16_t)(x - 2))
    if(rd16(DP + 0xCD + x) == v) wr16(DP + 0xCD + x, 0xFFFF);
}

// TG3000_ROUTINE($8A:9A81) byte_buffer_decimal2
void byte_buffer_decimal2(void) {
  uint16_t v = rd16(DP + 0x5E);
  if((uint16_t)(v - 10) & 0x8000) byte_buffer_push(0x20);
  byte_buffer_decimal(v);
}

// TG3000_ROUTINE($80:894E) func_80894E
void func_80894E(void) {
  static const uint16_t sources[4] = {0x01FB, 0x01FC, 0x02FB, 0x02FC};
  uint16_t w = rd16(0x1B24);
  for(int i = 0; i < 4; i++) w = func_80899F(rd8(sources[i]), w);
  wr16(0x1B24, w);
  for(int i = 0; i < 4; i++) wr8(sources[i], 0);
  uint16_t r = rd16(0x1B26);
  if(w == r) return;
  wr16(0x0466, 0);
  wr16(0x0465, 0xFF00);
  wr16(0x0463, (uint16_t)(rd16(0x1B30 + r) | 0xFF00));
  wr16(0x1B26, (uint16_t)((r + 1) & 7));
}

// TG3000_ROUTINE($80:86C9) binary_to_bcd
uint32_t binary_to_bcd(uint16_t value) {
  uint32_t bcd = 0;
  for(int i = 0; i < 8; i++) {
    bcd = bcd >> 4 | (uint32_t)(value % 10) << 28;
    value = (uint16_t)(value / 10);
  }
  wr16(DP + 0x16, (uint16_t)bcd);
  wr16(DP + 0x18, (uint16_t)(bcd >> 16));
  return bcd;
}

// TG3000_ROUTINE($8A:A005) func_8AA005
uint16_t func_8AA005(void) {
  wr16(DP + 0x56, 0);
  wr16(DP + 0x58, 0);
  func_8AA022(rd16(0x0018));
  func_8AA022(rd16(0x0014));
  func_8AA022(rd16(0x0010));
  return func_8AA022(rd16(0x000C));
}

// TG3000_ROUTINE($80:E0C8) func_80E0C8
uint16_t func_80E0C8(uint16_t a) {
  uint16_t d = (uint16_t)(a << 1);
  return (uint16_t)(d < 0x40 ? d + 0xB990 : d + 0x2400);
}

// TG3000_ROUTINE($80:83E8) oam_clear
void oam_clear(void) {
  for(uint16_t x = 0x0200; x != 0; x = (uint16_t)(x - 2)) wr16(0x08FE + x, 0xE0E0);
  wr16(0x0172, 0);
  wr16(0x0174, 0);
}

// TG3000_ROUTINE($80:FBD8) oam_clear_long
void oam_clear_long(void) { oam_clear(); }

// TG3000_ROUTINE($80:8FFC) func_808FFC
uint16_t func_808FFC(void) {
  uint16_t x = rd16(DP + 0x74), count = rd16(DP + 0xEE);
  if(count != 0) wr16(DP + 0xEE, --count);
  // absolute,X past $FFFF carries into DB + 1, whose low 8 KB is the same WRAM: 16 bits
  wr16((uint16_t)(0x0902 + x), 0x3892);
  wr16((uint16_t)(0x0900 + x), (uint16_t)(0x085A + count + rd16(DP + 0xE7)));
  wr16((uint16_t)(0x0B01 + x), 0x0100);
  wr16((uint16_t)(0x0906 + x), 0x3890);
  wr16((uint16_t)(0x0904 + x), (uint16_t)(0x0A5C + rd16(DP + 0xE7)));
  wr16((uint16_t)(0x0B05 + x), 0x0100);
  wr16(DP + 0x74, (uint16_t)(x + 8));
  return (uint16_t)(x + 8);
}

// TG3000_ROUTINE($8A:A4BE) func_8AA4BE
void func_8AA4BE(uint32_t table) {
  uint32_t bank = table & 0xFF0000;
  wr16(DP + 0x4A, (uint16_t)table);
  for(uint16_t y = 0;; y = (uint16_t)(y + 2)) {
    // (dp),Y through D+$4A, re-read each time as the original does; the index may carry
    // into the next bank
    uint16_t p = bus_rd16((bank | rd16(DP + 0x4A)) + y);
    if(p == 0) return;
    func_8AB680(bank | (uint16_t)(p + 4));
  }
}

// TG3000_ROUTINE($8A:8232) func_8A8232
uint16_t func_8A8232(void) {
  uint16_t v = (uint16_t)((rd16(DP + 0x36) >> 2) + 0x0108);
  wr16(DP + 0x4A, v);
  return v;
}

// TG3000_ROUTINE($82:AD12) func_82AD12
void func_82AD12(uint16_t a, uint16_t x) {
  if(rd16(0x047C) & 0x8000) return;
  wr16(0x0466, 0);
  wr16(0x0465, x);
  wr16(0x0463, a);
}

// TG3000_ROUTINE($8A:BFD0) binary_to_bcd5
void binary_to_bcd5(void) {
  uint16_t v = rd16(DP + 0x42), lo = 0, hi = 0;
  for(int x = 8; x >= 0; x -= 2) {
    uint16_t w = rom16(BCD_WEIGHTS + (uint32_t)x);
    for(; v >= w; v = (uint16_t)(v - w)) lo++;
    for(int i = 0; i < 4; i++) {  // ASL lo, ROL hi
      hi = (uint16_t)(hi << 1 | lo >> 15);
      lo = (uint16_t)(lo << 1);
    }
  }
  for(int i = 0; i < 4; i++) {  // LSR hi, ROR lo
    lo = (uint16_t)(lo >> 1 | (hi & 1) << 15);
    hi >>= 1;
  }
  wr16(DP + 0x42, v);
  wr16(DP + 0x44, lo);
  wr16(DP + 0x46, hi);
}

// TG3000_ROUTINE($82:F380) func_82F380
void func_82F380(void) {}

// TG3000_ROUTINE($82:FB1C) func_82FB1C
void func_82FB1C(void) {}

static bool shift32(uint16_t lo_adr, uint16_t hi_adr, bool left) {
  uint16_t lo = rd16(lo_adr), hi = rd16(hi_adr);
  bool out = false;
  for(int i = 0; i < 12; i++) {
    if(left) {  // ASL lo, ROL hi
      out = hi >> 15;
      hi = (uint16_t)(hi << 1 | lo >> 15);
      lo = (uint16_t)(lo << 1);
    } else {    // LSR hi, ROR lo
      out = lo & 1;
      lo = (uint16_t)(lo >> 1 | (hi & 1) << 15);
      hi >>= 1;
    }
  }
  wr16(lo_adr, lo);
  wr16(hi_adr, hi);
  return out;
}

// TG3000_ROUTINE($8A:C03D) func_8AC03D
bool func_8AC03D(void) {
  shift32(0x038E, 0x0390, true);
  return shift32(0x03BD, 0x03BF, true);
}

// TG3000_ROUTINE($8A:C024) func_8AC024
bool func_8AC024(void) {
  shift32(0x038E, 0x0390, false);
  return shift32(0x03BD, 0x03BF, false);
}

// TG3000_ROUTINE($8A:A32B) func_8AA32B
//
// The bit-7 path ($8A:A345) runs in no recorded movie; it is checked by the fuzz.
uint8_t func_8AA32B(uint16_t x) {
  uint8_t i = rom8(0x8AC599 + (uint8_t)(rd8(DP + 0x3C) - 10));
  uint8_t flags = rd8((uint16_t)(0x1516 + x)), v;
  if(flags & 0x80) {
    v = 0xE0;
  } else if(!(flags & 0x40)) {
    v = 0xE7;
  } else {
    uint8_t k = rom8(0x8AC5B7 + (rd8(DP + 0x6E) & 0x1F));
    v = (uint8_t)((rd8(0x0D23 + i) & 0xE0) | k);
    wr8(DP + 0x6E, (uint8_t)(rd8(DP + 0x6E) + 1));
  }
  wr8(0x0D23 + i, v);
  return v;
}

static void object_flags_41(uint16_t x, uint16_t set_at) {
  wr16((uint16_t)(0x1516 + x), (uint16_t)(rd16((uint16_t)(0x1516 + x)) & 0xFFBE));
  wr16((uint16_t)(set_at + x), (uint16_t)(rd16((uint16_t)(set_at + x)) | 0x0041));
  wr16(DP + 0x64, 0);
  wr16(DP + 0x66, 0);
  wr16(DP + 0x6E, 0);
  func_8AA48D();
}

// TG3000_ROUTINE($8A:A381) func_8AA381
//
// The bit-11 path ($8A:A3C2) runs in no recorded movie; it is checked by the fuzz. The
// original lowers D+$4A by 2 to read the word before it and puts it back afterwards.
int func_8AA381(uint16_t x) {
  static const uint16_t words[4] = {0x000E, 0x0012, 0x0016, 0x001A};
  for(int bit = 11; bit >= 10; bit--) {
    uint16_t mask = (uint16_t)(1u << bit);
    for(int i = 0; i < 4; i++) {
      uint16_t w = rd16(words[i]);
      if(!(w & mask)) continue;
      wr16(words[i], (uint16_t)(w & ~mask));
      uint16_t p = rd16(DP + 0x4A);
      if(bit == 11) {
        if(bus_rd16(0x8A0000 | (uint16_t)(p - 2)) != 0xFFFF) object_flags_41(x, 0x1514);
      } else {
        if(bus_rd16((0x8A0000 | p) + 8) != 0) object_flags_41(x, 0x1518);
      }
      return bit;
    }
  }
  return 0;
}

// TG3000_ROUTINE($8A:8290) func_8A8290
//
// Everything past the first test ($8A:8295) runs in no recorded movie; it is checked by
// the fuzz.
uint16_t func_8A8290(void) {
  if(rd16(DP + 0x54) == 0) return 0;
  uint16_t count = (uint16_t)(rd16(DP + 0x52) + 1);
  wr16(DP + 0x52, count);
  if(count & 7) return (uint16_t)(count & 7);
  uint16_t x = rd16(DP + 0x50);
  uint16_t v = bus_rd16(0x8A82B4 + (uint32_t)x);  // LDA long,X: the index may carry into $8B
  wr16(DP + 0x64, v);
  x = (uint16_t)(x + 2);
  wr16(DP + 0x50, x == 12 ? 0 : x);
  return v;
}

// TG3000_ROUTINE($85:CDAF) func_85CDAF (long)
bool func_85CDAF(uint16_t y) {
  wr16(DP + 0x14, y);
  uint16_t a = rd16((uint16_t)(0xF533 + y)), b = rd16((uint16_t)(0xF733 + y));
  wr16(DP + 0x16, a);
  wr16(DP + 0x18, b);
  uint16_t i = rd16(0x00FE);
  if(i == y) return false;
  do {
    if(rd16((uint16_t)(0xF733 + i)) == b && rd16((uint16_t)(0xF533 + i)) == a)
      wr16((uint16_t)(0xF733 + i), (uint16_t)(rd16((uint16_t)(0xF733 + i)) | 0x8000));
    i = (uint16_t)((i + 2) & 0xFF);
  } while(i != y);
  return true;
}

// TG3000_ROUTINE($8A:869A) func_8A869A
uint16_t func_8A869A(void) {
  uint16_t v = (uint16_t)(0x2A - (rd16(DP + 0x3C) >> 4));
  wr16(DP + 0x3A, v);
  return v;
}

// TG3000_ROUTINE($8A:823D) func_8A823D
uint16_t func_8A823D(void) {
  uint16_t v = (uint16_t)((rd16(DP + 0x38) >> 2) + 0xFFB7 + rd16(DP + 0x64));
  wr16(DP + 0x4C, v);
  return v;
}

// TG3000_ROUTINE($8A:8325) func_8A8325
uint16_t func_8A8325(void) {
  wr16(0x0600, (uint16_t)((rd16(DP + 0x3C) >> 2) - 0xCA));
  uint16_t v = (uint16_t)((rd16(DP + 0x3E) >> 2) + 0x1B);
  wr16(0x0700, v);
  return v;
}

// TG3000_ROUTINE($80:B556) func_80B556
void func_80B556(uint8_t a, uint16_t x, uint16_t y) {
  wr16(DP + 0xCC, x);
  wr8(DP + 0xCE, a);
  wr16(DP + 0xC8, y);
  wr16(DP + 0xCA, 0);
  for(uint16_t i = 0xCF; i <= 0xDD; i = (uint16_t)(i + 2)) wr16(DP + i, 0xFFFF);
  wr16(DP + 0xC6, 0);
  wr16(DP + 0xE3, 0);
  wr16(DP + 0xE5, 0x0100);
  wr16(DP + 0xE1, 9);
  wr16(DP + 0xF6, 0);
  wr16(DP + 0xF8, 0);
  wr16(DP + 0xF9, 0x0019);
}

// TG3000_ROUTINE($8A:BF76) func_8ABF76
bool func_8ABF76(void) {
  static const uint16_t words[2] = {0x038E, 0x03BD};
  for(int i = 0; i < 2; i++) {
    wr16(DP + 0x42, rd16(words[i]));
    binary_to_bcd5();
    wr16(words[i], rd16(DP + 0x44));
    wr16((uint16_t)(words[i] + 2), rd16(DP + 0x46));
  }
  return func_8AC03D();
}

// TG3000_ROUTINE($8A:BF4E) func_8ABF4E
uint16_t func_8ABF4E(void) {
  static const uint16_t words[2] = {0x038E, 0x03BD};
  func_8AC024();
  uint16_t a = 0;
  for(int i = 0; i < 2; i++) {
    wr16(DP + 0x44, rd16(words[i]));
    wr16(DP + 0x46, rd16((uint16_t)(words[i] + 2)));
    bcd_to_binary(rd16((uint16_t)(words[i] + 2)));
    a = rd16(DP + 0x42);
    wr16(words[i], a);
  }
  return a;
}

// TG3000_ROUTINE($8A:9202) func_8A9202
uint16_t func_8A9202(uint16_t x) {
  static const uint16_t tiles[10] = {0x04AA, 0x04AC, 0x04AE, 0x04C0, 0x04C2, 0x04C4, 0x04C6, 0x04C8, 0x04CA, 0x04CC};
  uint16_t base = rd16(DP + 0x7A), y = rd16(DP + 0x7C), a = base;
  uint32_t carry = 0;
  for(int k = 0; k < 10; k++) {
    if(k == 5) a = base;  // the second row restarts from D+$7A, keeping the carry
    wr16((uint16_t)(0x06C0 + x + 2 * k), a);
    if(k != 4 && k != 9) {
      uint32_t sum = (uint32_t)a + 0x10 + carry;
      carry = sum >> 16;
      a = (uint16_t)sum;
    }
  }
  for(int k = 0; k < 10; k++) wr16((uint16_t)(0x07C0 + x + 2 * k), k < 5 ? y : (uint16_t)(y + 0x10));
  for(int k = 0; k < 10; k++) wr16((uint16_t)(0x05C0 + x + 2 * k), 1);
  for(int k = 0; k < 10; k++) wr16((uint16_t)(0x08C0 + x + 2 * k), tiles[k]);
  return (uint16_t)(x + 0x14);
}

// TG3000_ROUTINE($8A:8000) func_8A8000
void func_8A8000(void) {}

// TG3000_ROUTINE($8A:987F) func_8A987F
void func_8A987F(void) {}

static uint16_t divide16by8(uint16_t dividend, uint8_t divisor) {  // the CPU divider
  return divisor ? (uint16_t)(dividend / divisor) : 0xFFFF;
}

// TG3000_ROUTINE($80:D6F3) func_80D6F3
//
// Its four negations ($80:D700, $80:D712, $80:D767, $80:D773) run in no recorded movie;
// they are checked by the fuzz.
uint16_t func_80D6F3(uint16_t target) {
  wr16(DP + 0x26, target);
  uint16_t dx = (uint16_t)(((target & 0xFF) << 8) - rd16(DP + 0x34));
  wr16(DP + 0x16, dx);
  uint16_t ax = (dx & 0x8000) ? (uint16_t)-dx : dx;
  wr16(DP + 0x14, ax);
  uint16_t dy = (uint16_t)((target & 0xFF00) - rd16(DP + 0x36));
  wr16(DP + 0x18, dy);
  uint16_t ay = (dy & 0x8000) ? (uint16_t)-dy : dy;
  if(!((uint16_t)(ay - ax) & 0x8000)) {  // CMP + BMI: the sign of the difference
    uint8_t d = (uint8_t)(ay >> 8);
    wr16(DP + 0x26, d);
    wr16(DP + 0x38, divide16by8(ax & 0xFF00, d));
    wr16(DP + 0x3A, 0x0100);
  } else {
    uint8_t d = (uint8_t)(ax >> 8);
    wr16(DP + 0x26, d);
    wr16(DP + 0x3A, divide16by8(ay, d));
    wr16(DP + 0x38, 0x0100);
  }
  if(dx & 0x8000) wr16(DP + 0x38, (uint16_t)-rd16(DP + 0x38));
  uint16_t a = dy;
  if(dy & 0x8000) {
    a = (uint16_t)-rd16(DP + 0x3A);
    wr16(DP + 0x3A, a);
  }
  wr8(DP + 0x34, (uint8_t)a);
  wr8(DP + 0x36, (uint8_t)a);
  return a;
}

// TG3000_ROUTINE($8A:A050) func_8AA050 (long)
uint16_t func_8AA050(void) {
  if(rd16(DP + 0x8E) != 0) {
    upload_7f_to_3000();
    oam_from_objects();
    uint16_t n = (uint16_t)(rd16(0x0164) + 1);
    wr16(0x0164, n);
    if((n & 7) == 0) wr16(0x0166, (uint16_t)(rd16(0x0166) + 1));
  }
  return func_8AA005();
}

// TG3000_ROUTINE($8A:BAF2) fade_in_start
void fade_in_start(void) {
  wr16(0x0198, 0x0F00);
  wr16(0x0196, 0x0080);
}

// TG3000_ROUTINE($8A:A09F) hdma_window_tables_init
void hdma_window_tables_init(void) {
  wr8(0x0D00, 0x4C);
  for(uint16_t a = 0x0D03; a <= 0x0D15; a += 3) wr8(a, 0x18);
  wr8(0x0D18, 0x00);
  wr8(0x0D20, 0x4C);
  for(uint16_t a = 0x0D22; a <= 0x0D2E; a += 2) wr8(a, 0x18);
  wr8(0x0D30, 0x00);
  for(uint16_t a = 0x0D23; a <= 0x0D2F; a += 2) wr8(a, 0xE0);
}

// TG3000_ROUTINE($8A:BC8A) password_encode
//
// The first shift's carry in is the borrow of the count division (bits / 5 by
// subtraction), the next ones what the previous shift left.
uint16_t password_encode(uint16_t bits, uint16_t* x, uint16_t* y) {
  uint16_t chars = 0, rest = bits;
  bool carry;
  do {
    chars++;
    carry = rest >= 5;
    rest = (uint16_t)(rest - 5);
  } while(!(rest & 0x8000));
  wr16(DP + 0x42, (uint16_t)(chars - 1));
  uint16_t at = 0;
  uint16_t left;
  do {
    uint16_t index = func_8ABB5C(5, &carry);
    wr8((uint32_t)(0x0D40 + at), rom8(0x8ABC5F + index));
    at++;
    left = (uint16_t)(rd16(DP + 0x42) - 1);
    wr16(DP + 0x42, left);
  } while(!(left & 0x8000));
  wr8((uint32_t)(0x0D40 + at), 0);
  wr8((uint32_t)(0x0D41 + at), 0);
  uint16_t index = rd16(DP + 0x92) & 0x001F;
  wr16((uint32_t)(0x0D40 + at), (uint16_t)(rom8(0x8ABC5F + index) | rd16((uint32_t)(0x0D40 + at))));
  at++;
  index = (uint16_t)((rd16(DP + 0x92) & 0x03E0) >> 5);
  uint16_t last = (uint16_t)(rom8(0x8ABC5F + index) | rd16((uint32_t)(0x0D40 + at)));
  wr16((uint32_t)(0x0D40 + at), last);
  at++;
  wr16((uint32_t)(0x0D40 + at), 0);
  *x = at;
  *y = index;
  return last;
}


static void fade_out_start(void) {
  wr16(0x0198, 0);
  wr16(0x0196, 0xFF80);
}

// TG3000_ROUTINE($8A:BAE3) fade_out_wait
void fade_out_wait(void) {
  fade_out_start();
  while(rd16(0x0196) != 0) platform_wait_vblank();
}

// a new A, B, X or Y press of the pad pair at `pressed`, taken
static bool take_confirm(uint32_t pressed) {
  uint16_t bits = rd16(pressed);
  wr16(pressed, (uint16_t)(bits & ~0xC0C0));
  return (bits & 0xC0C0) != 0;
}

// TG3000_ROUTINE($8A:9675) fade_out_on_confirm
bool fade_out_on_confirm(void) {
  if(!take_confirm(PAD_BASE + 2) && !take_confirm(PAD_BASE + 6)) return false;
  fade_out_start();
  return true;
}

// TG3000_ROUTINE($8A:8181) fade_out_on_confirm_once
void fade_out_on_confirm_once(void) {
  if(rd16(DP + 0x8C) != 0) return;
  if(!take_confirm(PAD_BASE + 2) && !take_confirm(PAD_BASE + 6)) return;
  fade_out_start();
  wr16(DP + 0x8C, (uint16_t)(rd16(DP + 0x8C) + 1));
}

// TG3000_ROUTINE($8A:99D0) func_8A99D0
void func_8A99D0(void) { wr16(DP + 0x66, 0x0028); }

// TG3000_ROUTINE($8A:96C2) func_8A96C2
uint16_t func_8A96C2(void) {
  wr16(DP + 0x40, (uint16_t)(rd16(DP + 0x40) + 1));
  return rd16(DP + 0x40);
}

// TG3000_ROUTINE($8A:9C86) hdma_table_0d00_init
void hdma_table_0d00_init(void) {
  wr8(0x0D00, 0x36);
  wr8(0x0D01, 0xFF);
  uint32_t x = 0;
  for(int n = 0; n < 0x15; n++, x += 2) {
    wr8(0x0D02 + x, 0x08);
    wr8(0x0D03 + x, 0xFF);
  }
  wr8(0x0D02 + x, 0);
  wr8(0x0D03 + x, 0);
}

// TG3000_ROUTINE($8A:BF2A) string_space_fours
void string_space_fours(void) {
  uint32_t x = 0, y = 0;
  uint8_t c;
  do {
    c = rd8(0x0D40 + x);
    wr8(0x0D60 + y, c);
    x = (uint16_t)(x + 1);
    y = (uint16_t)(y + 1);
    if((x & 3) == 0) {
      wr8(0x0D60 + y, 0x20);
      y = (uint16_t)(y + 1);
    }
  } while(c != 0);
}

// TG3000_ROUTINE($8A:8AE7) table_062a_fill
void table_062a_fill(uint16_t x) {
  uint16_t first = rd16(DP + 0x3C);
  for(int n = 0; n < 12; n++, x = (uint16_t)(x + 2)) {
    wr16(0x062A + (uint32_t)x, rd16(DP + 0x3C));
    wr16(0x072A + (uint32_t)x, rd16(DP + 0x3E));
    wr16(DP + 0x3C, (uint16_t)(rd16(DP + 0x3C) + 0x10));
  }
  wr16(DP + 0x3C, first);
}

// TG3000_ROUTINE($8A:BB99) password_pack
uint16_t password_pack(void) {
  func_8ABC7F();
  wr16(DP + 0x48, 0);
  wr16(DP + 0x92, 0);
  wr16(DP + 0x84, 0);
  for(uint32_t x = 0;; x += 6) {
    uint16_t at = rom16(0x8ACF61 + x);
    if(at == 0) break;
    wr16(DP + 0x42, at);
    wr16(DP + 0x44, rom16(0x8ACF63 + x));
    uint16_t shape = rom16(0x8ACF65 + x), width = (uint16_t)(shape >> 8);
    uint32_t field = ((uint32_t)rd8(DP + 0x44) << 16 | rd16(DP + 0x42));
    uint16_t value = bus_rd16(field);
    bool carry;
    value = lsr_n(value, (uint16_t)(shape & 0x00FF), &carry);
    func_8ABBF4((uint16_t)(value ^ rom16(0x8ABC3D + (uint32_t)width)), width);
  }
  return rd16(DP + 0x84);
}

// TG3000_ROUTINE($8A:9C27) hdma_table_0d00_step
void hdma_table_0d00_step(void) {
  wr16(DP + 0x6C, (uint16_t)(rd16(DP + 0x6C) + 1));
  if(rd16(DP + 0x6C) & 0x0003) return;
  uint16_t x = rd16(DP + 0x6E);
  if(x == 0x002A) return;
  if(rd16(0x0D01 + (uint32_t)x) != 0x00E0) wr16(0x0D01 + (uint32_t)x, (uint16_t)(rd16(0x0D01 + (uint32_t)x) - 1));
  wr16(DP + 0x6E, (uint16_t)(x + 2));
}

#define RACE_ORDER 0x4CFD  // $7E: 20 car indices (words)

static uint64_t race_total(uint16_t car) {
  return (uint64_t)rd16(0x69E1 + (uint32_t)car) << 32 | (uint64_t)rd16(0x69B9 + (uint32_t)car) << 16 |
         rd16(0x6991 + (uint32_t)car);
}

// TG3000_ROUTINE($85:A066) race_order_sort (long)
uint16_t race_order_sort(uint16_t* x, uint16_t* y) {
  for(uint32_t i = 0; i < 20; i++) wr16(RACE_ORDER + 2 * i, (uint16_t)(2 * i));
  for(wr16(DP + 0x14, 0x0026); rd16(DP + 0x14) != 0; wr16(DP + 0x14, (uint16_t)(rd16(DP + 0x14) - 2))) {
    for(wr16(DP + 0x16, 0); rd16(DP + 0x16) != rd16(DP + 0x14); wr16(DP + 0x16, (uint16_t)(rd16(DP + 0x16) + 2))) {
      const uint32_t at = RACE_ORDER + rd16(DP + 0x16);
      uint16_t a = rd16(at), b = rd16(at + 2);
      if(race_total(a) < race_total(b)) {  // the higher total first
        wr16(at, b);
        wr16(at + 2, a);
      }
    }
  }
  for(uint32_t i = 0; i < 20; i++) {
    uint16_t car = rd16(RACE_ORDER + 2 * i);
    if(rd16(0x6A09 + (uint32_t)car) & 0x0004) continue;
    uint16_t n = rd16(0x6A31);
    wr8(0x0446 + (uint32_t)n, (uint8_t)(car >> 1));
    wr16(0x6A31, (uint16_t)(n + 1));
  }
  wr16(0x03A8, 0xFFFF);
  wr16(0x03D7, 0xFFFF);
  uint16_t place = 0, left = 10, both;
  do {
    uint8_t car = rd8(0x0446 + (uint32_t)place);
    if(car == 0) wr16(0x03A8, place);
    else if(car == 1) wr16(0x03D7, place);
    place++;
    both = (uint16_t)(rd16(0x03A8) | rd16(0x03D7));
    if(!(both & 0x8000)) break;
  } while(--left);
  *x = place;
  *y = left;
  return both;
}

// TG3000_ROUTINE($8A:9C48) hdma_table_0d00_settle
void hdma_table_0d00_settle(void) {
  hdma_table_0d00_step();
  for(uint32_t x = 0; x < 0x2A; x += 2) {
    uint8_t b = rd8(0x0D01 + x);
    if(b != 0xFF && b != 0xE0) wr8(0x0D01 + x, (uint8_t)(b - 1));
  }
}

// TG3000_ROUTINE($80:D6A0) steer_state_init
void steer_state_init(void) {
  static const uint8_t cleared[] = {0x28, 0x34, 0x36, 0x24, 0x1E, 0x2C};
  for(unsigned i = 0; i < sizeof cleared; i++) wr16(DP + cleared[i], 0);
  wr16(DP + 0x30, 0x0028);
  wr16(DP + 0x32, 0x04B0);
  wr16(DP + 0x38, 0);
  wr16(DP + 0x3A, 0);
  wr16(DP + 0x3C, 0x0050);
  uint16_t at = rd16(0x0804);
  wr16(DP + 0x2A, at);
  wr16(DP + 0x2E, at);
  uint16_t a = at < 0x0080 ? 0 : (uint16_t)(at - 0x0080);
  if(a >= 0x0100) a = 0x0100;  // never run
  a = (uint16_t)(a - a % 3);   // the CPU's 16 / 8 divider's remainder
  wr16(DP + 0x22, a);
  wr16(DP + 0x20, a);
}

// A channel of the gradient (red, green, blue) keeps five words at $1516 + 10 * channel:
// its value before and now (8.8), its first and second steps, its dither pattern.
static uint16_t gradient_value(uint16_t colour, int channel) {
  switch(channel) {
  case 0: return (uint16_t)((colour & 0x001F) << 8);
  case 1: return (uint16_t)((colour & 0x03E0) << 3);
  default: return (uint16_t)((colour & 0x7C00) >> 2);
  }
}

// a step: 2 * the difference of two 8.8 values whose low bytes are 0 (its high byte)
static uint16_t gradient_step(uint16_t to, uint16_t from) {
  uint16_t d = (uint16_t)(to - from);
  if(d & 0x8000) d |= 0x00FF;
  d = (uint16_t)((d << 8) | (d >> 8));  // XBA
  return (uint16_t)(d << 1);
}

// TG3000_ROUTINE($8A:8FA4) hdma_gradient_build (long)
void hdma_gradient_build(void) {
  for(int c = 0; c < 3; c++) {
    const uint32_t f = 0x1516 + 10 * (uint32_t)c;  // before, now, step 1, step 2, dither
    uint16_t from = gradient_value(rd16(DP + 0x34), c), mid = gradient_value(rd16(DP + 0x36), c),
             to = gradient_value(rd16(DP + 0x38), c);
    wr16(f, from);
    wr16(f + 2, from);
    wr16(f + 8, 0);
    wr16(f + 6, gradient_step(to, mid));
    wr16(f + 4, gradient_step(mid, from));
  }
  for(int half = 0; half < 2; half++) {
    uint16_t pattern = rd16(DP + (half ? 0x3C : 0x3A));
    for(int line = 0; line < 0x80; line++) {
      uint16_t colour = 0;
      for(int c = 0; c < 3; c++) {
        const uint32_t f = 0x1516 + 10 * (uint32_t)c;
        uint16_t dither = rd16(f + 8);
        wr16(f + 8, (uint16_t)(dither >> 1));
        uint16_t v = (uint16_t)((dither & 1 ? rd16(f) : rd16(f + 2)) & 0x1F00);
        colour |= c == 0 ? (uint16_t)(v >> 8) : c == 1 ? (uint16_t)(v >> 3) : (uint16_t)(v << 2);
      }
      wr16(DP + 0x3E, colour);
      uint32_t table = (uint32_t)rd8(DP + 0x6A) << 16 | rd16(DP + 0x68);
      uint16_t y = rd16(DP + 0x6C);
      bus_wr16((table + y) & 0xFFFFFF, 0);
      bus_wr16((table + (uint16_t)(y + 2)) & 0xFFFFFF, colour);
      wr16(DP + 0x6C, (uint16_t)(y + 4));
      for(int c = 0; c < 3; c++) {
        const uint32_t f = 0x1516 + 10 * (uint32_t)c;
        uint16_t now = rd16(f + 2), next = (uint16_t)(now + rd16(f + (half ? 6 : 4)));
        if((now & 0xFF00) != (next & 0xFF00)) {  // a whole value crossed: dither the two
          wr16(f + 8, pattern);
          wr16(f, now);
        }
        wr16(f + 2, next);
      }
    }
  }
  wr16(DP + 0x70, 0);
}

// TG3000_ROUTINE($8A:9F64) flag_03dc_from_52
void flag_03dc_from_52(void) {
  if(rd16(DP + 0x52) != 0) wr16(0x03DC, (uint16_t)(rd16(0x03DC) | 0x1000));  // never run
  else wr16(0x03DC, (uint16_t)(rd16(0x03DC) & ~0x1000));
}

// TG3000_ROUTINE($8A:9F76) timer_0d7e_tick
uint16_t timer_0d7e_tick(uint16_t a) {
  wr16(0x0D7E, (uint16_t)(rd16(0x0D7E) - 1));
  if(rd16(0x0D7E) != 0) return a;
  // never run: its random pick goes through a third copy of rng_next ($8A:AE9B), also
  // never run
  wr16(0x03DC, 0x2000);
  bool carry;
  uint16_t pick = (uint16_t)(rng_next(RNG_STATE, &carry) & 0x0003);
  if(pick == 3) pick = 2;
  wr16(0x03DC, (uint16_t)(pick | rd16(0x03DC)));
  wr16(DP + 0x6C, (uint16_t)(rd16(DP + 0x6C) + 1));
  return (uint16_t)(rd16(0x03DC) & 0x0007);
}

// TG3000_ROUTINE($8A:9B85) text_line_put
uint16_t text_line_put(uint16_t at, uint16_t bank) {
  wr16(DP + 0x62, at);
  wr16(DP + 0x64, bank);
  const uint32_t src = (uint32_t)(bank & 0xFF) << 16 | at;
  uint16_t x = rd16(DP + 0x54), n = 0;
  for(;;) {
    uint8_t c = bus_rd8((src + n) & 0xFFFFFF);
    wr8((0x17918 + (uint32_t)(uint16_t)(x + n)) & 0x1FFFF, c);
    if(c == 0) break;
    n++;
  }
  func_8A9BAA();
  return n;
}

// TG3000_ROUTINE($82:ECEA) race_clear_0242_0208
void race_clear_0242_0208(void) {
  wr16(0x0242, 0);
  wr16(0x0208, 0);
}

// a countdown step passed: its value had reached `mark` (signed) and now has not
static bool countdown_passed(uint16_t before, uint16_t after, uint16_t mark) {
  return !((uint16_t)(before - mark) & 0x8000) && ((uint16_t)(after - mark) & 0x8000);
}

// TG3000_ROUTINE($82:AB9D) race_countdown
void race_countdown(void) {
  uint16_t count = rd16(0x1ECE);
  if(count & 0x8000) {
    if(rd16(0x047C) & 0x8000) return;
    wr16(0x0233, 0x003D);
    wr16(0x02E6, 0x003E);
    if((rd16(0x03DC) & 0x00F0) != 0x0030) wr16(0x02E6, 0x0045);
    return;
  }
  if(count == 0) return;
  uint16_t left = (uint16_t)(count - rd16(0x1E9E));
  if(left & 0x8000) left = 0;
  wr16(0x1ECE, left);
  wr16(DP + 0x14, 0x001E);
  uint16_t light = 0;
  if(countdown_passed(count, left, 0x00D2)) light = 2;
  if(countdown_passed(count, left, 0x008C)) light = 3;
  if(countdown_passed(count, left, 0x0046)) light = 4;
  if(countdown_passed(count, left, 0x0002)) {  // GO
    wr16(DP + 0x14, 0x0022);
    wr16(0x1ECE, 1);
    static const uint16_t flags[] = {0x0244, 0x02F7, 0x02A6, 0x0359};
    for(int i = 0; i < 4; i++) wr16(flags[i], (uint16_t)(rd16(flags[i]) | 0x0008));
    light = 5;
  }
  wr16(0x0233, light);
  wr16(0x02E6, light);
  wr16(0x0295, light);
  wr16(0x0348, light);
  if(light == 0) return;
  uint16_t volume = rd16(DP + 0x14);
  func_82AD12((uint16_t)(volume | 0xFF00), (uint16_t)(0x2000 | (volume >> 8)));
}

// TG3000_ROUTINE($82:ECF1) race_scroll_commit
uint16_t race_scroll_commit(void) {
  wr16(0x1EEE, 0);
  wr16(0x1EF0, 0);
  uint32_t total = ((uint32_t)rd16(0x1EF6) << 16 | rd16(0x1EF4)) + ((uint32_t)rd16(0x0206) << 16 | rd16(0x0204));
  wr16(0x0200, rd16(0x0204));
  wr16(0x0202, rd16(0x0206));
  wr16(0x1EF4, (uint16_t)total);
  wr16(0x1EF6, (uint16_t)(total >> 16));
  wr16(0x020A, 0);
  wr16(0x020C, 0);
  return (uint16_t)(total >> 16);
}

// TG3000_ROUTINE($83:F9D1) car_lap_check
void car_lap_check(uint16_t dp) {
  if(rd16(dp + 0x26) < rd16(0x019A)) return;
  wr16(dp + 0x35, (uint16_t)(rd16(dp + 0x35) + 1));
  if(rd16(dp + 0x35) == 0) return;
  uint16_t left = (uint16_t)(rd16(0x1F06) - rd16(dp + 0x35));
  if(left != 0) {
    wr16(dp + 0x33, (uint16_t)(left + 9));
    return;
  }
  if(rd16(dp + 0x31) != 0) wr16(dp + 0x7C, (uint16_t)(rd16(dp + 0x7C) | 0x0001));
  const uint32_t mark = 0xFADE + (uint32_t)rd16(0x1EBA);
  wr16(mark, (uint16_t)(rd16(mark) | 0x0004));
  wr16(dp + 0x44, (uint16_t)(rd16(dp + 0x44) | 0x0001));
  wr16(dp + 0x25, 0);
  wr16(dp + 0x26, rd16(0x019A));
  uint16_t x = rd16(0x1F08);
  wr16(0x0446 + (uint32_t)x, rd16(0x1ECA));
  wr16(dp + 0x33, (uint16_t)(x + 0x12));
  wr16(0x1F08, (uint16_t)(x + 1));
  wr16(dp + 0x44, (uint16_t)((rd16(dp + 0x44) | 0x0004) & ~0x0008));
}

// TG3000_ROUTINE($82:B769) func_82B769
void func_82B769(uint16_t a, uint16_t y) {
  if((rom16(0x82DC68 + (uint32_t)rd16(0x047B)) & 0x8000) && y != 0) a = (y & 0x8000) ? 0 : (uint16_t)(rd16(0x019A) - 1);
  func_82B78D(a);
  func_82B7B0();
}

// TG3000_ROUTINE($80:B94F) empty_80B94F
void empty_80B94F(void) {}

// TG3000_ROUTINE($82:DA46) clc_82DA46
void clc_82DA46(void) {}

// TG3000_ROUTINE($83:FA32) clc_83FA32
void clc_83FA32(void) {}

// TG3000_ROUTINE($81:E7CE) far_7fff_4
void far_7fff_4(void) {
  wr16(DP + 0x24, 0x7FFF);
  wr16(DP + 0x26, 0x7FFF);
  wr16(DP + 0x2C, 0x7FFF);
  wr16(DP + 0x2E, 0x7FFF);
}

// TG3000_ROUTINE($80:F225) empty_80F225
void empty_80F225(void) {}

#define WINDOW_BANK 0x10000  // $7F:0000

static void window_shape(uint16_t a, uint16_t x, uint32_t widths) {
  uint16_t y = (uint16_t)((uint16_t)func_80DC5A(0x7F0000 | x, a) + 0x0033);
  const uint16_t start = y;
  wr16(DP + 0x14, 0x001A);
  uint16_t row = rom16(0x81C852 + (uint32_t)(rd16(DP + 0x1E) & 0xFFFE));
  wr16(DP + 0x16, (uint16_t)((rd16(DP + 0x1E) >> 1) + 0x000C));
  do {
    uint8_t w = rom8(widths + row);
    uint8_t right = (uint8_t)((w >> 1) + rd8(DP + 0x2E) - rd8(DP + 0x20));
    wr8(WINDOW_BANK + (uint16_t)(y + 1), right);
    wr8(WINDOW_BANK + y, (uint8_t)(right - w + 1));
    y = (uint16_t)(y + 2);
    row = (uint16_t)(row + 1);
    wr16(DP + 0x14, (uint16_t)(rd16(DP + 0x14) - 1));
    wr16(DP + 0x16, (uint16_t)(rd16(DP + 0x16) - 1));
  } while(rd16(DP + 0x16) != 0);
  for(uint16_t n = rd16(DP + 0x14); n; n--, y = (uint16_t)(y + 2)) wr16(WINDOW_BANK + y, 0x00FF);
  wr16(WINDOW_BANK + y, 0x00FF);
  wr16(WINDOW_BANK + (uint16_t)(y + 2), 0);
  uint16_t to = start, from = start;  // the mirror above the start
  wr16(DP + 0x14, 0x0019);
  if(rd16(DP + 0x1E) & 1) {
    to = (uint16_t)(to - 2);
    wr16(WINDOW_BANK + to, rd16(WINDOW_BANK + from));
    wr16(DP + 0x14, (uint16_t)(rd16(DP + 0x14) - 1));
  }
  do {
    to = (uint16_t)(to - 2);
    from = (uint16_t)(from + 2);
    wr16(WINDOW_BANK + to, rd16(WINDOW_BANK + from));
    wr16(DP + 0x14, (uint16_t)(rd16(DP + 0x14) - 1));
  } while(rd16(DP + 0x14) != 0);
}

// TG3000_ROUTINE($80:DB7D) window_shape_a
void window_shape_a(uint16_t a, uint16_t x) { window_shape(a, x, 0x81C872); }

// TG3000_ROUTINE($80:DBCB) window_shape_b
void window_shape_b(uint16_t a, uint16_t x) { window_shape(a, x, 0x81C9AA); }

// A message `message` replaces the one at `slot` when its priority ($83:A0A4 + 2 * message)
// is at least the current one's.
static void message_request(uint32_t slot, uint16_t message) {
  uint16_t current = rom16(0x83A0A4 + (uint32_t)(uint16_t)(rd16(slot) << 1));
  if(current <= rom16(0x83A0A4 + (uint32_t)(uint16_t)(message << 1))) wr16(slot, message);
}

// TG3000_ROUTINE($83:FEBD) car_message_request
void car_message_request(uint16_t dp, uint16_t message) { message_request((uint16_t)(dp + 0x33), message); }

// TG3000_ROUTINE($82:9BC9) race_message_request_0233
void race_message_request_0233(uint16_t message) { message_request(0x0233, message); }

// TG3000_ROUTINE($82:9BE5) race_message_request_02e6
void race_message_request_02e6(uint16_t message) { message_request(0x02E6, message); }

// TG3000_ROUTINE($82:EADA) track_event_clear
uint16_t track_event_clear(void) {
  const uint32_t x = rd16(DP + 0x1A);
  wr16(0x1F0E, 0);
  wr16(0x3C6C + x, 0);
  wr16(0x3CAC + x, 0);
  wr16(0x1F10, 0);
  wr16(0x3F6C + x, 0);
  wr16(0x3FAC + x, 0);
  uint16_t flags = (uint16_t)(rd16(0x3D6C + x) | 0x0001);
  wr16(0x3D6C + x, flags);
  wr16(0x3DAC + x, flags);
  return flags;
}

// TG3000_ROUTINE($82:EA81) track_event_set ($2007)
// TG3000_ROUTINE($82:EA87) track_event_set ($2047)
// TG3000_ROUTINE($82:EA8D) track_event_set ($2087)
// TG3000_ROUTINE($82:EACE) track_event_set ($2107)
// TG3000_ROUTINE($82:EAD4) track_event_set ($C2E7)
uint16_t track_event_set(uint16_t pattern, uint16_t y) {
  wr16(0x1F0A, pattern);
  uint16_t at = func_82EC09(y);
  const uint32_t x = rd16(DP + 0x1A);
  wr16(0x1F0E, at);
  wr16(0x3C6C + x, at);
  wr16(0x3CAC + x, at);
  uint16_t far = (uint16_t)(at + rd16(DP + 0x1E));
  wr16(0x1F10, far);
  wr16(0x3F6C + x, far);
  wr16(0x3FAC + x, far);
  uint16_t flags = (uint16_t)(rd16(0x3D6C + x) | 0x0001);
  wr16(0x3D6C + x, flags);
  wr16(0x3DAC + x, flags);
  return flags;
}

// TG3000_ROUTINE($8A:824B) title_sprites_place
void title_sprites_place(void) {
  if(rd16(DP + 0x62) != 0xFFFF) return;  // never run
  wr16(0x080A, 0x00A4);
  wr16(0x060A, (uint16_t)(0x404D - rd16(DP + 0x4A) + rd16(DP + 0x74)));
  wr16(0x070A, (uint16_t)(0x0053 - rd16(DP + 0x4C) + rd16(DP + 0x76)));
  wr16(0x080C, 0x00A6);
  wr16(0x060C, (uint16_t)(rd16(0x060A) + 0x0010));
  wr16(0x070C, rd16(0x070A));
}

// TG3000_ROUTINE($8A:867C) rotation_from_angle
void rotation_from_angle(void) {
  const uint32_t x = (uint16_t)((rd16(DP + 0x34) >> 3) & 0x03FE);
  wr16(DP + 0x14, rom16(0x81CEA9 + x));
  wr16(DP + 0x1A, rom16(0x81CEA9 + x));
  wr16(DP + 0x16, rom16(0x81CDA9 + x));
  wr16(DP + 0x18, (uint16_t)(0 - rom16(0x81CDA9 + x)));
}

// TG3000_ROUTINE($81:E7DA) track_entry_bounds
void track_entry_bounds(void) {
  const uint32_t x = rd16(DP + 0x14);
  uint16_t at = (uint16_t)(rd8(0x8CB3 + x) << 4);
  if(at & 0x0800) at |= 0xF000;
  wr16(DP + 0x28, at);
  const uint32_t pair = 0x8380D0 + (uint32_t)(uint16_t)(rd8(0x8CB4 + x) << 2);
  uint16_t a = (uint16_t)(rom16(pair) + at), b = (uint16_t)(rom16(pair + 2) + at);
  wr16(DP + 0x24, a);
  wr16(DP + 0x2C, a);
  wr16(DP + 0x26, b);
  wr16(DP + 0x2E, b);
}

#define SPARKS 0x11000  // $7F:1000

static void spark_lay(uint16_t life, uint16_t at) {
  const uint32_t x = (uint16_t)(rd16(DP + 0x3E) + rd16(DP + 0x5E));
  wr16(SPARKS + x, life);
  wr16(SPARKS + 2 + x, at);
  wr16(DP + 0x3E, (uint16_t)((rd16(DP + 0x3E) + 4) & 0x007F));
}

// TG3000_ROUTINE($80:EA77) spark_step
void spark_step(void) {
  uint16_t x = (uint16_t)(rd16(DP + 0x3E) + rd16(DP + 0x5E));
  wr16(DP + 0x14, 0x0020);
  do {
    uint16_t life = rd16(SPARKS + (uint32_t)x), y = (uint16_t)((life >> 1) & 0xFFFE);
    if(y & 0x000F) {
      wr16(SPARKS + (uint32_t)x, (uint16_t)(life - 1));
      const uint32_t s = rd16(DP + 0x74);
      wr16(0x0902 + s, rom16(0x81D2C3 + (uint32_t)y));
      wr16(0x0900 + s, (uint16_t)(rd16(SPARKS + 2 + (uint32_t)x) + rd16(DP + 0xE7)));
      wr16(0x0B01 + s, 0x0100);
      wr16(DP + 0x74, (uint16_t)(s + 4));
    }
    x = (uint16_t)((x + 4) & 0x807F);
    wr16(DP + 0x14, (uint16_t)(rd16(DP + 0x14) - 1));
  } while(rd16(DP + 0x14) != 0);
  uint16_t n = rd16(DP + 0x86);
  if(n < 0x0064 || (uint16_t)(n - 0x0064) >= 0x001E) return;
  const uint32_t y = (uint16_t)((n - 0x0064) << 1);
  spark_lay(0x001C, rom16(0x81D2E3 + y));
  spark_lay(0x001C, (uint16_t)(rom16(0x81D2E3 + y) - 0x0808));
  if(n & 0x0003) return;
  spark_lay(0x001C, rom16(0x81D323 + y));
  spark_lay(0x003C, rom16(0x81D367 + y));  // (the original tests D+$86 & 3 again: the same)
}

// TG3000_ROUTINE($83:FD3C) car_clear_53
void car_clear_53(uint16_t dp) { wr8(dp + 0x53, 0); }

// TG3000_ROUTINE($80:E8B5) block_copy_7f2000
void block_copy_7f2000(uint16_t* x, uint16_t* y) {
  // DB-relative reads ($D419,Y with DB = $81): past $81:FFFF they reach bank $82
  wr16(DP + 0x14, bus_rd16((0x81D419 + (uint32_t)*y) & 0xFFFFFF));
  *y = (uint16_t)(*y + 2);
  do {  // the count is an 8-bit DEC
    wr8(0x12000 + (uint32_t)*x, bus_rd8((0x81D419 + (uint32_t)*y) & 0xFFFFFF));
    *x = (uint16_t)(*x + 1);
    *y = (uint16_t)(*y + 1);
    wr8(DP + 0x14, (uint8_t)(rd8(DP + 0x14) - 1));
  } while(rd8(DP + 0x14) != 0);
}

// TG3000_ROUTINE($8A:99E2) text_line_push
void text_line_push(uint16_t x) {
  for(uint16_t at = (uint16_t)((x >> 1) << 4);; at = (uint16_t)(at + 1)) {
    uint8_t c = rd8(0x17918 + (uint32_t)at);
    if(c == 0) break;
    byte_buffer_push(c);
  }
}

// $0128 = the x of track object $011A: the byte at $7E:8CB3 + entry, the top 8 bits of a
// signed 12-bit value; $012A = 0.
static uint16_t track_object_x(void) {
  wr16(0x012A, 0);
  uint16_t v = (uint16_t)(rd8(0x8CB3 + (uint32_t)rd16(0x011A)) << 4);
  if(v & 0x0800) v |= 0xF000;
  wr16(0x0128, v);
  return v;
}

// TG3000_ROUTINE($83:FA34) track_object_test
bool track_object_test(uint16_t dp) {
  uint16_t v = track_object_x();
  if(rd8(0x8CB5 + (uint32_t)rd16(0x011A)) & 0x30) return func_83FDCD();
  return func_83FD8A(dp, v);
}

// TG3000_ROUTINE($83:FA6A) track_object_bump
void track_object_bump(uint16_t dp) {
  uint16_t d = (uint16_t)(track_object_x() - rd16(dp + 0x06)), kind = rd16(0x011C);
  // the original's CMP then BMI/BPL: the sign of the difference, not a signed compare
  if(!(d & 0x8000)) {
    if(!((uint16_t)(d - bus_rd16(0x8380CE + kind)) & 0x8000)) return;
  } else if((uint16_t)(d - bus_rd16(0x8380CC + kind)) & 0x8000) return;
  if(rd16(dp + 0x1A) != 0) return;
  wr16(dp + 0x7E, 0x0008);
  wr16(dp + 0x1A, 0x0006);
  func_83FE9A(dp, 0xFF1D, rd16(0x1EBE), 0x0000);
}

// One BCD digit group add (the 65816's ADC in decimal mode, for valid BCD): the four
// digits of a + b + carry, and the carry out.
static uint16_t bcd_add4(uint16_t a, uint16_t b, bool* carry) {
  uint16_t sum = 0;
  unsigned c = *carry;
  for(int shift = 0; shift < 16; shift += 4) {
    unsigned d = ((a >> shift) & 0xF) + ((b >> shift) & 0xF) + c;
    c = d > 9;
    if(c) d -= 10;
    sum |= (uint16_t)(d << shift);
  }
  *carry = c;
  return sum;
}

// TG3000_ROUTINE($83:FAD7) track_object_money
bool track_object_money(uint16_t dp) {
  if(!func_83FC05(dp)) return false;
  wr16(0x8CB3 + (uint32_t)rd16(0x011A), 0);
  const uint32_t car = rd16(0x1ECC);
  bool carry = false;
  wr16(0x038E + car, bcd_add4(rd16(0x038E + car), 0x4000, &carry));
  wr16(0x0390 + car, bcd_add4(rd16(0x0390 + car), 0x0000, &carry));
  bool refused = func_83FE9A(dp, 0xFF19, rd16(0x1EBE), 0x0000);
  wr16(dp + 0x4A, 0x801E);
  wr16(dp + 0x33, 0x0047);
  return refused;
}

// TG3000_ROUTINE($83:FD26) car_set_53 (value 1)
// TG3000_ROUTINE($83:FD31) car_set_53 (value 2)
void car_set_53(uint16_t dp, uint8_t value) {
  wr8(dp + 0x53, value);
  const uint32_t x = rd16(0x011A);
  wr16(dp + 0xB1, (uint16_t)(rd8(0x8CB3 + x) | rd8(0x8CB5 + x) << 8));
}

static bool variant_2(void) { return (rd8(0x03DC) & 0x0F) == 2; }

// TG3000_ROUTINE($8A:9AB1) variant_lines
void variant_lines(void) { wr16(DP + 0x56, variant_2() ? 0x0004 : 0x0014); }

// TG3000_ROUTINE($8A:98F0) variant_5a
void variant_5a(void) { wr16(DP + 0x5A, variant_2() ? 0x0004 : 0x000A); }

// TG3000_ROUTINE($8A:99BE) variant_66
void variant_66(void) {
  if(variant_2()) wr16(DP + 0x66, 0x0008);
  else func_8A99D0();
}

// TG3000_ROUTINE($8A:9AC9) text_lines_copy
void text_lines_copy(void) {
  if(rd16(DP + 0x56) == 0) return;
  uint16_t y = 0;
  do {
    uint16_t x = rd16(DP + 0x54);
    uint8_t c;
    do {
      c = rom8(0x8A9DEE + (uint32_t)y);
      wr8(0x17918 + (uint32_t)x, c);
      x = (uint16_t)(x + 1);
      y = (uint16_t)(y + 1);
    } while(c != 0);
    wr16(DP + 0x54, (uint16_t)(rd16(DP + 0x54) + 0x0010));
    wr16(DP + 0x56, (uint16_t)(rd16(DP + 0x56) - 1));
  } while(rd16(DP + 0x56) != 0);
}

// TG3000_ROUTINE($83:FB15) track_object_count
bool track_object_count(uint16_t dp) {
  if(!func_83FC05(dp)) return false;
  if(rd8(dp + 0x4E) == 0x0A) return true;
  wr16(dp + 0x4E, (uint16_t)(rd16(dp + 0x4E) + 1));
  wr16(0x8CB3 + (uint32_t)rd16(0x011A), 0);
  bool refused = func_83FE9A(dp, 0xFF19, rd16(0x1EBE), 0x0000);
  wr16(dp + 0x4A, 0x801E);
  wr16(dp + 0x33, 0x004C);
  return refused;
}

static void race_section_seek(uint16_t position, uint16_t offset) {
  for(;;) {
    uint16_t pos = rd16(position);
    uint16_t entry = bus_rd16(0x840000 + (uint32_t)(uint16_t)(rd16(DP + 0x9C) + rd16(DP + offset)));
    if(pos < entry || (uint16_t)(pos - entry) < 0x0040) return;
    wr16(DP + offset, (uint16_t)(rd16(DP + offset) + 0x0014));
  }
}

// TG3000_ROUTINE($85:9D39) race_section_seek_1
void race_section_seek_1(void) { race_section_seek(0x023E, 0xA2); }

// TG3000_ROUTINE($85:9D5A) race_section_seek_2
void race_section_seek_2(void) { race_section_seek(0x02F1, 0xA6); }

static void row_copy(uint16_t a, uint16_t x, uint32_t to) {
  uint16_t from = (uint16_t)(a + bus_rd16(0x8384B1 + (uint32_t)x));  // DB = $83
  for(uint16_t y = 0; y < 0x20; y = (uint16_t)(y + 2), from = (uint16_t)(from + 2))
    wr16(to + y, bus_rd16(0x7F0000 + (uint32_t)from));
}

// TG3000_ROUTINE($85:82D8) row_copy_6a33
void row_copy_6a33(uint16_t a, uint16_t x) { row_copy(a, x, 0x6A33); }

// TG3000_ROUTINE($85:82FA) row_copy_6b33
void row_copy_6b33(uint16_t a, uint16_t x) { row_copy(a, x, 0x6B33); }

// TG3000_ROUTINE($82:AD99) tile_block_5x5
void tile_block_5x5(uint16_t first) {
  wr16(DP + 0x16, first);
  uint16_t x = 0;
  wr16(DP + 0x14, 0x0005);
  do {
    for(int row = 0; row < 5; row++) {
      wr16(0x10130 + (uint32_t)x, rd16(DP + 0x16));
      wr16(DP + 0x16, (uint16_t)(rd16(DP + 0x16) + 1));
      x = (uint16_t)(x + 0x0040);
    }
    x = (uint16_t)(x - 0x013E);  // back to the top, one column on
    wr16(DP + 0x14, (uint16_t)(rd16(DP + 0x14) - 1));
  } while(rd16(DP + 0x14) != 0);
}

// TG3000_ROUTINE($85:831C) track_settings_load
void track_settings_load(void) {
  const uint16_t track = rd16(0x047B);
  const uint16_t x4 = (uint16_t)(track << 2), x2 = (uint16_t)(x4 >> 1), x = (uint16_t)(track << 1);
  wr16(DP + 0x98, bus_rd16(0x84A590 + (uint32_t)x4));
  wr16(DP + 0x9A, bus_rd16(0x84A592 + (uint32_t)x4));
  wr16(DP + 0x9C, bus_rd16(0x84FF59 + (uint32_t)x2));
  wr16(0x1F06, bus_rd16(0x839515 + (uint32_t)x));
  wr16(0x01E4, bus_rd16((rd8(0x039C) ? 0x8395F1 : 0x839583) + (uint32_t)x));
  wr16(0x01E6, bus_rd16((rd8(0x03CB) ? 0x8395F1 : 0x839583) + (uint32_t)x));
  wr16(0x1F34, bus_rd16(0x83965F + (uint32_t)x));
}

// TG3000_ROUTINE($86:CE4A) rows_7f_copy8
void rows_7f_copy8(uint16_t* x, uint16_t* y) {
  for(int row = 0; row < 8; row++) {
    if(row) *y = (uint16_t)(*y + 8);
    for(int i = 0; i < 0x38; i++) {  // MVN: a byte at a time, upwards
      wr8(0x10000 + (uint32_t)*y, rd8(0x10000 + (uint32_t)*x));
      *x = (uint16_t)(*x + 1);
      *y = (uint16_t)(*y + 1);
    }
  }
}

// TG3000_ROUTINE($80:C31A) block_c4_build
void block_c4_build(uint16_t dp) {
  const uint16_t at = rd16(dp + 0xC4);
  // two MVNs into bank $81 (opcode, destination bank, source bank)
  for(uint16_t i = 0; i < 0x60; i++) wr8((uint16_t)(at + i), rom8(0x9B8000 + i));
  for(uint16_t i = 0; i < 0x20; i++) wr8((uint16_t)(at + 0x60 + i), rom8(0x80B2D2 + i));
  wr16((uint16_t)(at + 0x0102), 0x0E03);
  wr16((uint16_t)(at + 0x0104), 0x0040);
  wr16((uint16_t)(at + 0x0106), 0x109F);
  wr16((uint16_t)(at + 0x0108), 0x0000);
  wr16((uint16_t)(at + 0x0122), 0x0E03);
  wr16((uint16_t)(at + 0x0124), 0x0040);
}

// TG3000_ROUTINE($80:EBCE) queue_7f_init
void queue_7f_init(uint16_t dp) {
  wr16(dp + 0xCC, 0);
  wr16(dp + 0xC6, 0);
  wr16(dp + 0xE5, 0x0080);
  wr16(dp + 0xE1, 0x0040);
  wr16(dp + 0xD1, 0);
  uint16_t x = rd16(dp + 0x5E);
  for(int i = 0; i < 0x1C; i++, x = (uint16_t)(x + 2)) wr16(0x10000 + (uint32_t)x, 0xFFFF);
  wr16(0x10000 + (uint32_t)x, 0xFFFE);
}

// TG3000_ROUTINE($8A:9BF6) text_map_clear
void text_map_clear(void) {
  wr16(DP + 0x60, 0);
  wr16(DP + 0x14, 0);
  wr16(DP + 0x58, 0);
  for(uint32_t x = 0; x <= 0x140; x += 2) wr16(0x17918 + x, 0);
  for(uint32_t x = 0; x <= 0x26; x += 2) wr16(0x17FBC + x, 0);
}

// TG3000_ROUTINE($8A:98B4) variant_offsets_add
void variant_offsets_add(void) {
  variant_5a();
  const uint32_t table = variant_2() ? 0x8A9DB5 : 0x8A9D8D;
  uint16_t x = 0;
  do {
    const uint16_t y = (uint16_t)(rd8(0x0446 + (uint32_t)x) << 1);
    wr16(0x041C + (uint32_t)y, (uint16_t)(rd16(0x041C + (uint32_t)y) + rom16(table + (uint32_t)(uint16_t)(x << 1))));
    x = (uint16_t)(x + 1);
    wr16(DP + 0x5A, (uint16_t)(rd16(DP + 0x5A) - 1));
  } while(rd16(DP + 0x5A) != 0);
}

// TG3000_ROUTINE($8A:81BA) palettes_load_8a
void palettes_load_8a(void) {
  upload_cgram_now(0x80, 0x7E, 0x81E2, 0x0020);
  upload_cgram_now(0x90, 0x7E, 0x8202, 0x0020);
  upload_cgram_now(0xA0, 0x7E, 0x8222, 0x0020);
  upload_cgram_now(0x01, 0x7E, 0x8114, 0x001E);
  const uint16_t at = bus_rd16(0x8A934F + (uint32_t)(uint16_t)(rd16(0x1536) << 3));
  upload_cgram_now(0xA0, 0x7E, at, 0x0020);
}

// TG3000_ROUTINE($85:9D7B) track_map_load
uint16_t track_map_load(uint16_t dp, uint16_t* end) {
  if((bus_rd16(0x85A439 + (uint32_t)rd16(0x047B)) & 0x8000) && rd8(0x039C)) {
    for(uint32_t x = 0; x < 0x1040; x += 2) wr16(0x6C33 + x, 0x3FFF);
    return 0;
  }
  return decompress(dp, 0x8C, 0x95E3, 0x7E, 0x6C33, end);
}

// TG3000_ROUTINE($82:A954) track_segments_load
void track_segments_load(uint16_t dp) {
  uint16_t end;
  decompress(dp, 0x9A, 0x8000, 0x7F, 0x0000, &end);
  const uint16_t t = (uint16_t)(rd16(0x047B) << 1);
  uint16_t from = bus_rd16(0x859FAA + (uint32_t)t), to = 0x4F31;
  const uint16_t n = bus_rd16(0x85A008 + (uint32_t)t);
  for(uint32_t i = 0; i < (n ? n : 0x10000u); i++) {  // MVN $7F to $7E, a byte at a time
    wr8(to, rd8(0x10000 + (uint32_t)from));
    from = (uint16_t)(from + 1);
    to = (uint16_t)(to + 1);
  }
  const uint16_t count = (uint16_t)(n >> 2);
  for(uint32_t field = 0; field < 4; field += 2) {
    uint16_t sum = 0;
    for(uint16_t k = 0, x = 0; k < count; k++, x = (uint16_t)(x + 4)) {
      sum = (uint16_t)(sum + rd16(0x4F31 + field + x));
      wr16(0x4F31 + field + x, sum);
    }
  }
}

// TG3000_ROUTINE($8A:B0E1) variant_flags_apply
bool variant_flags_apply(void) {
  bool carry;
  wr16(0x1528, (uint16_t)((rd16(0x1528) & 0xFFC3) | lsr_n((uint16_t)(rd16(0x03DC) & 0x0400), 8, &carry)));
  wr16(0x1526, (uint16_t)((rd16(0x1526) & 0xFFC3) | lsr_n((uint16_t)(rd16(0x03DC) & 0x0300), 6, &carry)));
  return carry;
}

// TG3000_ROUTINE($8A:C19C) sprite_slots_six
void sprite_slots_six(void) {
  for(uint32_t x = 0; x < 0x0C; x += 2) {
    wr16(0x0800 + x, (uint16_t)(rom16(0x8AC443 + x) | 0x2000));
    wr16(0x0600 + x, rd16(DP + 0x42));
    wr16(DP + 0x42, (uint16_t)(rd16(DP + 0x42) + 0x0010));
    wr16(0x0700 + x, rd16(DP + 0x44));
    wr16(0x0500 + x, 0x0001);
  }
}

static void tiles_planar_to(uint16_t offset, uint16_t x, uint16_t vram);

// TG3000_ROUTINE($8A:8CB7) tiles_planar_upload
void tiles_planar_upload(uint16_t offset, uint16_t x) { tiles_planar_to(offset, x, 0x6FE0); }

// TG3000_ROUTINE($85:817B) tiles_planar_upload_5fe0
void tiles_planar_upload_5fe0(uint16_t offset, uint16_t x) { tiles_planar_to(offset, x, 0x5FE0); }

static void tiles_planar_to(uint16_t offset, uint16_t x, uint16_t vram) {
  wr16(DP + 0x38, offset);
  const uint16_t size = bus_rd16(0x83851B + (uint32_t)x);  // DB = $83 at entry
  wr16(DP + 0x3C, (uint16_t)(size >> 5));  // tiles
  uint16_t at = (uint16_t)(bus_rd16(0x8384B1 + (uint32_t)x) + offset);
  do {
    for(uint32_t row = 0; row < 0x10; row += 2) {
      wr8(DP + 0x3A, 0x04);
      do {  // a byte: two pixels, high nibble first, each bit rotated into its plane byte
        uint8_t b = rd8(0x10000 + (uint32_t)at);
        for(int half = 0; half < 2; half++, b = (uint8_t)(b << 4)) {
          static const uint8_t plane[4] = {0x25, 0x24, 0x15, 0x14};  // bit 3, 2, 1, 0
          for(int bit = 0; bit < 4; bit++) {
            const uint32_t p = DP + plane[bit] + row;
            wr8(p, (uint8_t)((rd8(p) << 1) | ((b >> (7 - bit)) & 1)));
          }
        }
        at = (uint16_t)(at + 1);
        wr8(DP + 0x3A, (uint8_t)(rd8(DP + 0x3A) - 1));
      } while(rd8(DP + 0x3A) != 0);
    }
    at = (uint16_t)(at - 0x20);
    for(uint32_t i = 0; i < 0x20; i += 2) wr16(0x10000 + (uint32_t)(uint16_t)(at + i), rd16(DP + 0x14 + i));
    at = (uint16_t)(at + 0x20);
    wr16(DP + 0x3C, (uint16_t)(rd16(DP + 0x3C) - 1));
  } while(rd16(DP + 0x3C) != 0);
  wr16(DP + 0x14, 0x007F);
  upload_vram_now(vram, 0x7F, (uint16_t)(bus_rd16(0x8384B1 + (uint32_t)x) + rd16(DP + 0x38)), size);
}

// TG3000_ROUTINE($82:A1D3) track_backdrop_load
void track_backdrop_load(uint16_t dp) {
  uint16_t end;
  decompress(dp, 0x98, 0x9FEB, 0x7F, 0x0000, &end);
  const uint16_t k = (uint16_t)(bus_rd8(0x84A66C + (uint32_t)rd16(0x047B)) << 2);
  wr16(0x1F28, bus_rd16(0x85A33A + (uint32_t)k));
  wr16(0x1F2A, bus_rd16(0x85A33C + (uint32_t)k));
  const uint32_t x = (uint16_t)(k << 1);
  const uint16_t map1 = bus_rd16(0x85A34A + x), map2 = bus_rd16(0x85A34C + x);
  wr16(dp + 0x14, 0x007F);
  upload_vram_now(0x3800, 0x7F, bus_rd16(0x85A346 + x), bus_rd16(0x85A348 + x));
  wr16(dp + 0x14, 0x007F);
  upload_vram_now(0x3000, 0x7F, map1, 0x0600);
  wr16(dp + 0x14, 0x007F);
  upload_vram_now(0x3400, 0x7F, map2, 0x0600);
  uint16_t at = map1;
  for(int i = 0; i < 0x600; i++, at = (uint16_t)(at + 2))
    wr16(0x10000 + (uint32_t)at, (uint16_t)((rd16(0x10000 + (uint32_t)at) & 0xDFFF) + 0x0400));
  wr16(dp + 0x14, 0x007F);
  upload_vram_now(0x7400, 0x7F, map1, 0x0600);
  wr16(dp + 0x14, 0x007F);
  upload_vram_now(0x7800, 0x7F, map2, 0x0600);
}

// A car's progress: the words at $7E:FAB5, $FAB7 and $FAD3 + `x`, low to high.
static uint64_t car_progress(uint16_t x) {
  return (uint64_t)rd16(0xFAB5 + (uint32_t)x) | (uint64_t)rd16(0xFAB7 + (uint32_t)x) << 16 |
         (uint64_t)rd16(0xFAD3 + (uint32_t)x) << 32;
}

// TG3000_ROUTINE($82:AD2C) race_positions_update
void race_positions_update(void) {
  wr16(0x0142, 0);
  wr16(0x0144, 0);
  const uint64_t p1 = car_progress(0x0000), p2 = car_progress(0x0035);
  for(uint16_t car = 0, x = 0; car < 20; car++, x = (uint16_t)(x + 0x35)) {
    const uint64_t v = car_progress(x);
    if(!(((v - p1) >> 47) & 1)) wr16(0x0142, (uint16_t)(rd16(0x0142) + 1));
    if(!(((v - p2) >> 47) & 1)) wr16(0x0144, (uint16_t)(rd16(0x0144) + 1));
  }
  if(rd16(0x0142) != rd16(0x1ED2)) {
    wr16(0x1ED2, rd16(0x0142));
    race_message_request_0233((uint16_t)(rd16(0x0142) + 0x25));
  }
  if(rd16(0x0144) != rd16(0x1ED4)) {
    wr16(0x1ED4, rd16(0x0144));
    race_message_request_02e6((uint16_t)(rd16(0x0144) + 0x25));
  }
}

static uint8_t band_a(void) { return rd8(0x4CFE + (uint32_t)rd16(DP + 0x14)); }
static uint8_t band_b(void) { return rd8(0x4D4A + (uint32_t)rd16(DP + 0x16)); }

// TG3000_ROUTINE($85:B1FF) race_bands_merge
uint16_t race_bands_merge(void) {
  wr16(DP + 0x14, rd16(0x0164));
  wr16(DP + 0x16, rd16(0x0168));
  wr16(DP + 0x18, 0x8080);
  uint16_t y = 0x0002, x = 0x0002;
  if(rd16(0x00B4) == 0) {
    y = 0x0034;
    x = 0x0000;
  }
  wr16(0x00C0, y);
  wr16(DP + 0x1A, x);
  uint8_t a = band_a(), b = band_b();
  if(a == b) {
    wr8(0x047E + (uint32_t)x, 0x39);
  } else {
    wr8(0x047E + (uint32_t)x, a < b ? 0x19 : 0x29);
    wr8(0x047F + (uint32_t)x, 0x39);
  }
  for(;;) {
    a = band_a();
    b = band_b();
    if(a == b) {
      wr8(0x0486 + (uint32_t)y, a);
      if(a == 0) return y;
      wr8(0x0596 + (uint32_t)y, (uint8_t)(0x0E | rd8(DP + 0x18) | rd8(DP + 0x19)));
      wr8(DP + 0x18, 0);
      wr8(DP + 0x19, 0);
      y = (uint16_t)(y + 1);
      wr8(DP + 0x14, (uint8_t)(rd8(DP + 0x14) + 1));
      wr8(DP + 0x16, (uint8_t)(rd8(DP + 0x16) + 1));
      continue;
    }
    // the lower line goes first; a list already at its zero gives way to the other
    bool first = a < b;
    for(;;) {
      if(first) {
        if((a = band_a()) == 0) { first = false; continue; }
        wr8(0x0486 + (uint32_t)y, a);
        wr8(0x0596 + (uint32_t)y, (uint8_t)(0x02 | rd8(DP + 0x19)));
        wr8(DP + 0x19, 0);
        wr8(DP + 0x14, (uint8_t)(rd8(DP + 0x14) + 1));
      } else {
        if((b = band_b()) == 0) { first = true; continue; }
        wr8(0x0486 + (uint32_t)y, b);
        wr8(0x0596 + (uint32_t)y, (uint8_t)(0x0C | rd8(DP + 0x18)));
        wr8(DP + 0x18, 0);
        wr8(DP + 0x16, (uint8_t)(rd8(DP + 0x16) + 1));
      }
      y = (uint16_t)(y + 1);
      break;
    }
  }
}

// TG3000_ROUTINE($8A:8F18) hdma_gradients_init
void hdma_gradients_init(void) {
  wr8(0x2C00, 0xFF);
  wr8(0x2C03, 0xFF);
  wr8(0x2C06, 0x00);
  wr16(0x2C01, 0x2800);
  wr16(0x2C04, 0x2A00);
  wr16(DP + 0x68, 0x2000);
  wr16(DP + 0x6A, 0x007E);
  wr16(DP + 0x6C, 0x0000);
  wr16(DP + 0x3A, 0);
  wr16(DP + 0x3C, 0);
  wr16(DP + 0x34, 0);
  wr16(DP + 0x36, 0);
  wr16(DP + 0x38, 0);
  hdma_gradient_build();
  uint32_t x = (uint16_t)(rd16(0x1536) << 3);
  wr16(DP + 0x34, 0);
  wr16(DP + 0x36, 0);
  wr16(DP + 0x38, bus_rd16(0x8A934B + x));
  hdma_gradient_build();
  wr16(DP + 0x3A, 1);
  wr16(DP + 0x3C, 1);
  x = (uint16_t)(rd16(0x1536) << 3);
  wr16(DP + 0x34, bus_rd16(0x8A934B + x));
  wr16(DP + 0x36, bus_rd16(0x8A934D + x));
  wr16(DP + 0x38, rd16(DP + 0x72));
  hdma_gradient_build();
}

static void flags_1e9c(uint16_t bits) { wr16(0x1E9C, (uint16_t)(rd16(0x1E9C) | bits)); }

// The original's TRB of `mask` from $000E: true when any of its bits were set.
static bool pressed_take(uint16_t mask) {
  uint16_t m = rd16(0x000E);
  wr16(0x000E, (uint16_t)(m & ~mask));
  return (m & mask) != 0;
}

// TG3000_ROUTINE($83:E2D7) player_frame_setup
void player_frame_setup(void) {
  wr16(0x1EBE, 0x8000);
  const uint16_t t = rd8(0x0395);
  wr16(0x1EB2, t);
  wr16(0x022B, func_83EA9B((uint16_t)((rd16(0x021F) << 1) + bus_rd16(0x86DAD9 + (uint32_t)((t & 0x7F) << 1)))));
  uint32_t x = (uint32_t)rd8(0x0393) << 1;
  wr16(0x1EA8, bus_rd16(0x86DCE1 + x));
  wr16(0x1EC0, bus_rd16(0x86DCED + x));
  x = (uint32_t)rd8(0x0397) << 1;
  wr16(0x1EB4, bus_rd16(0x86DD29 + x));
  wr16(0x1EB6, bus_rd16(0x86DD35 + x));
  wr16(0x1EB8, bus_rd16(0x86DD41 + ((uint32_t)rd8(0x0394) << 1)));
  wr16(0x1F30, bus_rd16(0x86DD4D + ((uint32_t)rd8(0x0396) << 1)));
  wr16(0x1ECA, 0);
  wr16(0x1EBA, 0);
  wr16(0x1ECC, 0);
  wr16(0x1F2C, rd16(0x1E66));
  wr16(0x1EBC, (uint16_t)((rd16(0x0226) << 1) & 0x003E));
  wr16(0x1E9C, 0);
  if((rd16(0x0244) & 0x0004) || !(rd16(0x03DC) & 0x0010)) {
    player_road_limits();
    return;
  }
  const uint16_t pad = rd16(0x000C);
  if(pad & rd16(0x039D)) flags_1e9c(0x0001);
  uint16_t view = 0x0058;
  if(pad & rd16(0x039F)) {
    flags_1e9c(0x0002);
    view = 0x109F;
  }
  wr16(0x2142, view);
  if(pad & 0x0200) flags_1e9c(0x0004);
  if(pad & 0x0100) flags_1e9c(0x0008);
  const uint32_t item2 = (uint16_t)(rd16(0x014C) << 1);
  if(pressed_take(rd16(0x03A1))) flags_1e9c(bus_rd16(0x86DD59 + item2));
  if(rd16(0x03A1) & rd16(0x000C)) flags_1e9c(bus_rd16(0x86DD63 + item2));
  if(pressed_take(rd16(0x03A5))) {  // the previous item carried, or item 0
    for(uint16_t i = rd16(0x014C);;) {
      i = (uint16_t)(i - 1);
      if(i == 0) { wr16(0x014C, i); break; }
      if(i & 0x8000) break;
      if(rd8(0x0397 + (uint32_t)i)) { wr16(0x014C, i); break; }
    }
  }
  if(pressed_take(rd16(0x03A3))) {  // the next item carried, below 5
    for(uint16_t i = rd16(0x014C);;) {
      i = (uint16_t)(i + 1);
      if(i == 5) break;
      if(rd8(0x0397 + (uint32_t)i)) { wr16(0x014C, i); break; }
    }
  }
  const uint16_t item = rd16(0x014C);
  wr16(0x025C, rd8(0x024E + (uint32_t)item));
  wr16(0x1F2E, item);
}

// One object: D+$7C = D+$84 + `dy`, D+$7A = the word at D+`base` + `dx` + that sum's carry.
static void scene_object_at(uint16_t dy, uint16_t base, uint16_t dx) {
  const uint32_t sum = (uint32_t)rd16(DP + 0x84) + dy;
  wr16(DP + 0x7C, (uint16_t)sum);
  wr16(DP + 0x7A, (uint16_t)(rd16(DP + base) + dx + (sum >> 16)));
}

// TG3000_ROUTINE($8A:86AB) scene_objects_place
uint16_t scene_objects_place(void) {
  const uint16_t x8 = (uint16_t)(rd16(0x1536) << 3);
  if(!(bus_rd16(0x8A9351 + (uint32_t)x8) & 0x0001)) return x8;
  const uint16_t v = (uint16_t)(rd16(DP + 0x3C) >> 6);
  wr16(DP + 0x7E, v);
  wr16(DP + 0x80, (uint16_t)(v >> 1));
  wr16(DP + 0x82, (uint16_t)(v >> 1));
  const uint16_t neg = (uint16_t)-rd16(DP + 0x62);
  wr16(DP + 0x84, neg);
  wr16(DP + 0x86, neg);
  wr16(DP + 0x88, neg);
  uint16_t x = 0;
  scene_object_at(0x0010, 0x7E, 0x0014);
  x = func_8A9202(x);
  scene_object_at(0x0020, 0x7E, 0x00C8);
  x = func_8A9202(x);
  wr16(DP + 0x7C, rd16(DP + 0x84));  // no add: the carry stays clear
  wr16(DP + 0x7A, (uint16_t)(rd16(DP + 0x80) + 0x000A));
  x = func_8A92CF(x);
  scene_object_at(0x0032, 0x82, 0x0012);
  x = func_8A92CF(x);
  scene_object_at(0x0028, 0x80, 0x00F0);
  x = func_8A92CF(x);
  scene_object_at(0x0032, 0x82, 0x00C8);
  x = func_8A92CF(x);
  scene_object_at(0x0019, 0x82, 0x0080);
  x = func_8A92CF(x);
  // CLC then SBC #$0A: $0B less; its carry (no borrow) goes into the ADC
  const uint16_t y = rd16(DP + 0x84);
  wr16(DP + 0x7C, (uint16_t)(y - 0x000B));
  wr16(DP + 0x7A, (uint16_t)(rd16(DP + 0x80) + 0x0140 + (y >= 0x000B)));
  return func_8A92CF(x);
}

// The results table by variant (bits 8-9 of $03DC), and the offset result_tally adds.
static uint32_t result_table(uint16_t* offset) {
  switch(rd16(0x03DC) & 0x0300) {
    case 0x0000: *offset = 0x0000; return 0x85D501;
    case 0x0100: *offset = 0x001C; return 0x85D530;
    default:     *offset = 0x0044; return 0x85D55F;
  }
}

// TG3000_ROUTINE($84:A31B) result_slot_step
void result_slot_step(void) {
  uint16_t off;
  const uint32_t t = result_table(&off);
  if(bus_rd8(t + (uint32_t)rd16(0x0444))) wr16(0x036C, (uint16_t)(rd16(0x036C) + 4));
}

// TG3000_ROUTINE($84:A34B) result_tally
void result_tally(void) {
  uint16_t off;
  const uint32_t t = result_table(&off);
  if(!bus_rd8(t + (uint32_t)rd16(0x0444))) return;
  const uint16_t x = bus_rd16(0x85D590 + (uint32_t)(uint16_t)(off + rd16(0x036C)));
  wr16(0x036E + (uint32_t)x, (uint16_t)(rd16(0x036E + (uint32_t)x) + 1));
}

// TG3000_ROUTINE($8A:9D1E) clear_041c
void clear_041c(void) {
  for(uint32_t x = 0; x < 0x28; x += 2) wr16(0x041C + x, 0);
}

// TG3000_ROUTINE($80:E8CE) name_copy_7f2000
void name_copy_7f2000(uint16_t* x, uint16_t* y) {
  *y = rd16(DP + 0x94);
  for(uint8_t c; (c = rd8(0x037E + (uint32_t)*y)) != 0; *x = (uint16_t)(*x + 1), *y = (uint16_t)(*y + 1))
    wr8(0x12000 + (uint32_t)*x, c);
  wr16(0x12000 + (uint32_t)*x, 0x0100);
  wr16(0x12002 + (uint32_t)*x, 0x1400);
  *x = (uint16_t)(*x + 4);
}

static uint16_t player_car(void) { return rd16(0x03A8 + (uint32_t)rd16(DP + 0x94)); }

// TG3000_ROUTINE($80:E8F4) car_blocks_7f2000
void car_blocks_7f2000(uint16_t* x, uint16_t* y) {
  const uint16_t car = player_car();
  if(car & 0x8000) {
    *y = 0x00A8;
    block_copy_7f2000(x, y);
    return;
  }
  *y = rom16(0x81D3C9 + (uint32_t)(uint16_t)(car << 1));
  block_copy_7f2000(x, y);
  *y = 0x00BE;
  block_copy_7f2000(x, y);
}

// TG3000_ROUTINE($80:E911) car_block_7f2000
void car_block_7f2000(uint16_t* x, uint16_t* y) {
  uint16_t car = player_car();
  if(car & 0x8000) car = 0x0013;
  *y = rom16(0x81D3F1 + (uint32_t)(uint16_t)(car << 1));
  block_copy_7f2000(x, y);
}

// TG3000_ROUTINE($84:A41B) track_next
uint16_t track_next(void) {
  uint32_t list;
  switch(rd16(0x03DC) & 0x0300) {
    case 0x0100: list = 0x8ACADE; break;
    case 0x0200: list = 0x8ACB01; break;
    default:     list = 0x8ACAC4; break;
  }
  uint16_t x = 0;
  while(bus_rd8(list + x) != rd16(0x0444)) x = (uint16_t)(x + 1);  // a 16-bit compare
  x = (uint16_t)(x + 1);
  wr16(0x0444, bus_rd8(list + x));
  return x;
}

// TG3000_ROUTINE($84:A4ED) results_init
void results_init(void) {
  wr16(0x0444, 0);
  for(uint16_t i = 0; i < 0x16; i++) wr8(0x0368 + i, rom8(0x8385AB + i));
  for(uint16_t i = 0; i < 0x0F; i++) wr8(0x038E + i, rom8(0x8385D1 + i));
  for(uint16_t i = 0; i < 0x0F; i++) wr8(0x03BD + i, rom8(0x838600 + i));
  for(uint16_t i = 0, v = 0x0100; i < 10; i++, v = (uint16_t)(v + 0x0202)) wr16(0x0446 + 2u * i, v);
  wr16(0x036C, 0);
  if(rd16(0x03E2) & 0x0002) {
    static const uint16_t items[8] = {0x0398, 0x0399, 0x039A, 0x039C, 0x03C7, 0x03C8, 0x03C9, 0x03CB};
    for(int i = 0; i < 8; i++) wr8(items[i], 0x01);
  }
}

// The 65816's 16-bit ADC in decimal mode, a digit at a time as the CPU does it (also for
// digits above 9, as in the unused $FFFF bonus entries); *carry in and out; *overflow =
// its V (taken before the last digit's adjustment, as the CPU does).
static uint16_t adc_decimal16(uint16_t a, uint16_t b, bool* carry, bool* overflow) {
  uint32_t r = (a & 0xF) + (b & 0xF) + (*carry ? 1u : 0u);
  if(r > 0x9) r = ((r + 0x6) & 0xF) + 0x10;
  r = (a & 0xF0) + (b & 0xF0) + r;
  if(r > 0x9F) r = ((r + 0x60) & 0xFF) + 0x100;
  r = (a & 0xF00) + (b & 0xF00) + r;
  if(r > 0x9FF) r = ((r + 0x600) & 0xFFF) + 0x1000;
  r = (uint32_t)(a & 0xF000) + (b & 0xF000) + r;
  *overflow = (a & 0x8000) == (b & 0x8000) && (b & 0x8000) != (r & 0x8000);
  if(r > 0x9FFF) r += 0x6000;
  *carry = r > 0xFFFF;
  return (uint16_t)r;
}

// One player's prize: decimal adds into the money at `money`; *v = the last add's V.
static bool prize_one(uint16_t place_at, uint16_t money, uint16_t bonus_at, bool carry, bool* v) {
  const uint16_t place = rd16(place_at);
  if(place & 0x8000) return carry;
  const uint32_t p = (uint16_t)(place << 2);
  bool c = false;
  wr16(money, adc_decimal16(rd16(money), bus_rd16(0x84A2B3 + p), &c, v));
  wr16(money + 2u, adc_decimal16(rd16(money + 2u), bus_rd16(0x84A2B5 + p), &c, v));
  for(uint32_t x = 0;; x += 4) {
    const uint16_t bits = rd16(bonus_at);
    wr16(bonus_at, (uint16_t)(bits >> 1));
    c = bits & 1;
    if(c) {
      c = false;
      wr16(money, adc_decimal16(rd16(money), bus_rd16(0x84A2DB + x), &c, v));
      wr16(money + 2u, adc_decimal16(rd16(money + 2u), bus_rd16(0x84A2DD + x), &c, v));
    }
    if(rd16(bonus_at) == 0) return c;
  }
}

// TG3000_ROUTINE($84:A388) prize_money_award
bool prize_money_award(bool carry, bool* overflow) {
  carry = prize_one(0x03A8, 0x038E, 0x03AB, carry, overflow);
  return prize_one(0x03D7, 0x03BD, 0x03DA, carry, overflow);
}

// TG3000_ROUTINE($8A:9CF8) slot_order_fill
uint16_t slot_order_fill(void) {
  for(uint32_t x = 0; x <= 0x26; x += 2) wr16(0x17FBC + x, 0);
  func_8A9BE2();
  func_8A99D0();
  uint16_t y = 0, slot;
  while(func_8A9BB5(&slot)) {
    wr16(0x0D00 + (uint32_t)y, (uint16_t)(slot >> 1));
    y = (uint16_t)(y + 2);
  }
  return y;
}

// TG3000_ROUTINE($8A:9AF1) player_names_put
void player_names_put(void) {
  const uint16_t v = (uint16_t)(rd16(0x03DC) & 0x000F);
  if(v == 0) {
    text_line_put(0x037E, 0x81);
    return;
  }
  if(v == 2) {
    text_line_put(0x03E4, 0x81);
    text_line_put(0x03F1, 0x81);
    text_line_put(0x03FE, 0x81);
    text_line_put(0x040B, 0x81);
    return;
  }
  if(v != 1) return;
  const uint16_t who = rd16(0x03DC);
  if((who & 0x0030) == 0x0010) {
    text_line_put(0x037E, 0x81);
    text_line_put(0x9B7D, 0x8A);  // "PLAYER2"
    return;
  }
  if(who & 0x0010) text_line_put(0x037E, 0x81);
  if(who & 0x0020) text_line_put(0x03AD, 0x81);
}

// TG3000_ROUTINE($82:8F8F) shake_step
void shake_step(void) {
  if(rd16(0x0231) == 0) {
    wr16(0x1F18, 0x8000);
    return;
  }
  const uint16_t s = rd16(0x1F18);
  if(s & 0x8000) {
    wr16(0x1F18, rd8(0x0397) >= 4 ? 0x0030 : 0x0000);
    return;
  }
  if(s < 0x0098) {
    wr16(0x1F1C, 0xFFFD);
    wr16(0x1F20, 0x0002);
    const uint16_t before = (uint16_t)(s & 0xFFF8);
    const uint16_t now = (uint16_t)((rd16(0x1E9E) << 1) + s);
    wr16(0x1F18, now);
    const uint16_t step = (uint16_t)(now & 0xFFF8);
    if(step >= 0x0098) wr16(0x1F18, 0x00B0);
    else if(step >= 0x0030 && before < 0x0030) wr16(0x1F18, 0x0098);
    return;
  }
  bool c;
  uint16_t k;
  do {
    wr16(0x1F1C, (uint16_t)((uint32_t)dsp4_multiply((int16_t)(rng_next_long(&c) >> 1), -10) >> 16));
    wr16(0x1F20, (uint16_t)((uint32_t)dsp4_multiply((int16_t)(rng_next_long(&c) >> 1), 10) >> 16));
    k = (uint16_t)(rng_next_long(&c) & 0x0006);
  } while(k == 0x0006);
  wr16(0x1F18, rom16((rd16(0x1F18) >= 0x00B0 ? 0x82913F : 0x829139) + k));
}

// TG3000_ROUTINE($82:AF33) track_objects_stream
void track_objects_stream(void) {
  wr16(0x45EC, 0x0140);
  uint16_t x = rd16(DP + 0xA0);
  if(rd16(0x0226) < rd16(0x5B91 + (uint32_t)x)) return;  // never run
  for(;;) {
    const uint16_t kind = rd16(0x5B94 + (uint32_t)x), slot = rd8(0x5B93 + (uint32_t)x);
    if(kind == 0) {
      wr16(0x45EC, (uint16_t)(slot << 1));
    } else {
      const bool high = (kind & 0xFF00) != 0;
      const uint16_t t = bus_rd16((high ? 0x86AC36 : 0x86AE36) + (uint32_t)(uint16_t)(kind << 1));
      if(!t) {
        x = (uint16_t)(kind << 1);  // the quirk: the original moves on from X = 2 * kind here
      } else {
        wr16(DP + 0x14, bus_rd16((high ? 0x860022 : 0x86001E) + (uint32_t)t));
        wr16(DP + 0x16, bus_rd16((high ? 0x860024 : 0x860020) + (uint32_t)t));
        const uint16_t base = bus_rd16(0x860000 + (uint32_t)t);
        wr16(DP + 0x14, (uint16_t)(rd16(0x6591 + (uint32_t)base) + rd16(DP + 0x14)));
        wr16(DP + 0x18, (uint16_t)(slot << 5));
        wr16(DP + 0x1A, (uint16_t)(slot << 1));
        // the kinds with a high byte keep their slots in another table
        wr16((high ? 0x495C : 0x45EC) + (uint32_t)(uint16_t)((kind & 0x00FF) << 1), rd16(DP + 0x1A));
        const uint16_t src = rd16(DP + 0x14);
        wr16(DP + 0x14, 0x007F);
        upload_vram_now(rd16(DP + 0x18), 0x7F, src, rd16(DP + 0x16));
      }
    }
    x = (uint16_t)(x + 5);
    wr16(DP + 0xA0, x);
    if(rd16(0x0226) < rd16(0x5B91 + (uint32_t)x)) return;
  }
}

// TG3000_ROUTINE($8A:9AA5) results_text_build
void results_text_build(void) {
  wr16(DP + 0x54, 0);
  variant_lines();
  player_names_put();
  text_lines_copy();
}

// TG3000_ROUTINE($8A:9CB0) player_in_top5
bool player_in_top5(void) {
  slot_order_fill();
  const uint16_t dc = rd16(0x03DC);
  const bool two = (dc & 0x000F) && (dc & 0x0030) != 0x0010;
  bool found = false;
  for(uint32_t x = 0; x < 0x0A && !found; x += 2) {
    const uint16_t slot = rd16(0x0D00 + x);
    found = slot == 0 || (two && slot == 1);
  }
  clear_041c();
  return !found;
}

// TG3000_ROUTINE($80:F6FD) title_scroll_start
void title_scroll_start(void) {
  wr16(0x0198, 0);
  wr16(0x0196, 0xFF80);
  wr16(DP + 0x3E, (uint16_t)(rd16(DP + 0x3E) + 1));
}

// TG3000_ROUTINE($80:8199) save_data_seal
void save_data_seal(void) {
  uint16_t sum = 0;
  for(uint32_t x = 0x78; x != 0; x--) sum = (uint16_t)(sum + rd8(0x0367 + x));
  wr16(0x0114, sum);
  wr16(0x0366, (uint16_t)(sum ^ 0x7645));
  for(uint32_t i = 0; i < 0x7A; i++) wr8(0xFA3B + i, rd8(0x0366 + i));
}

// TG3000_ROUTINE($8A:C355) money_skip_bonus
void money_skip_bonus(bool* overflow) {
  if(!(rd16(0x03DC) & 0x1000)) return;
  bool c = false;
  const uint16_t lo = adc_decimal16(rd16(0x038E), 0x5000, &c, overflow);
  wr16(0x038E, lo);
  wr16(0x03BD, lo);
  const uint16_t hi = adc_decimal16(rd16(0x0390), 0x0007, &c, overflow);
  wr16(0x0390, hi);
  wr16(0x03BF, hi);
}

// TG3000_ROUTINE($8A:C2DD) track_snap
uint16_t track_snap(bool* overflow) {
  uint32_t list;
  switch(rd16(0x03DC) & 0x0300) {
    case 0x0100: list = 0x8ACADE; break;
    case 0x0200: list = 0x8ACB01; break;
    default:     list = 0x8ACAC4; break;
  }
  uint16_t x = 0;
  for(;;) {
    const uint16_t d = (uint16_t)(bus_rd8(list + x) - rd16(0x0444));
    if(d == 0 || !(d & 0x8000)) break;
    x = (uint16_t)(x + 1);
    money_skip_bonus(overflow);
  }
  wr16(0x0444, bus_rd8(list + x));
  return x;
}

// TG3000_ROUTINE($81:F48B) effect_request_p1
bool effect_request_p1(uint16_t a, uint16_t x, uint16_t y) { return func_83FE9A(0x0200, a, x, y); }

// TG3000_ROUTINE($8A:8600) anim_tiles_put
void anim_tiles_put(uint16_t x) {
  static const uint16_t rows[3] = {0x0496, 0x0516, 0x0596};
  for(uint32_t r = 0; r < 3; r++)
    for(uint32_t k = 0; k < 5; k++)
      video_write_vram_word((uint16_t)(rows[r] + k), rd16(0x8000 + (uint32_t)(uint16_t)(x + 2 * (5 * r + k))));
}

// TG3000_ROUTINE($8A:85AF) tile_anim_step
void tile_anim_step(void) {
  const uint16_t mode = rd16(DP + 0x42);
  if(mode == 0) return;
  if(mode == 1) {
    const uint16_t f = (uint16_t)((rd16(DP + 0x40) + 1) & 0x0007);
    wr16(DP + 0x40, f);
    if(f == 1) anim_tiles_put(0x005A);
    else if(f == 3) anim_tiles_put(0x0078);
    else if(f == 5) anim_tiles_put(0x003C);
    return;
  }
  const uint16_t f = (uint16_t)((rd16(DP + 0x40) + 1) & 0x000F);
  wr16(DP + 0x40, f);
  anim_tiles_put((f & 0x0002) ? 0x0000 : 0x001E);
}

// The row's track object, by its kind (the handler table at $81:7FFE + 2 * kind).
static void track_row_object(uint16_t kind) {
  if(bus_rd16(0x817FFE + (uint32_t)(uint16_t)(kind << 1)) == 0xE7DA) track_entry_bounds();
  else far_7fff_4();
}

// `v` moved toward the limit: kept while v - limit is negative, else the limit.
static uint16_t row_clip(uint16_t v, uint16_t limit) { return ((uint16_t)(v - limit) & 0x8000) ? v : limit; }

// TG3000_ROUTINE($81:E631) track_rows_build
void track_rows_build(void) {
  const uint16_t n = rd16(0x019A);
  wr16(DP + 0x1E, (uint16_t)(n << 1));
  // ASL, ASL, SBC #3: the carry of the second shift (bit 14 of n) is the SBC's
  wr16(DP + 0x14, (uint16_t)((n << 2) - 3 - ((n & 0x4000) ? 0 : 1)));
  wr16(DP + 0x16, 0xC001);
  wr16(DP + 0x1A, 0xC001);
  wr16(DP + 0x18, 0x3FFF);
  wr16(DP + 0x1C, 0x3FFF);
  const int16_t first = (int16_t)((((uint16_t)(n << 2) >> 8) & 0x00FF) - 1);
  wr16(DP + 0x2A, (uint16_t)((uint16_t)dsp4_multiply(first, 10) + rd16(DP + 0x98)));
  wr16(DP + 0x30, 0x0040);
  wr16(DP + 0x20, 0x00DB);
  wr16(DP + 0x22, 0x00DB);
  uint16_t x;
  do {
    const uint16_t y = bus_rd16(0x840004 + (uint32_t)rd16(DP + 0x2A)) == 0xF8F3 ? 2 : 0;
    wr16(DP + 0x20, rd16(0x1F28 + (uint32_t)y));
    wr16(DP + 0x22, rd16(0x1F28 + (uint32_t)y));
    wr16(DP + 0x30, (uint16_t)(rd16(DP + 0x30) - 1));
    if(rd16(DP + 0x30) == 0) {
      wr16(DP + 0x2A, (uint16_t)(rd16(DP + 0x2A) - 0x000A));
      wr16(DP + 0x30, 0x0040);
    }
    wr16(DP + 0x16, (uint16_t)-row_clip((uint16_t)(0x0020 - rd16(DP + 0x16)), rd16(DP + 0x20)));
    wr16(DP + 0x18, row_clip((uint16_t)(rd16(DP + 0x18) + 0x0008), rd16(DP + 0x20)));
    wr16(DP + 0x1A, (uint16_t)-row_clip((uint16_t)(0x0008 - rd16(DP + 0x1A)), rd16(DP + 0x22)));
    wr16(DP + 0x1C, row_clip((uint16_t)(rd16(DP + 0x1C) + 0x0020), rd16(DP + 0x22)));
    const uint16_t kind = rd8(0x8CB4 + (uint32_t)rd16(DP + 0x14));
    if(kind) {
      track_row_object(kind);
      wr16(DP + 0x28, (uint16_t)(rd16(DP + 0x18) - rd16(DP + 0x26)));
      if((uint16_t)((uint16_t)(rd16(DP + 0x24) - rd16(DP + 0x16)) - rd16(DP + 0x28)) & 0x8000) {
        if((uint16_t)(rd16(DP + 0x16) - rd16(DP + 0x26)) & 0x8000) wr16(DP + 0x16, rd16(DP + 0x26));
      } else if((uint16_t)(rd16(DP + 0x24) - rd16(DP + 0x18)) & 0x8000) {
        wr16(DP + 0x18, rd16(DP + 0x24));
      }
    }
    const int16_t edge[4] = {(int16_t)rd16(DP + 0x16), (int16_t)rd16(DP + 0x18), (int16_t)rd16(DP + 0x1A),
                             (int16_t)rd16(DP + 0x1C)};
    uint16_t a = dsp4_row(edge);
    wr16(DP + 0x14, (uint16_t)(rd16(DP + 0x14) - 4));
    x = rd16(DP + 0x1E);
    wr16(DP + 0x28, a);
    if(a & 0x8000) {
      a = (uint16_t)(a + 0x1000);
      wr16(DP + 0x28, a);
    }
    if(a & 0x0800) {
      wr16(DP + 0x24, (uint16_t)(a & 0xF0FF));
      a = (uint16_t)(((rd16(DP + 0x28) + 0x0100) & 0x0F00) | rd16(DP + 0x24));
      wr16(DP + 0x28, a);
    }
    if(a & 0x0080) {
      wr16(DP + 0x24, (uint16_t)(a & 0xFF0F));
      a = (uint16_t)(((rd16(DP + 0x28) + 0x0010) & 0x00F0) | rd16(DP + 0x24));
      wr16(DP + 0x28, a);
    }
    if(a & 0x0008) {  // never run; it leaves D+$28 as it was
      wr16(DP + 0x24, (uint16_t)(a & 0xFFF0));
      a = (uint16_t)(((rd16(DP + 0x28) + 0x0001) & 0x000F) | rd16(DP + 0x24));
    }
    wr16(0xD1B1 + (uint32_t)x, a);
    x = (uint16_t)(x - 2);
    wr16(DP + 0x1E, x);
  } while(x != 0);
  const uint16_t to = (uint16_t)((n << 1) + 0xD1B3);
  for(uint16_t i = 0; i < 0x10; i++) wr8((uint16_t)(to + i), rd8((uint16_t)(0xD1B3 + i)));  // MVN, upwards
}

// TG3000_ROUTINE($82:ADC2) track_block_triple
void track_block_triple(uint16_t dp) {
  uint16_t end;
  decompress(dp, 0x99, 0x97E4, 0x7F, 0x0000, &end);
  const uint16_t from = (uint16_t)dsp4_multiply((int16_t)rd16(0x047B), 0x0190);
  for(uint16_t i = 0; i < 0x0190; i++) wr8(0x217C + (uint32_t)i, rd8(0x10000 + (uint32_t)(uint16_t)(from + i)));
  for(uint16_t i = 0; i < 0x0190; i++) wr8(0x230C + (uint32_t)i, rd8(0x217C + (uint32_t)i));
  for(uint16_t i = 0; i < 0x0190; i++) wr8(0x249C + (uint32_t)i, rd8(0x217C + (uint32_t)i));
}

// One player's palette row: 8 words from $7F:(*from) at `to` + 4, and 7 from $85:A1EC +
// *pick at `to`, `to` + 2 and `to` + $14-$1C; both sources move on by $10.
static void palette_row(uint16_t* from, uint16_t* pick, uint32_t to) {
  for(uint32_t k = 0; k < 16; k += 2) wr16(to + 4 + k, rd16(0x10000 + (uint32_t)(uint16_t)(*from + k)));
  *from = (uint16_t)(*from + 0x10);
  static const uint8_t at[7] = {0x00, 0x02, 0x14, 0x16, 0x18, 0x1A, 0x1C};
  for(uint32_t k = 0; k < 7; k++) wr16(to + at[k], bus_rd16(0x85A1EC + (uint32_t)(uint16_t)(*pick + 2 * k)));
  *pick = (uint16_t)(*pick + 0x10);
}

// TG3000_ROUTINE($82:A672) track_palettes_load
void track_palettes_load(uint16_t dp) {
  uint16_t end;
  decompress(dp, 0x88, 0x929A, 0x7F, 0x0000, &end);
  const uint16_t first = (uint16_t)dsp4_multiply((int16_t)rd16(0x047B), 0x00E0);
  uint16_t from1 = first, from2 = first, pick1 = 0, pick2 = 0;
  const uint16_t e4 = rd16(0x01E4);
  if(!(e4 & 0x8000) && e4 != 0) pick1 = pick2 = 0x0070;
  if(bus_rd16(0x839583 + (uint32_t)(uint16_t)(rd16(0x047B) << 1)) == 0) {
    if(rd8(0x039C)) {
      from1 = (uint16_t)(from1 + 0x70);
      pick1 = 0x00E0;
    }
    if(rd8(0x03CB)) {  // never run
      from2 = (uint16_t)(from2 + 0x70);
      pick2 = 0x00E0;
    }
  }
  for(uint32_t row = 0, y = 0; row < 7; row++, y += 0x20) {
    // the original keeps each source in D+$16/$1A/$18/$1C, written back after its row
    palette_row(&from1, &pick1, 0x6A55 + y);
    palette_row(&from2, &pick2, 0x6B55 + y);
  }
  wr16(dp + 0x16, from1);
  wr16(dp + 0x1A, from2);
  wr16(dp + 0x18, pick1);
  wr16(dp + 0x1C, pick2);
  wr16(dp + 0x14, 0);
}

// A component less the fade, 0 if it would go below.
static uint16_t fade_part(uint32_t at, uint16_t s) {
  const uint16_t v = bus_rd16(at);
  return v >= s ? (uint16_t)(v - s) : 0;
}

static uint16_t swap_bytes(uint16_t v) { return (uint16_t)(v << 8 | v >> 8); }

// TG3000_ROUTINE($80:FAEE) palette_fade_60
void palette_fade_60(uint16_t level) {
  const uint16_t s = (uint16_t)(bus_rd16(0x81D9AC + (uint16_t)(level << 1)) & 0x001F);
  wr16(DP + 0x14, s);
  for(uint32_t x = 0; x < 0x20; x += 2) {
    wr16(DP + 0x16, fade_part(0x81DB4C + x, s));
    wr16(DP + 0x16, (uint16_t)((swap_bytes(fade_part(0x81DB6C + x, s)) >> 3) | rd16(DP + 0x16)));
    wr16(0x0500 + x, (uint16_t)((uint16_t)(swap_bytes(fade_part(0x81DB8C + x, s)) << 2) | rd16(DP + 0x16)));
  }
  video_upload_cgram(0x60, 0x810500, 0x0020);
}

// TG3000_ROUTINE($8A:9C69) hdma_coldata_setup
void hdma_coldata_setup(void) {
  hdma_table_0d00_init();
  dma_hdma_setup(2, 0x00, 0x32, 0x810D00);
}

// TG3000_ROUTINE($8A:8787) hdma_title_setup
void hdma_title_setup(void) {
  dma_hdma_setup(1, 0x00, 0x05, 0x8A88DE);
  dma_hdma_setup(2, 0x00, 0x2C, 0x8A88E7);
  dma_hdma_setup(4, 0x00, 0x0B, 0x8A88F0);
  dma_hdma_setup(3, 0x00, 0x07, 0x8A88F9);
  for(uint32_t i = 0; i < 13; i++) wr8(0x0D00 + i, bus_rd8(0x8A8902 + i));  // MVN
  dma_hdma_setup(5, 0x02, 0x0E, 0x810D00);
  dma_hdma_set_indirect_bank(5, 0x81);
  for(uint32_t i = 0; i < 13; i++) wr8(0x0D20 + i, bus_rd8(0x8A890F + i));  // MVN
  dma_hdma_setup(7, 0x02, 0x0D, 0xA10D20);
  dma_hdma_set_indirect_bank(7, 0xA1);
}

// TG3000_ROUTINE($8A:8DBF) palette_load_70
void palette_load_70(uint16_t pick) {
  const uint16_t p = bus_rd16(0x8384B1 + (uint32_t)pick);
  video_set_color(0, (uint16_t)(bus_rd8(0x7F0000 + (uint32_t)p) | bus_rd8(0x7F0001 + (uint32_t)p) << 8));
  wr16(DP + 0x72, bus_rd16(0x7F0000 + (uint32_t)p));
  upload_cgram_now(0x70, 0x7F, p, 0x0020);
}

// TG3000_ROUTINE($8A:C1C6) text_screen_init
void text_screen_init(uint16_t str) {
  wr16(0x0D7C, (uint16_t)(rd16(0x0D7C) + 1));
  wr16(DP + 0x3E, 0x0000);
  wr16(DP + 0x40, 0x001F);
  wr16(DP + 0x68, 0);
  wr16(DP + 0x86, 0x0001);
  wr16(DP + 0x3C, 0x0003);
  for(uint16_t i = 0; i <= 0x60; i++) video_write_vram_word((uint16_t)(0x1000 + i), bus_rd16(0x8AC8E4 + 2u * i));
  for(uint32_t i = 0; i < 0x1C; i++) wr16(0x10184 + 2 * i, 0x0502);
  text_print_centered(0x8A0000 | str);
  for(uint32_t i = 0; i < 0x80; i++) wr16(0x10080 + 2 * i, (uint16_t)(rd16(0x10080 + 2 * i) & 0xE3FF));
}

// TG3000_ROUTINE($8A:A06D) hdma_window_setup
void hdma_window_setup(void) {
  hdma_window_tables_init();
  dma_hdma_setup(7, 0x02, 0x0F, 0x810D00);
  dma_hdma_setup(1, 0x00, 0x32, 0x810D20);
}

// TG3000_ROUTINE($8A:BACB) text_tiles_put_0
void text_tiles_put_0(void) {
  for(uint16_t i = 0; i <= 0x60; i++) video_write_vram_word(i, bus_rd16(0x8AC8E4 + 2u * i));
}

// TG3000_ROUTINE($8A:C24B) text_palettes_load
void text_palettes_load(void) {
  for(uint8_t i = 0; i < 0x30; i++) video_set_color((uint8_t)(0x20 + i), 0);
  upload_cgram_now(0x80, 0x8A, 0xC964, 0x0020);
  upload_cgram_now(0x60, 0x8A, 0xC984, 0x0040);
}

// A 32-bit word at `at` (low word first).
static uint32_t rd32w(uint32_t at) { return rd16(at) | (uint32_t)rd16(at + 2) << 16; }

// An object's car slot: by the $81:FE16 table in modes where $03DC & 3 is set (never run),
// else slot 0 for object 0 and none ($FFFF) for the others.
static uint16_t contact_slot(uint16_t obj) {
  if(rd16(0x03DC) & 0x0003) return bus_rd16(0x81FE16 + (uint32_t)(uint16_t)((obj / 0x35) << 1));
  return obj ? 0xFFFF : 0x0000;
}

// The original's division: the speed at `from` (+$14, 32 bits) times 2 over the slot's
// $022B, 40 shift-and-subtract steps through D+$20/$22 and A; the result goes to the
// slot's $0216/$0218.
static void contact_speed(uint16_t from, uint16_t slot) {
  wr16(DP + 0x24, rd16(0x022B + (uint32_t)slot));
  const uint16_t lo = rd16(0xFAC9 + (uint32_t)from), hi = rd16(0xFACB + (uint32_t)from);
  uint16_t q0 = (uint16_t)(lo << 1), q1 = (uint16_t)(hi << 1 | lo >> 15), a = (uint16_t)(hi >> 15);
  const uint16_t d = rd16(DP + 0x24);
  for(int i = 0; i < 0x28; i++) {
    bool c = a >= d;
    if(c) a = (uint16_t)(a - d);
    const bool c0 = (q0 & 0x8000) != 0;
    q0 = (uint16_t)(q0 << 1 | c);
    const bool c1 = (q1 & 0x8000) != 0;
    q1 = (uint16_t)(q1 << 1 | c0);
    a = (uint16_t)(a << 1 | c1);
  }
  wr16(DP + 0x20, q0);
  wr16(DP + 0x22, q1);
  wr16(0x0216 + (uint32_t)slot, q0);
  wr16(0x0218 + (uint32_t)slot, q1);
}

// TG3000_ROUTINE($81:F252) car_pair_contact
bool car_pair_contact(uint16_t x, uint16_t y) {
  if((rd16(0xFADE + (uint32_t)y) | rd16(0xFADE + (uint32_t)x)) & 0x0088) return false;
  const uint16_t d = (uint16_t)(rd16(0xFABF + (uint32_t)y) - rd16(0xFABF + (uint32_t)x));
  if(!(d & 0x8000) ? !((uint16_t)(d - 0x0018) & 0x8000) : ((uint16_t)(d - 0xFFE8) & 0x8000)) return false;
  const uint16_t e = (uint16_t)(rd16(0xFABB + (uint32_t)y) - rd16(0xFABB + (uint32_t)x));
  if(!(e & 0x8000) ? !((uint16_t)(e - 0x0040) & 0x8000) : ((uint16_t)(e - 0xFFC0) & 0x8000)) return false;
  wr16(DP + 0x14, x);
  wr16(DP + 0x16, y);
  wr16(DP + 0x18, contact_slot(x));
  wr16(DP + 0x1A, contact_slot(y));
  const uint32_t gap = rd32w(0xFAC1 + (uint32_t)y) - rd32w(0xFAC1 + (uint32_t)x);
  wr16(DP + 0x1C, (uint16_t)gap);
  wr16(DP + 0x1E, (uint16_t)(gap >> 16));
  const uint16_t sx = rd16(DP + 0x18), sy = rd16(DP + 0x1A);
  if((gap - 0x60) & 0x80000000u) {  // y less than $60 ahead: x pushed beside it
    const bool below = ((uint16_t)(rd16(0xFAD5 + (uint32_t)y) - rd16(0xFAD5 + (uint32_t)x)) & 0x8000) != 0;
    const uint16_t side = (uint16_t)(rd16(0xFABB + (uint32_t)y) + (below ? 0x0040 : (uint16_t)-0x0040));
    wr16(0xFABB + (uint32_t)x, side);
    if(!(sx & 0x8000) && !(rd16(0x0244 + (uint32_t)sx) & 0x0004))
      wr16(0x0206 + (uint32_t)sx, (uint16_t)(side + rd16(0x029C + (uint32_t)sx)));
    return false;  // the original leaves this way with the carry clear, as a miss
  }
  if(!(sy & 0x8000)) contact_speed(x, sy);
  if(!(sx & 0x8000)) {
    if(!(rd16(0x0244 + (uint32_t)sx) & 0x0004)) wr16(0x027C + (uint32_t)sx, (uint16_t)(rd16(0x027C + (uint32_t)sx) & 0xFFFB));
    contact_speed(y, sx);
  }
  const uint16_t vx_lo = rd16(0xFAC9 + (uint32_t)x), vx_hi = rd16(0xFACB + (uint32_t)x);
  wr16(0xFAC9 + (uint32_t)x, rd16(0xFAC9 + (uint32_t)y));
  wr16(0xFAC9 + (uint32_t)y, vx_lo);
  wr16(0xFACB + (uint32_t)x, rd16(0xFACB + (uint32_t)y));
  wr16(0xFACB + (uint32_t)y, vx_hi);
  const uint32_t was = rd32w(0xFAB5 + (uint32_t)x);
  uint32_t now = rd32w(0xFAB5 + (uint32_t)y) - 0x60;
  if((now - rd32w(0xFAC1 + (uint32_t)x)) & 0x80000000u) now = rd32w(0xFAC1 + (uint32_t)x);  // never run
  wr16(0xFAB5 + (uint32_t)x, (uint16_t)now);
  wr16(0xFAB7 + (uint32_t)x, (uint16_t)(now >> 16));
  const uint32_t drop = was - now;
  wr16(DP + 0x1C, (uint16_t)drop);
  wr16(DP + 0x1E, (uint16_t)(drop >> 16));
  if(!(sx & 0x8000) && !(rd16(0x0244 + (uint32_t)sx) & 0x0001)) {
    const uint32_t v = rd32w(0x0225 + (uint32_t)sx) - drop;
    wr16(0x0225 + (uint32_t)sx, (uint16_t)v);
    wr16(0x0227 + (uint32_t)sx, (uint16_t)(v >> 16));
  }
  uint16_t m = 0x0004;
  if(!(sx & 0x8000)) m &= rd16(0x0244 + (uint32_t)sx);
  if(!(sy & 0x8000)) m &= rd16(0x0244 + (uint32_t)sy);
  if(!(m & 0x0004)) effect_request_p1(0xFF24, 0x8000, y);
  return true;
}

// TG3000_ROUTINE($83:FC65) track_object_test_sides
bool track_object_test_sides(uint16_t dp) {
  const uint16_t x = track_object_x();
  const uint16_t sel = (uint16_t)((rd8(0x8CB5 + (uint32_t)rd16(0x011A)) & 0x30) >> 4);
  if(sel == 1) {
    if(func_83FD8A(dp, x) || func_83FDCD()) return true;
    wr16(0x0128, (uint16_t)-x);
    return func_83FD8A(dp, (uint16_t)-x) || func_83FDCD();
  }
  // x, -x, -x - $80, x + $80: each step works on $0128
  for(int step = 0; step < 4; step++) {
    if(step == 1 || step == 3) wr16(0x0128, (uint16_t)-rd16(0x0128));
    if(step == 2) wr16(0x0128, (uint16_t)(rd16(0x0128) - 0x0080));
    const bool hit = sel == 2 ? func_83FDCD() : func_83FD8A(dp, rd16(0x0128));
    if(hit) return true;
  }
  return false;
}

// A queue word at $7F:`at`.
static uint16_t q7f(uint16_t at) { return rd16(0x10000 + (uint32_t)at); }

// One 2-tile sprite (16 pixels tall) at D+$CA: tiles `top` and `bottom`, both | D+$16.
static void ticker_sprite(uint16_t top, uint16_t bottom) {
  const uint32_t o = rd16(DP + 0x74);
  wr16(0x0902 + o, (uint16_t)(top | rd16(DP + 0x16)));
  wr16(0x0906 + o, (uint16_t)(bottom | rd16(DP + 0x16)));
  wr16(0x0900 + o, rd16(DP + 0xCA));
  wr16(0x0904 + o, (uint16_t)(rd16(DP + 0xCA) + 0x0800));
  wr16(0x0B01 + o, 0);
  wr16(0x0B05 + o, 0);
  wr16(DP + 0x74, (uint16_t)(o + 8));
}

// The end: the marker while D+$E1 is $1E or more, and its countdown.
static void ticker_end(void) {
  if(rd16(DP + 0xE1) & 0x8000) return;
  if(q7f((uint16_t)(rd16(DP + 0x5E) + 0x1A)) != 0xFFFF) wr16(DP + 0xE1, 0x003B);
  uint16_t e = rd16(DP + 0xE1);
  if(e >= 0x001E) {
    uint16_t a = (uint16_t)(e - 0x0020);
    if(a & 0x8000) a = 0;
    wr16(DP + 0x16, (uint16_t)((((a >> 1) & 0x000E) << 8) + 0x3200));
    const uint32_t o = rd16(DP + 0x74);
    wr16(0x0902 + o, (uint16_t)(rd16(DP + 0x16) + bus_rd16(0x81CDA1)));
    wr16(0x0906 + o, (uint16_t)(bus_rd16(0x81CDA3) + rd16(DP + 0x16)));
    wr16(0x0900 + o, rd16(DP + 0xCA));
    wr16(0x0904 + o, (uint16_t)(rd16(DP + 0xCA) + 0x0800));
    wr16(0x0B01 + o, 0);
    wr16(0x0B05 + o, 0);
    wr16(DP + 0x74, (uint16_t)(o + 8));
  }
  e = (uint16_t)(rd16(DP + 0xE1) - 1);
  wr16(DP + 0xE1, (e & 0x8000) ? 0x003B : e);
}

// TG3000_ROUTINE($80:EBF7) ticker_draw
void ticker_draw(uint8_t bank, uint16_t x, uint16_t text) {
  const uint32_t b = (uint32_t)bank << 16;
  wr16(DP + 0xC8, x);
  wr16(DP + 0xCA, x);
  wr16(DP + 0xC6, (uint16_t)(rd16(DP + 0xC6) + rd16(DP + 0xE5)));
  wr16(DP + 0xD3, 0);
  uint16_t q = rd16(DP + 0x5E);
  for(int i = 0; i < 0x1B; i++, q = (uint16_t)(q + 2)) wr16(0x10000 + (uint32_t)q, q7f((uint16_t)(q + 2)));
  wr16(0x10000 + (uint32_t)q, 0xFFFF);
  // A quirk: after a step the loop goes on with A = the old D+$CC, not D+$C6, so it takes
  // one $100 step (more only while the counter is $100 or more)
  for(uint16_t a = rd16(DP + 0xC6); a >= 0x0100;) {
    wr16(DP + 0xC6, (uint16_t)(a - 0x0100));
    a = rd16(DP + 0xCC);
    wr16(0x10000 + (uint32_t)q, a);
    wr16(DP + 0xCC, (uint16_t)(a + 1));
  }
  wr16(DP + 0xDF, 0);
  uint16_t first = 0;
  for(uint16_t at = rd16(DP + 0x5E), n = 0x1C; n != 0; n--, at = (uint16_t)(at + 2))
    if(q7f(at) != 0xFFFF) {
      first = at;
      break;
    }
  wr16(DP + 0xCF, first);
  uint16_t y = text;
  if(rd16(DP + 0xDF) == rd16(DP + 0xCC)) {
    ticker_end();
    return;
  }
  for(;;) {
    const uint16_t w = bus_rd16(b + y);
    if(!(w & 0x00FF)) {  // a control code
      y = (uint16_t)(y + 1);
      switch(w >> 8) {
        case 0:  // the end: the pointer stays on it
          y = (uint16_t)(y - 2);
          wr16(DP + 0xC6, 0);
          goto next;
        case 1: {
          const uint16_t nx = (uint16_t)(bus_rd16(b + (uint32_t)(uint16_t)(y + 1)) + rd16(DP + 0xC8));
          wr16(DP + 0xCA, nx);
          wr16(DP + 0xC8, nx);
          y = (uint16_t)(y + 3);
          continue;
        }
        case 2: {  // never run
          const uint16_t df1 = (uint16_t)(rd16(DP + 0xDF) + 1);
          if(df1 != rd16(DP + 0xCC)) {
            if((uint16_t)(df1 + 1) != rd16(DP + 0xCC)) {
              y = (uint16_t)(y + 3);
              continue;
            }
            wr16(DP + 0xCC, (uint16_t)(rd16(DP + 0xCC) - 1));
          }
          if(rd16(DP + 0xD1) == 0) wr16(DP + 0xD1, bus_rd16(b + (uint32_t)(uint16_t)(y + 1)));
          wr16(DP + 0xD1, (uint16_t)(rd16(DP + 0xD1) - 1));
          if(rd16(DP + 0xD1) != 0) {
            wr16(0x10000 + (uint32_t)(uint16_t)(rd16(DP + 0x5E) + 0x36), 0xFFFF);
            ticker_end();
            return;
          }
          wr16(DP + 0xCC, (uint16_t)(rd16(DP + 0xCC) + 1));
          y = (uint16_t)(y + 3);
          continue;
        }
        case 3:  // never run
          wr16(DP + 0xE1, (uint16_t)(rd16(DP + 0xE1) | 0x8000));
          y = (uint16_t)(y + 1);
          continue;
        case 4:
          wr16(DP + 0xD3, bus_rd16(b + (uint32_t)(uint16_t)(y + 1)));
          y = (uint16_t)(y + 3);
          continue;
        default:  // 5
          wr16(DP + 0xD3, 0);
          y = (uint16_t)(y + 1);
          continue;
      }
    }
    if(rd16(DP + 0xD3) != 0 && (uint16_t)(rd16(DP + 0xDF) + 1) < q7f(rd16(DP + 0xCF))) {
      const uint16_t d3 = rd16(DP + 0xD3);
      const uint16_t tile = bus_rd16(0x81D2E3 + (uint32_t)d3);
      wr16(DP + 0xD3, (uint16_t)(d3 + 2));
      const uint32_t o = rd16(DP + 0x74);
      wr16(0x0902 + o, tile);
      wr16(0x0900 + o, rd16(DP + 0xCA));
      wr16(DP + 0xCA, (uint16_t)(rd16(DP + 0xCA) + 0x0010));
      wr16(0x0B01 + o, 0x0100);
      wr16(DP + 0x74, (uint16_t)(o + 4));
      wr16(DP + 0xDF, (uint16_t)(rd16(DP + 0xDF) + 2));
      y = (uint16_t)(y + 2);
      continue;
    }
    wr16(DP + 0x16, 0x3200);
    uint16_t at = rd16(DP + 0xCF);
    if(rd16(DP + 0xDF) == q7f(at)) {
      wr16(DP + 0x16, (uint16_t)(((((uint16_t)(at - rd16(DP + 0x5E)) >> 2) & 0x000E) << 8) + 0x3200));
      do at = (uint16_t)(at + 2); while(q7f(at) == 0xFFFF);
      wr16(DP + 0xCF, at);
    }
    const uint16_t c = w & 0x00FF;
    uint16_t t;
    if(c >= 0x30 && c < 0x3A) t = (uint16_t)(c - 0x16);
    else if(c >= 0x41 && c < 0x5B) t = (uint16_t)(c - 0x41);
    else if(c == 0x20) goto space;
    else {  // never run: a mark from the table at $80:EDE7 ($24-$32), or $33
      t = 0x24;
      while(t != 0x33 && bus_rd8(0x80EDC3 + (uint32_t)t) != c) t++;
    }
    wr16(DP + 0x14, (uint16_t)(t << 2));
    ticker_sprite(bus_rd16(0x81CCD9 + (uint32_t)(uint16_t)(t << 2)), bus_rd16(0x81CCDB + (uint32_t)(uint16_t)(t << 2)));
  space:
    wr16(DP + 0xCA, (uint16_t)(rd16(DP + 0xCA) + 8));
  next:
    y = (uint16_t)(y + 1);
    wr16(DP + 0xDF, (uint16_t)(rd16(DP + 0xDF) + 1));
    if(rd16(DP + 0xDF) == rd16(DP + 0xCC)) break;
  }
  ticker_end();
}

// TG3000_ROUTINE($8A:C15D) text_buffer_screen
void text_buffer_screen(void) {
  for(uint16_t x = 0;; x++) {
    const uint8_t c = rd8(0x17D00 + (uint32_t)x);
    if(c == 0) break;
    if(c != 0x80) {
      text_put_tile_char(c);
      continue;
    }
    wr16(DP + 0x3C, (uint16_t)(rd16(DP + 0x3C) + 1));
    wr16(DP + 0x3A, 0x0002);
    if(!((uint16_t)(rd16(DP + 0x3C) - 0x000C) & 0x8000)) {
      wr16(DP + 0x90, 0);
      if(rd16(DP + 0x3C) == 0x000C) wr16(DP + 0x3C, 0x000D);
    }
  }
  text_screen_init((rd16(0x1B37) & 0x8000) ? 0xC238 : 0xC224);  // the first: never run
}

// A palette row: 16 bytes from $86:`from` to `to` (MVN).
static void scenery_row(uint16_t from, uint16_t to) {
  for(uint16_t i = 0; i < 0x10; i++) wr8((uint16_t)(to + i), bus_rd8(0x860000 + (uint32_t)(uint16_t)(from + i)));
}

// A block of `rows` x `cols` words copied from $7F:`from` to the map at $7F:`at` (rows $40
// bytes apart), each plus D+$84 with the carry of the add before (one CLC first).
static void scenery_block(uint16_t from, uint16_t at, int rows, int cols) {
  uint32_t carry = 0;
  for(int r = 0; r < rows; r++)
    for(int c = 0; c < cols; c++) {
      const uint32_t sum = (uint32_t)rd16(0x10000 + (uint32_t)from) + rd16(DP + 0x84) + carry;
      carry = sum >> 16;
      wr16(0x10000 + (uint32_t)(uint16_t)(at + r * 0x40 + c * 2), (uint16_t)sum);
      from = (uint16_t)(from + 2);
    }
}

// TG3000_ROUTINE($80:DCBB) scenery_build
void scenery_build(void) {
  uint16_t y = bus_rd16(0x81C3D0 + (uint32_t)(uint16_t)(bus_rd8(0x84A240 + (uint32_t)rd16(0x0444)) << 1));
  wr16(DP + 0x7C, 0x8000);
  wr16(DP + 0x7E, 0x2002);
  wr16(DP + 0x7A, 0x000F);
  wr16(DP + 0x80, 0x000A);
  wr16(DP + 0x82, 0);
  wr16(DP + 0x86, 0);
  wr16(DP + 0x88, 0x0001);
  for(;; y = (uint16_t)(y + 0x0E)) {
    const uint32_t rec = 0x810000 | y;
    const uint16_t kind = bus_rd16(rec + 0xC3E8);
    if(kind == 0) break;
    const uint16_t column = bus_rd16(rec + 0xC3EA);
    uint16_t x = rd16(DP + 0x86);
    wr16(0x0882 + (uint32_t)x, (uint16_t)(kind & 0x7FFF));
    wr16(0x0884 + (uint32_t)x, (uint16_t)((uint16_t)(column << 3) + bus_rd16(0x81CAE2 + (uint32_t)(kind & 0x7FFF))));
    wr16(DP + 0x86, (uint16_t)(x + 4));
    if(kind & 0x8000) {
      if(!(rd16(DP + 0x88) & rd16(0x036A))) {
        x = rd16(DP + 0x82);
        wr16(0x0802 + (uint32_t)x, (uint16_t)(kind & 0x7FFF));
        wr16(0x0804 + (uint32_t)x, (uint16_t)((uint16_t)(column << 3) + bus_rd16(0x81CAE2 + (uint32_t)(kind & 0x7FFF))));
        wr16(0x0806 + (uint32_t)x, bus_rd16(rec + 0xC3F4));
        wr16(0x0808 + (uint32_t)x, rd16(DP + 0x88));
        wr16(DP + 0x82, (uint16_t)(x + 8));
      }
      wr16(DP + 0x88, (uint16_t)(rd16(DP + 0x88) << 1));
    }
    const uint16_t k = (uint16_t)(kind & 0x7FFF);
    if(k == 0) break;  // the table's first handler is the end
    if(k == 2) {
      const uint16_t v = (uint16_t)(column << 1);
      uint16_t at = (uint16_t)(v + (v < 0x40 ? 0xB250 : 0xCED0));  // the first: never run
      uint16_t w = 0x11DC;
      for(int r = 0; r < 6; r++)
        for(int c = 0; c < 6; c++) wr16(0x10000 + (uint32_t)(uint16_t)(at + r * 0x40 + c * 2), w++);
      wr16(0x0706, (uint16_t)(column << 3));
      wr16(0x0700, bus_rd16(rec + 0xC3F0));
      wr16(0x0702, 0);
      const uint16_t pal = bus_rd16(rec + 0xC3EC);
      for(uint16_t i = 0; i < 0x20; i++) wr8((uint16_t)(0x0580 + i), bus_rd8(0x860000 + (uint32_t)(uint16_t)(pal + i)));
      for(uint16_t i = 0; i < 0x10; i++) wr8((uint16_t)(0x0680 + i), bus_rd8(0x86DAC9 + i));
      continue;
    }
    static const uint16_t from[4][2] = {{0xBC50, 0xBC70}, {0xBC90, 0xBCB0}, {0xBCD0, 0xBCE8}, {0xBD00, 0xBD08}};
    const uint16_t a7a = rd16(DP + 0x7A);
    wr16(DP + 0x84, (uint16_t)(((a7a << 1) & 0x001C) << 8));
    const uint16_t src = from[k / 2 - 2][a7a & 1];
    const uint16_t at = func_80E0C8(column);
    if(k == 4 || k == 6) scenery_block(src, at, 4, 4);
    else if(k == 8) scenery_block(src, at, 4, 3);
    else scenery_block(src, (uint16_t)(at + 0x40), 2, 2);
    const uint16_t row = (uint16_t)((uint16_t)(rd16(DP + 0x7A) << 4) + 0x0500);
    wr16(DP + 0x84, row);
    scenery_row(bus_rd16(rec + 0xC3EC), row);
    scenery_row(bus_rd16(rec + 0xC3F2), (uint16_t)(row + 0x0100));
    uint16_t end;
    const uint16_t len = decompress_near(DP, 0x91, bus_rd16(rec + 0xC3EE), 0x7E, rd16(DP + 0x7C), &end);
    wr16(DP + 0x7C, (uint16_t)(len + rd16(DP + 0x7C)));
    const uint16_t v = (uint16_t)(column << 1);
    const uint16_t mx = (uint16_t)(v + (v < 0x40 ? 0x1310 : 0x19D0));
    uint16_t w = rd16(DP + 0x7E);
    for(int r = 0; r < 4; r++)
      for(int c = 0; c < 4; c++) wr16(0x1AE40 + (uint32_t)(uint16_t)(mx + r * 0x40 + c * 2), w++);
    wr16(DP + 0x7E, w);
    x = rd16(DP + 0x80);
    wr16(0x0700 + (uint32_t)x, bus_rd16(rec + 0xC3F0));
    wr16(0x0702 + (uint32_t)x, 0);
    wr16(0x0704 + (uint32_t)x, bus_rd16(0x81C2DA + (uint32_t)(uint16_t)(((rd16(DP + 0x7A) & 1) | kind) << 1)));
    wr16(0x0706 + (uint32_t)x, (uint16_t)(column << 3));
    wr16(0x0708 + (uint32_t)x, (uint16_t)((rd16(DP + 0x7A) << 8) & 0xFE00));
    wr16(0x070A + (uint32_t)x, 0);
    wr16(DP + 0x80, (uint16_t)(x + 0x0A));
    wr16(DP + 0x7A, (uint16_t)(rd16(DP + 0x7A) - 1));
  }
  wr16(0x0800, rd16(DP + 0x82));
  wr16(0x0880, rd16(DP + 0x86));
}

// TG3000_ROUTINE($81:F21E) car_contacts_chain
bool car_contacts_chain(uint16_t x, uint16_t y) {
  if((uint16_t)(rd16(0xFAB5 + (uint32_t)y) - rd16(0xFAB5 + (uint32_t)x)) >= 0x0060) return true;  // BCS to the RTS
  wr16(DP + 0x3C, (uint16_t)(rd16(DP + 0x3C) + 1));
  const uint16_t count = rd16(DP + 0x3C);
  bool hit = false;
  for(;;) {
    if(car_pair_contact(x, y)) {
      hit = true;
      break;
    }
    y = rd16(0xFACD + (uint32_t)y);
    if((uint16_t)(rd16(0xFAB5 + (uint32_t)y) - rd16(0xFAB5 + (uint32_t)x)) >= 0x0060) break;
    wr16(DP + 0x3C, (uint16_t)(rd16(DP + 0x3C) - 1));
    if(rd16(DP + 0x3C) == 0) break;
  }
  wr16(DP + 0x3C, count);
  return hit;
}

// The high byte of a word, sign-extended.
static uint16_t high_signed(uint16_t v) { return (uint16_t)(int16_t)(int8_t)(v >> 8); }

// TG3000_ROUTINE($80:E925) ticker_scene_step
void ticker_scene_step(void) {
  uint16_t a = rd16(DP + 0x82);
  if(a & 0x8000) {  // never run
    a = (uint16_t)(a + 0x0012);
    if(!(a & 0x8000)) a = 0;
  } else {
    a = (uint16_t)(a - 0x0012);
    if(a & 0x8000) a = 0;
  }
  wr16(DP + 0x82, a);
  const uint16_t v80 = rd16(DP + 0x80);
  const uint32_t lo = (uint32_t)rd16(DP + 0x7B) + v80;
  wr16(DP + 0x7B, (uint16_t)lo);
  wr8(DP + 0x7D, (uint8_t)(((v80 & 0x8000) ? 0xFF : 0x00) + rd8(DP + 0x7D) + (lo >> 16)));
  wr16(DP + 0x7E, (uint16_t)(rd16(DP + 0x82) + rd16(DP + 0x7E)));
  wr16(0x0026, (uint16_t)(rd16(0x0026) | 0x0002));
  const uint16_t x = (uint16_t)(rd16(DP + 0x84) & 0x03FE);
  uint16_t c = rd16(DP + 0x7C);
  for(int i = 0; i < 3; i++) c = (uint16_t)((c >> 1) | (c & 0x8000));
  wr16(DP + 0x2E, (uint16_t)(c + 0x0088));
  wr16(DP + 0x2E, (uint16_t)(high_signed(rd16(DP + 0x22)) + rd16(DP + 0x2E)));
  uint16_t row = rd16(DP + 0x7E);
  row = (uint16_t)(row << 8 | row >> 8);
  row = (uint16_t)(((row << 1) | (row >> 15)) & 0x01FF);
  if(row & 0x0100) row |= 0xFE00;  // never run
  wr16(DP + 0x30, (uint16_t)(row - 0x002E - rd16(DP + 0xE8)));
  wr16(DP + 0x30, (uint16_t)(high_signed(rd16(DP + 0x24)) + rd16(DP + 0x30)));
  wr16(DP + 0x32, bus_rd16(0x81CEA9 + (uint32_t)x));
  wr16(DP + 0x38, bus_rd16(0x81CEA9 + (uint32_t)x));
  wr16(DP + 0x34, bus_rd16(0x81CDA9 + (uint32_t)x));
  wr16(DP + 0x36, (uint16_t)-rd16(DP + 0x34));
  wr16(DP + 0x3A, rd16(DP + 0x2C));
  wr16(DP + 0x3C, 0x001C);
  const uint16_t at = (uint16_t)(0x0A10 + rd16(DP + 0xE7));
  if(!(rd16(DP + 0x46) & 0x8000)) {
    ticker_draw(0x7F, at, (uint16_t)(0x2000 + rd16(DP + 0x5E)));
  } else {
    if(!(rd16(DP + 0x46) & 0x0001)) {
      if(rd16(DP + 0x48) == 0) {
        wr16(DP + 0x98, 0);
        wr16(DP + 0x96, 0xFF80);
      } else {
        bool out;
        do {
          wr16(DP + 0x4A, (uint16_t)(rd16(DP + 0x4A) + 2));
          out = (rd16(DP + 0x48) & 1) != 0;
          wr16(DP + 0x48, (uint16_t)(rd16(DP + 0x48) >> 1));
        } while(!out);
        wr16(DP + 0x46, (uint16_t)(rd16(DP + 0x46) + 1));
        queue_7f_init(DP);
        wr16(DP + 0x4C, 0x00A0);
      }
    }
    ticker_draw(0x81, at, bus_rd16(0x81D5CF + (uint32_t)rd16(DP + 0x4A)));
    wr16(DP + 0x4C, (uint16_t)(rd16(DP + 0x4C) - 1));
    if(rd16(DP + 0x4C) == 0) wr16(DP + 0x46, (uint16_t)(rd16(DP + 0x46) + 1));
  }
  if(rd16(0x03A8 + (uint32_t)rd16(DP + 0x94)) == 0) spark_step();
  func_80EB66(DP);
  oam_finish_near(DP);
  wr16(DP + 0x86, (uint16_t)(rd16(DP + 0x86) + 1));
  const uint16_t s = rd16(DP + 0x46);
  if(!(s & 0x8000)) {
    wr16(DP + 0x46, (uint16_t)(s - 1));
    if(rd16(DP + 0x46) != 0) return;
    if(rd16(DP + 0x48) != 0 && !(rd16(0x03A8 + (uint32_t)rd16(DP + 0x94)) & 0x8000)) {
      wr16(DP + 0x46, 0x8000);
      return;
    }
    wr16(DP + 0x46, (uint16_t)(rd16(DP + 0x46) + 1));
  } else if(s != 0xFFFF) {
    return;
  }
  wr16(DP + 0x98, 0);
  wr16(DP + 0x96, 0xFF80);
}

// The window settings of the title's picture ($80:F644): windows 1 and 2 at 255-0, BG1/BG2
// masked by window 1, the line tables cleared.
static void title_windows(void) {
  wr16(DP + 0x7E, 0xFFFF);
  video_set_window_bg12(0x0A);
  wr16(0x0D00, 0);
  wr16(0x0D40, 0);
  wr16(0x0E00, 0);
  wr16(0x0E40, 0);
  video_set_window_position(0, 0xFF);
  video_set_window_position(1, 0x00);
  video_set_window_position(2, 0xFF);
  video_set_window_position(3, 0x00);
  video_set_window_bg12(0x02);
  video_set_window_bg34(0x00);
  video_set_window_logic_bg(0x00);
  video_set_window_logic_obj(0x00);
}

static uint16_t title_frame_tail(void);

// TG3000_ROUTINE($80:F5DE) title_frame_step
uint16_t title_frame_step(void) {
  static const uint16_t from[4] = {0x9180, 0x9768, 0x9D50, 0xA338}, to[4] = {0x0000, 0x02F4, 0x05E8, 0x08DC};
  const uint16_t f = rd16(DP + 0x40);
  for(int i = 0; i < 4; i++)
    if(f == 0x013A + 2 * i) video_upload_vram(to[i], 0x7E0000 | from[i], 0x0614);
  title_windows();
  return title_frame_tail();
}

// $80:F670 on: the curve from frame $15C, the frame's picture, the next frame.
static uint16_t title_frame_tail(void) {
  if(rd16(DP + 0x40) >= 0x015C) {
    wr16(DP + 0x42, (uint16_t)(rd16(DP + 0x42) + 1));
    wr16(DP + 0x30, (uint16_t)((rd16(DP + 0x42) >> 1) + 0x0040));
    wr16(DP + 0x36, 0x0800);
    window_curve_build();
  }
  uint16_t end;
  const uint16_t src = rd16(0xA920 + (uint32_t)(rd16(DP + 0x40) & 0xFFFE));
  const uint16_t len = decompress_near(DP, 0x7E, src, 0x7F, 0x0000, &end);
  wr16(DP + 0x40, (uint16_t)(rd16(DP + 0x40) + 2));
  if(rd16(DP + 0x40) == 0x017C) wr16(DP + 0x3E, (uint16_t)(rd16(DP + 0x3E) + 1));
  return len;
}

// TG3000_ROUTINE($80:F6AD) title_fade_step
uint16_t title_fade_step(void) {
  const uint16_t n = (uint16_t)(rd16(DP + 0x42) + 1);
  if(n == 0x007F) wr16(DP + 0x3E, (uint16_t)(rd16(DP + 0x3E) + 1));
  wr16(DP + 0x42, n);
  uint16_t level;
  if(n < 0x0020) {
    level = (uint16_t)((n >> 1) + 0x0040);
  } else {
    const uint16_t t = n >= 0x003F ? (uint16_t)(n - 0x003F) : 0;
    level = (uint16_t)((t >> 1) + 0x0020);
    if(level >= 0x0040) level = 0x003F;
  }
  wr16(DP + 0x30, level);
  palette_fade_60(level);
  wr16(DP + 0x36, 0x0800);
  window_curve_build();
  if(rd16(DP + 0x42) != 0x003F) return 0;
  uint16_t end;
  return decompress_near(DP, 0x7E, 0xC730, 0x7F, 0x0000, &end);
}

// TG3000_ROUTINE($80:F5CD) title_frame_begin
uint16_t title_frame_begin(void) {
  wr16(DP + 0x40, 0);
  wr16(DP + 0x3E, (uint16_t)(rd16(DP + 0x3E) + 1));
  wr16(DP + 0x42, 0x0001);
  wr16(DP + 0x1E, 0x003C);
  return title_frame_tail();
}

// TG3000_ROUTINE($80:C35E) menu_bar_draw
void menu_bar_draw(void) {
  const uint16_t at = (uint16_t)(rd16(DP + 0x4A) & 0xFF00);
  for(uint16_t x = 0;; x = (uint16_t)(x + 2)) {
    const uint16_t key = bus_rd16(0x81BE0A + (uint32_t)x), d = (uint16_t)(at - key);
    if(d & 0x8000) {
      const uint8_t step = (uint8_t)(bus_rd8(0x81BE0B + (uint32_t)x) - bus_rd8(0x81BE09 + (uint32_t)x));
      const uint16_t q = step ? (uint16_t)((uint16_t)(key - at) / step) : 0xFFFF;  // the CPU divider
      const uint8_t m = (uint8_t)(bus_rd8(0x81BE12 + (uint32_t)x) - bus_rd8(0x81BE10 + (uint32_t)x));
      video_set_m7a((int16_t)q);
      video_set_m7b((int16_t)(m << 8 | q >> 8));
      const uint16_t mid = (uint16_t)((uint32_t)((int32_t)(int16_t)q * (int8_t)m) >> 8);
      wr16(DP + 0x14, (uint16_t)(bus_rd16(0x81BE12 + (uint32_t)x) - mid));
      break;
    }
    if(d == 0) {
      wr16(DP + 0x14, bus_rd16(0x81BE12 + (uint32_t)x));
      break;
    }
  }
  uint16_t slot = rd16(DP + 0x74);
  wr16(DP + 0x18, (uint16_t)(rd16(DP + 0x4A) + 0x1D80 + rd16(DP + 0xE7)));
  wr16(DP + 0x1A, 0);
  slot = sprite_bar8(slot);
  wr16(DP + 0x18, (uint16_t)(rd16(DP + 0x18) + 0x0600));
  wr16(DP + 0x1A, 0x8000);
  slot = sprite_bar8(slot);
  wr16(DP + 0x74, slot);
  wr16(DP + 0x18, (uint16_t)(0x2080 + rd16(DP + 0xE7)));
  wr16(DP + 0x1A, (uint16_t)(rd16(DP + 0x48) + 1));
  static const uint16_t label[4] = {0xBDE0, 0xBDE5, 0xBDED, 0xBDF3};
  for(int i = 0; i < 4; i++) {
    wr16(DP + 0x14, 0x0100);
    wr16(DP + 0x1A, (uint16_t)(rd16(DP + 0x1A) - 1));
    if(rd16(DP + 0x1A) == 0) wr16(DP + 0x14, 0x0300);
    sprite_text_centered(0x810000 | label[i], (uint16_t)(rd16(DP + 0x18) + 0x0B00 * i));
  }
  const uint16_t key = bus_rd16(0x81BE0A + (uint32_t)(uint16_t)(rd16(DP + 0x48) << 1));
  const uint16_t d = (uint16_t)(at - key);
  if(d != 0) wr16(DP + 0x4B, (uint16_t)(rd16(DP + 0x4B) + ((d & 0x8000) ? 1 : (uint16_t)-1)));
}

// TG3000_ROUTINE($85:8000) track_tiles_load
void track_tiles_load(uint16_t track) {
  const uint16_t k = bus_rd8(0x838410 + (uint32_t)track);
  const uint16_t y = (uint16_t)(bus_rd16(0x838447 + (uint32_t)k) << 1);
  const uint16_t banks = bus_rd16(0x838597 + (uint32_t)y);
  uint16_t end;
  decompress(DP, (uint8_t)banks, bus_rd16(0x838585 + (uint32_t)y), (uint8_t)(banks >> 8), 0x0000, &end);
  static const uint16_t offsets[4][5] = {
    {0x0420, 0x0020, 0x0220, 0x0020, 0x0220},
    {0x0820, 0x0020, 0x0220, 0x0420, 0x0620},
    {0x0440, 0x0040, 0x0240, 0x0040, 0x0240},
    {0x0840, 0x0040, 0x0240, 0x0440, 0x0640},
  };
  const uint16_t handler = bus_rd16(0x858025 + (uint32_t)(uint16_t)(rd16(0x047B) << 1));
  const int layout = handler == 0x8093 ? 0 : handler == 0x80C5 ? 1 : handler == 0x80F7 ? 2 : 3;
  if(layout & 1) wr16(0x1F33, (uint16_t)(rd16(0x1F33) | 0x0080));
  else wr16(0x1F33, (uint16_t)(rd16(0x1F33) & ~0x0080));
  const uint16_t* o = offsets[layout];
  tiles_planar_upload_5fe0(o[0], k);
  upload_tiles_3300(k, o[1]);
  upload_tiles_3700(k, o[2]);
  upload_tiles_7700(k, o[3]);
  upload_tiles_7b00(k, o[4]);
  const bool rows = layout >= 2;
  row_copy_6a33((rows && rd8(0x039C)) ? 0x0020 : 0, k);  // the fourth layout's: never run
  row_copy_6b33((rows && rd8(0x03CB)) ? 0x0020 : 0, k);  // never run
}

// TG3000_ROUTINE($8A:8BC2) track_tiles_preview
void track_tiles_preview(uint16_t track) {
  const uint16_t k = bus_rd8(0x838410 + (uint32_t)track);
  const uint16_t y = (uint16_t)(bus_rd16(0x838447 + (uint32_t)k) << 1);
  const uint16_t banks = bus_rd16(0x838597 + (uint32_t)y);
  uint16_t end;
  decompress(DP, (uint8_t)banks, bus_rd16(0x838585 + (uint32_t)y), (uint8_t)(banks >> 8), 0x0000, &end);
  static const uint16_t offsets[4][3] = {
    {0x0420, 0x0000, 0x0200}, {0x0820, 0x0000, 0x0200}, {0x0440, 0x0020, 0x0220}, {0x0840, 0x0020, 0x0220},
  };
  const uint16_t handler = bus_rd16(0x8A8BED + (uint32_t)(uint16_t)(rd16(0x0444) << 1));
  const uint16_t* o = offsets[handler == 0x8C5B ? 0 : handler == 0x8C72 ? 1 : handler == 0x8C89 ? 2 : 3];
  tiles_planar_upload(o[0], k);
  upload_tiles_c280(k, o[1]);
  upload_tiles_c680(k, o[2]);
  palette_load_70(k);
}

// TG3000_ROUTINE($8A:C056) title_card_show
uint16_t title_card_show(void) {
  const uint16_t handler = rd16(0x0008);
  wr16(0x0196, 0);
  wr16(0x0198, 0);
  wr16(0x019A, 0);
  clear_dp_scratch();
  wr16(DP + 0x8E, 0);
  video_load_screen_long(0x8AC4B0);
  clear_vram();
  func_8ABA55(DP);
  decompress_to_vram(DP, 0x87, 0xF861, 0x0800);
  text_palettes_load();
  decompress_to_vram(DP, 0x87, 0xF5BC, 0x0000);
  func_8A8EEF();
  clear_tilemap_7f();
  wr16(0x0008, 0xFC5F);
  wr16(DP + 0x8E, (uint16_t)(rd16(DP + 0x8E) + 1));
  video_set_bg1_vofs(0xFFBF);
  wr16(DP + 0x42, 0x0058);
  wr16(DP + 0x44, 0x0014);
  sprite_slots_six();
  func_8AC0F2();
  wr16(0x0198, 0x0F00);
  wr16(0x0196, 0x0080);
  uint16_t frame = 0;
  for(int i = 0; i < 0xB4; i++) frame = wait_frames(1);
  wr16(0x0198, 0);
  wr16(0x0196, 0xFFC0);
  while(rd16(0x019A) != 0) platform_wait_vblank();
  wr16(0x0008, handler);
  return frame;
}

// TG3000_ROUTINE($8A:8828) vram_planes_build
void vram_planes_build(void) {
  video_set_vram_address(0x0000);
  dma_stream_setup(0, 0x00, 0x19, 0x7E8633);
  dma_stream_size(0, 0x1200);
  dma_stream_kick(0x01);
  video_set_vram_address(0x1200);
  dma_stream_setup(0, 0x00, 0x19, 0x7E9833);
  dma_stream_size(0, 0x06C0);
  dma_stream_kick(0x01);
  video_set_vram_address(0x0000);
  video_set_vram_increment(0x00);
  dma_stream_setup(0, 0x08, 0x18, 0x8A88DD);  // a fixed source
  dma_stream_size(0, 0x4000);
  dma_stream_kick(0x01);
  for(uint32_t i = 0; i < 6; i++) {
    dma_stream_setup(0, 0x00, 0x18, 0x7E80BA + 15 * i);
    dma_stream_size(0, 0x000F);
    video_set_vram_address((uint16_t)(0x0308 + 0x80 * i));
    dma_stream_kick(0x01);
  }
  video_set_vram_increment(0x80);
}

// The road shift's front ($82:F1C1, $82:EFDA): D+$22 = the word at $020B; the 32-bit sum
// of the two DSP-4 multiplies, negated into $020A:$020C; D+$24 = -$1EF6.
static void road_shift_front(void) {
  wr16(DP + 0x22, rd16(0x020B));
  const uint32_t x = rd16(DP + 0x16);
  const uint16_t near = rd16(0x3DEC + x), mid = rd16(0x3DEE + x);
  int32_t p = dsp4_multiply((int16_t)(uint16_t)((uint16_t)(mid - near) << 2),
                            (int16_t)(uint16_t)((uint16_t)(0x8000 - rd16(DP + 0x14)) >> 1));
  wr16(DP + 0x1A, (uint16_t)p);
  wr16(DP + 0x1C, (uint16_t)((uint32_t)p >> 16));
  uint16_t width;
  const uint16_t flags = rd16(0x3D6C + x);
  if((flags & 0x8000) && !(flags & 0x0002))
    width = (uint16_t)(mid - (uint16_t)(int16_t)(int8_t)rd8(0x3AEF + x) - near);
  else
    width = (uint16_t)(rd16(0x3DF0 + x) - mid);
  p = dsp4_multiply((int16_t)(uint16_t)(width << 1), (int16_t)rd16(DP + 0x14));
  const uint32_t lo = (uint32_t)rd16(DP + 0x1A) + (uint16_t)p;
  wr16(DP + 0x1A, (uint16_t)lo);
  wr16(DP + 0x1C, (uint16_t)(rd16(DP + 0x1C) + (uint16_t)((uint32_t)p >> 16) + (lo >> 16)));
  const uint32_t sum = (uint32_t)rd16(DP + 0x1A) | (uint32_t)rd16(DP + 0x1C) << 16;
  wr16(0x020A, (uint16_t)-sum);
  wr16(0x020C, (uint16_t)((uint32_t)-sum >> 16));
  wr16(DP + 0x24, (uint16_t)-rd16(0x1EF6));
}

// The car's offset D+$1E:$20 = $1EF4:$1EF6 less $1EC2:$1EC4.
static void road_shift_offset(void) {
  const uint32_t off = ((uint32_t)rd16(0x1EF4) | (uint32_t)rd16(0x1EF6) << 16) -
                       ((uint32_t)rd16(0x1EC2) | (uint32_t)rd16(0x1EC4) << 16);
  wr16(DP + 0x1E, (uint16_t)off);
  wr16(DP + 0x20, (uint16_t)(off >> 16));
}

// The road shift's tail: $01CE gains D+$22 less the new $020B; $0204:$0206 lose the offset,
// are copied to $0200:$0202 and added to $1EF4:$1EF6.
static void road_shift_move(void) {
  wr16(0x01CE, (uint16_t)(rd16(DP + 0x22) - rd16(0x020B) + rd16(0x01CE)));
  const uint32_t at = ((uint32_t)rd16(0x0204) | (uint32_t)rd16(0x0206) << 16) -
                      ((uint32_t)rd16(DP + 0x1E) | (uint32_t)rd16(DP + 0x20) << 16);
  wr16(0x0204, (uint16_t)at);
  wr16(0x0206, (uint16_t)(at >> 16));
  wr16(0x0200, (uint16_t)at);
  wr16(0x0202, (uint16_t)(at >> 16));
  const uint32_t car = ((uint32_t)rd16(0x1EF4) | (uint32_t)rd16(0x1EF6) << 16) + at;
  wr16(0x1EF4, (uint16_t)car);
  wr16(0x1EF6, (uint16_t)(car >> 16));
}

// TG3000_ROUTINE($82:F1C1) road_shift_apply
void road_shift_apply(void) {
  road_shift_front();
  if(!(rd16(0x1E66) & 0x8000)) {
    wr16(0x020C, 0);
    wr16(DP + 0x1E, 0);
    wr16(DP + 0x20, 0);
  } else {
    road_shift_offset();
  }
  road_shift_move();
}

// TG3000_ROUTINE($82:EFDA) road_shift_settle
void road_shift_settle(void) {
  road_shift_front();
  uint16_t s24 = rd16(DP + 0x24);
  const uint16_t a206 = rd16(0x0206);
  uint16_t v;
  if((a206 ^ s24) & 0x8000) goto zero;
  if(!(((uint16_t)(a206 - s24) ^ s24) & 0x8000)) goto snap;
  {
    uint16_t v26 = a206;
    if(a206 & 0x8000) {
      s24 = (uint16_t)-s24;
      wr16(DP + 0x24, s24);
      v26 = (uint16_t)-a206;
    }
    wr16(DP + 0x26, v26);
    if(!((uint16_t)(s24 - 0x0100) & 0x8000)) {
      const uint16_t a = (uint16_t)-(uint16_t)((s24 >> 1) - v26);
      if(!((uint16_t)(a - 0x0080) & 0x8000)) goto snap;
      if((uint16_t)(a - 0xFF80) & 0x8000) goto zero;
      v = (uint16_t)(a + 0x0080);
    } else {
      // the fraction remainder * 256 / s, on the CPU divider ($FFFF and the dividend kept
      // as the remainder for a divisor of 0)
      const uint8_t d = (uint8_t)s24;
      const uint16_t r = d ? (uint16_t)(v26 % d) : v26;
      const uint16_t r8 = (uint16_t)(r << 8 | r >> 8);
      v = d ? (uint16_t)(r8 / d) : 0xFFFF;
    }
  }
  wr16(DP + 0x28, v);
  {
    int32_t p = dsp4_multiply((int16_t)v, (int16_t)rd16(0x020C));
    wr16(DP + 0x24, (uint16_t)p);
    wr16(DP + 0x26, (uint16_t)((uint32_t)p >> 16));
    wr16(0x020C, rd16(DP + 0x25));
    p = dsp4_multiply((int16_t)v, (int16_t)(uint16_t)(rd16(0x1EF6) - rd16(0x1EC4)));
    wr16(DP + 0x24, (uint16_t)p);
    wr16(DP + 0x26, (uint16_t)((uint32_t)p >> 16));
    wr16(DP + 0x1E, (uint16_t)(rd16(DP + 0x23) & 0xFF00));
    wr16(DP + 0x20, rd16(DP + 0x25));
  }
  road_shift_move();
  return;
zero:
  wr16(0x020C, 0);
  wr16(DP + 0x1E, 0);
  wr16(DP + 0x20, 0);
  wr16(0x1E66, (uint16_t)(rd16(0x1E66) & ~0x8000));
  road_shift_move();
  return;
snap:
  wr16(DP + 0x1E, rd16(0x1EF4));
  wr16(DP + 0x20, rd16(0x1EF6));
  wr16(0x1E66, (uint16_t)(rd16(0x1E66) | 0x8000));
  road_shift_offset();
  road_shift_move();
}

// A 32-bit word pair at `at` (low word first).
static uint32_t rd32pair(uint32_t at) { return rd16(at) | (uint32_t)rd16(at + 2) << 16; }

// TG3000_ROUTINE($83:F73E) follow_car_step
void follow_car_step(uint16_t dp) {
  if(!(rd16(dp + 0x44) & 0x0001)) return;  // never run
  const uint32_t car = rd32pair(0xFAB5 + (uint32_t)rd16(0x1EBA));
  uint32_t gap = car - rd32pair(dp + 0x25) - 0x40;
  wr16(0x0114, (uint16_t)gap);
  wr16(0x0116, (uint16_t)(gap >> 16));
  uint16_t step = (uint16_t)((uint16_t)(rd16(0x0114) >> 4) + 8);
  if(step >= 0x0080) step = 0x0080;
  const uint32_t pos = rd32pair(dp + 0x25) + (uint32_t)dsp4_multiply((int16_t)rd16(0x1E9E), (int16_t)step);
  wr16(dp + 0x25, (uint16_t)pos);
  wr16(dp + 0x27, (uint16_t)(pos >> 16));
  const uint32_t ahead = car - pos;  // before the $40
  gap = ahead - 0x40;
  wr16(0x0114, (uint16_t)gap);
  wr16(0x0116, (uint16_t)(gap >> 16));
  if(gap != 0 && ahead >= 0x40) return;
  const uint32_t at = car - 0x40;
  wr16(dp + 0x25, (uint16_t)at);
  wr16(dp + 0x27, (uint16_t)(at >> 16));
  wr16(dp + 0x44, (uint16_t)(rd16(dp + 0x44) & ~0x0001));
  wr16(0x1ECE, (uint16_t)(rd16(0x1ECE) - 1));
  if(rd16(0x1ECE) == 0x8000) wr16(0x1ECE, 0x0118);
}

// The tile pair of digit `d` at $83:A4AC.
static void hud_digit(uint16_t d, uint32_t lo, uint32_t hi) {
  wr16(lo, bus_rd16(0x83A4AC + 4u * d));
  wr16(hi, bus_rd16(0x83A4AE + 4u * d));
}

// The 8 rows of a bar's partial cell at $0080-$008E: `first`, then $0022 from row n on.
static void hud_bar_cell(uint16_t n, uint16_t first) {
  wr16(DP + 0x14, n);
  uint16_t a = first;
  for(int y = 0x0E; y >= 0; y -= 2) {
    wr16(DP + 0x14, (uint16_t)(rd16(DP + 0x14) - 1));
    if(rd16(DP + 0x14) == 0) a = 0x0022;
    wr16(0x0080 + (uint32_t)y, a);
  }
}

// The offset of the block func_82B7B0 masked (the X it leaves): 2 * $0158 + the word at
// $83:8C65 + ($0156 & ~1).
static uint16_t hud_block_offset(void) {
  return (uint16_t)(2 * rd16(DP + 0x58) + bus_rd16(0x838C65 + (uint32_t)(rd16(DP + 0x56) & 0xFFFE)));
}

// TG3000_ROUTINE($82:B4ED) race_hud_digits
void race_hud_digits(void) {
  if(!(rd16(0x0244) & 0x0004)) {
    uint16_t a = (uint16_t)((uint32_t)dsp4_multiply((int16_t)rd16(0x0213), 0x0160) >> 16);
    if(a & 0x8000) a = (uint16_t)-a;  // never run
    if(a >= 0x000C) a = 0x000C;
    wr16(0x003E, a);
    static const uint16_t blanks[] = {0x0042, 0x0044, 0x0046, 0x0048, 0x1E96, 0x1E98};
    for(unsigned i = 0; i < sizeof blanks / sizeof blanks[0]; i++) wr16(blanks[i], 0x0157);
    uint16_t s = rd16(0x0210);
    if(s & 0x8000) s = (uint16_t)-s;  // never run
    a = (uint16_t)((uint32_t)dsp4_multiply((int16_t)(uint16_t)(s << 3), (int16_t)(rd8(0x03A7) ? 0x4D3B : 0x3000)) >> 16);
    if(!(rd16(0x1E94) & 0x8000)) a = (uint16_t)(rd16(0x1E94) + 1);  // never run
    if(a >= 0x03E8) {  // never run
      hud_digit((uint16_t)(a / 0x03E8), 0x1E96, 0x1E98);
      a = (uint16_t)(a % 0x03E8);
    }
    if(a >= 10) {
      if(a >= 100) {  // the CPU divider
        hud_digit((uint16_t)(a / 100), 0x0042, 0x0044);
        a = (uint16_t)(a % 100);
      }
      hud_digit((uint16_t)(a / 10), 0x0046, 0x0048);
      a = (uint16_t)(a % 10);
    }
    hud_digit(a, 0x004A, 0x004C);
    wr16(0x0064, 0x0157);
    a = rd16(DP + 0x42);
    if(a >= 10) {
      if(a >= 20) {
        wr16(0x0064, bus_rd16(0x83A488));
        a = (uint16_t)(a - 20);
      } else {
        wr16(0x0064, bus_rd16(0x83A486));
        a = (uint16_t)(a - 10);
      }
    }
    wr16(0x0066, bus_rd16(0x83A484 + 2u * a));
    a = rd16(0x0258);
    if(a < 9 && (rd16(0x0297) & 0x8000)) a = 0;
    wr16(0x0074, 0x295B);
    if(a < 8) {
      wr16(0x0074, 0x295C);
      hud_bar_cell((uint16_t)(a + 1), 0x1C3E);
      wr16(0x008E, 0x003E);
      for(int x = 6; x >= 0; x -= 2) wr16(0x006C + (uint32_t)x, 0x2959);
    } else {
      for(int x = 6; x >= 0; x -= 2) {
        a = (uint16_t)(a - 8);
        if(a >= 8) {
          wr16(0x006C + (uint32_t)x, 0x295A);
          continue;
        }
        wr16(0x006C + (uint32_t)x, 0x295C);
        hud_bar_cell((uint16_t)(a + 1), 0x1C22);
        for(x -= 2; x >= 0; x -= 2) wr16(0x006C + (uint32_t)x, 0x2959);
        break;
      }
    }
    wr16(0x00A0, bus_rd16(0x838C35 + (uint32_t)((rd16(0x025B) >> 3) & 0x001E)));
    const uint32_t row = (uint16_t)(rd16(DP + 0x4C) << 3);
    for(uint32_t i = 0; i < 8; i += 2) wr16(0x00A4 + i, bus_rd16(0x83A4D4 + row + i));
  }
  if(rd16(0x03DC) & 0x0003) {  // never run: both players' blocks
    func_82B838(rd16(DP + 0x50));
    func_82B838(rd16(DP + 0x52));
    func_82B769(rd16(0x0226), rd16(0x0235));
    wr16(DP + 0x50, hud_block_offset());
    func_82B769(rd16(0x02D9), rd16(0x02E8));
    wr16(DP + 0x52, hud_block_offset());
  } else {
    func_82B838(rd16(DP + 0x50));
    func_82B769(rd16(0x0226), rd16(0x0235));
    wr16(DP + 0x50, hud_block_offset());
  }
  wr16(DP + 0x14, rd16(0x4E3D));
  wr16(DP + 0x16, rd16(0x0233));
  wr16(DP + 0x18, 0x4E61);
  wr16(DP + 0x1A, rd16(0x4E45));
  wr16(DP + 0x1C, rd16(0x4E4D));
  wr16(DP + 0x3A, 0x0C00);
  race_message_update();
  wr16(0x4E3D, rd16(DP + 0x14));
  wr16(0x0233, rd16(DP + 0x16));
  wr16(0x4E45, rd16(DP + 0x1A));
  wr16(0x4E4D, rd16(DP + 0x1C));
}

// TG3000_ROUTINE($83:FB4A) track_object_random
void track_object_random(uint16_t dp) {
  if(!func_83FC05(dp)) return;
  wr16(0x8CB3 + (uint32_t)rd16(0x011A), 0);
  func_83FE9A(dp, 0xFF19, rd16(0x1EBE), 0x0000);
  wr16(dp + 0x4A, 0x801E);
  const uint32_t car = rd16(0x1ECC);
  uint16_t lo = 0, hi = 0, shown = 0;
  switch(bus_rd16(0x83FB76 + (uint32_t)(rng_next_long(0) & 0x001E))) {
    case 0xFBA8: lo = 0x4000; shown = 0x0047; break;
    case 0xFBC0: lo = 0x1000; shown = 0x0048; break;
    case 0xFBB4: hi = 0x0001; shown = 0x004D; break;
    case 0xFB96:
      if((rd16(dp + 0x4E) & 0x00FF) == 10) return;
      wr16(dp + 0x4E, (uint16_t)(rd16(dp + 0x4E) + 1));
      wr16(dp + 0x33, 0x004C);
      return;
    case 0xFBE2:
      wr16(dp + 0x58, 0x0028);
      wr16(dp + 0x56, 0xFFFF);
      wr16(dp + 0x33, 0x004A);
      return;
    case 0xFBF2:
      wr16(dp + 0x5A, 0);
      wr16(dp + 0x33, 0x004B);
      return;
    default:  // $FBFA
      wr16(dp + 0x48, 0x0300);
      wr16(dp + 0x33, 0x004E);
      return;
  }
  wr16(dp + 0x33, shown);
  bool carry = false;
  wr16(0x038E + car, bcd_add4(rd16(0x038E + car), lo, &carry));
  wr16(0x0390 + car, bcd_add4(rd16(0x0390 + car), hi, &carry));
}

// The car's 32-bit position at D+$25.
static uint32_t car_position(uint16_t dp) { return rd16(dp + 0x25) | (uint32_t)rd16(dp + 0x27) << 16; }
static void car_position_set(uint16_t dp, uint32_t v) {
  wr16(dp + 0x25, (uint16_t)v);
  wr16(dp + 0x27, (uint16_t)(v >> 16));
}
// The segment the position is in (its bits 8-23, rounded up by $100).
static uint16_t car_segment(uint16_t dp) { return (uint16_t)((car_position(dp) + 0x100) >> 8); }

// $83:F898 / $83:F8AA: D+$70 rises by 4 * $1E9E to at most $100 (`rise`), then the position
// moves by $1E9E * D+$70 (a DSP-4 multiply).
static void car_step_speed(uint16_t dp, bool rise) {
  if(rise) {
    uint16_t v = (uint16_t)(4 * rd16(0x1E9E) + rd16(dp + 0x70));
    if(v >= 0x0100) v = 0x0100;
    wr16(dp + 0x70, v);
  }
  car_position_set(dp, car_position(dp) + (uint32_t)dsp4_multiply((int16_t)rd16(0x1E9E), (int16_t)rd16(dp + 0x70)));
}

// $83:FD45: D+$B1 = the object entry's bytes 0 and 2 (the tail of car_set_53).
static void car_object_mark(uint16_t dp) {
  const uint32_t x = rd16(0x011A);
  wr16(dp + 0xB1, (uint16_t)(rd8(0x8CB3 + x) | rd8(0x8CB5 + x) << 8));
}

// TG3000_ROUTINE($83:FD68) car_object_stop
// A stop marker: unless bit 5 of D+$44 is set, D+$53 = 4, D+$B1 = the entry's byte 0 and
// D+$5E = its word at +2 (the segment to stop at).
static void car_object_stop(uint16_t dp) {
  if(rd16(dp + 0x44) & 0x0020) return;
  wr8(dp + 0x53, 4);
  const uint32_t x = rd16(0x011A);
  wr16(dp + 0xB1, rd8(0x8CB3 + x));
  wr16(dp + 0x5E, rd16(0x8CB5 + x));
}

// One track object's handler, by its kind's entry in the table at $83:7FFE (kinds 1-104;
// those whose entry is 0 never occur).
static void track_object_run(uint16_t dp, uint16_t kind) {
  switch(bus_rd16(0x837FFE + 2u * kind)) {
    case 0xFA34: track_object_test(dp); break;
    case 0xFC65: track_object_test_sides(dp); break;
    case 0xFA32: clc_83FA32(); break;
    case 0xFB15: track_object_count(dp); break;
    case 0xFAD7: track_object_money(dp); break;
    case 0xFB4A: track_object_random(dp); break;
    case 0xFA6A: track_object_bump(dp); break;
    case 0xFD26: car_set_53(dp, 1); break;
    case 0xFD31: car_set_53(dp, 2); break;
    case 0xFD5D: car_set_53(dp, 3); break;
    case 0xFD3C: car_clear_53(dp); break;
    case 0xFD45: car_object_mark(dp); break;
    case 0xFD68: car_object_stop(dp); break;
    default: break;
  }
}

// TG3000_ROUTINE($83:F7F9) car_track_step
void car_track_step(uint16_t dp) {
  wr16(dp + 0x2D, rd16(dp + 0x25));
  wr16(dp + 0x2F, rd16(dp + 0x27));
  wr16(0x0114, car_segment(dp));
  wr16(0x0120, 0);
  if(rd16(dp + 0x44) & 0x0080) {
    wr16(dp + 0x44, (uint16_t)(rd16(dp + 0x44) & ~0x0080));
    const uint32_t f = 0xFADE + (uint32_t)rd16(0x1EBA);
    wr16(f, (uint16_t)(rd16(f) & 0xFFF7));
  }
  const uint16_t state = rd16(dp + 0x44);
  bool end = false;
  if(!(state & 0x0020)) {
    if(!(state & 0x0001))
      car_position_set(dp, car_position(dp) + (uint32_t)dsp4_multiply((int16_t)rd16(0x1E9E), (int16_t)rd16(dp + 0x10)));
  } else if(!(state & 0x0001)) {
    if(state & 0x0100) {
      if(rd16(dp + 0x5E) != 0) {
        const uint16_t v = rd16(dp + 0x5E), step = rd16(0x1E9E);
        wr16(dp + 0x5E, v >= step ? (uint16_t)(v - step) : 0);
        car_step_speed(dp, true);
      } else {
        const uint16_t v = (uint16_t)(rd16(dp + 0x70) - 4 * rd16(0x1E9E));
        if((uint16_t)(v - rd16(dp + 0x72)) & 0x8000) end = true;
        else {
          wr16(dp + 0x70, v);
          car_step_speed(dp, false);
        }
      }
    } else {
      car_step_speed(dp, true);
      if(rd16(dp + 0x26) >= rd16(dp + 0x5E)) {
        wr16(dp + 0x26, rd16(dp + 0x5E));
        end = true;
      } else {
        wr16(0x0120, (uint16_t)(rd16(0x0120) - 1));
      }
    }
  }
  if(end) {
    wr16(dp + 0x44, (uint16_t)((rd16(dp + 0x44) & ~0x0120) | 0x0080));
    wr16(dp + 0x1A, 0x0004);
    func_83FE9A(dp, 0xFF2A, rd16(0x1EBE), 0x0000);
    wr16(0x0120, (uint16_t)(rd16(0x0120) - 1));
  }
  car_lap_check(dp);
  wr16(0x0116, (uint16_t)(car_segment(dp) - rd16(0x0114)));
  if(rd16(0x0116) != 0) {
    wr16(0x0114, (uint16_t)(rd16(0x0114) << 2));
    wr16(0x0118, (uint16_t)(rd16(0x019A) << 2));
    wr16(0x012C, rd16(0xFABF + (uint32_t)rd16(0x1EBA)));
    wr16(0x011E, (uint16_t)-rd16(0x3DEC + (uint32_t)((rd16(0x0114) >> 1) & 0x003E)));
    do {
      const uint32_t x = (rd16(0x0114) >> 1) & 0x003E;
      const uint16_t curve = (uint16_t)(rd16(0x3DEE + x) - rd16(0x3DEC + x));
      wr16(0x011E, (uint16_t)(rd16(0x011E) - curve - rd16(dp + 0x0C)));
      if(rd16(0x0114) >= rd16(0x0118)) wr16(0x0114, (uint16_t)(rd16(0x0114) - rd16(0x0118)));
      wr16(0x011A, rd16(0x0114));
      const uint16_t entry = rd16(0x8CB3 + (uint32_t)rd16(0x0114));
      if(entry != 0) {
        wr16(0x011C, (uint16_t)((entry >> 8) << 2));
        track_object_run(dp, (uint16_t)(entry >> 8));
      }
      wr16(0x0114, (uint16_t)(rd16(0x0114) + 4));
      wr16(0x0116, (uint16_t)(rd16(0x0116) - 1));
    } while(rd16(0x0116) != 0);
  }
  const uint32_t moved = car_position(dp) - (rd16(dp + 0x2D) | (uint32_t)rd16(dp + 0x2F) << 16);
  wr16(dp + 0x2D, (uint16_t)moved);
  wr16(dp + 0x2F, (uint16_t)(moved >> 16));
}

// TG3000_ROUTINE($82:D8C6) minimap_car
void minimap_car(uint16_t y, uint16_t at, uint16_t attr) {
  if((bus_rd16(0x82DC68 + (uint32_t)rd16(0x047B)) & 0x8000) && rd16(DP + 0x14) != 0)
    at = (rd16(DP + 0x14) & 0x8000) ? 0 : (uint16_t)(((uint16_t)(rd16(0x019A) - 1) >> 2) & 0xFFFC);
  const uint16_t py = (uint16_t)((y + rd8(0x4F32 + (uint32_t)at)) & 0x00FF);
  const uint16_t px = (uint16_t)(rd8(0x4F31 + (uint32_t)at) + 0x00BE);
  uint16_t out[2];
  if(!dsp4_sprite(px, py, attr, out)) return;
  const uint32_t o = rd16(0x0184);
  wr16(0x1576 + o, out[0]);
  wr16(0x1578 + o, out[1]);
  wr16(0x0184, (uint16_t)(o + 4));
}

// The player's place against a segment's start: $0226 + 2 - start - 1 (+1 when $0226 + 2
// wrapped: the carry of the original's ADC #2 feeds its SBC).
static uint16_t segment_gap(uint32_t seg) {
  const uint32_t a = (uint32_t)rd16(0x0226) + 2;
  return (uint16_t)(a - bus_rd16(0x840000 + seg) - 1 + (a >> 16));
}

// TG3000_ROUTINE($82:F2B3) player_segment_step
void player_segment_step(void) {
  wr16(DP + 0x2A, 0);
  if(rd16(0x0226) >= rd16(0x1EDE)) {
    wr16(0x0226, (uint16_t)(rd16(0x1EE0) + rd16(0x0226)));
    wr16(0x023E, (uint16_t)(rd16(0x1EE0) + rd16(0x023E)));
    const uint32_t x = (uint16_t)(rd16(0x1EDE) << 1) & 0x003E;
    const uint16_t d = (uint16_t)(rd16(0x3DEE + x) - rd16(0x3DF0 + x));
    const uint16_t v = (uint16_t)(d + rd16(0x3DEE + x));
    wr16(0x3DEC + x, v);
    wr16(0x3E2C + x, v);
    wr16(0x0206, (uint16_t)(rd16(0x0206) - v));
    wr16(0x0202, rd16(0x0206));
    wr16(0x020C, d);
    wr16(0x1EC2, 0);
    wr16(0x1EC4, rd16(0x3DEC + x));
    wr16(0x1EE0, 0);
    wr16(0x1EDE, 0x7FFF);
  }
  for(;;) {
    uint32_t seg = (uint16_t)(rd16(0x019C) + rd16(0x1E5E));
    if(!((uint16_t)(segment_gap(seg) - 0x0040) & 0x8000)) {
      switch(bus_rd16(0x82F374 + (uint32_t)bus_rd8(0x840002 + seg))) {
        case 0xF381:  // never run
          if(rd16(0x1E66) & 0x8000) {
            const uint32_t at = ((uint32_t)rd16(0x0204) | (uint32_t)rd16(0x0206) << 16) +
                                ((uint32_t)rd16(0x1EC2) | (uint32_t)rd16(0x1EC4) << 16);
            wr16(0x0204, (uint16_t)at);
            wr16(0x0206, (uint16_t)(at >> 16));
            const uint32_t s1 = (uint32_t)bus_rd16(0x840000 + seg) + 0x003F;
            wr16(0x1EDE, (uint16_t)(s1 + bus_rd16(0x840010 + seg) + (s1 >> 16)));
            wr16(0x1EE0, bus_rd16(0x840012 + seg));
            wr16(0x020A, 0);
            wr16(0x020C, 0);
          } else {
            wr16(0x0226, (uint16_t)(rd16(0x1EE6) + rd16(0x0226)));
            wr16(0x023E, (uint16_t)(rd16(0x1EE6) + rd16(0x023E)));
            wr16(0x1EE6, 0);
          }
          break;
        case 0xF3D0:  // never run
          wr16(0x1E66, 0);
          break;
        default:  // $F380: nothing
          break;
      }
      wr16(0x1E5E, (uint16_t)(rd16(0x1E5E) + 0x0014));
      continue;
    }
    seg = (uint16_t)(rd16(0x019C) + rd16(0x1E60));
    if(segment_gap(seg) & 0x8000) return;
    wr16(0x1E60, (uint16_t)(rd16(0x1E60) + 0x0014));
    wr16(DP + 0x2A, (uint16_t)(rd16(DP + 0x2A) | 0x0002));
  }
}

// One player's words for the segment step ($82:F2B3 and $82:FA4F differ only in these). The
// handlers sit at fixed places after their table: +$0C nothing, +$0D kinds 6/10, +$5C kind 8.
typedef struct {
  uint16_t place, place2, jumpAt, jumpBy, cursor, cursor2, rows, rows2;
  uint16_t at0, at2, at4, at6, atA, atC, shiftLo, shiftHi, flags, pending;
  uint32_t handlers;
} SegmentPlayer;

// The player's place against a segment's start (see segment_gap).
static uint16_t segment_gap_of(const SegmentPlayer* p, uint32_t seg) {
  const uint32_t a = (uint32_t)rd16(p->place) + 2;
  return (uint16_t)(a - bus_rd16(0x840000 + seg) - 1 + (a >> 16));
}

static void segment_step(const SegmentPlayer* p) {
  wr16(DP + 0x2A, 0);
  if(rd16(p->place) >= rd16(p->jumpAt)) {
    wr16(p->place, (uint16_t)(rd16(p->jumpBy) + rd16(p->place)));
    wr16(p->place2, (uint16_t)(rd16(p->jumpBy) + rd16(p->place2)));
    const uint32_t x = (uint16_t)(rd16(p->jumpAt) << 1) & 0x003E;
    const uint16_t d = (uint16_t)(rd16(p->rows + 2 + x) - rd16(p->rows + 4 + x));
    const uint16_t v = (uint16_t)(d + rd16(p->rows + 2 + x));
    wr16(p->rows + x, v);
    wr16(p->rows2 + x, v);
    wr16(p->at6, (uint16_t)(rd16(p->at6) - v));
    wr16(p->at2, rd16(p->at6));
    wr16(p->atC, d);
    wr16(p->shiftLo, 0);
    wr16(p->shiftHi, rd16(p->rows + x));
    wr16(p->jumpBy, 0);
    wr16(p->jumpAt, 0x7FFF);
  }
  for(;;) {
    uint32_t seg = (uint16_t)(rd16(0x019C) + rd16(p->cursor));
    if(!((uint16_t)(segment_gap_of(p, seg) - 0x0040) & 0x8000)) {
      const uint16_t handler = bus_rd16(p->handlers + (uint32_t)bus_rd8(0x840002 + seg));
      if(handler == (uint16_t)(p->handlers + 0x0D)) {  // kinds 6 and 10
        if(rd16(p->flags) & 0x8000) {
          const uint32_t at = ((uint32_t)rd16(p->at4) | (uint32_t)rd16(p->at6) << 16) +
                              ((uint32_t)rd16(p->shiftLo) | (uint32_t)rd16(p->shiftHi) << 16);
          wr16(p->at4, (uint16_t)at);
          wr16(p->at6, (uint16_t)(at >> 16));
          const uint32_t s1 = (uint32_t)bus_rd16(0x840000 + seg) + 0x003F;
          wr16(p->jumpAt, (uint16_t)(s1 + bus_rd16(0x840010 + seg) + (s1 >> 16)));
          wr16(p->jumpBy, bus_rd16(0x840012 + seg));
          wr16(p->atA, 0);
          wr16(p->atC, 0);
        } else {
          wr16(p->place, (uint16_t)(rd16(p->pending) + rd16(p->place)));
          wr16(p->place2, (uint16_t)(rd16(p->pending) + rd16(p->place2)));
          wr16(p->pending, 0);
        }
      } else if(handler == (uint16_t)(p->handlers + 0x5C)) {  // kind 8 (kinds 0-4: nothing)
        wr16(p->flags, 0);
      }
      wr16(p->cursor, (uint16_t)(rd16(p->cursor) + 0x0014));
      continue;
    }
    seg = (uint16_t)(rd16(0x019C) + rd16(p->cursor2));
    if(segment_gap_of(p, seg) & 0x8000) return;
    wr16(p->cursor2, (uint16_t)(rd16(p->cursor2) + 0x0014));
    wr16(DP + 0x2A, (uint16_t)(rd16(DP + 0x2A) | 0x0002));
  }
}

// TG3000_ROUTINE($82:FA4F) player2_segment_step
void player2_segment_step(void) {
  static const SegmentPlayer second = {
    .place = 0x02D9, .place2 = 0x02F1, .jumpAt = 0x1EE2, .jumpBy = 0x1EE4, .cursor = 0x1E62,
    .cursor2 = 0x1E64, .rows = 0x436C, .rows2 = 0x43AC, .at0 = 0x02B3, .at2 = 0x02B5,
    .at4 = 0x02B7, .at6 = 0x02B9, .atA = 0x02BD, .atC = 0x02BF, .shiftLo = 0x1EC6,
    .shiftHi = 0x1EC8, .flags = 0x1E68, .pending = 0x1EE8, .handlers = 0x82FB10,
  };
  segment_step(&second);
}

// TG3000_ROUTINE($82:EEB7) road_shift_by_place
void road_shift_by_place(void) {
  const uint16_t flags = rd16(0x1E66);
  if(rd16(0x0226) & 0x0020) {
    wr16(0x1E66, (uint16_t)(((flags & 0x7FFF) >> 1) | ((flags & 0x0001) ? 0x8000 : 0)));
    road_shift_apply();
    wr16(0x1E66, flags);
  } else {
    road_shift_settle();
    wr16(0x1E66, (uint16_t)((flags & 0xFFFE) | (rd16(0x1E66) >> 15)));
  }
}

// The high word of a DSP-4 product.
static uint16_t dsp4_high(uint16_t a, uint16_t b) {
  return (uint16_t)((uint32_t)dsp4_multiply((int16_t)a, (int16_t)b) >> 16);
}

// A road row's place at t: the row's word plus the high word of (its slope * 2) * t.
static uint16_t road_row_at(uint32_t row, uint32_t x, uint16_t t) {
  return (uint16_t)(dsp4_high((uint16_t)((uint16_t)(rd16(row + 2 + x) - rd16(row + x)) << 1), t) + rd16(row + x));
}

// TG3000_ROUTINE($82:ED27) road_edges_side
// The road's edges against the car (never run): D+$1C and D+$1E = $1EF6 less the rows
// $3C6C and $3F6C at t; unless -$0202 lies below the first and at or above the second,
// $0242, $0208 and D+$3C are cleared. Else $0242 = -(row $3E6C at t), D+$18 = its slope * 2,
// D+$1A:$1C = $1EF8 * 4 * ($8000 - t) / 2 plus that slope * t (DSP-4 multiplies), $0208 =
// -(its bits 8-23) and D+$3C = 1.
static void road_edges_side(void) {
  const uint32_t x = rd16(DP + 0x16);
  const uint16_t t = rd16(DP + 0x14);
  wr16(DP + 0x1C, (uint16_t)(rd16(0x1EF6) - road_row_at(0x3C6C, x, t)));
  wr16(DP + 0x1E, (uint16_t)(rd16(0x1EF6) - road_row_at(0x3F6C, x, t)));
  const uint16_t a = (uint16_t)-rd16(0x0202);
  if(!((uint16_t)(a - rd16(DP + 0x1C)) & 0x8000) || ((uint16_t)(a - rd16(DP + 0x1E)) & 0x8000)) {
    wr16(0x0242, 0);
    wr16(0x0208, 0);
    wr16(DP + 0x3C, 0);
    return;
  }
  const uint16_t slope = (uint16_t)((uint16_t)(rd16(0x3E6E + x) - rd16(0x3E6C + x)) << 1);
  wr16(DP + 0x18, slope);
  wr16(0x0242, (uint16_t)-(uint16_t)(dsp4_high(slope, t) + rd16(0x3E6C + x)));
  const uint32_t p = (uint32_t)dsp4_multiply((int16_t)(uint16_t)(rd16(0x1EF8) << 2), (int16_t)(uint16_t)((uint16_t)(0x8000 - t) >> 1));
  wr16(DP + 0x1A, (uint16_t)p);
  wr16(DP + 0x1C, (uint16_t)(p >> 16));
  const uint32_t q = (uint32_t)dsp4_multiply((int16_t)slope, (int16_t)t);
  const uint32_t lo = (uint32_t)rd16(DP + 0x1A) + (uint16_t)q;
  wr16(DP + 0x1A, (uint16_t)lo);
  wr16(DP + 0x1C, (uint16_t)(rd16(DP + 0x1C) + (uint16_t)(q >> 16) + (lo >> 16)));
  wr16(0x0208, (uint16_t)-rd16(DP + 0x1B));
  wr16(DP + 0x3C, 1);
}

// TG3000_ROUTINE($82:EE2A) road_edge_shift
// The edge shift (never run): with D+$3C clear, race_scroll_commit's moves without its
// $1EEE/$1EF0 clears; else $020A = 0 and $020C = -($1EFA * 4 * ($8000 - t) / 2 plus the
// row $3DEC's slope * 2 * t, the high word), $1EEE = $1EF0 = 0.
static void road_edge_shift(void) {
  const uint32_t x = rd16(DP + 0x16);
  const uint16_t t = rd16(DP + 0x14);
  if(rd16(DP + 0x3C) == 0) {
    wr16(0x020A, 0);
    wr16(0x020C, 0);
    wr16(0x0200, rd16(0x0204));
    const uint32_t lo = (uint32_t)rd16(0x0204) + rd16(0x1EF4);
    wr16(0x1EF4, (uint16_t)lo);
    wr16(0x0202, rd16(0x0206));
    wr16(0x1EF6, (uint16_t)(rd16(0x0206) + rd16(0x1EF6) + (lo >> 16)));
    return;
  }
  const uint32_t p = (uint32_t)dsp4_multiply((int16_t)(uint16_t)(rd16(0x1EFA) << 2), (int16_t)(uint16_t)((uint16_t)(0x8000 - t) >> 1));
  wr16(DP + 0x1A, (uint16_t)p);
  wr16(0x020C, (uint16_t)(p >> 16));
  wr16(0x020A, 0);
  const uint32_t q = (uint32_t)dsp4_multiply((int16_t)(uint16_t)((uint16_t)(rd16(0x3DEE + x) - rd16(0x3DEC + x)) << 1), (int16_t)t);
  const uint16_t carry = (uint16_t)(((uint32_t)rd16(DP + 0x1A) + (uint16_t)q) >> 16);
  wr16(0x020C, (uint16_t)-(uint16_t)((uint16_t)(q >> 16) + carry + rd16(0x020C)));
  wr16(0x1EEE, 0);
  wr16(0x1EF0, 0);
}

// TG3000_ROUTINE($82:EC36) road_shift_frame
void road_shift_frame(void) {
  wr16(DP + 0x3C, 0);
  const uint32_t x = (uint16_t)(rd16(0x0226) << 1) & 0x003E;
  wr16(DP + 0x16, (uint16_t)x);
  const uint16_t t = (uint16_t)((rd16(0x0224) >> 1) & 0xFF80);
  const uint32_t p = (uint32_t)dsp4_multiply((int16_t)(uint16_t)((uint16_t)(rd16(0x3DEE + x) - rd16(0x3DEC + x)) << 1), (int16_t)t);
  wr16(DP + 0x14, t);
  wr16(0x1EF4, (uint16_t)p);
  uint16_t at = (uint16_t)((uint16_t)(p >> 16) + rd16(0x3DEC + x));
  if(at != 0 && !(at & 0x8000)) at--;
  wr16(0x1EF6, at);
  switch(bus_rd16(0x82ED23 + (uint32_t)rd8(0x0261))) {
    case 0xECEA: race_clear_0242_0208(); break;
    case 0xED27: road_edges_side(); break;
    default: break;
  }
  const uint16_t lo = rd16(0x1EF4), hi = rd16(0x1EF6);
  switch(bus_rd16(0x82ED17 + (uint32_t)rd8(0x0260))) {
    case 0xECF1: race_scroll_commit(); break;
    case 0xEE2A: road_edge_shift(); break;
    case 0xEEB7: road_shift_by_place(); break;
    case 0xEFCC:  // never run: by the row
      if((rd16(0x0226) & 0x003E) < 0x0018) road_shift_settle();
      else road_shift_apply();
      break;
    case 0xF1C1: road_shift_apply(); break;
    default: break;
  }
  wr16(0x1EC4, hi);
  wr16(0x1EC2, lo);
  const uint16_t v = rd16(0x01CE);
  wr16(0x0229, v);
  const uint16_t n = (v & 0x8000) ? (uint16_t)-v : v;
  const uint8_t d = rd8(0x1E9E);
  const uint16_t q = d ? (uint16_t)(n / d) : 0xFFFF;  // the CPU divider
  wr16(0x01CE, (uint16_t)dsp4_multiply((v & 0x8000) ? 1 : -1, (int16_t)q));
}

// TG3000_ROUTINE($82:EC32) road_shift_frame_call
void road_shift_frame_call(void) { road_shift_frame(); }

// TG3000_ROUTINE($82:EA80) track_event_none
void track_event_none(void) {}

// TG3000_ROUTINE($82:A9AF) player_segment_step_long
void player_segment_step_long(void) { player_segment_step(); }

// TG3000_ROUTINE($82:A9B3) player2_segment_step_long
void player2_segment_step_long(void) { player2_segment_step(); }

// A bank-$85 table entry for track $047B, or (negative) the word the track header names.
static uint16_t track_setup_word(uint32_t table, uint32_t x) {
  const uint16_t v = bus_rd16(table + x);
  if(!(v & 0x8000)) return v;
  return bus_rd16(0x908000 + (uint32_t)(uint16_t)(rd16(0x5395) - 0x8007));
}

// TG3000_ROUTINE($82:A7F9) track_setup_load
void track_setup_load(void) {
  uint16_t end;
  const uint32_t x2 = (uint16_t)(rd16(0x047B) << 1);
  decompress(DP, 0x9B, bus_rd16(0x84FD33 + x2), 0x7E, 0x5391, &end);
  const uint16_t banks = bus_rd16(0x84FE0F + x2);
  decompress(DP, (uint8_t)banks, bus_rd16(0x84FDA1 + x2), 0x7E, 0x8CB3, &end);
  const uint32_t x4 = (uint16_t)(rd16(0x047B) << 2);
  const uint16_t src = bus_rd16(0x84FE7D + x4), count = bus_rd16(0x84FE7F + x4);
  for(uint32_t i = 0; i <= count; i++)  // MVN $7E,$8B: count + 1 bytes
    wr8((uint16_t)(0x5B91 + i), bus_rd8(0x8B0000 + (uint32_t)(uint16_t)(src + i)));
  const bool variant2 = (rd16(0x03DC) & 0x0003) == 0x0002;
  if(variant2) decompress(DP, 0x89, 0x8512, 0x7F, 0x0000, &end);  // never run
  else if(rd16(0x01E4) & 0x8000) decompress(DP, 0x96, 0x8020, 0x7F, 0x0000, &end);
  else if(rd16(0x01E4) == 0) decompress(DP, 0x96, 0x8097, 0x7F, 0x0000, &end);
  else decompress(DP, 0x97, 0xA24D, 0x7F, 0x0000, &end);
  wr16(0x003A, track_setup_word((rd8(0x039C) ? 0x85A3CC : 0x85A35E), x2));
  wr16(0x0038, track_setup_word((rd8(0x03CB) ? 0x85A3CC : 0x85A35E), x2));  // $85:A3CC: never run
  wr16(DP + 0x14, variant2 ? 0x0612 : 0x00D2);
  for(uint32_t y = 0x0200; y > 0; y -= 2) wr16(0x658F + y, 0xFFFF);
  for(uint16_t y = (uint16_t)(rd16(0x019A) << 2); y != 0; y = (uint16_t)(y - 4)) {
    const uint8_t kind = rd8(0x8CB0 + (uint32_t)y);
    if(kind == 0) continue;
    const uint16_t x = bus_rd16(0x86AE36 + 2u * kind);
    if(x == 0) continue;
    const uint32_t slot = 0x6591 + (uint32_t)bus_rd16(0x860000 + (uint32_t)x);
    if(rd16(slot) != 0xFFFF) continue;
    const uint16_t at = rd16(DP + 0x14);
    wr16(slot, at);
    const uint16_t len = decompress(DP, (uint8_t)bus_rd16(0x860002 + (uint32_t)x), bus_rd16(0x860004 + (uint32_t)x), 0x7F, at, &end);
    wr16(DP + 0x14, (uint16_t)(len + at));
  }
}

// The bytes at $8A:`at` pushed to the byte buffer up to `stop`.
static void byte_buffer_string(uint16_t at, uint8_t stop) {
  for(uint32_t i = 0; bus_rd8(0x8A0000 + (uint32_t)(uint16_t)(at + i)) != stop; i++)
    byte_buffer_push(bus_rd8(0x8A0000 + (uint32_t)(uint16_t)(at + i)));
}

// TG3000_ROUTINE($8A:99FC) text_line_centered
void text_line_centered(uint16_t x) {
  const uint32_t line = 0x7F7918 - 0x7E0000 + (uint32_t)(uint16_t)((x >> 1) << 4);
  uint16_t pad = 0x001C;
  for(uint32_t i = 0; rd8(line + (uint16_t)i) != 0; i++) pad = (uint16_t)(pad - 1);
  uint16_t n = (uint16_t)(pad >> 1);
  do {
    byte_buffer_space();
    n = (uint16_t)(n - 1);
  } while(n != 0);
  for(uint32_t i = 0; rd8(line + (uint16_t)i) != 0; i++) byte_buffer_push(rd8(line + (uint16_t)i));
}

// TG3000_ROUTINE($8A:9908) results_text
void results_text(void) {
  results_text_build();
  func_8A9BE2();
  wr16(DP + 0x5E, 0);
  variant_66();
  uint16_t slot;
  while(func_8A9BB5(&slot)) {
    wr16(DP + 0x5E, (uint16_t)(rd16(DP + 0x5E) + 1));
    byte_buffer_decimal2();
    byte_buffer_space();
    text_line_push(slot);
    byte_buffer_pad(0x0016);
    byte_buffer_space();
    byte_buffer_space();
    byte_buffer_space();
    byte_buffer_decimal(rd16(0x041C + (uint32_t)slot));
    byte_buffer_end();
  }
  if(rd16(0x1B37) & 0x8000) {  // never run
    byte_buffer_end();
    byte_buffer_end();
    byte_buffer_string(0x9DBF, 0x00);
    func_8A9BE2();
    wr16(DP + 0x5E, 0);
    wr16(DP + 0x66, 0x0008);
    for(uint32_t i = 0; i < 8; i += 2) wr16(0x17FBC + i, 0);
    func_8A9BB5(&slot);
    byte_buffer_end();
    byte_buffer_end();
    text_line_centered(slot);
    byte_buffer_end();
    byte_buffer_end();
    byte_buffer_string(0x9DD5, 0x00);
  }
  byte_buffer_string(0x9DBD, 0xFF);
}

// The $78 bytes after `at` summed into a word (kept in $0114, as the original's running
// sum), ^ $7645 (the saved data's checksum).
static uint16_t save_data_sum(uint32_t at) {
  uint16_t sum = 0;
  for(uint32_t x = 0x78; x != 0; x--) sum = (uint16_t)(sum + rd8(at + x));
  wr16(0x0114, sum);
  return (uint16_t)(sum ^ 0x7645);
}

// TG3000_ROUTINE($80:8119) save_data_check
bool save_data_check(void) {
  wr16(0x03E2, 0);
  if(save_data_sum(0xFA3C) == rd16(0xFA3B)) {  // never run: the stored copy restored
    for(uint32_t i = 0; i < 0x7A; i++) wr8(0x0368 + i, rd8(0xFA3D + i));
    wr16(0x045A, (uint16_t)(rd16(0x045A) & ~0x00FF));
    return false;
  }
  if(save_data_sum(0x0367) == rd16(0x0366)) {  // never run
    wr16(0x045A, (uint16_t)(rd16(0x045A) & ~0x00FF));
    return false;
  }
  for(uint32_t i = 0; i < 0x7A; i++) wr8(0x0366 + i, bus_rd8(0x8385A9 + i));
  save_data_seal();
  wr16(0x045A, (uint16_t)(rd16(0x045A) | 0x00FF));
  return true;
}

// TG3000_ROUTINE($82:AE15) oam_high_load
void oam_high_load(void) {
  dsp4_port_write16(0x0006);
  const uint32_t y = rd16(DP + 0x8E);
  for(uint32_t i = 0; i < 32; i += 2) wr16(0x1776 + y + i, dsp4_port_read16());
  const uint16_t room = (uint16_t)(0x0200 - (uint16_t)(rd16(DP + 0x86) - rd16(DP + 0x8E)));
  if(room == 0) return;
  uint16_t n = (uint16_t)(room >> 2), at = rd16(DP + 0x86);
  do {
    wr16(0x1576 + (uint32_t)at, 0xE0E0);
    at = (uint16_t)(at + 4);
    n = (uint16_t)(n - 1);
  } while(n != 0);
}

// $82:DDBF's overflow: the chip's remaining answers drained (each nonzero flag with its
// pair), then `open` 0 words ending the lists still open.
static void sprite_parts_drain(int open) {
  while(dsp4_port_read16() != 0) {
    (void)dsp4_port_read16();
    (void)dsp4_port_read16();
  }
  for(int i = 0; i < open; i++) dsp4_port_write16(0);
}

// One list of parts from $7F:0000 + *x (the 0 word ending it left at *x). Returns false
// when the buffer filled (its rest drained, `open` lists ended).
static bool sprite_parts_list(uint16_t* x, uint16_t* y, int open) {
  uint16_t w = rd16(0x10000 + (uint32_t)*x);
  dsp4_port_write16(w);
  while(w != 0) {
    dsp4_port_write16(rd16(0x10002 + (uint32_t)*x));
    dsp4_port_write16(rd16(0x10004 + (uint32_t)*x));
    if(dsp4_port_read16() != 0) {
      for(;;) {
        *y = (uint16_t)(*y + 2);
        wr16(0x1574 + (uint32_t)*y, dsp4_port_read16());
        *y = (uint16_t)(*y + 2);
        wr16(0x1574 + (uint32_t)*y, dsp4_port_read16());
        if(*y == rd16(DP + 0x1A)) {
          sprite_parts_drain(open);
          return false;
        }
        if(dsp4_port_read16() == 0) break;
      }
    }
    *x = (uint16_t)(*x + 6);
    w = rd16(0x10000 + (uint32_t)*x);
    dsp4_port_write16(w);
  }
  return true;
}

// TG3000_ROUTINE($82:DDBF) sprite_parts_stream
static bool sprite_parts_stream(uint16_t x) {
  uint16_t y = rd16(DP + 0x84);
  bool full = !sprite_parts_list(&x, &y, 2);  // the first list's overflow: never run
  if(!full) {
    x = (uint16_t)(x + 2);
    full = !sprite_parts_list(&x, &y, 1);
  }
  wr16(DP + 0x84, y);
  return full;
}

// The object's graphics place: its slot's place plus its part set's offset.
static uint16_t sprite_parts_at(void) {
  const uint16_t d24 = rd16(DP + 0x24);
  const uint16_t place = rd16(0x6591 + (uint32_t)bus_rd16(0x860000 + (uint32_t)d24));
  const uint16_t set = (uint16_t)(bus_rd16(0x848000 + (uint32_t)rd16(0x39EC + (uint32_t)rd16(DP + 0x14))) + d24);
  return (uint16_t)(place + bus_rd16(0x860002 + (uint32_t)set));
}

static bool sprite_put_rows(uint32_t third, uint32_t fourth, uint16_t flags) {
  const uint32_t x = rd16(DP + 0x14);
  dsp4_port_write16(rd16(0x386C + x));
  dsp4_port_write16(rd16(0x38EC + x));
  dsp4_port_write16(rd16(third + x));
  dsp4_port_write16(rd16(fourth + x));
  dsp4_port_write16(rd16(DP + 0x20));
  dsp4_port_write16(rd16(DP + 0x22));
  dsp4_port_write16((uint16_t)(rd16(DP + 0x26) | flags));
  return sprite_parts_stream(sprite_parts_at());
}

// TG3000_ROUTINE($82:D934) sprite_put_rows_a
bool sprite_put_rows_a(void) { return sprite_put_rows(0x35EC, 0x37EC, rd16(DP + 0x3A)); }

// TG3000_ROUTINE($82:D988) sprite_put_rows_b
bool sprite_put_rows_b(void) { return sprite_put_rows(0x366C, 0x386C, 0x1000); }

// One list of parts from `base` + *x (bus addresses: $7F:0000 or $86:0000), as
// sprite_parts_list.
static bool sprite_parts_list_at(uint32_t base, uint16_t* x, uint16_t* y, int open) {
  uint16_t w = bus_rd16(base + *x);
  dsp4_port_write16(w);
  while(w != 0) {
    dsp4_port_write16(bus_rd16(base + (uint16_t)(*x + 2)));
    dsp4_port_write16(bus_rd16(base + (uint16_t)(*x + 4)));
    if(dsp4_port_read16() != 0) {
      for(;;) {
        *y = (uint16_t)(*y + 2);
        wr16(0x1574 + (uint32_t)*y, dsp4_port_read16());
        *y = (uint16_t)(*y + 2);
        wr16(0x1574 + (uint32_t)*y, dsp4_port_read16());
        if(*y == rd16(DP + 0x1A)) {
          sprite_parts_drain(open);
          return false;
        }
        if(dsp4_port_read16() == 0) break;
      }
    }
    *x = (uint16_t)(*x + 6);
    w = bus_rd16(base + *x);
    dsp4_port_write16(w);
  }
  return true;
}

// TG3000_ROUTINE($82:DEA8) sprite_parts_stream_rom
bool sprite_parts_stream_rom(uint16_t x) {
  uint16_t y = rd16(DP + 0x84);
  bool full = !sprite_parts_list_at(0x860000, &x, &y, 2);  // the first list's overflow: never run
  if(!full) {
    x = (uint16_t)(x + 2);
    full = !sprite_parts_list_at(0x860000, &x, &y, 1);
  }
  wr16(DP + 0x84, y);
  return full;
}

// TG3000_ROUTINE($82:D267) car_sprite_head
uint16_t car_sprite_head(uint16_t x, uint16_t y) {
  dsp4_port_write16(rd16(0x386A + (uint32_t)x));
  dsp4_port_write16(0x9000);
  dsp4_port_write16(rd16(0xFAB4 + (uint32_t)y));
  uint16_t h = rd8(0x0243);
  if(h & 0x0080) h |= 0xFF00;  // never run
  const uint16_t height = (uint16_t)(h + 0x0033 + rd16(DP + 0x34));
  dsp4_port_write16(height);
  dsp4_port_write16(height);
  dsp4_port_write16(0);
  dsp4_port_write16(rd16(DP + 0x32));
  const uint16_t frame = (uint16_t)(((rd16(0xFAB5 + (uint32_t)y) >> 1) & 0x007E) + rd16(0x39EC + (uint32_t)rd16(DP + 0x14)));
  dsp4_port_write16(bus_rd16(0x83ACEC + (uint32_t)frame));
  dsp4_port_write16(0);
  const uint16_t answer = dsp4_port_read16();
  wr16(DP + 0x2A, answer);
  return answer;
}

// The object sprite's common setup: D+$24, D+$26's bits, D+$20 (see object_sprite_raised).
static uint32_t object_sprite_setup(uint16_t x) {
  wr16(DP + 0x24, bus_rd16(0x86AF06 + (uint32_t)x));
  const uint32_t o = rd16(DP + 0x1C);
  wr16(DP + 0x26, (uint16_t)(((rd16(0x8CB4 + o) & 0x0700) << 1) | rd16(DP + 0x26)));
  uint16_t at = (uint16_t)(rd8(0x8CB3 + o) << 4);
  if(at & 0x0800) at |= 0xF000;
  wr16(DP + 0x20, at);
  return o;
}

// TG3000_ROUTINE($82:DAFD) object_sprite_raised
bool object_sprite_raised(uint16_t x) {
  wr16(DP + 0x24, bus_rd16(0x86AF06 + (uint32_t)x));
  const uint32_t o = rd16(DP + 0x1C);
  uint16_t at = (uint16_t)(rd8(0x8CB3 + o) << 4);
  if(at & 0x0800) at |= 0xF000;
  wr16(DP + 0x20, at);
  wr16(DP + 0x26, (uint16_t)(((rd16(0x8CB4 + o) & 0x0700) << 1) | rd16(DP + 0x26)));
  const uint16_t lift = (uint16_t)((rd16(0x8CB5 + o) & 0x00F0) >> 1);
  const uint32_t row = (uint16_t)(rd16(0x0024) - rd16(0x8CB6 + o)) & 0x003E;
  wr16(DP + 0x22, (uint16_t)(bus_rd16(0x84A200 + row) - lift - 0x0020));
  return sprite_put_rows_a();
}

// TG3000_ROUTINE($82:DB4D) object_sprite_flat
bool object_sprite_flat(uint16_t x) {
  const uint32_t o = object_sprite_setup(x);
  wr16(DP + 0x22, 0);
  return (rd16(0x8CB5 + o) & 0x0010) ? sprite_put_rows_b() : sprite_put_rows_a();
}

// D+$20 negated, or less $80.
static void sprite_place_negate(void) { wr16(DP + 0x20, (uint16_t)-rd16(DP + 0x20)); }
static void sprite_place_back(void) { wr16(DP + 0x20, (uint16_t)(rd16(DP + 0x20) - 0x0080)); }

// TG3000_ROUTINE($82:DB87) object_sprite_group
bool object_sprite_group(uint16_t x) {
  wr16(DP + 0x24, bus_rd16(0x86AF06 + (uint32_t)x));
  const uint32_t o = rd16(DP + 0x1C);
  wr16(DP + 0x26, (uint16_t)(((rd16(0x8CB4 + o) & 0x0700) << 1) | rd16(DP + 0x26)));
  const uint16_t handler = bus_rd16(0x82DBAA + (uint32_t)((rd16(0x8CB5 + o) & 0x0030) >> 3));
  uint16_t at = (uint16_t)(rd8(0x8CB3 + o) << 4);
  if(at & 0x0800) at |= 0xF000;  // kind 2's: never run
  wr16(DP + 0x20, at);
  wr16(DP + 0x22, 0);
  if(handler == 0xDBF5) {  // kind 1: _a and _b at p, then at -p
    if(sprite_put_rows_a() || sprite_put_rows_b()) return true;
    sprite_place_negate();
    if(sprite_put_rows_a()) return true;
    return sprite_put_rows_b();
  }
  bool (*put)(void) = handler == 0xDC27 ? sprite_put_rows_b : sprite_put_rows_a;
  if(put()) return true;
  sprite_place_negate();
  if(put()) return true;
  sprite_place_back();
  if(put()) return true;
  sprite_place_negate();
  return put();
}

// TG3000_ROUTINE($82:DA48) object_sprite_shadowed
bool object_sprite_shadowed(uint16_t x) {
  wr16(DP + 0x24, bus_rd16(0x86AF06 + (uint32_t)x));
  const uint32_t o = rd16(DP + 0x1C);
  uint16_t at = (uint16_t)(rd8(0x8CB3 + o) << 4);
  if(at & 0x0800) at |= 0xF000;
  wr16(DP + 0x20, at);
  wr16(DP + 0x26, (uint16_t)(((rd16(0x8CB4 + o) & 0x0700) << 1) | rd16(DP + 0x26)));
  const uint32_t r = (uint16_t)(rd16(0x0024) - rd16(0x8CB6 + o)) & 0x003E;
  wr16(DP + 0x22, (uint16_t)(bus_rd16(0x84A200 + r) - 0x0040));
  const bool second_b = (rd16(0x8CB5 + r) & 0x0010) != 0;  // indexed by r, as the original
  if(sprite_put_rows_a()) return true;
  wr16(DP + 0x22, 0);
  wr16(DP + 0x24, 0xBC30);
  return second_b ? sprite_put_rows_b() : sprite_put_rows_a();
}

// TG3000_ROUTINE($81:8527) sprite_part_put
bool sprite_part_put(uint16_t x, uint16_t* y) {
  dsp4_port_write16(x);
  dsp4_port_write16((uint16_t)(rd16(DP + 0x32) - 8));
  dsp4_port_write16(rd16(DP + 0x16));
  while(dsp4_port_read16() != 0) {
    *y = (uint16_t)(*y + 2);
    wr16(0x1574 + (uint32_t)*y, dsp4_port_read16());
    *y = (uint16_t)(*y + 2);
    wr16(0x1574 + (uint32_t)*y, dsp4_port_read16());
    if(*y == rd16(DP + 0x1A)) return true;  // never run
  }
  return false;
}

// TG3000_ROUTINE($81:8487) sprite_bar_put
bool sprite_bar_put(uint16_t length, uint16_t at, uint16_t d32) {
  if(length == 0) {
    dsp4_port_write16(0);
    dsp4_port_write16(0);
    return false;
  }
  wr16(DP + 0x32, d32);
  const uint16_t saved14 = rd16(DP + 0x14), saved16 = rd16(DP + 0x16);
  wr16(DP + 0x14, (uint16_t)(length | 1));
  wr16(DP + 0x16, (uint16_t)(at - (rd16(DP + 0x14) >> 1)));
  uint16_t y = rd16(DP + 0x84);
  dsp4_port_write16(0);
  uint16_t part = 0x20F6;
  for(;;) {
    wr16(DP + 0x14, (uint16_t)(rd16(DP + 0x14) - 8));
    if(rd16(DP + 0x14) & 0x8000) break;
    if(sprite_part_put(part, &y)) goto full;
    wr16(DP + 0x16, (uint16_t)(rd16(DP + 0x16) + 8));
    part = 0x20F7;
  }
  wr16(DP + 0x16, (uint16_t)(rd16(DP + 0x14) + rd16(DP + 0x16)));
  if(sprite_part_put(0x60F6, &y)) goto full;
  wr16(DP + 0x16, saved16);
  wr16(DP + 0x14, saved14);
  dsp4_port_write16(0);
  wr16(DP + 0x84, y);
  return false;
full:  // never run
  while(dsp4_port_read16() != 0) {
    (void)dsp4_port_read16();
    (void)dsp4_port_read16();
  }
  dsp4_port_write16(0);
  wr16(DP + 0x84, y);
  return true;
}

// TG3000_ROUTINE($85:9ADB) race_grid_setup
void race_grid_setup(void) {
  wr16(DP + 0x18, 0x00B9);
  wr16(DP + 0x1A, 0);
  wr16(DP + 0x1C, 0);
  wr16(DP + 0x16, 0x0040);
  wr16(DP + 0x14, (uint16_t)(rd16(0x019A) - 0x0014));
  const uint16_t variant = rd16(0x03DD) & 0x0003;
  uint16_t ahead = bus_rd8(0x859FA7 + variant);
  wr16(DP + 0x1E, ahead);
  const uint32_t t = (uint16_t)(rd16(0x047B) << 1);
  const uint32_t table = variant == 0 ? 0x859EEB : variant == 1 ? 0x859E8D : 0x859E2F;  // variant 0: never run
  wr16(DP + 0x1E, (uint16_t)(bus_rd16(table + t) + ahead));
  wr16(DP + 0x20, bus_rd16(0x859F49 + t));
  wr16(DP + 0x22, (uint16_t)((rd16(0x0391) & 0xFF00) << 1));
  wr16(DP + 0x24, (uint16_t)((rd16(0x03C0) & 0xFF00) << 1));
  for(uint32_t x = 0, y = 0; y != 0x28; x += 0x35, y += 2) {
    wr16(0xFADD + x, rd16(DP + 0x1C));
    wr16(DP + 0x1C, (uint16_t)(rd16(DP + 0x1C) + 1));
    static const uint16_t cleared[] = {0xFAB5, 0xFAB7, 0xFABD, 0xFABF, 0xFAB9, 0xFABB, 0xFAC5, 0xFADE};
    for(unsigned i = 0; i < sizeof cleared / sizeof cleared[0]; i++) wr16(cleared[i] + x, 0);
    wr16(0xFAD3 + x, 0xFFFF);
    wr16(0xFAE0 + x, rd16(0x019C));
    wr16(0xFAE2 + x, 0x7000);
    wr16(0xFACB + x, 0);
    wr16(DP + 0x18, 0);  // (0 - 5, negative, kept at 0)
    uint16_t lane = rd16(DP + 0x1A);
    wr16(0xFAD1 + x, lane);
    do {
      lane = (uint16_t)(lane + 0x0200);
      if(lane == 0x0E00) lane = 0;
    } while(lane == rd16(DP + 0x22) || lane == rd16(DP + 0x24));
    wr16(DP + 0x1A, lane);
    const uint16_t r = (uint16_t)(rng_next_long(0) & 0x001F);
    wr16(0xFAD7 + x, (uint16_t)(bus_rd16(0x859DDF + y) - r - rd16(DP + 0x1E)));
    wr16(0xFAD9 + x, (uint16_t)(bus_rd16(0x859E07 + y) - rd16(DP + 0x20)));
    uint16_t drift;
    for(;;) {
      drift = rng_next_long(0);
      if((uint16_t)(drift - 0xFFF0) & 0x8000) break;
      if(!((uint16_t)(drift - 0x0010) & 0x8000)) break;
    }
    drift &= 0x81FF;
    if(drift & 0x8000) drift |= 0xFE00;
    wr16(0xFADB + x, drift);
  }
  wr16(0xFAD1, (uint16_t)((rd16(0x0391) & 0xFF00) << 1));
  wr16(0xFB06, (uint16_t)((rd16(0x03C0) & 0xFF00) << 1));
  wr16(DP + 0x16, 0x0040);
  wr16(DP + 0x14, (uint16_t)(rd16(0x019A) - 0x0014));
  for(uint32_t y = 0; y != 0x28; y += 2) {
    const uint32_t x = rd16(0x1E6A + y);
    wr16(0xFAB6 + x, rd16(DP + 0x14));
    wr16(DP + 0x14, (uint16_t)(rd16(DP + 0x14) + 1));
    wr16(0xFABB + x, rd16(DP + 0x16));
    wr16(DP + 0x16, (uint16_t)(rd16(DP + 0x16) ^ 0xFF80));
  }
  wr16(DP + 0x14, 0);
  wr16(DP + 0x16, 0);
  for(uint32_t y = 0; y != 0x28; y += 2) {
    const uint32_t x = rd16(0x1E6A + y);
    wr16(0xFAE6 + x, rd16(DP + 0x14));
    wr16(DP + 0x14, (uint16_t)(rd16(DP + 0x14) + 0x0600));
    wr16(0xFAE8 + x, rd16(DP + 0x16));
    wr16(DP + 0x16, (uint16_t)(rd16(DP + 0x16) + 0x0300));
  }
  wr16(0xFADE, 2);
  wr16(0xFB13, 2);
  wr16(0xFED9, 0xFFFF);
  wr16(0xFEDB, 0x7FFF);
  uint32_t p = (rd16(0xFAB5) | (uint32_t)rd16(0xFAB7) << 16) - 0x4040;
  wr16(0x0225, (uint16_t)p);
  wr16(0x0227, (uint16_t)(p >> 16));
  wr16(0xFAC5, 2);
  p = (rd16(0xFAEA) | (uint32_t)rd16(0xFAEC) << 16) - 0x4040;
  wr16(0x02D8, (uint16_t)p);
  wr16(0x02DA, (uint16_t)(p >> 16));
  wr16(0xFAFA, 4);
  wr16(0x023E, (uint16_t)(rd16(0x0226) - 0x0020));
  wr16(0x02F1, (uint16_t)(rd16(0x02D9) - 0x0020));
  wr16(0xFED9, 0xFFFF);
  wr16(0xFEDB, 0x7FFF);
  wr16(0x1E92, 0x0424);
  wr16(0x1EDE, 0xFFFF);
  wr16(0x1EE2, 0xFFFF);
  race_section_seek_1();
  race_section_seek_2();
  player_segment_step_long();
  player2_segment_step_long();
  wr16(0x1EE6, 0);
  wr16(0x1EE8, 0);
  wr16(0x1EDE, 0xFFFF);
  wr16(0x1EE2, 0xFFFF);
  wr16(0x0204, rd16(0xFAB9));
  wr16(0x0206, rd16(0xFABB));
  wr16(0x02B7, rd16(0xFAB9 + 0x35));
  wr16(0x02B9, rd16(0xFABB + 0x35));
}

// TG3000_ROUTINE($82:A28B) race_state_reset
void race_state_reset(void) {
  wr16(0x00B4, 0);
  wr16(0x00B6, 0);
  wr16(0x00B8, 0);
  wr16(0x028B, 0x0040);
  wr16(0x028D, 0xFFC0);
  if((rd16(0x03DC) & 0x0030) != 0x0030) {
    wr16(0x03C2, rd16(0x0393));
    wr16(0x03C4, rd16(0x0395));
    if((rd16(0x0393) & 0x00FF) != 0x0005) wr16(0x03C2, (uint16_t)(rd16(0x03C2) + 1));
  }
  wr16(0x1ED0, 0);
  wr16(0x025C, 0);
  wr16(0x030F, 0);
  wr16(0x027C, 0x001C);
  wr16(0x032F, 0x001C);
  wr16(0x1F18, 0xFFFF);
  wr16(0x1F1A, 0xFFFF);
  wr16(0x1F24, 0xFFFF);
  wr16(0x1F26, 0xFFFF);
  wr16(0x0216, 0);
  wr16(0x0218, 0);
  wr16(0x02C9, 0);
  wr16(0x02CB, 0);
  wr16(0x0278, 0);
  wr16(0x027A, 0);
  wr16(0x032B, 0);
  wr16(0x032D, 0);
  wr16(0x0253, 0);
  wr16(0x0306, 0);
  wr16(0x0223, 0);
  wr16(0x02D6, 0);
  wr16(0x0285, 0);
  wr16(0x0338, 0);
  wr16(0x1F08, 0);
  wr16(0x1F0A, 0);
  wr16(0x1F0C, 0);
  wr16(0x4E3D, 0x0000);
  wr16(0x4E3F, 0x0000);
  wr16(0x4E41, 0x0000);
  wr16(0x4E43, 0x0000);
  wr16(0x0233, 0x0001);
  wr16(0x02E6, 0x0001);
  wr16(0x0295, 0x0001);
  wr16(0x0348, 0x0001);
  wr16(0x1F0E, 0);
  wr16(0x1F10, 0);
  wr16(0x1F12, 0);
  wr16(0x1F14, 0);
  wr16(0x0231, 0);
  wr16(0x02E4, 0);
  wr16(0x0258, 0x0028);
  wr16(0x030B, 0x0028);
  wr16(0x0256, 0xFFFF);
  wr16(0x0309, 0xFFFF);
  wr16(0x025A, 0);
  wr16(0x030D, 0);
  wr16(0x1F32, (uint16_t)(rd16(0x1F32) & ~0x00FF));
  wr16(0x1E94, 0x8000);
  wr16(0x1ECE, 0);
  wr16(0x021F, 0);
  wr16(0x02D2, 0);
  wr16(0x0244, 0x0001);
  wr16(0x02F7, 0x0001);
  wr16(0x0221, 0);
  wr16(0x02D4, 0);
  wr16(0x021A, 0);
  wr16(0x02CD, 0);
  wr16(0x0212, 0);
  wr16(0x0214, 0);
  wr16(0x02C5, 0);
  wr16(0x02C7, 0);
  wr16(0x1E5E, 0);
  wr16(0x1E62, 0);
  wr16(0x1E60, 0);
  wr16(0x1E64, 0);
  wr16(0x1E54, 0);
  wr16(0x1E5A, 0);
  wr16(0x023A, 0x0300);
  wr16(0x02ED, 0x0300);
  wr16(0x00E8, 0x000C);
  wr16(0x00E6, 0x0008);
  wr16(0x00E4, 0x0004);
  wr16(0x00E0, 0x0000);
  wr16(0x1E2E, 0x06C2);
  wr16(0x1E32, 0x06E0);
  wr16(0x1E36, 0x06FE);
  wr16(0x1E3A, 0x071C);
  wr16(0x1E30, 0x073A);
  wr16(0x1E34, 0x0758);
  wr16(0x1E38, 0x0776);
  wr16(0x1E3C, 0x0794);
  wr16(0x00BE, 0);
  wr16(0x00C0, 0);
  wr16(0x00BC, 0);
  wr16(0x0486, 0);
  wr16(0x00C4, 0);
  wr16(0x00C6, 0);
  wr16(0x00C2, 0);
  wr16(0x050E, 0);
  wr16(0x0176, 0);
  wr16(0x017A, 0);
  wr16(0x0208, 0);
  wr16(0x02BB, 0);
  wr16(0x026A, 0);
  wr16(0x031D, 0);
  wr16(0x0200, 0);
  wr16(0x0202, 0);
  wr16(0x02B3, 0);
  wr16(0x02B5, 0);
  wr16(0x0262, 0);
  wr16(0x0264, 0);
  wr16(0x0315, 0);
  wr16(0x0317, 0);
  wr16(0x023E, 0);
  wr16(0x02F1, 0);
  wr16(0x0210, 0);
  wr16(0x02C3, 0);
  wr16(0x0272, 0);
  wr16(0x0325, 0);
  wr16(0x0225, 0);
  wr16(0x0227, 0);
  wr16(0x02D8, 0);
  wr16(0x02DA, 0);
  wr16(0x0242, 0);
  wr16(0x02F5, 0);
  wr16(0x027E, 0x8000);
  wr16(0x0331, 0x8000);
  // 8-bit stores
  wr8(0x024E, 0x04);
  wr8(0x0301, 0x04);
  wr8(0x024F, 0);
  wr8(0x0302, 0);
  wr8(0x0251, 0x04);
  wr8(0x0304, 0x04);
  wr8(0x0252, 0x04);
  wr8(0x0305, 0x04);
  wr8(0x0250, 0x96);
  wr8(0x0303, 0x96);
  wr8(0x0280, 0);
  wr8(0x0333, 0);
  wr16(0x014C, 0);
  wr16(0x014E, 0);
  wr16(0x025C, rd8(0x024E));
  wr16(0x030F, rd8(0x0301));
  wr16(0x0240, 0);
  wr16(0x02F3, 0);
  wr16(0x003E, 0);
  wr16(0x0040, 0);
  wr16(0x00FE, 0);
  wr16(0x0100, 0);
  wr16(0x01A0, 0);
  wr16(0x01A2, 0);
  wr16(0x01A4, 0);
  wr16(0x01A6, 0);
  wr16(0x1E02, 0);
  wr16(0x1E0A, 0);
  wr16(0x0204, 0);
  wr16(0x0206, 0);
  wr16(0x02B7, 0);
  wr16(0x02B9, 0);
  wr16(0x020A, 0);
  wr16(0x02BD, 0);
  wr16(0x020C, 0);
  wr16(0x02BF, 0);
  static const uint16_t tables[] = {0x272C, 0x282C, 0x2B2C, 0x2C2C, 0x292C, 0x2A2C, 0x2D2C, 0x2E2C};
  for(uint32_t x = 0; x < 8; x += 2)
    for(unsigned i = 0; i < sizeof tables / sizeof tables[0]; i++) wr16(tables[i] + x, 0x18A6);
  // the colour offsets: each 5-bit channel less 6, 7, 8
  uint16_t v = rd16(0x6A33);
  wr16(0x4E55, (uint16_t)(((v & 0x001F) - 6) | 0x0020));
  wr16(0x4E57, (uint16_t)((((v & 0x03E0) >> 5) - 7) | 0x0040));
  wr16(0x4E59, (uint16_t)((((v & 0x7C00) >> 10) - 8) | 0x0080));
  v = rd16(0x6B33);
  wr16(0x4E5B, (uint16_t)((v & 0x001F) - 6));
  wr16(0x4E5D, (uint16_t)(((v & 0x03E0) >> 5) - 7));
  wr16(0x4E5F, (uint16_t)(((v & 0x7C00) >> 10) - 8));
  for(uint32_t y = 0; y < 0x14; y++)  // the cars' order: their record offsets
    wr16(0x1E6A + 2 * y, (uint16_t)dsp4_multiply((int16_t)rd8(0x0446 + y), 0x0035));
  race_grid_setup();
  wr16(0x00EA, 0xA411);
  for(uint32_t i = 0; i < 0x11; i++) wr8(0x4CCC + i, bus_rd8(0x83A411 + i));
  uint16_t at = (uint16_t)((uint16_t)dsp4_multiply(0x001E, (int16_t)rd8(0x0392)) + 0xA118);
  for(uint32_t i = 0; i < 0x1E; i++) wr8(0x2140 + i, bus_rd8(0x850000 + (uint32_t)(uint16_t)(at + i)));
  at = (uint16_t)((uint16_t)dsp4_multiply(0x001E, (int16_t)rd8(0x03C1)) + 0xA118);
  for(uint32_t i = 0; i < 0x1E; i++) wr8(0x215E + i, bus_rd8(0x850000 + (uint32_t)(uint16_t)(at + i)));
  track_palettes_load(DP);
  wr16(0x1E66, 0);
  wr16(0x1E68, 0);
  for(uint32_t i = 0; i < 0x28; i++) {
    const uint16_t w = bus_rd16(0x97A2B8 + 2 * i);
    for(uint32_t k = 0; k < 8; k += 2) wr16(0x2000 + 8 * i + k, w);
  }
  wr16(DP + 0x14, 0);
  for(uint32_t x = 0x14; x != 0; x -= 2) wr16(0x0444 + x, 0xFFFF);
}

// A track object's place as an event's position: its byte * 16, a signed 12-bit value.
static uint16_t track_event_place(uint16_t y) {
  uint16_t at = (uint16_t)(rd8(0x8CB3 + (uint32_t)y) << 4);
  if(at & 0x0800) at |= 0xF000;
  return at;
}

// One road's track event written: `pattern` (unless 0) to its pattern word, the near and
// far positions to its two words and its rows at D+$1A; its flag row's bit 0 set (`flag`).
typedef struct { uint16_t pattern, nearWord, farWord, nearRow, farRow, flagRow; } TrackEventRoad;
static const TrackEventRoad TRACK_ROAD_1 = {0x1F0A, 0x1F0E, 0x1F10, 0x3C6C, 0x3F6C, 0x3D6C};
static const TrackEventRoad TRACK_ROAD_2 = {0x1F0C, 0x1F12, 0x1F14, 0x41EC, 0x44EC, 0x42EC};
static void track_event_put(const TrackEventRoad* road, uint16_t pattern, uint16_t nearAt, uint16_t farAt, bool flag) {
  const uint32_t x = rd16(DP + 0x1A);
  if(pattern) wr16(road->pattern, pattern);
  wr16(road->nearWord, nearAt);
  wr16(road->nearRow + x, nearAt);
  wr16(road->nearRow + 0x40 + x, nearAt);
  wr16(road->farWord, farAt);
  wr16(road->farRow + x, farAt);
  wr16(road->farRow + 0x40 + x, farAt);
  if(flag) {
    const uint16_t f = (uint16_t)(rd16(road->flagRow + x) | 0x0001);
    wr16(road->flagRow + x, f);
    wr16(road->flagRow + 0x40 + x, f);
  }
}

// TG3000_ROUTINE($82:EA93) track_event_20c7
// TG3000_ROUTINE($82:EAFE) track_event_place_only
// TG3000_ROUTINE($82:EB45) track_event2_set (and $82:EB4B, $82:EB51, $82:EB57, $82:EB98)
// TG3000_ROUTINE($82:EB5D) track_event2_20c7
// TG3000_ROUTINE($82:EB9E) track_event2_clear
// TG3000_ROUTINE($82:EBC2) track_event2_place_only
// Object `y`'s track event (kind: its +1 byte), by its handler in the table at $82:819E.
// The first road's handlers are track_event_set/_clear and two never run ($82:EA93: pattern
// $20C7, both positions the object's place; $82:EAFE: func_82EC09's positions, no pattern,
// no flag); the second road's ($82:EB45-$82:EC08, never run) mirror them on $1F0C, $1F12/
// $1F14 and the rows $41EC/$44EC/$42EC.
static void track_event_run(uint16_t kind, uint16_t y) {
  const uint16_t handler = bus_rd16(0x82819E + 2u * kind);
  uint16_t at;
  switch(handler) {
    case 0xEA81: track_event_set(0x2007, y); break;
    case 0xEA87: track_event_set(0x2047, y); break;
    case 0xEA8D: track_event_set(0x2087, y); break;
    case 0xEACE: track_event_set(0x2107, y); break;
    case 0xEAD4: track_event_set(0xC2E7, y); break;
    case 0xEADA: track_event_clear(); break;
    case 0xEA93:  // never run
      at = track_event_place(y);
      track_event_put(&TRACK_ROAD_1, 0x20C7, at, at, true);
      break;
    case 0xEAFE:  // never run
      at = func_82EC09(y);
      track_event_put(&TRACK_ROAD_1, 0, at, (uint16_t)(at + rd16(DP + 0x1E)), false);
      break;
    case 0xEB45: case 0xEB4B: case 0xEB51: case 0xEB57: case 0xEB98: {  // never run
      const uint16_t pattern = handler == 0xEB45 ? 0x2007 : handler == 0xEB4B ? 0x2047 : handler == 0xEB51 ? 0x2087
                             : handler == 0xEB57 ? 0xC2E7 : 0x2107;
      at = func_82EC09(y);
      track_event_put(&TRACK_ROAD_2, pattern, at, (uint16_t)(at + rd16(DP + 0x1E)), true);
      break;
    }
    case 0xEB5D:  // never run
      at = track_event_place(y);
      track_event_put(&TRACK_ROAD_2, 0x20C7, at, at, true);
      break;
    case 0xEB9E:  // never run
      track_event_put(&TRACK_ROAD_2, 0, 0, 0, true);
      break;
    case 0xEBC2:  // never run
      at = func_82EC09(y);
      track_event_put(&TRACK_ROAD_2, 0, at, (uint16_t)(at + rd16(DP + 0x1E)), false);
      break;
    default:  // $EA80: nothing
      break;
  }
}

// One road object spawned from the stream at $7E:5B91 + x into slot y (the tables at
// $7E:F433-$F933): its kind's entry (`table`, `field` its offsets), its graphics slot's
// place added, its lane and row from the stream's bytes; func_85CDAF. Returns the next slot.
static uint16_t road_object_spawn(uint32_t x, uint16_t y, uint16_t kind2, uint32_t table, uint32_t field, uint16_t base) {
  const uint32_t t = bus_rd16(table + kind2);
  const uint32_t s = y;
  wr16(0xF533 + s, bus_rd16(0x860000 + field + t));
  wr16(0xF433 + s, bus_rd16(0x860002 + field + t));
  wr16(0xF733 + s, 0x007F);
  wr16(0xF533 + s, (uint16_t)(rd16(0x6591 + (uint32_t)bus_rd16(0x860000 + t)) + rd16(0xF533 + s)));
  const uint16_t lane = rd8(0x5B93 + x);
  wr16(0xF633 + s, (uint16_t)(lane << 5));
  wr16(0xF933 + s, (uint16_t)(lane << 1));
  wr16(0xF833 + s, (uint16_t)((rd8(0x5B94 + x) << 1) + base));
  func_85CDAF(y);
  return (uint16_t)((y + 2) & 0x00FF);
}

// TG3000_ROUTINE($82:DFAD) road_rows_build
void road_rows_build(void) {
  // the road's sideways place $01CE over the rows crossed ($7E:3AEE, by DSP-4 products)
  uint32_t x = (uint16_t)((uint16_t)(rd16(0x023E) + rd16(0x1EE6)) << 1) & 0x003E;
  uint16_t crossed = (uint16_t)(rd16(0x0226) - rd16(0x023E));
  if(crossed != 0) {
    const uint16_t back = (uint16_t)(0 - rd16(0x023C));
    uint32_t p = (uint32_t)dsp4_multiply((int16_t)((back >> 8) & 0x00FF), (int16_t)rd16(0x3AEE + x));
    wr16(DP + 0x14, (uint16_t)((p << 8) & 0xFF00));
    wr16(DP + 0x16, (uint16_t)((p >> 8) & 0x00FF));
    uint16_t a = (uint16_t)(p >> 8);
    wr16(0x01CE, a);
    x += 2;
    if(--crossed != 0) {
      uint32_t c = rd16(0x023C) == 0;  // the carry of the SBC above
      do {
        const uint32_t sum = (uint32_t)a + rd16(0x3AEE + x) + c;
        a = (uint16_t)sum;
        c = sum >> 16;
        x += 2;
      } while(--crossed != 0);
      wr16(0x01CE, a);
    }
    p = (uint32_t)dsp4_multiply((int16_t)(uint16_t)((rd16(0x0224) & 0xFF00) >> 1), (int16_t)(uint16_t)(rd16(0x3AEE + x) << 1));
    wr16(0x01CE, (uint16_t)((uint16_t)(p >> 16) + rd16(0x01CE)));
  } else {
    const uint32_t p = (uint32_t)dsp4_multiply((int16_t)(uint16_t)((uint16_t)((rd16(0x0224) & 0xFF00) - rd16(0x023C)) >> 1),
                                               (int16_t)(uint16_t)(rd16(0x3AEE + x) << 1));
    wr16(0x01CE, (uint16_t)(p >> 16));
  }
  uint16_t row = (uint16_t)(rd16(0x023E) + 0x0020 + rd16(0x1EE6));
  if(row >= rd16(0x019A)) row = (uint16_t)(row - rd16(0x019A));
  wr16(DP + 0x16, row);
  wr16(0x023C, (uint16_t)(rd16(0x0224) & 0xFF00));
  const uint16_t rows = (uint16_t)(rd16(0x0226) - rd16(0x023E));
  if(rows != 0) {
    wr16(DP + 0x14, rows);
    wr16(0x0240, (uint16_t)(rows + rd16(0x0240)));
    wr16(DP + 0x22, rd16(0x1EDE));
    x = (uint16_t)(rd16(0x023E) << 1) & 0x003E;
    wr16(0x1EF8, (uint16_t)(rd16(0x3E6E + x) - rd16(0x3E6C + x)));
    wr16(0x1EFA, (uint16_t)(rd16(0x3DEE + x) - rd16(0x3DEC + x)));
    // each new row of the road (D+$14 of them), at row D+$16
    do {
      if(rd16(DP + 0x16) >= rd16(DP + 0x22)) {  // the armed jump: the rows past it
        wr16(DP + 0x16, (uint16_t)(rd16(DP + 0x16) + rd16(0x1EE0)));
        wr16(DP + 0x22, 0x7FFF);
      }
      const uint16_t d16 = rd16(DP + 0x16);
      wr16(DP + 0x18, (uint16_t)((uint16_t)dsp4_multiply((int16_t)((uint16_t)(d16 << 2) >> 8), 0x000A) + rd16(DP + 0x98)));
      const uint32_t entry = rd16(DP + 0x18);
      const uint32_t y = (uint16_t)(d16 << 1) & 0x003E;
      wr16(DP + 0x1A, (uint16_t)y);
      wr16(DP + 0x1C, (uint16_t)(d16 & 0x003E));
      const uint32_t c16 = d16 >> 15;  // the ASL's carry
      uint32_t at = (uint16_t)(rd16(DP + 0x1C) + bus_rd16(0x840000 + entry) + c16);
      wr16(0x3AEC + y, bus_rd16(0x850000 + at));
      wr16(0x3B2C + y, bus_rd16(0x850000 + at));
      at = (uint16_t)(rd16(DP + 0x1C) + bus_rd16(0x840002 + entry));
      const uint16_t up = (uint16_t)(bus_rd16(0x850000 + at) - rd16(0x1E1E));
      wr16(0x3B6C + y, up);
      wr16(0x3BAC + y, up);
      if((y & 0x0006) == 0x0006) wr16(0x1E1E, bus_rd16(0x850000 + at));
      uint16_t a = (y & 0x0002) ? (uint16_t)(rd16(DP + 0x1C) | 0x0001) : rd16(DP + 0x1C);
      a = (uint16_t)(a << 1);
      if(!(a & 0x0040)) at = (uint16_t)(a + bus_rd16(0x840006 + entry));
      else at = (uint16_t)((a & 0x003F) + bus_rd16(0x840008 + entry));
      wr16(0x3CEC + y, rd16(0x5391 + at));
      wr16(0x3D2C + y, rd16(0x5391 + at));
      // the first pair of the list at $85:(+4) whose limit reaches the row
      uint32_t list = bus_rd16(0x840004 + entry);
      uint16_t v;
      for(;;) {
        v = bus_rd16(0x850000 + list);
        if(bus_rd16(0x850002 + list) >= rd16(DP + 0x1A)) break;
        v = bus_rd16(0x850004 + list);  // never run
        if(bus_rd16(0x850006 + list) >= rd16(DP + 0x1A)) break;
        list = (uint16_t)(list + 8);
      }
      wr16(0x3BEC + y, v);
      wr16(0x3C2C + y, v);
      wr16(0x3D6C + y, 0);
      wr16(0x3DAC + y, 0);
      uint32_t seg = (uint16_t)(rd16(DP + 0x9C) + rd16(DP + 0xA2));
      if(rd16(DP + 0x16) < bus_rd16(0x840000 + seg)) {  // before the segment
        if((uint16_t)(rd16(DP + 0x16) + 1) == rd16(DP + 0x22) && (rd16(0x1E66) & 0x8000)) {
          wr16(0x3D6C + y, 0x8002);
          wr16(0x3DAC + y, 0x8002);
        }
        const uint16_t edge = (uint16_t)(((rd16(DP + 0x16) & 0x0007) << 3) + rd16(0x1F0A));
        wr16(0x3FEC + y, edge);
        wr16(0x402C + y, edge);
        goto flat;
      }
      uint16_t diff = (uint16_t)(rd16(DP + 0x16) - bus_rd16(0x840000 + seg));
      while(diff >= 0x0040) {  // past the segment's end: the next ones
        const uint16_t f = (uint16_t)(0x0008 | rd16(0x3D6C + y));
        wr16(0x3D6C + y, f);
        wr16(0x3DAC + y, f);
        wr16(0x3FEC + y, rd16(0x3CEA + y));
        wr16(0x402C + y, rd16(0x3CEA + y));
        wr16(DP + 0xA2, (uint16_t)(rd16(DP + 0xA2) + 0x0014));
        if(!(rd16(0x1E66) & 0x8000)) {
          wr16(0x1EE6, bus_rd16(0x840010 + seg));
          wr16(DP + 0x16, (uint16_t)(rd16(0x1EE6) + rd16(DP + 0x16)));
        }
        seg = (uint16_t)(rd16(DP + 0x9C) + rd16(DP + 0xA2));
        if(rd16(DP + 0x16) < bus_rd16(0x840000 + seg)) goto flat;
        diff = (uint16_t)(rd16(DP + 0x16) - bus_rd16(0x840000 + seg));
      }
      if(diff == 0x003E && (rd16(0x1E66) & 0x8000)) {
        wr16(0x3D6C + y, 0x8001);
        wr16(0x3DAC + y, 0x8001);
      }
      {
        const uint16_t d1e = (uint16_t)(diff << 1);
        wr16(DP + 0x1E, d1e);
        wr16(DP + 0x20, (uint16_t)seg);
        at = (uint16_t)(d1e + bus_rd16(0x840004 + seg));
        wr16(0x3DEC + y, bus_rd16(0x850000 + at));
        wr16(0x3E2C + y, bus_rd16(0x850000 + at));
        at = (uint16_t)(d1e + bus_rd16(0x840006 + seg));
        wr16(0x3E6C + y, bus_rd16(0x850000 + at));
        wr16(0x3EAC + y, bus_rd16(0x850000 + at));
        if(bus_rd16(0x82E261 + (uint32_t)bus_rd16(0x84000A + seg)) == 0xE265) {  // a flat shape
          wr16(0x3C6C + y, 0xFF00);
          wr16(0x3CAC + y, 0xFF00);
          wr16(0x3F6C + y, 0x0100);
          wr16(0x3FAC + y, 0x0100);
        } else if(rd16(0x3DEC + y) & 0x8000) {  // $E279: the edges by the curve's sign
          const uint16_t e = (uint16_t)((rd16(0x3DEC + y) >> 1) | 0x8000);
          wr16(0x3C6C + y, e);
          wr16(0x3CAC + y, e);
          wr16(0x3F6C + y, 0x3FFF);
          wr16(0x3FAC + y, 0x3FFF);
        } else {
          const uint16_t e = (uint16_t)(rd16(0x3DEC + y) >> 1);
          wr16(0x3F6C + y, e);
          wr16(0x3FAC + y, e);
          wr16(0x3C6C + y, 0xC001);
          wr16(0x3CAC + y, 0xC001);
        }
        uint32_t s2 = rd16(DP + 0x20);
        uint16_t b = d1e;
        if(b >= 0x0040) {
          b = (uint16_t)(b - 0x0040);
          s2 += 2;
        }
        at = (uint16_t)(b + bus_rd16(0x84000C + s2));
        wr16(0x3FEC + y, rd16(0x5391 + at));
        wr16(0x402C + y, rd16(0x5391 + at));
        goto event;
      }
    flat:  // $E2D5: no curve, the edges at the events' places
      wr16(0x3DEC + y, 0);
      wr16(0x3E2C + y, 0);
      wr16(0x3E6C + y, 0);
      wr16(0x3EAC + y, 0);
      wr16(0x3C6C + y, rd16(0x1F0E));
      wr16(0x3CAC + y, rd16(0x1F0E));
      wr16(0x3F6C + y, rd16(0x1F10));
      wr16(0x3FAC + y, rd16(0x1F10));
    event: {
        const uint16_t obj = (uint16_t)(rd16(DP + 0x16) << 2);
        const uint16_t kind = rd8(0x8CB4 + (uint32_t)obj);
        if(kind != 0) track_event_run(kind, obj);
      }
      wr16(DP + 0x16, (uint16_t)(rd16(DP + 0x16) + 1));
      if(rd16(DP + 0x16) == rd16(0x019A)) {
        wr16(DP + 0x16, 0);
        wr16(DP + 0xA2, 0);
      }
      wr16(DP + 0x14, (uint16_t)(rd16(DP + 0x14) - 1));
    } while(rd16(DP + 0x14) != 0);
    // the road objects whose place came up, and the lap's wrap
    for(;;) {
      uint32_t ox = rd16(DP + 0xA0);
      if(rd16(0x0226) >= rd16(0x5B91 + ox)) {
        uint16_t slot = rd16(0x0100);
        do {
          const uint16_t v = rd16(0x5B94 + ox);
          if(v == 0) {
            wr16(0x45EC, (uint16_t)(rd8(0x5B93 + ox) << 1));
          } else if(v & 0xFF00) {
            slot = road_object_spawn(ox, slot, (uint16_t)(v << 1), 0x86AC36, 0x22, 0x495C);
          } else {
            slot = road_object_spawn(ox, slot, (uint16_t)(v << 1), 0x86AE36, 0x1E, 0x45EC);
          }
          ox += 5;
          wr16(DP + 0xA0, (uint16_t)ox);
        } while(rd16(0x0226) >= rd16(0x5B91 + ox));
        wr16(0x0100, slot);
      }
      if(rd16(0x0226) < rd16(0x019A)) break;
      wr16(0x0226, (uint16_t)(rd16(0x0226) - rd16(0x019A)));
      wr16(0x023E, rd16(0x0226));
      wr16(DP + 0xA0, 0);
      wr16(0x1E5E, 0);
      wr16(0x1E60, 0);
      wr16(0x45EC, 0x0140);
    }
    wr16(0x023E, rd16(0x0226));
    // the road's kind at the player ($0260): the segment it is in
    wr16(0x0260, 0);
    for(uint32_t s = rd16(DP + 0x9C);; s = (uint16_t)(s + 0x0014)) {
      const uint16_t ahead = (uint16_t)(rd16(0x0226) + 2);
      const uint16_t start = bus_rd16(0x840000 + s);
      if(ahead < start) break;
      const uint16_t into = (uint16_t)(ahead - start);
      if(into >= 0x0041) continue;
      if((bus_rd16(0x840002 + s) & 0x00FF) == 0x000A && (uint16_t)(rd16(0x1EDE) + 1) == start) break;
      wr16(0x0260, bus_rd16(0x840002 + s));
      break;
    }
    wr16(0x0142, (uint16_t)(rd16(0x0260) & 0x00FF));
  }
  // $E497: the road's tilt $0237 from the heights a row ahead and three behind
  uint16_t a = (uint16_t)(rd16(0x0226) + 1);
  if(a >= rd16(0x019A)) a = (uint16_t)(a - rd16(0x019A));
  wr16(DP + 0x14, func_82EA49(a));
  a = rd16(0x0226);
  if(a < 3) a = (uint16_t)(a - 3 + rd16(0x019A));
  else a = (uint16_t)(a - 3);
  wr16(DP + 0x16, func_82EA49(a));
  const uint16_t d16 = rd16(DP + 0x16);
  const uint32_t p = (uint32_t)dsp4_multiply((int16_t)(uint16_t)(rd16(DP + 0x14) - d16), (int16_t)(uint16_t)((rd16(0x0225) + 0x0100) & 0x03FF));
  const uint16_t top = (d16 & 0x8000) ? 3 : 0;
  const uint16_t r = (uint16_t)((uint16_t)(d16 << 2) | top);
  uint16_t s = (uint16_t)(((r >> 8) | (r << 8)) & 0x03FF);
  if(s & 0x0080) s |= 0xFF00;
  wr16(0x0237, (uint16_t)-(uint16_t)((uint16_t)(s + (uint16_t)(p >> 16)) + 9));
}

// TG3000_ROUTINE($82:DFA9) road_rows_build_call
void road_rows_build_call(void) { road_rows_build(); }

// the order list's i-th car (0: last place, 19: the leader)
static uint16_t cars_order(int i) { return rd16(ORDER_LIST + 2 * (uint32_t)i); }
static void cars_order_set(int i, uint16_t car) { wr16(ORDER_LIST + 2 * (uint32_t)i, car); }

// +$1A the car behind, +$18 the car ahead (round)
static void cars_order_link(void) {
  for(int i = 0; i < 20; i++) {
    const uint32_t x = cars_order(i);
    wr16(0xFACF + x, cars_order((i + 19) % 20));
    wr16(0xFACD + x, cars_order((i + 1) % 20));
  }
}

static void cars_handler(uint16_t car) {
  const uint16_t kind = rd16(0xFAC5 + (uint32_t)car);
  switch(bus_rd16(0x81F876 + kind)) {
    case 0xF887: car_road_step(car); break;
    case 0xFA8F: (void)car_sync_player(car); break;
    case 0xFB20: (void)car_sync_player2(car); break;  // never run
    case 0xF886: break;  // an RTS
    case 0xFC72: car_sync_slot(car, 0x0000); break;  // never run
    case 0xFCD1: car_sync_slot(car, 0x00B3); break;  // never run
    case 0xFD30: car_sync_slot(car, 0x0062); break;  // never run
    case 0xFD8F: car_sync_slot(car, 0x0115); break;  // never run
    default: break;  // not a handler (no kind the game sets)
  }
}

// pair k (cars k and k+1, k+1 meant to be ahead) and on down: the original's unrolled
// insertion sort
static void cars_order_sort(void) {
  wr16(DP + 0x3C, 0);
  int k = 18;
  uint16_t x = cars_order(18);
  for(;;) {
    const uint16_t y = cars_order(k + 1);
    const uint32_t lo_y = rd16(0xFAB5 + (uint32_t)y), lo_x = rd16(0xFAB5 + (uint32_t)x);
    const uint32_t hi_y = rd16(0xFAB7 + (uint32_t)y), hi_x = rd16(0xFAB7 + (uint32_t)x);
    const uint32_t borrow = lo_y < lo_x ? 1 : 0;
    const uint16_t hi = (uint16_t)(hi_y - hi_x - borrow);
    bool swap = false;
    if(hi == 0) (void)car_contacts_chain(x, y);
    else {
      wr16(DP + 0x3C, 0);
      if(hi & 0x8000) {
        func_81F251();
        swap = hi_y < hi_x + borrow;  // the SBC's carry clear
      }
    }
    if(swap) {
      cars_order_set(k, y);
      cars_order_set(k + 1, x);
      (void)func_81F1FC();
      if(k != 18) {
        k++;  // the same car against the next one up
        continue;
      }
    }
    if(k == 0) return;
    k--;
    x = cars_order(k);
  }
}

// the leader and the cars after it past the lap's end; returns whether the leader was
static bool cars_wrap_forward(void) {
  const uint16_t lap = rd16(0x019A);
  static const int cars[] = {19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 2, 1, 0};
  int n = 0;
  for(unsigned i = 0; i < sizeof cars / sizeof cars[0]; i++) {
    const uint32_t x = cars_order(cars[i]);
    const uint16_t at = rd16(0xFAB6 + x);
    if(at < lap) {
      if(i == 0) return false;
      break;
    }
    wr16(0xFAB6 + x, (uint16_t)(at - lap));
    if(cars[i] != 0) n++;  // the last one is not counted
  }
  while(n-- > 0) {
    const uint16_t x = cars_order(19);
    for(int i = 19; i > 0; i--) cars_order_set(i, cars_order(i - 1));
    cars_order_set(0, x);
    wr16(0xFAD3 + (uint32_t)x, (uint16_t)(rd16(0xFAD3 + (uint32_t)x) + 1));
    wr16(0xFADE + (uint32_t)x, (uint16_t)(rd16(0xFADE + (uint32_t)x) | 0x0001));
    wr16(0xFAE0 + (uint32_t)x, rd16(0x019C));
  }
  return true;
}

// the last car and the cars after it behind the line; returns whether the last car was
static bool cars_wrap_back(void) {
  const uint16_t lap = rd16(0x019A);
  int n = 0;
  for(int i = 0; i < 20; i++) {
    const uint32_t x = cars_order(i);
    const uint16_t at = rd16(0xFAB6 + x);
    if(!(at & 0x8000)) {
      if(i == 0) return false;
      break;
    }
    wr16(0xFAB6 + x, (uint16_t)(at + lap));
    if(i != 19) n++;  // the last one is not counted
  }
  while(n-- > 0) {
    const uint16_t x = cars_order(0);
    for(int i = 0; i < 19; i++) cars_order_set(i, cars_order(i + 1));
    cars_order_set(19, x);
    wr16(0xFAD3 + (uint32_t)x, (uint16_t)(rd16(0xFAD3 + (uint32_t)x) - 1));
    wr16(0xFADE + (uint32_t)x, (uint16_t)(rd16(0xFADE + (uint32_t)x) & 0xFFFE));
  }
  return true;
}

// TG3000_ROUTINE($81:E812) race_cars_frame
void race_cars_frame(void) {
  cars_order_link();
  for(uint32_t o = 0; o < 20 * 0x35; o += 0x35) {
    wr16(0xFAD5 + o, rd16(0xFABB + o));
    wr16(0xFAC1 + o, rd16(0xFAB5 + o));
    wr16(0xFAC3 + o, rd16(0xFAB7 + o));
    wr16(0xFAC7 + o, 0);
    cars_handler((uint16_t)o);
  }
  do cars_order_sort();
  while(cars_wrap_forward() || cars_wrap_back());
  for(int i = 0; i < 20; i++) {
    const uint32_t x = cars_order(i);
    const uint16_t f = rd16(0xFADE + x);
    if((f & 0x0001) && !(f & 0x0002) && rd16(0xFAD3 + x) == rd16(0x1F06)) {
      wr8(0xFADE + x, (uint8_t)(rd8(0xFADE + x) | 0x04));
      const uint16_t j = rd16(0x1F08);
      wr8(0x0446 + (uint32_t)j, rd8(0xFADD + x));
      wr16(0x1F08, (uint16_t)(j + 1));
    }
    wr16(0xFADE + x, (uint16_t)(rd16(0xFADE + x) & 0xFF7E));
  }
  cars_order_link();
}

static bool dsp4_answers_run(uint16_t* x, uint16_t* a) {
  uint16_t n = dsp4_port_read16();
  if(n == 0) return false;
  do {
    *x = dsp4_port_read16();
    *a = dsp4_port_read16();
    wr16(*x, *a);  // bank $7E (its last byte's word reaches $7F:0000)
  } while(--n != 0);
  return true;
}

// TG3000_ROUTINE($81:89F5) dsp4_answers_store
bool dsp4_answers_store(uint16_t* x, uint16_t* a) {
  (void)dsp4_answers_run(x, a);
  return dsp4_answers_run(x, a);
}

// N of a 16-bit CMP
static bool cmp_n(uint16_t a, uint16_t b) { return ((uint16_t)(a - b) & 0x8000) != 0; }

// the product's high word: command 0 by the port (the answer's low word read and dropped)
static uint16_t road_port_product(uint16_t a, uint16_t b) {
  dsp4_port_write16(0);
  dsp4_port_write16(a);
  dsp4_port_write16(b);
  (void)dsp4_port_read16();
  return dsp4_port_read16();
}

static void road_port_row(uint32_t row) {
  dsp4_port_write16(rd16(0x38EC + row));
  dsp4_port_write16(rd16(0x35EC + row));
  dsp4_port_write16(rd16(0x37EC + row));
  dsp4_port_write16(rd16(0x366C + row));
  dsp4_port_write16(rd16(0x386C + row));
}

static void road_port_pair(uint32_t at) {
  for(int i = 0; i < 2; i++) {
    dsp4_port_write16(rd16(0x3C6C + at));
    dsp4_port_write16(rd16(0x3F6C + at));
  }
}

// a product's high byte, rounded and kept in 0-$FF
static uint16_t road_edge_byte(uint16_t a, uint16_t b) {
  uint16_t v = (uint16_t)(road_port_product(a, b) + 0x0080);
  if(v & 0x8000) v = 0;
  if(v & 0xFF00) v = 0x00FF;
  return v;
}

// TG3000_ROUTINE($81:8564) road_rows_stream
bool road_rows_stream(void) {
  const uint16_t buffer = rd16(0x00B4) ? 0x2E2C : 0x318C;
  wr16(DP + 0x16, buffer);
  wr16(0x1E4E, buffer);
  wr16(DP + 0x1A, (uint16_t)((rd16(0x0226) << 1) & 0x003E));
  for(uint32_t x = buffer, n = 0; n < 0x1B; n++, x += 0x20)
    for(uint32_t k = 0; k < 0x20; k += 4) {
      wr16(x + k, 0x00FF);
      wr16(x + k + 2, 0xFF00);
    }
  uint32_t x = rd16(DP + 0x1A);
  const uint16_t row = (uint16_t)(((rd16(0x0225) >> 1) & 0x007F) << 8);
  wr16(DP + 0x1E, row);
  wr16(DP + 0x1C, (uint16_t)(road_port_product((uint16_t)((rd16(0x3C6E + x) - rd16(0x3C6C + x)) << 1), row) + rd16(0x3C6C + x)));
  wr16(DP + 0x1E, (uint16_t)(road_port_product((uint16_t)((rd16(0x3F6E + x) - rd16(0x3F6C + x)) << 1), rd16(DP + 0x1E)) +
                             rd16(0x3F6C + x)));
  dsp4_port_write16(0x0008);
  static const uint16_t head[] = {0x00FF, 0x00FF, 0x00FF, 0x00FF, 0, 0, 0, 0, 0x00A6, 0x00A6, 0x00A6, 0x00A6,
                                  0x00A5, 0x00A5, 0x00A7, 0x00A7, 0x0081, 0x007F, 0x0081, 0x007F};
  for(unsigned i = 0; i < sizeof head / sizeof head[0]; i++) dsp4_port_write16(head[i]);
  const uint16_t base = rd16(DP + 0x16), p1 = rd16(0x0242), p2 = rd16(0x1EF2);
  uint16_t a = (uint16_t)(base + 0x035C);
  if(p1 & 0x8000) {
    a = (uint16_t)((uint16_t)(p1 << 2) + base + 0x035C);
    if(a < base) a = base;
  }
  dsp4_port_write16(a);
  dsp4_port_write16(a);
  wr16(DP + 0x18, a);
  a = (uint16_t)(base + 0x035E);
  if(p2 & 0x8000) {
    a = (uint16_t)((uint16_t)(p2 << 2) + base + 0x035E);
    if(a < base) a = (uint16_t)(base + 2);
  }
  dsp4_port_write16(a);
  dsp4_port_write16(a);
  a = (p1 & 0x8000) ? (uint16_t)(p1 + 0x0033) : 0x0033;
  dsp4_port_write16(a);
  dsp4_port_write16(a);
  a = (p2 & 0x8000) ? (uint16_t)(p2 + 0x0033) : 0x0033;
  dsp4_port_write16(a);
  dsp4_port_write16(a);
  static const uint16_t mid[] = {0xFF5B, 0xFF5B, 0xFF5B, 0xFF5B, 0x2FC8, 0x0034, 0xFF5C, 0x0035, 0x7FFF};
  for(unsigned i = 0; i < sizeof mid / sizeof mid[0]; i++) dsp4_port_write16(mid[i]);
  dsp4_port_write16(rd16(0x0202));
  dsp4_port_write16((uint16_t)(p1 + 0x0033));
  dsp4_port_write16(rd16(0x1EF6));
  dsp4_port_write16((uint16_t)(p2 + 0x0033));
  x = rd16(DP + 0x1A);
  if(rd16(0x3D6E + x) & 1) {
    wr16(DP + 0x1C, rd16(0x3C6C + x));
    wr16(DP + 0x1E, rd16(0x3F6C + x));
  }
  for(int i = 0; i < 2; i++) {
    dsp4_port_write16(rd16(DP + 0x1C));
    dsp4_port_write16(rd16(DP + 0x1E));
  }
  const uint16_t fill = (uint16_t)(0 - p1);
  if(!(fill & 0x8000) && fill != 0) {  // $0242 negative (never run)
    const uint16_t w = dsp4_port_read16();
    uint32_t at = rd16(DP + 0x18);
    for(uint16_t n = fill < 0x00D8 ? fill : 0x00D8; n != 0; n--) {
      at = (uint16_t)(at + 4);
      wr16(at, w);
    }
  } else (void)dsp4_port_read16();
  wr16(DP + 0x14, 2);
  do {
    road_port_row(rd16(DP + 0x14));
    x = rd16(DP + 0x1A);
    road_port_pair((rd16(0x3D6E + x) & 1) ? x : x + 2);
    uint16_t ax = 0, aa = 0;
    (void)dsp4_answers_store(&ax, &aa);
    if(rd16(0x3D6E + (uint32_t)rd16(DP + 0x1A)) & 1) {  // read again: the answers may land anywhere
      road_port_row(rd16(DP + 0x14));
      road_port_pair(rd16(DP + 0x1A) + 2);
      (void)dsp4_answers_store(&ax, &aa);
    }
    wr16(DP + 0x1A, (uint16_t)(rd16(DP + 0x1A) + 2));
    wr16(DP + 0x14, (uint16_t)(rd16(DP + 0x14) + 2));
  } while(rd16(DP + 0x14) != rd16(0x0190));
  dsp4_port_write16(0x8000);
  const bool far = dsp4_port_read16() != 0;
  static const uint8_t hdma_near[] = {0xC2, 0xB0, 0xA1}, hdma_far[] = {0xC8, 0xB0, 0x21, 0x20, 0x42, 0x82};
  wr16(DP + 0xC8, far ? 1 : 0);
  for(int i = 0; i < 3; i++) wr8(DP + 0xB6 + (uint32_t)i, far ? hdma_far[i] : hdma_near[i]);
  for(int i = 0; i < 3; i++) wr8(DP + 0xB9 + (uint32_t)i, far ? hdma_far[3 + i] : rd8(0x4E55 + 2 * (uint32_t)i));  // never run when far
  wr16(DP + 0x14, 0x3FFF);
  wr16(DP + 0x16, far ? 0xC001 : 0x3FFF);
  for(uint32_t r = 2;; ) {
    uint16_t v = rd16(0x386C + r);
    if(v == rd16(DP + 0x14) || cmp_n(v, rd16(DP + 0x14))) {
      wr16(DP + 0x14, v);
      wr16(DP + 0x18, (uint16_t)r);
    }
    v = rd16(0x37EC + r);
    const bool take = far ? !cmp_n(v, rd16(DP + 0x16)) : (v == rd16(DP + 0x16) || cmp_n(v, rd16(DP + 0x16)));
    if(take) {
      wr16(DP + 0x16, v);
      wr16(DP + 0x1A, (uint16_t)r);
    }
    r = (uint16_t)(r + 2);
    if(r == rd16(0x0194)) break;
  }
  bool split = false;
  if(cmp_n(rd16(DP + 0x16), rd16(DP + 0x14))) {  // never run
    split = true;
    x = rd16(DP + 0x1A);
    wr16(DP + 0x18, rd16(0x34EC + x));
    wr16(DP + 0x1C, rd16(0x38EC + x));
    x = (uint16_t)(((rd16(0x0226) << 1) & 0x003E) + rd16(DP + 0x1A));
    wr16(DP + 0x1A, (uint16_t)-(rd16(0x3F6C + x) + rd16(DP + 0x18)));
    wr16(DP + 0x18, (uint16_t)-(rd16(0x3C6C + x) + rd16(DP + 0x18)));
    const uint16_t lo = road_edge_byte((uint16_t)(rd16(DP + 0x18) << 1), rd16(DP + 0x1C));
    const uint16_t hi = road_edge_byte((uint16_t)(rd16(DP + 0x1A) << 1), rd16(DP + 0x1C));
    wr16(DP + 0x18, (uint16_t)(hi << 8 | lo));
    wr16(DP + 0x14, (uint16_t)(((uint16_t)(rd16(DP + 0x14) + 0x00A5) << 2) + rd16(0x1E4E)));
    uint32_t at = (uint16_t)(((uint16_t)(rd16(DP + 0x16) + 0x00A5) << 2) + rd16(0x1E4E));
    do {
      wr16(0x0002 + at, rd16(DP + 0x18));
      at = (uint16_t)(at + 4);
    } while(at != rd16(DP + 0x14));
  }
  uint16_t e = (uint16_t)(rd16(0x00EA) + 0x0011);
  if(e == 0xA422) e = 0xA411;
  wr16(0x00EA, e);
  return split;
}

static uint16_t xba16(uint16_t v) { return (uint16_t)(v << 8 | v >> 8); }
static uint16_t sext8(uint8_t v) { return (uint16_t)(int16_t)(int8_t)v; }

// a product's two answer words (command 0 by the port)
static void road_port_product32(uint16_t a, uint16_t b, uint16_t* lo, uint16_t* hi) {
  dsp4_port_write16(0);
  dsp4_port_write16(a);
  dsp4_port_write16(b);
  *lo = dsp4_port_read16();
  *hi = dsp4_port_read16();
}

// TG3000_ROUTINE($85:B3D7) road_rows_project
void road_rows_project(void) {
  const bool small = rd16(0x00B4) != 0;
  const uint16_t band = small ? 0x0023 : 0x0045;
  wr16(DP + 0x18, small ? 0x272B : 0x282B);
  wr16(DP + 0x1A, small ? 0x06BC : 0x0D7C);
  wr16(DP + 0x1C, small ? 0x0004 : 0x01A4);
  wr16(DP + 0x14, (uint16_t)(0x0080 - ((rd16(0x0225) >> 1) & 0x007E)));
  wr16(DP + 0x16, (uint16_t)((rd16(0x0226) << 1) & 0x003E));
  const uint32_t x = rd16(DP + 0x16);
  const uint16_t at = rd16(DP + 0x14);
  const uint16_t ahead = xba16((uint16_t)((uint16_t)(0x0080 - at) >> 1)), behind = xba16((uint16_t)(at >> 1));
  uint16_t lo, hi;
  road_port_product32(rd16(0x3AEC + x), 0xFC00, &lo, &hi);
  wr16(0x0174, (uint16_t)(hi + (lo >= 0x8000 ? 1 : 0)));
  road_port_product32(rd16(0x3B6E + x), ahead, &lo, &hi);
  wr16(DP + 0x1E, (uint16_t)(rd16(0x0208) - (uint16_t)(hi << 2 | lo >> 14)));
  road_port_product32(rd16(0x0208), behind, &lo, &hi);
  wr16(DP + 0x20, (uint16_t)(hi << 2 | lo >> 14));
  road_port_product32(rd16(0x3AEE + x), ahead, &lo, &hi);
  {
    const uint16_t d = (uint16_t)-(uint16_t)(hi << 2 | lo >> 14);
    const uint32_t c1 = (hi >> 14) & 1;  // the second ROL's carry, into the ADC
    const uint32_t s = (uint32_t)rd16(0x020B) + d + c1;
    wr16(DP + 0x26, (uint16_t)s);
    wr16(DP + 0x28, (uint16_t)(sext8(rd8(0x020D)) + ((d & 0x8000) ? 0xFFFF : 0) + (s >> 16)));
  }
  road_port_product32(rd16(DP + 0x26), behind, &lo, &hi);
  wr16(DP + 0x2A, hi);
  wr16(DP + 0x2C, (hi & 0x8000) ? 0xFFFF : 0);
  {
    const uint16_t top = (uint16_t)(rd16(DP + 0x28) + ((rd16(DP + 0x26) & 0x8000) ? 1 : 0));
    road_port_product32(top, behind, &lo, &hi);
    const uint32_t s = (uint32_t)lo + rd16(DP + 0x2A);
    const uint16_t h = (uint16_t)(hi + rd16(DP + 0x2C) + (s >> 16));
    wr16(DP + 0x2A, (uint16_t)(s << 2));
    wr16(DP + 0x2C, (uint16_t)(h << 2 | (uint16_t)s >> 14));
  }
  wr16(0x4CFD + (uint32_t)band, 0);
  if(rd16(DP + 0xE4) & 0x8000) dsp4_port_write16(0x0001);
  else {
    dsp4_port_write16(0x000F);
    dsp4_port_write16(rd16(DP + 0xE4));
  }
  dsp4_port_write16(0);
  const uint16_t p = rd16(0x0242);
  const bool in = (p & 0x8000) != 0;
  dsp4_port_write16((uint16_t)(p + 0x0033));
  wr16(0x4CFD + (uint32_t)band, in ? (uint16_t)(p + 0x0033 + 0x00A9) : 0x00DC);
  dsp4_port_write16(in ? (uint16_t)(p + 0x0033) : 0x0033);
  dsp4_port_write16(0xFF5B);
  dsp4_port_write16(0x00BF);
  dsp4_port_write16(in ? (uint16_t)(p + 0x00DC) : 0x00DC);
  dsp4_port_write16(rd16(0x0200));
  uint16_t v = rd16(0x0202);
  if(v != 0) {
    v = (uint16_t)(v - 1);
    if(v & 0x8000) v = (uint16_t)(v + 2);
  }
  dsp4_port_write16(v);
  dsp4_port_write16(0x0180);
  dsp4_port_write16(in ? (uint16_t)((uint16_t)(p << 2) + rd16(DP + 0x1A)) : rd16(DP + 0x1A));
  dsp4_port_write16(0x00C0);
  wr16(DP + 0x22, (uint16_t)(band - 1));
  dsp4_port_write16((uint16_t)(rd16(DP + 0x1D) & 0xFF00));
  dsp4_port_write16(sext8(rd8(DP + 0x1F)));
  dsp4_port_write16((uint16_t)((rd16(DP + 0x25) & 0xFF00) | rd8(0x020A)));
  dsp4_port_write16(rd16(DP + 0x27));
  wr16(DP + 0x24, 2);
  bool first = true;
  do {
    const uint32_t row = rd16(DP + 0x14);
    wr16(DP + 0x30, bus_rd16(0x83ACEC + row));
    dsp4_port_write16(rd16(DP + 0x30));
    if(first) {
      dsp4_port_write16(rd16(DP + 0x20));
      dsp4_port_write16((uint16_t)(rd16(DP + 0x29) & 0xFF00));
      dsp4_port_write16(rd16(DP + 0x2B));
      first = false;
    }
    uint32_t k = rd16(DP + 0x16);
    dsp4_port_write16(rd16(0x3B6E + k));
    dsp4_port_write16(rd16(0x3AEE + k));
    dsp4_port_write16(rd16(0x3BEC + k));
    wr16(DP + 0x16, (uint16_t)(k + 2));
    k = rd16(DP + 0x24);
    wr16(0x34EC + k, dsp4_port_read16());
    wr16(0x35EC + k, dsp4_port_read16());
    wr16(0x36EC + k, dsp4_port_read16());
    wr16(0x37EC + k, dsp4_port_read16());
    wr16(0x38EC + k, rd16(DP + 0x30));
    wr16(0x39EC + k, rd16(DP + 0x14));
    wr16(DP + 0x24, (uint16_t)(k + 2));
    const uint16_t count = dsp4_port_read16();
    if(count != 0) {
      const uint32_t b = rd16(DP + 0x22);
      wr8(0x4CFD + b, (uint8_t)(rd8(0x4CFE + b) - (uint8_t)count));
      const uint32_t s = rd16(0x3CEA + (uint32_t)rd16(DP + 0x16));  // the object's bytes in bank $90
      if(rd16(DP + 0xE4) & 0x8000) {  // copied (MVP, top down)
        const uint16_t y = rd16(DP + 0x18);
        for(uint16_t i = 0; i < 8; i++) wr8((uint16_t)(y - i), bus_rd8(0x900000 + (uint16_t)(s - i)));
        wr16(DP + 0x18, (uint16_t)(y - 8));
      } else {
        const uint16_t src = (uint16_t)(s - 7);
        dsp4_port_write16(bus_rd16(0x83ACEC + (uint32_t)rd16(DP + 0x14)));
        dsp4_port_write16(bus_rd16(0x900000 + src));
        wr16(DP + 0x18, (uint16_t)(rd16(DP + 0x18) - 8));
        wr16(0x0001 + (uint32_t)rd16(DP + 0x18), dsp4_port_read16());
        for(uint32_t w = 2; w < 8; w += 2) {
          dsp4_port_write16(rd16(0x6C33 + (uint32_t)rd16(DP + 0x14)));
          dsp4_port_write16(bus_rd16(0x900000 + (uint16_t)(src + w)));
          wr16(w + 1 + rd16(DP + 0x18), dsp4_port_read16());
        }
      }
      for(uint16_t n = count; n != 0; n--) {
        const uint32_t i = dsp4_port_read16();
        wr16(0x0798 + i, dsp4_port_read16());  // in low WRAM (DB $30)
        wr16(0x0796 + i, dsp4_port_read16());
      }
      wr16(DP + 0x22, (uint16_t)(rd16(DP + 0x22) - 1));
    }
    k = rd16(DP + 0x16);
    const uint16_t flags = rd16(0x3D6A + k);
    if(flags & 0x8000) {
      dsp4_port_write16(0x8001);
      dsp4_port_write16(rd16(0x38EA + (uint32_t)rd16(DP + 0x24)));
      if(flags & 0x0002) {
        wr16(DP + 0x20, sext8(rd8(0x3AED + k)));
        const uint16_t d = (uint16_t)(rd16(0x3DEE + k) - rd16(0x3DF0 + k));
        dsp4_port_write16((uint16_t)-(uint16_t)(d + rd16(0x3DEE + k)));
        dsp4_port_write16((uint16_t)(d - rd16(DP + 0x20)));
      } else {
        wr16(DP + 0x20, sext8(rd8(0x3AEB + k)));
        const uint16_t e = rd16(0x3DEC + k);
        dsp4_port_write16(e);
        dsp4_port_write16((uint16_t)(e - rd16(0x3DEA + k) - rd16(DP + 0x20)));
      }
    }
    wr16(DP + 0x14, (uint16_t)(rd16(DP + 0x14) + 0x0080));
  } while(rd16(DP + 0x14) < 0x0E00);
  dsp4_port_write16(0x8000);
  wr16(DP + 0x90, rd16(DP + 0x24));
  uint16_t end = 0;
  if(in) {
    end = (uint16_t)(p + 0x00DC);
    if((uint16_t)(end - 4) & 0x8000) end = 4;
  }
  wr16(0x4CFD + (uint32_t)band, end);
  wr16(0x0164, rd16(DP + 0x22));
  wr16(0x015C, (uint16_t)(rd16(DP + 0x18) + 1));
  wr16(0x1E3E, rd16(DP + 0x1A));
}

// an object's source byte: bank $90 (ROM) when the pointer is negative, else bank $7E
static uint8_t road_object_byte(bool rom, uint16_t at) { return rom ? bus_rd8(0x900000 + at) : rd8(at); }
static uint16_t road_object_word(bool rom, uint16_t at) {  // a long read (its high byte may cross the bank)
  return rom ? bus_rd16(0x900000 + (uint32_t)at) : rd16(at);
}

// TG3000_ROUTINE($85:B828) road_rows_project2
void road_rows_project2(void) {
  const bool small = rd16(0x00B4) != 0;
  const uint16_t band = small ? 0x0023 : 0x0045;
  wr16(DP + 0x18, small ? 0x292B : 0x2A2B);
  wr16(DP + 0x1A, small ? 0x035C : 0x050C);
  wr16(DP + 0x1C, small ? 0x0004 : 0x01A4);
  wr16(DP + 0x14, (uint16_t)(0x0080 - ((rd16(0x0225) >> 1) & 0x007E)));
  wr16(DP + 0x16, (uint16_t)((rd16(0x0226) << 1) & 0x003E));
  const uint32_t x = rd16(DP + 0x16);
  const uint16_t at = rd16(DP + 0x14);
  const uint16_t ahead = xba16((uint16_t)((uint16_t)(0x0080 - at) >> 1)), behind = xba16((uint16_t)(at >> 1));
  wr16(0x376C, rd16(0x3E6C + x));
  wr16(0x356C, rd16(0x3DEC + x));
  uint16_t lo, hi;
  road_port_product32((uint16_t)((rd16(0x3E6E + x) - rd16(0x3E6C + x)) << 1), (uint16_t)((rd16(0x0224) >> 1) & 0xFF00), &lo, &hi);
  wr16(0x1EF2, (uint16_t)(rd16(0x0242) + hi + rd16(0x3E6C + x)));
  road_port_product32(rd16(0x3AEC + x), 0xFC00, &lo, &hi);
  wr16(0x017C, (uint16_t)(hi + (lo >= 0x8000 ? 1 : 0)));
  road_port_product32(rd16(0x3B6E + x), ahead, &lo, &hi);
  wr16(DP + 0x1E, (uint16_t)(rd16(0x1EDA) - (uint16_t)(hi << 2 | lo >> 14)));
  road_port_product32(rd16(0x3AEE + x), ahead, &lo, &hi);
  wr16(DP + 0x26, (uint16_t)-(uint16_t)(hi << 2 | lo >> 14));
  road_port_product32(rd16(DP + 0x26), behind, &lo, &hi);
  wr16(DP + 0x2A, (uint16_t)(hi << 2 | lo >> 14));
  wr16(0x4D49 + (uint32_t)band, 0);
  if(rd16(DP + 0xE4) & 0x8000) dsp4_port_write16(0x0007);
  else {
    dsp4_port_write16(0x0010);
    dsp4_port_write16(rd16(DP + 0xE4));
  }
  dsp4_port_write16(0);
  const uint16_t q = rd16(0x1EF2);
  const bool in = (q & 0x8000) != 0;
  dsp4_port_write16((uint16_t)(q + 0x0033));
  wr16(0x4D49 + (uint32_t)band, in ? (uint16_t)(q + 0x0033 + 0x00A9) : 0x00DC);
  dsp4_port_write16(in ? (uint16_t)(q + 0x0033) : 0x0033);
  dsp4_port_write16(0xFF5B);
  dsp4_port_write16(0x00BF);
  dsp4_port_write16(in ? (uint16_t)(q + 0x00DC) : 0x00DC);
  dsp4_port_write16(rd16(0x1EF4));
  dsp4_port_write16(rd16(0x1EF6));
  dsp4_port_write16(0x0180);
  dsp4_port_write16(in ? (uint16_t)((uint16_t)(q << 2) + rd16(DP + 0x1A)) : rd16(DP + 0x1A));
  dsp4_port_write16(0x00C0);
  wr16(DP + 0x22, (uint16_t)(band - 1));
  wr16(DP + 0x24, 2);
  do {
    wr16(DP + 0x30, bus_rd16(0x83ACEC + (uint32_t)rd16(DP + 0x14)));
    dsp4_port_write16(rd16(DP + 0x30));
    uint32_t r = rd16(DP + 0x24), k = rd16(DP + 0x16);
    dsp4_port_write16(rd16(0x37EC + r));
    dsp4_port_write16(rd16(0x3E6E + k));
    dsp4_port_write16(rd16(0x35EC + r));
    dsp4_port_write16(rd16(0x3DEE + k));
    dsp4_port_write16(rd16(0x3BEC + k));
    k += 2;
    wr16(DP + 0x16, (uint16_t)k);
    wr16(0x376C + r, (uint16_t)(rd16(0x3E6C + k) + rd16(0x36EC + r)));
    wr16(0x356C + r, (uint16_t)(rd16(0x3DEC + k) + rd16(0x34EC + r)));
    wr16(0x366C + r, dsp4_port_read16());
    wr16(0x386C + r, dsp4_port_read16());
    wr16(0x396C + r, rd16(DP + 0x30));
    wr16(0x3A6C + r, rd16(DP + 0x14));
    wr16(DP + 0x24, (uint16_t)(r + 2));
    const uint16_t count = dsp4_port_read16();
    if(count != 0) {
      const uint32_t b = rd16(DP + 0x22);
      wr8(0x4D49 + b, (uint8_t)(rd8(0x4D4A + b) - (uint8_t)count));
      const uint16_t s = rd16(0x3FEC + (uint32_t)rd16(DP + 0x16));  // the object's bytes
      if(rd16(DP + 0xE4) & 0x8000) {  // copied (MVP, top down), from bank $90 or $7E by its sign
        const bool rom = (s & 0x8000) != 0;
        const uint16_t y = rd16(DP + 0x18);
        for(uint16_t i = 0; i < 8; i++) wr8((uint16_t)(y - i), road_object_byte(rom, (uint16_t)(s - i)));
        wr16(DP + 0x18, (uint16_t)(y - 8));
      } else {
        const uint16_t src = (uint16_t)(s - 7);
        const bool rom = (src & 0x8000) != 0;
        dsp4_port_write16(bus_rd16(0x83ACEC + (uint32_t)rd16(DP + 0x14)));
        dsp4_port_write16(road_object_word(rom, src));
        wr16(DP + 0x18, (uint16_t)(rd16(DP + 0x18) - 8));
        wr16(0x0001 + (uint32_t)rd16(DP + 0x18), dsp4_port_read16());
        for(uint16_t w = 2; w < 8; w += 2) {
          dsp4_port_write16(rd16(0x6C33 + (uint32_t)rd16(DP + 0x14)));
          dsp4_port_write16(road_object_word(rom, (uint16_t)(src + w)));
          wr16((uint32_t)w + 1 + rd16(DP + 0x18), dsp4_port_read16());
        }
      }
      for(uint16_t n = count; n != 0; n--) {
        const uint32_t i = dsp4_port_read16();
        wr16(0x0E58 + i, dsp4_port_read16());  // in low WRAM (DB $30)
        wr16(0x0E56 + i, dsp4_port_read16());
      }
      wr16(DP + 0x22, (uint16_t)(rd16(DP + 0x22) - 1));
    }
    wr16(DP + 0x14, (uint16_t)(rd16(DP + 0x14) + 0x0080));
  } while(rd16(DP + 0x14) < 0x0E00);
  dsp4_port_write16(0x8000);
  wr16(DP + 0x94, rd16(DP + 0x24));
  uint16_t end = 0;
  if(in) {
    end = (uint16_t)(q + 0x00DC);
    if((uint16_t)(end - 4) & 0x8000) end = 4;
  }
  wr16(0x4D49 + (uint32_t)band, end);
  wr16(0x0168, rd16(DP + 0x22));
  wr16(0x0160, (uint16_t)(rd16(DP + 0x18) + 1));
  wr16(0x1E40, rd16(DP + 0x1A));
}

// TG3000_ROUTINE($85:B36F) race_road_frame_alt
uint16_t race_road_frame_alt(void) {
  road_rows_project();
  road_rows_project2();
  wr16(0x1E46, rd16(0x00D0));
  wr16(0x1E4A, rd16(0x00D0));
  (void)scroll_tables_build();
  for(uint32_t i = 0; i < 8; i += 2) wr16(0x016C + i, rd16(0x015C + i));
  wr16(0x01DC, rd16(0x00E0));
  wr16(0x01DE, rd16(0x00E0));
  return race_bands_merge();
}

// TG3000_ROUTINE($85:B3A9) race_road_frame
uint16_t race_road_frame(void) {
  road_rows_project();
  road_rows_project2();
  (void)scroll_tables_build();
  wr16(0x1E46, rd16(0x00D0));
  wr16(0x1E48, rd16(0x00D2));
  wr16(0x016C, rd16(0x015C));
  wr16(0x0170, rd16(0x0160));
  wr16(0x01DC, rd16(0x00E0));
  return race_bands_merge();
}

// TG3000_ROUTINE($8A:84B9) objects_scatter_init
void objects_scatter_init(void) {
  uint16_t y = 0xFE84;
  for(uint32_t x = 0; x != 0x20; x += 2) {
    wr16(0x0510 + x, 0xFFFF);
    wr16(0x054C + x, 0xFFFF);
    wr16(0x0588 + x, 0xFFFF);
    wr16(0x0810 + x, 0x0402);
    wr16(0x084C + x, 0x0403);
    wr16(0x0888 + x, 0x0403);
    wr16(0x0710 + x, y);
    wr16(0x074C + x, y);
    wr16(0x0788 + x, y);
    wr16(0x0610 + x, rng_next(DP + 0x10, 0));
    wr16(0x064C + x, rng_next(DP + 0x10, 0));
    const uint16_t r = rng_next(DP + 0x10, 0);
    wr16(0x0688 + x, r);
    wr16(DP + 0x88, r);
    y = (uint16_t)(y + 0x000F);
  }
}

static uint16_t adc16(uint16_t a, uint16_t b, uint32_t* carry) {
  const uint32_t s = (uint32_t)a + b + *carry;
  *carry = s >> 16;
  return (uint16_t)s;
}

// TG3000_ROUTINE($8A:898F) screen_objects_place
uint16_t screen_objects_place(void) {
  if(rd16(DP + 0x5E) != 0) return rd16(DP + 0x5E);  // never run
  const uint16_t x0 = rd16(DP + 0x3C);
  wr16(DP + 0x3C, (uint16_t)((x0 >> 2) - 0x0100));
  if(!((uint16_t)(rd16(DP + 0x3C) - 0x0120) & 0x8000)) {  // never run
    wr16(DP + 0x3C, x0);
    wr16(DP + 0x5E, (uint16_t)(rd16(DP + 0x5E) + 1));
    objects_scatter_init();
    return (uint16_t)(0xFE84 + 16 * 0x000F);
  }
  uint16_t v = (uint16_t)(rd16(DP + 0x3C) + 0x0030);
  wr16(0x0602, v);
  wr16(0x0606, v);
  v = (uint16_t)(v + 0x0010);
  wr16(0x0604, v);
  wr16(0x0608, v);
  const uint16_t y0 = rd16(DP + 0x3E);
  wr16(DP + 0x3E, (uint16_t)(y0 >> 2));
  v = (uint16_t)(rd16(DP + 0x3E) + 0x0008);
  wr16(0x0702, v);
  wr16(0x0704, v);
  v = (uint16_t)(v + 0x0010);
  wr16(0x0706, v);
  wr16(0x0708, v);
  for(uint16_t x = 0, n = 6; n != 0; n--, x = (uint16_t)(x + 24)) {
    table_062a_fill(x);
    wr16(DP + 0x3E, (uint16_t)(rd16(DP + 0x3E) + 0x0010));
  }
  for(uint32_t x = 0, y = 0x48; y != 0; x += 2, y--) {
    uint16_t kind = rd16(0x8152 + x);  // bank $7E
    if(kind == 4) continue;
    if(y < 0x32) kind |= 0x3000;
    wr16(0x082A + x, kind);
    wr16(0x052A + x, 1);
  }
  wr16(DP + 0x3E, y0);
  wr16(DP + 0x3C, x0);
  uint32_t c = 0;
  v = rd16(0x072A);
  for(int col = 0; col < 3; col++) {
    v = adc16(v, col == 0 ? 0x0010 : 0x0008, &c);
    for(uint32_t k = 0; k < 8; k += 2) wr16(0x0712 + 8 * (uint32_t)col + k, v);
  }
  v = rd16(0x062A);
  for(int row = 0; row < 4; row++) {
    v = adc16(v, row == 0 ? 0x0050 : 0x0008, &c);
    for(uint32_t k = 0; k < 0x18; k += 8) wr16(0x0612 + 2 * (uint32_t)row + k, v);
  }
  return v;
}

#define SCENE(o) (DP + (o))
static void scene_inc(uint32_t a) { wr16(a, (uint16_t)(rd16(a) + 1)); }
static bool scene_dec(uint32_t a) {  // DEC: true when it reaches 0
  wr16(a, (uint16_t)(rd16(a) - 1));
  return rd16(a) == 0;
}

// $8A:83BA: the 12-word row at $7E:BDF2 + D+$78 into $0812, $0512 = $FFFF
static void scene_row_copy(void) {
  for(uint32_t y = 0, x = rd16(SCENE(0x78)); y != 0x18; y += 2, x += 2) {
    wr16(0x0812 + y, rd16(0xBDF2 + (uint32_t)(uint16_t)x));
    wr16(0x0512 + y, 0xFFFF);
  }
}

// $8A:8524: the 16 objects' three bytes stepped by D+$8A, D+$8A - 1, D+$8A - 2
static void scene_objects_jitter(void) {
  for(int32_t x = 0x1E; x >= 0; x -= 2) {
    const uint16_t j = rd16(SCENE(0x8A));
    wr16(0x0610 + (uint32_t)x, (uint16_t)((rd16(0x0610 + (uint32_t)x) + j) & 0x00FF));
    wr16(0x064C + (uint32_t)x, (uint16_t)((rd16(0x064C + (uint32_t)x) + (uint16_t)(j - 1)) & 0x00FF));
    wr16(0x0688 + (uint32_t)x, (uint16_t)((rd16(0x0688 + (uint32_t)x) + (uint16_t)(j - 2)) & 0x00FF));
  }
}

// $8A:855A: D+$7E = by; 112 words from $0710 moved by it
static void scene_rows_move(uint16_t by) {
  wr16(SCENE(0x7E), by);
  for(int32_t x = 0xDE; x >= 0; x -= 2) wr16(0x0710 + (uint32_t)x, (uint16_t)(rd16(0x0710 + (uint32_t)x) + by));
}

static void scene_slide(void) {
  wr16(SCENE(0x54), 0);
  wr16(SCENE(0x3C), (uint16_t)(rd16(SCENE(0x3C)) + rd16(SCENE(0x56))));
  wr16(SCENE(0x36), (uint16_t)(rd16(SCENE(0x36)) - rd16(SCENE(0x56))));
}

// TG3000_ROUTINE($8A:82C0) scene_step
void scene_step(void) {
  if(rd16(SCENE(0x48)) != 0) {
    wr16(SCENE(0x48), (uint16_t)(rd16(SCENE(0x48)) - 1));
    return;
  }
  switch(rd16(SCENE(0x44))) {
    case 0x82CA:
      scene_slide();
      if(scene_dec(SCENE(0x46))) {
        wr16(SCENE(0x46), 0x0040);
        wr16(SCENE(0x44), 0x82EC);
      }
      (void)func_8A8325();
      break;
    case 0x82EC:
      scene_slide();
      scene_inc(SCENE(0x60));
      if(!(rd16(SCENE(0x60)) & 1) && rd16(SCENE(0x56)) != 0) wr16(SCENE(0x56), (uint16_t)(rd16(SCENE(0x56)) - 1));
      (void)func_8A8325();
      if(scene_dec(SCENE(0x46))) {
        wr16(SCENE(0x48), 0x0008);
        wr16(SCENE(0x46), 0x0036);
        wr16(SCENE(0x44), 0x833D);
      }
      break;
    case 0x8322:  // never run, from here on
      scene_inc(SCENE(0x58));
      break;
    case 0x833D:
      scene_inc(0x0600);
      if(rd16(0x0600) == 0x006C) wr16(SCENE(0x42), 1);
      if(scene_dec(SCENE(0x46))) {
        wr16(SCENE(0x48), 0x0008);
        wr16(SCENE(0x46), 0);
        wr16(SCENE(0x44), 0x8361);
      }
      break;
    case 0x8361: {
      wr16(SCENE(0x48), 0x0006);
      uint16_t x = rd16(SCENE(0x46));
      wr16(0x0800, rd16(0x8096 + (uint32_t)x));  // bank $7E
      x = (uint16_t)(x + 2);
      if(x == 0x0024) {
        wr16(0x0500, 0);
        wr16(SCENE(0x48), 0x0020);
        wr16(SCENE(0x44), 0x8393);
        wr16(SCENE(0x46), 0x000E);
        wr16(SCENE(0x5A), 1);
        scene_inc(SCENE(0x5C));
      } else wr16(SCENE(0x46), x);
      break;
    }
    case 0x8393:
      scene_inc(SCENE(0x4E));
      if(rd16(SCENE(0x4E)) & 0x0007) break;
      scene_inc(SCENE(0x5C));
      if(scene_dec(SCENE(0x46))) {
        wr16(SCENE(0x44), 0x83D6);
        wr16(SCENE(0x46), 0x005A);
        wr16(SCENE(0x56), 0);
        wr16(SCENE(0x5A), 2);
        wr16(SCENE(0x78), 0);
        scene_row_copy();
      }
      break;
    case 0x83D6: {
      wr16(SCENE(0x38), (uint16_t)(rd16(SCENE(0x38)) + rd16(SCENE(0x56))));
      scene_inc(SCENE(0x60));
      if(!(rd16(SCENE(0x60)) & 0x000F)) scene_inc(SCENE(0x56));
      if(scene_dec(SCENE(0x46))) {
        wr16(SCENE(0x44), 0x8412);
        wr16(SCENE(0x46), 0x0010);
        scene_inc(SCENE(0x54));
      }
      const uint16_t a = (uint16_t)(rd16(SCENE(0x4C)) + 0x0049);  // - $FFB7
      if(!((uint16_t)(a - 0x0015) & 0x8000)) break;
      wr16(SCENE(0x4E), (uint16_t)(a << 3));
      wr16(SCENE(0x78), (uint16_t)((uint16_t)(a << 4) + rd16(SCENE(0x4E))));
      scene_row_copy();
      break;
    }
    case 0x8412:
      scene_inc(0x0160);
      if(rd16(0x0160) & 0x0003) break;
      scene_inc(SCENE(0x74));
      wr16(SCENE(0x76), (uint16_t)(rd16(SCENE(0x76)) - 1));
      if(scene_dec(SCENE(0x46))) {
        wr16(SCENE(0x44), 0x843D);
        wr16(SCENE(0x46), 0x0370);
        wr16(SCENE(0x48), 0x0006);
        wr16(SCENE(0x56), 0);
        wr16(0x050A, 0);
        wr16(0x050C, 0);
      }
      break;
    case 0x843D:
      scene_objects_jitter();
      wr16(SCENE(0x42), 2);
      wr16(SCENE(0x3C), (uint16_t)(rd16(SCENE(0x3C)) + rd16(SCENE(0x56)) + rd16(SCENE(0x56))));
      scene_inc(SCENE(0x60));
      if(!(rd16(SCENE(0x60)) & 1)) {
        uint16_t a = rd16(SCENE(0x56));
        if(a == 0x0063) {
          wr16(SCENE(0x5A), 3);
          scene_inc(SCENE(0x56));
          a = 3;  // the LDA #3 is what the next CMP sees
        }
        if(!((uint16_t)(a - 0x0064) & 0x8000)) {
          wr16(SCENE(0x62), (uint16_t)(rd16(SCENE(0x62)) - 1));
          if(!((uint16_t)(rd16(SCENE(0x34)) - 0x0100) & 0x8000)) wr16(SCENE(0x62), (uint16_t)(rd16(SCENE(0x62)) - 1));
          scene_rows_move(1);
          const uint16_t w = (uint16_t)(rd16(0x2C01) - 4);  // bank $7E
          wr16(0x2C01, w);
          wr16(0x2C04, (uint16_t)(w + 0x0200));
          if((uint16_t)(rd16(SCENE(0x34)) - 0x01E0) & 0x8000) wr16(SCENE(0x34), (uint16_t)(rd16(SCENE(0x34)) + 8));
        }
        if(rd16(SCENE(0x56)) != 0x00AA) scene_inc(SCENE(0x56));
      }
      if(scene_dec(SCENE(0x46))) {
        wr16(SCENE(0x46), 0x0046);
        wr16(SCENE(0x44), 0x856D);
      }
      break;
    case 0x856D: {
      scene_objects_jitter();
      const uint16_t a = rd16(SCENE(0x34));
      if(a == 0 || (a & 0x8000)) {
        wr16(SCENE(0x34), 0);
        wr16(SCENE(0x42), 1);
      } else {
        wr16(SCENE(0x38), (uint16_t)(rd16(SCENE(0x38)) + 2));
        wr16(SCENE(0x36), (uint16_t)(rd16(SCENE(0x36)) + 2));
        scene_rows_move(1);
      }
      if(scene_dec(SCENE(0x46))) wr16(SCENE(0x44), 0x8322);
      else if(rd16(SCENE(0x46)) == 0x0032) {
        wr16(SCENE(0x98), 0);
        wr16(SCENE(0x96), 0xFFC0);
      }
      break;
    }
    default:  // no state the game sets
      break;
  }
}

// TG3000_ROUTINE($80:B7E7) split_screen_setup
void split_screen_setup(uint16_t ret, uint8_t db) {
  for(uint32_t i = 0; i < 0x2C; i++) {
    wr8(0x0196 + i, bus_rd8(0x84A2EF + i));
    wr8(0x0296 + i, bus_rd8(0x84A2EF + i));
  }
  static const uint16_t cleared[] = {0x01C2, 0x02C2, 0x0164, 0x0264, 0x0196, 0x0296, 0x019A, 0x029A, 0x0176, 0x0276};
  for(unsigned i = 0; i < sizeof cleared / sizeof cleared[0]; i++) wr16(cleared[i], 0);
  const uint16_t mode = rd16(0x03DC);
  if(!(mode & 0x000F) || (mode & 0x0030) == 0x0010) wr16(0x0276, (uint16_t)(rd16(0x0276) | 0x0020));  // the $10 test: never run
  static const uint16_t words[][2] = {
    {0x01C4, 0x0500}, {0x02C4, 0x0700}, {0x015C, 0}, {0x025C, 0x4000}, {0x015E, 0}, {0x025E, 0x8000},
    {0x0170, 0}, {0x0172, 0x0400}, {0x0174, 0x0400}, {0x016E, 0x0900}, {0x016C, 0x0D00}, {0x0270, 0x0800},
    {0x0272, 0x0C00}, {0x0274, 0x0C00}, {0x026E, 0x1100}, {0x026C, 0x1500}, {0x01E7, 0}, {0x01E8, 0},
    {0x02E7, 0x7400}, {0x02E8, 0x0074}, {0x018A, 0}, {0x018C, 0}, {0x018E, 0}, {0x028A, 0x0100},
    {0x028C, 0x0100}, {0x028E, 0x0100}, {0x01EC, 0xB94F}, {0x02EC, 0xB94F}, {0x0192, 0}, {0x0292, 0x001B},
    {0x0194, 0}, {0x0294, 0x002F}};
  for(unsigned i = 0; i < sizeof words / sizeof words[0]; i++) wr16(words[i][0], words[i][1]);
  for(uint32_t x = 0x0200; x != 0; x -= 2) {
    wr16(0x08FE + x, 0xE0E0);
    wr16(0x0CFE + x, 0xE0E0);
    wr16(0x10FE + x, 0xE0E0);
    wr16(0x14FE + x, 0xE0E0);
    wr16(0x0AFE + x, 0);
    wr16(0x0EFE + x, 0);
    wr16(0x12FE + x, 0);
    wr16(0x16FE + x, 0);
  }
  (void)wait_frames(1);
  wr16(0x0278, 0x1D8C);
  wr16(0x1D8D, 0x0200);
  wr16(0x1D8F, 0);
  wr16(0x1D91, 0);
  wr16(0x1D93, 0);
  wr8(0x1D95, db);
  wr8(0x1D96, 0);
  wr16(0x1D97, 0xB94E);
  wr8(0x1D99, 0x80);
  wr16(0x1D9A, ret);
  wr16(0x0008, 0xB97B);
  platform_set_irq_v(0x006C);
  platform_set_irq_h(0x0010);
  platform_set_interrupts(0xB1);
  wr16(0x0005, 0xBC9B);  // then CLI
}

// TG3000_ROUTINE($8A:891C) title_scene_load
void title_scene_load(void) {
  (void)decompress_to_vram(DP, 0x7E, 0x8256, 0x2000);
  (void)decompress_to_vram(DP, 0x7E, 0xB385, 0x2A00);
  (void)decompress_to_vram(DP, 0x7E, 0x9F07, 0x3000);
  wr16(DP + 0x14, 0x007E);
  upload_vram_now(0x3600, 0x7E, 0xA071, 0x0800);
  wr16(DP + 0x5A, 0);
  wr16(DP + 0x3C, 0);
  wr16(DP + 0x3E, 0x0200);
  (void)screen_objects_place();
  wr16(DP + 0x36, 3);
  wr16(DP + 0x4A, 0);  // 3 / 4
  wr16(DP + 0x38, 0);
  wr16(DP + 0x4C, 0);
  wr16(0x0600, 0xFF36);
  wr16(0x0700, 0x009B);
  wr16(0x0800, 0x0100);
  wr16(0x0500, 1);
}

// TG3000_ROUTINE($8A:8768) track_preview_maybe
bool track_preview_maybe(void) {
  const uint16_t track = rd16(0x1536);
  if(!(bus_rd16(0x8A9351 + (uint32_t)(uint16_t)(track << 3)) & 0x0002)) return false;
  track_tiles_preview(track);
  return true;
}

void hud_markers_put(void) {
  for(uint16_t i = 0; i < 4; i++)
    if(rd8(0x0398 + i) == 0) video_write_vram_word((uint16_t)(0x7C57 + i), 0x0157);
  if(rd8(0x03A7) != 0)
    for(uint16_t i = 0; i < 4; i++) video_write_vram_word((uint16_t)(0x7F45 + i), bus_rd16(0x83A4FC + 2 * (uint32_t)i));
}

// TG3000_ROUTINE($82:8961) hud_markers_write
void hud_markers_write(void) {
  hud_markers_put();
  race_hud_digits();
}

// D = dp ($0200); DB = $30 inside (chip at $8000), $83 on return; all DSP-4 use is command 0

static uint16_t xba16b(uint16_t v) { return (uint16_t)(v << 8 | v >> 8); }
static void mul32(uint16_t a, uint16_t b, uint16_t* lo, uint16_t* hi) {
  const uint32_t p = (uint32_t)dsp4_multiply((int16_t)a, (int16_t)b);
  *lo = (uint16_t)p;
  *hi = (uint16_t)(p >> 16);
}
// (hi:lo at dp+o) += (h:l), 32-bit
static void add32(uint32_t lo_at, uint32_t hi_at, uint16_t l, uint16_t h) {
  const uint32_t s = (uint32_t)rd16(lo_at) + l;
  wr16(lo_at, (uint16_t)s);
  wr16(hi_at, (uint16_t)(rd16(hi_at) + h + (s >> 16)));
}

// $83:F6E1 (never run): ($011E:$011C) * $1E9E -> $0114/$0116/$0118
static void car_step_product(void) {
  uint16_t lo, hi;
  mul32(rd16(0x011C), rd16(0x1E9E), &lo, &hi);
  wr16(0x0118, 0);
  wr16(0x0114, lo);
  wr16(0x0116, hi);
  if(hi & 0x8000) wr16(0x0118, 0xFFFF);
  uint16_t a = rd16(0x011E);
  if(rd16(0x011C) & 0x8000) a++;
  mul32(a, rd16(0x1E9E), &lo, &hi);
  const uint32_t s = (uint32_t)rd16(0x0116) + lo;
  wr16(0x0116, (uint16_t)s);
  wr16(0x0118, (uint16_t)(rd16(0x0118) + hi + (s >> 16)));
}

// $83:F590 / $83:F5EA: D+$4C counts the frames down; at 0, with no sound request queued
// ($0463), D+$4C = $10 and func_83FE9A(message)
static void scene_timer_ping(uint16_t dp, uint16_t message) {
  const uint32_t at = (uint32_t)dp + 0x4C;
  const uint16_t t = (uint16_t)(rd16(at) - rd16(0x1E9E));
  wr16(at, t);
  if(!(t & 0x8000)) return;
  wr16(at, 0);
  if(rd16(0x0463) & 0x00FF) return;
  wr16(at, 0x0010);
  (void)func_83FE9A(dp, message, rd16(0x1EBE), 0);
}

// TG3000_ROUTINE($83:EAC6) player_car_frame
void player_car_frame(uint16_t dp) {
  #define P(o) ((uint32_t)dp + (o))
  uint16_t lo, hi;
  const uint16_t steps = rd16(0x1E9E);
  if(rd16(0x1ECE) != 0) {  // following: only the view's distance
    if(rd16(0x1ECE) & 0x8000) follow_car_step(dp);
    if(rd16(0x1E9C) & 0x0001) {
      mul32(0x0060, xba16b(steps), &lo, &hi);
      add32(P(0x12), P(0x14), lo, hi);
      if(rd16(P(0x14)) >= 0x0009) {
        wr16(P(0x14), 0x0009);
        wr16(P(0x12), 0xFFFF);
      }
    }
    mul32(0xFFE0, xba16b(steps), &lo, &hi);
    add32(P(0x12), P(0x14), lo, hi);
    if(rd16(P(0x14)) & 0x8000) {
      wr16(P(0x12), 0);
      wr16(P(0x14), 0);
    }
    return;
  }
  wr16(0x013A, 0);
  wr16(0x013C, 0);
  if((rd16(0x1E9C) & 0x0010) && rd16(P(0x31)) == 0) {
    if(rd8(P(0x4E)) != 0) {
      wr16(P(0x4E), (uint16_t)(rd16(P(0x4E)) - 1));
      wr16(P(0x31), rd16(0x1EB6));
      (void)func_83FE9A(dp, 0xFF1B, rd16(0x1EBE), 0x003C);
    } else (void)func_83FE9A(dp, 0xFF1C, rd16(0x1EBE), 0);
  }
  {
    const uint16_t a = (uint16_t)(rd16(P(0x31)) - rd16(0x1E9E));
    wr16(P(0x31), (a & 0x8000) ? 0 : a);
  }
  if(rd16(0x1E9C) & 0x0080) {
    wr16(0x1E9C, (uint16_t)(rd16(0x1E9C) & ~0x0080));
    if(rd16(P(0x1A)) == 0 && rd8(P(0x51)) != 0) {
      wr16(P(0x51), (uint16_t)(rd16(P(0x51)) - 1));
      if(!(rd16(P(0x44)) & 0x0020)) {
        wr16(P(0x44), (uint16_t)(rd16(P(0x44)) | 0x0120));
        wr16(P(0x1A), 5);
        (void)func_83FE9A(dp, 0xFF29, rd16(0x1EBE), 0);
        const uint32_t car = rd16(0x1EBA);
        wr16(0xFADE + car, (uint16_t)(rd16(0xFADE + car) | 0x0008));
        wr16(P(0x5E), 0x0020);
        wr16(P(0x70), rd16(P(0x10)));
        wr16(P(0x72), rd16(P(0x10)));
      }
    }
  }
  if(rd16(0x1E9C) & 0x0020) {
    wr16(0x1E9C, (uint16_t)(rd16(0x1E9C) & ~0x0020));
    if(rd16(P(0x1A)) == 0) {  // (its AND #$7FFF / DEC / CMP #5 tests see A = 0)
      wr16(P(0x7E), 0);
      wr16(P(0x1A), 1);
      (void)func_83FE9A(dp, 0xFF1D, rd16(0x1EBE), 0);
    }
  }
  if(!(rd16(0x1F31) & 0x8000) && !(rd16(P(0x7E)) & 0x8000)) {
    const uint16_t x = rd16(P(0x7E)), a = (uint16_t)(x + rd16(0x1E9E));
    wr16(P(0x7E), a);
    if((x < 0x0054 && a >= 0x0054) || (x >= 0x0054 && x < 0x0058 && a >= 0x0058))
      (void)func_83FE9A(dp, 0x7F21, rd16(0x1EBE), 0);
  }
  if((rd16(0x1E9C) & 0x0040) && (rd16(P(0x50)) & 0x007F) && !(rd16(P(0x81)) & 0x8000)) {  // (EC33)
    const uint8_t s = (uint8_t)(rd8(0x1E9E) << 3);
    const uint16_t t = (uint16_t)(rd8(P(0x80)) + (uint8_t)~s + 1);
    wr8(P(0x80), (uint8_t)t);
    wr8(P(0x50), (uint8_t)(rd8(P(0x50)) - ((t >> 8) ? 0 : 1)));
    uint16_t x = rd16(P(0x81));
    bool record = x == 0;  // the player's record (ECAA) with X = 0
    if(!record && (rd16(0x03DC) & 0x000F) && x == 0x0035) {  // never run
      x = 0x00B3;
      record = true;
    }
    if(!record) {  // a car's (EC68)
      const uint32_t c = x;
      wr16(0x011C, rd16(0xFAC9 + c));
      wr16(0x011E, rd16(0xFACB + c));
      car_step_product();
      const uint32_t s1 = (uint32_t)rd16(0xFAC9 + c) - rd16(0x0115);
      wr16(0xFAC9 + c, (uint16_t)s1);
      const uint16_t h = (uint16_t)(rd16(0xFACB + c) - rd16(0x0117) - ((s1 >> 16) ? 1 : 0));
      wr16(0xFACB + c, h);
      if(h & 0x8000) {
        wr16(0xFAC9 + c, 0);
        wr16(0xFACB + c, 0);
      }
      wr16(0x013C, (uint16_t)(rd16(0x013C) - rd16(0x0116)));
    } else {  // never run (ECAA)
      const uint32_t r = x;
      wr16(0x011C, rd16(0x020E + r));
      wr16(0x011E, rd16(0x0210 + r));
      car_step_product();
      mul32(rd16(0x0116), rd16(0x0295 + r), &lo, &hi);
      const uint32_t s1 = (uint32_t)rd16(0x0216 + r) - lo;
      wr16(0x0216 + r, (uint16_t)s1);
      uint16_t h = (uint16_t)(rd16(0x0218 + r) - hi - ((s1 >> 16) ? 1 : 0));
      if(h & 0x8000) {
        wr16(0x0216, 0);  // the original's STZ is not indexed
        h = 0;
      }
      wr16(0x0218 + r, h);
      wr16(0x013C, (uint16_t)(rd16(0x013C) - rd16(0x0117)));
    }
  }
  // ECF6
  mul32(rd16(P(0x16)), rd16(0x1E9E), &lo, &hi);
  wr16(0x0136, lo);
  wr16(0x0138, hi);
  {
    uint16_t a = rd16(P(0x18));
    if(rd16(P(0x16)) & 0x8000) a++;
    mul32(a, rd16(0x1E9E), &lo, &hi);
    wr16(0x0138, (uint16_t)(rd16(0x0138) + lo));
  }
  if(!(rd16(0x1F31) & 0x8000) && rd16(P(0x58)) != 0) {
    mul32(rd16(P(0x17)), 0x3000, &lo, &hi);
    mul32(rd16(0x1E9E), hi, &lo, &hi);  // the answer's second word is skipped by a write
    const uint32_t s1 = (uint32_t)rd16(P(0x56)) - lo;
    wr16(P(0x56), (uint16_t)s1);
    const uint16_t h = (uint16_t)(rd16(P(0x58)) - ((s1 >> 16) ? 1 : 0));
    wr16(P(0x58), (h & 0x8000) ? 0 : h);
  }
  wr16(0x0134, rd16(0x1E9E));
  do {  // ED8A: one step per frame counted in $1E9E
    if(rd16(0x1E9C) & 0x0001) {
      uint16_t a = rd16(P(0x48)) ? 0x2400 : 0;
      if(rd16(P(0x31))) a = (uint16_t)(a + rd16(0x1EB4));
      a = (uint16_t)(a + rd16(0x1EA8));
      if(!(rd16(P(0x44)) & 0x0010)) a = (uint16_t)(a >> 1);
      mul32(a, rd16(0x1EA6), &lo, &hi);
      wr16(0x0114, (uint16_t)((lo >> 8) & 0x00FF));
      uint16_t h = hi;
      if(h & 0xFF80) h = 0x007F;
      const uint16_t v = (uint16_t)(((h << 8) & 0xFF00) | rd16(0x0114));
      mul32(v, 1, &lo, &hi);
      add32(P(0x16), P(0x18), lo, hi);
    }
    // EDF9
    const uint16_t d = (uint16_t)(rd16(0x1EA4) - rd16(0x1EC0));
    mul32(rd16(P(0x17)), rd16(P(0x17)), &lo, &hi);
    const uint16_t sq_lo = lo;
    uint16_t sq_hi = hi;
    if(sq_lo & 0x8000) sq_hi++;
    mul32(sq_lo, d, &lo, &hi);
    wr16(0x0114, lo);
    wr16(0x0116, hi);
    wr16(0x0118, (hi & 0x8000) ? 0xFFFF : 0);
    mul32(sq_hi, d, &lo, &hi);
    add32(0x0116, 0x0118, lo, hi);
    wr16(0x011A, 0);
    {
      const uint32_t s1 = (uint32_t)rd16(P(0x16)) - rd16(0x0117);
      wr16(P(0x16), (uint16_t)s1);
      wr16(P(0x18), (uint16_t)(rd16(P(0x18)) - rd16(0x0119) - ((s1 >> 16) ? 1 : 0)));
    }
    mul32(d, 1, &lo, &hi);
    {
      const uint32_t s1 = (uint32_t)rd16(P(0x16)) - lo;
      wr16(P(0x16), (uint16_t)s1);
      const uint16_t h = (uint16_t)(rd16(P(0x18)) - hi - ((s1 >> 16) ? 1 : 0));
      wr16(P(0x18), h);
      if(h & 0x8000) {
        wr16(P(0x16), 0);
        wr16(P(0x18), 0);
      }
    }
    wr16(0x0134, (uint16_t)(rd16(0x0134) - 1));
  } while(rd16(0x0134) != 0);
  // EEA0
  if(!(rd16(P(0x44)) & 0x0010)) {
    const uint16_t x = (rd16(0x1E9C) & 0x0001) ? 0xFFF0 : 0xFFC0;
    mul32(x, xba16b(rd16(0x1E9E)), &lo, &hi);
    add32(P(0x12), P(0x14), lo, hi);
    uint16_t a = rd16(P(0x14));
    if(a & 0x8000) {
      wr16(P(0x14), 0);
      wr16(P(0x12), 0);
      a = 0;
    }
    if(a <= rd16(P(0x18))) {
      wr16(P(0x44), (uint16_t)(rd16(P(0x44)) | 0x0010));
      wr16(P(0x12), rd16(P(0x16)));
      wr16(P(0x14), rd16(P(0x18)));
    }
  } else {
    wr16(P(0x12), rd16(P(0x16)));
    wr16(P(0x14), rd16(P(0x18)));
  }
  // EEEF
  mul32(rd16(P(0x17)), (uint16_t)(rd16(0x1EA0) >> 1), &lo, &hi);
  wr16(P(0x0E), (uint16_t)(lo << 1));
  wr16(P(0x10), (uint16_t)(hi << 1 | lo >> 15));
  {
    const uint16_t a = rd16(P(0x17));
    bool step = true;
    if(a >= rd16(0x1EAA)) wr16(P(0x1F), (uint16_t)(rd16(P(0x1F)) + 1));
    else if(a >= rd16(0x1EAC)) step = false;  // -> EF6E
    else wr16(P(0x1F), (uint16_t)(rd16(P(0x1F)) - 1));
    if(step) {
      const uint32_t x = (uint32_t)((rd16(0x1EB2) & 0x007F) << 1);
      wr16(P(0x2B), func_83EA9B((uint16_t)((uint16_t)(rd16(P(0x1F)) << 1) + bus_rd16(0x86DAD9 + x))));
      if(!(rd16(0x1F31) & 0x8000)) wr16(P(0x95), rd16(0x1EA2));
      // EF3F: (D+$10:D+$0E) * 2 divided by $1EA0, bit by bit (40 steps) -> D+$16/D+$18
      uint16_t q0 = (uint16_t)(rd16(P(0x0E)) << 1), q1 = (uint16_t)(rd16(P(0x10)) << 1 | rd16(P(0x0E)) >> 15);
      uint16_t rem = (uint16_t)(rd16(P(0x10)) >> 15);
      const uint16_t div = rd16(0x1EA0);
      for(int n = 0; n < 0x28; n++) {
        bool c = rem >= div;
        if(c) rem = (uint16_t)(rem - div);
        const bool c0 = (q0 >> 15) != 0;
        q0 = (uint16_t)(q0 << 1 | (c ? 1 : 0));
        const bool c1 = (q1 >> 15) != 0;
        q1 = (uint16_t)(q1 << 1 | (c0 ? 1 : 0));
        rem = (uint16_t)(rem << 1 | (c1 ? 1 : 0));
      }
      wr16(0x0114, q0);
      wr16(0x0116, q1);
      wr16(P(0x16), q0);
      wr16(P(0x18), q1);
    }
  }
  // EF6E
  {
    mul32(rd16(P(0x10)), rd16(P(0x10)), &lo, &hi);
    wr16(0x0114, (uint16_t)(lo >> 8));
    uint16_t x = 0x7FFF;
    if(hi < 0x0100) x = (uint16_t)(((hi & 0x00FF) << 8) | rd16(0x0114));
    mul32(0, x, &lo, &hi);  // (the original multiplies by 0 here)
    add32(0x013A, 0x013C, lo, hi);
  }
  if(rd16(P(0x10)) >= 0x0010) {
    mul32(rd16(P(0x37)), rd16(0x1E9E), &lo, &hi);
    wr16(0x013C, (uint16_t)(rd16(0x013C) + lo));
  }
  if(!(rd16(0x1F31) & 0x8000) && rd16(P(0x58)) == 0 && rd16(P(0x10)) >= 0x0010) {
    mul32(rd16(0x1E9E), 0x0080, &lo, &hi);
    wr16(0x013C, (uint16_t)(rd16(0x013C) + lo));
  }
  if(!(rd16(0x1F31) & 0x8000)) {  // F004: the road's slope at the player's row
    const uint32_t x = rd16(0x1EBC);
    uint16_t a = 0;
    if(!(rd16(0x3D6E + x) & 0x0008)) {
      mul32((uint16_t)((rd16(0x3DEE + x) - rd16(0x3DEC + x)) << 1), (uint16_t)((rd16(0x0224) & 0xFF00) >> 1), &lo, &hi);
      a = (uint16_t)-(uint16_t)(rd16(0x3DEC + x) + hi);  // a write takes the answer's first word
    }
    wr16(P(0x9E), a);
    wr16(P(0x9C), 0);
    if(rd16(0x1F2C) & 0x8001) wr16(P(0x9C), rd16(P(0x9E)));
  }
  {  // F05B
    const uint32_t x = rd16(0x1EBC);
    const uint32_t y = rd16(0x3BEC + x) ? 2 : 0;
    uint16_t a = rd16(P(0x06));
    if(a & 0x8000) a = (uint16_t)-a;
    bool skip = a < rd16(0x1F28 + y);
    if(!skip && !(rd16(0x1F31) & 0x8000)) {
      uint16_t b = (uint16_t)(rd16(P(0x06)) - rd16(P(0x9E)));
      if(b & 0x8000) b = (uint16_t)-b;
      if(b < rd16(0x1F28 + y)) skip = true;
      else wr16(P(0x7C), (uint16_t)(rd16(P(0x7C)) & ~0x0010));
    }
    if(!skip && rd16(P(0x10)) >= 0x0010) {
      mul32(rd16(0x1E9E), 0x0080, &lo, &hi);
      wr16(0x013C, (uint16_t)(rd16(0x013C) + lo));
    }
  }
  if(!(rd16(0x1F31) & 0x8000) && rd16(P(0x10)) >= 0x0010) {  // F0C5
    mul32(rd16(0x1E9E), (uint16_t)(rd8(P(0x5B)) >> 2), &lo, &hi);
    wr16(0x013C, (uint16_t)(rd16(0x013C) + lo));
  }
  if(rd16(0x1E9C) & 0x0002) {  // F0F1
    mul32(rd16(0x1E9E), 0x0100, &lo, &hi);
    wr16(0x013C, (uint16_t)(rd16(0x013C) + lo));
  }
  {  // F115: the drag
    mul32(rd16(0x013C), rd16(0x1EA2), &lo, &hi);
    const uint32_t s1 = (uint32_t)rd16(P(0x16)) - lo;
    wr16(P(0x16), (uint16_t)s1);
    const uint16_t h = (uint16_t)(rd16(P(0x18)) - hi - ((s1 >> 16) ? 1 : 0));
    if(h & 0x8000) {
      wr16(P(0x16), 0);
      wr16(P(0x18), 0);
    } else wr16(P(0x18), h);
  }
  // the second operand of the side steps: D+$0F, or $7FFF from $80 on
  const uint16_t side = rd16(P(0x10)) >= 0x0080 ? 0x7FFF : rd16(P(0x0F));
  if(rd16(P(0x21)) != 0) {  // F13B: the side speed D+$21 eases back to 0
    const uint16_t was = rd16(P(0x21));
    wr16(0x0114, was);
    mul32(rd16(0x1E9E), (was & 0x8000) ? 0x000C : 0xFFF4, &lo, &hi);
    mul32(side, lo, &lo, &hi);
    const uint16_t a = (uint16_t)(rd16(P(0x21)) + hi);
    wr16(P(0x21), a);
    if((a ^ rd16(0x0114)) & 0x8000) wr16(P(0x21), 0);
  }
  wr16(0x013C, 0);  // F186
  if(rd16(0x1E9C) & 0x000C) {  // steering: $18 right ($1E9C bit 3) or $FFE8 left (bit 2)
    const uint16_t by = (rd16(0x1E9C) & 0x0008) ? 0x0018 : 0xFFE8;
    mul32(rd16(0x1E9E), by, &lo, &hi);
    mul32(side, lo, &lo, &hi);
    uint16_t a = (uint16_t)(rd16(P(0x21)) + hi);
    if(!((uint16_t)(a - 0x0080) & 0x8000)) {
      a = 0x0080;
      wr16(0x013C, a);
    }
    if((uint16_t)(a - 0xFF80) & 0x8000) {
      a = 0xFF80;
      wr16(0x013C, a);
    }
    wr16(P(0x21), a);
  }
  if(rd16(P(0x1C)) != 0) {  // F1EC: a push across D+$1C, slowing by $A0 a frame
    mul32(rd16(P(0x1C)), xba16b(rd16(0x1E9E)), &lo, &hi);
    add32(P(0x04), P(0x06), lo, hi);
    mul32(0x00A0, rd16(0x1E9E), &lo, &hi);  // its second word is skipped by a write
    if(rd16(P(0x1C)) & 0x8000) {
      const uint16_t a = (uint16_t)(rd16(P(0x1C)) + lo);
      wr16(P(0x1C), (a & 0x8000) ? a : 0);
    } else {
      const uint16_t a = (uint16_t)(rd16(P(0x1C)) - lo);
      wr16(P(0x1C), (a & 0x8000) ? 0 : a);
    }
  }
  if(!(rd16(P(0x44)) & 0x0021)) {  // F250 (bit 5 or bit 0 set: F35C)
    mul32(rd16(P(0x0E)), rd16(0x1EB8), &lo, &hi);
    wr16(0x0114, lo);
    wr16(0x0116, hi);
    wr16(0x0118, (hi & 0x8000) ? 0xFFFF : 0);
    uint16_t a = rd16(P(0x10));
    if(rd16(P(0x0E)) & 0x8000) a++;
    mul32(a, rd16(0x1EB8), &lo, &hi);
    add32(0x0116, 0x0118, lo, hi);
    a = rd16(P(0x21));
    if(a & 0x8000) {
      a = (uint16_t)(a + rd16(0x0117));
      if(!(a & 0x8000)) a = 0;
    } else {
      a = (uint16_t)(a - rd16(0x0117));
      if(a & 0x8000) a = 0;
    }
    mul32(a, rd16(0x1E9E), &lo, &hi);
    const uint16_t xs = lo;  // F2C0 (its second word read by a BIT)
    mul32((uint16_t)(rd16(P(0x0F)) >> 2), xs, &lo, &hi);
    wr16(0x0114, lo);
    wr16(0x0116, hi);
    add32(P(0x04), P(0x06), rd16(0x0114), rd16(0x0116));
    if(rd16(P(0x1C)) == 0) {  // F2EF: the side's drift
      wr16(0x0118, (uint16_t)(rd16(P(0x28)) & 0xFF00));
      wr16(0x011A, sext8(rd8(P(0x2A))));
      add32(P(0x04), P(0x06), rd16(0x0118), rd16(0x011A));
    }
    bool clear = false;  // F314
    if(rd16(P(0x44)) & 0x0010) {
      if(rd16(P(0x29)) == 0 || !((rd16(0x013C) ^ rd16(P(0x29))) & 0x8000) ||
         ((uint16_t)(rd16(0x0115) + rd16(0x0119)) & 0x8000))
        clear = true;
    }
    if(!clear) {  // F32F
      const uint16_t k = (uint16_t)((rd16(P(0x1A)) & 0x7FFF) - 1);
      if(k == 0 || k == 5) clear = true;
      else {
        const uint16_t t = (uint16_t)(rd16(P(0x46)) - rd16(0x1E9E));
        wr16(P(0x46), t);
        if(t & 0x8000) {
          wr16(P(0x46), (uint16_t)(t + 4));
          (void)func_83FE9A(dp, 0x4F21, rd16(0x1EBE), 0);
        }
      }
    }
    if(clear) wr16(P(0x46), 0);
  }
  // F35C: the side kept within $280 of the road's middle (or of D+$9E)
  if(rd16(0x1F31) & 0x8000) {  // F407 (decoded by hand: $83:F403-$F422 holds data the bench read)
    uint16_t a = rd16(P(0x06));
    if(a & 0x8000) a = (uint16_t)-a;
    if(a >= 0x0280) wr16(P(0x06), (rd16(P(0x06)) & 0x8000) ? 0xFD80 : 0x0280);
  } else {
    uint16_t a = rd16(P(0x06));
    if(a & 0x8000) a = (uint16_t)-a;
    if(a >= 0x0280) {
      uint16_t b = (uint16_t)(rd16(P(0x06)) - rd16(P(0x9E)));
      if(b & 0x8000) b = (uint16_t)-b;
      if(b >= 0x0280) {
        if(!(rd16(0x1F2C) & 0x8001)) wr16(P(0x06), (rd16(P(0x06)) & 0x8000) ? 0xFD80 : 0x0280);
        else if((uint16_t)(rd16(P(0x06)) - rd16(P(0x9E))) & 0x8000) wr16(P(0x06), (uint16_t)(rd16(P(0x9E)) + 0xFD80));
        else wr16(P(0x06), (uint16_t)(rd16(P(0x9E)) + 0x0280));
      }
    }
    // F3B0: by D+$60's byte, the table at $83:F3B9: 6 runs F3C5, 0/2/4/8/10 go on
    if(rd8(P(0x60)) == 6) {
      const uint16_t t = (uint16_t)(rd16(P(0x26)) & 0x003F);
      if(t < 0x0038 && t >= 0x0018) {
        uint16_t h = rd16(P(0x9E));
        const bool neg = (h & 0x8000) != 0;
        h = neg ? (uint16_t)(h >> 1 | 0x8000) : (uint16_t)(h >> 1);
        const bool below = ((uint16_t)(h - rd16(P(0x06))) & 0x8000) != 0;
        // $1F2C negative: kept to the right of half D+$9E's side; else to the left
        const bool take = (rd16(0x1F2C) & 0x8000) ? (neg ? below : !below) : (neg ? !below : below);
        if(take) wr16(P(0x06), h);
      }
    }
  }
  // F423: the engine's state D+$1A (its gear table at $86:DCF7/DD03/DD0F/DD1B) or the side
  // speed's band
  bool band = (rd16(0x03DC) & 0x0003) == 2 || rd16(P(0x1A)) == 0;
  if(!band) {
    if(!(rd16(P(0x1A)) & 0x8000)) {
      wr16(P(0x1A), (uint16_t)(rd16(P(0x1A)) | 0x8000));
      const uint32_t x = (uint16_t)(rd16(P(0x1A)) << 1);
      wr16(P(0x44), (uint16_t)(rd16(P(0x44)) & ~0x0040));
      wr16(P(0x3A), bus_rd16(0x86DCF7 + x));
    } else {
      const uint32_t x = (uint16_t)(rd16(P(0x1A)) << 1);
      if(!(rd16(P(0x44)) & 0x0040)) {
        mul32(rd16(0x1E9E), bus_rd16(0x86DD1B + x), &lo, &hi);
        const uint16_t a = (uint16_t)(rd16(P(0x3A)) + lo);
        wr16(P(0x3A), a);
        if(a >= bus_rd16(0x86DD03 + x)) {
          if(bus_rd16(0x86DD0F + x) & 1) {
            wr16(P(0x44), (uint16_t)(rd16(P(0x44)) | 0x0040));
            wr16(P(0x3A), bus_rd16(0x86DD03 + x));
          } else {
            wr16(P(0x1A), 0);
            band = true;
          }
        }
      }
    }
  }
  if(band) {  // F48E
    const uint16_t a = rd16(P(0x21));
    static const uint16_t steps[] = {0xFFA0, 0xFFC0, 0xFFE8, 0x0018, 0x0040, 0x0060};
    uint16_t x = 6;
    for(unsigned i = 0; i < 6; i++, x--)
      if((uint16_t)(a - steps[i]) & 0x8000) break;
    wr16(P(0x3A), (uint16_t)(x << 8));
  }
  {  // F4BB
    const uint32_t x = (uint32_t)(rd8(P(0x3B)) << 1);
    wr16(0xFABF + (uint32_t)rd16(0x1EBA), bus_rd16(0x86CB54 + x));
    mul32(rd16(0x1E9E), 0x001E, &lo, &hi);  // its second word is skipped by a write
    const uint16_t a = (uint16_t)(rd16(P(0x48)) - lo);
    wr16(P(0x48), (a & 0x8000) ? 0 : a);
  }
  bool items = !(rd16(0x1F31) & 0x8000);  // F4F6: the road items under the car (D+$53, by $83:F54B)
  if(items) {
    const uint16_t k = (uint16_t)((rd16(P(0x1A)) & 0x7FFF) - 1);
    if(k == 0 || k == 5 || rd8(P(0x53)) == 0) items = false;
  }
  if(items) {
    uint16_t a = (uint16_t)(rd8(P(0xB1)) << 4);
    if(a & 0x0800) a |= 0xF000;
    wr16(0x0114, a);
    wr16(0x0116, (uint16_t)(rd8(P(0xB2)) + 0x0020));
    uint16_t d = (uint16_t)(rd16(P(0x06)) - rd16(0x0114));
    if(d & 0x8000) d = (uint16_t)-d;
    if(!((uint16_t)(d - rd16(0x0116)) & 0x8000)) items = false;
  }
  if(!items) wr16(P(0x4C), 0);  // F645
  else switch(rd8(P(0x53))) {
    case 1:  // F555
      car_message_request(dp, 0x003A);
      mul32(0x2000, rd16(0x1E9E), &lo, &hi);
      add32(P(0x56), P(0x58), lo, hi);
      if(rd16(P(0x58)) >= 0x0028) {
        wr16(P(0x58), 0x0028);
        wr16(P(0x56), 0xFFFF);
      }
      scene_timer_ping(dp, 0xFF20);
      break;
    case 2:  // F5B8
      car_message_request(dp, 0x003B);
      if(!(rd16(0x1F31) & 0x8000)) {
        mul32(0x0200, rd16(0x1E9E), &lo, &hi);
        wr16(P(0x5A), rd16(P(0x5A)) >= lo ? (uint16_t)(rd16(P(0x5A)) - lo) : 0);
      }
      scene_timer_ping(dp, 0xFF14);
      break;
    case 3:  // F611
      wr16(P(0x48), 0x0258);
      break;
    case 4:  // F618
      if(!(rd16(P(0x44)) & 0x0020)) {
        wr16(P(0x44), (uint16_t)(rd16(P(0x44)) | 0x0020));
        wr16(P(0x1A), 5);
        const uint32_t car = rd16(0x1EBA);
        wr16(0xFADE + car, (uint16_t)(rd16(0xFADE + car) | 0x0008));
        (void)func_83FE9A(dp, 0xFF29, rd16(0x1EBE), 0);
      }
      break;
    default:  // no item the game places
      break;
  }
  if(!(rd16(0x1F31) & 0x8000)) {  // F647: D+$97 counts up by $800 a frame; its carry plays a sound
    mul32(rd16(0x1E9E), 0x0800, &lo, &hi);
    const uint32_t t = (uint32_t)rd16(P(0x97)) + lo;
    wr16(P(0x97), (uint16_t)t);
    if((t >> 16) && rd16(P(0x58)) != 0 && rd16(P(0x58)) < 0x0009 && !(rd16(P(0x44)) & 0x0004))
      (void)func_83FE9A(dp, 0xFF26, rd16(0x1EBE), 0);
  }
  {  // F68D: D+$1E's byte moves on by the speed, at most $80
    mul32(rd16(P(0x10)), 0x0100, &lo, &hi);
    wr16(0x0114, lo);
    wr16(0x0116, hi);
    mul32(rd16(0x0115), rd16(0x1E9E), &lo, &hi);
    const uint16_t a = lo >= 0x0080 ? 0x0080 : lo;
    wr8(P(0x1E), (uint8_t)(rd8(P(0x1E)) + (uint8_t)a));
  }
  #undef P
}
