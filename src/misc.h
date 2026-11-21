// SPDX-License-Identifier: GPL-3.0-or-later
// Routines whose purpose is not known yet are named func_<address> until it is.
#ifndef TG3000_MISC_H
#define TG3000_MISC_H

#include <stdbool.h>
#include <stdint.h>

// $81:F251: an empty routine (a lone RTS) that 19 places call.
void func_81F251(void);

// Clear the direct-page scratch area $0114-$0193 (64 words).
void clear_dp_scratch(void);

// Link the 20-entry order list into "next" pointers: starting from the index in the
// first entry, each entry's value is stored at $FACD + the previous value, and the list
// is closed by storing the first entry at $FACD + the index in $1E90. Most likely the
// race order (20 cars). Returns the first entry.
uint16_t func_81F1FC(void);

// Two objects from the table entry at byte offset `entry` ($0D60 x, $0D68 y): slots come
// from take_slot; the second sits 2 * $014C lower. Advances the entry's x by 8.
void func_8AA216(uint16_t entry);

// A 2-tile sprite at byte offset `slot` of the tables $05C0/$06C0/$07C0/$08C0: x from
// $017A (and x + 16), y from $017C, tiles $04A0/$04A2. Returns slot + 4.
uint16_t func_8A92CF(uint16_t slot);

// Offset of a cell in a 32-column map of words: row ($0138) * 64 + column ($0136) * 2.
// The original doubles the column in place ($0136 <<= 1). Returns the offset.
uint16_t tilemap_offset(void);

// If $013C >= 10: set the word at $0D04 + ($013C - 10) to 4 when $015A is odd, else 0.
// (With $013C < 10 the original returns with Y still on the stack, i.e. through a bogus
// return address; no movie reaches that, and this C does nothing then.)
void func_8AB700(void);

// Move the word at `value` one step towards the word at `target`, up to `steps` times
// (the original's Y), stopping early once they are equal. Returns the steps left;
// `last_diff` receives the last target - value the original computed (its A).
uint16_t step_toward(uint16_t value, uint16_t target, uint16_t steps, uint16_t* last_diff);

// Append byte `b` to the buffer at $7F:7D00 (index $0158, then +1) and count it in the
// byte $0160.
void byte_buffer_push(uint8_t b);

// byte_buffer_push of a space ($20).
void byte_buffer_space(void);

// Push spaces until the byte count $0160 (as a word, with $0161) equals `count`.
void byte_buffer_pad(uint16_t count);

// Push the end marker $80 and reset the count word $0160.
void byte_buffer_end(void);

// Push `value` in decimal as ASCII digits, without leading zeros ("0" for zero). Leaves
// $015A = 1 (a digit was pushed; untouched for zero) and $015C = 0 (the remainder).
void byte_buffer_decimal(uint16_t value);

// $01FB = $11 / $13.
void func_80E4C5(void);
void func_80E4DB(void);

// $01CC = x, byte $01CE = a, $01E3 = 0, $01E1 = 9.
void func_80B58D(uint8_t a, uint16_t x);

// Shift the 48-bit number D+$46 : D+$44 : `low` right by 4 (D+$44 and D+$46 keep the
// result) and return its low word shifted right by 12 more (lsr_n; *carry = the last bit
// out): bits 16-19 of the original number.
uint16_t func_8ABFBC(uint16_t low, bool* carry);

// $0142 = the five BCD digits of $0144 (four nibbles, low first) and $0146 (its low
// nibble) as a binary number (weights 1, 10, 100, 1000, 10000 at $8A:CABA), taking the
// digits off with func_8ABFBC (which shifts $0144-$0146 right by 4 each time). Returns
// the A it leaves: the last sum, or 0 if the last digit was 0.
uint16_t bcd_to_binary(uint16_t a);

// If the byte `b` is not zero: store it at $1B30 + x (an 8-byte ring) and return
// (x + 1) & 7; else return x.
uint16_t func_80899F(uint8_t b, uint16_t x);

// Shift the inverse of bit 0 of `a` into the word $0156 (from the right) and add it to
// the word $0158. Returns the new $0158.
uint16_t func_8AA022(uint16_t a);

// Set to $FFFF each of the seven words $01CF-$01DB that equals $01DD.
void func_80B7BB(void);

// byte_buffer_decimal of $015E, after a space if it is below 10 (signed): two columns.
void byte_buffer_decimal2(void);

// Queue the request bytes $01FB, $01FC, $02FB, $02FC (non-zero ones; func_80899F into the
// ring at $1B30, write index $1B24) and clear them; then, if the ring is not empty, take
// one (read index $1B26): $0463 = it | $FF00, $0465 = $FF00, $0467 = 0 (byte).
void func_80894E(void);

// The 8 decimal digits of `value` as BCD, ones in the low nibble, by dividing by 10 on
// the CPU divider; left in D+$16 (low word) and D+$18 (high word). Returns it.
uint32_t binary_to_bcd(uint16_t value);

// $0156 = $0158 = 0, then func_8AA022 of the words $0018, $0014, $0010, $000C in turn.
// Returns the final $0158.
uint16_t func_8AA005(void);

// 2a + $B990 when 2a < $40, else 2a + $2400.
uint16_t func_80E0C8(uint16_t a);

// Hide every sprite: fill the 512-byte table at $0900 with $E0 and reset the slot
// indices $0172 and $0174.
void oam_clear(void);

// oam_clear, by a long call.
void oam_clear_long(void);

// Two sprites at the slot D+$74 (then advanced by 8): tile $3892 at (D+$EE + D+$E7 + $5A,
// $08) and tile $3890 at (D+$E7 + $5C, $0A), both with $0100 in the extra table at $0B01;
// D+$EE counts down to 0 first. Returns the new D+$74.
uint16_t func_808FFC(void);

// For each non-zero word p of the zero-terminated table at `table` (24-bit; the original
// reads it through DB), func_8AB680 of the list at p + 4 in the same bank. D+$4A = the
// table's address.
void func_8AA4BE(uint32_t table);

// $014A = ($0136 >> 2) + $0108.
uint16_t func_8A8232(void);

// Unless bit 15 of $047C is set: $0463 = a, $0465 = x, $0467 = 0 (byte; $0466 is then
// overwritten by x's high byte).
void func_82AD12(uint16_t a, uint16_t x);

// $0142 in BCD into $0144 (four digits, ones lowest) and $0146 (the ten-thousands), by
// repeated subtraction of the weights at $8A:CABA; $0142 ends as 0.
void binary_to_bcd5(void);

// An empty routine (RTS).
void func_82F380(void);
// Another one.
void func_82FB1C(void);

// Shift the 32-bit numbers $0390:$038E and $03BF:$03BD left (func_8AC03D) or right
// (func_8AC024) by 12 bits, one bit at a time. Return the last bit shifted out of the
// second number.
bool func_8AC03D(void);
bool func_8AC024(void);

// Set the byte $0D23 + i, i = the byte at $8A:C599 + (D+$3C - 10), from the flags byte
// $1516 + x: bit 7 set -> $E0; else bit 6 clear -> $E7; else its low 5 bits from the
// byte at $8A:C5B7 + (D+$6E & $1F) (D+$6E then counts up, as a byte). Returns the byte.
uint8_t func_8AA32B(uint16_t x);

// The first of the words $000E, $0012, $0016, $001A with bit 11 set (else, the first with
// bit 10) has that bit cleared, and then, for the object x: on bit 11, unless the word
// before D+$4A in bank $8A is $FFFF; on bit 10, if the word 8 bytes after D+$4A is not
// zero: clear bits 0 and 6 of $1516 + x, set bits 0 and 6 of $1514 + x (bit 11) or
// $1518 + x (bit 10), zero D+$64, D+$66, D+$6E, and func_8AA48D. Returns 11, 10 or 0
// (no bit found).
int func_8AA381(uint16_t x);

// If D+$54 is not zero, count D+$52 up; every 8th count, D+$64 = the next word of the
// cycle 1, 2, 3, 4, 3, 2 at $8A:82B4 (index D+$50, in bytes, wrapping at 12). Returns the
// value the original leaves in A (0; the count's low 3 bits; or the word).
uint16_t func_8A8290(void);

// $8A:A050 (long; D = $0100): when D+$8E is not 0, upload_7f_to_3000, oam_from_objects
// and count $0164 (every 8th count also $0166); then func_8AA005. Returns its result.
uint16_t func_8AA050(void);

// Start a fade up to full brightness: target $0F00, step $0080 (the NMI moves $019A).
void fade_in_start(void);

// The fade to black: target $0198 = 0, step $0196 = -$80.
// - fade_out_wait ($8A:BAE3): start it, then wait until the NMI has finished it;
// - fade_out_on_confirm ($8A:9675): if controller 1 or the first multitap pad has a new
//   A, B, X or Y press ($C0C0 of $000E, then of $0012: those bits are taken), start it;
//   true if it started;
// - fade_out_on_confirm_once ($8A:8181): as fade_out_on_confirm while D+$8C is 0, which
//   then counts up.
void fade_out_wait(void);
bool fade_out_on_confirm(void);
void fade_out_on_confirm_once(void);

// $8A:9C86: the HDMA table at $0D00: $36 lines of $FF, then 21 entries of 8 lines of
// $FF, then the end (0).
void hdma_table_0d00_init(void);

// $8A:BF2A: copy the zero-ended string at $0D40 to $0D60, a space after every 4 bytes
// (counted from the start, the zero included).
void string_space_fours(void);

// $8A:8AE7: 12 words from index `x`: $062A + x = D+$3C + 16 * n, $072A + x = D+$3E.
void table_062a_fill(uint16_t x);

// $8A:9C27: every fourth call (D+$6C counts), the next word of the HDMA table at $0D01
// (D+$6E, up to $2A) steps down by one unless it is $E0.
void hdma_table_0d00_step(void);

// $85:A066 (long): the race order. The 20 cars (word indices 0-38 at $7E:4CFD) are bubble
// sorted by their 48-bit totals ($7E:6991 low, $7E:69B9, $7E:69E1 high), highest first;
// then each car without bit 2 of $7E:6A09 is listed, car number = index / 2, at $0446 +
// $7E:6A31 (which counts on). $03A8 and $03D7 become the places of cars 0 and 1 in the
// first ten of $0446 (or $FFFF). It leaves DB = $83 (its PEA $837E). Returns A = $03A8 |
// $03D7, *x the place after the last one read, *y the places left.
uint16_t race_order_sort(uint16_t* x, uint16_t* y);

// $8A:9C48: hdma_table_0d00_step, then every entry's byte of the HDMA table at $0D01
// (21 of them, a word apart) steps down by one unless it is $FF or $E0.
void hdma_table_0d00_settle(void);

