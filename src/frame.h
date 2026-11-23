// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef TG3000_FRAME_H
#define TG3000_FRAME_H

#include <stdint.h>

// Incremented once per frame by the NMI handler.
#define FRAME_COUNTER 0x0024

// Wait until the frame counter has advanced by `n` (n = 0 waits a full 65,536 frames,
// as the original does). Returns the counter value it waited for.
uint16_t wait_frames(uint16_t n);

// Set bit 4 of D+$76 and wait until an interrupt sets its bit 3. Returns D+$76.
uint16_t wait_flag_76(uint16_t dp);

// wait_frames(1), the caller's A kept.
void wait_one_frame(void);

// Interrupt handlers (docs/DECISIONS.md, "HDMA, measured"): called by the platform where the
// console takes the interrupt.

// IRQ $85:95BD: acknowledges the interrupt and returns; nothing else.
void irq_ack_only(void);

// IRQ $80:E3F7, the first split of that screen: while $017A (the split's height) is
// below 100, the next interrupt's dot $100, then (after its WAI) BG1-BG3 scrolled by the
// word $0120, main screen $17 and sub screen $10, colour math $05, colour 0 = $7CC6, the
// interrupts $B1 with the next one at line $B3, dot $10, handled by irq_split_bottom (the
// trampoline at $0005). Always: fixed colour black, $2130 = $02, windows $AA/$0A/$0A.
void irq_split_top(void);

// IRQ $80:E48A, the second split of the screen $80:E28C sets up: the next line interrupt
// at dot $100, then (on the console, after a WAI for it) BG1-BG3 scroll back to 0, main
// screen BG1-BG3 and sprites, no sub screen, colour math on BG1 and the backdrop ($05).
void irq_split_bottom(void);

// $80:8934, the NMI's common tail (most of its dispatch targets end here): read the pads
// unless bit 2 of $0026 is set, step the sound request queue (func_80894E), send the
// sound message (sound_frame), count the frame (FRAME_COUNTER).
void nmi_tail(void);

// NMI dispatch targets that end in nmi_tail.
// $80:FBD1: func_8A8000, then the tail.
void nmi_target_80FBD1(void);
// $80:FC5F: set bit 1 of $0026, func_8AA050 (with D = $0100, DB = $8A), then the tail.
void nmi_target_80FC5F(void);
// $80:E28C: the split screen. $017A (the split's height) shrinks by 4 to 0; the HDMA
// tables at $1B00, $0D00, $0D28 and $0D14 are rebuilt for it; the line interrupt is set up
// (line $7E, dot $10, irq_split_top through the trampoline at $0005), the top of the screen
// gets scroll 0, colour 0 = $1BE6, colour math $17 / $12 with the fixed colour from $017C
// and $017D, windows $22/$02/$82, layers $17/$00; then the tail.
void nmi_target_80E28C(void);
// $80:E175 (D = $0100 while it runs): until $0116 is set, rebuild the HDMA table at $1B00
// for a band that opens by 4 lines a frame ($0114, up to $3A, then $0116 = 1); then the
// tail.
void nmi_target_80E175(void);
// $80:B97B: the menus' NMI frame (the two-task kernel's, up to its stack switch at $80:BC54).
// Forced blank; the fade at $0296-$029A as nmi_prologue's; both pads' new presses ($000C to
// $010C/$010E, $0010 to $020C/$020E); the frame's task ($0190/$0290 by the counter's bit 0);
// that task's upload ring drained against a $0C00 budget (task 1 $018E-$018C, task 2
// $028E-$028C, a part sent when it does not fit); the shadowed screen registers ($019C-
// $01C1, the vertical scrolls less $01C2/$01C4); the palette ($81:0500) and sprites ($81:
// $016E); the raster IRQ (line $6C, dot $10, $80:BC9B through the trampoline); the large
// upload at $0164-$016A in $1C00-byte parts; the brightness $019B (0: forced blank).
void nmi_menu_frame(void);
// $80:D784: copy the 7-byte table at $81:C207 to $1B00, upload 4 KB from $7F:0000 to VRAM
// $0000, set the screen up (mode $09, maps $51/$03/$59, tiles $03 for BG3/4, sprites $03,
// main screen BG2, BG3 scrolled to (0, $3FF), $2130 = $02, colour 2 black); then the tail.
void nmi_target_80D784(void);
void nmi_prologue(void);  // $80:8807, up to its dispatch through $0008

// $80:80E4: the interrupt jumps in low RAM: $0000 JML $80:8807 (the NMI), $0004 JML
// $80:89B2 (the IRQ), the NMI's dispatch target $0008 = $8934; then $0026, $019A,
// $002A and $0028 cleared.
void interrupt_vectors_init(void);
void nmi_race_prologue(void);  // $85:8377, up to its dispatch (nothing to do)
void nmi_race_tail(void);  // $85:838A: the race NMI's tail (every race frame)

// $84:A4CD: wait until the word $03E0 is 0 (the interrupts count it down). Returns 0.
uint16_t wait_03e0(void);

#endif
