// SPDX-License-Identifier: GPL-3.0-or-later
#include "shop.h"

#include <stdbool.h>

#include "misc.h"
#include "text.h"
#include "tg3000.h"

// TG3000_ROUTINE($80:9F23) shop_part_level_2
uint16_t shop_part_level_2(uint16_t car) { return (uint16_t)(rd16((uint32_t)(0x0392 + car)) + 1); }
// TG3000_ROUTINE($80:9F0A) shop_part_level_3
uint16_t shop_part_level_3(uint16_t car) { return (uint16_t)(rd16((uint32_t)(0x0393 + car)) + 1); }
// TG3000_ROUTINE($80:9F0F) shop_part_level_4
uint16_t shop_part_level_4(uint16_t car) { return (uint16_t)(rd16((uint32_t)(0x0394 + car)) + 1); }
// TG3000_ROUTINE($80:9F14) shop_part_level_5
uint16_t shop_part_level_5(uint16_t car) { return (uint16_t)(rd16((uint32_t)(0x0395 + car)) + 1); }
// TG3000_ROUTINE($80:9F19) shop_part_level_6
uint16_t shop_part_level_6(uint16_t car) { return (uint16_t)(rd16((uint32_t)(0x0396 + car)) + 1); }
// TG3000_ROUTINE($80:9F1E) shop_part_level_7
uint16_t shop_part_level_7(uint16_t car) { return (uint16_t)(rd16((uint32_t)(0x0397 + car)) + 1); }

// TG3000_ROUTINE($80:993E) shop_money_take
void shop_money_take(void) {
  uint16_t car = rd16(DP + 0x94);
  wr16(DP + 0x52, rd16((uint32_t)(0x038E + car)));
  wr16(DP + 0x54, rd16((uint32_t)(0x0390 + car)));
  wr16(DP + 0x56, 0xFFFF);
  wr16(DP + 0x76, (uint16_t)(rd16(DP + 0x76) | 0x0004));
}

// TG3000_ROUTINE($80:E4D0) shop_flag_fb (long)
void shop_flag_fb(void) { wr8(DP + 0xFB, 0x12); }

// TG3000_ROUTINE($80:9DBC) shop_panel_upload
uint16_t shop_panel_upload(void) {
  wr16(DP + 0x14, 0x007F);
  return func_80B537(DP, 0x03C0, rd16(DP + 0x5E), (uint16_t)(rd16(DP + 0x5C) + 0x0400));
}

// The current item's table entry: list $48 (a word index into $81:A892), row $4C or $4A.
static uint16_t shop_item_entry(uint16_t row) {
  return (uint16_t)((row << 4) + rom16(0x81A892 + (uint32_t)(uint16_t)(rd16(DP + 0x48) << 1)));
}

// TG3000_ROUTINE($80:9BF2) shop_item_picture
void shop_item_picture(void) {
  uint16_t entry = shop_item_entry(rd16(DP + 0x4C));
  func_80B58D(0x81, rom16(0x81A89D + (uint32_t)entry));
}

// TG3000_ROUTINE($80:B096) shop_item_text
void shop_item_text(void) {
  uint16_t text = rom16(0x81A895 + (uint32_t)shop_item_entry(rd16(DP + 0x4A)));
  uint16_t size = rom16(0x81A894 + (uint32_t)text), src = rom16(0x81A892 + (uint32_t)text);
  uint16_t dest = (uint16_t)(rd16(DP + 0xC4) + 0x0100);
  for(uint32_t i = 0; i < (size ? size : 0x10000u); i++)  // MVN $89 -> $81: size bytes (0: 64 KB)
    bus_wr8(0x810000 | (uint16_t)(dest + i), bus_rd8(0x890000 | (uint16_t)(src + i)));
}

// TG3000_ROUTINE($80:A7E4) shop_sprite_block
uint16_t shop_sprite_block(uint16_t pos, uint16_t slot) {
  const uint32_t s = slot;
  for(uint32_t i = 0; i < 16; i += 4) wr16(0x0902 + s + i, pos);
  uint16_t attr = rd16(DP + 0x7E);
  for(uint32_t i = 0; i < 16; i += 4) wr16(0x0B00 + s + i, attr);
  uint16_t tile = (uint16_t)((attr & 0x00FF) | rd16(DP + 0x16));
  wr16(0x0900 + s, tile);
  wr16(0x0904 + s, (uint16_t)(tile + 0x0800));
  wr16(0x0908 + s, (uint16_t)(tile + 0x1000));
  wr16(0x090C + s, (uint16_t)(tile + 0x1400));
  for(uint32_t i = 0; i < 16; i += 4) wr16(0x0B02 + s + i, 0);
  return (uint16_t)(slot + 0x10);
}

