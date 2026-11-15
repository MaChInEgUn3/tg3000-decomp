// SPDX-License-Identifier: GPL-3.0-or-later
#include "misc.h"

#include "dsp4.h"
#include "frame.h"
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
