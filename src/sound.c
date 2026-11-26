// SPDX-License-Identifier: GPL-3.0-or-later
#include "sound.h"

#include "decompress.h"
#include "platform.h"
#include "tg3000.h"

// TG3000_ROUTINE($86:DFAE) sound_fade_step
uint8_t sound_fade_step(void) {
  uint16_t fade = rd16(SOUND_FADE);
  uint8_t v = (uint8_t)((uint8_t)(fade << 1) + rd8(SOUND_VOLUME));
  if((v & 0x80) != 0) {  // past the end: up to $7F, down to 0
    wr8(SOUND_FADE, 0);
    wr8(SOUND_FADE + 1, 0);
    v = (fade & 0x8000) ? 0x00 : 0x7F;
  }
  wr8(SOUND_VOLUME, v);
  return v;
}

// TG3000_ROUTINE($86:DFFB) sound_send_message
void sound_send_message(void) {
  uint8_t msg[16] = {
    rd8(0x045C), rd8(0x045D), rd8(0x045F), (uint8_t)((video_is_pal() ? 0x02 : 0) | rd8(SOUND_FLAGS) | 0x80),
    rd8(0x0461), rd8(0x045E), rd8(0x0460), (uint8_t)(rd8(0x0462) & 0x7F),
    rd8(0x0464), rd8(0x0467), rd8(0x0466), (uint8_t)(rd8(SOUND_REQUEST_A) | 0x80),
    rd8(0x0469), rd8(0x046C), rd8(0x046B), (uint8_t)(rd8(SOUND_REQUEST_B) & 0x7F),
  };
  sound_send_frame(msg);
  wr8(SOUND_REQUEST_A, 0);
  wr8(SOUND_REQUEST_B, 0);
  wr8(0x045F, 0xFF);
}

// TG3000_ROUTINE($86:E096) sound_boot_upload
// The block list is the game's; how each block reaches the chip (the console's start-up
// loader) is the platform's.
void sound_boot_upload(uint32_t src) {
  sound_boot_begin();
  for(;;) {
    uint16_t size = (uint16_t)(rd8(src) | rd8(src + 1) << 8);
    uint16_t dest = (uint16_t)(rd8(src + 2) | rd8(src + 3) << 8);
    src += 4;
    if(size == 0) {
      sound_boot_start(dest);
      return;
    }
    sound_boot_block(dest, src, size);
    src += size;
  }
}

// The protocol flags each step by toggling a byte the original keeps at $0479: two steps for a
// block's size and destination, one for each three bytes (or fewer, last).
static void sound_toggle_block(uint16_t size) {
  if(((2u + (size + 2u) / 3u) & 1u) != 0) wr8(0x0479, (uint8_t)(rd8(0x0479) ^ 0xFF));
}

// TG3000_ROUTINE($86:E0F0) sound_upload_list
// The list is the game's; the driver's protocol (its handshakes, the bytes in threes) is the
// platform's.
void sound_upload_list(uint32_t src) {
  sound_upload_begin();
  wr8(0x0479, 0);
  for(;;) {
    uint16_t size = (uint16_t)(rd8(src) | rd8(src + 1) << 8);
    if(size == 0) break;
    uint16_t dest = (uint16_t)(rd8(src + 2) | rd8(src + 3) << 8);
    sound_upload_block(dest, src + 4, size);
    sound_toggle_block(size);
    src += 4u + size;
  }
  sound_upload_end();
}

// TG3000_ROUTINE($86:E2B4) sound_upload_one
// One sample, as a list of one block: $0477 bytes at the long pointer D+$E0, to $0475.
static void sound_upload_one(uint16_t dp, uint8_t slot, uint16_t loop) {
  uint16_t size = rd16(0x0477);
  sound_upload_begin();
  wr8(0x0479, 0);
  if(size != 0) {
    uint32_t src = (uint32_t)(rd16(dp + 0xE0) | rd8(dp + 0xE2) << 16);
    sound_upload_sample(slot, rd16(0x0475), src, size, loop);
    sound_toggle_block(size);
  }
  sound_upload_end();
}

static void sound_wait_frame(void) {
  const uint16_t was = rd16(0x0024);
  while(rd16(0x0024) == was) platform_wait_vblank();
}

