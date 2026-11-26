// SPDX-License-Identifier: GPL-3.0-or-later
#include "boot.h"

#include <string.h>

#include "frame.h"
#include "misc.h"
#include "platform.h"
#include "sound.h"
#include "tg3000.h"
#include "video_dma.h"
#include "shop.h"
#include "vs.h"

// TG3000_ROUTINE($80:8002) game_boot_head
void game_boot_head(void) {
  platform_set_interrupts(0x00);
  platform_wait_vblank_start();
  video_hdma_enable(0x00);
  video_set_display(0x80);
  platform_wait_vblank_start();
}

// TG3000_ROUTINE($80:803C) game_boot_body
// (between the two, $80:802E-$80:803B: D = $0100, the stack at $1FFF, a JML into the fast
// bank: the CPU's own set-up)
void game_boot_body(void) {
  memset(&g_ram[0x10000], 0, 0x10000);   // $7F:0000-$FFFF
  memset(&g_ram[0x0000], 0, 0x0366);     // $7E:0000-$0365
  memset(&g_ram[0x03E0], 0, 0xFA3A - 0x03E0);  // $7E:03E0-$FA39
  video_load_screen(0x8080A0);
  sound_boot(DP);
  interrupt_vectors_init();
  platform_set_interrupts(0x81);
}

// TG3000_ROUTINE($80:807A) game_boot_tail
// (the NMI is on from here: its first one lands during the saved data's check)
void game_boot_tail(void) {
  save_data_check();
  platform_wait_dsp4_start();
}

void game_boot(void) {
  game_boot_head();
  game_boot_body();
  game_boot_tail();
}

// TG3000_ROUTINE($80:FC2D) options_entry_head
// (its D = $0100 and DB = $8A are the CPU's)
void options_entry_head(void) { video_hdma_enable(0x00); }

// TG3000_ROUTINE($80:FC45) options_entry_tail
void options_entry_tail(void) {
  clear_dp_scratch();
  for(uint16_t x = 0; x < 0x0100; x += 2) wr16((uint16_t)(0x0D00 + x), 0);
  video_hdma_enable(0x00);
}

void options_entry(uint16_t x) {
  options_entry_head();
  options_screens(x);
  options_entry_tail();
}

// TG3000_ROUTINE($80:89EB) top_volumes
void top_volumes(void) {
  wr8(0x045F, 0x01);
  wr8(0x045D, 0x7F);
  wr8(0x045C, 0x7F);
  wr8(0x045E, 0x7F);
}

// TG3000_ROUTINE($80:8A2B) top_after_options
bool top_after_options(void) {
  if(!(rd16(0x03DC) & 0x5000)) results_init();
  const uint16_t v = rd16(0x03DC);
  wr16(0x03DC, (uint16_t)(v & ~0x8000));
  if(v & 0x8000) tg3000_unported("$80:8A3F: the top loop's bit 15 of $03DC");
  wr16(0x03DC, (uint16_t)(rd16(0x03DC) & ~0x4000));
  if(rd16(0x03DC) & 0x2000) tg3000_unported("$80:8A61: the top loop's bit 13 of $03DC");
  wr16(0x047D, (uint16_t)(rd16(0x047D) & 0xFF00));
  return (rd16(0x03DC) & 0x000F) == 2;
}

// TG3000_ROUTINE($80:8A9B) top_tallies
// (the track in D+$14)
void top_tallies(void) {
  if(rd16(0x0444) == 0) return;
  wr16(DP + 0x14, 0);
  do {
    result_tally_at(rd16(DP + 0x14));
    result_slot_step_at(rd16(DP + 0x14));
    wr16(DP + 0x14, (uint16_t)(rd16(DP + 0x14) + 1));
  } while(rd16(DP + 0x14) != rd16(0x0444));
}

// TG3000_ROUTINE($80:8AC3) top_race_prep
void top_race_prep(void) {
  wr16(0x03E0, 0xFFFF);
  wr16(0x047B, rd16(0x0444));
}

// TG3000_ROUTINE($80:8ADF) top_after_race
void top_after_race(void) {
  wr16(0x03E0, 0x0001);
  if(rd16(0x0446) == 0xFFFF) tg3000_unported("$80:8B11: the championship's end (no ticker)");
}

// TG3000_ROUTINE($80:8AF1) top_after_ticker
// (the two players' test at $80:8B09 never ran; decoded from the ROM)
void top_after_ticker(void) {
  if(rd16(0x03E2) & 0x0001) return;
  uint16_t v = rd16(0x03A8);
  if((rd16(0x03DC) & 0x00F0) == 0x0030) v &= rd16(0x03D7);
  if(v & 0x8000) tg3000_unported("$80:8B11: the championship's end (no place)");
}

// TG3000_ROUTINE($80:8B25) top_after_title
bool top_after_title(void) {
  if(rd16(0x0444) == 0x00FF) tg3000_unported("$80:8B5F: the championship's last track done");
  wr16(0x047B, rd16(0x0444));
  if((uint16_t)rom8(0x84A240 + (uint32_t)rd16(0x0444)) == rd16(0x0368)) return false;
  if(!(rd16(0x03E2) & 0x0001) && player_in_top5()) tg3000_unported("$80:8B50: the player not in the first five");
  return true;
}

// The championship screen whole ($80:BE32, or $80:BEC6 the first time): its pieces are in C,
// its session's two tasks are not yet (port request D).
static void champ_screen(bool first) {
  (void)first;
  tg3000_unported("the championship screen's two tasks ($80:BE32/$80:BEC6; port request D)");
}

// $80:8A8C: the championship's loop (it never returns: its ways out are traps)
static void championship(void) {
  champ_screen(true);
  save_data_seal();
  player_in_top5();
  bool overflow = false;
  track_snap(&overflow);
  top_tallies();
  for(;;) {
    result_tally();
    shop_screen();
    result_slot_step();
    top_race_prep();
    race_wrapper();
    wait_03e0();
    sound_load_menu(DP, 1);
    top_after_race();
    ticker_screen();
    top_after_ticker();
    prize_money_award(false, &overflow);
    results_screen_run();
    title_screen_run(rd16(0x047B));
    track_next();
    if(top_after_title()) options_entry(1);
    champ_screen(false);
  }
}

void game_main(void) {
  game_boot();
  if(!(rd16(0x0459) & 0x8000)) tg3000_unported("$80:8A04: the top loop without the title card");
  options_entry(2);
  sound_load_set(DP, 0);
  top_volumes();
  intro_screen();
  for(;;) {  // $80:8A1F
    results_init();
    options_entry(0);
    save_data_seal();
    if(top_after_options()) {  // $80:8B79: the two-player game
      player_in_top5();
      top_race_prep();
      race_wrapper();
      tg3000_unported("$80:8B8E: after the two-player race");
    }
    championship();
  }
}