// TG3000_ROUTINE($80:AAF1) shop_bar_cap
uint16_t shop_bar_cap(void) {
  const uint32_t x = rd16(DP + 0x74);
  uint16_t row = (uint16_t)(((0x51 - rd16(DP + 0x7A) + rd16(DP + 0xE8)) & 0xFF) << 8);
  wr16(DP + 0x16, row);
  uint16_t y = (uint16_t)(rd16(DP + 0x80) + rd16(DP + 0x14) - 1);
  for(uint32_t i = 0; i < 16; i += 4) wr16(0x0B00 + x + i, y);
  uint16_t t = (uint16_t)((y & 0x00FF) | row);
  for(uint32_t i = 0; i < 16; i += 4) wr16(0x0900 + x + i, (uint16_t)(t + 0x0800 * (i / 4)));
  uint16_t tile = rd16(DP + 0x7E);
  wr16(0x0902 + x, tile);
  wr16(0x0906 + x, tile);
  wr16(0x090A + x, tile);
  wr16(0x090E + x, (uint16_t)(tile + 1));
  for(uint32_t i = 0; i < 16; i += 4) wr16(0x0B02 + x + i, 0);
  uint16_t next = (uint16_t)(x + 0x10);
  wr16(DP + 0x74, next);
  return next;
}

// TG3000_ROUTINE($80:A9BD) shop_bar_column
uint16_t shop_bar_column(void) {
  const uint32_t y = rd16(DP + 0x74);
  uint16_t row = (uint16_t)((((0x6C - rd16(DP + 0x7A) - rd16(DP + 0x7E)) & 0xFF) << 8) + rd16(DP + 0xE7));
  wr16(DP + 0x82, row);
  uint16_t at = (uint16_t)(rd16(DP + 0x14) + rd16(DP + 0x80));
  for(uint32_t i = 0; i < 12; i += 4) wr16(0x0B00 + y + i, at);
  uint16_t t = (uint16_t)((at & 0x00FF) | row);
  for(uint32_t i = 0; i < 12; i += 4) wr16(0x0900 + y + i, (uint16_t)(t + 0x0800 * (i / 4)));
  uint16_t tile = rd16(DP + 0x84);
  wr16(0x0902 + y, tile);
  wr16(0x0906 + y, (uint16_t)(tile + 1));
  wr16(0x090A + y, (uint16_t)(tile + 1));
  for(uint32_t i = 0; i < 12; i += 4) wr16(0x0B02 + y + i, 0);
  uint16_t next = (uint16_t)(y + 0x0C);
  wr16(DP + 0x74, next);
  return next;
}

// The half stage past $10, divided by 6 into D+$80 (segments x 6, up to `full`) and D+$7E
// (the rest). Returns false when the bar is full (the original returns at once).
static bool shop_bar_split(uint16_t full) {
  wr16(DP + 0x80, 0);
  wr16(DP + 0x7E, 0);
  uint16_t d = (uint16_t)((rd16(DP + 0x3E) >> 1) - 0x10);
  if((rd16(DP + 0x3E) >> 1) < 0x10) return true;
  wr16(DP + 0x7E, d);
  for(;;) {
    if(rd16(DP + 0x7E) < 6) return true;
    wr16(DP + 0x7E, (uint16_t)(rd16(DP + 0x7E) - 6));
    wr16(DP + 0x80, (uint16_t)(rd16(DP + 0x80) + 6));
    if(rd16(DP + 0x80) == full) return false;
  }
}

// One 3-sprite column of a bar at slot `y`: row ((`top` - D+$7A) & $FF) << 8 + D+$E7 into
// D+$82, y position D+$14 + D+$80 + `dy`, tiles from D+$7E * 2 + D+$84 (the middle one
// flipped). Returns the next slot.
static uint16_t shop_bar_piece(uint16_t y, uint16_t top, uint16_t dy) {
  const uint32_t s = y;
  uint16_t row = (uint16_t)((((top - rd16(DP + 0x7A)) & 0xFF) << 8) + rd16(DP + 0xE7));
  wr16(DP + 0x82, row);
  uint16_t at = (uint16_t)(rd16(DP + 0x14) + rd16(DP + 0x80) + dy);
  for(uint32_t i = 0; i < 12; i += 4) wr16(0x0B00 + s + i, at);
  uint16_t t = (uint16_t)((at & 0x00FF) | row);
  for(uint32_t i = 0; i < 12; i += 4) wr16(0x0900 + s + i, (uint16_t)(t + 0x0800 * (i / 4)));
  uint16_t tile = (uint16_t)((rd16(DP + 0x7E) << 1) + rd16(DP + 0x84));
  wr16(0x0902 + s, tile);
  wr16(0x0906 + s, (uint16_t)(tile | 0x8000));
  wr16(0x090A + s, (uint16_t)(((tile | 0x8000) ^ 0x8000) + 1));
  for(uint32_t i = 0; i < 12; i += 4) wr16(0x0B02 + s + i, 0);
  return (uint16_t)(y + 0x0C);
}

