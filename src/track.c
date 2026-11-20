// SPDX-License-Identifier: GPL-3.0-or-later
#include "track.h"

#include "dsp4.h"
#include "rng.h"
#include "tg3000.h"

#define SEGMENTS 0x840000
#define SEG_SIZE 0x14
#define SEG_START 0x00
#define SEG_LENGTH 0x10
#define SEG_JUMP 0x12
#define NEAR 0x3F  // a record counts as passed once the car is this far past its start

static uint16_t car_rd(uint16_t field, uint16_t car) { return rd16((uint32_t)field + car); }
static void car_wr(uint16_t field, uint16_t car, uint16_t v) { wr16((uint32_t)field + car, v); }

// TG3000_ROUTINE($81:FBB1) func_81FBB1
bool func_81FBB1(uint16_t car) {
  uint16_t flags = car_rd(CAR_FLAGS, car);
  if(!(flags & 0x40)) {
    car_wr(CAR_FLAGS, car, flags | 0x40);
    for(;;) {
      uint16_t seg = car_rd(CAR_SEG, car), pos = car_rd(CAR_POS, car);
      uint16_t start = bus_rd16(SEGMENTS + seg + SEG_START);
      if(pos < start || (uint16_t)(pos - start) < NEAR) return false;
      car_wr(CAR_SEG, car, (uint16_t)(seg + SEG_SIZE));
    }
  }
  for(;;) {
    uint16_t seg = car_rd(CAR_SEG, car), pos = car_rd(CAR_POS, car);
    uint16_t start = bus_rd16(SEGMENTS + seg + SEG_START);
    if(pos < start) break;
    if(!(car_rd(CAR_FLAGS, car) & 0x10)) {  // choose this record's branch, once
      car_wr(CAR_FLAGS, car, car_rd(CAR_FLAGS, car) & ~0x20);
      bool carry;
      uint16_t r = rng_next_long(&carry);
      car_wr(CAR_FLAGS, car, (uint16_t)(car_rd(CAR_FLAGS, car) | (r & 0x20) | 0x10));
    }
    if((uint16_t)(pos - start) < NEAR) break;
    car_wr(CAR_SEG, car, (uint16_t)(seg + SEG_SIZE));
    uint16_t length = bus_rd16(SEGMENTS + seg + SEG_LENGTH);
    if(!(car_rd(CAR_FLAGS, car) & 0x20)) {
      car_wr(CAR_POS, car, (uint16_t)(pos + length));
      car_wr(CAR_FLAGS, car, car_rd(CAR_FLAGS, car) | 0x80);
    } else {
      uint32_t end = (uint32_t)start + length;  // its carry goes into the next add
      car_wr(CAR_LIMIT, car, (uint16_t)(end + NEAR + (end >> 16)));
      car_wr(CAR_SKIP, car, bus_rd16(SEGMENTS + seg + SEG_JUMP));
    }
  }
  uint16_t pos = car_rd(CAR_POS, car);
  if(pos < car_rd(CAR_LIMIT, car)) return false;
  car_wr(CAR_POS, car, (uint16_t)(pos + car_rd(CAR_SKIP, car)));
  car_wr(CAR_LIMIT, car, 0x7000);
  car_wr(CAR_FLAGS, car, car_rd(CAR_FLAGS, car) | 0x80);
  return true;
}

// Add a 32-bit value to the two words at `lo` and `lo` + 2, the low word's carry into the
// high one, plus `carry` into the low word (as the original's ADC chain).
static void add32(uint32_t lo, uint32_t value, bool carry) {
  uint32_t low = (uint32_t)rd16(lo) + (value & 0xFFFF) + (carry ? 1 : 0);
  wr16(lo, (uint16_t)low);
  wr16(lo + 2, (uint16_t)(rd16(lo + 2) + (value >> 16) + (low >> 16)));
}

// Per-car fields used here (bank $7E, plus the car's byte offset).
#define CAR_DIST 0xFAB5    // 32-bit, moved by speed x step
#define CAR_SIDE 0xFAB9    // 32-bit; its high word (+2) the car's place across the road
#define CAR_SPEED 0xFACB
#define CAR_DRIFT 0xFADB   // sideways speed (flipped at the road's edges)
#define CAR_LAG 0xFAC9     // 32-bit with CAR_SPEED: the speed's fraction
#define CAR_AHEAD 0xFACD   // the car in front (its offset)