// $80:D6A0: the steering state starts over: D+$28, $34, $36, $24, $1E, $2C, $38, $3A
// cleared; D+$30 = $28, D+$32 = $4B0, D+$3C = $50; D+$2A = D+$2E = $0804; D+$22 = D+$20 =
// ($0804 - $80, within 0-$100) rounded down to a multiple of 3 (the CPU divider).
void steer_state_init(void);

// $8A:8FA4 (long): a 256-line colour gradient for HDMA, from the colours D+$34 to D+$36
// (lines 0-127) and D+$36 to D+$38 (lines 128-255). Each channel (red, green, blue: its
// state at $1516, $1520, $152A) steps by 2 * (its next colour's value - its current) in
// 1/256ths a line; when a step crosses a whole value, the line pattern D+$3A (first half)
// or D+$3C (second) picks, a line at a time, the new value or the one before (a dither).
// Each line writes (0, the colour) at [D+$68] + D+$6C, which moves on by 4.
void hdma_gradient_build(void);

// $8A:9F64: bit 12 of $03DC set while D+$52 is not 0, else cleared.
void flag_03dc_from_52(void);

// Three settings by the variant in the low nibble of $03DC (a random 0-2, $8A:9F76): with
// variant 2, the second value (never run in the movies).
// $8A:9AB1: D+$56 = $14, or 4.
void variant_lines(void);
// $8A:98F0: D+$5A = $0A, or 4.
void variant_5a(void);
// $8A:99BE: D+$66 = $28 (func_8A99D0, its tail), or 8.
void variant_66(void);

// $8A:9AC9: D+$56 lines of text from $8A:9DEE (zero-ended strings, one after another)
// into the text map at $7F:7918 + D+$54, each with its zero; D+$54 moves on by 16 a line.
void text_lines_copy(void);

// $8A:9F76: $0D7E counts down; at 0 (never run): $03DC = $2000 | a random 0-2 (rng_next's
// copy at $8A:AE9B, its low two bits, 3 taken as 2) and D+$6C + 1. Returns A as the
// original leaves it (the caller's A while counting).
uint16_t timer_0d7e_tick(uint16_t a);

// $82:ECEA: $0242 and $0208 cleared.
void race_clear_0242_0208(void);

// $82:AB9D: the start countdown. While $1ECE is positive it falls by $1E9E (to 0 at
// least); as it passes $D2, $8C and $46 the light becomes 2, 3, 4 and, past 2, 5 (GO:
// $1ECE = 1, bit 3 of $0244, $02F7, $02A6 and $0359 set, and the sound's volume $22
// instead of $1E). The light goes to $0233, $02E6, $0295 and $0348, and a light not 0
// is announced through func_82AD12 ($FF00 | volume, $2000). A negative $1ECE (unless
// $047C is negative) sets $0233 = $3D and $02E6 = $3E, or $45 when $03DC's bits 4-7 are
// not 3.
void race_countdown(void);

// $82:ECF1: $1EEE and $1EF0 cleared; $0200 = $0204 and $0202 = $0206, which also add into
// the 32-bit $1EF6:$1EF4; $020A and $020C cleared. Returns the new $1EF6.
uint16_t race_scroll_commit(void);

// $83:F9D1, a car's lap (its direct page `dp`: $0200 for the player). Once the frame
// counter $019A has reached dp+$26: dp+$35 counts on (a word at an odd address); when it
// equals $1F06 the car has finished: bit 0 of dp+$7C if dp+$31 is set, bit 2 of
// $7E:FADE + $1EBA, dp+$44 bits 0 and 2 set and 3 cleared, dp+$25 = 0, dp+$26 = $019A,
// its place $1ECA written at $0446 + $1F08 (which counts on), dp+$33 = $12 + that place;
// otherwise dp+$33 = $1F06 - dp+$35 + 9.
void car_lap_check(uint16_t dp);

// $83:FEBD: a car's message request (its direct page `dp`): `message` replaces dp+$33 when
// its priority ($83:A0A4 + 2 * message) is at least the current one's.
void car_message_request(uint16_t dp, uint16_t message);
// The same request for the race's two message slots: $82:9BC9 for $0233, $82:9BE5 for $02E6.
void race_message_request_0233(uint16_t message);
void race_message_request_02e6(uint16_t message);

// $81:E7DA: the bounds of track entry D+$14: its x (the byte at $7E:8CB3 + entry, the top
// of a signed 12-bit value) in D+$28; D+$24 = D+$2C and D+$26 = D+$2E = that plus the pair
// at $83:80D0 + 4 * (the byte at $7E:8CB4 + entry).
void track_entry_bounds(void);

// $82:EADA (DB = $7E): a track event's state cleared: $1F0E, $1F10 and, at D+$1A, the
// words at $7E:3C6C, $3CAC, $3F6C, $3FAC; bit 0 of $3D6C set, and $3DAC set to the same.
// Returns that word.
uint16_t track_event_clear(void);

// A track event set (DB = $7E): its pattern goes to $1F0A; func_82EC09 for entry `y` gives
// a position, written to $1F0E and, at D+$1A, $7E:3C6C/$3CAC; that plus D+$1E to $1F10 and
// $3F6C/$3FAC; bit 0 of $3D6C set, and $3DAC set to the same. Returns that word. Five
// entries load the pattern and jump to the shared body at $82:EB1A: $82:EA81 ($2007),
// $82:EA87 ($2047), $82:EA8D ($2087), $82:EACE ($2107), $82:EAD4 ($C2E7).
uint16_t track_event_set(uint16_t pattern, uint16_t y);

// $81:E7CE: D+$24, $26, $2C and $2E = $7FFF.
void far_7fff_4(void);

// $80:F225: a lone RTS (a step that does nothing).
void empty_80F225(void);

// $82:DA46 and $83:FA32: CLC, RTS (a step that always says no).
void clc_82DA46(void);
void clc_83FA32(void);

// $80:B94F: a lone RTS (the shop's idle frame hook).
void empty_80B94F(void);

// $8A:824B: two sprites of the title placed at D+$74/$76 less D+$4A/$4C (tiles $A4 and $A6,
// x $404D + ..., y $53 + ..., the second $10 to the right); only while D+$62 is $FFFF
// (else nothing, never run).
void title_sprites_place(void);

// $8A:867C: the Mode 7 rotation for the angle D+$34 (bits 4-12): D+$14 = D+$1A = cos,
// D+$16 = sin, D+$18 = -sin, from the tables at $81:CEA9 / $81:CDA9.
void rotation_from_angle(void);

// $80:EA77: the sparks. 32 of them at $7F:1000 + (D+$3E + D+$5E) (4 bytes each: life,
// position; the index wraps within $7F, the buffer bit $8000 kept): each alive one
// (bits 1-4 of its life / 2 not 0) loses a point and draws a sprite, tile $81:D2C3 by its
// life, at its position + D+$E7. Then, while D+$86 - $64 is below $1E, new sparks are
// laid at D+$3E (life $1C): two (positions $81:D2E3 by that value, and $808 less), and
// when D+$86 is a multiple of 4 two more ($81:D323 with life $1C, $81:D367 with life $3C).
void spark_step(void);

// $83:FD3C: the byte dp+$53 = 0 (a car's direct page `dp`).
void car_clear_53(uint16_t dp);

// $83:FD26 (`value` 1) and $83:FD31 (`value` 2): the byte dp+$53 = `value`, then (their
// shared tail at $83:FD45) dp+$B1 = track object $011A's bytes at $7E:8CB3 (low) and
// $8CB5 (high).
void car_set_53(uint16_t dp, uint8_t value);

// $80:E8B5: a block from $81:D419 + `*y` (DB = $81) copied to $7F:2000 + `*x`: its length
// (the low byte of the word there; 0 = 256, kept in D+$14) then that many bytes. Both
// indices move on.
void block_copy_7f2000(uint16_t* x, uint16_t* y);

// $8A:99E2: line `x` / 2 (16 bytes a line) of the text map at $7F:7918 pushed to the byte
// buffer (byte_buffer_push), up to its zero.
void text_line_push(uint16_t x);

// $8A:99D0: D+$66 = $28.
void func_8A99D0(void);
// $8A:96C2: D+$40 + 1, returned.
uint16_t func_8A96C2(void);
// The two window HDMA tables at $0D00 and $0D20: 7 bands of 24 lines (the second with
// $E0 words), each table ending in 0.
void hdma_window_tables_init(void);

// Mark the earlier copies of entry `y` in the pair of word tables $7E:F533 / $7E:F733:
// walking from the index $00FE (in bytes, wrapping at $100) up to `y`, every entry equal
// to entry y in both tables gets bit 15 set in its $F733 word. D+$14 = y, D+$16/D+$18 =
// the entry. Returns whether the walk ran (the index $00FE was not already y).
bool func_85CDAF(uint16_t y);

// $013A = $2A - ($013C >> 4).
uint16_t func_8A869A(void);
// $014C = ($0138 >> 2) - $49 + $0164.
uint16_t func_8A823D(void);
// $0600 = ($013C >> 2) - $CA, $0700 = ($013E >> 2) + $1B. Returns the second.
uint16_t func_8A8325(void);

// Reset a block of direct-page state: $01CC = x, byte $01CE = a, $01C8 = y, $01CA = 0,
// the eight words $01CF-$01DD = $FFFF, $01C6 = $01E3 = 0, $01E5 = $0100, $01E1 = 9,
// $01F6 = $01F8 = 0, $01F9 = $19.
void func_80B556(uint8_t a, uint16_t x, uint16_t y);

// binary_to_bcd5 of the words $038E and $03BD, each result back in place as 32 bits
// ($038E:$0390, $03BD:$03BF), then func_8AC03D. Returns its carry.
bool func_8ABF76(void);

// The inverse: func_8AC024, then bcd_to_binary of $038E:$0390 into $038E and of
// $03BD:$03BF into $03BD. Returns the second result.
uint16_t func_8ABF4E(void);

// Ten entries at the word index x of the tables $06C0, $07C0, $05C0, $08C0 (two rows of
// five): $06C0 = D+$7A, +$10, ... per row (the original chains its ADCs, so a carry out of
// one row's sums adds 1 to the next sum), $07C0 = D+$7C (first row) and D+$7C + $10
// (second), $05C0 = 1, $08C0 = the tiles $04AA, $04AC, $04AE, $04C0 ... $04CC. Returns
// x + $14.
uint16_t func_8A9202(uint16_t x);

