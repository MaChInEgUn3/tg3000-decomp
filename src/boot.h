// SPDX-License-Identifier: GPL-3.0-or-later
// The game's start-up (port request C).
#ifndef TG3000_BOOT_H
#define TG3000_BOOT_H

#include <stdbool.h>
#include <stdint.h>

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

// The game whole (port request C): game_boot, then the top loop ($80:89DB), which never returns:
// the title card, the intro, the options' main menu, then the championship's loop (its screen,
// the shop, the race, the ticker, the prizes, the results, the next track's title) or the
// two-player game, and back to the options. The screens no movie reached are traps
// (tg3000_unported, named): the ending, the two-player game's last screen, the options' choice 3.
void game_main(void);

// $80:FC2D (from $80:FC2A, $80:FC25, $80:FC20 with X = 0, 1, 2): the options screens' entry from
// the top loop (options_screens(x)), with HDMA off around it, clear_dp_scratch and $0D00-$0DFF
// cleared after. Its two rows: before the call ($80:FC2D) and after it ($80:FC45).
void options_entry(uint16_t x);
void options_entry_head(void);
void options_entry_tail(void);

// The top loop's own pieces between its calls (rows of their own):
// $80:89EB: the sound volumes after set 0 ($045F = 1, $045D/$045C/$045E = $7F).
void top_volumes(void);
// $80:8A2B: after the options' main menu: the results reset unless $03DC & $5000, its bits 15
// (a trap when set) and 14 cleared, bit 13 (a trap), $047D's low byte cleared. Returns whether
// the game is the two-player one ($03DC & $0F == 2).
bool top_after_options(void);
// $80:8A9B: the results counters and slots stepped for each track before $0444.
void top_tallies(void);
// $80:8AC3: before the race: $03E0 = $FFFF, $047B = $0444.
void top_race_prep(void);
// $80:8ADF: after the race: $03E0 = 1; returns whether there is no ticker ($0446 = $FFFF): the
// championship is over.
bool top_after_race(void);
// $80:8AF1: after the ticker: returns whether the player(s) have no place: the championship is over.
bool top_after_ticker(void);
// $80:8B25: after the next track's title. The last track done: the game's ending, the next
// difficulty, the championship over. Else $047B = $0444; then the next championship screen,
// after the options' password screen when the track's group ($84:A240) changed, unless the
// player is not in the first five (the options' choice 3, the championship over).
enum { TOP_NEXT, TOP_PASSWORD, TOP_OVER };
int top_after_title(void);

#endif
