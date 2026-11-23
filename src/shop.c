// SPDX-License-Identifier: GPL-3.0-or-later
#include "shop.h"

#include <stdbool.h>
#include <stddef.h>

#include "frame.h"
#include "misc.h"
#include "platform.h"
#include "sprites.h"
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

static uint16_t shop_item_entry(uint16_t row);

// A 16-bit decimal-mode SBC of valid BCD words (the money and the prices always are):
// a - b - !carry, the borrow out as !carry.
static uint16_t bcd_sub16(uint16_t a, uint16_t b, bool* carry) {
  int borrow = *carry ? 0 : 1;
  uint16_t r = 0;
  for(int k = 0; k < 16; k += 4) {
    int d = (int)((a >> k) & 0xF) - (int)((b >> k) & 0xF) - borrow;
    borrow = d < 0;
    if(borrow) d += 10;
    r = (uint16_t)(r | (uint16_t)(d << k));
  }
  *carry = !borrow;
  return r;
}

// TG3000_ROUTINE($80:9957) shop_money_after_price
uint16_t shop_money_after_price(void) {
  ShopMoneyFlags f;
  return shop_money_after_price_flags(&f);
}

static uint16_t shop_money_settle(ShopMoneyFlags* f);

uint16_t shop_money_after_price_flags(ShopMoneyFlags* f) {
  const uint16_t price = rom16(0x81A89B + (uint32_t)shop_item_entry(rd16(DP + 0x4A)));
  const uint32_t car = rd16(DP + 0x94);
  const uint16_t lo = rd16(0x038E + car), hi = rd16(0x0390 + car);
  bool c = true;
  wr16(DP + 0x52, bcd_sub16(lo, rom16(0x81A890 + (uint32_t)price), &c));
  wr16(DP + 0x54, bcd_sub16(hi, rom16(0x81A88E + (uint32_t)price), &c));
  return shop_money_settle(f);
}

// A 16-bit decimal-mode ADC of valid BCD words: a + b + carry, the carry out.
static uint16_t bcd_add16(uint16_t a, uint16_t b, bool* carry) {
  int c = *carry ? 1 : 0;
  uint16_t r = 0;
  for(int k = 0; k < 16; k += 4) {
    int d = (int)((a >> k) & 0xF) + (int)((b >> k) & 0xF) + c;
    c = d > 9;
    if(c) d -= 10;
    r = (uint16_t)(r | (uint16_t)(d << k));
  }
  *carry = c != 0;
  return r;
}

// TG3000_ROUTINE($80:990D) shop_money_after_refund
// The money plus the price of row `row` of the current list (the part owned, sold back), to
// D+$52/D+$54; D+$76 bit 2 cleared; then the settled digits as shop_money_after_price's.
// The buying handlers' path for a part already bought here (bit 7); the shop-buy movie runs
// it once.
uint16_t shop_money_after_refund(uint16_t row) {
  wr16(DP + 0x7A, row);
  const uint16_t price = rom16(0x81A89B + (uint32_t)shop_item_entry(row));
  const uint32_t car = rd16(DP + 0x94);
  bool c = false;
  wr16(DP + 0x52, bcd_add16(rd16(0x038E + car), rom16(0x81A890 + (uint32_t)price), &c));
  wr16(DP + 0x54, bcd_add16(rd16(0x0390 + car), rom16(0x81A88E + (uint32_t)price), &c));
  wr16(DP + 0x76, (uint16_t)(rd16(DP + 0x76) & ~0x0004));
  ShopMoneyFlags f;
  return shop_money_settle(&f);
}

// $80:997E on: the digits that settle, from the money (D+$94's) and D+$52/D+$54.
static uint16_t shop_money_settle(ShopMoneyFlags* f) {
  const uint32_t car = rd16(DP + 0x94);
  const uint16_t lo = rd16(0x038E + car), hi = rd16(0x0390 + car);
  bool c = false;
  wr16(DP + 0x56, 0);
  wr16(DP + 0x58, 0);
  wr16(DP + 0x7A, (uint16_t)(lo | rd16(DP + 0x52)));
  wr16(DP + 0x7C, (uint16_t)(hi | rd16(DP + 0x54)));
  // From the top, the digits that neither number has, or that stay the same, settle (D+$58).
  // The mask tested each time is every digit from the top down to the current one (ORA of
  // what settled before the shift).
  uint16_t y = 4, a = 0xF000;
  f->v = (rd16(DP + 0x7C) >> 14) & 1;  // BIT $7C's V
  for(; y != 0; y--) {
    if(a & rd16(DP + 0x7C)) {
      wr16(DP + 0x7E, a);
      wr16(DP + 0x80, (uint16_t)(a & hi));
      const uint16_t mine = (uint16_t)(a & rd16(DP + 0x54));
      c = mine >= rd16(DP + 0x80);  // CMP
      if(mine != rd16(DP + 0x80)) goto done;
    }
    a = (uint16_t)(a | rd16(DP + 0x58));
    wr16(DP + 0x58, a);
    c = (a >> 3) & 1;  // the last of four LSRs
    a = (uint16_t)(a >> 4);
  }
  // all four high digits settled: the low word's leading zeros the same way (D+$56); no
  // run takes this path
  f->v = (rd16(DP + 0x7A) >> 14) & 1;  // BIT $7A's V
  for(y = 4, a = 0xF000; y != 0; y--) {
    if(a & rd16(DP + 0x7A)) goto done;
    a = (uint16_t)(a | rd16(DP + 0x56));
    wr16(DP + 0x56, a);
    c = (a >> 3) & 1;
    a = (uint16_t)(a >> 4);
  }
done:
  wr16(DP + 0x5A, 4);
  f->c = c;
  return y;
}

// The digit mask D+$7C:D+$7A one digit lower (LSR $7C / ROR $7A, four times); the carry
// the last ROR leaves.
static bool shop_money_mask_down(void) {
  const uint32_t m = (uint32_t)rd16(DP + 0x7A) | (uint32_t)rd16(DP + 0x7C) << 16;
  wr16(DP + 0x7A, (uint16_t)(m >> 4));
  wr16(DP + 0x7C, (uint16_t)(m >> 20));
  return (m >> 3) & 1;
}

// TG3000_ROUTINE($80:99D1) shop_money_roll
uint16_t shop_money_roll(void) {
  ShopMoneyFlags f = {false, false};
  return shop_money_roll_flags(&f);
}