// A step toward the target (x, y) = (low byte, high byte of `target`) from the 8.8
// position D+$34 / D+$36: d = (x << 8) - D+$34 and (y << 8) - D+$36 (kept in D+$16 / D+$18),
// and the step (D+$38, D+$3A) is $0100 along the longer axis and |shorter| / (|longer| >>
// 8) along the other (the CPU divider: a zero divisor gives $FFFF), each with the sign of
// its d. The low bytes of D+$34 and D+$36 are then set to the value it leaves in A (d_y, or
// the negated D+$3A when d_y is negative). D+$14 = |d_x|, D+$26 = the divisor. The longer
// axis is chosen by the sign of |d_y| - |d_x|. Returns A.
uint16_t func_80D6F3(uint16_t target);

// Two empty long routines (a lone RTL each).
void func_8A8000(void);
void func_8A987F(void);

// Append the words (a, x, y, D+$14) to the four tables $1900, $1940, $19C0, $1980 at
// index D+$8C, then advance it: D+$8C = ((index + 2) & $3F) + D+$8A (32 entries in a
// ring, D+$8A choosing the ring). Returns the new index.
uint16_t func_80B537(uint16_t dp, uint16_t a, uint16_t x, uint16_t y);

// One step of a two-column wipe: through the long pointer at $0170 (+ $0172 bank), set
// the word at columns $0176 and $0174 of 28 rows ($40 bytes apart) to tile $3D, counting
// changed cells in $0178; then the columns move apart ($0176 += 2, $0174 -= 2).
void func_8AB942(void);

// Clear the object tables' first column ($0500-$05F7) and the slot counter $018C.
void clear_object_slots(void);

// $0D90 = $013C * 8 - $1C and $0D78 = $015A * 8 (targets for step_toward_0d68/0d70).
void func_8AA469(void);

// Skip `n` zero-terminated strings starting at bus address `list`, returning the
// address of the next one (also left in $0144). The original steps over each terminator
// with an 8-bit increment of the pointer's low byte, so a terminator at $xxFF wraps
// within the page; that quirk is kept.
uint16_t string_list_nth(uint32_t list, uint16_t n);

// Shift the 32-bit value at $0118 (high word) : $011A (low word) left by 4 and return
// the 4 bits shifted out (a nibble reader).
uint16_t take_nibble(void);

// A request ($0463 = a, $0465 = x, $0467 = 0) that is refused while the word at $047C has
// bit 15 set (returns false, nothing stored) or while a cooldown runs: with the direct
// page `dp`'s word $23 non-zero, it is reduced by $1E9E; if still >= 0 the request is
// refused (returns true), else the cooldown restarts from y and the request is stored.
// Returns true when refused by the cooldown, where the original's carry is set too; on the
// other paths the original leaves the caller's carry, so false there is not a carry.
bool func_83FE9A(uint16_t dp, uint16_t a, uint16_t x, uint16_t y);

// Draw the sprite frame of the table entry at byte offset `entry`: first its width
// ($0D70), y ($0D68) and x offset ($0D80) each step towards their targets ($0D78, $0D90,
// $0D88) by up to $017E, $017C and $017A steps. The frame is three rows $014C apart,
// centred on x ($0D60) + the offset: a left column (tiles $3000/$3001/$B000), one
// func_8AA216 column per 8 pixels of width, and a right column (the left one flipped,
// $7000/$7001/$F000). $0148 = width / 2; x, y and width are left as they were.
void sprite_frame(uint16_t entry);

// Show or hide seven objects by the mask word for $0158 (table $8A:9FF9): object i
// (offsets at $8A:9FE9) gets bit 7 of its $1516 flags cleared when mask bit i is set,
// set otherwise. $0158 is 0-5 in the game.
void func_8A9FA1(void);

// Fill the 32x64 map of words at $7F:0000 with tile $003D (a blank).
void clear_tilemap_7f(void);

// $0192 += $0DCA.
void func_8ABC32(void);

// Clear the 16 words $0DAC-$0DCB.
void func_8ABC7F(void);

// For entry `x`: the byte at $7E:8CB3 + x as the top 8 bits of a signed 12-bit value
// (sign-extended, times 16) minus the byte at $7E:8CB5 + x; that byte, doubled, is left
// in $011E. Returns the difference.
uint16_t func_82EC09(uint16_t x);

// Wipe the map at $7F:0000 with tile $3D from both edges inwards: 16 steps of
// func_8AB942, waiting a frame after each step that changed a cell. Returns the frame
// the last wait ended on, or 0 when the last step changed nothing (the original's A).
uint16_t wipe_columns(void);

// $01FB = $11 (or $13 for func_8AA49E) and $0D7E = $0A8C.
void func_8AA48D(void);
void func_8AA49E(void);

// Set the first word of each of the 128 entries of $0900 (4 bytes each) to $E000.
void clear_sprites(void);

// Load entry `i` (a byte offset) of six parallel word tables in bank $86 ($DAF5, $DB47,
// $DB99, $DBEB, $DC3D, $DC8F; 41 entries each) into $1EA0, $1EA2, $1EA4, $1EA6, $1EAA and
// $1EAC. Returns the $1EA0 value.
uint16_t func_83EA9B(uint16_t i);

// The 16 words $0DAC-$0DCB are a 256-bit number, $0DAC the most significant word.
// func_8ABB5C shifts it right `count` times (1-65536; 0 means 65536): the first bit in at
// the top is *carry, the next ones are taken from the top of $0146 (which fills with 1s);
// returns the bits shifted out at the bottom (last one in bit 0) and leaves the last
// $0146 bit taken in *carry.
uint16_t func_8ABB5C(uint16_t count, bool* carry);

// The password text at $0D40: (bits / 5) + 1 characters of 5 bits each, shifted out of
// the big number (func_8ABB5C), then two characters from $0192 (its bits 0-4 and 5-9),
// then a 0 word; characters from the 32-entry table at $8A:BC5F. Returns the last
// character word; *x the offset of the terminating word, *y the last table index.
uint16_t password_encode(uint16_t bits, uint16_t* x, uint16_t* y);

// $8A:BB99: the password's bits. The 256-bit number is cleared (func_8ABC7F), then each
// field of the table at $8A:CF61 (6 bytes: a 24-bit address, a shift, a width; a zero
// word ends it) goes in: the word at the address shifted right, XOR the width's mask
// ($8A:BC3D), pushed in with func_8ABBF4. D+$48, D+$92 and the bit count D+$84 start at 0;
// returns the bit count.
uint16_t password_pack(void);

// Shift the 256-bit number left `count` times (0 means 65536), feeding in the bits of
// `bits` from bit 0 up; after each shift $0184 += 1 and func_8ABC32 ($0192 += $0DCA).
void func_8ABBF4(uint16_t bits, uint16_t count);

// The 16-bit sibling of take_nibble: A = the top 4 bits of $011A, which is shifted left 4.
uint16_t take_nibble16(void);

// Blocks of 2 rows x 5 words in a map at $7E:230C (rows $50 bytes apart).
// func_82B78D: from the entry at $7E:4F31 + ((A >> 2) & ~3) (two bytes), $0156 = byte 0
// - 2 and $0158 = byte 1 - 2 (both 8-bit). Returns $0158.
uint16_t func_82B78D(uint16_t a);
// $82:B769: A as given, or (when the $82:DC68 entry for $047B is negative and Y is not 0)
// $019A - 1 for a positive Y, 0 for a negative one; then func_82B78D and func_82B7B0.
void func_82B769(uint16_t a, uint16_t y);
// func_82B7B0: AND the block at row $0158, column $0156 (offset 2 * $0158 + the word at
// $83:8C65 + ($0156 & ~1)) with one of 8 ten-word masks in bank $83 (pointer table at
// $83:8C55, chosen by $0156 & 7).
void func_82B7B0(void);
// func_82B838: copy the block at byte offset `at` from $7E:217C to $7E:230C. Returns the
// last word copied.
uint16_t func_82B838(uint16_t at);

// Write a small HDMA-style table at bus address `table` (DB:X in the original; bank $7F in
// the game): 3-byte entries (line count, word) — $31 lines of $00FF; if A's high byte H
// is not 0, H lines of (L | L << 8) with L = A's low byte + $69; 1 line of (L << 8 | $69);
// $4D - H lines of $00FF; then the byte $B4. Also leaves A in $0114, L in $0116 and H in
// $0118. Returns the address of the final byte.
uint32_t func_80DC5A(uint32_t table, uint16_t a);

// $80:DB7D / $80:DBCB (DB = $7F within): a window table for HDMA at $7F:x. func_80DC5A
// writes its head (from `a`); then, from $33 bytes past its end, a line pair per entry:
// right edge = width / 2 + D+$2E - D+$20 and left edge = right - width + 1 (bytes), the
// widths from $81:C872 (first) or $81:C9AA (second) at the row $81:C852 + (D+$1E & ~1)
// names, for D+$1E / 2 + $0C lines; the rest of $1A lines are empty ($00FF), then the end
// ($00FF, 0). The $19 lines above the start mirror those below it (one more when D+$1E is
// odd). D+$14 and D+$16 count the lines.
void window_shape_a(uint16_t a, uint16_t x);
void window_shape_b(uint16_t a, uint16_t x);

// Take a free slot (word 0 in $7F:7FBC + slot, slots 0..$0166 step 2) whose kind
// ($041C + slot) is $0168; if there is none, lower $0168 and look again, down to kind 0.
// The slot taken is marked $FFFF and stored in *slot. Returns false when even kind 0 has
// no free slot (*slot = $0166 then).
bool func_8A9BB5(uint16_t* slot);

// $0130 = 0, the 128 words at $0500 cleared, then clear_sprites.
void func_8A8EEF(void);

// $0154 += $10, $0156 -= 1.
void func_8A9BAA(void);

// $8A:9B85: the zero-ended string at bank `bank` : `at` (kept in D+$62/$64) copied, its zero
// included, into the map at $7F:7918 + D+$54; then func_8A9BAA (next line). Returns the
// string's length.
uint16_t text_line_put(uint16_t at, uint16_t bank);

// $0168 = the largest of the 20 slot kinds at $041C (largest by the sign of the
// difference, as the original's BMI does), starting from 0. Returns it.
uint16_t func_8A9BE2(void);

// For each 8-byte record of the list at bus address `list` (the original's A + 4, bank DB)
// until a zero first word: the word at $1516 + (word 4) = word 6. $0160 follows the
// record pointer.
void func_8AB680(uint32_t list);

// Overlap test for the entry at $011A of the $7E:8CB3 table (D = `dp`, $0200 in the
// game): its 12-bit signed value (the byte at $8CB3, times 16) goes to $0128 and, minus
// D+$06, must lie within the kind's limits ($83:80CC below, $83:80CE above; kind offset
// $011C); then -(high nibble of the byte at $8CB5) * 8 minus $012C must lie within
// $83:826C/$83:826E. "Within" is judged by the sign of each difference, as the original's
// BPL/BMI do. Returns true when both hold (the original's carry).
bool func_83FC05(uint16_t dp);

