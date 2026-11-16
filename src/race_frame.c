// SPDX-License-Identifier: GPL-3.0-or-later
#include "race_frame.h"

#include "platform.h"
#include "tg3000.h"

// TG3000_ROUTINE($83:8644) race_upload_queue_drain
//
// Sends queued transfers while the frame's DMA budget lasts. Each whole transfer costs its
// size plus 100. A transfer larger than what is left sends that much (the budget), the
// entry keeps the rest for the next frame (its size shrinks, source and VRAM address
// advance), and the budget becomes 0. Returns whether it wrote a flag word (the original
// then leaves its data bank at $83; the C has no data bank).
bool race_upload_queue_drain(void) {
  bool flagged = false;
  int16_t budget = (int16_t)rd16(RACE_DMA_BUDGET);
  uint16_t x = rd16(RACE_QUEUE_TAIL);
  if(budget <= 0 || x == rd16(RACE_QUEUE_HEAD)) return flagged;
  do {
    if(!(rd16(RACE_QUEUE_BANK + x) & 0x8000)) {
      budget = (int16_t)rd16(RACE_DMA_BUDGET);
      if(budget <= 0) break;
      uint16_t size = rd16(RACE_QUEUE_SIZE + x);
      if((int16_t)(budget - size) < 0) {  // only part of it fits
        uint16_t part = (uint16_t)budget;
        uint16_t src = rd16(RACE_QUEUE_SRC + x), dest = rd16(RACE_QUEUE_DEST + x);
        wr16(0x003C, (uint16_t)(part >> 1));  // the words it sends (scratch)
        wr16(RACE_QUEUE_SIZE + x, (uint16_t)(size - part));
        wr16(RACE_QUEUE_SRC + x, (uint16_t)(src + part));
        wr16(RACE_QUEUE_DEST + x, (uint16_t)(dest + (part >> 1)));
        video_upload_vram(dest, (uint32_t)rd8(RACE_QUEUE_BANK + x) << 16 | src, part ? part : 0x10000);
        wr16(RACE_DMA_BUDGET, 0);
        wr16(RACE_QUEUE_TAIL, x);
        return flagged;
      }
      wr16(RACE_DMA_BUDGET, (uint16_t)(budget - size - 100));
      video_upload_vram(rd16(RACE_QUEUE_DEST + x), (uint32_t)rd8(RACE_QUEUE_BANK + x) << 16 | rd16(RACE_QUEUE_SRC + x),
                        size ? size : 0x10000);
      uint16_t flag = rd16(RACE_QUEUE_FLAG_ADR + x);
      if(flag) {
        wr16(flag, rd16(RACE_QUEUE_FLAG_VAL + x));  // DB = $7E
        flagged = true;
      }
    }
    x = (uint16_t)((x + 2) & 0xFF);
  } while(x != rd16(RACE_QUEUE_HEAD));
  wr16(RACE_QUEUE_TAIL, x);
  return flagged;
}

// TG3000_ROUTINE($83:96D1) race_upload_queue_drain_long (long)
bool race_upload_queue_drain_long(void) { return race_upload_queue_drain(); }

// TG3000_ROUTINE($83:8989) func_838989
//
// Unless the game is paused ($0026 bit 4), adds the signed word $01CC to the 24-bit value
// at $00D4. Then writes its upper word + $3C to the two words at $1536 and $1556 (indexed
// by $00E2); when $1F32 is negative, the first gets that word less an eighth instead.
void func_838989(void) {
  uint16_t x = rd16(0x00E2);
  if(!(rd16(0x0026) & 0x0010)) {
    uint16_t step = rd16(0x01CC);
    uint32_t sum = (uint32_t)rd16(0x00D4) + step;
    wr16(0x00D4, (uint16_t)sum);
    wr8(0x00D6, (uint8_t)(rd8(0x00D6) + ((step & 0x8000) ? 0xFF : 0x00) + (sum >> 16)));
  }
  uint16_t pos = rd16(0x00D5);
  wr16((uint16_t)(0x1536 + x), (uint16_t)(pos + 0x3C));
  wr16((uint16_t)(0x1556 + x), (uint16_t)(pos + 0x3C));
  if(rd16(0x1F32) & 0x8000) wr16((uint16_t)(0x1536 + x), (uint16_t)(pos - (pos >> 3)));
}

// TG3000_ROUTINE($83:96D5) func_838989_long (long)
void func_838989_long(void) { func_838989(); }

// TG3000_ROUTINE($85:9147) race_cycle_blocks
//
// Five groups of 64 bytes at $7E:2000-$7E:213F, each eight blocks of 8 bytes (four
// words). Every block becomes four copies of the next block's first word; the last block
// gets the first block's. The race NMI runs it every fourth frame.
void race_cycle_blocks(void) {
  for(uint32_t group = 0x2000; group < 0x2140; group += 0x40) {
    uint16_t first = rd16(group);
    for(uint32_t k = 0; k < 8; k++) {
      uint16_t next = k < 7 ? rd16(group + 8 * (k + 1)) : first;
      for(uint32_t w = 0; w < 8; w += 2) wr16(group + 8 * k + w, next);
    }
  }
}

// The race HUD's tilemap words, written one after another from an address.
static uint16_t hud_at, hud_step;
static void hud_from(uint16_t vram_word) { hud_at = vram_word; }
static void hud_put(uint16_t tile) {
  video_write_vram_word(hud_at, tile);
  hud_at = (uint16_t)(hud_at + hud_step);
}

