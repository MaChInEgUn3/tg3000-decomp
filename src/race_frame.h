// SPDX-License-Identifier: GPL-3.0-or-later
// Pieces of the race NMI's full frame ($85:83CE).
#ifndef TG3000_RACE_FRAME_H
#define TG3000_RACE_FRAME_H

#include <stdbool.h>

// The race's VRAM upload queue: a ring of 128 transfers in parallel word tables in
// $7E:F433-$7E:F9B2, indexed by a byte offset (even, wraps at $100). Each entry: size,
// source address, VRAM word address, source bank (low byte; bit 15 set = skip the entry),
// and an optional WRAM address ($7E:xxxx, 0 = none) that gets a word once it is sent.
#define RACE_QUEUE_SIZE 0xF433
#define RACE_QUEUE_SRC 0xF533
#define RACE_QUEUE_DEST 0xF633
#define RACE_QUEUE_BANK 0xF733
#define RACE_QUEUE_FLAG_ADR 0xF833
#define RACE_QUEUE_FLAG_VAL 0xF933
#define RACE_QUEUE_TAIL 0x00FE    // next entry to send
#define RACE_QUEUE_HEAD 0x0100    // where the next entry goes
#define RACE_DMA_BUDGET 0x00FC    // bytes the frame may still send by DMA (signed)

bool race_upload_queue_drain(void);       // true: a flag word was written
bool race_upload_queue_drain_long(void);  // $83:96D1
void func_838989(void);
void func_838989_long(void);              // $83:96D5
void race_cycle_blocks(void);             // $85:9147
void race_hud_draw(void);                 // $85:8814 (long)
void irq_race_band(void);                 // $85:87BA, the race's raster IRQ

#endif