uint16_t shop_money_roll_flags(ShopMoneyFlags* f) {
  wr16(DP + 0x7A, 0);
  wr16(DP + 0x7C, 0xF000);
  wr16(DP + 0x84, 8);
  const uint32_t car = rd16(DP + 0x94);
  uint16_t a;
  // the digits already settled (D+$56/D+$58), from the top, are passed over
  for(;;) {
    wr16(DP + 0x7E, (uint16_t)((rd16(DP + 0x56) & rd16(DP + 0x7A)) | (rd16(DP + 0x58) & rd16(DP + 0x7C))));
    a = (uint16_t)(rd16(DP + 0x7A) | rd16(DP + 0x7C));
    if(a != rd16(DP + 0x7E)) break;
    f->c = shop_money_mask_down();
    wr16(DP + 0x84, (uint16_t)(rd16(DP + 0x84) - 1));
    if(rd16(DP + 0x84) == 0) return a;
  }
  wr16(DP + 0x5A, (uint16_t)(rd16(DP + 0x5A) - 1));
  if(rd16(DP + 0x5A) == 0) {  // the first unsettled digit takes its new value and settles
    wr16(0x038E + car, (uint16_t)((rd16(0x038E + car) & ~rd16(DP + 0x7A)) | (rd16(DP + 0x52) & rd16(DP + 0x7A))));
    wr16(0x0390 + car, (uint16_t)((rd16(0x0390 + car) & ~rd16(DP + 0x7C)) | (rd16(DP + 0x54) & rd16(DP + 0x7C))));
    wr16(DP + 0x7E, (uint16_t)(rd16(DP + 0x52) & rd16(DP + 0x7A)));
    wr16(DP + 0x80, (uint16_t)(rd16(DP + 0x54) & rd16(DP + 0x7C)));
    wr16(DP + 0x56, (uint16_t)(rd16(DP + 0x56) | rd16(DP + 0x7A)));
    wr16(DP + 0x58, (uint16_t)(rd16(DP + 0x58) | rd16(DP + 0x7C)));
    wr16(DP + 0x5A, 4);
    a = 4;
    goto next;
  }
  for(;;) {  // this digit and every one below it roll down by one (0 to 9)
    {
      const uint16_t m1 = rd16(DP + 0x7A), m2 = rd16(DP + 0x7C);
      const uint16_t one1 = (uint16_t)(0x1111 & m1), one2 = (uint16_t)(0x1111 & m2);
      const uint16_t now1 = (uint16_t)(rd16(0x038E + car) & m1), now2 = (uint16_t)(rd16(0x0390 + car) & m2);
      const uint32_t d1 = (uint32_t)now1 - one1;
      const uint16_t lo0 = (uint16_t)d1, borrow1 = (d1 >> 16) & 1;
      const uint32_t d2 = (uint32_t)now2 - one2 - borrow1;
      const uint16_t hi0 = (uint16_t)d2;
      f->v = ((now2 ^ one2) & (now2 ^ hi0) & 0x8000) != 0;  // SBC's V
      uint16_t lo = lo0, hi = hi0;
      if(d2 >> 16) {  // a borrow out: the digit was 0 and wraps to 9
        const uint32_t s1 = (uint32_t)lo0 + 0xAAAA;
        const uint32_t s2 = (uint32_t)hi0 + 0xAAAA + (s1 >> 16);
        f->v = (~(hi0 ^ 0xAAAA) & (hi0 ^ (uint16_t)s2) & 0x8000) != 0;  // ADC's V
        lo = (uint16_t)(s1 & m1);
        hi = (uint16_t)(s2 & m2);
      }
      wr16(DP + 0x7E, lo);
      wr16(DP + 0x80, hi);
      wr16(0x038E + car, (uint16_t)((rd16(0x038E + car) & ~m1) | lo));
      a = (uint16_t)((rd16(0x0390 + car) & ~m2) | hi);
      wr16(0x0390 + car, a);
    }
  next:
    f->c = shop_money_mask_down();
    wr16(DP + 0x84, (uint16_t)(rd16(DP + 0x84) - 1));
    if(rd16(DP + 0x84) == 0) return a;
  }
}

// The buying of one part of the car, lists 1-5 (shop_buy_list1-5): the part's level byte at $0393 + `field` +
// D+$94 (bit 7: bought here). Owned already: a sound, the icon shown and settled. Else the
// money is taken; unless it covers the price (shop_item_affordable), shop_flag_fb and out.
// Bought: a sound, the panel rises (D+$42 by 6 a frame up to `height`, the money rolling,
// the screen redrawn), the level byte becomes the row | $80, the upload is queued, the money
// after the price, the panel falls (down to 0), the money stored, the icon settled.
// `settle`: false stops before the final shop_item_icon_settle (list 1's JMP to it, where its
// row ends). `f` (or NULL): the flags shop_money_after_price leaves when the money is short
// after a refund (the last call on that path for lists 2-5).
ShopBuyEnd shop_buy_part(uint16_t field, uint16_t height, bool price_first, bool settle, ShopMoneyFlags* f) {
  ShopMoneyFlags unused;
  if(f == NULL) f = &unused;
  const uint32_t at = 0x0393 + (uint32_t)field + rd16(DP + 0x94);
  if((rd16(at) & 0x007F) == rd16(DP + 0x4A)) {
    func_80E4C5();
    func_80B58D(0x80, 0xB535);
    shop_item_show_icon();
    if(settle) shop_item_icon_settle();
    return SHOP_BUY_OWNED;
  }
  shop_money_take();
  if(rd16(at) & 0x0080) {  // a part bought here before is sold back first
    shop_money_after_refund((uint16_t)(rd16(at) & 0x007F));
    if(!shop_item_affordable()) {
      if(price_first) {
        (void)shop_money_after_price_flags(f);
        shop_flag_fb();
      } else {
        shop_flag_fb();
        (void)shop_money_after_price_flags(f);
      }
      return SHOP_BUY_REFUNDED_SHORT;
    }
  } else if(!shop_item_affordable()) {
    shop_flag_fb();
    return SHOP_BUY_SHORT;
  }
  func_80E4DB();
  func_80B58D(0x80, 0xB535);
  wr16(DP + 0x76, (uint16_t)(rd16(DP + 0x76) | 0x0002));
  shop_item_show_icon();
  do {
    wait_frames(1);
    wr16(DP + 0x42, (uint16_t)(rd16(DP + 0x42) + 6));
    shop_money_roll();
    shop_item_redraw();
  } while(rd16(DP + 0x42) != height);
  wr8(at, (uint8_t)(rd8(DP + 0x4A) | 0x80));
  shop_item_dispatch_upload();
  wr16(DP + 0x76, (uint16_t)(rd16(DP + 0x76) & ~0x0002));
  shop_money_after_price();
  do {
    wait_frames(1);
    wr16(DP + 0x42, (uint16_t)(rd16(DP + 0x42) - 6));
    shop_money_roll();
    shop_item_redraw();
  } while(rd16(DP + 0x42) != 0);
  shop_money_store();
  if(settle) shop_item_icon_settle();
  return SHOP_BUY_BOUGHT;
}

// TG3000_ROUTINE($80:9053) shop_buy_list1
void shop_buy_list1(void) { (void)shop_buy_part(0, 0x0078, true, true, NULL); }
// TG3000_ROUTINE($80:9100) shop_buy_list2
void shop_buy_list2(void) { (void)shop_buy_part(1, 0x00B4, false, true, NULL); }
// TG3000_ROUTINE($80:91AE) shop_buy_list3
void shop_buy_list3(void) { (void)shop_buy_part(2, 0x00B4, false, true, NULL); }
// TG3000_ROUTINE($80:925C) shop_buy_list4
void shop_buy_list4(void) { (void)shop_buy_part(3, 0x00B4, false, true, NULL); }
// TG3000_ROUTINE($80:95C0) shop_buy_list5
void shop_buy_list5(void) { (void)shop_buy_part(4, 0x00B4, false, true, NULL); }

