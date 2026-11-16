// SPDX-License-Identifier: GPL-3.0-or-later
// Controllers: the per-frame read of up to five pads (docs/DECISIONS.md, "Controllers").
#ifndef TG3000_PADS_H
#define TG3000_PADS_H

#include <stdint.h>

// Five (held, pressed) word pairs: $000C/$000E controller 1, then the four multitap pads
// at $0010/$0012, $0014/$0016, $0018/$001A, $001C/$001E. "Pressed" collects new presses:
// (pressed | (old ^ new)) & new, so a button stays in it until released (the game clears
// what it has used). A pad whose signature bits (3-0) are not zero, or controller 1 with
// no controller plugged in (bit 0 forced on), reads as held = pressed = 1.
#define PAD_BASE 0x000C
#define PAD_SCRATCH 0x0020  // the last multitap pair read ($0020 and $0022)

// $83:87C1: read a multitap pair into $0020 (first data line) and $0022 (second).
void pads_read_pair(int pair);

// $83:86FF: read all five pads and update their pairs.
void pads_read(void);

// $83:86FB: the same, by a long call.
void pads_read_long(void);
void pads_read_long_2(void);  // $83:96CD, the same

#endif