// TG3000_ROUTINE($81:F887) car_road_step
//
// The car's distance moves on by speed x step; the segment table moves it along
// (func_81FBB1); then it moves sideways by drift x (speed x step >> 8), bounces off the
// road's edges (command $0A between the rows at its place), and its speed eases towards
// the car in front's target. Two quirks of the original are kept: the sideways sum takes
// in the carry the segment step left (set when a jump wrapped the position past $FFFF),
// and the gap to the target takes its borrow from an add (a 32-bit subtraction whose low
// borrow is lost).
void car_road_step(uint16_t car) {
  const uint32_t c = car;
  const uint16_t step = rd16(0x1E9E);
  wr16(0xFAC7 + c, 0xFFFF);
  if(rd16(0x1ECE)) return;
  add32(CAR_DIST + c, (uint32_t)dsp4_multiply((int16_t)rd16(CAR_SPEED + c), (int16_t)step), false);
  bool carry = false;
  if(func_81FBB1(car)) carry = rd16(CAR_POS + c) < rd16(CAR_SKIP + c);  // the jump wrapped
  uint32_t moved = (uint32_t)dsp4_multiply((int16_t)rd16(CAR_SPEED + c), (int16_t)((step >> 8) | (step << 8)));
  wr16(DP + 0x14, (uint16_t)moved);
  wr16(DP + 0x16, (uint16_t)(moved >> 16));
  add32(CAR_SIDE + c, (uint32_t)dsp4_multiply((int16_t)rd16(CAR_DRIFT + c), (int16_t)rd16(DP + 0x15)), carry);
  int16_t edge[4];
  uint16_t row = (uint16_t)(rd16(CAR_POS + c) << 1);
  dsp4_edges((uint16_t)((rd16(0xFAB4 + c) & 0xFF00) >> 1), rd16(0xD1B3 + (uint32_t)row), rd16(0xD1B5 + (uint32_t)row), edge);
  for(int j = 0; j < 4; j++) wr16(DP + 0x18 + 2 * (uint32_t)j, (uint16_t)edge[j]);
  uint16_t side = rd16(CAR_SIDE + 2 + c);
  if(!((uint16_t)(side - (uint16_t)edge[1]) & 0x8000)) {  // past the right edge: back, drift left
    wr16(CAR_SIDE + 2 + c, (uint16_t)(side - step));
    uint16_t d = rd16(CAR_DRIFT + c);
    if(!(d & 0x8000)) wr16(CAR_DRIFT + c, (uint16_t)-d);
  } else if((uint16_t)(side - (uint16_t)edge[0]) & 0x8000) {  // past the left edge: back, drift right
    wr16(CAR_SIDE + 2 + c, (uint16_t)(side + step));
    uint16_t d = rd16(CAR_DRIFT + c);
    if(d & 0x8000) wr16(CAR_DRIFT + c, (uint16_t)-d);
  }
  uint16_t cool = rd16(0xFAE8 + c);
  if(cool) {
    uint16_t r = (uint16_t)(cool - (uint16_t)(step << 3));
    wr16(0xFAE8 + c, (r & 0x8000) ? 0 : r);
  }
  uint16_t ahead = rd16(CAR_AHEAD + c);
  if(rd16(CAR_POS + (uint32_t)ahead) != rd16(CAR_POS + c)) {
    uint16_t lag = rd16(CAR_LAG + c);
    uint16_t gap_lo = (uint16_t)(0 - lag);
    uint32_t sum = (uint32_t)(rd16(0xFAE9 + c) & 0xFF) + rd16(0xFAD7 + c);
    bool adc_carry = sum > 0xFFFF;
    uint16_t gap_hi = (uint16_t)((uint16_t)sum - rd16(CAR_SPEED + c) - (adc_carry ? 0 : 1));
    wr16(DP + 0x14, gap_lo);
    wr16(DP + 0x16, gap_hi);
    uint16_t limit = rd16(0xFAD9 + c);
    if(gap_hi & 0x8000) {  // slowing down: the gap negated, at most the limit
      bool borrow = gap_lo == 0;
      gap_lo = (uint16_t)(~gap_lo + 1);
      gap_hi = (uint16_t)(~gap_hi + (borrow ? 1 : 0));
      wr16(DP + 0x14, gap_lo);
      wr16(DP + 0x16, gap_hi);
      uint16_t m = (gap_hi != 0 || gap_lo >= limit) ? limit : gap_lo;
      uint32_t p = (uint32_t)dsp4_multiply((int16_t)m, (int16_t)step);
      uint32_t low = (uint32_t)rd16(CAR_LAG + c) - (p & 0xFFFF);
      wr16(CAR_LAG + c, (uint16_t)low);
      wr16(CAR_SPEED + c, (uint16_t)(rd16(CAR_SPEED + c) - (p >> 16) - ((low >> 16) ? 1 : 0)));
      return;
    }
    uint16_t m = (gap_hi != 0 || gap_lo >= limit) ? limit : gap_lo;
    add32(CAR_LAG + c, (uint32_t)dsp4_multiply((int16_t)m, (int16_t)step), false);
  }
  uint16_t boost = rd16(0xFAE6 + c);
  if(!boost) return;
  add32(CAR_LAG + c, (uint32_t)dsp4_multiply((int16_t)boost, (int16_t)step), false);
  uint16_t r = (uint16_t)(boost - step);
  wr16(0xFAE6 + c, (r & 0x8000) ? 0 : r);
}