// Lists 6-8's buying (shop_buy_upgrade): as shop_buy_part's, with `field` 5-7 and the level byte
// the row + 1. Their panel moves are written apart from shop_buy_part's for now (the same
// code in the ROM, which repeats it per list).
static void shop_panel_rise(uint16_t height) {
  wr16(DP + 0x76, (uint16_t)(rd16(DP + 0x76) | 0x0002));
  shop_item_show_icon();
  do {
    wait_frames(1);
    wr16(DP + 0x42, (uint16_t)(rd16(DP + 0x42) + 6));
    shop_money_roll();
    shop_item_redraw();
  } while(rd16(DP + 0x42) != height);
}
ShopBuyEnd shop_buy_upgrade(uint16_t field, ShopMoneyFlags* f) {
  ShopMoneyFlags unused;
  if(f == NULL) f = &unused;
  const uint32_t at = 0x0393 + (uint32_t)field + rd16(DP + 0x94);
  if((uint16_t)((rd16(at) & 0x007F) - 1) == rd16(DP + 0x4A)) {
    func_80E4C5();
    func_80B58D(0x80, 0xB535);
    shop_item_show_icon();
    shop_item_icon_settle();
    return SHOP_BUY_OWNED;
  }
  shop_money_take();
  const uint16_t level = (uint16_t)(rd16(at) & 0x00FF);
  if(level == 0) {  // the first purchase: the panel is up at once
    if(!shop_item_affordable()) {
      shop_flag_fb();
      return SHOP_BUY_SHORT;
    }
    func_80E4DB();
    func_80B58D(0x80, 0xB535);
    wr16(DP + 0x42, 0x00B4);
    wr16(DP + 0x44, 0x000F);
    func_80B58D(0x80, 0xB535);
    shop_item_show_icon();
  } else {
    const uint16_t before = (uint16_t)(level - 1);
    if(before & 0x0080) {  // bought here before: sold back first
      shop_money_after_refund((uint16_t)(before & 0x007F));
      if(!shop_item_affordable()) {
        shop_flag_fb();
        (void)shop_money_after_price_flags(f);
        return SHOP_BUY_REFUNDED_SHORT;
      }
    } else if(!shop_item_affordable()) {
      shop_flag_fb();
      return SHOP_BUY_SHORT;
    }
    func_80E4DB();
    func_80B58D(0x80, 0xB535);
    shop_panel_rise(0x00B4);
  }
  wr8(at, (uint8_t)((uint8_t)(rd8(DP + 0x4A) + 1) | 0x80));
  shop_item_dispatch_upload();
  wr16(DP + 0x76, (uint16_t)(rd16(DP + 0x76) & ~0x0002));
  shop_money_after_price();
  do {
    wait_frames(1);
    wr16(DP + 0x42, (uint16_t)(rd16(DP + 0x42) - 6));
    shop_money_roll();
    shop_item_redraw();
  } while(rd16(DP + 0x42) != 0);
  shop_money_store();
  shop_item_icon_settle();
  return SHOP_BUY_BOUGHT;
}
// TG3000_ROUTINE($80:94DE) shop_buy_list6
void shop_buy_list6(void) { (void)shop_buy_upgrade(5, NULL); }
// TG3000_ROUTINE($80:93FC) shop_buy_list7
void shop_buy_list7(void) { (void)shop_buy_upgrade(6, NULL); }
// TG3000_ROUTINE($80:931A) shop_buy_list8
void shop_buy_list8(void) { (void)shop_buy_upgrade(7, NULL); }

// TG3000_ROUTINE($80:930A) shop_buy_colour
void shop_buy_colour(void) {
  wr8(0x0392 + (uint32_t)rd16(DP + 0x94), rd8(DP + 0x4A));
  func_80E4DB();
}

