// SPDX-License-Identifier: GPL-3.0-or-later
#include "shop.h"

#include <stdbool.h>
#include <stddef.h>

#include "decompress.h"
#include "frame.h"
#include "misc.h"
#include "platform.h"
#include "rng.h"
#include "sprites.h"
#include "text.h"
#include "tg3000.h"
#include "video_dma.h"

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

// TG3000_ROUTINE($80:9F30) shop_part_level_8
uint16_t shop_part_level_8(uint16_t car) { return rd16((uint32_t)(0x0398 + car)); }
// TG3000_ROUTINE($80:9F2C) shop_part_level_9
uint16_t shop_part_level_9(uint16_t car) { return rd16((uint32_t)(0x0399 + car)); }
// TG3000_ROUTINE($80:9F28) shop_part_level_a
uint16_t shop_part_level_a(uint16_t car) { return rd16((uint32_t)(0x039A + car)); }
// TG3000_ROUTINE($80:9F38) shop_part_level_b
uint16_t shop_part_level_b(uint16_t car) { return rd16((uint32_t)(0x039B + car)); }
// TG3000_ROUTINE($80:9F34) shop_part_level_c
uint16_t shop_part_level_c(uint16_t car) { return rd16((uint32_t)(0x039C + car)); }

// TG3000_ROUTINE($80:993E) shop_money_take
void shop_money_take(uint16_t dp) {
  uint16_t car = rd16(dp + 0x94);
  wr16(dp + 0x52, rd16((uint32_t)(0x038E + car)));
  wr16(dp + 0x54, rd16((uint32_t)(0x0390 + car)));
  wr16(dp + 0x56, 0xFFFF);
  wr16(dp + 0x76, (uint16_t)(rd16(dp + 0x76) | 0x0004));
}

static uint16_t shop_item_entry(uint16_t dp, uint16_t row);

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
uint16_t shop_money_after_price(uint16_t dp) {
  ShopMoneyFlags f;
  return shop_money_after_price_flags(dp, &f);
}

static uint16_t shop_money_settle(uint16_t dp, ShopMoneyFlags* f);