// The object hit test with the position in `a` (D = `dp`, $0200 in the game): unless $0120
// has bit 15 set, a - D+$06 and $012A - $012C must lie within the kind's limits (as in
// func_83FC05). On a hit: clears bit 3 of D+$7C, requests effect $FF23 for $1EBE through
// func_83FE9A, adds (D+$0F / 2) * $1F30 / 32768 to D+$5A (DSP-4 multiply; saturating;
// skipped when $1F31 has bit 15 set), sets D+$1A to 2 (behind, $0128 < 0) or 3 if it was 0
// and D+$10 >= $60, D+$1C to $1000 or $F000, and halves the signed 32-bit D+$18:D+$16.
// Returns whether it hit.
bool func_83FD8A(uint16_t dp, uint16_t a);

// $83:FDCD: always "no hit" (its test was disabled: a BRA skips it).
bool func_83FDCD(void);

// Three of the track objects' handlers (D = `dp`, $0200 in the game; the object is entry
// $011A of $7E:8CB3, kind $011C). Each first sets $012A = 0 and, but the money one,
// $0128 = the entry's x (the byte at $8CB3, the top of a signed 12-bit value).
// $83:FA34: the hit test func_83FD8A at that x, or func_83FDCD when bits 4-5 of the byte
// at $8CB5 are set. Returns its carry (a hit).
bool track_object_test(uint16_t dp);
// $83:FA6A: a side bump. When x - D+$06 lies within the kind's limits ($83:80CC below,
// $83:80CE above, judged by sign) and D+$1A is 0: D+$7E = 8, D+$1A = 6 and effect $FF1D
// requested for $1EBE (func_83FE9A).
void track_object_bump(uint16_t dp);
// $83:FAD7: money. On a hit (func_83FC05): the entry's word is cleared, 4000 (BCD) added
// to the money of car $1ECC ($038E low four digits, $0390 high), effect $FF19 requested
// for $1EBE, D+$4A = $801E and D+$33 = message $47. Returns the carry it leaves: false on
// a miss, func_83FE9A's on a hit.
bool track_object_money(uint16_t dp);
// $83:FB15: a counted pickup. On a hit (func_83FC05), unless the low byte of D+$4E is
// already 10: D+$4E counts up, the entry's word is cleared, effect $FF19 requested for
// $1EBE, D+$4A = $801E and D+$33 = message $4C. Returns the carry it leaves: false on a
// miss, true when the count is full, func_83FE9A's otherwise.
bool track_object_count(uint16_t dp);

// $85:9D39 (player 1: $023E, D+$A2) and $85:9D5A (player 2: $02F1, D+$A6): while the
// position is at least $40 past the word at $84:(D+$9C + the offset), the offset moves
// on by $14 (one entry).
void race_section_seek_1(void);
void race_section_seek_2(void);

// $85:82D8 and $85:82FA: 16 words from $7F:(`a` + the word at $83:84B1 + `x`), one after
// another, to $7E:6A33 ($85:82D8) or $7E:6B33 ($85:82FA).
void row_copy_6a33(uint16_t a, uint16_t x);
void row_copy_6b33(uint16_t a, uint16_t x);

// The word in bank $85 for A: row (A >> 6) & $FF times 10 (DSP-4 multiply) plus $0198 (also
// left in $0118) picks an entry of a table at $84:0002; that entry plus (A & $3E) (and the
// first add's carry) is the offset in bank $85.
uint16_t func_82EA49(uint16_t a);

// Two linear interpolations at the fraction $FAB5,x / 256 ($0114): between the words at
// $34EC,y and $34EE,y (minus $FABB,x, into $0116) and between $36EC,y and $36EE,y (minus
// $0C, into $0118; returned). Bank $7E; each uses a DSP-4 multiply's high word.
uint16_t func_82AB3F(uint16_t x, uint16_t y);

// Targets for $028B/$028D: unless $0281 is negative (then $0118 = -$30, $0116 = 0),
// func_82AB3F at ($0281, $0283), then $0116 = -(2 * $0116 * $0293 >> 16) and $0118 =
// (2 * $0118 * $0293 >> 16) - $33 (DSP-4 multiplies). $028B and $028D then move towards
// $0116 and $0118 by at most $1E9E * 4 (judged by sign, as the original). Returns $028D.
uint16_t func_82A9B7(void);

// $7E:4E2D = (bits 8-23 of $0213 * $300, a DSP-4 multiply) - $B00, at most $3000 (by
// sign). If $0244 bit 0: $7E:4E35 = ($B0 - 2 * the high word of the 32-bit distance
// $7E:FAB4 - $0224, both at least 0) & $F8. If also bit 2: $7E:4E2D = ((its value + $B00,
// at least 0) / (high word of $FAB6 - $0226, at least 0, + $10, at most $D0)) * 16 - $B00,
// at most $3000 (the division on the CPU's divider). $0114 is left as scratch.
void func_82AC52(void);

// $82:AD99: a 5x5 block of tile numbers, `first` up (kept in D+$16), column by column
// into the 32-wide map at $7F:0130 (rows $40 bytes apart).
void tile_block_5x5(uint16_t first);

// $85:831C: the track's settings by $047B: D+$98/$9A (the pair at $84:A590 + 4 * track),
// D+$9C ($84:FF59 + 2 * track), $1F06 ($83:9515), $01E4 and $01E6 ($83:9583, or $83:95F1
// when the byte $039C, resp. $03CB, is set; the second never runs), $1F34 ($83:965F).
void track_settings_load(void);

// $86:CE4A: eight rows of $38 bytes copied within bank $7F (MVN) from `*x` to `*y`, the
// destination rows 8 bytes further apart ($40). Both indices move on.
void rows_7f_copy8(uint16_t* x, uint16_t* y);

// $80:C31A: the block at [D+$C4] (low WRAM): $60 bytes from $9B:8000, then $20 from
// $80:B2D2, then the words $0E03, $40, $109F, 0 at +$102 and $0E03, $40 at +$122.
void block_c4_build(uint16_t dp);

// $80:EBCE: a queue reset (D = `dp`): D+$CC, D+$C6, D+$D1 = 0, D+$E5 = $80, D+$E1 = $40, and
// the list at $7F:(D+$5E) = 28 words $FFFF then $FFFE.
void queue_7f_init(uint16_t dp);

// $8A:9BF6: the text map cleared: D+$60, D+$14, D+$58 = 0, the words $7F:7918-$7A59 and
// $7F:7FBC-$7FE3 = 0.
void text_map_clear(void);

// $8A:98B4: D+$5A (variant_5a: $0A, or 4 with variant 2) entries: the word at $041C +
// 2 * (the byte at $0446 + entry) gains the entry's word of $8A:9D8D (variant 2:
// $8A:9DB5, never run).
void variant_offsets_add(void);

// $8A:81BA: five palettes into CGRAM (upload_cgram_now) from bank $7E: $81E2 to $80,
// $8202 to $90, $8222 to $A0, $8114 ($1E bytes) to $01, and the one $8A:934F + 8 * $1536
// points at to $A0.
void palettes_load_8a(void);

// $85:9D7B: the map at $7E:6C33. On a track whose word at $85:A439 + $047B has bit 15
// set, with the byte $039C set: $820 words of $3FFF. Otherwise unpacked (decompress, D =
// `dp`) from $8C:95E3; returns its length and leaves the source's end in `*end` (0 and
// `*end` untouched when filled).
uint16_t track_map_load(uint16_t dp, uint16_t* end);

// $82:A954: the track's segments. Unpacks $9A:8000 to $7F:0000 (decompress, D = `dp`), copies
// the track's part of it (start $85:9FAA, length $85:A008, by 2 * $047B) to $7E:4F31 (MVN),
// then turns both words of each 4-byte entry into running sums.
void track_segments_load(uint16_t dp);

// $8A:B0E1: bits 2-5 of $1528 = bit 10 of $03DC moved to bit 2; bits 2-5 of $1526 =
// bits 8-9 of $03DC moved to bits 2-3 (lsr_n). Returns the carry of the last shift.
bool variant_flags_apply(void);

// $8A:C19C: six sprite slots (X = 0, 2, ... $0A): $0800 = the word at $8A:C443 | $2000,
// $0600 = D+$42 (which moves on by $10), $0700 = D+$44, $0500 = 1.
void sprite_slots_six(void);

// $8A:8CB7: graphic `x` (an even index) at $7F:(the word at $83:84B1 + x + `offset`, kept
// in D+$38), (the word at $83:851B + x) bytes of packed pixels (two 4-bit pixels a byte,
// eight a row, eight rows a tile), turned into the SNES's 4-bit planar tiles in place: per
// row, bit 0 of each pixel to plane 0, bit 1 to plane 1 (bytes 0-15, the planes of a row
// together), bits 2 and 3 to planes 2 and 3 (bytes 16-31). D+$14..$33 hold the tile
// being built. Then the whole graphic goes to VRAM word $6FE0 (upload_vram_now).
void tiles_planar_upload(uint16_t offset, uint16_t x);
// $85:817B: the same, byte for byte but its own loop's address, to VRAM word $5FE0.
void tiles_planar_upload_5fe0(uint16_t offset, uint16_t x);

// $8A:8F18: the HDMA gradient tables. Their headers at $7E:2C00 ($FF and $7E:2800, $FF and
// $7E:2A00, then 0); then three hdma_gradient_build runs into $7E:2000 + D+$6C (from 0):
// all zero colours with patterns 0; colours 0, 0 and the word at $8A:934B + 8 * $1536; and
// with patterns 1, the words at $8A:934B and $8A:934D + 8 * $1536 and D+$72.
void hdma_gradients_init(void);

// $82:A1D3: the track's backdrop (D = `dp`). Unpacks $98:9FEB to $7F:0000 (decompress);
// with k = 4 * (the byte at $84:A66C + $047B): $1F28/$1F2A = the pair at $85:A33A + k;
// with x = 2 * k, uploads from bank $7F (upload_vram_now, D+$14 = $7F each time): the
// graphics ($85:A346 + x, size $85:A348 + x) to VRAM $3800, the maps ($85:A34A + x and
// $85:A34C + x, $600 bytes each) to $3000 and $3400; then the first map's $600 words
// get bit 13 cleared and $400 added (palette and tile base), and both maps go to
// $7400 and $7800.
void track_backdrop_load(uint16_t dp);

