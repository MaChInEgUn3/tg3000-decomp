// SPDX-License-Identifier: GPL-3.0-or-later
// The game's data decompressor ($82:FB70). A stream of commands, each a byte (some take
// one or two more):
//   $00          end
//   $01-$3F      copy that many literal bytes
//   $4n          n+3 bytes continuing the byte progression of the last two bytes
//   $5n          n+2 words continuing the word progression of the last two words
//   $6n          repeat the last byte n+3 times
//   $7n          repeat the last word n+2 times
//   $80-$BF      copy 3 bytes from (cmd & $3F) + 3 back
//   $C0-$DF, d   copy 4 + ((cmd >> 2) & 7) bytes from ((cmd & 3) << 8 | d) + 3 back
//   $E0-$FF, d, l  copy l + 5 bytes from ((cmd & $1F) << 8 | d) + 3 back
// Copies move words in a fixed order (see decompress.c), which matters when a copy
// reads bytes it has just written.
#ifndef TG3000_DECOMPRESS_H
#define TG3000_DECOMPRESS_H

#include <stdint.h>

// Unpack the stream at src_bank:src into dst_bank:dst (offsets wrap within the bank).
// `dp` is the direct page the original runs with: it leaves its scratch there ($00-$02
// output pointer, $04 the input position of the end byte, $08 a count, $0A the end
// offset). Returns the number of bytes written; *src_end receives the input position
// just after the end byte.
uint16_t decompress(uint16_t dp, uint8_t src_bank, uint16_t src, uint8_t dst_bank, uint16_t dst, uint16_t* src_end);

// $80:FBA0: the same, for callers in bank $80 (a JSL/RTS bridge). Returns the length.
uint16_t decompress_near(uint16_t dp, uint8_t src_bank, uint16_t src, uint8_t dst_bank, uint16_t dst, uint16_t* src_end);

#endif