// TG3000_ROUTINE($86:DDEF) sound_load_set (long)
// The set table ($86:EFAB, by set) points into bank $86 at: the compressed blocks (word
// address, byte bank, a spare byte; until a 0 address), a 0 (a list of uncompressed blocks,
// never used), the directory's address, then the samples (start, end, bank word, slot; until
// a 0 start). Each sample starts with its loop offset (a word, not sent).
void sound_load_set(uint16_t dp, uint16_t set) {
  wr16(0x0471, set);
  wr16(SOUND_ENABLE, 0);
  wr8(SOUND_FLAGS, 1);
  uint16_t y = rom16(0x86EFAB + (uint16_t)(set << 1));
  wr16(0x0473, y);
  wr16(0x0461, 0x4000);
  while(rom16(0x860000 | y) != 0) {
    uint16_t end;
    decompress(dp, rom8(0x860000 | (uint16_t)(y + 2)), rom16(0x860000 | y), 0x7F, 0x0000, &end);
    wr16(dp + 0xE0, 0x0000);
    wr16(dp + 0xE2, 0x007F);
    sound_wait_frame();
    sound_send_message();
    sound_upload_list(0x7F0000);
    y = (uint16_t)(rd16(0x0473) + 4);
    wr16(0x0473, y);
  }
  y = (uint16_t)(y + 2);
  wr16(0x0473, y);
  if(rom16(0x860000 | y) != 0) tg3000_unported("sound_load_set: uncompressed blocks ($86:DE6F)");
  y = (uint16_t)(y + 2);
  const uint16_t dir = rom16(0x860000 | y);
  wr16(0x0461, dir);
  wr16(0x7F0002, dir);
  wr16(0x7F0000, 0);
  y = (uint16_t)(y + 2);
  uint16_t top = 0;  // the highest slot
  for(uint16_t p = y; rom16(0x860000 | p) != 0; p = (uint16_t)(p + 8)) {
    if(top < rom16(0x860000 | (uint16_t)(p + 6))) top = rom16(0x860000 | (uint16_t)(p + 6));
  }
  wr16(0x0475, (uint16_t)(((top + 1) << 2) + dir));  // the samples start after the directory
  for(;;) {
    const uint16_t start = rom16(0x860000 | y);
    if(start == 0) break;
    wr16(dp + 0xE0, start);
    wr16(0x0477, (uint16_t)(rom16(0x860000 | (uint16_t)(y + 2)) - start - 2));
    wr16(dp + 0xE2, rom16(0x860000 | (uint16_t)(y + 4)));
    const uint16_t slot = rom16(0x860000 | (uint16_t)(y + 6));
    const uint16_t x = (uint16_t)(slot << 2);
    wr16(0x7F0004 + x, rd16(0x0475));
    wr16(0x0473, (uint16_t)(y + 8));
    const uint16_t loop = (uint16_t)(bus_rd16((uint32_t)(start | rd8(dp + 0xE2) << 16)) + rd16(0x0475));
    wr16(0x7F0006 + x, loop);
    if(x >= rd16(0x7F0000)) wr16(0x7F0000, x);
    wr16(dp + 0xE0, (uint16_t)(start + 2));
    sound_wait_frame();
    sound_send_message();
    sound_upload_one(dp, (uint8_t)slot, loop);
    y = rd16(0x0473);
    wr16(0x0475, (uint16_t)(rd16(0x0477) + rd16(0x0475)));
  }
  const uint16_t n = (uint16_t)(rd16(0x7F0000) + 4);
  wr16(0x7F0000, n);
  wr16(0x7F0004 + n, 0);
  wr16(dp + 0xE0, 0x0000);
  wr16(dp + 0xE2, 0x007F);
  sound_wait_frame();
  sound_send_message();
  sound_upload_list(0x7F0000);
  while(!sound_is_ready()) {
  }
  static const uint16_t cleared[] = {SOUND_FLAGS, SOUND_REQUEST_A, 0x0464, SOUND_REQUEST_B, 0x0469, SOUND_VOLUME, 0x045F};
  for(unsigned i = 0; i < sizeof cleared / sizeof cleared[0]; i++) wr8(cleared[i], 0);
  wr8(0x045D, 0xFF);
  wr16(0x046F, 0);
  wr16(SOUND_ENABLE, 0xFFFF);
}

// TG3000_ROUTINE($86:DD85) sound_boot (long)
void sound_boot(uint16_t dp) {
  static const uint16_t cleared[] = {SOUND_VOLUME, 0x045D, 0x045F, SOUND_REQUEST_A, 0x0464, SOUND_REQUEST_B, 0x0469, SOUND_FLAGS};
  wr16(SOUND_ENABLE, 0);
  for(unsigned i = 0; i < sizeof cleared / sizeof cleared[0]; i++) wr8(cleared[i], 0);
  uint16_t end;
  decompress(dp, 0x86, 0xE478, 0x7F, 0x0000, &end);
  wr16(dp + 0xE0, 0x0000);
  wr16(dp + 0xE2, 0x007F);
  sound_boot_upload(0x7F0000);
  sound_clear_ports();
  while(!sound_is_ready()) {
  }
  wr16(SOUND_ENABLE, 0xFFFF);
}

// TG3000_ROUTINE($86:DFE1) sound_frame (long)
bool sound_frame(void) {
  sound_fade_step();
  if(!(rd16(SOUND_ENABLE) & 0x8000) || !sound_is_ready()) return false;
  sound_send_message();
  return true;
}