// $82:AD2C: the two players' race positions. Each of the 20 cars ($35 bytes apart from
// $7E:FAB5; player 1 is car 0, player 2 car 1) has a 48-bit progress (words at +0, +2,
// +$1E, low to high). $0142 (player 1) and $0144 (player 2) count the cars whose progress
// is at least the player's (a subtraction whose sign decides, as the original's CMP, SBC,
// SBC and BMI), the player included. A position that differs from the last one shown
// ($1ED2, $1ED4) is kept there and its message ($25 + position) requested.
void race_positions_update(void);

// $85:B1FF: the race's band table from the two players' line lists (zero-ended, ascending;
// player 1's at $7E:4CFE + $0164, player 2's at $7E:4D4A + $0168, the indices kept in D+$14
// and D+$16 and moved on by an 8-bit INC). Merged into $0486 + Y (Y from 2, or $34 when
// $00B4 is 0; kept in $00C0), each line's flags in $0596 + Y: $02 player 1's, $0C player
// 2's, $0E both, | $80 on each list's first (D+$18, D+$19). The first marks go to $047E +
// X (X = 2, or 0): $39 when the lists start on the same line, else $19 or $29 then $39.
// Ends at the zero both lists end with (stored). Returns the final Y.
uint16_t race_bands_merge(void);

// $83:E2D7: player 1's car, each frame. $1EBE = $8000; $1EB2 = the byte $0395; $022B =
// func_83EA9B(2 * $021F + the word at $86:DAD9 + 2 * ($0395 & $7F)); the car's settings
// from bank $86 by the bytes $0393 ($1EA8, $1EC0), $0397 ($1EB4, $1EB6), $0394 ($1EB8),
// $0396 ($1F30); $1ECA, $1EBA, $1ECC, $1E9C = 0, $1F2C = $1E66, $1EBC = 2 * $0226 & $3E.
// Then, unless the controls are read ($0244 bit 2 clear and $03DC bit 4 set),
// player_road_limits (D = $0200). With the controls ($000C held, $000E pressed, the
// masks at $039D-$03A5): flags in $1E9C (1, 2, 4, 8 and the item's two table bits),
// $7E:2142 = $58 or $109F, and the item in $014C stepped down ($03A5) or up ($03A3) to
// the next one carried (bytes $0397 + item, up to 4); $025C = the byte at $024E + item,
// $1F2E = the item.
void player_frame_setup(void);

// $8A:86AB: the scene's objects, when bit 0 of the word at $8A:9351 + 8 * $1536 is set
// (else it returns X = 8 * $1536). D+$7E = D+$3C >> 6, D+$80 = D+$82 = D+$7E >> 1,
// D+$84/$86/$88 = -D+$62; then eight objects, each at (D+$7A, D+$7C), through
// func_8A9202 (two) and func_8A92CF (six), X their running slot from 0. Each D+$7A takes
// the carry of its D+$7C sum (no CLC between); the last D+$7C is D+$84 - $0B (a CLC then
// SBC #$0A). Returns the final X.
uint16_t scene_objects_place(void);

// The results by variant (bits 8-9 of $03DC: 0, $100, else): the byte at $85:D501,
// $85:D530 or $85:D55F + $0444 (variant 0 never runs in the movies).
// $84:A31B: when that byte is set, $036C moves on by 4.
void result_slot_step(void);
// $84:A34B: when it is set, the word $036E + (the word at $85:D590 + (0, $1C or $44 by
// variant) + $036C) counts up.
void result_tally(void);

// $8A:9D1E: the 20 words at $041C = 0.
void clear_041c(void);

// $80:E8CE: the player's name (bytes at $037E + D+$94, up to its zero) to $7F:2000 + `*x`,
// then the words $0100 and $1400. `*x` moves past them, `*y` ends at the zero.
void name_copy_7f2000(uint16_t* x, uint16_t* y);

// $80:E8F4: the player's car's blocks (the car is the word $03A8 + D+$94) to $7F:2000 +
// `*x` (block_copy_7f2000): the one $81:D3C9 + 2 * car points at, then block $BE; with no
// car (negative), block $A8 alone (never run).
void car_blocks_7f2000(uint16_t* x, uint16_t* y);

// $80:E911: the one block $81:D3F1 + 2 * car points at (car $13 when negative, never run).
void car_block_7f2000(uint16_t* x, uint16_t* y);

// $84:A41B: the next track ($0444) in the variant's order (bits 8-9 of $03DC: $100 the
// list at $8A:CADE, $200 at $8A:CB01, else at $8A:CAC4, never run): the byte after the
// first one equal to $0444. Returns the index of that byte.
uint16_t track_next(void);

// $84:A4ED: the results reset. $0444 = 0; $16 bytes from $83:85AB to $0368, $0F from
// $83:85D1 to $038E and from $83:8600 to $03BD (MVN); the ten words at $0446 = $0100,
// $0302, ... (+$202 each); $036C = 0. With bit 1 of $03E2 (never run): the bytes $0398,
// $0399, $039A, $039C, $03C7, $03C8, $03C9, $03CB = 1.
void results_init(void);

// $84:A388: the prize money, in decimal (BCD, eight digits: low word, high word), for both
// players (place $03A8, money $038E/$0390, bonus bits $03AB; player 2: $03D7, $03BD/$03BF,
// $03DA). A player with a place (not negative) gets the pair at $84:A2B3 + 4 * place,
// then, for each bit of the bonus word shifted out (LSR until it is 0, at least once),
// the pair at $84:A2DB + 4 * bit when the bit is set. Returns the carry the original
// leaves: that of the last bonus step, or `carry` when neither player has a place;
// *overflow likewise becomes the V of the last decimal add (kept when there is none).
bool prize_money_award(bool carry, bool* overflow);

// $8A:9CF8: the slots' order. The 20 slot words at $7F:7FBC = 0; func_8A9BE2 (the largest
// kind) and func_8A99D0 (20 slots); then each slot func_8A9BB5 takes (largest kind first)
// goes to $0D00 + Y as its index (slot / 2), Y moving on by 2, until none is left.
// Returns the final Y.
uint16_t slot_order_fill(void);

// $8A:9AF1: the players' names as text lines (text_line_put, bank $81), by the variant (the
// low nibble of $03DC): 0, player 1's ($037E); 2, four names ($03E4, $03F1, $03FE, $040B);
// 1, by bits 4-5: $10 alone, player 1's and the string "PLAYER2" ($8A:9B7D), else player
// 1's when bit 4 is set and player 2's ($03AD) when bit 5 is. Only variant 0 runs in the
// movies.
void player_names_put(void);

// $82:8F8F: the shake ($1F18, with its offsets $1F1C/$1F20), each race frame. With $0231
// clear: $1F18 = $8000. A negative $1F18 starts it: 0, or $30 when the byte $0397 is at
// least 4. Below $98 it runs: offsets -3, 2; $1F18 += 2 * $1E9E; at $98 and up (by its
// multiple of 8) it becomes $B0, and from below $30 to $30 and up it jumps to $98. At $98
// and up it ends (never run in the movies): offsets (rng / 2) * -10 and * 10 (the DSP-4's
// high words), then $1F18 from $82:9139 (below $B0) or $82:913F by a random 0, 2 or 4.
void shake_step(void);

// $82:AF33: the track objects' graphics, streamed as the car ($0226) passes their entries:
// 5-byte records at $7E:5B91 + D+$A0 (a word, where it starts; a byte, its tile slot; a
// word, the object kind). $7E:45EC = $140; then for each record reached (its word at most
// $0226; the first one not reached ends it, a JMP to the RTS that never runs for the
// first): kind 0 sets $7E:45EC = 2 * slot; else the kind's entry ($86:AE36 + 2 * kind, or
// $86:AC36 when its high byte is set) gives the source (its offset at $86:001E/$86:0022 +
// the word at $7E:6591 + its first word's) and size ($86:0020/$86:0024), the VRAM word
// (slot * 32) and $7E:45EC (high kinds: $7E:495C) + 2 * (the kind's low byte) = 2 * slot,
// and the graphics go up. An empty entry leaves X = 2 * kind, from which the next record
// is counted (the original's quirk; the game's kinds have entries)
// (upload_vram_now from bank $7F). D+$A0 moves on by 5 a record.
void track_objects_stream(void);

// $8A:9AA5: the results text: D+$54 = 0, then variant_lines, player_names_put and
// text_lines_copy.
void results_text_build(void);

// $8A:9CB0: slot_order_fill, then whether the player's slot (0; with two players shown,
// 0 or 1: a variant other than 0 whose bits 4-5 are not $10) is among the first five of
// $0D00; clear_041c. Returns the carry it leaves: set when it is not (never run).
bool player_in_top5(void);

// $80:F6FD: $0198 = 0, $0196 = $FF80, D+$3E counts up.
void title_scroll_start(void);

// $80:8199: the saved data sealed: $0366 = the sum of the $78 bytes at $0368 ^ $7645,
// then the $7A bytes from $0366 copied to $7E:FA3B (MVN).
void save_data_seal(void);

// $8A:C355: with bit 12 of $03DC, both players' money = player 1's + 75,000 (decimal: the
// low word + $5000, the high word + 7 and the carry). Never run in the movies. *overflow
// becomes the V of its last decimal add (kept without bit 12).
void money_skip_bonus(bool* overflow);

// $8A:C2DD: $0444 moved to the first track of the variant's list (as track_next's) at
// or above it (by the sign of the difference, BEQ/BPL); each track skipped calls
// money_skip_bonus (never run in the movies). Returns the index of that track.
uint16_t track_snap(bool* overflow);

// $81:F48B: func_83FE9A for player 1's car (its cooldown at $0223: D = $0200), with A, X, Y
// as given. The cooldown path never runs in the movies.
bool effect_request_p1(uint16_t a, uint16_t x, uint16_t y);

// $8A:8600: a 5x3 block of tiles from $7E:8000 + `x` (words, row by row) written straight
// to VRAM at $0496, $0516 and $0596 (video_write_vram_word). Never run in the movies.
void anim_tiles_put(uint16_t x);

// $8A:85AF: a tile animation by D+$42 (0, the movies' only case: nothing). 1: D+$40 counts
// 0-7 and frames $5A, $78, $3C show at 1, 3, 5; else: D+$40 counts 0-15 and the frame is 0
// or $1E by its bit 1 (anim_tiles_put).
void tile_anim_step(void);

