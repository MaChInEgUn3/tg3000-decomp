// SPDX-License-Identifier: GPL-3.0-or-later
#include "boot.h"

#include <string.h>

#include "frame.h"
#include "misc.h"
#include "platform.h"
#include "sound.h"
#include "tg3000.h"
#include "video_dma.h"

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
