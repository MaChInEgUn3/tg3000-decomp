// SPDX-License-Identifier: GPL-3.0-or-later
#include "track.h"

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