// $81:E631: the track's row words at $7E:D1B1 + 2 * row, rows $019A down to 1. The track
// table index starts at D+$98 + (high byte of 4 * $019A, less 1) * 10 (a DSP-4 multiply)
// and steps back 10 every $40 rows. Per row: the limits D+$20/$22 ($1F28, or $1F2A where the
// table's word at +4 is $F8F3), the four edges D+$16, $18, $1A, $1C moved toward them,
// clipped by the row's track object (its kind, the byte at $7E:8CB4 + D+$14, picks
// far_7fff_4 or track_entry_bounds), then the DSP-4's command $11 makes the row word, each
// negative nibble moved up by one (a negative word first gains $1000). At the end the
// first 16 bytes are copied past the last row (MVN).
void track_rows_build(void);

// $82:ADC2: the track's $190-byte block (D = `dp` for decompress). Unpacks $99:97E4 to
// $7F:0000, then copies the block at $7F:($047B * $190, the DSP-4 product's low word; the
// original writes the chip instead of reading the high word) to $7E:217C, and that to
// $7E:230C and $7E:249C (MVN).
void track_block_triple(uint16_t dp);

// $82:A672: both players' palette rows (D = `dp` for decompress). Unpacks $88:929A to
// $7F:0000. The rows start at $7F:($047B * $E0, the DSP-4 product's low word) for both
// players (D+$16, D+$1A) and at $85:A1EC + (0, or $70 when $01E4 is above 0) (D+$18,
// D+$1C). On a track whose word at $83:9583 is 0, a player whose byte ($039C, $03CB: never
// run) is set takes the next rows ($70 on, and $E0). Then 7 rows ($20 apart): 8 words from
// $7F to $7E:6A59 / $6B59, and 7 from bank $85 to $6A55, $6A57, $6A69-$6A71 / $6B55... .
void track_palettes_load(uint16_t dp);

// $80:FAEE: 16 colours faded by step `level`: s = the low 5 bits of $81:D9AC + 2 * level.
// Each colour's red, green and blue words ($81:DB4C, $DB6C, $DB8C) less s (0 below)
// make the BGR555 word at $0500 + 2 * i (green and blue placed by byte swaps and shifts),
// sent to CGRAM colours $60-$6F by DMA.
void palette_fade_60(uint16_t level);

// $8A:9C69: hdma_table_0d00_init, then HDMA channel 2 set up on the fixed colour ($2132,
// one byte a line) from the table at $81:0D00 (not enabled here).
void hdma_coldata_setup(void);

// $8A:8787: six HDMA channels set up (not enabled): 1 on the BG mode ($2105), 2 on the main
// screen ($212C), 4 on the BG1/BG2 tile addresses ($210B), 3 on BG1's tilemap ($2107), from
// tables at $8A:88DE, $88E7, $88F0, $88F9 (one byte a line); 5 and 7 on BG1's vertical
// and horizontal scroll (two bytes a line) from $0D00 and $0D20, whose 13 bytes are copied
// from $8A:8902 and $890F (MVN). Channels 5 and 7 also get an indirect bank ($81, $A1).
void hdma_title_setup(void);

// $8A:8DBF: palette `pick` (D = $0100): p = the word at $83:84B1 + pick; colour 0 and D+$72
// take the word at $7F:p, and 16 colours from $7F:p go to CGRAM $70 (upload_cgram_now).
void palette_load_70(uint16_t pick);

// $8A:C1C6: a text screen (D = $0100). $0D7C counts up; the text box is columns 0-$1F
// (D+$3E, D+$40), D+$68 = 0, D+$86 = 1, D+$3C = 3; 97 words from $8A:C8E4 go to VRAM $1000
// on; 28 tiles $0502 to $7F:0184 on; the string at $8A:`str` is printed centred
// (text_print_centered); then the 128 tiles at $7F:0080 lose bits 10-12 (their palette).
void text_screen_init(uint16_t str);

// $8A:A06D: hdma_window_tables_init, then HDMA channel 7 on BG2's horizontal scroll ($210F,
// two bytes a line) from $81:0D00 and channel 1 on the fixed colour ($2132) from $81:0D20
// (not enabled here).
void hdma_window_setup(void);

// $8A:BACB: 97 words from $8A:C8E4 to VRAM 0 on (word writes).
void text_tiles_put_0(void);

// $8A:C24B: CGRAM colours $20-$4F cleared (direct writes), then 16 colours from $8A:C964
// to CGRAM $80 and 32 from $8A:C984 to $60 (upload_cgram_now).
void text_palettes_load(void);

// $81:F252: contact between two race objects `x` and `y` (offsets into the object tables
// at $7E:FAB5 on; D = $0100). No contact (returns false) when either has bit 3 or 7 of its
// +$29 flags set, their +$0A words are $18 or more apart, or their +$06 words $40 or more
// apart (each as the sign of the difference, not a true signed compare). Else (after the
// original's PHX/PHY; returns true, the original's carry, only for the swap below):
// - each object's car slot (D+$18 for x, D+$1A for y): object 0 is slot 0, any other none
//   ($FFFF); in modes where $03DC & 3 is set, the table at $81:FE16 by object number (x /
//   $35, the CPU divider; never run);
// - when y's position (+$0C, 32 bits) is less than $60 ahead of x's, x is pushed beside
//   y: x's +$06 = y's +$06 plus $40 (y's +$20 below x's) or minus $40, and x's car (if
//   any, not flagged 4 at $0244) gets that plus $029C in $0206;
// - else the two swap speeds (+$14, 32 bits) and x drops to $60 behind y (+$00, 32 bits;
//   not below its own +$0C: never run). Each car slot recomputes its $0216 (32 bits) as
//   the other's speed * 2 over its own $022B (a 40-step shift-and-subtract division, as
//   written); x's car loses bit 2 of $027C (not flagged 4) and, not flagged 1, its $0225
//   (32 bits) by the drop. When neither car is flagged 4, effect_request_p1($FF24, $8000).
bool car_pair_contact(uint16_t x, uint16_t y);

// $83:FC65: a track object tested on both sides (D = `dp`; a handler of $83:F7F9's table,
// like track_object_test). $012A = 0 and $0128 = the entry's x; then, by bits 4-5 of the
// byte at $8CB5 (a jump table at $83:FC90), up to four tests at $0128 = x, -x, -x - $80
// and x + $80, stopping at the first hit:
// - 0 and 3: func_83FD8A at each;
// - 1: func_83FD8A then func_83FDCD at x, the same at -x;
// - 2: func_83FDCD at each (never a hit).
// Returns whether one hit.
bool track_object_test_sides(uint16_t dp);

// $80:EBF7: a line of scrolling text drawn as sprites (D = $0100), from `text` in `bank`
// (its DB), starting at screen x `x` (D+$C8, D+$CA).
// - D+$C6 gains D+$E5; the 28-word queue at $7F:D+$5E moves up a word and its new last
//   word is $FFFF, or D+$CC (counting up) for each $100 D+$C6 loses. D+$CF = the first
//   queued word not $FFFF (0 if none).
// - Then, until D+$DF (from 0) reaches D+$CC, it reads the text: a character draws a
//   2-tile sprite at D+$CA (OAM at $0900 + D+$74; digits and capitals from the tile pairs
//   at $81:CCD9, a few marks from the table at $80:EDE7 (never run), a space draws
//   nothing), in palette D+$16 ($3200, or a band once D+$DF reaches the queued word);
//   D+$CA moves on by 8. While D+$D3 is set and D+$DF + 1 is below the queued word, the
//   text's next two bytes are replaced by a 1-tile sprite from $81:D2E3 + D+$D3.
// - A 0 byte starts a control code: 0 ends the text (D+$C6 = 0; the rest is padding), 1
//   sets the x (D+$C8 + a word), 2 repeats (a count in D+$D1; never run), 3 sets bit 15
//   of D+$E1 (never run), 4 sets D+$D3, 5 clears it.
// - Last, unless D+$E1 is negative: D+$E1 = $3B while the queue's 14th word is set; from
//   $1E on it draws an end marker ($81:CDA1) in a fading palette; it counts down, $3B
//   after 0.
void ticker_draw(uint8_t bank, uint16_t x, uint16_t text);

// $8A:C15D: the text buffer at $7F:7D00 printed as tiles (D = $0100), up to its 0 byte:
// $80 starts a new row (D+$3C, from column 2 in D+$3A; from row $0C on, D+$90 = 0, and
// row $0C itself is skipped), any other byte is text_put_tile_char's. Then
// text_screen_init with the string $8A:C224 ($C238 when $1B37 bit 15 is set: never run).
void text_buffer_screen(void);

// $80:DCBB: the track's scenery built (D = $0100), from its list of 14-byte records at
// $81:C3E8 (start: the word at $81:C3D0 by the byte at $84:A240 + $0444). Per record (+0
// its kind word, +2 its column; 0 ends the list): an entry (+0 & $7FFF, +2 * 8 plus the
// kind's x at $81:CAE2) appended to the list at $0882 (D+$86); a negative kind also
// appends one to $0802 (D+$82: with +$0C and the bit D+$88) unless that bit is set in
// $036A, and D+$88 moves to the next bit. Then by the kind ($8000 also ends the list):
// - 2: a 6x6 block of map words $11DC on at $7F:CED0 + 2 * column (never run: $B250 +),
//   the record's +8 at $0700, 32 + 16 bytes of palette from bank $86 to $0580/$0680;
// - 4, 6, 8, 10: a 4x4, 4x4, 4x3 or 2x2 block copied from $7F (by the kind and the parity
//   of D+$7A) to the map at func_80E0C8(column), each word plus D+$84 (the band of D+$7A)
//   with the carry of the add before; two 16-byte palette rows from bank $86 to $0500 +
//   D+$7A * 16 and $100 on; tiles unpacked from $91:(+6) to $7E:D+$7C (moving on); a 4x4
//   block of map words D+$7E on at $7F:AE40 + $1310 / $19D0 + 2 * column; an entry at
//   $0700 + D+$80 (10 bytes); D+$7A counts down.
// At the end $0800 = D+$82 and $0880 = D+$86.
void scenery_build(void);

// $81:F21E: object `x`'s contacts down a chain (D = $0100). Unless object `y` is $60 or
// more ahead of x (+$00, unsigned), D+$3C counts up and, from y along the links at +$18
// ($7E:FACD), car_pair_contact(x, y) runs for each object less than $60 ahead, until one
// hits, the chain leaves that range, or D+$3C (counting down) reaches 0. D+$3C is put back
// to its counted-up value. Returns the original's carry: set when y is $60 or more ahead
// (nothing done) or on a hit (a swap), clear otherwise.
bool car_contacts_chain(uint16_t x, uint16_t y);