// TG3000_ROUTINE($80:A4B8) shop_bar_segments
void shop_bar_segments(void) {
  uint16_t y = rd16(DP + 0x74);
  if(!shop_bar_split(0x1E)) {  // at $1E: back to the half segment $1B, with the rest less 3
    wr16(DP + 0x80, 0x1B);
    uint16_t rest = rd16(DP + 0x7E);
    if(rest >= 3) return;
    wr16(DP + 0x7E, (uint16_t)(rest + 3));
  }
  for(;;) {
    y = shop_bar_piece(y, 0x5B, 0x0005);
    wr16(DP + 0x7E, 0);
    uint16_t at = (uint16_t)(rd16(DP + 0x80) + 6);
    wr16(DP + 0x80, at);
    if(at == 0x1E) {
      wr16(DP + 0x80, 0x1B);
      wr16(DP + 0x7E, 3);
      continue;
    }
    if(at == 0x21) break;
  }
  wr16(DP + 0x74, y);
}

// TG3000_ROUTINE($80:A87D) shop_bar_segments_long
void shop_bar_segments_long(void) {
  uint16_t y = rd16(DP + 0x74);
  if(!shop_bar_split(0x24)) return;
  do {
    y = shop_bar_piece(y, 0x5A, 0x0003);
    wr16(DP + 0x7E, 0);
    wr16(DP + 0x80, (uint16_t)(rd16(DP + 0x80) + 6));
  } while(rd16(DP + 0x80) != 0x24);
  wr16(DP + 0x74, y);
}

// TG3000_ROUTINE($80:A403) shop_bar_levels
void shop_bar_levels(void) {
  uint16_t x = rom16(0x81A896 + (uint32_t)rd16(DP + 0x7C));
  wr16(DP + 0x80, 0);
  do {
    wr16(DP + 0x7E, 0);
    uint16_t level = (uint16_t)((rd16(DP + 0x3E) >> 1) - rom16(0x81A892 + (uint32_t)x));
    if(!(level & 0x8000)) {
      uint16_t length = rom16(0x81A894 + (uint32_t)x);
      wr16(DP + 0x7E, level < length ? level : length);
    }
    const uint32_t y = rd16(DP + 0x74);
    uint16_t row = (uint16_t)((((0x60 - rd16(DP + 0x7A) + rd16(DP + 0x7E)) & 0xFF) << 8) + rd16(DP + 0xE7));
    wr16(DP + 0x7E, row);
    uint16_t at = (uint16_t)(rd16(DP + 0x14) + rd16(DP + 0x80) + 0x2B);
    wr16(0x0B00 + y, at);
    wr16(0x0B04 + y, at);
    uint16_t t = (uint16_t)((at & 0x00FF) | row);
    wr16(0x0900 + y, t);
    wr16(0x0904 + y, (uint16_t)(t + 0x0800));
    wr16(0x0902 + y, 0x012C);
    wr16(0x0906 + y, 0x012C);
    wr16(0x0B02 + y, 0);
    wr16(0x0B06 + y, 0);
    wr16(DP + 0x74, (uint16_t)(y + 8));
    x = (uint16_t)(x + 4);
    wr16(DP + 0x80, (uint16_t)(rd16(DP + 0x80) + 3));
  } while(rd16(DP + 0x80) != 0x12);
  x = rom16(0x81A896 + (uint32_t)rd16(DP + 0x7C));
  wr16(DP + 0x80, 0);
  do {  // (the stage is read again each time: a tile word may land on it)
    uint16_t level = (uint16_t)((rd16(DP + 0x3E) >> 1) - rom16(0x81A892 + (uint32_t)x));
    if(!(level & 0x8000)) {
      if(level >= 8) level = 7;
      uint16_t tile = rom16(0x81A882 + (uint32_t)(level << 1));
      wr16((uint16_t)(rom16(0x81A876 + (uint32_t)rd16(DP + 0x80)) + rd16(DP + 0xC4)), tile);
    }
    x = (uint16_t)(x + 4);
    wr16(DP + 0x80, (uint16_t)(rd16(DP + 0x80) + 2));
  } while(rd16(DP + 0x80) != 0x0C);
}

// A part's level for the bar entry at `x` (+`part` x 4): the half stage less its start
// (0 when below), at most its length.
static uint16_t shop_part_level(uint16_t x, uint32_t part) {
  uint16_t half = (uint16_t)(rd16(DP + 0x3E) >> 1), start = rom16(0x81A892 + part * 4 + (uint32_t)x);
  uint16_t level = half < start ? 0 : (uint16_t)(half - start);
  uint16_t length = rom16(0x81A894 + part * 4 + (uint32_t)x);
  return level < length ? level : length;
}

