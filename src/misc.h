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
// Returns true when refused (the original's carry).
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

#endif
