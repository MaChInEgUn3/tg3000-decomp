// SPDX-License-Identifier: GPL-3.0-or-later
// The game's start-up (port request C).
#ifndef TG3000_BOOT_H
#define TG3000_BOOT_H

// $80:8002 (the reset vector's code) up to its call of the top loop ($80:809B). With the
// interrupts off, after two waits for the vertical blank: HDMA off and the screen blanked;
// then (the CPU's own set-up, D = $0100 and the stack at $1FFF, done by a port's start) the
// WRAM cleared except $7E:0366-$03DF and $7E:FA3A-$FFFF (kept across a reset: the saved
// data's copy is at $FA3B on); the start screen's registers ($80:80A0); the sound
// driver started (sound_boot); the interrupt vectors; the NMI and the controllers' automatic
// read on ($4200 = $81); the saved data checked; then the DSP-4's start-up awaited. A port
// calls it once, after loading the ROM, with WRAM as the console leaves it (any contents:
// what is kept is the saved data a port restores).
void game_boot(void);
// Its three parts: $80:8002 up to the CPU's set-up ($80:802E); $80:803C up to the interrupts
// switched on ($80:807A); then the saved data's check and the DSP-4's start.
void game_boot_head(void);
void game_boot_body(void);
void game_boot_tail(void);

#endif
