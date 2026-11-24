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

// The player car's limits across the road ($0114 left, $0116 right, their middle $011C)
// from the road's edges at its place ($0A on the near row, then on the far row: the
// widest of the two), the car in front, a pushed side, and the side flags; then which
// way the car is off the middle ($1E9C bits 2/3). The original runs with D = $0200.
// ($83:E8AB)
void player_road_limits(uint16_t dp);

// $81:FA8F: the player's car `car` (its offset in the tables at $7E:FAB5) takes the player's
// motion. Directly (bit 0 of $0244 clear): its position gains $022D:$022F, its speed
// (+$14, +$16) = $020E/$0210 and its +$04/+$06 = $0204/$0206; else its position gains
// $1E9E * $0210 (a DSP-4 multiply) and its speed = $020E/$0210. Its +$12 = $FFFF; then
// func_81FBB1. Directly, its flags lose bit 5, get it back from bit 15 of $1E66, and get
// bit 4. Returns func_81FBB1's result (the original's carry).
bool car_sync_player(uint16_t car);

// $81:FB20: car_sync_player for the second player (the record at $02F7; bit 15 of $1E68).
// Handler 4 of $81:F876; never run in a one-player race.
bool car_sync_player2(uint16_t car);

// $81:FC72 / $81:FCD1 / $81:FD30 / $81:FD8F (b = 0, $B3, $62, $115): the car takes the record
// at $0244 + b's distance moved (directly or the DSP-4 product) and, directly, its sides;
// +$12 = $FFFF. Nothing else. Handlers 8-14 of $81:F876; never run.
void car_sync_slot(uint16_t car, uint16_t b);

#endif
