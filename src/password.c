// SPDX-License-Identifier: GPL-3.0-or-later
#include "password.h"

#include <stdbool.h>

#include "misc.h"
#include "text.h"
#include "tg3000.h"

#define CURSOR_X 0x0D88
#define CURSOR_Y 0x0D90
#define CURSOR_AT_X 0x0D80
#define CURSOR_AT_Y 0x0D68
#define CURSOR_WIDTH 0x0D78

// Whether `button` is a new press on any of the four pads.
static bool any_pad_pressed(uint16_t button) {
  return (button & rd16(0x000E)) != 0 || (button & rd16(0x0012)) != 0 || (button & rd16(0x0016)) != 0 ||
         (button & rd16(0x001A)) != 0;
}

// TG3000_ROUTINE($8A:B4D2) password_pad_right
void password_pad_right(void) {
  if(any_pad_pressed(0x0100) && rd16(CURSOR_X) == rd16(CURSOR_AT_X)) password_cursor_right();
}
// TG3000_ROUTINE($8A:B4FF) password_pad_left
void password_pad_left(void) {
  if(any_pad_pressed(0x0200) && rd16(CURSOR_X) == rd16(CURSOR_AT_X)) password_cursor_left();
}
// TG3000_ROUTINE($8A:B52C) password_pad_up
void password_pad_up(void) {
  if(any_pad_pressed(0x0800) && rd16(CURSOR_Y) == rd16(CURSOR_AT_Y)) password_cursor_up();
}
// TG3000_ROUTINE($8A:B559) password_pad_down
void password_pad_down(void) {
  if(any_pad_pressed(0x0400) && rd16(CURSOR_Y) == rd16(CURSOR_AT_Y)) password_cursor_down();
}

// The tests are the original's: a CMP and the sign of the difference (BPL), not a signed
// compare.
static bool not_below(uint16_t a, uint16_t b) { return !((uint16_t)(a - b) & 0x8000); }

// TG3000_ROUTINE($8A:B586) password_cursor_right
void password_cursor_right(void) {
  const uint16_t x = rd16(CURSOR_X);
  if(rd16(CURSOR_Y) == 0x00AC && not_below(x, 0xFFA4)) return;  // past `!`
  if(!not_below(x, 0x003C)) {
    wr16(CURSOR_X, (uint16_t)(x + 0x10));
    wr16(CURSOR_WIDTH, 0x0006);
  } else if(x != 0x0064) {  // onto the last column
    wr16(CURSOR_X, (uint16_t)(x + 0x28));
    wr16(CURSOR_WIDTH, 0x0026);
  }
}
// TG3000_ROUTINE($8A:B5BF) password_cursor_left
void password_cursor_left(void) {
  const uint16_t x = rd16(CURSOR_X);
  if(x == 0x0064) {  // off the last column
    wr16(CURSOR_X, (uint16_t)(x - 0x28));
    wr16(CURSOR_WIDTH, 0x0006);
  } else if(x != 0xFF8C) {
    wr16(CURSOR_X, (uint16_t)(x - 0x10));
  }
}
// TG3000_ROUTINE($8A:B5E2) password_cursor_up
void password_cursor_up(void) {
  const uint16_t y = rd16(CURSOR_Y);
  if(y != 0x0064) wr16(CURSOR_Y, (uint16_t)(y - 0x18));
}
// TG3000_ROUTINE($8A:B5F2) password_cursor_down
void password_cursor_down(void) {
  const uint16_t y = rd16(CURSOR_Y);
  if(y == 0x0094 && not_below(rd16(CURSOR_X), 0xFFB4)) return;  // no fourth row under these
  if(y != 0x00AC) wr16(CURSOR_Y, (uint16_t)(y + 0x18));
}

#define TYPED 0x0D40
#define TYPED_MAX 0x15

// TG3000_ROUTINE($8A:B403) password_type_char
void password_type_char(uint8_t ch) {
  const uint16_t count = rd16(DP + 0x80);
  if(count == TYPED_MAX) {
    func_8AA4AF();
    return;
  }
  wr8(TYPED + (uint32_t)count, ch);
  wr8(DP + 0x80, (uint8_t)(rd8(DP + 0x80) + 1));  // an 8-bit INC
  password_text_draw();
  func_8AA48D();
}

// TG3000_ROUTINE($8A:B420) password_delete
void password_delete(void) {
  if(rd16(DP + 0x80) == 0) {
    func_8AA4AF();
    return;
  }
  wr16(DP + 0x80, (uint16_t)(rd16(DP + 0x80) - 1));
  wr8(TYPED + (uint32_t)rd16(DP + 0x80), 0);
  password_text_draw();
  func_8AA48D();
}

// TG3000_ROUTINE($8A:B43D) password_text_draw
void password_text_draw(void) {
  wr16(DP + 0x3A, 2);
  if(rd16(DP + 0x80) != 0) {
    wr16(DP + 0x48, 0);
    uint16_t i = 0;
    do {
      text_put_char(rd8(TYPED + (uint32_t)i));
      wr16(DP + 0x48, (uint16_t)(rd16(DP + 0x48) + 1));
      if((rd16(DP + 0x48) & 3) == 0 && password_no_vowel()) text_put_char(' ');
      i++;
    } while(i != rd16(DP + 0x80));
  }
  text_put_char(' ');
  wr16(DP + 0x3A, (uint16_t)(rd16(DP + 0x3A) - 1));
  password_caret_place();
}

