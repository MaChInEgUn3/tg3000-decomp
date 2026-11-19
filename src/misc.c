// SPDX-License-Identifier: GPL-3.0-or-later
#include "misc.h"

#include "decompress.h"
#include "dsp4.h"
#include "frame.h"
#include "pads.h"
#include "rng.h"
#include "platform.h"
#include "sprites.h"
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