// TG3000_ROUTINE($85:8814) race_hud_draw (long)
//
// The race NMI runs it when the frame's DMA budget allows: the HUD's tilemap words in
// VRAM. A 7-word bar from $003E (two units per word, the half word at the end), the
// words in $0042-$004C and $1E94-$1E98, the BCD clock $5A-$5C as digit tiles, the
// $0235 counter (capped at $1F06), a column from $006C-$0074, the rows $0080-$008E and
// $00A4-$00AA, the gauge from $025B/$025C, five lights with the one at $014C lit, and two
// 26-byte strips from $7E:4E61 (or, when the $7E:4E4E accumulator goes negative,
// $83:A5A4). Tables in bank $83 ($A422.., the digits at $A484 and $A498) and bank $85.
void race_hud_draw(void) {
  hud_step = 1;
  hud_from(0x7E0F);
  hud_put(rd16(0x1E94));
  hud_from(0x7C62);
  uint16_t bar = rd16(0x003E), y = 0;
  for(uint16_t x = bar & 0xFFFE; x; x = (uint16_t)(x - 2)) {
    hud_put(rom16(0x83A422 + y));
    y = (uint16_t)(y + 2);
  }
  if(bar & 1) {
    hud_put(rom16(0x83A430 + y));
    y = (uint16_t)(y + 2);
  }
  for(; y != 0x000E; y = (uint16_t)(y + 2)) hud_put(rom16(0x83A43E));
  hud_from(0x7F21);
  hud_put(rd16(0x1E96));
  hud_put(rd16(0x0042));
  hud_put(rd16(0x0046));
  hud_put(rd16(0x004A));
  hud_from(0x7F41);
  hud_put(rd16(0x1E98));
  hud_put(rd16(0x0044));
  hud_put(rd16(0x0048));
  hud_put(rd16(0x004C));
  // the clock's digits: $5C low digit, $5B both, $5A both
  hud_from(0x7C4D);
  hud_put(rom16(0x83A498 + (uint32_t)(rd8(0x005C) & 0x0F) * 2));
  hud_from(0x7C4F);
  hud_put(rom16(0x83A484 + (uint32_t)((rd8(0x005B) & 0xF0) >> 3)));
  hud_put(rom16(0x83A498 + (uint32_t)(rd8(0x005B) & 0x0F) * 2));
  hud_from(0x7C52);
  hud_put(rom16(0x83A484 + (uint32_t)((rd8(0x005A) & 0xF0) >> 3)));
  hud_put(rom16(0x83A484 + (uint32_t)(rd8(0x005A) & 0x0F) * 2));
  hud_from(0x7C22);
  hud_put(rd16(0x0064));
  hud_put(rd16(0x0066));
  hud_from(0x7C3A);
  uint16_t count = rd16(0x0235);
  if(count & 0x8000) count = 0;
  count = (uint16_t)(count + 1);
  if(!((uint16_t)(count - rd16(0x1F06)) & 0x8000)) count = rd16(0x1F06);
  hud_put(rom16(0x83A484 + (uint16_t)(count << 1)));
  video_set_vram_increment(0x81);  // a column
  hud_step = 32;
  hud_from(0x7EDE);
  for(uint16_t a = 0x006C; a <= 0x0074; a += 2) hud_put(rd16(a));
  video_set_vram_increment(0x80);
  hud_step = 1;
  hud_from(0x5AE0);
  for(uint16_t a = 0x0080; a <= 0x008E; a += 2) hud_put(rd16(a));
  hud_from(0x7F3C);
  hud_put(rd16(0x00A4));
  hud_put(rd16(0x00A6));
  hud_from(0x7F5C);
  hud_put(rd16(0x00A8));
  hud_put(rd16(0x00AA));
  uint16_t gauge = rd16(0x025B);  // its high byte is $025C, the count below
  if(gauge & 0x8000) {
    uint32_t x = (uint32_t)((gauge & 0x7E00) >> 8);
    hud_from(0x7F3B);
    hud_put(rom16(0x858AC1 + x));
    hud_from(0x7F57);
    for(int i = 0; i < 4; i++) hud_put(rom16(0x858ABB));
    hud_put(rom16(0x858AD9 + x));
  } else {
    hud_from(0x7F3B);
    hud_put(rom16(0x858ABB));
    hud_from(0x7F57);
    uint16_t k = (uint16_t)(0x000A - rd16(0x025C));
    while(k) {  // k / 2 full segments, then a half one when k is odd
      if(--k == 0) {
        hud_put(rom16(0x858ABD));
        break;
      }
      hud_put(rom16(0x858ABB));
      k--;
    }
    for(int16_t e = (int16_t)(rd16(0x025C) - 2); e >= 0; e = (int16_t)(e - 2)) hud_put(rom16(0x858ABF));
  }
  hud_from(0x7C76);  // five lights, the one at $014C lit
  uint16_t lit = (uint16_t)(rd16(0x014C) + 1);
  for(int i = 0; i < 5; i++) {
    lit = (uint16_t)(lit - 1);
    hud_put(lit == 0 ? 0x295E : 0x0157);
  }
  uint8_t acc = (uint8_t)(rd8(0x4E4D) + rd8(0x4E4E));
  wr8(0x4E4E, acc);
  uint32_t strip = (acc & 0x80) ? 0x83A5A4 : 0x7E4E61;
  video_upload_vram(0x7DE9, strip, 0x1A);
  video_upload_vram(0x7E09, strip + 0x1A, 0x1A);
}