// $80:E925: one frame of the scrolling-text scene (D = $0100).
// - D+$82 decays toward 0 by $12 (from below: never run); the 24-bit D+$7B gains the
//   sign-extended D+$80, D+$7E gains D+$82; $0026 gets bit 1.
// - The view: D+$2E = D+$7C / 8 (signed) + $88 + the signed high byte of D+$22; D+$30 = the
//   9-bit row from D+$7E's high byte (rounded by bit 7; sign-extended: never run) - $2E -
//   D+$E8 + the signed high byte of D+$24; D+$32 = D+$38 and D+$34 = -D+$36 from $81:CEA9 /
//   $81:CDA9 by D+$84 & $3FE; D+$3A = D+$2C, D+$3C = $1C.
// - The text (ticker_draw at x $0A10 + D+$E7): with D+$46 not negative, the line at
//   $7F:2000 + D+$5E. Else the list $81:D5CF by D+$4A: on an even D+$46, D+$48 = 0 starts a
//   fade ($98 = 0, $96 = $FF80), or else D+$4A moves on past D+$48's lowest set bit, D+$46
//   counts up, the queue restarts and D+$4C = $A0; D+$4C counts the line's frames, D+$46
//   counting up at 0.
// - spark_step unless the car's $03A8 is set, func_80EB66, oam_finish_near; D+$86 counts.
// - D+$46 not negative counts down; at 0, unless D+$48 is set and the car's $03A8 is not
//   negative (then D+$46 = $8000), it counts back up and a fade starts. At $FFFF a fade
//   starts.
void ticker_scene_step(void);

// $80:F5DE: a title step by its frame D+$40 (D = $0100). At frames $13A-$140 (even), $614
// bytes from $7E:9180 / $9768 / $9D50 / $A338 go to VRAM 0 / $2F4 / $5E8 / $8DC. Then
// D+$7E = $FFFF; window 1 at 255-0 and window 2 likewise, BG1/BG2 masked by window 1
// ($2123 = $0A, then $02), the HDMA line tables at $0D00/$0D40/$0E00/$0E40 cleared. From
// $80:F670 on (also the tail of $80:F5CD): from frame $15C on D+$42 counts, D+$30 = D+$42 /
// 2 + $40, D+$36 = $800 and window_curve_build; the frame's picture unpacks from $7E:(the
// word at $7E:A920 + the frame) to $7F:0000; D+$40 moves on by 2 (D+$3E counting at
// $17C). Returns the unpacked length (the original also returns with C set).
uint16_t title_frame_step(void);

// $80:F5CD: the title frames restarted: D+$40 = 0, D+$3E counts, D+$42 = 1, D+$1E = $3C;
// then $80:F670 on, as title_frame_step (frame 0's picture unpacked, D+$40 = 2). Returns
// the unpacked length.
uint16_t title_frame_begin(void);

// $80:F6AD: a title fade step (D = $0100): D+$42 counts (D+$3E counting at $7F); the
// level D+$30 = D+$42 / 2 + $40 below $20, else (D+$42 - $3F, 0 below) / 2 + $20, at most
// $3F; palette_fade_60 of it, D+$36 = $800, window_curve_build; at D+$42 = $3F the picture
// $7E:C730 unpacks to $7F:0000. Returns that length, or 0 (the original returns with C
// set).
uint16_t title_fade_step(void);

// $80:C35E: a four-item menu with a sliding bar (D = $0100). The bar's position
// D+$4A & $FF00 is placed between the keys at $81:BE0A (0, $B00, $1600, $2100): at a key,
// D+$14 = its value at $81:BE12; between two, the value less (key - position) / (the keys'
// high bytes' step, the CPU divider) times the values' step (bits 8-23, the PPU
// multiplier). Two 8-sprite bars at D+$4A + $1D80 + D+$E7 (sprite_bar8, $1A = 0 and
// $8000; D+$74 after them); the four labels $81:BDE0/$BDE5/$BDED/$BDF3 centred at $2080 +
// D+$E7 and $B00 apart, item D+$48 in attribute $300 (the others $100). Last, D+$4B moves
// one step toward item D+$48's key.
void menu_bar_draw(void);

// $85:8000: the track's tiles (long; D = $0100, DB = $83). The kind k = the byte at $83:8410
// + `track`; its picture (the word at $83:8447 + k, doubled, picks the bank and address at
// $83:8597 / $83:8585) unpacks to $7F:0000. Then by the layout of $047B (a table at
// $85:8025: four layouts), $1F33 bit 7 cleared (first and third) or set, and the uploads
// (each with k): tiles_planar_upload_5fe0 and the four upload_tiles_* at the layout's
// offsets; row_copy_6a33 / row_copy_6b33 at 0, or in the third and fourth layouts at $20
// for a player whose flag ($039C, $03CB) is set (player 2's, and the fourth layout's:
// never run).
void track_tiles_load(uint16_t track);

// $8A:8BC2: the track's tiles for a preview (D = $0100, DB = $83): the kind k and its
// picture as track_tiles_load; then by the layout of $0444 (a table at $8A:8BED: four
// layouts) tiles_planar_upload, upload_tiles_c280 and upload_tiles_c680 at the layout's
// offsets, and palette_load_70 (all with k).
void track_tiles_preview(uint16_t track);

// $8A:C056: a title card shown for three seconds (D = $0100). The fade state ($0196-$019A)
// and the direct page scratch are cleared; the screen is loaded ($8A:C4B0), VRAM cleared,
// tiles unpacked ($8A:BA55, $87:F861 to VRAM $0800, $87:F5BC to 0) with the text
// palettes; sprites and the $7F tilemap cleared; the frame handler $0008 set to $FC5F
// (the old one put back at the end); BG1's vertical scroll $FFBF; six sprite slots at
// ($58, $14) and five lines of tile text. It fades in ($0198 = $0F00, $0196 = $80), waits
// 180 frames one at a time, fades out ($0196 = $FFC0) and waits until the fade ends
// ($019A = 0). Returns the frame its last one-frame wait ended on.
uint16_t title_card_show(void);

// $8A:8828: VRAM built byte by byte with DMA channel 0 (one register a time). The high
// bytes of words 0-$8FF from $7E:8633 and of $1200-$155F from $7E:9833 (VMAIN as found);
// then, with VMAIN = 0 (step after the low byte), the low bytes of words 0-$3FFF filled with
// the byte at $8A:88DD; then six rows of 15 low bytes from $7E:80BA on (the source going
// on) at $0308, $0388, ... $0588; VMAIN = $80 at the end.
void vram_planes_build(void);

// $82:F1C1: the player car's shift along the road (D = $0100, DB = $30). Row x = D+$16 of
// the tables at $7E:3DEC/$3DEE/$3DF0: the 32-bit sum of (+$3DEE - +$3DEC) * 4 times ($8000
// - D+$14) / 2 and of the next step's width times 2 times D+$14 (two DSP-4 multiplies; the
// width is +$3DF0 - +$3DEE, or, for a row flagged at $3D6C (bit 15 set, bit 1 clear),
// +$3DEE less its signed byte at $3AEF less +$3DEC), negated into $020A:$020C (the word at
// $020B is read first into D+$22). Then the car's offset D+$1E:$20 ($1EF4:$1EF6 less
// $1EC2:$1EC4, or 0 with $020C = 0 while $1E66 is not negative); $01CE gains D+$22 less the
// new $020B; $0204:$0206 lose the offset and are copied to $0200:$0202, and added to
// $1EF4:$1EF6. D+$24 = -$1EF6 (the old).
void road_shift_apply(void);

// $82:EFDA: road_shift_apply's front and tail with the offset settled (D = $0100, DB = $30):
// $0206 and D+$24 (-$1EF6) of opposite signs zero it ($020C = 0, $1E66 bit 15 cleared);
// $0206 past D+$24 snaps it to $1EF4 - $1EC2 ($1E66 bit 15 set); else the factor v (from
// the gap and |D+$24|: a fraction on the CPU divider below $100, else the gap less half
// of it, plus $80, zeroed or snapped out of range) scales $020C and $1EF6 - $1EC4 (DSP-4
// multiplies: $020C = the product >> 8, the offset = the product << 8).
void road_shift_settle(void);

// $83:F73E: the view follows car $1EBA (D = `dp`, $0200 in the game; DB = $30), while bit 0
// of D+$44 is set (else nothing: never run). The gap ($7E:FAB5 of the car, 32 bits, less
// D+$25:$27, less $40) / 16 + 8, at most $80, times $1E9E (a DSP-4 multiply) moves D+$25:$27
// on; the new gap is left in $0114:$0116. Reaching or passing it (the gap 0 or below $40
// before the $40) snaps D+$25:$27 to the car's position less $40, clears bit 0 of D+$44 and
// counts $1ECE down ($8000 becomes $118).
void follow_car_step(uint16_t dp);

// $82:B4ED: the race HUD's numbers (D = $0100, DB = $83), unless $0244 has bit 2:
// - $003E = the high word of $0213 * $160 (a DSP-4 multiply), at most $0C (its absolute
//   value: never run);
// - the speed: the high word of $0210 (made positive: never run) * 8 times $3000 ($4D3B
//   when $03A7 is set) on the chip, or $1E94 + 1 while $1E94 is not negative (never run);
//   its digits as tile pairs from $83:A4AC (4 bytes a digit): thousands to $1E96/$1E98
//   (never run), hundreds to $0042/$0044, tens to $0046/$0048, ones to $004A/$004C
//   (blank ones, $0157, where none);
// - $0064/$0066: D+$42 as a tens tile ($83:A486/$A488 from 10/20) and a ones tile
//   ($83:A484 + 2n);
// - a bar of 8 cells from $0258 (0 below 9 while $0297 is negative): $0074, $006C-$0072
//   ($2959 empty, $295A full, $295C partial) and the partial cell's 8 rows at $0080-$008E;
// - $00A0 from $83:8C35 by $025B, $00A4-$00AA from $83:A4D4 by D+$4C.
// Then the car's map block: func_82B838 at D+$50, func_82B769 with $0226/$0235, D+$50 = the
// block's offset (two players' blocks: never run); and race_message_update on the
// messages at $7E:4E3D/$0233/$7E:4E45/$7E:4E4D (written back after).
void race_hud_digits(void);

// $83:FB4A: the random pickup (object kind 24; never run in a movie, reached through
// car_track_step). On func_83FC05's hit: the entry at $7E:8CB3 + $011A cleared, effect
// $FF19 for $1EBE (func_83FE9A), D+$4A = $801E, and one of 16 prizes by rng_next_long's
// bits 1-4 (the table at $83:FB76): money + 4000, + 1000 or + 10000 (BCD, car $1ECC;
// D+$33 = $47, $48, $4D); D+$4E + 1 unless its low byte is 10 ($4C); D+$58 = $28 and
// D+$56 = $FFFF ($4A); D+$5A = 0 ($4B); D+$48 = $300 ($4E). The value in parentheses
// goes to D+$33.
void track_object_random(uint16_t dp);

