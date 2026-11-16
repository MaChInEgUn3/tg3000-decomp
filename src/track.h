// SPDX-License-Identifier: GPL-3.0-or-later
// Cars moving along the track's segment table.
#ifndef TG3000_TRACK_H
#define TG3000_TRACK_H

#include <stdbool.h>
#include <stdint.h>

// Per-car fields, bank $7E, indexed by the car's byte offset.
#define CAR_POS 0xFAB6    // position along the track
#define CAR_FLAGS 0xFADE  // $80 moved by the table, $40 started, $20 branch taken, $10 branch chosen
#define CAR_SEG 0xFAE0    // offset of the current record in the segment table
#define CAR_LIMIT 0xFAE2  // position at which the car jumps by CAR_SKIP ($7000: none)
#define CAR_SKIP 0xFAE4

// Advance car `car` through the segment table ($84:0000, 20-byte records: +$00 start
// position, +$10 length, +$12 jump). The first call only moves the current record up to
// the car (while it is $3F or more past a record's start) and marks the car started.
// Later calls do the same, but at each record passed: once per record a random bit
// decides whether the car takes the record's branch ($20); without it the car's position
// moves on by the record's length, with it the car gets a jump point (start + length +
// $3F) and a jump (+$12). Finally, a car at or past its jump point jumps (CAR_SKIP added,
// the point reset to $7000). Returns whether it jumped.
bool func_81FBB1(uint16_t car);

// Move car `car` along the road for one frame (DSP-4 multiplies by the frame's step $1E9E,
// and the road's edges from command $0A); nothing while $1ECE is set. ($81:F887)
void car_road_step(uint16_t car);

#endif