// TG3000_ROUTINE($80:9ABB) shop_money_store
void shop_money_store(void) {
  const uint32_t car = rd16(DP + 0x94);
  wr16(0x038E + car, rd16(DP + 0x52));
  wr16(0x0390 + car, rd16(DP + 0x54));
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

// One of a bar entry's sprite groups ($7E:word at $81:A892 + `entry` + `field`) at x =
// `x` + D+$E8, the tile base $0100.
static void shop_item_group(uint16_t entry, uint32_t field, uint16_t x) {
  wr16(DP + 0x16, (uint16_t)(x + rd16(DP + 0xE8)));
  wr16(DP + 0x18, 0x0100);
  sprite_group(DP, 0x7E0000 | rom16(0x81A892 + field + (uint32_t)entry));
}

// The stage moves on, stopping at $7FFF.
static void shop_item_stage_step(void) {
  uint16_t stage = (uint16_t)(rd16(DP + 0x3E) + 1);
  if(stage & 0x8000) stage = (uint16_t)(stage - 1);
  wr16(DP + 0x3E, stage);
}

// TG3000_ROUTINE($80:A2CB) shop_item_view_a
void shop_item_view_a(uint16_t entry) {
  wr16(DP + 0x7A, rd16(DP + 0x16));
  wr16(DP + 0x7C, entry);
  shop_bar_levels();
  shop_item_group(rd16(DP + 0x7C), 0, rd16(DP + 0x7A));
  wr16(DP + 0x84, 0x2320);
  shop_bar_segments();
  shop_item_group(rd16(DP + 0x7C), 6, (uint16_t)(0x30 - rd16(DP + 0x7A)));
  shop_item_group(rd16(DP + 0x7C), 2, (uint16_t)(0x30 - rd16(DP + 0x7A)));
  shop_item_stage_step();
}

// TG3000_ROUTINE($80:A831) shop_item_view_b
void shop_item_view_b(uint16_t entry) {
  wr16(DP + 0x7A, rd16(DP + 0x16));
  wr16(DP + 0x7C, entry);
  shop_bar_double();
  wr16(DP + 0x84, 0x2312);
  shop_bar_segments_long();
  shop_item_group(rd16(DP + 0x7C), 0, rd16(DP + 0x16));
  shop_item_group(rd16(DP + 0x7C), 2, (uint16_t)(0x30 - rd16(DP + 0x7A)));
  shop_item_stage_step();
}

// TG3000_ROUTINE($80:AA15) shop_item_view_c
void shop_item_view_c(uint16_t entry) {
  wr16(DP + 0x7A, rd16(DP + 0x16));
  wr16(DP + 0x7C, entry);
  shop_item_group(rd16(DP + 0x7C), 0, rd16(DP + 0x16));
  shop_gauge();
  shop_item_group(rd16(DP + 0x7C), 2, (uint16_t)(0x30 - rd16(DP + 0x7A)));
  shop_item_stage_step();
}

// TG3000_ROUTINE($80:A717) shop_item_view_d
void shop_item_view_d(uint16_t entry) {
  wr16(DP + 0x7A, rd16(DP + 0x16));
  wr16(DP + 0x7C, entry);
  shop_item_group(rd16(DP + 0x7C), 0, rd16(DP + 0x7A));
  uint16_t y = rd16(DP + 0x74);
  wr16(DP + 0x16, (uint16_t)(((0x30 - rd16(DP + 0x7A) + rd16(DP + 0xE8) + 0x20) & 0xFF) << 8));
  uint16_t half = (uint16_t)(rd16(DP + 0x3E) >> 1);
  uint16_t blocks;
  bool draw = true;
  if(half < 0x20) {  // not started: four full blocks
    wr16(DP + 0x7E, (uint16_t)(rd16(DP + 0x14) + 0x14));
    blocks = 4;
  } else {
    uint16_t done = (uint16_t)((half - 0x20) & 0xFFF8);
    if(done >= 0x20) {
      draw = false;
      blocks = 0;
    } else {  // the block under way, then the full ones left
      wr16(DP + 0x7E, (uint16_t)(done + rd16(DP + 0x14) + 0x1C));
      y = shop_sprite_block((uint16_t)((half & 7) + 0x2316), y);
      blocks = (uint16_t)((uint16_t)(0x1F - (uint16_t)(half - 0x20)) >> 3);
    }
  }
  if(draw) {
    for(; blocks; blocks--) {
      wr16(DP + 0x7E, (uint16_t)(rd16(DP + 0x7E) + 8));
      y = shop_sprite_block(0x2316, y);
    }
    wr16(DP + 0x74, y);
  }
  shop_item_group(rd16(DP + 0x7C), 2, (uint16_t)(0x30 - rd16(DP + 0x7A)));
  shop_item_group(rd16(DP + 0x7C), 4, (uint16_t)(0x30 - rd16(DP + 0x7A)));
  shop_item_stage_step();
}

// TG3000_ROUTINE($80:AB54) shop_item_view_e
void shop_item_view_e(uint16_t entry) {
  wr16(DP + 0x7A, rd16(DP + 0x16));
  wr16(DP + 0x7C, entry);
  shop_item_group(rd16(DP + 0x7C), 0, rd16(DP + 0x16));
  shop_item_group(rd16(DP + 0x7C), 2, (uint16_t)(0x30 - rd16(DP + 0x7A)));
  shop_item_group(rd16(DP + 0x7C), 4, (uint16_t)(0x30 - rd16(DP + 0x7A)));
  const uint32_t y = rd16(DP + 0xC4);
  uint16_t wave = rd16(DP + 0x40);
  uint32_t x = (wave & 0x0020) ? (uint32_t)(uint16_t)(0x1E - (wave & 0x001E)) : (uint32_t)(wave & 0x001E);
  wr16(0x012E + y, rd16(0x4E1A + x));
  wr16(0x012A + y, rd16(0x4E3A + x));
  wr16(0x0128 + y, rd16(0x4E5A + x));
  wr16(0x0126 + y, rd16(0x4E7A + x));
  wr16(0x0124 + y, rd16(0x4E9A + x));
  wr16(0x0122 + y, rd16(0x4EBA + x));
  wr16(DP + 0x40, (uint16_t)(rd16(DP + 0x40) + 1));
  shop_item_stage_step();
}

// TG3000_ROUTINE($80:A1E4) shop_item_icon_draw
void shop_item_icon_draw(uint16_t group) {
  wr16(DP + 0x18, (uint16_t)(rd16(DP + 0x44) << 1) < 0x0010 ? 0x0E00 : 0x2E00);
  wr16(DP + 0x14, rd16(DP + 0x42));
  wr16(DP + 0x16, rd16(DP + 0xE8));
  sprite_group(DP, 0x7E0000 | group);
  uint16_t step = rd16(DP + 0x44);
  if(step & 0x8000) return;
  const uint32_t x = (uint16_t)(step << 1), y = rd16(DP + 0xC4);
  wr16(0x01EE + y, rd16(0x4EDA + x));
  wr16(0x01F0 + y, rd16(0x4EFA + x));
  if(x < 0x0010) {
    wr16(0x0182 + y, rd16(0x4F1A + x));
    wr16(0x0186 + y, rd16(0x4F2A + x));
    wr16(0x0188 + y, rd16(0x4F3A + x));
    wr16(0x018E + y, rd16(0x4F4A + x));
  }
  if(step == 0x000F || (step == 0x0007 && !(rd16(DP + 0x76) & 0x8000))) step |= 0x8000;
  else step = (uint16_t)(step + 1);
  wr16(DP + 0x44, step);
}

// One list's handler: its level byte at $0393 + `field` + D+$94; the icon table at $81:`icons`;
// the upload's VRAM offset at $81:`offsets` and size at $81:`sizes` (4-byte rows); with
// `skipZero`, level 0 queues nothing.
static void shop_item_icon_from(bool upload, uint16_t field, uint16_t icons, uint16_t offsets, uint16_t sizes,
                                bool skipZero) {
  uint16_t level = (uint16_t)(rd16(0x0393 + (uint32_t)field + rd16(DP + 0x94)) & 0x007F);
  if(!upload) {
    shop_item_icon_draw(rom16(0x810000 + (uint32_t)icons + (uint32_t)(uint16_t)(level << 1)));
    return;
  }
  if(skipZero && level == 0) return;
  const uint32_t row = (uint32_t)(uint16_t)(level << 2);
  wr16(DP + 0x14, 0x007F);
  func_80B537(DP, rom16(0x810000 + (uint32_t)sizes + row),
              (uint16_t)(rom16(0x810000 + (uint32_t)offsets + row) + rd16(DP + 0x5E) + 0x0900),
              (uint16_t)(rd16(DP + 0x5C) + 0x1000));
}
static void shop_item_icon(bool upload, uint16_t field, uint16_t icons, uint16_t uploads) {
  shop_item_icon_from(upload, field, icons, uploads, (uint16_t)(uploads + 2), false);
}

// TG3000_ROUTINE($80:9F5B) shop_item_icon_1
void shop_item_icon_1(bool upload) { shop_item_icon(upload, 0, 0xBC2E, 0xBC16); }

// TG3000_ROUTINE($80:9F9A) shop_item_icon_2
void shop_item_icon_2(bool upload) { shop_item_icon(upload, 1, 0xBC52, 0xBC3A); }

// TG3000_ROUTINE($80:9FD9) shop_item_icon_3
void shop_item_icon_3(bool upload) { shop_item_icon(upload, 2, 0xBC76, 0xBC5E); }

// TG3000_ROUTINE($80:A057) shop_item_icon_4
void shop_item_icon_4(bool upload) { shop_item_icon(upload, 3, 0xBCBE, 0xBCA6); }

// TG3000_ROUTINE($80:A018) shop_item_icon_5
void shop_item_icon_5(bool upload) { shop_item_icon(upload, 4, 0xBC9A, 0xBC82); }

// The list's level getter ($80:9EEA table): lists 0-5 the part levels plus 1; lists 6, 7, 8
// the bytes at $0398, $0399, $039A as they are (9: $039C).
static uint16_t shop_list_level(uint16_t list, uint16_t car) {
  switch(list) {
  case 0: return shop_part_level_2(car);
  case 1: return shop_part_level_3(car);
  case 2: return shop_part_level_4(car);
  case 3: return shop_part_level_5(car);
  case 4: return shop_part_level_6(car);
  case 5: return shop_part_level_7(car);
  case 6: return rd16(0x0398 + (uint32_t)car);
  case 7: return rd16(0x0399 + (uint32_t)car);
  case 8: return rd16(0x039A + (uint32_t)car);
  default: return rd16(0x039C + (uint32_t)car);
  }
}

static uint16_t shop_mark(uint16_t slot, uint16_t tile, uint16_t pos) {
  const uint32_t x = slot;
  wr16(0x0902 + x, tile);
  wr16(0x0900 + x, pos);
  wr16(0x0B01 + x, 0x0100);
  return (uint16_t)(slot + 4);
}

// TG3000_ROUTINE($80:9E49) shop_level_marks
void shop_level_marks(void) {
  uint16_t slot = rd16(DP + 0x74);
  uint16_t gap = (uint16_t)((uint16_t)(rd16(DP + 0x4A) << 4) + 0x14 - rd16(DP + 0xEE));
  if(gap) wr16(DP + 0xEE, (uint16_t)(rd16(DP + 0xEE) + ((gap & 0x8000) ? -1 : 1)));
  slot = shop_mark(slot, 0x3892, (uint16_t)(0x085A + rd16(DP + 0xEE) + rd16(DP + 0xE7)));
  slot = shop_mark(slot, 0x3890, (uint16_t)(0x0A5C + rd16(DP + 0xE7)));
  wr16(DP + 0x74, slot);
  uint16_t list = rd16(DP + 0x48);
  wr16(DP + 0x24, (uint16_t)(rd16(0x036E + (uint32_t)list) & 0x00FF));
  wr16(DP + 0x26, (uint16_t)(0x0A70 + rd16(DP + 0xE7)));
  wr16(DP + 0x28, 0x3850);
  uint16_t left = (uint16_t)(shop_list_level(list, rd16(DP + 0x94)) & 0x007F);
  do {
    uint16_t pos = rd16(DP + 0x26);
    wr16(DP + 0x26, (uint16_t)(pos + 0x10));
    uint16_t tile = rd16(DP + 0x28);
    if(--left == 0) tile = (uint16_t)(tile + 0x20);
    const uint32_t x = slot;
    wr16(0x0900 + x, pos);
    wr16(0x0902 + x, tile);
    wr16(0x0B01 + x, 0x0100);
    wr16(DP + 0x28, (uint16_t)(rd16(DP + 0x28) + 2));
    slot = (uint16_t)(slot + 4);
    wr16(DP + 0x24, (uint16_t)(rd16(DP + 0x24) - 1));
  } while(rd16(DP + 0x24) != 0);
  wr16(DP + 0x74, slot);
}

#define SHOP_PANEL_MAP 0x10000  // $7F:0000

// TG3000_ROUTINE($80:B0C0) shop_panel_blank
void shop_panel_blank(void) {
  const uint32_t x = rd16(DP + 0x5E);
  for(uint32_t col = 0; col < 16; col += 2)
    for(uint32_t row = 0; row < 16; row++) wr16(SHOP_PANEL_MAP + (uint16_t)(0x0700 + row * 0x20 + x + col), 0xFFFF);
  wr16(DP + 0x4E, 0);
  wr16(DP + 0xEC, 0xB149);
}

// TG3000_ROUTINE($80:B10C) shop_panel_clear
void shop_panel_clear(void) {
  for(;;) {
    wr16(DP + 0xEC, 0xB94F);
    if(rd16(DP + 0x8C) == rd16(DP + 0x8E)) break;
    platform_wait_vblank();
  }
  const uint32_t x = rd16(DP + 0x5E);
  for(uint32_t col = 0; col < 18; col += 2)
    for(uint32_t row = 0; row < 9; row++) wr16(SHOP_PANEL_MAP + (uint16_t)(0x00C4 + row * 0x40 + x + col), 0);
}

static uint16_t shop_list_key(void) {
  uint16_t a = (uint16_t)(rd16(DP + 0x48) << 3);
  a = (uint16_t)((a << 8) | (a >> 8));  // XBA
  return (uint16_t)(a + rd16(DP + 0x4A));
}

// TG3000_ROUTINE($80:B359) shop_list_locked
bool shop_list_locked(void) {
  uint16_t key = shop_list_key();
  if(key & 0x8000) return true;
  return !(key == 0x1800 || ((uint16_t)(key - 0x1800) & 0x8000));
}

// TG3000_ROUTINE($80:B333) shop_list_prev
void shop_list_prev(void) {
  if(shop_list_locked() || rd16(DP + 0x48) == 0) return;
  wr16(DP + 0x48, (uint16_t)(rd16(DP + 0x48) - 1));
  wr16(DP + 0x4E, 0);
  func_80E4C5();
}

// TG3000_ROUTINE($80:B345) shop_list_next
void shop_list_next(void) {
  if(shop_list_locked() || rd16(DP + 0x48) == rd16(DP + 0xF0)) return;
  wr16(DP + 0x48, (uint16_t)(rd16(DP + 0x48) + 1));
  wr16(DP + 0x4E, 0);
  func_80E4C5();
}

static void shop_item_dispatch(bool upload) {
  uint16_t entry = shop_item_entry(rd16(DP + 0x4A));
  switch(rom16(0x81A8A3 + (uint32_t)entry)) {
  case 0x9F5B: shop_item_icon_1(upload); break;
  case 0x9F9A: shop_item_icon_2(upload); break;
  case 0x9FD9: shop_item_icon_3(upload); break;
  case 0xA057: shop_item_icon_4(upload); break;
  case 0xA018: shop_item_icon_5(upload); break;
  // lists 6-8 ($80:A121, $80:A0E0, $80:A09F): as the others, but level 0 queues nothing
  case 0xA121: shop_item_icon_from(upload, 5, 0xBCFA, 0xBCD6, 0xBCD8, true); break;
  case 0xA0E0: shop_item_icon_from(upload, 6, 0xBCF6, 0xBCCE, 0xBCC8, true); break;
  case 0xA09F: shop_item_icon_from(upload, 7, 0xBCF2, 0xBCC6, 0xBCC8, true); break;
  case 0xA096:  // list 0 ($80:A096): a fixed icon, no upload
    if(!upload) shop_item_icon_draw(0x4F6A);
    break;
  default: break;  // the table holds no other handler for lists 0-8
  }
}

// TG3000_ROUTINE($80:9F40) shop_item_dispatch_icon
void shop_item_dispatch_icon(void) { shop_item_dispatch(false); }

// TG3000_ROUTINE($80:9C0B) shop_item_dispatch_upload
void shop_item_dispatch_upload(void) { shop_item_dispatch(true); }

// TG3000_ROUTINE($80:ABF6) shop_item_view_g
void shop_item_view_g(uint16_t entry) {
  wr16(DP + 0x7A, rd16(DP + 0x16));
  wr16(DP + 0x7C, entry);
  shop_item_group(rd16(DP + 0x7C), 0, rd16(DP + 0x7A));
  shop_item_group(rd16(DP + 0x7C), 2, (uint16_t)(0x30 - rd16(DP + 0x7A)));
  shop_item_stage_step();
}

static uint16_t shop_ring_piece(uint32_t table, uint16_t angle, uint16_t slot) {
  const uint32_t y = slot;
  uint16_t frame = (uint16_t)((angle >> 8) & 0x000F);
  wr16(DP + 0x16, frame);
  const uint32_t x = (uint32_t)frame * 6;
  wr16(0x0902 + y, rd16(table + x));
  uint16_t row = (uint16_t)(((rd16(table + 2 + x) + rd16(DP + 0x1A)) & 0xFF) << 8);
  wr16(DP + 0x16, row);
  uint16_t at = (uint16_t)(rd16(DP + 0x18) + rd16(table + 4 + x));
  wr16(0x0B00 + y, at);
  wr16(0x0900 + y, (uint16_t)((at & 0x00FF) | row));
  wr16(0x0B02 + y, 0);
  return (uint16_t)(slot + 4);
}

// TG3000_ROUTINE($80:A69F) shop_ring_piece_front
uint16_t shop_ring_piece_front(uint16_t angle, uint16_t slot) { return shop_ring_piece(0x4D0A, angle, slot); }

// TG3000_ROUTINE($80:A6DB) shop_ring_piece_back
uint16_t shop_ring_piece_back(uint16_t angle, uint16_t slot) { return shop_ring_piece(0x4D6A, angle, slot); }

// TG3000_ROUTINE($80:A579) shop_ring
void shop_ring(void) {
  wr16(DP + 0x1A, (uint16_t)(0x5E - rd16(DP + 0x7A) + rd16(DP + 0xE8)));
  wr16(DP + 0x18, (uint16_t)(rd16(DP + 0x14) + 0x2B));
  uint16_t y = rd16(DP + 0x74);
  for(int behind = 1; behind >= 0; behind--) {
    for(uint16_t n = 0; n < 2; n++) {
      uint16_t angle = (uint16_t)(rd16(DP + 0x40) + n * 0x0400);
      if(((angle & 0x0800) != 0) != behind) continue;
      y = (n & 1) ? shop_ring_piece_back(angle, y) : shop_ring_piece_front(angle, y);
    }
    if(behind) {  // the centre, between the two halves
      const uint32_t s = y;
      wr16(0x0902 + s, 0x2526);
      wr16(0x0B02 + s, 0x0001);
      uint16_t at = rd16(DP + 0x18);
      wr16(0x0B00 + s, at);
      uint16_t row = (uint16_t)((rd16(DP + 0x1A) & 0x00FF) << 8);
      wr16(DP + 0x16, row);
      wr16(0x0900 + s, (uint16_t)((at & 0x00FF) | row));
      y = (uint16_t)(y + 4);
    }
  }
  wr16(DP + 0x74, y);
  wr16(DP + 0x40, (uint16_t)(rd16(DP + 0x40) + 0x0080));
}

// TG3000_ROUTINE($80:A333) shop_item_view_f
void shop_item_view_f(uint16_t entry) {
  wr16(DP + 0x7A, rd16(DP + 0x16));
  wr16(DP + 0x7C, entry);
  shop_ring();
  shop_item_group(rd16(DP + 0x7C), 0, rd16(DP + 0x7A));
  wr16(DP + 0x84, 0x2338);
  shop_bar_segments();
  shop_item_group(rd16(DP + 0x7C), 6, (uint16_t)(0x30 - rd16(DP + 0x7A)));
  shop_item_group(rd16(DP + 0x7C), 2, (uint16_t)(0x30 - rd16(DP + 0x7A)));
  shop_item_stage_step();
}

static uint16_t shop_view_x(uint16_t base) { return (uint16_t)(base - rd16(DP + 0x7A) + rd16(DP + 0xE8)); }

static void shop_view_group(uint16_t x, uint16_t group) {
  wr16(DP + 0x16, x);
  wr16(DP + 0x18, 0x0100);
  sprite_group(DP, 0x7E0000 | group);
}

// D+$40 + $40, back by `wrap` when it reaches it
static void shop_view_spin(uint16_t wrap) {
  uint16_t a = (uint16_t)(rd16(DP + 0x40) + 0x0040);
  if(a >= wrap) a = (uint16_t)(a - wrap);
  wr16(DP + 0x40, a);
}

// TG3000_ROUTINE($80:AF36) shop_level_blocks
bool shop_level_blocks(void) {
  wr16(DP + 0x80, 0);
  uint16_t y = rd16(DP + 0x74);
  wr16(DP + 0x7E, 0);
  uint16_t a = (uint16_t)(rd16(DP + 0x3E) >> 1);
  if(a >= 0x0010) {
    wr16(DP + 0x7E, (uint16_t)(a - 0x0010));
    for(;;) {
      if(rd16(DP + 0x7E) < 6) break;
      wr16(DP + 0x7E, (uint16_t)(rd16(DP + 0x7E) - 6));
      wr16(DP + 0x80, (uint16_t)(rd16(DP + 0x80) + 6));
      if(rd16(DP + 0x80) != 0x002A) continue;
      // all seven full: the last is drawn half a block over ($80:AF5E; the password movie
      // runs it); with three or more left over, nothing is drawn (a JMP to the RTS)
      wr16(DP + 0x80, (uint16_t)(rd16(DP + 0x80) - 3));
      if(rd16(DP + 0x7E) >= 3) return true;
      wr16(DP + 0x7E, (uint16_t)(rd16(DP + 0x7E) + 3));
      break;
    }
  }
  uint16_t pos;
  do {
    wr16(DP + 0x82, (uint16_t)(((uint16_t)(0x62 - rd16(DP + 0x7A)) << 8) + rd16(DP + 0xE7)));
    const uint32_t s = y;
    uint16_t at = (uint16_t)(rd16(DP + 0x14) + rd16(DP + 0x80) + 0x000A);
    wr16(0x0B00 + s, at);
    wr16(0x0B04 + s, at);
    uint16_t word = (uint16_t)((at & 0x00FF) | rd16(DP + 0x82));
    wr16(0x0900 + s, word);
    wr16(0x0904 + s, (uint16_t)(word + 0x0800));
    uint16_t tile = (uint16_t)((rd16(DP + 0x7E) << 1) + rd16(DP + 0x84));
    wr16(0x0902 + s, tile);
    wr16(0x0906 + s, (uint16_t)(tile + 1));
    wr16(0x0B02 + s, 0);
    wr16(0x0B06 + s, 0);
    y = (uint16_t)(y + 8);
    wr16(DP + 0x7E, 0);
    pos = (uint16_t)(rd16(DP + 0x80) + 6);
    wr16(DP + 0x80, pos);
    if(pos == 0x002A) {
      pos = 0x0027;
      wr16(DP + 0x80, pos);
      wr16(DP + 0x7E, 3);
    }
  } while(pos != 0x002D);
  wr16(DP + 0x74, y);
  return false;
}

// TG3000_ROUTINE($80:ADD9) shop_gauge_line
void shop_gauge_line(uint8_t speed, uint16_t y, uint16_t string) {
  uint8_t high = (uint8_t)(rd8(DP + 0x47) & 0x7F);
  int16_t m7a = (int16_t)((uint16_t)(high << 8) | rd8(DP + 0x46));
  video_set_m7a(m7a);
  video_set_m7b((int16_t)((uint16_t)(speed << 8) | high));
  uint16_t product = (uint16_t)((int32_t)m7a * (int8_t)speed);
  wr16(DP + 0x80, (uint16_t)((((uint16_t)(y + product) >> 5) & 0x007F) - 0x0020));
  uint32_t at = string;
  uint16_t slot = rd16(DP + 0x74);
  uint16_t tile = rd16(at);
  do {
    const uint32_t x = slot;
    wr16(0x0902 + x, tile);
    uint16_t col = (uint16_t)(rd16(DP + 0x80) + rd16(DP + 0x14));
    if(col & 0xFF00) col |= 0xFF00;
    wr16(0x0B00 + x, col);
    wr16(DP + 0x1A, (uint16_t)(col & 0x00FF));
    wr16(0x0900 + x, (uint16_t)((uint16_t)((rd16(DP + 0x16) + rd16(DP + 0x7E)) << 8) | rd16(DP + 0x1A)));
    wr16(0x0B02 + x, 0);
    wr16(DP + 0x80, (uint16_t)(rd16(DP + 0x80) + 8));
    if(!((uint16_t)(rd16(DP + 0x80) - 0x0048) & 0x8000)) break;
    slot = (uint16_t)(slot + 4);
    at = (uint16_t)(at + 2);
    tile = rd16(at);
  } while(tile != 0);
  wr16(DP + 0x74, slot);
}

// TG3000_ROUTINE($80:ACB6) shop_gauge_lines
void shop_gauge_lines(void) {
  wr16(DP + 0x16, shop_view_x(0x50));
  uint16_t slot = rd16(DP + 0x74);
  wr16(DP + 0x7E, 0);
  do {
    const uint32_t x = slot;
    wr16(0x0902 + x, 0x032F);
    uint16_t col = (uint16_t)(0x0040 + rd16(DP + 0x14));
    if(col & 0xFF00) col |= 0xFF00;  // $80:ACD6, never run
    wr16(0x0B00 + x, col);
    wr16(DP + 0x1A, (uint16_t)(col & 0x00FF));
    wr16(0x0900 + x, (uint16_t)((uint16_t)((rd16(DP + 0x16) + rd16(DP + 0x7E)) << 8) | rd16(DP + 0x1A)));
    wr16(0x0B02 + x, 0);
    slot = (uint16_t)(slot + 4);
    wr16(DP + 0x7E, (uint16_t)(rd16(DP + 0x7E) + 8));
  } while(rd16(DP + 0x7E) != 0x0020);
  wr16(DP + 0x74, slot);
  static const struct { uint16_t row, speed, string; } lines[12] = {
    {0x00, 0x40, 0x44BA}, {0x05, 0x30, 0x44BA}, {0x10, 0x1E, 0x44C0}, {0x1C, 0x20, 0x44C4},
    {0x0A, 0x16, 0x44CA}, {0x14, 0x4E, 0x44CE}, {0x12, 0x0E, 0x44DA}, {0x0C, 0x1B, 0x44E0},
    {0x08, 0x14, 0x44CA}, {0x0F, 0x5F, 0x44CE}, {0x03, 0x13, 0x44DA}, {0x1D, 0x2F, 0x44E0},
  };
  for(int i = 0; i < 12; i++) {
    wr16(DP + 0x7E, lines[i].row);
    shop_gauge_line((uint8_t)lines[i].speed, 0x0020, lines[i].string);
  }
}

// TG3000_ROUTINE($80:AC37) shop_item_view_j
void shop_item_view_j(uint16_t entry) {
  wr16(DP + 0x7A, rd16(DP + 0x16));
  wr16(DP + 0x7C, entry);
  shop_item_group(rd16(DP + 0x7C), 0, rd16(DP + 0x7A));
  uint16_t frame = (uint16_t)((rd16(DP + 0x40) >> 8) & 0x000F);
  shop_view_group(shop_view_x(0x50), rom16(0x81BD06 + (uint32_t)(uint16_t)(frame << 1)));
  shop_gauge_lines();
  const uint32_t y = (uint16_t)(rd16(DP + 0xC4) + 0x0110);
  const uint32_t x = (uint16_t)(((rd16(DP + 0x40) >> 8) << 1) & 0x0006);
  wr16(y + 0, rd16(0x4F5A + x));
  wr16(y + 2, rd16(0x4F5C + x));
  wr16(y + 4, rd16(0x4F5E + x));
  wr16(y + 6, rd16(0x4F60 + x));
  wr16(DP + 0x40, (uint16_t)(rd16(DP + 0x40) + 0x0040));
  wr16(DP + 0x46, (uint16_t)(rd16(DP + 0x46) - 1));
  shop_item_stage_step();
}

// TG3000_ROUTINE($80:AE4E) shop_item_view_i
void shop_item_view_i(uint16_t entry) {
  wr16(DP + 0x7A, rd16(DP + 0x16));
  wr16(DP + 0x7C, entry);
  shop_item_group(rd16(DP + 0x7C), 0, rd16(DP + 0x7A));
  shop_view_group(shop_view_x(0x50), 0x4686);
  uint16_t frame = (uint16_t)((rd16(DP + 0x40) >> 8) & 0x00FF);
  shop_view_group(shop_view_x(0x50), rom16(0x81BD26 + (uint32_t)(uint16_t)(frame << 1)));
  shop_view_spin(0x0F00);
  shop_item_stage_step();
}

// TG3000_ROUTINE($80:AEBE) shop_item_view_h
void shop_item_view_h(uint16_t entry) {
  wr16(DP + 0x7A, rd16(DP + 0x16));
  wr16(DP + 0x7C, entry);
  shop_item_group(rd16(DP + 0x7C), 0, rd16(DP + 0x7A));
  wr16(DP + 0x84, 0x257B);
  (void)shop_level_blocks();
  uint16_t frame = (uint16_t)((rd16(DP + 0x40) >> 8) & 0x00FF);
  shop_view_group(shop_view_x(0x50), rom16(0x81BD44 + (uint32_t)(uint16_t)(frame << 1)));
  shop_view_group(shop_view_x(0x50), 0x46BA);
  shop_view_spin(0x1E00);
  shop_item_stage_step();
}

// TG3000_ROUTINE($80:A251) shop_item_dispatch_view
void shop_item_dispatch_view(void) {
  uint16_t entry = shop_item_entry(rd16(DP + 0x4A));
  uint16_t bars = rom16(0x81A8A1 + (uint32_t)entry);
  switch(rom16(0x81A897 + (uint32_t)entry)) {
  case 0xA2CB: shop_item_view_a(bars); break;
  case 0xA333: shop_item_view_f(bars); break;
  case 0xA717: shop_item_view_d(bars); break;
  case 0xA831: shop_item_view_b(bars); break;
  case 0xAA15: shop_item_view_c(bars); break;
  case 0xAB54: shop_item_view_e(bars); break;
  case 0xABF6: shop_item_view_g(bars); break;
  case 0xAEBE: shop_item_view_h(bars); break;
  case 0xAE4E: shop_item_view_i(bars); break;
  case 0xAC37: shop_item_view_j(bars); break;
  default: break;  // the table holds no other view for lists 0-8
  }
}

// One picture's upload: VRAM D+$5C + `vram`, the pair at $81:A892 + `pictures` + `pair`.
static void shop_panel_picture(uint16_t pictures, uint16_t pair, uint16_t vram) {
  wr16(DP + 0x14, 0x007F);
  const uint32_t at = 0x81A892 + (uint32_t)(uint16_t)(pictures + pair);
  func_80B537(DP, rom16(at + 2), (uint16_t)(rom16(at) + rd16(DP + 0x5E) + 0x0900),
              (uint16_t)(rd16(DP + 0x5C) + vram));
}

// TG3000_ROUTINE($80:9C28) shop_item_dispatch_panel
void shop_item_dispatch_panel(void) {
  uint16_t entry = shop_item_entry(rd16(DP + 0x4A));
  uint16_t pictures = rom16(0x81A89B + (uint32_t)entry);
  switch(rom16(0x81A899 + (uint32_t)entry)) {
  // (handler: the uploads in order, as (VRAM offset, pair))
  case 0x9C47: shop_panel_picture(pictures, 0, 0x11E0); shop_panel_picture(pictures, 4, 0x1780); break;  // list 1
  case 0x9C85: shop_panel_picture(pictures, 4, 0x1120); shop_panel_picture(pictures, 0, 0x1260); break;  // list 2
  case 0x9CC3: shop_panel_picture(pictures, 4, 0x1120); shop_panel_picture(pictures, 0, 0x1240); break;  // list 3
  case 0x9D02: shop_panel_picture(pictures, 4, 0x1140); shop_panel_picture(pictures, 0, 0x1340); break;  // list 5
  case 0x9D41: shop_panel_picture(pictures, 4, 0x1400); shop_panel_picture(pictures, 0, 0x1560); break;  // list 4
  case 0x9D80: shop_panel_picture(pictures, 0, 0x1000); break;                                          // list 0
  case 0x9D9E: shop_panel_picture(pictures, 0, 0x1140); break;                                          // lists 6-8
  default: break;  // the table holds no other handler for lists 0-8
  }
}

// TG3000_ROUTINE($80:9847) shop_item_redraw
void shop_item_redraw(void) {
  if(rd16(DP + 0x8C) == rd16(DP + 0x8E)) {
    text_type_step();
    shop_panel_upload();
  }
  wr16(DP + 0x14, 0x0010);
  wr16(DP + 0x16, 0x0018);
  shop_item_price();
  shop_item_dispatch_view();
  shop_item_dispatch_icon();
  shop_level_marks();
  func_809DD0();
  oam_finish_near(DP);
}

// one frame of a slide: the frame's wait, the typewriter and panel when the queue is empty
static void shop_slide_wait(void) {
  wait_frames(1);
  if(rd16(DP + 0x8C) == rd16(DP + 0x8E)) {
    text_type_step();
    shop_panel_upload();
  }
}

// the item drawn at (D+$14, D+$16)
static void shop_slide_draw(void) {
  shop_item_dispatch_view();
  shop_item_dispatch_icon();
  shop_level_marks();
  func_809DD0();
  oam_finish_near(DP);
}

// the new item: wait for the queue to drain (to the slot it had), then its panel and text
static void shop_slide_swap(void) {
  uint16_t slot = rd16(DP + 0x8C);
  while(rd16(DP + 0x8E) != slot) platform_wait_vblank();
  text_type_step();
  shop_panel_upload();
  wr16(DP + 0x3E, 0);
  wr16(DP + 0x40, 0);
  wr16(DP + 0x42, 0);
  wr16(DP + 0x4A, rd16(DP + 0x4C));
  shop_item_dispatch_panel();
  shop_item_text();
}

// TG3000_ROUTINE($80:9AC8) shop_item_slide_row
void shop_item_slide_row(void) {
  wr16(DP + 0x4E, 0x0018);
  do {
    shop_slide_wait();
    wr16(DP + 0x14, 0x0010);
    wr16(DP + 0x4E, (uint16_t)(rd16(DP + 0x4E) - 6));
    wr16(DP + 0x16, rd16(DP + 0x4E));
    shop_slide_draw();
  } while(!((uint16_t)(rd16(DP + 0x4E) - 0xFFC0) & 0x8000));
  shop_slide_swap();
  wr16(DP + 0x4E, 0xFFC2);
  do {
    shop_slide_wait();
    wr16(DP + 0x4E, (uint16_t)(rd16(DP + 0x4E) + 6));
    wr16(DP + 0x14, rd16(DP + 0x4E));
    wr16(DP + 0x16, 0x0018);
    shop_slide_draw();
  } while(rd16(DP + 0x4E) != 0x0010);
}

// TG3000_ROUTINE($80:9B99) shop_item_slide_list_in
void shop_item_slide_list_in(void) {
  shop_slide_swap();
  wr16(DP + 0x4E, 0xFFCA);
  do {
    shop_slide_wait();
    wr16(DP + 0x14, 0x0010);
    wr16(DP + 0x4E, (uint16_t)(rd16(DP + 0x4E) + 6));
    wr16(DP + 0x16, rd16(DP + 0x4E));
    shop_slide_draw();
  } while(rd16(DP + 0x4E) != 0x0018);
}

// TG3000_ROUTINE($80:9B5D) shop_item_slide_list
void shop_item_slide_list(void) {
  wr16(DP + 0x4E, 0x0010);
  do {
    shop_slide_wait();
    wr16(DP + 0x4E, (uint16_t)(rd16(DP + 0x4E) - 6));
    wr16(DP + 0x14, rd16(DP + 0x4E));
    wr16(DP + 0x16, 0x0018);
    shop_slide_draw();
  } while(rd16(DP + 0x4E) != 0xFFC2);
  shop_item_slide_list_in();
}

// leaving the list: returns true (the original drops its return address and returns from
// its caller)
static bool shop_item_leave(void) {
  wr16(DP + 0x44, (uint16_t)(rd16(DP + 0x44) | 0x8000));
  shop_panel_blank();
  while(rd16(DP + 0x4E) <= 0x0004) platform_wait_vblank();
  shop_panel_clear();
  return true;
}

// TG3000_ROUTINE($80:8FA1) shop_item_down
bool shop_item_down(void) {
  uint16_t last = (uint16_t)((rd16(0x036E + (uint32_t)rd16(DP + 0x48)) & 0x00FF) - 1);
  if(last == rd16(DP + 0x4A)) return shop_item_leave();
  func_80E4C5();
  wr16(DP + 0x4C, (uint16_t)(rd16(DP + 0x4A) + 1));
  shop_item_picture();
  shop_item_slide_list();
  return false;
}

// TG3000_ROUTINE($80:8FD4) shop_item_up
bool shop_item_up(void) {
  func_80E4C5();
  if(rd16(DP + 0x4A) == 0) return shop_item_leave();
  wr16(DP + 0x4C, (uint16_t)(rd16(DP + 0x4A) - 1));
  shop_item_picture();
  shop_item_slide_row();
  return false;
}

// TG3000_ROUTINE($80:9832) shop_item_show_icon
void shop_item_show_icon(void) {
  wr16(DP + 0x76, (uint16_t)(rd16(DP + 0x76) | 0x8000));
  wr16(DP + 0x44, (uint16_t)(rd16(DP + 0x44) & ~0x8000));
  do {
    wait_frames(1);
    shop_item_redraw();
  } while(!(rd16(DP + 0x44) & 0x8000));
}

// TG3000_ROUTINE($80:98C4) shop_item_affordable
//
// The original subtracts in decimal mode (CMP, then SBC with D set): for BCD amounts its
// carry is the 32-bit comparison.
bool shop_item_affordable(void) {
  uint16_t entry = shop_item_entry(rd16(DP + 0x4A));
  const uint32_t price = 0x81A892 + (uint32_t)rom16(0x81A89B + (uint32_t)entry);
  uint32_t money = (uint32_t)rd16(DP + 0x54) << 16 | rd16(DP + 0x52);
  uint32_t cost = (uint32_t)rom16(price - 4) << 16 | rom16(price - 2);
  if(money >= cost) return true;
  for(uint32_t x = 0; x < 0x20; x += 2) {
    wait_frames(1);
    const uint32_t y = rd16(DP + 0xC4);
    wr16(0x01C2 + y, rd16(0x4DDA + x));
    wr16(0x01C4 + y, rd16(0x4DFA + x));
    shop_item_redraw();
  }
  return false;
}

// TG3000_ROUTINE($80:9870) shop_item_icon_settle
void shop_item_icon_settle(void) {
  uint16_t step;
  do {
    wait_frames(1);
    shop_item_redraw();
    const uint32_t x = (uint16_t)(rd16(DP + 0x44) << 1), y = rd16(DP + 0xC4);
    wr16(0x01EE + y, rd16(0x4EDA + x));
    wr16(0x01F0 + y, rd16(0x4EFA + x));
    if(x < 0x0010) {  // never run: the icon's other colours, as shop_item_icon_draw's
      wr16(0x0182 + y, rd16(0x4F1A + x));
      wr16(0x0186 + y, rd16(0x4F2A + x));
      wr16(0x0188 + y, rd16(0x4F3A + x));
      wr16(0x018E + y, rd16(0x4F4A + x));
    }
    step = rd16(DP + 0x44);
    if(step == 0x8007) step++;  // never run: it holds there
    step--;
    wr16(DP + 0x44, step);
  } while(step != 0x8007);
  wr16(DP + 0x76, (uint16_t)(rd16(DP + 0x76) & ~0x8000));
}

// TG3000_ROUTINE($80:B2F2) shop_list_label
uint16_t shop_list_label(uint16_t x) {
  int16_t row = (int16_t)rd16(DP + 0x22);
  if((int16_t)(row - 0x1000) >= 0 && (int16_t)(row - 0x5800) < 0)
    sprite_text_centered(0x800000 | rom16(0x80B371 + (uint32_t)x), (uint16_t)(rd16(DP + 0x22) + rd16(DP + 0xE7)));
  wr16(DP + 0x22, (uint16_t)(rd16(DP + 0x22) + 0x0800));
  x = (uint16_t)(x + 2);
  if(x == 0x0020) return x;
  if(rd8(0x036E + (uint32_t)(x >> 1)) != 0) return x;
  wr16(DP + 0xF0, (uint16_t)(x >> 1));
  return 0x001E;
}