// $83:F7F9: the player's car moves on (D = `dp`, $0200 in the game; DB = $30 inside, $83
// after). By D+$44: bit 5 clear: position D+$25:$27 += $1E9E * D+$10 (unless bit 0); bit 5
// set, bit 0 clear: bit 8 set: D+$5E counts down by $1E9E, else (D+$5E 0) D+$70 drops by
// 4 * $1E9E, ending at D+$72 (the end below); bit 8 clear: D+$70 rises by 4 * $1E9E to at
// most $100 and the position moves by $1E9E * D+$70, ending when D+$26 reaches D+$5E. The
// end: D+$44 loses bits 5 and 8, gains bit 7, D+$1A = 4, effect $FF2A for $1EBE. Bit 7 of
// D+$44 on entry clears bit 3 of the car's flags ($7E:FADE + $1EBA). Then car_lap_check,
// and every track segment (256 position units) crossed runs its object (the entry at
// $7E:8CB3 + 4 * the segment, wrapping at 4 * $019A) through its kind's handler (the
// table at $83:7FFE, kinds 1-104: the hit tests and pickups, D+$53 markers, a stop
// marker setting D+$5E), with $011E summing the road's curve ($7E:3DEC) less D+$0C. D+$2D:$2F
// = the position's change.
void car_track_step(uint16_t dp);

// $82:D8C6: one car's dot on the mini-map (DB = $30). Its place in the map table at
// $7E:4F31 is `at` (4 bytes a step), or, when the $82:DC68 entry for $047B is negative and
// D+$14 is not 0, the start (D+$14 negative) or the end (($019A - 1) / 4, a multiple of 4)
// of the track. A DSP-4 sprite entry (dsp4_sprite) gets x = the table's byte 0 + $BE, y =
// (`y` + its byte 1) & $FF and the attribute `attr`; a shown entry's two words go to
// $1576/$1578 + $0184, which moves on by 4.
void minimap_car(uint16_t y, uint16_t at, uint16_t attr);

// $82:F2B3: the player's position $0226 crosses the track's segments (the 20-byte entries
// at $84:0000 + $019C; D = $0100). An armed jump ($0226 at or past $1EDE) first moves
// $0226 and $023E by $1EE0, rebuilds the road's curve at $7E:3DEC/$3E2C for row $1EDE
// (moving $0206/$0202 and setting $020C, $1EC2:$1EC4), and disarms ($1EE0 = 0, $1EDE =
// $7FFF). Then each segment whose start is $3F or more behind $0226 + 1 (the cursor $1E5E)
// runs its kind's handler (byte +2: the table at $82:F374): nothing; with bit 15 of $1E66,
// $0204:$0206 += $1EC2:$1EC4 and the segment's jump is armed ($1EDE = start + $3F + its
// +$10, $1EE0 = its +$12, $020A = $020C = 0), else $0226 and $023E move by $1EE6 (then 0);
// or $1E66 = 0 (never run). A second cursor ($1E60) follows each start $0226 + 1 reaches,
// setting bit 1 of D+$2A (cleared first).
void player_segment_step(void);

// $82:FA4F: player_segment_step for the second player: its place $02D9 (and $02F1), its
// jump $1EE2/$1EE4, its cursors $1E62/$1E64, its road rows at $7E:436C/$43AC, its words
// $02B5-$02BF (as $0202-$020C), $1EC6/$1EC8, $1E68 and $1EE8; the same segment table
// ($019C) and handler kinds (the table at $82:FB10).
void player2_segment_step(void);

// $82:EEB7: one of the road-shift handlers (the table at $82:ED17), by bit 5 of $0226. Set:
// road_shift_apply with $1E66's bit 0 moved to bit 15 (the rest shifted right), then $1E66
// as it was. Clear: road_shift_settle, then $1E66 keeps its bits 15-1 from before and takes
// bit 0 from the bit 15 the settle left.
void road_shift_by_place(void);

// $82:EC36: the road shift's frame step (D = $0100; DB = $30 inside, $83 after). D+$3C = 0,
// D+$16 = the row offset ($0226 * 2, & $3E), D+$14 = t ($0224 / 2, & $FF80); $1EF4:$1EF6 =
// the road's place at t ($7E:3DEC row and its slope times t, a DSP-4 multiply; the high
// word less 1 when positive). Then a handler by $0261 (the table at $82:ED23: clear $0242
// and $0208, or the road's edges from rows $3C6C/$3F6C/$3E6C, never run) and one by $0260
// (the table at $82:ED17: race_scroll_commit, the edge shift from $1EFA (never run),
// road_shift_by_place, by row (never run), race_scroll_commit, road_shift_apply);
// $1EC2:$1EC4 = $1EF4:$1EF6 from between the two; $0229 = $01CE; $01CE = |$01CE| / the
// low byte of $1E9E (the CPU divider) with the opposite sign (a DSP-4 multiply by -1 or 1).
void road_shift_frame(void);
// $82:EC32: JSR road_shift_frame.
void road_shift_frame_call(void);

// $82:EA80: the track-event table's empty handler (RTS).
void track_event_none(void);

// $82:A9AF / $82:A9B3: JSR player_segment_step / player2_segment_step; RTL (reached by JSL
// from bank $85).
void player_segment_step_long(void);
void player2_segment_step_long(void);

// $82:A7F9: the track's setup load (D = $0100), for track $047B (tables in bank $84):
// its header unpacked to $7E:5391 (from bank $9B), its object table to $7E:8CB3, a block
// from bank $8B copied to $7E:5B91; the scenery unpacked to $7F:0000 by the variant
// ($03DC & 3 = 2: $89:8512, never run) or $01E4 (negative $96:8020, zero $96:8097,
// positive $97:A24D); $003A and $0038 from the bank-$85 tables ($85:A3CC with $039C /
// $03CB set, the latter never run; else $85:A35E), a negative entry replaced by the word
// at $90:8000 + the header's $5395 - $8007. Then each object kind on the track (from the
// last entry down) unpacks its graphics once into $7F, from D+$14 = $D2 ($0612 for
// variant 2, never run) on: its slot in the table at $7E:6591 ($FFFF, cleared first)
// takes the place.
void track_setup_load(void);

// $8A:99FC: line `x` of the text map at $7F:7918 (as text_line_push) centred in $1C
// columns: ($1C - its length) / 2 spaces first (none counted as 65536), then the line.
void text_line_centered(uint16_t x);

// $8A:9908: the results text (D = $0100, DB = $8A). results_text_build, func_8A9BE2 (the
// largest kind), D+$5E = 0, variant_66; then for each slot func_8A9BB5 takes, D+$5E + 1:
// its rank (byte_buffer_decimal2), a space, its name (text_line_push), padding to $16, three
// spaces, its value ($041C + slot, byte_buffer_decimal) and the line's end. With bit 15 of
// $1B37 (never run): two line ends, the string at $8A:9DBF, the slots restarted (func_8A9BE2,
// D+$66 = 8, the first four cleared), one taken and its name centred between two pairs of
// line ends, and the string at $8A:9DD5. Last the string at $8A:9DBD (to $FF).
void results_text(void);

// $80:8119: the saved data checked at boot ($03E2 = 0). The $78 bytes at $7E:FA3D summed
// (8-bit bytes into a word) ^ $7645 equal to $7E:FA3B: that copy is restored to $0368 ($7A
// bytes) and $045A loses its low byte (never run). Else the same over $0368 against $0366:
// $045A loses its low byte (never run). Else the defaults ($7A bytes from $83:85A9) go to
// $0366, save_data_seal, and $045A's low byte is set. Returns whether the defaults were
// loaded (the original's carry).
bool save_data_check(void);

// $82:AE15: the sprites' high table from the DSP-4 (D = $0100): command $0006 through the
// port (dsp4_port_write16), its 16 words (dsp4_port_read16; the chip's high bits of the
// sprites the earlier commands placed) to $1776 + D+$8E; then the sprites left unused, from
// D+$86 to D+$8E + $200, moved off screen ($E0E0 every 4 bytes from $1576 + D+$86; a
// count below 4 bytes is taken as 65536).
void oam_high_load(void);

// $82:D934 / $82:D988: one object's sprite streamed to the DSP-4 (D = $0100; a $09 sprite
// stream is open): seven words through the port (the object's rows $386C, $38EC, then $35EC
// and $37EC, or $366C and $386C, at D+$14; D+$20, D+$22; D+$26 | D+$3A, or | $1000), then
// its parts from the list at $7F:0000 + the object's graphics place ($7E:6591 slot of
// $86:0000 + D+$24, plus $86:0002 + the part set's offset): $82:DDBF. Two lists, each
// 6-byte parts (a nonzero word, two more), ended by a 0 word; each part's answer is pairs
// of OAM words (to $1574 + Y, Y from D+$84 on, 4 bytes a pair) while the chip's flag is
// not 0. Y reaching D+$1A fills the buffer: the rest of the answers are drained, the open
// lists ended (a 0 word each) and the result is true. D+$84 = Y.
bool sprite_put_rows_a(void);
bool sprite_put_rows_b(void);

// $82:DEA8: sprite_parts_stream for a parts list in ROM, at $86:0000 + `x` (D = $0100; a
// $09 sprite stream open). Returns whether the OAM buffer filled (the original's carry).
bool sprite_parts_stream_rom(uint16_t x);

// $82:D267: a car's sprite head streamed to the DSP-4 (D = $0100; a sprite stream open):
// the row $7E:386A + `x`, $9000, the car's word $7E:FAB4 + `y`, twice its height ($0243's
// byte, signed (never run when negative), + $33 + D+$34), 0, D+$32, its frame (the word at
// $83:ACEC + (its $FAB5 / 2 & $7E) + the row $39EC at D+$14), 0; the chip's answer word to
// D+$2A, which it returns.
uint16_t car_sprite_head(uint16_t x, uint16_t y);

// $82:DAFD / $82:DB4D: a track object's sprite (D = $0100): D+$24 = the graphics word at
// $86:AF06 + `x`; for the object at $7E:8CB3 + D+$1C: D+$20 = its place (its byte * 16, a
// signed 12-bit value), D+$26 gains its bits 8-10 (of the word at +1) moved up one. The
// first: D+$22 = the height at $84:A200 + (($0024 - its word at +3) & $3E) less (its +2
// byte's high nibble * 8) and $20; then sprite_put_rows_a. The second: D+$22 = 0, then
// sprite_put_rows_b when its +2 byte has bit 4, else sprite_put_rows_a. Return their result.
bool object_sprite_raised(uint16_t x);
bool object_sprite_flat(uint16_t x);

#endif