// TG3000_ROUTINE($8A:B47C) password_no_vowel
bool password_no_vowel(void) {
  for(int i = 3; i >= 0; i--) {
    const uint8_t ch = rd8(TYPED + (uint32_t)i);
    if(ch == 'A' || ch == 'E' || ch == 'I' || ch == 'O' || ch == 'U') return false;
  }
  return true;
}

// TG3000_ROUTINE($8A:B4A4) password_press_last_column
bool password_press_last_column(void) {
  if(rd16(CURSOR_AT_X) != 0x0064) return false;
  switch(rd16(CURSOR_AT_Y)) {
    case 0x0094:  // EXIT
      func_8AA49E();
      return true;
    case 0x007C:  // DEL (never pressed in a movie)
      password_delete();
      return false;
    case 0x0064:  // SPACE (never pressed in a movie)
      password_type_char(' ');
      return false;
    default:
      return false;
  }
}

// TG3000_ROUTINE($8A:B339) password_press_a
bool password_press_a(void) {
  bool pressed = false;
  for(uint32_t pad = 0x000E; pad <= 0x001A && !pressed; pad += 4) {  // TRB: the bits written off
    const uint16_t v = rd16(pad);
    wr16(pad, (uint16_t)(v & ~0xC0C0));
    pressed = (v & 0xC0C0) != 0;
  }
  if(!pressed) return false;
  if(password_press_last_column()) return true;
  const uint16_t at = (uint16_t)(rd16(CURSOR_AT_X) + 0x74);  // the column, from x $FF8C
  if(at & 0x000F) return false;                               // not on a letter's column
  if(not_below(rd16(CURSOR_AT_X), 0x003D)) return false;
  static const uint16_t rows[4] = {0x0064, 0x007C, 0x0094, 0x00AC};
  for(uint32_t row = 0; row < 4; row++)
    if(rd16(CURSOR_Y) == rows[row]) password_type_char(rom8(0x8AC572 + 12 * row + (uint32_t)(at >> 4)));
  return false;
}

// A character's index in the password alphabet ($8A:BC5F, 32 letters). The ROM searches on
// until a byte matches, through whatever follows the table, and so does this.
static uint16_t alphabet_index(uint8_t ch) {
  uint16_t y = 0;
  while(bus_rd8(0x8ABC5F + (uint32_t)y) != ch) y++;
  return y;
}

// The typed text's byte at `x` (an index the ROM keeps in 16 bits: below 0 it reads below
// $0D40, through the bank's WRAM mirror).
static uint8_t typed_at(uint16_t x) { return rd8((uint16_t)(TYPED + x)); }

// TG3000_ROUTINE($8A:BCEA) password_decode
void password_decode(void) {
  wr16(DP + 0x48, 0);
  wr16(DP + 0x92, 0);
  func_8ABC7F();
  wr16(DP + 0x84, 0);
  uint16_t x = 0;
  while(typed_at(x) != 0) x++;
  x = (uint16_t)(x - 3);  // the last character before the two of the checksum
  do {
    func_8ABBF4(alphabet_index(typed_at(x)), 5);
    x--;
  } while(!(x & 0x8000));
}

// TG3000_ROUTINE($8A:BD25) password_checksum_ok
bool password_checksum_ok(void) {
  wr16(DP + 0x92, (uint16_t)(rd16(DP + 0x92) & 0x03FF));
  uint16_t x = 0;
  while(typed_at((uint16_t)(x + 2)) != 0) x++;
  wr16(DP + 0x42, alphabet_index(typed_at(x)));
  wr16(DP + 0x42, (uint16_t)((uint16_t)(alphabet_index(typed_at((uint16_t)(x + 1))) << 5) | rd16(DP + 0x42)));
  return rd16(DP + 0x42) == rd16(DP + 0x92);
}

// TG3000_ROUTINE($8A:B612) password_lines_print
void password_lines_print(void) {
  static const uint16_t lines[4] = {0xC4F6, 0xC515, 0xC534, 0xC553};
  wr16(DP + 0x68, 0);
  wr16(DP + 0x3C, 0x000D);
  for(int i = 0; i < 4; i++) {
    if(i) wr16(DP + 0x3C, (uint16_t)(rd16(DP + 0x3C) + 3));
    text_print_centered(0x8A0000 | lines[i]);
  }
}

// TG3000_ROUTINE($8A:B644) password_caret_place
void password_caret_place(void) {
  wr16(0x0678, (uint16_t)(rd16(DP + 0x3A) << 3));
  wr16(0x0778, (uint16_t)((uint16_t)(rd16(DP + 0x3C) << 3) + 0x000F));
}

// TG3000_ROUTINE($8A:B2BC) password_caret_blink
void password_caret_blink(void) { wr16(0x0578, (rd16(DP + 0x66) & 1) ? 0 : 0xFFFF); }