uint16_t shop_money_after_price_flags(uint16_t dp, ShopMoneyFlags* f) {
  const uint16_t price = rom16(0x81A89B + (uint32_t)shop_item_entry(dp, rd16(dp + 0x4A)));
  const uint32_t car = rd16(dp + 0x94);
  const uint16_t lo = rd16(0x038E + car), hi = rd16(0x0390 + car);
  bool c = true;
  wr16(dp + 0x52, bcd_sub16(lo, rom16(0x81A890 + (uint32_t)price), &c));
  wr16(dp + 0x54, bcd_sub16(hi, rom16(0x81A88E + (uint32_t)price), &c));
  return shop_money_settle(dp, f);
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
uint16_t shop_money_after_refund(uint16_t dp, uint16_t row) {
  ShopMoneyFlags f;
  return shop_money_after_refund_flags(dp, row, &f);
}
uint16_t shop_money_after_refund_flags(uint16_t dp, uint16_t row, ShopMoneyFlags* f) {
  wr16(dp + 0x7A, row);
  const uint16_t price = rom16(0x81A89B + (uint32_t)shop_item_entry(dp, row));
  const uint32_t car = rd16(dp + 0x94);
  bool c = false;
  wr16(dp + 0x52, bcd_add16(rd16(0x038E + car), rom16(0x81A890 + (uint32_t)price), &c));
  wr16(dp + 0x54, bcd_add16(rd16(0x0390 + car), rom16(0x81A88E + (uint32_t)price), &c));
  wr16(dp + 0x76, (uint16_t)(rd16(dp + 0x76) & ~0x0004));
  return shop_money_settle(dp, f);
}

// $80:997E on: the digits that settle, from the money (D+$94's) and D+$52/D+$54.
static uint16_t shop_money_settle(uint16_t dp, ShopMoneyFlags* f) {
  const uint32_t car = rd16(dp + 0x94);
  const uint16_t lo = rd16(0x038E + car), hi = rd16(0x0390 + car);
  bool c = false;
  wr16(dp + 0x56, 0);
  wr16(dp + 0x58, 0);
  wr16(dp + 0x7A, (uint16_t)(lo | rd16(dp + 0x52)));
  wr16(dp + 0x7C, (uint16_t)(hi | rd16(dp + 0x54)));
  // From the top, the digits that neither number has, or that stay the same, settle (D+$58).
  // The mask tested each time is every digit from the top down to the current one (ORA of
  // what settled before the shift).
  uint16_t y = 4, a = 0xF000;
  f->v = (rd16(dp + 0x7C) >> 14) & 1;  // BIT $7C's V
  for(; y != 0; y--) {
    if(a & rd16(dp + 0x7C)) {
      wr16(dp + 0x7E, a);
      wr16(dp + 0x80, (uint16_t)(a & hi));
      const uint16_t mine = (uint16_t)(a & rd16(dp + 0x54));
      c = mine >= rd16(dp + 0x80);  // CMP
      if(mine != rd16(dp + 0x80)) goto done;
    }
    a = (uint16_t)(a | rd16(dp + 0x58));
    wr16(dp + 0x58, a);
    c = (a >> 3) & 1;  // the last of four LSRs
    a = (uint16_t)(a >> 4);
  }
  // all four high digits settled: the low word's leading zeros the same way (D+$56); no
  // run takes this path
  f->v = (rd16(dp + 0x7A) >> 14) & 1;  // BIT $7A's V
  for(y = 4, a = 0xF000; y != 0; y--) {
    if(a & rd16(dp + 0x7A)) goto done;
    a = (uint16_t)(a | rd16(dp + 0x56));
    wr16(dp + 0x56, a);
    c = (a >> 3) & 1;
    a = (uint16_t)(a >> 4);
  }
done:
  wr16(dp + 0x5A, 4);
  f->c = c;
  return y;
}

// The digit mask D+$7C:D+$7A one digit lower (LSR $7C / ROR $7A, four times); the carry
// the last ROR leaves.
static bool shop_money_mask_down(uint16_t dp) {
  const uint32_t m = (uint32_t)rd16(dp + 0x7A) | (uint32_t)rd16(dp + 0x7C) << 16;
  wr16(dp + 0x7A, (uint16_t)(m >> 4));
  wr16(dp + 0x7C, (uint16_t)(m >> 20));
  return (m >> 3) & 1;
}

// TG3000_ROUTINE($80:99D1) shop_money_roll
uint16_t shop_money_roll(uint16_t dp) {
  ShopMoneyFlags f = {false, false};
  return shop_money_roll_flags(dp, &f);
}

uint16_t shop_money_roll_flags(uint16_t dp, ShopMoneyFlags* f) {
  wr16(dp + 0x7A, 0);
  wr16(dp + 0x7C, 0xF000);
  wr16(dp + 0x84, 8);
  const uint32_t car = rd16(dp + 0x94);
  uint16_t a;
  // the digits already settled (D+$56/D+$58), from the top, are passed over
  for(;;) {
    wr16(dp + 0x7E, (uint16_t)((rd16(dp + 0x56) & rd16(dp + 0x7A)) | (rd16(dp + 0x58) & rd16(dp + 0x7C))));
    a = (uint16_t)(rd16(dp + 0x7A) | rd16(dp + 0x7C));
    if(a != rd16(dp + 0x7E)) break;
    f->c = shop_money_mask_down(dp);
    wr16(dp + 0x84, (uint16_t)(rd16(dp + 0x84) - 1));
    if(rd16(dp + 0x84) == 0) return a;
  }
  wr16(dp + 0x5A, (uint16_t)(rd16(dp + 0x5A) - 1));
  if(rd16(dp + 0x5A) == 0) {  // the first unsettled digit takes its new value and settles
    wr16(0x038E + car, (uint16_t)((rd16(0x038E + car) & ~rd16(dp + 0x7A)) | (rd16(dp + 0x52) & rd16(dp + 0x7A))));
    wr16(0x0390 + car, (uint16_t)((rd16(0x0390 + car) & ~rd16(dp + 0x7C)) | (rd16(dp + 0x54) & rd16(dp + 0x7C))));
    wr16(dp + 0x7E, (uint16_t)(rd16(dp + 0x52) & rd16(dp + 0x7A)));
    wr16(dp + 0x80, (uint16_t)(rd16(dp + 0x54) & rd16(dp + 0x7C)));
    wr16(dp + 0x56, (uint16_t)(rd16(dp + 0x56) | rd16(dp + 0x7A)));
    wr16(dp + 0x58, (uint16_t)(rd16(dp + 0x58) | rd16(dp + 0x7C)));
    wr16(dp + 0x5A, 4);
    a = 4;
    goto next;
  }
  for(;;) {  // this digit and every one below it roll down by one (0 to 9)
    {
      const uint16_t m1 = rd16(dp + 0x7A), m2 = rd16(dp + 0x7C);
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
      wr16(dp + 0x7E, lo);
      wr16(dp + 0x80, hi);
      wr16(0x038E + car, (uint16_t)((rd16(0x038E + car) & ~m1) | lo));
      a = (uint16_t)((rd16(0x0390 + car) & ~m2) | hi);
      wr16(0x0390 + car, a);
    }
  next:
    f->c = shop_money_mask_down(dp);
    wr16(dp + 0x84, (uint16_t)(rd16(dp + 0x84) - 1));
    if(rd16(dp + 0x84) == 0) return a;
  }
}

// The buying's shared steps (the ROM repeats them in each list's handler). The panel rises:
// D+$42 up by 6 a frame to `height`, the money rolling, the screen redrawn.
static void shop_panel_rise(uint16_t dp, uint16_t height) {
  wr16(dp + 0x76, (uint16_t)(rd16(dp + 0x76) | 0x0002));
  shop_item_show_icon(dp);
  do {
    wait_frames(1);
    wr16(dp + 0x42, (uint16_t)(rd16(dp + 0x42) + 6));
    shop_money_roll(dp);
    shop_item_redraw(dp);
  } while(rd16(dp + 0x42) != height);
}
// The purchase made: the level byte at `at` = `level`, the upload queued, the money after the
// price, the panel falls (down to 0), the money stored, then (`settle`) the icon settled.
static void shop_buy_finish(uint16_t dp, uint32_t at, uint8_t level, bool settle) {
  wr8(at, level);
  shop_item_dispatch_upload(dp);
  wr16(dp + 0x76, (uint16_t)(rd16(dp + 0x76) & ~0x0002));
  shop_money_after_price(dp);
  do {
    wait_frames(1);
    wr16(dp + 0x42, (uint16_t)(rd16(dp + 0x42) - 6));
    shop_money_roll(dp);
    shop_item_redraw(dp);
  } while(rd16(dp + 0x42) != 0);
  shop_money_store(dp);
  if(settle) shop_item_icon_settle(dp);
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
ShopBuyEnd shop_buy_part(uint16_t dp, uint16_t field, uint16_t height, bool price_first, bool settle, ShopMoneyFlags* f) {
  ShopMoneyFlags unused;
  if(f == NULL) f = &unused;
  const uint32_t at = 0x0393 + (uint32_t)field + rd16(dp + 0x94);
  if((rd16(at) & 0x007F) == rd16(dp + 0x4A)) {
    func_80E4C5();
    func_80B58D(0x80, 0xB535);
    shop_item_show_icon(dp);
    if(settle) shop_item_icon_settle(dp);
    return SHOP_BUY_OWNED;
  }
  shop_money_take(dp);
  if(rd16(at) & 0x0080) {  // a part bought here before is sold back first
    shop_money_after_refund(dp, (uint16_t)(rd16(at) & 0x007F));
    if(!shop_item_affordable(dp)) {
      if(price_first) {
        (void)shop_money_after_price_flags(dp, f);
        shop_flag_fb(dp);
      } else {
        shop_flag_fb(dp);
        (void)shop_money_after_price_flags(dp, f);
      }
      return SHOP_BUY_REFUNDED_SHORT;
    }
  } else if(!shop_item_affordable(dp)) {
    shop_flag_fb(dp);
    return SHOP_BUY_SHORT;
  }
  func_80E4DB();
  func_80B58D(0x80, 0xB535);
  shop_panel_rise(dp, height);
  shop_buy_finish(dp, at, (uint8_t)(rd8(dp + 0x4A) | 0x80), settle);
  return SHOP_BUY_BOUGHT;
}

// TG3000_ROUTINE($80:9053) shop_buy_list1
void shop_buy_list1(uint16_t dp) { (void)shop_buy_part(dp, 0, 0x0078, true, true, NULL); }
// TG3000_ROUTINE($80:9100) shop_buy_list2
void shop_buy_list2(uint16_t dp) { (void)shop_buy_part(dp, 1, 0x00B4, false, true, NULL); }
// TG3000_ROUTINE($80:91AE) shop_buy_list3
void shop_buy_list3(uint16_t dp) { (void)shop_buy_part(dp, 2, 0x00B4, false, true, NULL); }
// TG3000_ROUTINE($80:925C) shop_buy_list4
void shop_buy_list4(uint16_t dp) { (void)shop_buy_part(dp, 3, 0x00B4, false, true, NULL); }
// TG3000_ROUTINE($80:95C0) shop_buy_list5
void shop_buy_list5(uint16_t dp) { (void)shop_buy_part(dp, 4, 0x00B4, false, true, NULL); }

// Lists 6-10's buying (shop_buy_upgrade): as shop_buy_part's, with `field` 5-9 and the level
// byte the row + 1.
ShopBuyEnd shop_buy_upgrade(uint16_t dp, uint16_t field, ShopMoneyFlags* f) {
  ShopMoneyFlags unused;
  if(f == NULL) f = &unused;
  const uint32_t at = 0x0393 + (uint32_t)field + rd16(dp + 0x94);
  if((uint16_t)((rd16(at) & 0x007F) - 1) == rd16(dp + 0x4A)) {
    func_80E4C5();
    func_80B58D(0x80, 0xB535);
    shop_item_show_icon(dp);
    shop_item_icon_settle(dp);
    return SHOP_BUY_OWNED;
  }
  shop_money_take(dp);
  const uint16_t level = (uint16_t)(rd16(at) & 0x00FF);
  if(level == 0) {  // the first purchase: the panel is up at once
    if(!shop_item_affordable(dp)) {
      shop_flag_fb(dp);
      return SHOP_BUY_SHORT;
    }
    func_80E4DB();
    func_80B58D(0x80, 0xB535);
    wr16(dp + 0x42, 0x00B4);
    wr16(dp + 0x44, 0x000F);
    func_80B58D(0x80, 0xB535);
    shop_item_show_icon(dp);
  } else {
    const uint16_t before = (uint16_t)(level - 1);
    if(before & 0x0080) {  // bought here before: sold back first
      shop_money_after_refund(dp, (uint16_t)(before & 0x007F));
      if(!shop_item_affordable(dp)) {
        shop_flag_fb(dp);
        (void)shop_money_after_price_flags(dp, f);
        return SHOP_BUY_REFUNDED_SHORT;
      }
    } else if(!shop_item_affordable(dp)) {
      shop_flag_fb(dp);
      return SHOP_BUY_SHORT;
    }
    func_80E4DB();
    func_80B58D(0x80, 0xB535);
    shop_panel_rise(dp, 0x00B4);
  }
  shop_buy_finish(dp, at, (uint8_t)((uint8_t)(rd8(dp + 0x4A) + 1) | 0x80), true);
  return SHOP_BUY_BOUGHT;
}
// TG3000_ROUTINE($80:94DE) shop_buy_list6
void shop_buy_list6(uint16_t dp) { (void)shop_buy_upgrade(dp, 5, NULL); }
// TG3000_ROUTINE($80:93FC) shop_buy_list7
void shop_buy_list7(uint16_t dp) { (void)shop_buy_upgrade(dp, 6, NULL); }
// TG3000_ROUTINE($80:931A) shop_buy_list8
void shop_buy_list8(uint16_t dp) { (void)shop_buy_upgrade(dp, 7, NULL); }

// TG3000_ROUTINE($80:966E) shop_buy_list9
void shop_buy_list9(uint16_t dp) { (void)shop_buy_upgrade(dp, 9, NULL); }
// TG3000_ROUTINE($80:9750) shop_buy_list10 (never run)
void shop_buy_list10(uint16_t dp) { (void)shop_buy_upgrade(dp, 8, NULL); }

// The ROM pushes the return to its own RTS ($80:9052), then the handler's address - 1, and
// RTSes into it.
uint16_t shop_buy_handler(uint16_t dp) {
  const uint16_t entry = (uint16_t)((rd16(dp + 0x4A) << 4) + rom16(0x81A892 + (uint32_t)(uint16_t)(rd16(dp + 0x48) << 1)));
  return rom16(0x81A89F + (uint32_t)entry);
}
// TG3000_ROUTINE($80:9039) shop_buy_dispatch
void shop_buy_dispatch(uint16_t dp) {
  switch(shop_buy_handler(dp)) {
    case 0x930A: shop_buy_colour(dp); break;
    case 0x9053: shop_buy_list1(dp); break;
    case 0x9100: shop_buy_list2(dp); break;
    case 0x91AE: shop_buy_list3(dp); break;
    case 0x925C: shop_buy_list4(dp); break;
    case 0x95C0: shop_buy_list5(dp); break;
    case 0x94DE: shop_buy_list6(dp); break;
    case 0x93FC: shop_buy_list7(dp); break;
    case 0x931A: shop_buy_list8(dp); break;
    case 0x966E: shop_buy_list9(dp); break;
    case 0x9750: shop_buy_list10(dp); break;
    default: break;  // no other word in the table's rows: the ROM would jump to it
  }
}

// TG3000_ROUTINE($80:930A) shop_buy_colour
void shop_buy_colour(uint16_t dp) {
  wr8(0x0392 + (uint32_t)rd16(dp + 0x94), rd8(dp + 0x4A));
  func_80E4DB();
}

// TG3000_ROUTINE($80:9ABB) shop_money_store
void shop_money_store(uint16_t dp) {
  const uint32_t car = rd16(dp + 0x94);
  wr16(0x038E + car, rd16(dp + 0x52));
  wr16(0x0390 + car, rd16(dp + 0x54));
}

// TG3000_ROUTINE($80:E4D0) shop_flag_fb (long)
void shop_flag_fb(uint16_t dp) { wr8(dp + 0xFB, 0x12); }

// TG3000_ROUTINE($80:9DBC) shop_panel_upload
uint16_t shop_panel_upload(uint16_t dp) {
  wr16(dp + 0x14, 0x007F);
  return func_80B537(dp, 0x03C0, rd16(dp + 0x5E), (uint16_t)(rd16(dp + 0x5C) + 0x0400));
}

// The current item's table entry: list $48 (a word index into $81:A892), row $4C or $4A.
static uint16_t shop_item_entry(uint16_t dp, uint16_t row) {
  return (uint16_t)((row << 4) + rom16(0x81A892 + (uint32_t)(uint16_t)(rd16(dp + 0x48) << 1)));
}

// TG3000_ROUTINE($80:9BF2) shop_item_picture
void shop_item_picture(uint16_t dp) {
  uint16_t entry = shop_item_entry(dp, rd16(dp + 0x4C));
  func_80B58D(0x81, rom16(0x81A89D + (uint32_t)entry));
}

// TG3000_ROUTINE($80:B096) shop_item_text
void shop_item_text(uint16_t dp) {
  uint16_t text = rom16(0x81A895 + (uint32_t)shop_item_entry(dp, rd16(dp + 0x4A)));
  uint16_t size = rom16(0x81A894 + (uint32_t)text), src = rom16(0x81A892 + (uint32_t)text);
  uint16_t dest = (uint16_t)(rd16(dp + 0xC4) + 0x0100);
  for(uint32_t i = 0; i < (size ? size : 0x10000u); i++)  // MVN $89 -> $81: size bytes (0: 64 KB)
    bus_wr8(0x810000 | (uint16_t)(dest + i), bus_rd8(0x890000 | (uint16_t)(src + i)));
}

// TG3000_ROUTINE($80:A7E4) shop_sprite_block
uint16_t shop_sprite_block(uint16_t dp, uint16_t pos, uint16_t slot) {
  const uint32_t s = slot;
  for(uint32_t i = 0; i < 16; i += 4) wr16(0x0902 + s + i, pos);
  uint16_t attr = rd16(dp + 0x7E);
  for(uint32_t i = 0; i < 16; i += 4) wr16(0x0B00 + s + i, attr);
  uint16_t tile = (uint16_t)((attr & 0x00FF) | rd16(dp + 0x16));
  wr16(0x0900 + s, tile);
  wr16(0x0904 + s, (uint16_t)(tile + 0x0800));
  wr16(0x0908 + s, (uint16_t)(tile + 0x1000));
  wr16(0x090C + s, (uint16_t)(tile + 0x1400));
  for(uint32_t i = 0; i < 16; i += 4) wr16(0x0B02 + s + i, 0);
  return (uint16_t)(slot + 0x10);
}

// TG3000_ROUTINE($80:AAF1) shop_bar_cap
uint16_t shop_bar_cap(uint16_t dp) {
  const uint32_t x = rd16(dp + 0x74);
  uint16_t row = (uint16_t)(((0x51 - rd16(dp + 0x7A) + rd16(dp + 0xE8)) & 0xFF) << 8);
  wr16(dp + 0x16, row);
  uint16_t y = (uint16_t)(rd16(dp + 0x80) + rd16(dp + 0x14) - 1);
  for(uint32_t i = 0; i < 16; i += 4) wr16(0x0B00 + x + i, y);
  uint16_t t = (uint16_t)((y & 0x00FF) | row);
  for(uint32_t i = 0; i < 16; i += 4) wr16(0x0900 + x + i, (uint16_t)(t + 0x0800 * (i / 4)));
  uint16_t tile = rd16(dp + 0x7E);
  wr16(0x0902 + x, tile);
  wr16(0x0906 + x, tile);
  wr16(0x090A + x, tile);
  wr16(0x090E + x, (uint16_t)(tile + 1));
  for(uint32_t i = 0; i < 16; i += 4) wr16(0x0B02 + x + i, 0);
  uint16_t next = (uint16_t)(x + 0x10);
  wr16(dp + 0x74, next);
  return next;
}

// TG3000_ROUTINE($80:A9BD) shop_bar_column
uint16_t shop_bar_column(uint16_t dp) {
  const uint32_t y = rd16(dp + 0x74);
  uint16_t row = (uint16_t)((((0x6C - rd16(dp + 0x7A) - rd16(dp + 0x7E)) & 0xFF) << 8) + rd16(dp + 0xE7));
  wr16(dp + 0x82, row);
  uint16_t at = (uint16_t)(rd16(dp + 0x14) + rd16(dp + 0x80));
  for(uint32_t i = 0; i < 12; i += 4) wr16(0x0B00 + y + i, at);
  uint16_t t = (uint16_t)((at & 0x00FF) | row);
  for(uint32_t i = 0; i < 12; i += 4) wr16(0x0900 + y + i, (uint16_t)(t + 0x0800 * (i / 4)));
  uint16_t tile = rd16(dp + 0x84);
  wr16(0x0902 + y, tile);
  wr16(0x0906 + y, (uint16_t)(tile + 1));
  wr16(0x090A + y, (uint16_t)(tile + 1));
  for(uint32_t i = 0; i < 12; i += 4) wr16(0x0B02 + y + i, 0);
  uint16_t next = (uint16_t)(y + 0x0C);
  wr16(dp + 0x74, next);
  return next;
}

// The half stage past $10, divided by 6 into D+$80 (segments x 6, up to `full`) and D+$7E
// (the rest). Returns false when the bar is full (the original returns at once).
static bool shop_bar_split(uint16_t dp, uint16_t full) {
  wr16(dp + 0x80, 0);
  wr16(dp + 0x7E, 0);
  uint16_t d = (uint16_t)((rd16(dp + 0x3E) >> 1) - 0x10);
  if((rd16(dp + 0x3E) >> 1) < 0x10) return true;
  wr16(dp + 0x7E, d);
  for(;;) {
    if(rd16(dp + 0x7E) < 6) return true;
    wr16(dp + 0x7E, (uint16_t)(rd16(dp + 0x7E) - 6));
    wr16(dp + 0x80, (uint16_t)(rd16(dp + 0x80) + 6));
    if(rd16(dp + 0x80) == full) return false;
  }
}

// One 3-sprite column of a bar at slot `y`: row ((`top` - D+$7A) & $FF) << 8 + D+$E7 into
// D+$82, y position D+$14 + D+$80 + `dy`, tiles from D+$7E * 2 + D+$84 (the middle one
// flipped). Returns the next slot.
static uint16_t shop_bar_piece(uint16_t dp, uint16_t y, uint16_t top, uint16_t dy) {
  const uint32_t s = y;
  uint16_t row = (uint16_t)((((top - rd16(dp + 0x7A)) & 0xFF) << 8) + rd16(dp + 0xE7));
  wr16(dp + 0x82, row);
  uint16_t at = (uint16_t)(rd16(dp + 0x14) + rd16(dp + 0x80) + dy);
  for(uint32_t i = 0; i < 12; i += 4) wr16(0x0B00 + s + i, at);
  uint16_t t = (uint16_t)((at & 0x00FF) | row);
  for(uint32_t i = 0; i < 12; i += 4) wr16(0x0900 + s + i, (uint16_t)(t + 0x0800 * (i / 4)));
  uint16_t tile = (uint16_t)((rd16(dp + 0x7E) << 1) + rd16(dp + 0x84));
  wr16(0x0902 + s, tile);
  wr16(0x0906 + s, (uint16_t)(tile | 0x8000));
  wr16(0x090A + s, (uint16_t)(((tile | 0x8000) ^ 0x8000) + 1));
  for(uint32_t i = 0; i < 12; i += 4) wr16(0x0B02 + s + i, 0);
  return (uint16_t)(y + 0x0C);
}

// TG3000_ROUTINE($80:A4B8) shop_bar_segments
void shop_bar_segments(uint16_t dp) {
  uint16_t y = rd16(dp + 0x74);
  if(!shop_bar_split(dp, 0x1E)) {  // at $1E: back to the half segment $1B, with the rest less 3
    wr16(dp + 0x80, 0x1B);
    uint16_t rest = rd16(dp + 0x7E);
    if(rest >= 3) return;
    wr16(dp + 0x7E, (uint16_t)(rest + 3));
  }
  for(;;) {
    y = shop_bar_piece(dp, y, 0x5B, 0x0005);
    wr16(dp + 0x7E, 0);
    uint16_t at = (uint16_t)(rd16(dp + 0x80) + 6);
    wr16(dp + 0x80, at);
    if(at == 0x1E) {
      wr16(dp + 0x80, 0x1B);
      wr16(dp + 0x7E, 3);
      continue;
    }
    if(at == 0x21) break;
  }
  wr16(dp + 0x74, y);
}

// TG3000_ROUTINE($80:A87D) shop_bar_segments_long
void shop_bar_segments_long(uint16_t dp) {
  uint16_t y = rd16(dp + 0x74);
  if(!shop_bar_split(dp, 0x24)) return;
  do {
    y = shop_bar_piece(dp, y, 0x5A, 0x0003);
    wr16(dp + 0x7E, 0);
    wr16(dp + 0x80, (uint16_t)(rd16(dp + 0x80) + 6));
  } while(rd16(dp + 0x80) != 0x24);
  wr16(dp + 0x74, y);
}

// TG3000_ROUTINE($80:A403) shop_bar_levels
void shop_bar_levels(uint16_t dp) {
  uint16_t x = rom16(0x81A896 + (uint32_t)rd16(dp + 0x7C));
  wr16(dp + 0x80, 0);
  do {
    wr16(dp + 0x7E, 0);
    uint16_t level = (uint16_t)((rd16(dp + 0x3E) >> 1) - rom16(0x81A892 + (uint32_t)x));
    if(!(level & 0x8000)) {
      uint16_t length = rom16(0x81A894 + (uint32_t)x);
      wr16(dp + 0x7E, level < length ? level : length);
    }
    const uint32_t y = rd16(dp + 0x74);
    uint16_t row = (uint16_t)((((0x60 - rd16(dp + 0x7A) + rd16(dp + 0x7E)) & 0xFF) << 8) + rd16(dp + 0xE7));
    wr16(dp + 0x7E, row);
    uint16_t at = (uint16_t)(rd16(dp + 0x14) + rd16(dp + 0x80) + 0x2B);
    wr16(0x0B00 + y, at);
    wr16(0x0B04 + y, at);
    uint16_t t = (uint16_t)((at & 0x00FF) | row);
    wr16(0x0900 + y, t);
    wr16(0x0904 + y, (uint16_t)(t + 0x0800));
    wr16(0x0902 + y, 0x012C);
    wr16(0x0906 + y, 0x012C);
    wr16(0x0B02 + y, 0);
    wr16(0x0B06 + y, 0);
    wr16(dp + 0x74, (uint16_t)(y + 8));
    x = (uint16_t)(x + 4);
    wr16(dp + 0x80, (uint16_t)(rd16(dp + 0x80) + 3));
  } while(rd16(dp + 0x80) != 0x12);
  x = rom16(0x81A896 + (uint32_t)rd16(dp + 0x7C));
  wr16(dp + 0x80, 0);
  do {  // (the stage is read again each time: a tile word may land on it)
    uint16_t level = (uint16_t)((rd16(dp + 0x3E) >> 1) - rom16(0x81A892 + (uint32_t)x));
    if(!(level & 0x8000)) {
      if(level >= 8) level = 7;
      uint16_t tile = rom16(0x81A882 + (uint32_t)(level << 1));
      wr16((uint16_t)(rom16(0x81A876 + (uint32_t)rd16(dp + 0x80)) + rd16(dp + 0xC4)), tile);
    }
    x = (uint16_t)(x + 4);
    wr16(dp + 0x80, (uint16_t)(rd16(dp + 0x80) + 2));
  } while(rd16(dp + 0x80) != 0x0C);
}

// A part's level for the bar entry at `x` (+`part` x 4): the half stage less its start
// (0 when below), at most its length.
static uint16_t shop_part_level(uint16_t dp, uint16_t x, uint32_t part) {
  uint16_t half = (uint16_t)(rd16(dp + 0x3E) >> 1), start = rom16(0x81A892 + part * 4 + (uint32_t)x);
  uint16_t level = half < start ? 0 : (uint16_t)(half - start);
  uint16_t length = rom16(0x81A894 + part * 4 + (uint32_t)x);
  return level < length ? level : length;
}

// TG3000_ROUTINE($80:A91E) shop_bar_double
void shop_bar_double(uint16_t dp) {
  const uint32_t y = rd16(dp + 0x74);
  uint16_t row = (uint16_t)((((0x6C - rd16(dp + 0x7A)) & 0xFF) << 8) + rd16(dp + 0xE7));
  wr16(dp + 0x82, row);
  uint16_t top = (uint16_t)(rd16(dp + 0x14) + 0x2C), bottom = (uint16_t)(rd16(dp + 0x14) + 0x34);
  wr16(0x0B00 + y, top);
  wr16(0x0900 + y, (uint16_t)((top & 0x00FF) | row));
  wr16(0x0B04 + y, bottom);
  wr16(0x0904 + y, (uint16_t)((bottom & 0x00FF) | row));
  wr16(0x0902 + y, 0x2322);
  wr16(0x0906 + y, 0x2322);
  wr16(0x0B02 + y, 0);
  wr16(0x0B06 + y, 0);
  wr16(dp + 0x74, (uint16_t)(y + 8));
  uint16_t x = rom16(0x81A896 + (uint32_t)rd16(dp + 0x7C));
  wr16(dp + 0x7E, shop_part_level(dp, x, 0));
  wr16(dp + 0x80, 0x002C);
  wr16(dp + 0x84, 0x231E);
  shop_bar_column(dp);
  x = rom16(0x81A896 + (uint32_t)rd16(dp + 0x7C));
  wr16(dp + 0x7E, shop_part_level(dp, x, 1));
  wr16(dp + 0x80, 0x0033);
  wr16(dp + 0x84, 0x2320);
  shop_bar_column(dp);
}

// TG3000_ROUTINE($80:AA59) shop_gauge
void shop_gauge(uint16_t dp) {
  uint16_t stage = rd16(dp + 0x3E);
  wr16(dp + 0x16, (uint16_t)(0x30 - rd16(dp + 0x7A) + rd16(dp + 0xE8)));
  wr16(dp + 0x18, 0x0100);
  sprite_group(dp, stage == 0x10 ? 0x7E33DA : 0x7E3438);
  stage = rd16(dp + 0x3E);
  uint16_t past = stage < 0x10 ? 0 : (uint16_t)(stage - 0x10);
  if(past < 0x38) {
    wr16(dp + 0x80, (uint16_t)((past & 0xFFF8) + 8));
    wr16(dp + 0x7E, (uint16_t)(((past & 7) << 1) + 0x2524));
    shop_bar_cap(dp);
    uint16_t n = (uint16_t)((uint16_t)(0x37 - past) >> 3);
    wr16(dp + 0x82, n);
    while(rd16(dp + 0x82)) {
      wr16(dp + 0x7E, 0x2524);
      wr16(dp + 0x80, (uint16_t)(rd16(dp + 0x80) + 8));
      shop_bar_cap(dp);
      wr16(dp + 0x82, (uint16_t)(rd16(dp + 0x82) - 1));
    }
  }
  stage = rd16(dp + 0x3E);
  uint16_t low = stage < 0x0C ? 0 : (uint16_t)(stage - 0x0C);
  if(low < 0x3C) {
    wr16(dp + 0x80, (uint16_t)(low & 0xFFFC));
    wr16(dp + 0x7E, (uint16_t)(((low & 3) << 1) + 0x251C));
    shop_bar_cap(dp);
  }
}

// TG3000_ROUTINE($80:A26E) shop_item_price
uint16_t shop_item_price(uint16_t dp) {
  uint16_t stage = (uint16_t)(rd16(dp + 0x3E) - 0x10);
  if(stage & 0x8000) stage = 0;
  if(stage < 8) {
    const uint32_t y = rd16(dp + 0xC4);
    wr16(0x01A2 + y, rd16(0x4DCA + (uint32_t)(stage << 1)));
    wr16(0x01A4 + y, rd16(0x4DCA + (uint32_t)(stage & 0xFFFE)));
  }
  uint16_t base = rd16(dp + 0x14), x = rd16(dp + 0x16);
  uint16_t price = rom16(0x81A89B + (uint32_t)shop_item_entry(dp, rd16(dp + 0x4A)));
  wr16(dp + 0x14, 0x3A00);
  uint32_t value = (uint32_t)rom16(0x81A88E + (uint32_t)price) << 16 | rom16(0x81A890 + (uint32_t)price);
  uint16_t shift = sprite_number_centered(dp, value, (uint16_t)(0x5C2E + rd16(dp + 0xE7)));
  wr16(dp + 0x16, x);
  wr16(dp + 0x14, base);
  return shift;
}

// One of a bar entry's sprite groups ($7E:word at $81:A892 + `entry` + `field`) at x =
// `x` + D+$E8, the tile base $0100.
static void shop_item_group(uint16_t dp, uint16_t entry, uint32_t field, uint16_t x) {
  wr16(dp + 0x16, (uint16_t)(x + rd16(dp + 0xE8)));
  wr16(dp + 0x18, 0x0100);
  sprite_group(dp, 0x7E0000 | rom16(0x81A892 + field + (uint32_t)entry));
}

// The stage moves on, stopping at $7FFF.
static void shop_item_stage_step(uint16_t dp) {
  uint16_t stage = (uint16_t)(rd16(dp + 0x3E) + 1);
  if(stage & 0x8000) stage = (uint16_t)(stage - 1);
  wr16(dp + 0x3E, stage);
}

// TG3000_ROUTINE($80:A2CB) shop_item_view_a
void shop_item_view_a(uint16_t dp, uint16_t entry) {
  wr16(dp + 0x7A, rd16(dp + 0x16));
  wr16(dp + 0x7C, entry);
  shop_bar_levels(dp);
  shop_item_group(dp, rd16(dp + 0x7C), 0, rd16(dp + 0x7A));
  wr16(dp + 0x84, 0x2320);
  shop_bar_segments(dp);
  shop_item_group(dp, rd16(dp + 0x7C), 6, (uint16_t)(0x30 - rd16(dp + 0x7A)));
  shop_item_group(dp, rd16(dp + 0x7C), 2, (uint16_t)(0x30 - rd16(dp + 0x7A)));
  shop_item_stage_step(dp);
}

// TG3000_ROUTINE($80:A831) shop_item_view_b
void shop_item_view_b(uint16_t dp, uint16_t entry) {
  wr16(dp + 0x7A, rd16(dp + 0x16));
  wr16(dp + 0x7C, entry);
  shop_bar_double(dp);
  wr16(dp + 0x84, 0x2312);
  shop_bar_segments_long(dp);
  shop_item_group(dp, rd16(dp + 0x7C), 0, rd16(dp + 0x16));
  shop_item_group(dp, rd16(dp + 0x7C), 2, (uint16_t)(0x30 - rd16(dp + 0x7A)));
  shop_item_stage_step(dp);
}

// TG3000_ROUTINE($80:AA15) shop_item_view_c
void shop_item_view_c(uint16_t dp, uint16_t entry) {
  wr16(dp + 0x7A, rd16(dp + 0x16));
  wr16(dp + 0x7C, entry);
  shop_item_group(dp, rd16(dp + 0x7C), 0, rd16(dp + 0x16));
  shop_gauge(dp);
  shop_item_group(dp, rd16(dp + 0x7C), 2, (uint16_t)(0x30 - rd16(dp + 0x7A)));
  shop_item_stage_step(dp);
}

// TG3000_ROUTINE($80:A717) shop_item_view_d
void shop_item_view_d(uint16_t dp, uint16_t entry) {
  wr16(dp + 0x7A, rd16(dp + 0x16));
  wr16(dp + 0x7C, entry);
  shop_item_group(dp, rd16(dp + 0x7C), 0, rd16(dp + 0x7A));
  uint16_t y = rd16(dp + 0x74);
  wr16(dp + 0x16, (uint16_t)(((0x30 - rd16(dp + 0x7A) + rd16(dp + 0xE8) + 0x20) & 0xFF) << 8));
  uint16_t half = (uint16_t)(rd16(dp + 0x3E) >> 1);
  uint16_t blocks;
  bool draw = true;
  if(half < 0x20) {  // not started: four full blocks
    wr16(dp + 0x7E, (uint16_t)(rd16(dp + 0x14) + 0x14));
    blocks = 4;
  } else {
    uint16_t done = (uint16_t)((half - 0x20) & 0xFFF8);
    if(done >= 0x20) {
      draw = false;
      blocks = 0;
    } else {  // the block under way, then the full ones left
      wr16(dp + 0x7E, (uint16_t)(done + rd16(dp + 0x14) + 0x1C));
      y = shop_sprite_block(dp, (uint16_t)((half & 7) + 0x2316), y);
      blocks = (uint16_t)((uint16_t)(0x1F - (uint16_t)(half - 0x20)) >> 3);
    }
  }
  if(draw) {
    for(; blocks; blocks--) {
      wr16(dp + 0x7E, (uint16_t)(rd16(dp + 0x7E) + 8));
      y = shop_sprite_block(dp, 0x2316, y);
    }
    wr16(dp + 0x74, y);
  }
  shop_item_group(dp, rd16(dp + 0x7C), 2, (uint16_t)(0x30 - rd16(dp + 0x7A)));
  shop_item_group(dp, rd16(dp + 0x7C), 4, (uint16_t)(0x30 - rd16(dp + 0x7A)));
  shop_item_stage_step(dp);
}

// TG3000_ROUTINE($80:AB54) shop_item_view_e
void shop_item_view_e(uint16_t dp, uint16_t entry) {
  wr16(dp + 0x7A, rd16(dp + 0x16));
  wr16(dp + 0x7C, entry);
  shop_item_group(dp, rd16(dp + 0x7C), 0, rd16(dp + 0x16));
  shop_item_group(dp, rd16(dp + 0x7C), 2, (uint16_t)(0x30 - rd16(dp + 0x7A)));
  shop_item_group(dp, rd16(dp + 0x7C), 4, (uint16_t)(0x30 - rd16(dp + 0x7A)));
  const uint32_t y = rd16(dp + 0xC4);
  uint16_t wave = rd16(dp + 0x40);
  uint32_t x = (wave & 0x0020) ? (uint32_t)(uint16_t)(0x1E - (wave & 0x001E)) : (uint32_t)(wave & 0x001E);
  wr16(0x012E + y, rd16(0x4E1A + x));
  wr16(0x012A + y, rd16(0x4E3A + x));
  wr16(0x0128 + y, rd16(0x4E5A + x));
  wr16(0x0126 + y, rd16(0x4E7A + x));
  wr16(0x0124 + y, rd16(0x4E9A + x));
  wr16(0x0122 + y, rd16(0x4EBA + x));
  wr16(dp + 0x40, (uint16_t)(rd16(dp + 0x40) + 1));
  shop_item_stage_step(dp);
}

// TG3000_ROUTINE($80:A1E4) shop_item_icon_draw
void shop_item_icon_draw(uint16_t dp, uint16_t group) {
  wr16(dp + 0x18, (uint16_t)(rd16(dp + 0x44) << 1) < 0x0010 ? 0x0E00 : 0x2E00);
  wr16(dp + 0x14, rd16(dp + 0x42));
  wr16(dp + 0x16, rd16(dp + 0xE8));
  sprite_group(dp, 0x7E0000 | group);
  uint16_t step = rd16(dp + 0x44);
  if(step & 0x8000) return;
  const uint32_t x = (uint16_t)(step << 1), y = rd16(dp + 0xC4);
  wr16(0x01EE + y, rd16(0x4EDA + x));
  wr16(0x01F0 + y, rd16(0x4EFA + x));
  if(x < 0x0010) {
    wr16(0x0182 + y, rd16(0x4F1A + x));
    wr16(0x0186 + y, rd16(0x4F2A + x));
    wr16(0x0188 + y, rd16(0x4F3A + x));
    wr16(0x018E + y, rd16(0x4F4A + x));
  }
  if(step == 0x000F || (step == 0x0007 && !(rd16(dp + 0x76) & 0x8000))) step |= 0x8000;
  else step = (uint16_t)(step + 1);
  wr16(dp + 0x44, step);
}

// One list's handler: its level byte at $0393 + `field` + D+$94; the icon table at $81:`icons`;
// the upload's VRAM offset at $81:`offsets` and size at $81:`sizes` (4-byte rows); with
// `skipZero`, level 0 queues nothing.
static void shop_item_icon_from(uint16_t dp, bool upload, uint16_t field, uint16_t icons, uint16_t offsets, uint16_t sizes,
                                bool skipZero) {
  uint16_t level = (uint16_t)(rd16(0x0393 + (uint32_t)field + rd16(dp + 0x94)) & 0x007F);
  if(!upload) {
    shop_item_icon_draw(dp, rom16(0x810000 + (uint32_t)icons + (uint32_t)(uint16_t)(level << 1)));
    return;
  }
  if(skipZero && level == 0) return;
  const uint32_t row = (uint32_t)(uint16_t)(level << 2);
  wr16(dp + 0x14, 0x007F);
  func_80B537(dp, rom16(0x810000 + (uint32_t)sizes + row),
              (uint16_t)(rom16(0x810000 + (uint32_t)offsets + row) + rd16(dp + 0x5E) + 0x0900),
              (uint16_t)(rd16(dp + 0x5C) + 0x1000));
}
static void shop_item_icon(uint16_t dp, bool upload, uint16_t field, uint16_t icons, uint16_t uploads) {
  shop_item_icon_from(dp, upload, field, icons, uploads, (uint16_t)(uploads + 2), false);
}

// TG3000_ROUTINE($80:9F5B) shop_item_icon_1
void shop_item_icon_1(uint16_t dp, bool upload) { shop_item_icon(dp, upload, 0, 0xBC2E, 0xBC16); }

// TG3000_ROUTINE($80:9F9A) shop_item_icon_2
void shop_item_icon_2(uint16_t dp, bool upload) { shop_item_icon(dp, upload, 1, 0xBC52, 0xBC3A); }

// TG3000_ROUTINE($80:9FD9) shop_item_icon_3
void shop_item_icon_3(uint16_t dp, bool upload) { shop_item_icon(dp, upload, 2, 0xBC76, 0xBC5E); }

// TG3000_ROUTINE($80:A057) shop_item_icon_4
void shop_item_icon_4(uint16_t dp, bool upload) { shop_item_icon(dp, upload, 3, 0xBCBE, 0xBCA6); }

// TG3000_ROUTINE($80:A018) shop_item_icon_5
void shop_item_icon_5(uint16_t dp, bool upload) { shop_item_icon(dp, upload, 4, 0xBC9A, 0xBC82); }

// TG3000_ROUTINE($80:A096) shop_item_icon_0
void shop_item_icon_0(uint16_t dp, bool upload) {
  if(!upload) shop_item_icon_draw(dp, 0x4F6A);
}
// TG3000_ROUTINE($80:A121) shop_item_icon_6
void shop_item_icon_6(uint16_t dp, bool upload) { shop_item_icon_from(dp, upload, 5, 0xBCFA, 0xBCD6, 0xBCD8, true); }
// TG3000_ROUTINE($80:A0E0) shop_item_icon_7
void shop_item_icon_7(uint16_t dp, bool upload) { shop_item_icon_from(dp, upload, 6, 0xBCF6, 0xBCCE, 0xBCC8, true); }
// TG3000_ROUTINE($80:A09F) shop_item_icon_8
void shop_item_icon_8(uint16_t dp, bool upload) { shop_item_icon_from(dp, upload, 7, 0xBCF2, 0xBCC6, 0xBCC8, true); }
// TG3000_ROUTINE($80:A162) shop_item_icon_9
void shop_item_icon_9(uint16_t dp, bool upload) { shop_item_icon_from(dp, upload, 9, 0xBCFE, 0xBCDE, 0xBCE0, true); }
// TG3000_ROUTINE($80:A1A3) shop_item_icon_10 (never run)
void shop_item_icon_10(uint16_t dp, bool upload) { shop_item_icon_from(dp, upload, 8, 0xBD02, 0xBCE6, 0xBCE8, true); }

// The list's level getter ($80:9EEA table): lists 0-5 the part levels plus 1; lists 6-10
// the bytes at $0398, $0399, $039A, $039C, $039B as they are. (The table's entries past list
// 10 point at a getter that sets Y = 0 and returns A as it was: no list uses them.)
static uint16_t shop_list_level(uint16_t list, uint16_t car) {
  switch(list) {
  case 0: return shop_part_level_2(car);
  case 1: return shop_part_level_3(car);
  case 2: return shop_part_level_4(car);
  case 3: return shop_part_level_5(car);
  case 4: return shop_part_level_6(car);
  case 5: return shop_part_level_7(car);
  case 6: return shop_part_level_8(car);
  case 7: return shop_part_level_9(car);
  case 8: return shop_part_level_a(car);
  case 9: return shop_part_level_c(car);
  default: return shop_part_level_b(car);  // list 10 (shop-23: was $039C before)
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
void shop_level_marks(uint16_t dp) {
  uint16_t slot = rd16(dp + 0x74);
  uint16_t gap = (uint16_t)((uint16_t)(rd16(dp + 0x4A) << 4) + 0x14 - rd16(dp + 0xEE));
  if(gap) wr16(dp + 0xEE, (uint16_t)(rd16(dp + 0xEE) + ((gap & 0x8000) ? -1 : 1)));
  slot = shop_mark(slot, 0x3892, (uint16_t)(0x085A + rd16(dp + 0xEE) + rd16(dp + 0xE7)));
  slot = shop_mark(slot, 0x3890, (uint16_t)(0x0A5C + rd16(dp + 0xE7)));
  wr16(dp + 0x74, slot);
  uint16_t list = rd16(dp + 0x48);
  wr16(dp + 0x24, (uint16_t)(rd16(0x036E + (uint32_t)list) & 0x00FF));
  wr16(dp + 0x26, (uint16_t)(0x0A70 + rd16(dp + 0xE7)));
  wr16(dp + 0x28, 0x3850);
  uint16_t left = (uint16_t)(shop_list_level(list, rd16(dp + 0x94)) & 0x007F);
  do {
    uint16_t pos = rd16(dp + 0x26);
    wr16(dp + 0x26, (uint16_t)(pos + 0x10));
    uint16_t tile = rd16(dp + 0x28);
    if(--left == 0) tile = (uint16_t)(tile + 0x20);
    const uint32_t x = slot;
    wr16(0x0900 + x, pos);
    wr16(0x0902 + x, tile);
    wr16(0x0B01 + x, 0x0100);
    wr16(dp + 0x28, (uint16_t)(rd16(dp + 0x28) + 2));
    slot = (uint16_t)(slot + 4);
    wr16(dp + 0x24, (uint16_t)(rd16(dp + 0x24) - 1));
  } while(rd16(dp + 0x24) != 0);
  wr16(dp + 0x74, slot);
}

#define SHOP_PANEL_MAP 0x10000  // $7F:0000

// TG3000_ROUTINE($80:B0C0) shop_panel_blank
void shop_panel_blank(uint16_t dp) {
  const uint32_t x = rd16(dp + 0x5E);
  for(uint32_t col = 0; col < 16; col += 2)
    for(uint32_t row = 0; row < 16; row++) wr16(SHOP_PANEL_MAP + (uint16_t)(0x0700 + row * 0x20 + x + col), 0xFFFF);
  wr16(dp + 0x4E, 0);
  wr16(dp + 0xEC, 0xB149);
}

// TG3000_ROUTINE($80:B149) shop_panel_frame
void shop_panel_frame(uint16_t dp) {
  static const uint16_t icon_from[6] = {0x4EDA, 0x4EFA, 0x4F1A, 0x4F2A, 0x4F3A, 0x4F4A};  // $7E
  static const uint16_t icon_to[6] = {0x01EE, 0x01F0, 0x0182, 0x0186, 0x0188, 0x018E};
  const bool typed = rd16(dp + 0x8C) == rd16(dp + 0x8E);
  uint16_t frame = rd16(dp + 0x44);
  if(frame & 0x8000) {  // the icon's frames: one a call, counting down to $8000
    const uint16_t x = (uint16_t)(frame << 1), y = rd16(dp + 0xC4);
    for(int i = 0; i < 6; i++) wr16((uint16_t)(icon_to[i] + y), rd16(icon_from[i] + (uint32_t)x));
    if(frame != 0x8000) frame--;
    wr16(dp + 0x44, frame);
    if(!typed) {
      shop_item_dispatch_icon(dp);
      shop_level_marks(dp);
      func_809DD0();
      oam_finish_near(dp);
    }
  }
  if(!typed) return;
  text_type_step(dp);
  // the panel's static: 9 x 9 random tiles (palette bits and tile $1000 + 0-15, D+$62 on)
  // and 16 x 8 random words below them, in the panel's map at $7F:0000 + D+$5E
  const uint16_t at = rd16(dp + 0x5E);
  for(uint16_t col = 0; col < 18; col += 2)
    for(uint16_t row = 0; row < 9; row++) {
      const uint16_t r = rng_next(RNG_STATE, NULL);
      wr16(SHOP_PANEL_MAP + (uint16_t)(0x00C4 + row * 0x40 + at + col),
           (uint16_t)((r & 0xC00F) + 0x1000 + rd16(dp + 0x62)));
    }
  for(uint16_t col = 0; col < 16; col += 2)
    for(uint16_t row = 0; row < 16; row++)
      wr16(SHOP_PANEL_MAP + (uint16_t)(0x0710 + row * 0x20 + at + col), rng_next(RNG_STATE, NULL));
  wr16(dp + 0x14, 0x007F);
  func_80B537(dp, 0x0200, (uint16_t)(at + 0x0700), rd16(dp + 0x60));
  wr16(dp + 0x14, 0x007F);
  func_80B537(dp, 0x03C0, at, (uint16_t)(rd16(dp + 0x5C) + 0x0400));
  wr16(dp + 0x4E, (uint16_t)(rd16(dp + 0x4E) + 1));
}

// TG3000_ROUTINE($80:B10C) shop_panel_clear
void shop_panel_clear(uint16_t dp) {
  for(;;) {
    wr16(dp + 0xEC, 0xB94F);
    if(rd16(dp + 0x8C) == rd16(dp + 0x8E)) break;
    platform_wait_vblank();
  }
  const uint32_t x = rd16(dp + 0x5E);
  for(uint32_t col = 0; col < 18; col += 2)
    for(uint32_t row = 0; row < 9; row++) wr16(SHOP_PANEL_MAP + (uint16_t)(0x00C4 + row * 0x40 + x + col), 0);
}

static uint16_t shop_list_key(uint16_t dp) {
  uint16_t a = (uint16_t)(rd16(dp + 0x48) << 3);
  a = (uint16_t)((a << 8) | (a >> 8));  // XBA
  return (uint16_t)(a + rd16(dp + 0x4A));
}

// TG3000_ROUTINE($80:B359) shop_list_locked
bool shop_list_locked(uint16_t dp) {
  uint16_t key = shop_list_key(dp);
  if(key & 0x8000) return true;
  return !(key == 0x1800 || ((uint16_t)(key - 0x1800) & 0x8000));
}

// TG3000_ROUTINE($80:B333) shop_list_prev
void shop_list_prev(uint16_t dp) {
  if(shop_list_locked(dp) || rd16(dp + 0x48) == 0) return;
  wr16(dp + 0x48, (uint16_t)(rd16(dp + 0x48) - 1));
  wr16(dp + 0x4E, 0);
  func_80E4C5();
}

// TG3000_ROUTINE($80:B345) shop_list_next
void shop_list_next(uint16_t dp) {
  if(shop_list_locked(dp) || rd16(dp + 0x48) == rd16(dp + 0xF0)) return;
  wr16(dp + 0x48, (uint16_t)(rd16(dp + 0x48) + 1));
  wr16(dp + 0x4E, 0);
  func_80E4C5();
}

static void shop_item_dispatch(uint16_t dp, bool upload) {
  uint16_t entry = shop_item_entry(dp, rd16(dp + 0x4A));
  switch(rom16(0x81A8A3 + (uint32_t)entry)) {
  case 0x9F5B: shop_item_icon_1(dp, upload); break;
  case 0x9F9A: shop_item_icon_2(dp, upload); break;
  case 0x9FD9: shop_item_icon_3(dp, upload); break;
  case 0xA057: shop_item_icon_4(dp, upload); break;
  case 0xA018: shop_item_icon_5(dp, upload); break;
  case 0xA121: shop_item_icon_6(dp, upload); break;
  case 0xA0E0: shop_item_icon_7(dp, upload); break;
  case 0xA09F: shop_item_icon_8(dp, upload); break;
  case 0xA162: shop_item_icon_9(dp, upload); break;
  case 0xA1A3: shop_item_icon_10(dp, upload); break;  // (shop-25: lists 9 and 10 did nothing here)
  case 0xA096: shop_item_icon_0(dp, upload); break;
  default: break;  // the table holds no other handler for lists 0-10
  }
}

// TG3000_ROUTINE($80:9F40) shop_item_dispatch_icon
void shop_item_dispatch_icon(uint16_t dp) { shop_item_dispatch(dp, false); }

// TG3000_ROUTINE($80:9C0B) shop_item_dispatch_upload
void shop_item_dispatch_upload(uint16_t dp) { shop_item_dispatch(dp, true); }

// TG3000_ROUTINE($80:ABF6) shop_item_view_g
void shop_item_view_g(uint16_t dp, uint16_t entry) {
  wr16(dp + 0x7A, rd16(dp + 0x16));
  wr16(dp + 0x7C, entry);
  shop_item_group(dp, rd16(dp + 0x7C), 0, rd16(dp + 0x7A));
  shop_item_group(dp, rd16(dp + 0x7C), 2, (uint16_t)(0x30 - rd16(dp + 0x7A)));
  shop_item_stage_step(dp);
}

static uint16_t shop_ring_piece(uint16_t dp, uint32_t table, uint16_t angle, uint16_t slot) {
  const uint32_t y = slot;
  uint16_t frame = (uint16_t)((angle >> 8) & 0x000F);
  wr16(dp + 0x16, frame);
  const uint32_t x = (uint32_t)frame * 6;
  wr16(0x0902 + y, rd16(table + x));
  uint16_t row = (uint16_t)(((rd16(table + 2 + x) + rd16(dp + 0x1A)) & 0xFF) << 8);
  wr16(dp + 0x16, row);
  uint16_t at = (uint16_t)(rd16(dp + 0x18) + rd16(table + 4 + x));
  wr16(0x0B00 + y, at);
  wr16(0x0900 + y, (uint16_t)((at & 0x00FF) | row));
  wr16(0x0B02 + y, 0);
  return (uint16_t)(slot + 4);
}

// TG3000_ROUTINE($80:A69F) shop_ring_piece_front
uint16_t shop_ring_piece_front(uint16_t dp, uint16_t angle, uint16_t slot) { return shop_ring_piece(dp, 0x4D0A, angle, slot); }

// TG3000_ROUTINE($80:A6DB) shop_ring_piece_back
uint16_t shop_ring_piece_back(uint16_t dp, uint16_t angle, uint16_t slot) { return shop_ring_piece(dp, 0x4D6A, angle, slot); }

// TG3000_ROUTINE($80:A579) shop_ring
void shop_ring(uint16_t dp) {
  wr16(dp + 0x1A, (uint16_t)(0x5E - rd16(dp + 0x7A) + rd16(dp + 0xE8)));
  wr16(dp + 0x18, (uint16_t)(rd16(dp + 0x14) + 0x2B));
  uint16_t y = rd16(dp + 0x74);
  for(int behind = 1; behind >= 0; behind--) {
    for(uint16_t n = 0; n < 2; n++) {
      uint16_t angle = (uint16_t)(rd16(dp + 0x40) + n * 0x0400);
      if(((angle & 0x0800) != 0) != behind) continue;
      y = (n & 1) ? shop_ring_piece_back(dp, angle, y) : shop_ring_piece_front(dp, angle, y);
    }
    if(behind) {  // the centre, between the two halves
      const uint32_t s = y;
      wr16(0x0902 + s, 0x2526);
      wr16(0x0B02 + s, 0x0001);
      uint16_t at = rd16(dp + 0x18);
      wr16(0x0B00 + s, at);
      uint16_t row = (uint16_t)((rd16(dp + 0x1A) & 0x00FF) << 8);
      wr16(dp + 0x16, row);
      wr16(0x0900 + s, (uint16_t)((at & 0x00FF) | row));
      y = (uint16_t)(y + 4);
    }
  }
  wr16(dp + 0x74, y);
  wr16(dp + 0x40, (uint16_t)(rd16(dp + 0x40) + 0x0080));
}

// TG3000_ROUTINE($80:A333) shop_item_view_f
void shop_item_view_f(uint16_t dp, uint16_t entry) {
  wr16(dp + 0x7A, rd16(dp + 0x16));
  wr16(dp + 0x7C, entry);
  shop_ring(dp);
  shop_item_group(dp, rd16(dp + 0x7C), 0, rd16(dp + 0x7A));
  wr16(dp + 0x84, 0x2338);
  shop_bar_segments(dp);
  shop_item_group(dp, rd16(dp + 0x7C), 6, (uint16_t)(0x30 - rd16(dp + 0x7A)));
  shop_item_group(dp, rd16(dp + 0x7C), 2, (uint16_t)(0x30 - rd16(dp + 0x7A)));
  shop_item_stage_step(dp);
}

static uint16_t shop_view_x(uint16_t dp, uint16_t base) { return (uint16_t)(base - rd16(dp + 0x7A) + rd16(dp + 0xE8)); }

static void shop_view_group(uint16_t dp, uint16_t x, uint16_t group) {
  wr16(dp + 0x16, x);
  wr16(dp + 0x18, 0x0100);
  sprite_group(dp, 0x7E0000 | group);
}

// D+$40 + $40, back by `wrap` when it reaches it
static void shop_view_spin(uint16_t dp, uint16_t wrap) {
  uint16_t a = (uint16_t)(rd16(dp + 0x40) + 0x0040);
  if(a >= wrap) a = (uint16_t)(a - wrap);
  wr16(dp + 0x40, a);
}

// TG3000_ROUTINE($80:AF36) shop_level_blocks
bool shop_level_blocks(uint16_t dp) {
  wr16(dp + 0x80, 0);
  uint16_t y = rd16(dp + 0x74);
  wr16(dp + 0x7E, 0);
  uint16_t a = (uint16_t)(rd16(dp + 0x3E) >> 1);
  if(a >= 0x0010) {
    wr16(dp + 0x7E, (uint16_t)(a - 0x0010));
    for(;;) {
      if(rd16(dp + 0x7E) < 6) break;
      wr16(dp + 0x7E, (uint16_t)(rd16(dp + 0x7E) - 6));
      wr16(dp + 0x80, (uint16_t)(rd16(dp + 0x80) + 6));
      if(rd16(dp + 0x80) != 0x002A) continue;
      // all seven full: the last is drawn half a block over ($80:AF5E; the password movie
      // runs it); with three or more left over, nothing is drawn (a JMP to the RTS)
      wr16(dp + 0x80, (uint16_t)(rd16(dp + 0x80) - 3));
      if(rd16(dp + 0x7E) >= 3) return true;
      wr16(dp + 0x7E, (uint16_t)(rd16(dp + 0x7E) + 3));
      break;
    }
  }
  uint16_t pos;
  do {
    wr16(dp + 0x82, (uint16_t)(((uint16_t)(0x62 - rd16(dp + 0x7A)) << 8) + rd16(dp + 0xE7)));
    const uint32_t s = y;
    uint16_t at = (uint16_t)(rd16(dp + 0x14) + rd16(dp + 0x80) + 0x000A);
    wr16(0x0B00 + s, at);
    wr16(0x0B04 + s, at);
    uint16_t word = (uint16_t)((at & 0x00FF) | rd16(dp + 0x82));
    wr16(0x0900 + s, word);
    wr16(0x0904 + s, (uint16_t)(word + 0x0800));
    uint16_t tile = (uint16_t)((rd16(dp + 0x7E) << 1) + rd16(dp + 0x84));
    wr16(0x0902 + s, tile);
    wr16(0x0906 + s, (uint16_t)(tile + 1));
    wr16(0x0B02 + s, 0);
    wr16(0x0B06 + s, 0);
    y = (uint16_t)(y + 8);
    wr16(dp + 0x7E, 0);
    pos = (uint16_t)(rd16(dp + 0x80) + 6);
    wr16(dp + 0x80, pos);
    if(pos == 0x002A) {
      pos = 0x0027;
      wr16(dp + 0x80, pos);
      wr16(dp + 0x7E, 3);
    }
  } while(pos != 0x002D);
  wr16(dp + 0x74, y);
  return false;
}

// TG3000_ROUTINE($80:ADD9) shop_gauge_line
void shop_gauge_line(uint16_t dp, uint8_t speed, uint16_t y, uint16_t string) {
  uint8_t high = (uint8_t)(rd8(dp + 0x47) & 0x7F);
  int16_t m7a = (int16_t)((uint16_t)(high << 8) | rd8(dp + 0x46));
  video_set_m7a(m7a);
  video_set_m7b((int16_t)((uint16_t)(speed << 8) | high));
  uint16_t product = (uint16_t)((int32_t)m7a * (int8_t)speed);
  wr16(dp + 0x80, (uint16_t)((((uint16_t)(y + product) >> 5) & 0x007F) - 0x0020));
  uint32_t at = string;
  uint16_t slot = rd16(dp + 0x74);
  uint16_t tile = rd16(at);
  do {
    const uint32_t x = slot;
    wr16(0x0902 + x, tile);
    uint16_t col = (uint16_t)(rd16(dp + 0x80) + rd16(dp + 0x14));
    if(col & 0xFF00) col |= 0xFF00;
    wr16(0x0B00 + x, col);
    wr16(dp + 0x1A, (uint16_t)(col & 0x00FF));
    wr16(0x0900 + x, (uint16_t)((uint16_t)((rd16(dp + 0x16) + rd16(dp + 0x7E)) << 8) | rd16(dp + 0x1A)));
    wr16(0x0B02 + x, 0);
    wr16(dp + 0x80, (uint16_t)(rd16(dp + 0x80) + 8));
    if(!((uint16_t)(rd16(dp + 0x80) - 0x0048) & 0x8000)) break;
    slot = (uint16_t)(slot + 4);
    at = (uint16_t)(at + 2);
    tile = rd16(at);
  } while(tile != 0);
  wr16(dp + 0x74, slot);
}

// TG3000_ROUTINE($80:ACB6) shop_gauge_lines
void shop_gauge_lines(uint16_t dp) {
  wr16(dp + 0x16, shop_view_x(dp, 0x50));
  uint16_t slot = rd16(dp + 0x74);
  wr16(dp + 0x7E, 0);
  do {
    const uint32_t x = slot;
    wr16(0x0902 + x, 0x032F);
    uint16_t col = (uint16_t)(0x0040 + rd16(dp + 0x14));
    if(col & 0xFF00) col |= 0xFF00;  // $80:ACD6, never run
    wr16(0x0B00 + x, col);
    wr16(dp + 0x1A, (uint16_t)(col & 0x00FF));
    wr16(0x0900 + x, (uint16_t)((uint16_t)((rd16(dp + 0x16) + rd16(dp + 0x7E)) << 8) | rd16(dp + 0x1A)));
    wr16(0x0B02 + x, 0);
    slot = (uint16_t)(slot + 4);
    wr16(dp + 0x7E, (uint16_t)(rd16(dp + 0x7E) + 8));
  } while(rd16(dp + 0x7E) != 0x0020);
  wr16(dp + 0x74, slot);
  static const struct { uint16_t row, speed, string; } lines[12] = {
    {0x00, 0x40, 0x44BA}, {0x05, 0x30, 0x44BA}, {0x10, 0x1E, 0x44C0}, {0x1C, 0x20, 0x44C4},
    {0x0A, 0x16, 0x44CA}, {0x14, 0x4E, 0x44CE}, {0x12, 0x0E, 0x44DA}, {0x0C, 0x1B, 0x44E0},
    {0x08, 0x14, 0x44CA}, {0x0F, 0x5F, 0x44CE}, {0x03, 0x13, 0x44DA}, {0x1D, 0x2F, 0x44E0},
  };
  for(int i = 0; i < 12; i++) {
    wr16(dp + 0x7E, lines[i].row);
    shop_gauge_line(dp, (uint8_t)lines[i].speed, 0x0020, lines[i].string);
  }
}

// TG3000_ROUTINE($80:AC37) shop_item_view_j
void shop_item_view_j(uint16_t dp, uint16_t entry) {
  wr16(dp + 0x7A, rd16(dp + 0x16));
  wr16(dp + 0x7C, entry);
  shop_item_group(dp, rd16(dp + 0x7C), 0, rd16(dp + 0x7A));
  uint16_t frame = (uint16_t)((rd16(dp + 0x40) >> 8) & 0x000F);
  shop_view_group(dp, shop_view_x(dp, 0x50), rom16(0x81BD06 + (uint32_t)(uint16_t)(frame << 1)));
  shop_gauge_lines(dp);
  const uint32_t y = (uint16_t)(rd16(dp + 0xC4) + 0x0110);
  const uint32_t x = (uint16_t)(((rd16(dp + 0x40) >> 8) << 1) & 0x0006);
  wr16(y + 0, rd16(0x4F5A + x));
  wr16(y + 2, rd16(0x4F5C + x));
  wr16(y + 4, rd16(0x4F5E + x));
  wr16(y + 6, rd16(0x4F60 + x));
  wr16(dp + 0x40, (uint16_t)(rd16(dp + 0x40) + 0x0040));
  wr16(dp + 0x46, (uint16_t)(rd16(dp + 0x46) - 1));
  shop_item_stage_step(dp);
}

// TG3000_ROUTINE($80:AE4E) shop_item_view_i
void shop_item_view_i(uint16_t dp, uint16_t entry) {
  wr16(dp + 0x7A, rd16(dp + 0x16));
  wr16(dp + 0x7C, entry);
  shop_item_group(dp, rd16(dp + 0x7C), 0, rd16(dp + 0x7A));
  shop_view_group(dp, shop_view_x(dp, 0x50), 0x4686);
  uint16_t frame = (uint16_t)((rd16(dp + 0x40) >> 8) & 0x00FF);
  shop_view_group(dp, shop_view_x(dp, 0x50), rom16(0x81BD26 + (uint32_t)(uint16_t)(frame << 1)));
  shop_view_spin(dp, 0x0F00);
  shop_item_stage_step(dp);
}

// TG3000_ROUTINE($80:AEBE) shop_item_view_h
void shop_item_view_h(uint16_t dp, uint16_t entry) {
  wr16(dp + 0x7A, rd16(dp + 0x16));
  wr16(dp + 0x7C, entry);
  shop_item_group(dp, rd16(dp + 0x7C), 0, rd16(dp + 0x7A));
  wr16(dp + 0x84, 0x257B);
  (void)shop_level_blocks(dp);
  uint16_t frame = (uint16_t)((rd16(dp + 0x40) >> 8) & 0x00FF);
  shop_view_group(dp, shop_view_x(dp, 0x50), rom16(0x81BD44 + (uint32_t)(uint16_t)(frame << 1)));
  shop_view_group(dp, shop_view_x(dp, 0x50), 0x46BA);
  shop_view_spin(dp, 0x1E00);
  shop_item_stage_step(dp);
}

// TG3000_ROUTINE($80:A251) shop_item_dispatch_view
void shop_item_dispatch_view(uint16_t dp) {
  uint16_t entry = shop_item_entry(dp, rd16(dp + 0x4A));
  uint16_t bars = rom16(0x81A8A1 + (uint32_t)entry);
  switch(rom16(0x81A897 + (uint32_t)entry)) {
  case 0xA2CB: shop_item_view_a(dp, bars); break;
  case 0xA333: shop_item_view_f(dp, bars); break;
  case 0xA717: shop_item_view_d(dp, bars); break;
  case 0xA831: shop_item_view_b(dp, bars); break;
  case 0xAA15: shop_item_view_c(dp, bars); break;
  case 0xAB54: shop_item_view_e(dp, bars); break;
  case 0xABF6: shop_item_view_g(dp, bars); break;
  case 0xAEBE: shop_item_view_h(dp, bars); break;
  case 0xAE4E: shop_item_view_i(dp, bars); break;
  case 0xAC37: shop_item_view_j(dp, bars); break;
  default: break;  // the table holds no other view for lists 0-8
  }
}

// One picture's upload: VRAM D+$5C + `vram`, the pair at $81:A892 + `pictures` + `pair`.
static void shop_panel_picture(uint16_t dp, uint16_t pictures, uint16_t pair, uint16_t vram) {
  wr16(dp + 0x14, 0x007F);
  const uint32_t at = 0x81A892 + (uint32_t)(uint16_t)(pictures + pair);
  func_80B537(dp, rom16(at + 2), (uint16_t)(rom16(at) + rd16(dp + 0x5E) + 0x0900),
              (uint16_t)(rd16(dp + 0x5C) + vram));
}

// TG3000_ROUTINE($80:9C47) shop_item_panel_1 (list 1)
void shop_item_panel_1(uint16_t dp, uint16_t pictures) { shop_panel_picture(dp, pictures, 0, 0x11E0); shop_panel_picture(dp, pictures, 4, 0x1780); }
// TG3000_ROUTINE($80:9C85) shop_item_panel_2 (list 2)
void shop_item_panel_2(uint16_t dp, uint16_t pictures) { shop_panel_picture(dp, pictures, 4, 0x1120); shop_panel_picture(dp, pictures, 0, 0x1260); }
// TG3000_ROUTINE($80:9CC3) shop_item_panel_3 (list 3)
void shop_item_panel_3(uint16_t dp, uint16_t pictures) { shop_panel_picture(dp, pictures, 4, 0x1120); shop_panel_picture(dp, pictures, 0, 0x1240); }
// TG3000_ROUTINE($80:9D02) shop_item_panel_5 (list 5)
void shop_item_panel_5(uint16_t dp, uint16_t pictures) { shop_panel_picture(dp, pictures, 4, 0x1140); shop_panel_picture(dp, pictures, 0, 0x1340); }
// TG3000_ROUTINE($80:9D41) shop_item_panel_4 (list 4)
void shop_item_panel_4(uint16_t dp, uint16_t pictures) { shop_panel_picture(dp, pictures, 4, 0x1400); shop_panel_picture(dp, pictures, 0, 0x1560); }
// TG3000_ROUTINE($80:9D80) shop_item_panel_0 (list 0)
void shop_item_panel_0(uint16_t dp, uint16_t pictures) { shop_panel_picture(dp, pictures, 0, 0x1000); }
// TG3000_ROUTINE($80:9D9E) shop_item_panel_6 (lists 6-10)
void shop_item_panel_6(uint16_t dp, uint16_t pictures) { shop_panel_picture(dp, pictures, 0, 0x1140); }

// TG3000_ROUTINE($80:9C28) shop_item_dispatch_panel
void shop_item_dispatch_panel(uint16_t dp) {
  uint16_t entry = shop_item_entry(dp, rd16(dp + 0x4A));
  uint16_t pictures = rom16(0x81A89B + (uint32_t)entry);
  switch(rom16(0x81A899 + (uint32_t)entry)) {
  case 0x9C47: shop_item_panel_1(dp, pictures); break;
  case 0x9C85: shop_item_panel_2(dp, pictures); break;
  case 0x9CC3: shop_item_panel_3(dp, pictures); break;
  case 0x9D02: shop_item_panel_5(dp, pictures); break;
  case 0x9D41: shop_item_panel_4(dp, pictures); break;
  case 0x9D80: shop_item_panel_0(dp, pictures); break;
  case 0x9D9E: shop_item_panel_6(dp, pictures); break;  // lists 6-10
  default: break;  // the table holds no other handler for lists 0-10
  }
}

// TG3000_ROUTINE($80:9847) shop_item_redraw
void shop_item_redraw(uint16_t dp) {
  if(rd16(dp + 0x8C) == rd16(dp + 0x8E)) {
    text_type_step(dp);
    shop_panel_upload(dp);
  }
  wr16(dp + 0x14, 0x0010);
  wr16(dp + 0x16, 0x0018);
  shop_item_price(dp);
  shop_item_dispatch_view(dp);
  shop_item_dispatch_icon(dp);
  shop_level_marks(dp);
  func_809DD0();
  oam_finish_near(dp);
}

// one frame of a slide: the frame's wait, the typewriter and panel when the queue is empty
static void shop_slide_wait(uint16_t dp) {
  wait_frames(1);
  if(rd16(dp + 0x8C) == rd16(dp + 0x8E)) {
    text_type_step(dp);
    shop_panel_upload(dp);
  }
}

// the item drawn at (D+$14, D+$16)
static void shop_slide_draw(uint16_t dp) {
  shop_item_dispatch_view(dp);
  shop_item_dispatch_icon(dp);
  shop_level_marks(dp);
  func_809DD0();
  oam_finish_near(dp);
}

// the new item: wait for the queue to drain (to the slot it had), then its panel and text
static void shop_slide_swap(uint16_t dp) {
  uint16_t slot = rd16(dp + 0x8C);
  while(rd16(dp + 0x8E) != slot) platform_wait_vblank();
  text_type_step(dp);
  shop_panel_upload(dp);
  wr16(dp + 0x3E, 0);
  wr16(dp + 0x40, 0);
  wr16(dp + 0x42, 0);
  wr16(dp + 0x4A, rd16(dp + 0x4C));
  shop_item_dispatch_panel(dp);
  shop_item_text(dp);
}

// TG3000_ROUTINE($80:9AC8) shop_item_slide_row
void shop_item_slide_row(uint16_t dp) {
  wr16(dp + 0x4E, 0x0018);
  do {
    shop_slide_wait(dp);
    wr16(dp + 0x14, 0x0010);
    wr16(dp + 0x4E, (uint16_t)(rd16(dp + 0x4E) - 6));
    wr16(dp + 0x16, rd16(dp + 0x4E));
    shop_slide_draw(dp);
  } while(!((uint16_t)(rd16(dp + 0x4E) - 0xFFC0) & 0x8000));
  shop_slide_swap(dp);
  wr16(dp + 0x4E, 0xFFC2);
  do {
    shop_slide_wait(dp);
    wr16(dp + 0x4E, (uint16_t)(rd16(dp + 0x4E) + 6));
    wr16(dp + 0x14, rd16(dp + 0x4E));
    wr16(dp + 0x16, 0x0018);
    shop_slide_draw(dp);
  } while(rd16(dp + 0x4E) != 0x0010);
}

// TG3000_ROUTINE($80:9B99) shop_item_slide_list_in
void shop_item_slide_list_in(uint16_t dp) {
  shop_slide_swap(dp);
  wr16(dp + 0x4E, 0xFFCA);
  do {
    shop_slide_wait(dp);
    wr16(dp + 0x14, 0x0010);
    wr16(dp + 0x4E, (uint16_t)(rd16(dp + 0x4E) + 6));
    wr16(dp + 0x16, rd16(dp + 0x4E));
    shop_slide_draw(dp);
  } while(rd16(dp + 0x4E) != 0x0018);
}

// TG3000_ROUTINE($80:9B5D) shop_item_slide_list
void shop_item_slide_list(uint16_t dp) {
  wr16(dp + 0x4E, 0x0010);
  do {
    shop_slide_wait(dp);
    wr16(dp + 0x4E, (uint16_t)(rd16(dp + 0x4E) - 6));
    wr16(dp + 0x14, rd16(dp + 0x4E));
    wr16(dp + 0x16, 0x0018);
    shop_slide_draw(dp);
  } while(rd16(dp + 0x4E) != 0xFFC2);
  shop_item_slide_list_in(dp);
}

// leaving the list: returns true (the original drops its return address and returns from
// its caller)
static bool shop_item_leave(uint16_t dp) {
  wr16(dp + 0x44, (uint16_t)(rd16(dp + 0x44) | 0x8000));
  shop_panel_blank(dp);
  while(rd16(dp + 0x4E) <= 0x0004) platform_wait_vblank();
  shop_panel_clear(dp);
  return true;
}

// TG3000_ROUTINE($80:8FA1) shop_item_down
bool shop_item_down(uint16_t dp) {
  uint16_t last = (uint16_t)((rd16(0x036E + (uint32_t)rd16(dp + 0x48)) & 0x00FF) - 1);
  if(last == rd16(dp + 0x4A)) return shop_item_leave(dp);
  func_80E4C5();
  wr16(dp + 0x4C, (uint16_t)(rd16(dp + 0x4A) + 1));
  shop_item_picture(dp);
  shop_item_slide_list(dp);
  return false;
}

// TG3000_ROUTINE($80:8FD4) shop_item_up
bool shop_item_up(uint16_t dp) {
  func_80E4C5();
  if(rd16(dp + 0x4A) == 0) return shop_item_leave(dp);
  wr16(dp + 0x4C, (uint16_t)(rd16(dp + 0x4A) - 1));
  shop_item_picture(dp);
  shop_item_slide_row(dp);
  return false;
}

// TG3000_ROUTINE($80:9832) shop_item_show_icon
void shop_item_show_icon(uint16_t dp) {
  wr16(dp + 0x76, (uint16_t)(rd16(dp + 0x76) | 0x8000));
  wr16(dp + 0x44, (uint16_t)(rd16(dp + 0x44) & ~0x8000));
  do {
    wait_frames(1);
    shop_item_redraw(dp);
  } while(!(rd16(dp + 0x44) & 0x8000));
}

// TG3000_ROUTINE($80:98C4) shop_item_affordable
//
// The original subtracts in decimal mode (CMP, then SBC with D set): for BCD amounts its
// carry is the 32-bit comparison.
bool shop_item_affordable(uint16_t dp) {
  uint16_t entry = shop_item_entry(dp, rd16(dp + 0x4A));
  const uint32_t price = 0x81A892 + (uint32_t)rom16(0x81A89B + (uint32_t)entry);
  uint32_t money = (uint32_t)rd16(dp + 0x54) << 16 | rd16(dp + 0x52);
  uint32_t cost = (uint32_t)rom16(price - 4) << 16 | rom16(price - 2);
  if(money >= cost) return true;
  for(uint32_t x = 0; x < 0x20; x += 2) {
    wait_frames(1);
    const uint32_t y = rd16(dp + 0xC4);
    wr16(0x01C2 + y, rd16(0x4DDA + x));
    wr16(0x01C4 + y, rd16(0x4DFA + x));
    shop_item_redraw(dp);
  }
  return false;
}

// TG3000_ROUTINE($80:9870) shop_item_icon_settle
void shop_item_icon_settle(uint16_t dp) {
  uint16_t step;
  do {
    wait_frames(1);
    shop_item_redraw(dp);
    const uint32_t x = (uint16_t)(rd16(dp + 0x44) << 1), y = rd16(dp + 0xC4);
    wr16(0x01EE + y, rd16(0x4EDA + x));
    wr16(0x01F0 + y, rd16(0x4EFA + x));
    if(x < 0x0010) {  // never run: the icon's other colours, as shop_item_icon_draw's
      wr16(0x0182 + y, rd16(0x4F1A + x));
      wr16(0x0186 + y, rd16(0x4F2A + x));
      wr16(0x0188 + y, rd16(0x4F3A + x));
      wr16(0x018E + y, rd16(0x4F4A + x));
    }
    step = rd16(dp + 0x44);
    if(step == 0x8007) step++;  // never run: it holds there
    step--;
    wr16(dp + 0x44, step);
  } while(step != 0x8007);
  wr16(dp + 0x76, (uint16_t)(rd16(dp + 0x76) & ~0x8000));
}

// TG3000_ROUTINE($80:B2F2) shop_list_label
uint16_t shop_list_label(uint16_t dp, uint16_t x) {
  int16_t row = (int16_t)rd16(dp + 0x22);
  if((int16_t)(row - 0x1000) >= 0 && (int16_t)(row - 0x5800) < 0)
    sprite_text_centered(0x800000 | rom16(0x80B371 + (uint32_t)x), (uint16_t)(rd16(dp + 0x22) + rd16(dp + 0xE7)));
  wr16(dp + 0x22, (uint16_t)(rd16(dp + 0x22) + 0x0800));
  x = (uint16_t)(x + 2);
  if(x == 0x0020) return x;
  if(rd8(0x036E + (uint32_t)(x >> 1)) != 0) return x;
  wr16(dp + 0xF0, (uint16_t)(x >> 1));
  return 0x001E;
}

// TG3000_ROUTINE($80:8EB4) shop_item_screen
//
// A shop list's item screen: on the list D+$F0 names, a fade out instead. Else it opens,
// then runs a frame at a time until down past the last item or up past the first leaves it
// (shop_item_down/up: the buy handlers never leave, checked in the listing).
void shop_item_screen(uint16_t dp) {
  if(!shop_item_screen_open(dp)) return;
  shop_panel_clear(dp);
  shop_item_dispatch_upload(dp);
  shop_item_slide_list_in(dp);
  const uint16_t typed = rd16(dp + 0x8C);
  while(typed != rd16(dp + 0x8E)) platform_wait_vblank();
  shop_panel_upload(dp);
  for(;;) {
    shop_item_frame_view(dp);
    const uint16_t was = rd16(dp + 0x4E);
    while(rd16(0x0024) == was) platform_wait_vblank();
    shop_item_frame_draw(dp);
    // up/down on the pad move the cursor, A/B/X/Y (D+$0E) buy; down past the last item or up
    // past the first leaves the screen (the original drops its return address there: port
    // request Q)
    if((rd16(dp + 0x0C) & 0x0900) && shop_item_down(dp)) return;
    if((rd16(dp + 0x0C) & 0x0600) && shop_item_up(dp)) return;
    const uint16_t press = rd16(dp + 0x0E);
    wr16(dp + 0x0E, (uint16_t)(press & ~0xC0C0));
    if(press & 0xC0C0) shop_buy_dispatch(dp);
    wait_frames(1);
    shop_item_frame_tail(dp);
  }
}

// (the part up to shop_panel_clear: its row is shop_item_screen's entry with endAt $80:8F3B)
// The picture, the two sprite groups, the panel words cleared, the list's $20-byte record
// from $80:B2D2 (MVN into bank $81 at D+$C4 + $80), the list's graphic decompressed to
// $7F:(D+$5E + $0900), the panel blanked; D+$4E set from the frame counter once it is past
// 4. False on the fade-out path.
bool shop_item_screen_open(uint16_t dp) {
  if(rd16(dp + 0x48) == rd16(dp + 0xF0)) {
    func_80E4DB();
    wr16(dp + 0x96, 0xFF80);
    wr16(dp + 0x98, 0);
    return false;
  }
  func_80E4C5();
  wr16(dp + 0x4C, 0);
  shop_item_picture(dp);
  func_809DD0();
  func_808FFC();
  oam_finish_near(dp);
  func_808FFC();
  func_809DD0();
  oam_finish_near(dp);
  wr16(dp + 0x3E, 0);
  wr16(dp + 0x40, 0);
  wr16(dp + 0x42, 0);
  wr16(dp + 0x44, 0);
  const uint16_t rec = (uint16_t)(rd16(dp + 0xC4) + 0x0080);
  for(uint16_t i = 0; i < 0x20; i++) wr8((uint16_t)(rec + i), rom8(0x80B2D2 + i));
  while(rd16(dp + 0x8C) != rd16(dp + 0x8E)) platform_wait_vblank();
  wait_frames(1);
  shop_panel_blank(dp);
  const uint16_t list = rom16(0x81A892 + (uint32_t)(uint16_t)(rd16(dp + 0x48) << 1));
  wr16(dp + 0x14, rom8(0x81A894 + (uint32_t)list));
  uint16_t end;
  decompress_near(dp, (uint8_t)rd16(dp + 0x14), rom16(0x81A892 + (uint32_t)list), 0x7F, (uint16_t)(rd16(dp + 0x5E) + 0x0900), &end);
  wr16(dp + 0x4A, 0);
  while(rd16(dp + 0x4E) <= 0x0004) platform_wait_vblank();
  wr16(dp + 0x4E, rd16(0x0024));
  return true;
}

// TG3000_ROUTINE($80:8F4D) shop_item_frame_view
// A frame's first part: the price (at 16, 24) and the item's view.
void shop_item_frame_view(uint16_t dp) {
  wr16(dp + 0x14, 0x0010);
  wr16(dp + 0x16, 0x0018);
  shop_item_price(dp);
  shop_item_dispatch_view(dp);
}

// TG3000_ROUTINE($80:8F64) shop_item_frame_draw
// After the frame counter moved past D+$4E: the icon, the level marks, the sprites.
void shop_item_frame_draw(uint16_t dp) {
  shop_item_dispatch_icon(dp);
  shop_level_marks(dp);
  func_809DD0();
  oam_finish_near(dp);
}

// TG3000_ROUTINE($80:8F94) shop_item_frame_tail
// With the text typed (D+$8C = D+$8E), its next step and the panel upload ($80:8F4A).
void shop_item_frame_tail(uint16_t dp) {
  if(rd16(dp + 0x8C) == rd16(dp + 0x8E)) {
    text_type_step(dp);
    shop_panel_upload(dp);
  }
}

// The shop's main screen ($80:8BC5: the list of upgrade kinds, a frame at a time) in parts,
// the waits and the frame loop between them still in the original's hands:

// TG3000_ROUTINE($80:8BC5) shop_screen_begin
// The fade level cleared and the screen's image loaded.
void shop_screen_begin(void) {
  wr16(0x019A, 0);
  video_load_screen(0x84A26F);
}

// TG3000_ROUTINE($80:8BD7) shop_screen_graphics
// After a frame: the panel graphics to $7E:2000, four tile sets to VRAM, and VRAM words
// $0000-$000F cleared.
void shop_screen_graphics(uint16_t dp) {
  uint16_t end;
  decompress_near(dp, 0x81, 0x8B62, 0x7E, 0x2000, &end);
  decompress_to_vram(dp, 0x8D, 0x8000, 0x0010);
  decompress_to_vram(dp, 0x88, 0x8000, 0x1310);
  decompress_to_vram(dp, 0x87, 0x8190, 0x2000);
  decompress_to_vram(dp, 0x8D, 0x895F, 0x2500);
  for(uint16_t i = 0; i < 0x10; i++) video_write_vram_word(i, 0x0000);
}

// TG3000_ROUTINE($80:8C22) shop_screen_records
// After split_screen_setup: the car's record (by D+$92, from $81:8B35), three MVNs into bank
// $81 at D+$C4 ($40 bytes from $9C:8000, $40 from $9D:9ECA, $20 from $94:8000 at + $180) and
// ten colour words. (Both players' tasks run it: D = $0100 or $0200.)
void shop_screen_records(uint16_t dp) {
  const uint32_t rec = 0x818B35 + (uint32_t)rd16((uint16_t)(dp + 0x92));
  wr16((uint16_t)(dp + 0xB8), rom16(rec));
  wr16((uint16_t)(dp + 0xB4), (uint16_t)(rom16(rec + 3) + 4));
  wr16((uint16_t)(dp + 0x5C), rom16(rec + 5));
  wr8((uint16_t)(dp + 0xAD), rom8(rec + 2));
  wr16((uint16_t)(dp + 0x60), rom16(rec + 7));
  wr16((uint16_t)(dp + 0x62), rom16(rec + 9));
  const uint16_t c4 = rd16((uint16_t)(dp + 0xC4));
  for(uint16_t i = 0; i < 0x40; i++) wr8((uint16_t)(c4 + i), rom8(0x9C8000 + i));
  for(uint16_t i = 0; i < 0x40; i++) wr8((uint16_t)(c4 + 0x40 + i), rom8(0x9D9ECA + i));
  for(uint16_t i = 0; i < 0x20; i++) wr8((uint16_t)(c4 + 0x0180 + i), rom8(0x948000 + i));
  static const uint16_t colours[10][2] = {
    {0x0122, 0x0C70}, {0x0124, 0x0C69}, {0x0142, 0x109F}, {0x0144, 0x108C}, {0x01C2, 0x13FF},
    {0x01C4, 0x1210}, {0x01A2, 0x7FFF}, {0x01A4, 0x1210}, {0x01E2, 0x7F9C}, {0x01E4, 0x4210},
  };
  for(int i = 0; i < 10; i++) wr16((uint16_t)(c4 + colours[i][0]), colours[i][1]);
}

// TG3000_ROUTINE($80:8CB9) shop_screen_text_setup
// After the car graphic's upload: the typed text's and the ticker's words (D: the player's).
void shop_screen_text_setup(uint16_t dp) {
  wr16((uint16_t)(dp + 0xAE), 0x554D);
  wr16((uint16_t)(dp + 0xB0), 0x0F00);
  wr16((uint16_t)(dp + 0xBA), 0x000A);
  wr8((uint16_t)(dp + 0xA9), 0x08);
  wr16((uint16_t)(dp + 0xBE), 0x1011);
  wr16((uint16_t)(dp + 0xB2), 0);
  wr16((uint16_t)(dp + 0xBC), 0x1013);
  wr16((uint16_t)(dp + 0xC0), 0x0002);
}

// TG3000_ROUTINE($80:8D1A) shop_screen_list_setup
// The fade in, $03E0 set, the sprite group for the track's place ($85:D530/$85:D55F by
// $03DC's bits 8-9, with $036C: $D60E without one), the list words cleared and its panel
// built ($7F:(D+$5E), $3C0 bytes to D+$5C + $400).
void shop_screen_list_setup(uint16_t dp) {
  wr16(dp + 0x98, 0x0F00);
  wr16(dp + 0x96, 0x0100);
  wr16(0x03E0, 0x0001);
  wr16(dp + 0x50, 0xD60E);
  const uint16_t track = rd16(0x0444), mode = (uint16_t)(rd16(0x03DC) & 0x0300);
  uint16_t got, base;
  if(mode == 0) {  // never run
    got = rom16(0x85D501 + (uint32_t)track);
    base = 0x0000;
  } else if(mode == 0x0100) {
    got = rom16(0x85D530 + (uint32_t)track);
    base = 0x001C;
  } else {
    got = rom16(0x85D55F + (uint32_t)track);
    base = 0x0044;
  }
  if(got & 0x00FF) wr16(dp + 0x50, rom16(0x85D58E + (uint32_t)(uint16_t)(base + rd16(0x036C))));
  func_80B556(0x85, rd16(dp + 0x50), (uint16_t)(rd16(dp + 0x5E) + 0x0118));
  wr16(dp + 0x48, 0);
  wr16(dp + 0x4A, 0);
  wr16(dp + 0x4C, 0);
  wr16(dp + 0x4E, 0);
  wr16(dp + 0xEE, 0);
  wr16(dp + 0x14, 0x007F);
  func_80B537(dp, 0x03C0, rd16(dp + 0x5E), (uint16_t)(rd16(dp + 0x5C) + 0x0400));
}

// TG3000_ROUTINE($80:8DA0) shop_screen_frame
// A frame of the shop's main screen: the list's labels (from D+$4A + $2830, the cursor's
// line D+$48 lit), D+$4A scrolled a line toward the cursor, the sprites.
void shop_screen_frame(uint16_t dp) {
  uint16_t x = 0;
  wr16(dp + 0x22, (uint16_t)(rd16(dp + 0x4A) + 0x2830));
  wr16(dp + 0x20, rd16(dp + 0x48));
  if(rd16(dp + 0x20) != 0) {
    do {
      wr16(dp + 0x14, 0x2200);
      x = shop_list_label(dp, x);
      wr16(dp + 0x20, (uint16_t)(rd16(dp + 0x20) - 1));
    } while(rd16(dp + 0x20) != 0);
  }
  wr16(dp + 0x14, 0x2400);
  x = shop_list_label(dp, x);
  while(x != 0x0020) {
    wr16(dp + 0x14, 0x2200);
    x = shop_list_label(dp, x);
  }
  uint16_t a = (uint16_t)(rd16(dp + 0x48) << 3);
  a = (uint16_t)((uint16_t)(a << 8 | a >> 8) + rd16(dp + 0x4A));
  if(a & 0x8000) wr16(dp + 0x4A, (uint16_t)(rd16(dp + 0x4A) + 0x0100));
  else if(a != 0x1800 && !((uint16_t)(a - 0x1800) & 0x8000)) wr16(dp + 0x4A, (uint16_t)(rd16(dp + 0x4A) - 0x0100));
  func_809DD0();
  func_808FFC();
  oam_finish_near(dp);
}

// TG3000_ROUTINE($80:8E12) shop_screen_after_items
// Back from a list's item screen: D+$4A as it was (`saved`, which the original keeps on the
// stack and pulls just before), the sprite group again and the two colours.
void shop_screen_after_items(uint16_t dp, uint16_t saved) {
  wr16(dp + 0x4A, saved);
  func_80B58D(0x85, rd16(dp + 0x50));
  const uint16_t c4 = rd16(dp + 0xC4);
  wr16((uint16_t)(c4 + 0x0122), 0x0C70);
  wr16((uint16_t)(c4 + 0x0124), 0x0C69);
}

// TG3000_ROUTINE($80:8E2A) shop_screen_frame_end
// The frame's end: up/down pressed (D+$0E) move the cursor, held (D+$0C) repeat every 8
// frames after 24 (never run); the cursor's colour cycles ($80:B400 by D+$4E); the text
// typed on. Returns its CMP's carry (D+$8C at or past D+$8E).
bool shop_screen_frame_end(uint16_t dp) {
  uint16_t press = rd16(dp + 0x0E);
  wr16(dp + 0x0E, (uint16_t)(press & ~0x0800));
  if(press & 0x0800) {
    wr16(dp + 0xEA, 0x0018);
    shop_list_prev(dp);
  }
  press = rd16(dp + 0x0E);
  wr16(dp + 0x0E, (uint16_t)(press & ~0x0400));
  if(press & 0x0400) {
    wr16(dp + 0xEA, 0x0018);
    shop_list_next(dp);
  }
  if(rd16(dp + 0x0C) & 0x0C00) {
    wr16(dp + 0xEA, (uint16_t)(rd16(dp + 0xEA) - 1));
    if(rd16(dp + 0xEA) == 0) {  // never run
      wr16(dp + 0xEA, 0x0008);
      if(rd16(dp + 0x0C) & 0x0800) shop_list_prev(dp);
      if(rd16(dp + 0x0C) & 0x0400) shop_list_next(dp);
    }
  }
  wr16((uint16_t)(rd16(dp + 0xC4) + 0x0142), rom16(0x80B400 + (uint32_t)((rd16(dp + 0x4E) << 1) & 0x003E)));
  wr16(dp + 0x4E, (uint16_t)(rd16(dp + 0x4E) + 1));
  const bool carry = rd16(dp + 0x8C) >= rd16(dp + 0x8E);
  if(rd16(dp + 0x8C) == rd16(dp + 0x8E)) text_type_step(dp);
  return carry;
}

// TG3000_ROUTINE($80:8E91) shop_screen_close
// Leaving the shop: bit 7 cleared in the player's ten upgrade bytes ($0393 + D+$94), D+$76
// waited for, the NMI and IRQ targets back to the race's ($8934, $89B2), interrupts masked
// (SEI: a comment, as elsewhere).
static void shop_screen_leave(uint16_t dp) {  // ($80:8EA3)
  wait_flag_76(dp);
  wr16(0x0008, 0x8934);
  wr16(0x0005, 0x89B2);
}
void shop_screen_close(uint16_t dp) {
  for(uint16_t i = 0, x = rd16(dp + 0x94); i < 10; i++, x++)
    wr16((uint16_t)(0x0393 + x), (uint16_t)(rd16((uint16_t)(0x0393 + x)) & 0xFF7F));
  shop_screen_leave(dp);
}

// TG3000_ROUTINE($80:8BC5) shop_screen
//
// The shop's main screen, whole: the parts above with the waits between them, then the
// frame loop until D+$9A's low 15 bits are 0. A/B/X/Y or Select (D+$0E bits $C1C0) opens
// the list's item screen.
// After split_screen_setup, each player's half (port request D): `dp` is the task's D.
static void shop_task(uint16_t dp) {
  shop_screen_records(dp);
  uint16_t end;
  queue_unpacked_upload(dp, 0x87, 0x8000, rd16(dp + 0x5C), &end);
  shop_screen_text_setup(dp);
  while(rd16(dp + 0x64) != 0) platform_wait_vblank();
  wait_frames(1);
  while(rd16(dp + 0x64) != 0) platform_wait_vblank();
  for(uint16_t i = 0, x = rd16(dp + 0x5E); i < 0x01E0; i++, x = (uint16_t)(x + 2)) wr16(0x7F0000 + (uint32_t)x, 0);
  wr16(dp + 0x9C, 0x0004);
  wr16(dp + 0x9E, 0x0005);
  wait_frames(1);
  if(rd16(dp + 0x76) & 0x0020) {  // (straight to the leaving, the upgrade bytes kept)
    shop_screen_leave(dp);
    return;
  }
  {
    shop_screen_list_setup(dp);  // (its tail, from $80:8D86, is the loop's head)
    for(;;) {
      wait_frames(1);
      shop_screen_frame(dp);
      const uint16_t press = rd16(dp + 0x0E);
      wr16(dp + 0x0E, (uint16_t)(press & ~0xC1C0));
      if(press & 0xC1C0) {
        const uint16_t saved = rd16(dp + 0x4A);
        shop_item_screen(dp);
        shop_screen_after_items(dp, saved);
      }
      shop_screen_frame_end(dp);
      if(!(rd16(dp + 0x9A) & 0x7FFF)) break;
      wr16(dp + 0x14, 0x007F);
      func_80B537(dp, 0x03C0, rd16(dp + 0x5E), (uint16_t)(rd16(dp + 0x5C) + 0x0400));
    }
  }
  shop_screen_close(dp);
}

static void shop_task2(uint16_t dp) { shop_task(dp); }

void shop_screen(void) {
  shop_screen_begin();
  wait_frames(1);
  shop_screen_graphics(DP);
  split_screen_setup(0x8C21, 0x81);
  platform_task_split(shop_task2);  // player 2's half
  shop_task(DP);
  platform_task_join();
}
