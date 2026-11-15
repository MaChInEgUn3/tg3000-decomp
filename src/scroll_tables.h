// SPDX-License-Identifier: GPL-3.0-or-later
// The race's per-line scroll tables: indirect HDMA tables (entries of a line count and a
// pointer) that channels 5 and 6 feed to $210D and $210F (docs/DECISIONS.md, "HDMA,
// measured").
#ifndef TG3000_SCROLL_TABLES_H
#define TG3000_SCROLL_TABLES_H

#include <stdint.h>

// Rotate the table buffers ($00E0 <-> $00E4; $1E2E/$1E32/$1E36 and $1E30/$1E34/$1E38
// each one step) and build the two tables into the buffers that come up ($00D0, $00D2),
// from the end backwards, three bytes per entry: the line count (bit 7 set for a repeat
// run) and a 16-bit pointer. Each table's lines are split, from the top, by $0242 /
// $1EF2 (a negative value adds a band), the byte at $7E:4CFE / $7E:4D4A indexed by $0164 /
// $0168 (0 counts as $DC), and the bands at $0237 + $B0 and $0237 + $70; the pointers come
// from $1E3E / $1E40, $00E0 and fixed addresses. Each table ends with an entry of 4
// lines; its first entry's address is stored back in $00D0 / $00D2, and
// $1538 / $1558 + $00E0 = $BB - $0237 - $70. Returns that last value.
uint16_t scroll_tables_build(void);

#endif