// N of a - b, as the original's CMP tests it (not a signed compare)
static bool below(uint16_t a, uint16_t b) { return ((uint16_t)(a - b) & 0x8000) != 0; }

// TG3000_ROUTINE($83:E8AB) player_road_limits
void player_road_limits(void) {
  const uint32_t dp = 0x0200;
  int16_t edge[4];
  uint16_t t = (uint16_t)((rd16(dp + 0x24) & 0xFF00) >> 1);
  uint16_t row = (uint16_t)(rd16(dp + 0x26) << 1);
  uint16_t near = rd16(0xD1C3 + (uint32_t)row);
  dsp4_edges(t, near, near, edge);
  for(int j = 0; j < 4; j++) wr16(0x0114 + 2 * (uint32_t)j, (uint16_t)edge[j]);
  uint16_t far = rd16(0xD1B3 + (uint32_t)row);
  dsp4_edges(t, far, far, edge);
  if(below((uint16_t)edge[0], rd16(0x0114))) wr16(0x0114, (uint16_t)edge[0]);   // the wider of the two
  if(!below((uint16_t)edge[1], rd16(0x0116))) wr16(0x0116, (uint16_t)edge[1]);
  if(below((uint16_t)edge[2], rd16(0x0118))) wr16(0x0118, (uint16_t)edge[2]);
  if(!below((uint16_t)edge[3], rd16(0x011A))) wr16(0x011A, (uint16_t)edge[3]);
  if(!rd16(0x1ECE)) wr16(0x1E9C, (uint16_t)(rd16(0x1E9C) | 0x0001));
  // the car in front: keep clear of it by $20, on the side flag $0244 bit 1 says
  uint16_t ahead = rd16(0xFACD + (uint32_t)rd16(0x1EBA));
  uint16_t at = rd16(0xFABB + (uint32_t)ahead);
  uint16_t side = rd16(dp + 0x44);
  bool pass_left;  // true: limit the right side ($0116 = at - $20); false: the left ($0114 = at + $20)
  bool decided = true;
  if(side & 0x0002) {
    if(!below((uint16_t)(at - 0x80), rd16(0x0114)) || !below((uint16_t)(at + 0x80), rd16(0x0116))) {
      pass_left = true;
    } else if(below((uint16_t)(at + 0x80), rd16(0x0114))) {
      decided = false;
      pass_left = false;
    } else {
      wr16(dp + 0x44, (uint16_t)(rd16(dp + 0x44) & ~0x0002));
      pass_left = false;
    }
  } else {
    if(below((uint16_t)(at + 0x80), rd16(0x0116)) || below((uint16_t)(at - 0x80), rd16(0x0114))) {
      pass_left = false;
    } else if(!below((uint16_t)(at - 0x80), rd16(0x0116))) {
      decided = false;
      pass_left = false;
    } else {
      wr16(dp + 0x44, (uint16_t)(rd16(dp + 0x44) | 0x0002));
      pass_left = true;
    }
  }
  if(decided) {
    if(pass_left) wr16(0x0116, (uint16_t)(at - 0x20));
    else wr16(0x0114, (uint16_t)(at + 0x20));
  }
  wr16(0x011E, 0);
  if(!(rd16(0x1F31) & 0x8000)) {
    uint16_t kind = (uint16_t)((rd16(dp + 0x1A) & 0x7FFF) - 1);
    if(kind != 0 && kind != 5 && (rd8(dp + 0x53) != 0)) {  // a pushed side: both limits at it
      uint16_t push = (uint16_t)(rd8(dp + 0xB1) << 4);
      if(push & 0x0800) push |= 0xF000;
      wr16(0x0114, push);
      wr16(0x0116, push);
    }
    wr16(0x011E, 0);
    if(rd16(0x1F2C) & 0x0001) {  // the road bends: everything shifts by the bend
      uint16_t bend = rd16(0x3DEC + (uint32_t)rd16(0x1EBC));
      wr16(dp + 0x14, (uint16_t)(rd16(dp + 0x14) - bend));
      wr16(dp + 0x16, (uint16_t)(rd16(dp + 0x16) - bend));
      wr16(0x011E, (uint16_t)(0 - bend));
    }
  }
  uint16_t width = (uint16_t)(rd16(0x0116) - rd16(0x0114));
  uint16_t middle = (uint16_t)(((width >> 1) | (width & 0x8000)) + rd16(0x0114) + rd16(0x011E));
  wr16(0x011C, middle);
  uint32_t lane = rd16(0x1EBC);
  for(uint32_t flag = 0x3B08; ; flag = 0x3AEC) {  // two side flags: keep to the left or right limit
    uint16_t f = rd16(flag + lane);
    if(f) {
      if(f & 0x8000) {
        wr16(dp + 0x44, (uint16_t)(rd16(dp + 0x44) & ~0x0002));
        wr16(0x011C, rd16(0x0116));
      } else {
        wr16(dp + 0x44, (uint16_t)(rd16(dp + 0x44) | 0x0002));
        wr16(0x011C, rd16(0x0114));
      }
    }
    if(flag == 0x3AEC) break;
  }
  uint16_t off = (uint16_t)((rd16(0x011C) - rd16(dp + 0x06)) << 1);
  if(!below(off, 0xFFD0) && below(off, 0x0030)) off = 0;  // within $18 of the middle: none
  if(off & 0x8000) wr16(0x1E9C, (uint16_t)(rd16(0x1E9C) | 0x0004));
  else if(off) wr16(0x1E9C, (uint16_t)(rd16(0x1E9C) | 0x0008));
}