// TG3000_ROUTINE($80:A91E) shop_bar_double
void shop_bar_double(void) {
  const uint32_t y = rd16(DP + 0x74);
  uint16_t row = (uint16_t)((((0x6C - rd16(DP + 0x7A)) & 0xFF) << 8) + rd16(DP + 0xE7));
  wr16(DP + 0x82, row);
  uint16_t top = (uint16_t)(rd16(DP + 0x14) + 0x2C), bottom = (uint16_t)(rd16(DP + 0x14) + 0x34);
  wr16(0x0B00 + y, top);
  wr16(0x0900 + y, (uint16_t)((top & 0x00FF) | row));
  wr16(0x0B04 + y, bottom);
  wr16(0x0904 + y, (uint16_t)((bottom & 0x00FF) | row));
  wr16(0x0902 + y, 0x2322);
  wr16(0x0906 + y, 0x2322);
  wr16(0x0B02 + y, 0);
  wr16(0x0B06 + y, 0);
  wr16(DP + 0x74, (uint16_t)(y + 8));
  uint16_t x = rom16(0x81A896 + (uint32_t)rd16(DP + 0x7C));
  wr16(DP + 0x7E, shop_part_level(x, 0));
  wr16(DP + 0x80, 0x002C);
  wr16(DP + 0x84, 0x231E);
  shop_bar_column();
  x = rom16(0x81A896 + (uint32_t)rd16(DP + 0x7C));
  wr16(DP + 0x7E, shop_part_level(x, 1));
  wr16(DP + 0x80, 0x0033);
  wr16(DP + 0x84, 0x2320);
  shop_bar_column();
}

// TG3000_ROUTINE($80:AA59) shop_gauge
void shop_gauge(void) {
  uint16_t stage = rd16(DP + 0x3E);
  wr16(DP + 0x16, (uint16_t)(0x30 - rd16(DP + 0x7A) + rd16(DP + 0xE8)));
  wr16(DP + 0x18, 0x0100);
  sprite_group(DP, stage == 0x10 ? 0x7E33DA : 0x7E3438);
  stage = rd16(DP + 0x3E);
  uint16_t past = stage < 0x10 ? 0 : (uint16_t)(stage - 0x10);
  if(past < 0x38) {
    wr16(DP + 0x80, (uint16_t)((past & 0xFFF8) + 8));
    wr16(DP + 0x7E, (uint16_t)(((past & 7) << 1) + 0x2524));
    shop_bar_cap();
    uint16_t n = (uint16_t)((uint16_t)(0x37 - past) >> 3);
    wr16(DP + 0x82, n);
    while(rd16(DP + 0x82)) {
      wr16(DP + 0x7E, 0x2524);
      wr16(DP + 0x80, (uint16_t)(rd16(DP + 0x80) + 8));
      shop_bar_cap();
      wr16(DP + 0x82, (uint16_t)(rd16(DP + 0x82) - 1));
    }
  }
  stage = rd16(DP + 0x3E);
  uint16_t low = stage < 0x0C ? 0 : (uint16_t)(stage - 0x0C);
  if(low < 0x3C) {
    wr16(DP + 0x80, (uint16_t)(low & 0xFFFC));
    wr16(DP + 0x7E, (uint16_t)(((low & 3) << 1) + 0x251C));
    shop_bar_cap();
  }
}

// TG3000_ROUTINE($80:A26E) shop_item_price
uint16_t shop_item_price(void) {
  uint16_t stage = (uint16_t)(rd16(DP + 0x3E) - 0x10);
  if(stage & 0x8000) stage = 0;
  if(stage < 8) {
    const uint32_t y = rd16(DP + 0xC4);
    wr16(0x01A2 + y, rd16(0x4DCA + (uint32_t)(stage << 1)));
    wr16(0x01A4 + y, rd16(0x4DCA + (uint32_t)(stage & 0xFFFE)));
  }
  uint16_t base = rd16(DP + 0x14), x = rd16(DP + 0x16);
  uint16_t price = rom16(0x81A89B + (uint32_t)shop_item_entry(rd16(DP + 0x4A)));
  wr16(DP + 0x14, 0x3A00);
  uint32_t value = (uint32_t)rom16(0x81A88E + (uint32_t)price) << 16 | rom16(0x81A890 + (uint32_t)price);
  uint16_t shift = sprite_number_centered(value, (uint16_t)(0x5C2E + rd16(DP + 0xE7)));
  wr16(DP + 0x16, x);
  wr16(DP + 0x14, base);
  return shift;
}
