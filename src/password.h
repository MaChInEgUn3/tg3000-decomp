// SPDX-License-Identifier: GPL-3.0-or-later
// The password screen ($8A:B1xx-B6xx): its letter grid and the caret under the typed text.
#ifndef TG3000_PASSWORD_H
#define TG3000_PASSWORD_H

#include <stdint.h>

// The grid's cursor: $0D88 (x) and $0D90 (y) are where it goes; $0D80 and $0D68 are where it
// is, gliding after them. Columns step $10 (x from $FF8C), the last column (SPACE, DEL,
// EXIT, at x $64) is $28 on; rows step $18 from y $64 to $AC (the row of three: . ? !).
// $0D78 is its sprite's width: 6, or $26 on the last column.

// $8A:B4D2/B4FF/B52C/B559: a press of right/left/up/down on any of the four pads (the new
// presses at $000E, $0012, $0016, $001A) moves the cursor one place, once it has stopped
// (where it goes is where it is). Each returns whether a press was seen.
void password_pad_right(void);
void password_pad_left(void);
void password_pad_up(void);
void password_pad_down(void);
// $8A:B586/B5BF/B5E2/B5F2: one place right/left/up/down, held at the grid's edges. The row of
// three is reached only from its own columns.
void password_cursor_right(void);
void password_cursor_left(void);
void password_cursor_up(void);
void password_cursor_down(void);

// $8A:B612: the screen's four lines of text ($8A:C4F6, C515, C534, C553), centred on rows $0D,
// $10, $13, $16 (D+$68 = 0 first).
void password_lines_print(void);
// $8A:B644: the caret (object 0 at $0578/$0678/$0778) under the text cell (D+$3A, D+$3C): x
// = column * 8, y = row * 8 + $0F.
void password_caret_place(void);
// $8A:B2BC: the caret blinks: shown ($0578 = 0) while D+$66 is odd, else hidden ($FFFF).
void password_caret_blink(void);

#endif