// TG3000_ROUTINE($81:FA8F) car_sync_player
bool car_sync_player(uint16_t car) {
  const uint32_t o = car;
  const bool direct = !(rd16(0x0244) & 0x0001);
  uint32_t pos = rd16(0xFAB5 + o) | (uint32_t)rd16(0xFAB7 + o) << 16;
  if(direct) pos += rd16(0x022D) | (uint32_t)rd16(0x022F) << 16;
  else pos += (uint32_t)dsp4_multiply((int16_t)rd16(0x1E9E), (int16_t)rd16(0x0210));
  wr16(0xFAB5 + o, (uint16_t)pos);
  wr16(0xFAB7 + o, (uint16_t)(pos >> 16));
  wr16(0xFAC9 + o, rd16(0x020E));
  wr16(0xFACB + o, rd16(0x0210));
  if(direct) {
    wr16(0xFAB9 + o, rd16(0x0204));
    wr16(0xFABB + o, rd16(0x0206));
  }
  wr16(0xFAC7 + o, 0xFFFF);
  const bool carry = func_81FBB1(car);
  if(direct) {
    uint16_t f = (uint16_t)(rd16(CAR_FLAGS + o) & ~0x0020);
    if(rd16(0x1E66) & 0x8000) f |= 0x0020;
    wr16(CAR_FLAGS + o, (uint16_t)(f | 0x0010));
  }
  return carry;
}
