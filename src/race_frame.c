// SPDX-License-Identifier: GPL-3.0-or-later
#include <stdio.h>
#include "race_frame.h"

#include "frame.h"
#include "platform.h"
#include "tg3000.h"

// HDMA slot 4's table in the VS race, set each frame by nmi_vs_full; the split saves its address
// at $7F:FFFE (port request H: the original reads it back from $4342, which HDMA never moves).
#define VS_SLOT4_TABLE 0x83A411u

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

// The 24-bit value at `at` plus the signed word `step` (the original's 16-bit ADC, then its
// carry and the step's sign into the high byte).
static void add24_signed(uint16_t at, uint16_t step) {
  const uint32_t sum = (uint32_t)rd16(at) + step;
  wr16(at, (uint16_t)sum);
  wr8((uint16_t)(at + 2), (uint8_t)(rd8((uint16_t)(at + 2)) + ((step & 0x8000) ? 0xFF : 0x00) + (sum >> 16)));
}

// TG3000_ROUTINE($83:886A) vs_view_scrolls
//
// func_838989 for the VS race's split screen: unless the game is paused ($0026 bit 4), four
// 24-bit values move by their signed steps ($00D4 by $01CC, $00D7 by $01D4, $00DA by $01D0,
// $00DD by $01D8). Then their upper words go to the views' words, indexed by $00E2: the
// first two + $3C at $1536 and $1546, the last two - $4C at $1556 and $1566.
void vs_view_scrolls(void) {
  const uint16_t x = rd16(0x00E2);
  if(!(rd16(0x0026) & 0x0010)) {
    add24_signed(0x00D4, rd16(0x01CC));
    add24_signed(0x00D7, rd16(0x01D4));
    add24_signed(0x00DA, rd16(0x01D0));
    add24_signed(0x00DD, rd16(0x01D8));
  }
  wr16((uint16_t)(0x1536 + x), (uint16_t)(rd16(0x00D5) + 0x3C));
  wr16((uint16_t)(0x1546 + x), (uint16_t)(rd16(0x00D8) + 0x3C));
  wr16((uint16_t)(0x1556 + x), (uint16_t)(rd16(0x00DB) - 0x4C));
  wr16((uint16_t)(0x1566 + x), (uint16_t)(rd16(0x00DE) - 0x4C));
}

// TG3000_ROUTINE($83:8909) mode1_view_scrolls
//
// vs_view_scrolls for the race wrapper's mode 1, the two-view race: unless the game is paused
// ($0026 bit 4), two 24-bit values move by their signed steps ($00D4 by $01CC, $00D7 by $01D0).
// Their upper words + $3C go to the views' words, indexed by $00E2: the first to $1536 and
// $1556, the second to $1546 and $1566. ($1F32 negative takes a block that never ran: a trap.)
void mode1_view_scrolls(void) {
  const uint16_t x = rd16(0x00E2);
  if(!(rd16(0x0026) & 0x0010)) {
    add24_signed(0x00D4, rd16(0x01CC));
    add24_signed(0x00D7, rd16(0x01D0));
  }
  const uint16_t first = (uint16_t)(rd16(0x00D5) + 0x3C), second = (uint16_t)(rd16(0x00D8) + 0x3C);
  wr16((uint16_t)(0x1536 + x), first);
  wr16((uint16_t)(0x1556 + x), first);
  wr16((uint16_t)(0x1546 + x), second);
  wr16((uint16_t)(0x1566 + x), second);
  if(rd16(0x1F32) & 0x8000) tg3000_unported("$83:8968: mode1_view_scrolls with $1F32 negative");
}

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

// map-fw-18: mode 1's HUD ($85:95C7), drafted by a delegated agent from the listing and race_hud_draw;
// checked by the differential test
// (draft, map-fw: mode 1's HUD; to append to src/race_frame.c after vs_hud_draw. It uses
// race_frame.c's hud_from/hud_put/hud_step.)

// Mode 1's HUD for one player (race_hud_draw's pieces at that player's VRAM words, from that
// player's variables). `record` is the player's record ($0235, $02E8: the count at +0, the gauge
// word at +$26, its count byte at +$27). `lead` (0: none) is the word in front of each of the
// two rows of three words (player 1's $1E96/$1E98). The traps name the original's never-run
// spans (incbin in the listing).
struct mode1_hud_player {
  uint16_t barAt, bar;         // the 7-word bar (two units a word, a half word at the end)
  uint16_t rowsAt, lead, rows; // two rows of 3 words: rows, +4, +8 at rowsAt; +2, +6, +$A at rowsAt + $20
  uint16_t clockAt, clock;     // the BCD clock: clock+2 low digit, clock+1 both, clock both
  uint16_t pairAt, pair;       // two words
  uint16_t countAt, record;    // the record's count + 1 as a digit (capped at $1F06: never run)
  uint16_t columnAt, column;   // a tilemap column of 5 words
  uint16_t lineAt, line;       // 8 words
  uint16_t pairsAt, pairs;     // 2 words, 2 more a tilemap row ($20 words) below
  uint16_t lightsAt, lit;      // five lights, the one at `lit` lit
  uint16_t gaugeAt;            // the gauge: one word, then from gaugeAt + $1C
  uint16_t acc, strip, stripAt;  // the strips: accumulator pair, $7E source, VRAM (and + $20)
  const char *capTrap, *lightTrap, *gaugeTrap, *halfTrap;
};

static void mode1_hud_player_draw(const struct mode1_hud_player* p) {
  // the bar: as race_hud_draw's
  hud_from(p->barAt);
  uint16_t bar = rd16(p->bar), y = 0;
  for(uint16_t x = bar & 0xFFFE; x; x = (uint16_t)(x - 2)) {
    hud_put(rom16(0x83A422 + y));
    y = (uint16_t)(y + 2);
  }
  if(bar & 1) {
    hud_put(rom16(0x83A430 + y));
    y = (uint16_t)(y + 2);
  }
  for(; y != 0x000E; y = (uint16_t)(y + 2)) hud_put(rom16(0x83A43E));
  for(uint16_t r = 0; r < 2; r++) {  // the two rows
    hud_from((uint16_t)(p->rowsAt + 0x20 * r));
    if(p->lead) hud_put(rd16((uint16_t)(p->lead + 2 * r)));
    for(uint16_t k = 0; k < 3; k++) hud_put(rd16((uint16_t)(p->rows + 2 * r + 4 * k)));
  }
  // the clock's digits
  const uint16_t c = p->clock;
  hud_from(p->clockAt);
  hud_put(rom16(0x83A498 + (uint32_t)(rd8((uint16_t)(c + 2)) & 0x0F) * 2));
  hud_from((uint16_t)(p->clockAt + 2));
  hud_put(rom16(0x83A484 + (uint32_t)((rd8((uint16_t)(c + 1)) & 0xF0) >> 3)));
  hud_put(rom16(0x83A498 + (uint32_t)(rd8((uint16_t)(c + 1)) & 0x0F) * 2));
  hud_from((uint16_t)(p->clockAt + 5));
  hud_put(rom16(0x83A484 + (uint32_t)((rd8(c) & 0xF0) >> 3)));
  hud_put(rom16(0x83A484 + (uint32_t)(rd8(c) & 0x0F) * 2));
  hud_from(p->pairAt);
  hud_put(rd16(p->pair));
  hud_put(rd16((uint16_t)(p->pair + 2)));
  hud_from(p->countAt);
  uint16_t count = rd16(p->record);
  if(count & 0x8000) count = 0;
  count = (uint16_t)(count + 1);
  if(!((uint16_t)(count - rd16(0x1F06)) & 0x8000)) tg3000_unported(p->capTrap);
  hud_put(rom16(0x83A484 + (uint16_t)(count << 1)));
  video_set_vram_increment(0x81);  // a column
  hud_step = 32;
  hud_from(p->columnAt);
  for(uint16_t k = 0; k < 10; k += 2) hud_put(rd16((uint16_t)(p->column + k)));
  video_set_vram_increment(0x80);
  hud_step = 1;
  hud_from(p->lineAt);
  for(uint16_t k = 0; k < 16; k += 2) hud_put(rd16((uint16_t)(p->line + k)));
  hud_from(p->pairsAt);
  hud_put(rd16(p->pairs));
  hud_put(rd16((uint16_t)(p->pairs + 2)));
  hud_from((uint16_t)(p->pairsAt + 0x20));
  hud_put(rd16((uint16_t)(p->pairs + 4)));
  hud_put(rd16((uint16_t)(p->pairs + 6)));
  // five lights (before the gauge here, after it in race_hud_draw): only the first lit ran
  hud_from(p->lightsAt);
  uint16_t lit = (uint16_t)(rd16(p->lit) + 1);
  for(int i = 0; i < 5; i++) {
    lit = (uint16_t)(lit - 1);
    if(lit == 0 && i) tg3000_unported(p->lightTrap);
    hud_put(lit == 0 ? 0x295E : 0x0157);
  }
  // the gauge: only race_hud_draw's second form ran, and only with an even count of segments
  const uint16_t gauge = (uint16_t)(p->record + 0x26), segs = (uint16_t)(gauge + 1);
  if(rd16(gauge) & 0x8000) tg3000_unported(p->gaugeTrap);
  hud_from(p->gaugeAt);
  hud_put(rom16(0x858ABB));
  hud_from((uint16_t)(p->gaugeAt + 0x1C));
  uint16_t k = (uint16_t)(0x000A - rd16(segs));
  while(k) {  // k / 2 full segments (a half one when k is odd: never run)
    if(--k == 0) tg3000_unported(p->halfTrap);
    hud_put(rom16(0x858ABB));
    k--;
  }
  for(int16_t e = (int16_t)(rd16(segs) - 2); e >= 0; e = (int16_t)(e - 2)) hud_put(rom16(0x858ABF));
  // the two 26-byte strips (DMA channel 7, as race_hud_draw's)
  uint8_t acc = (uint8_t)(rd8(p->acc) + rd8((uint16_t)(p->acc + 1)));
  wr8((uint16_t)(p->acc + 1), acc);
  uint32_t strip = (acc & 0x80) ? 0x83A5A4 : 0x7E0000u | p->strip;
  video_upload_vram(p->stripAt, strip, 0x1A);
  video_upload_vram((uint16_t)(p->stripAt + 0x20), strip + 0x1A, 0x1A);
}

// TG3000_ROUTINE($85:95C7) mode1_hud_draw
//
// race_hud_draw for mode 1, the two-view race, run by its NMI when the frame's DMA budget allows
// ($85:8E9D, on $00B8 bit 2): the word at $1E94, then each player's HUD (mode1_hud_player_draw):
// the first player's at race_hud_draw's places in the top view (its rows, column, words and
// gauge moved up, its strips at $7CA9), the second's in the bottom view from the second set of
// variables ($40, $4E-$58, the clock $5D-$5F, $68, the record at $02E8, $76-$7E, $90-$9E,
// $AC-$B2, $014E, the strip at $7E:4E95 with its accumulator $7E:4E4F/$4E50). Tables in bank $83
// ($A422.., the digits at $A484 and $A498) and bank $85 ($8ABB-$8ABF). Paths that never ran
// (the counts' cap, lights 2-5, the gauge's first form and its half segment) are traps.
void mode1_hud_draw(void) {
  static const struct mode1_hud_player p[2] = {
    {0x7C62, 0x003E, 0x7D61, 0x1E96, 0x0042, 0x7C4D, 0x005A, 0x7C22, 0x0064, 0x7C3A, 0x0235, 0x7D1E, 0x006C,
     0x5AE0, 0x0080, 0x7D7C, 0x00A4, 0x7C76, 0x014C, 0x7D7B, 0x4E4D, 0x4E61, 0x7CA9,
     "$85:96C8: mode 1's HUD: player 1's count + 1 reaches $1F06 (the cap)",
     "$85:9771: mode 1's HUD: player 1's lit light is 2-5 ($014C 1-4)",
     "$85:979A: mode 1's HUD: player 1's gauge word $025B negative (the other gauge)",
     "$85:97F7: mode 1's HUD: player 1's gauge with an odd count of segments (the half one)"},
    {0x7E22, 0x0040, 0x7F22, 0x0000, 0x004E, 0x7E0D, 0x005D, 0x7DE2, 0x0068, 0x7DFA, 0x02E8, 0x7EDE, 0x0076,
     0x5AE8, 0x0090, 0x7F3C, 0x00AC, 0x7E36, 0x014E, 0x7F3B, 0x4E4F, 0x4E95, 0x7E69,
     "$85:9942: mode 1's HUD: player 2's count + 1 reaches $1F06 (the cap)",
     "$85:99EB: mode 1's HUD: player 2's lit light is 2-5 ($014E 1-4)",
     "$85:9A14: mode 1's HUD: player 2's gauge word $030E negative (the other gauge)",
     "$85:9A71: mode 1's HUD: player 2's gauge with an odd count of segments (the half one)"},
  };
  hud_step = 1;
  hud_from(0x7E0F);  // (player 2's clock writes $7E0F again later in the same call)
  hud_put(rd16(0x1E94));
  for(int i = 0; i < 2; i++) mode1_hud_player_draw(&p[i]);
}

/*
 * Notes for the port (mode1_hud_draw, $85:95C7-$85:9ADA)
 *
 * Player 1 ($95C7-$985D) against race_hud_draw ($8814-$8AAD), paired instruction by instruction:
 *   - $7E0F <- $1E94, the bar at $7C62 from $3E ($A422/$A430/$A43E, to y = $0E): same.
 *   - row 1 at $2116: $7F21 -> $7D61 ($1E96, $42, $46, $4A); row 2: $7F41 -> $7D81 ($1E98, $44, $48, $4C).
 *   - the clock $7C4D/$7C4F/$7C52 from $5C/$5B/$5A: same addresses, same tables.
 *   - $7C22 <- $64, $66: same.  $7C3A <- the $0235 count: same, except the cap: `cmp $1F06; bmi`
 *     not taken lands on incbin $85:96C8-$96CA (3 bytes, where race_hud_draw has `lda $1F06`):
 *     a trap, behind the branch.
 *   - the column (VMAIN $81): $7EDE -> $7D1E ($6C-$74).  $5AE0 <- $80-$8E: same.
 *   - $7F3C -> $7D7C ($A4, $A6); $7F5C -> $7D9C ($A8, $AA).
 *   - ORDER: the five lights ($7C76, $014C: same address) come BEFORE the gauge in mode 1 (race
 *     draws the gauge first). Only the first light's `lda #$295E` ($85:9767) is code; lights 2-5's
 *     are incbin ($85:9771, $977B, $9785, $978F; 3 bytes each): a trap when $014C is 1-4 (each
 *     sits behind its own `bne`; one trap message for all four).
 *   - the gauge: `lda $025B; bpl` not taken -> incbin $85:979A-$97CA (race's $89A8-$89D8 form,
 *     49 bytes = race's 49): a trap. The second form: $7F3B -> $7D7B, $7F57 -> $7D97 (= +$1C), same
 *     $858ABB/$858ABF, k = $0A - $025C. Inside the loop the `dey; beq` is `db $F0,$08` at $85:97ED
 *     (decoded as data: never taken) whose target $85:97F7-$97FD is incbin (7 bytes = race's
 *     `lda.l $858ABD; sta $2118`): a trap where k reaches 0 on the first `dey` (k odd).
 *   - the strips: same accumulator ($7E:4E4D + $7E:4E4E -> $4E4E), same sources ($7E:4E61, or
 *     $83:A5A4 when negative), same channel 7 setup; VRAM $7DE9 -> $7CA9 and $7E09 -> $7CC9.
 *   - the end: race_hud_draw has `rep #$20; plb; rtl`; mode 1 has `rep #$20` ($85:985D) and goes on.
 * Player 2 ($985F-$9AD7), the same sequence with (player 1 -> player 2):
 *   bar $7C62/$3E -> $7E22/$40 (same tables); rows $7D61/$7D81 with the lead words $1E96/$1E98 ->
 *   $7F22/$7F42 with NO lead word ($4E, $52, $56 / $50, $54, $58); clock $7C4D/$7C4F/$7C52 from
 *   $5C/$5B/$5A -> $7E0D/$7E0F/$7E12 from $5F/$5E/$5D; pair $7C22/$64,$66 -> $7DE2/$68,$6A; count
 *   $7C3A/$0235 -> $7DFA/$02E8 (cap incbin $85:9942: trap); column $7D1E/$6C-$74 -> $7EDE/$76-$7E;
 *   line $5AE0/$80-$8E -> $5AE8/$90-$9E; pairs $7D7C,$7D9C/$A4-$AA -> $7F3C,$7F5C/$AC-$B2; lights
 *   $7C76/$014C -> $7E36/$014E (lights 2-5 incbin $85:99EB, $99F5, $99FF, $9A09: trap); gauge
 *   $025B/$025C -> $030E/$030F (= record + $26/$27, records $B3 apart), first form incbin
 *   $85:9A14-$9A44 (trap), $7D7B/$7D97 -> $7F3B/$7F57 (race_hud_draw's own), the loop's `beq`
 *   `db $F0,$08` at $85:9A67 to incbin $85:9A71-$9A77 (trap); strips $7E:4E61, acc $4E4D/$4E4E,
 *   $7CA9/$7CC9 -> $7E:4E95, acc $4E4F/$4E50, $7E69/$7E89. Ends `rep #$20; plb; rts`.
 * No reuse from the VS HUD: vs_hud_draw's bars are 6 words from other tables ($83:9F2E..), its
 * counts are rows of marks; only hud_from/hud_put/hud_step are shared. No incbin replaces code
 * on an always-run path: every incbin above sits behind a branch.
 *
 * Unsure:
 *   - $7E0F is written twice per call (the $1E94 word first, player 2's clock tens later); the C
 *     keeps both writes in order (the bench's VRAM effect log should see both).
 *   - The light trap fires before the lit word is written (the original would have loaded
 *     $295E there); the other traps fire at their branch before any further write, as asked.
 *   - The lights' X is 16-bit (`ldx $014C`): $014C >= 5 (or $FFFF) lights none, as in race.
 *   - The gauge loop with $025C/$030F above 10 (k "negative") would write ~32K words in the
 *     original (even k) or trap at the end (odd k); the fuzz should keep them 0-10, even.
 *
 * Bench (tools/bench/routines.c), mirroring race_hud_draw's:
 *   // ---- mode1_hud_draw ($85:95C7) ----  no register outputs (mode 1's NMI loads A next, $85:8EA0)
 *   static void call_mode1_hud_draw(const Regs* in, Regs* out) { (void)in; (void)out; mode1_hud_draw(); }
 *   At the RTS: M = 16 (`rep #$20`), X = 16 (never changed), DB restored by `plb` (the caller's,
 *   N/Z from it), A/X/Y leftovers (X = $7E89) that the caller overwrites: nothing to return.
 *   Row: {.name = "mode1_hud_draw", .entry = 0x8595C7, .end = 0x859ADB, .source = "src/race_frame.c",
 *         .needM16 = true, .needX16 = true, .call = call_mode1_hud_draw, .fuzz = fuzz_mode1_hud_draw,
 *         .holes = {{0x8596C8, 0x8596CB}, {0x859771, 0x859774}, {0x85977B, 0x85977E}, {0x859785, 0x859788},
 *                   {0x85978F, 0x859792}, {0x85979A, 0x8597CB}, {0x8597F7, 0x8597FE}, {0x859942, 0x859945},
 *                   {0x8599EB, 0x8599EE}, {0x8599F5, 0x8599F8}, {0x8599FF, 0x859A02}, {0x859A09, 0x859A0C},
 *                   {0x859A14, 0x859A45}, {0x859A71, 0x859A78}}},   // 14 never-run spans (traps)
 *   (no .rtl: it ends with RTS; race_hud_draw's row has no .db/.lowWramDb either: its `pea $8383`
 *   sets DB itself.) A negative row as race's (lights one off) would hit the light trap instead;
 *   use e.g. the $1E96 word XOR 1 around the call (wr16 before, restore after, as race's
 *   wrong row does with $014C); not a bar + 1 (a bar of 15 loops for ever in both).
 *   Fuzz, kept off the traps: $3E/$40 0-14; $014C/$014E 0 or 5-$FFFF (not 1-4); $025C and
 *   $030F as words (wr16ram, high byte $025D/$0310 = 0), even 0-10 ($025B/$030E then never
 *   negative; their low bytes random); $0235/$02E8
 *   negative or below $1F06 - 1 with $1F06 2-11; $1E94-$1E98, $0042-$00B2 random (the clock
 *   $5A-$5F any byte: only table lookups); $7E:4E4D-$4E50 random.
 */

// A VS HUD bar: 6 words at `vram`: bar / 2 full segments ($83:9F2E + y), a half one when the
// bar is odd ($83:9F3A + y), then empty ones ($83:9F46 + y). (The original runs to y = 12
// exactly: bars 0-12.)
static void vs_hud_bar(uint16_t vram, uint16_t bar) {
  hud_from(vram);
  uint16_t y = 0;
  for(uint16_t x = (uint16_t)(bar & 0xFFFE); x; x = (uint16_t)(x - 2)) {
    hud_put(rom16(0x839F2E + y));
    y = (uint16_t)(y + 2);
  }
  if(bar & 1) {
    hud_put(rom16(0x839F3A + y));
    y = (uint16_t)(y + 2);
  }
  for(; y != 0x000C; y = (uint16_t)(y + 2)) hud_put(rom16(0x839F46 + y));
}

// A VS HUD count: 6 words at `vram`: n of $83:9F68, then $83:9F52 to the sixth (n 0-5; the
// original's last loop runs at least once and to 6 exactly).
static void vs_hud_count(uint16_t vram, uint8_t n) {
  hud_from(vram);
  uint16_t y = n;
  for(uint16_t i = 0; i < n; i++) hud_put(rom16(0x839F68));
  do {
    hud_put(rom16(0x839F52));
    y = (uint16_t)(y + 1);
  } while(y != 6);
}

// TG3000_ROUTINE($83:9B49) vs_hud_draw
//
// (drafted by a delegated agent; its never-run spans decoded here.) race_hud_draw for the VS
// split screen's four players, run by its interrupt handler (D = 0, DB = $83). Each player's:
// a 6-word bar from its word ($3E, $40, $64, $68), three words ($42-$46, $4E-$52, $48-$4C,
// $54-$58), a 6-word count from the low byte of $024E, $0301, $02B0, $0363, one tilemap word
// read back and rewritten ORed with $6000 or $0006 (at the VRAM address in $A4, $AC, $A6, $AE:
// the player's place on the race's progress strip), and two 26-byte strips from
// $7E:4E61, $4E95, $4EC9, $4EFD, or $83:A3DD when its accumulator pair ($7E:4E4D/$4E4E, ...)
// adds up negative.
// (map-fw-3: from $57A0, not $57C0: mode 1 reads its strip from $57BF)
#define VS_STRIP_FIRST 0x57A0
#define VS_STRIP_WORDS 0xE0
static uint16_t vs_strip[VS_STRIP_WORDS];

void vs_strip_load(uint32_t src) {
  for(uint16_t i = 0; i < VS_STRIP_WORDS; i++) vs_strip[i] = bus_rd16(src + 2u * i);
}

uint16_t vs_strip_word(uint16_t vram_word) {
  const uint16_t i = (uint16_t)(vram_word - VS_STRIP_FIRST);
  if(i >= VS_STRIP_WORDS) {
    static char what[96];
    snprintf(what, sizeof what, "vs_hud_draw: VRAM word $%04X, outside the progress strip ($57A0-$587F)", vram_word);
    tg3000_unported(what);
  }
  return vs_strip[i];
}

void vs_strip_set(uint16_t vram_word, uint16_t value) {
  const uint16_t i = (uint16_t)(vram_word - VS_STRIP_FIRST);
  if(i < VS_STRIP_WORDS) vs_strip[i] = value;
}

void vs_hud_draw(void) {
  static const struct {
    uint16_t bar, barAt, wordsAt, words[3], countAt, count, pick, bits, acc, strip, at1, at2;
  } p[4] = {
    {0x003E, 0x7C21, 0x7D81, {0x0042, 0x0044, 0x0046}, 0x7C28, 0x024E, 0x00A4, 0x6000, 0x4E4D, 0x4E61, 0x7CA1, 0x7CC1},
    {0x0040, 0x7C32, 0x7D92, {0x004E, 0x0050, 0x0052}, 0x7C39, 0x0301, 0x00AC, 0x0006, 0x4E4F, 0x4E95, 0x7CB2, 0x7CD2},
    {0x0064, 0x7DE1, 0x7F41, {0x0048, 0x004A, 0x004C}, 0x7DE8, 0x02B0, 0x00A6, 0x6000, 0x4E51, 0x4EC9, 0x7E61, 0x7E81},
    {0x0068, 0x7DF2, 0x7F52, {0x0054, 0x0056, 0x0058}, 0x7DF9, 0x0363, 0x00AE, 0x0006, 0x4E53, 0x4EFD, 0x7E72, 0x7E92},
  };
  hud_step = 1;
  for(int i = 0; i < 4; i++) {
    vs_hud_bar(p[i].barAt, rd16(p[i].bar));
    hud_from(p[i].wordsAt);
    for(int k = 0; k < 3; k++) hud_put(rd16(p[i].words[k]));
    vs_hud_count(p[i].countAt, rd8(p[i].count));
    const uint16_t at = rd16(p[i].pick);
    // (the original reads the word at `at` back from VRAM, VMADD set and a dead read first; the
    // C has its own copy)
    const uint16_t word = (uint16_t)(vs_strip_word(at) | p[i].bits);
    video_write_vram_word(at, word);
    vs_strip_set(at, word);
    const uint8_t acc = (uint8_t)(rd8(p[i].acc) + rd8((uint32_t)p[i].acc + 1));
    wr8((uint32_t)p[i].acc + 1, acc);
    const uint32_t strip = (acc & 0x80) ? 0x83A3DD : 0x7E0000u | p[i].strip;
    video_upload_vram(p[i].at1, strip, 0x1A);
    video_upload_vram(p[i].at2, strip + 0x1A, 0x1A);
  }
}

// TG3000_ROUTINE($85:87BA) irq_race_band
//
// The race's raster IRQ, once per band of the screen: it sends the next chunk of the
// streams the race NMI set up (slot 1: 4 colours to CGRAM $21; slot 2: the CGRAM address
// for slot 3 from its byte; slot 3: 4 colours; slot 7: the BG mode byte), the slots named
// by the band's mask at $0596, then moves the IRQ to the next band's line ($0486, 0 =
// none: then line $FF, dot $78, for the next frame). The original waits (WAI) for dot $FF
// of its line, so the colours change in that line's horizontal blank; a port applies them
// from the next line (docs/DECISIONS.md, "Raster interrupts").
void irq_race_band(void) {
  uint16_t band = rd16(0x00BC);
  platform_set_irq_h_low(0xFF);
  dma_stream_size(2, 1);
  dma_stream_size(7, 1);
  dma_stream_size(1, 8);
  dma_stream_size(3, 8);
  video_set_cgram_address(0x21);
  dma_stream_kick(rd8((uint32_t)(0x0596 + band)));
  band = (uint16_t)(band + 1);
  uint8_t line = rd8((uint32_t)(0x0486 + band));
  if(line) {
    platform_set_irq_v_low(line);
    platform_set_irq_h_low(0x65);
    wr16(0x00BC, band);
  } else {
    platform_set_irq_v(0x00FF);
    platform_set_irq_h(0x0078);
  }
}

// One band of the VS race's raster IRQs (irq_race_band's, with the next IRQ at dot $63): the
// band's chunk, the slots named at masks + the cursor; then the next band's line, if any (the
// cursor moved on). Returns that line, 0 for none.
static uint8_t vs_band_send(uint16_t cursorAt, uint16_t masks, uint16_t lines) {
  uint16_t band = rd16(cursorAt);
  platform_set_irq_h_low(0xFF);
  dma_stream_size(2, 1);
  dma_stream_size(7, 1);
  dma_stream_size(1, 8);
  dma_stream_size(3, 8);
  video_set_cgram_address(0x21);
  dma_stream_kick(rd8((uint32_t)(masks + band)));
  band = (uint16_t)(band + 1);
  const uint8_t line = rd8((uint32_t)(lines + band));
  if(line) {
    platform_set_irq_v_low(line);
    platform_set_irq_h_low(0x63);
    wr16(cursorAt, band);
  }
  return line;
}

// TG3000_ROUTINE($83:8A26) irq_vs_band
//
// The VS race's raster IRQ for the top pair of views (cursor $00BC, slots $0596, lines $0486):
// irq_race_band's, next IRQ at dot $63. After the last band, the split ($83:8A84) at line $6C,
// dot $78. (The original waits (WAI) for dot $FF, as irq_race_band.)
void irq_vs_band(void) {
  if(vs_band_send(0x00BC, 0x0596, 0x0486)) return;
  wr16(0x0005, 0x8A84);
  platform_set_irq_v(0x006C);
  platform_set_irq_h(0x0078);
}

// TG3000_ROUTINE($83:8BE3) irq_vs_band2
//
// The same for the bottom pair (cursor $00C2, slots $061E, lines $050E). After the last band the
// IRQ line goes to 0 (none this frame) and the trampoline stays.
void irq_vs_band2(void) {
  if(!vs_band_send(0x00C2, 0x061E, 0x050E)) platform_set_irq_v(0x0000);
}

// TG3000_ROUTINE($83:8A84) irq_vs_split
//
// (drafted by a delegated agent.) The split between the VS race's two halves, at line $6C
// after the top pair's last band: the display in forced blank (at dot $10A), the bottom pair's
// sprite table ($19B6 / $1BD6 by $00B6), colours $21 and $31 = $0038, the streams re-pointed to
// the bottom pair's sources ($00CA, $00CE; the BG mode bytes at $0482 / $0484, still through
// bank $81, the sprite DMA's), their first chunk ($0032), colour $21 again, OBSEL = 8; then, at
// line $73 (the black divider between the halves), the display back ($014B), colour 7 = $00A2,
// and the bottom pair's band IRQ: the handler word at $0036, line $0034, dot $63. HDMA slot 4's
// table address goes to $7F:FFFE (read back from its register). The original waits (WAI) twice;
// a port applies each part from the next line (docs/DECISIONS.md, "Raster interrupts").
void irq_vs_split(void) {
  platform_set_irq_h(0x010A);
  video_set_display(0x80);
  video_set_oam_address(0x0000);
  const uint16_t buffer = rd16(0x00B6);
  video_upload_oam(0x0000, 0x810000u | (buffer ? 0x19B6 : 0x1BD6), 0x0220);
  video_set_oam_address(0x0000);
  const uint16_t shade = rd16(0x0038);
  video_set_color(0x21, shade);
  video_set_color(0x31, shade);
  dma_hdma_set_mode(7, 0x00, 0x05);  // (slot 7 re-pointed: its bank stays the sprite DMA's, $81)
  dma_stream_set_source(7, buffer ? 0x0482 : 0x0484);
  dma_stream_set_source(1, rd16(0x00CA));
  dma_stream_set_source(3, rd16(0x00CE));
  video_set_bg_mode(0x09);
  dma_stream_size(2, 1);
  dma_stream_size(7, 1);
  dma_stream_size(1, 8);
  dma_stream_size(3, 8);
  video_set_cgram_address(0x21);
  dma_stream_kick(rd8(0x0032));
  video_set_color(0x21, rd16(0x0038));
  video_set_obj_select(0x08);
  platform_set_irq_v(0x0073);
  wr16(0x0005, rd16(0x0036));
  video_set_display(rd8(0x014B));
  video_set_color(0x07, rd16(0x00A2));
  wr16(0x1FFFE, (uint16_t)VS_SLOT4_TABLE);  // $7F:FFFE: the table nmi_vs_full gave slot 4
  platform_set_irq_h(0x0063);
  platform_set_irq_v(rd16(0x0034));
}

// A four-digit BCD add (the 65816's ADC in decimal mode, for valid BCD operands): the low
// four digits of the sum.
static uint16_t bcd_add(uint16_t a, uint16_t b) {
  uint16_t sum = 0;
  unsigned carry = 0;
  for(int shift = 0; shift < 16; shift += 4) {
    unsigned d = ((a >> shift) & 0xF) + ((b >> shift) & 0xF) + carry;
    carry = d > 9;
    if(carry) d -= 10;
    sum |= (uint16_t)(d << shift);
  }
  return sum;
}

// The race's band table: per band, the line it starts on ($0486, from $00BE; 0 ends it)
// and the stream slots its IRQ sends ($0596).
static void race_bands_start(void) {
  uint16_t band = rd16(0x00BE);
  wr16(0x00BC, band);
  uint16_t first = rd8((uint32_t)(0x0486 + band));
  bool raster = false;
  uint16_t line = 0;
  if(first) {
    uint16_t top = (uint16_t)(first - 4);
    wr16(0x002E, top);
    video_set_bg_mode(0x09);
    if(top == 0 || (top & 0x8000)) {
      // the first band starts within 4 lines: send its chunk now
      dma_stream_size(2, 1);
      dma_stream_size(7, 1);
      dma_stream_size(1, 8);
      dma_stream_size(3, 8);
      video_set_cgram_address(0x21);
      dma_stream_kick(rd8((uint32_t)(0x0596 + band)));
      band = (uint16_t)(band + 1);
      wr16(0x00BC, band);
      line = rd8((uint32_t)(0x0486 + band));
      if(line) {
        platform_set_irq_v(line);
        wr16(0x0005, 0x87BA);  // the IRQ trampoline: irq_race_band
        platform_set_irq_h(0x0065);
      } else {
        platform_set_irq_v(0x00FF);
        platform_set_irq_h(0x0078);
      }
      return;
    }
    if(!((uint16_t)(top - 0x00D8) & 0x8000)) raster = false;  // at or below line $DC: none
    else { raster = true; line = (uint16_t)(top + 4); }
  }
  if(raster) {
    wr16(0x0005, 0x87BA);
    platform_set_irq_v(line);
    platform_set_irq_h(0x0065);
    video_set_bg_mode(0x09);
  } else {
    platform_set_irq_v(0x00FF);
    platform_set_irq_h(0x0078);
    video_set_bg_mode(0x29);
  }
}

// The VS band IRQ: the first band's line and its stream slots ($0486/$0596 from $00BE).
static void vs_bands_start(void) {
  uint16_t band = rd16(0x00BE);
  wr16(0x00BC, band);
  const uint16_t first = rd8((uint32_t)(0x0486 + band));
  if(first) {
    uint16_t top = (uint16_t)(first - 4);
    if(top & 0x0080) top = (uint16_t)(top | 0xFF00);  // (never run: a line under 4, or past $83)
    wr16(0x002E, top);
    video_set_bg_mode(0x09);
    if(top == 0 || (top & 0x8000)) {  // the first band starts now: send its chunk
      dma_stream_size(2, 1);
      dma_stream_size(7, 1);
      dma_stream_size(1, 8);
      dma_stream_size(3, 8);
      video_set_cgram_address(0x21);
      dma_stream_kick(rd8((uint32_t)(0x0596 + band)));
      band = (uint16_t)(band + 1);
      wr16(0x00BC, band);
      const uint16_t next = rd8((uint32_t)(0x0486 + band));
      if(next) {
        platform_set_irq_v(next);
        wr16(0x0005, 0x8A26);
        platform_set_irq_h(0x006B);
      } else {  // (never run)
        wr16(0x0005, 0x8A84);
        platform_set_irq_v(0x006C);
        platform_set_irq_h(0x0078);
      }
      return;
    }
    if(top < 0x0068) {  // a raster band
      wr16(0x0005, 0x8A26);
      platform_set_irq_v((uint16_t)(top + 4));
      platform_set_irq_h(0x006B);
      video_set_bg_mode(0x09);
      return;
    }
  }
  wr16(0x0005, 0x8A84);  // no raster
  platform_set_irq_v(0x006C);
  platform_set_irq_h(0x0078);
  video_set_bg_mode(0x29);
}

// The second view's band words $0030-$0036 for its IRQ (the band table $050E from $00C4).
static void vs_band_words(void) {
  uint16_t at = rd16(0x00C4);
  wr16(0x00C2, at);
  const uint16_t v = rd8((uint32_t)(0x050E + at));
  if(v) {
    uint16_t d = (uint16_t)(v - 0x0074);
    if(d & 0x0080) d = (uint16_t)(d | 0xFF00);  // (never run)
    wr16(0x0030, d);
    if(d == 0 || (d & 0x8000)) {
      wr8(0x0032, rd8((uint32_t)(0x061E + at)));
      at = (uint16_t)(at + 1);
      wr16(0x00C2, at);
      const uint16_t next = rd8((uint32_t)(0x050E + at));
      if(next) {
        wr16(0x0034, next);
        wr16(0x0036, 0x8BE3);
      } else {
        wr16(0x0034, 0x0020);  // (never run)
      }
      return;
    }
    if(d < 0x0068) {
      wr16(0x0034, (uint16_t)(d + 0x0074));
      wr8(0x0032, 0);
      wr16(0x0036, 0x8BE3);
      return;
    }
  }
  wr16(0x0032, 0);
  wr16(0x0034, 0x0020);
}

// One of the VS race's two clocks (BCD, $5A-$5C / $5D-$5F): two a frame, one every third.
static void vs_clock_step(uint16_t tick, uint16_t clock) {
  const uint16_t t = (uint16_t)(rd16(tick) - 1);
  wr16(tick, t);
  uint16_t step = 2;
  if(t == 0) {
    wr16(tick, 3);
    step = 1;
  }
  const uint16_t c = bcd_add(rd16(clock), step);
  wr16(clock, c);
  if(c >= 0x6000) {  // (never run)
    wr16(clock, (uint16_t)(c - 0x6000));
    wr16((uint16_t)(clock + 2), (uint16_t)(rd16((uint16_t)(clock + 2)) + 1));
  }
}

// TG3000_ROUTINE($83:9777) nmi_vs_full
//
// (drafted by a delegated agent; its never-run spans decoded here.) The VS split screen's NMI
// frame (the VS NMI $83:9721 dispatches here through $0008), then its tail. nmi_race_full's
// counterpart, with a DMA budget ($00FC) of $1000:
// - Every request ($00B8, cleared whole): the scroll tables (slots 5 and 6, from $00D0 / $00D2),
//   the double buffer flipped, the per-frame values copied, and the VRAM transfer described at
//   $1DFE-$1E04 (never run).
// - The sprite table, colours $21 and $31 from ROM $90 (by the word at $7E:5395), the race VRAM
//   queue, the HUD when $0800 of the budget is left (vs_hud_draw), the four views' scrolls.
// - BG1/BG2/Mode 7 scroll: the original's interleaved byte writes leave BG1 H = M7 H = $0100,
//   BG1 V = 0, M7 V = $0053, BG2 H = 0.
// - Two clocks in BCD (no pause test), HDMA and the streams as the race's (the window from
//   $83:A411, slots 5 and 6 only), the two views' band IRQs, and one of the four cars' sound
//   request, by the frame count.
void nmi_vs_full(void) {
  wr16(RACE_DMA_BUDGET, 0x1000);
  if(rd16(0x00B8)) {
    wr16(0x00B8, 0);
    dma_hdma_set_table(5, rd16(0x00D0));
    dma_hdma_set_table(6, rd16(0x00D2));
    const uint16_t buffer = (uint16_t)(rd16(0x00B4) ^ 0x0001);
    wr16(0x00B4, buffer);
    wr16(0x00B6, buffer);
    static const uint16_t copies[][2] = {
      {0x01CC, 0x01CE}, {0x01D0, 0x01D2}, {0x01D4, 0x01D6}, {0x01D8, 0x01DA}, {0x00C8, 0x016C}, {0x00CC, 0x0170},
      {0x00CA, 0x016E}, {0x00CE, 0x0172}, {0x00BE, 0x00C0}, {0x00C4, 0x00C6}, {0x00E2, 0x00E0}, {0x00EC, 0x0174},
      {0x00F0, 0x0178}, {0x00EE, 0x0176}, {0x00F2, 0x017A}, {0x00F4, 0x017C}, {0x00F8, 0x0180}, {0x00F6, 0x017E},
      {0x00FA, 0x0182},
    };
    for(unsigned k = 0; k < sizeof copies / sizeof copies[0]; k++) wr16(copies[k][0], rd16(copies[k][1]));
    const uint16_t size = rd16(0x1E02);
    if(size) {  // (never run)
      wr16(RACE_DMA_BUDGET, (uint16_t)(rd16(RACE_DMA_BUDGET) - size));
      video_upload_vram(rd16(0x1E00), (uint32_t)rd8(0x1E04) << 16 | rd16(0x1DFE), size);
    }
  }
  const uint16_t sprites = rd16(0x00B4) ? 0x1576 : 0x1796;
  video_set_oam_address(0x0000);
  video_upload_oam(0x0000, 0x810000 | sprites, 0x0220);
  video_set_obj_select(0x00);
  video_set_oam_address(0x0000);
  const uint16_t shade = rom16(0x908000 + (uint32_t)(uint16_t)(rd16(0x5395) - 0x8007));
  wr16(0x0038, shade);
  video_set_color(0x21, shade);
  video_set_color(0x31, shade);
  race_upload_queue_drain();
  if(!((uint16_t)(rd16(RACE_DMA_BUDGET) - 0x0800) & 0x8000)) vs_hud_draw();
  vs_view_scrolls();
  video_set_bg1_hofs(0x0100);
  video_set_bg1_vofs(0x0000);
  video_set_m7_vofs(0x0053);
  video_set_bg2_hofs(0x0000);
  vs_clock_step(0x0060, 0x005A);
  vs_clock_step(0x0062, 0x005D);
  dma_hdma_set_mode(5, 0x43, 0x0D);
  dma_hdma_set_mode(6, 0x43, 0x0F);
  dma_hdma_setup(4, 0x44, 0x26, VS_SLOT4_TABLE);
  dma_stream_setup(1, 0x00, 0x22, 0x7E0000 | rd16(0x00C8));
  dma_stream_setup(2, 0x08, 0x21, 0x838D35);
  dma_stream_setup(3, 0x00, 0x22, 0x7E0000 | rd16(0x00CC));
  dma_stream_setup(7, 0x00, 0x05, rd16(0x00B4) ? 0x7E047E : 0x7E0480);
  dma_hdma_set_bank(5, 0x83);
  dma_hdma_set_bank(6, 0x83);
  dma_hdma_set_indirect_bank(5, 0x83);
  dma_hdma_set_indirect_bank(6, 0x83);
  dma_hdma_set_indirect_bank(4, 0x83);
  video_hdma_enable(0x60);
  vs_bands_start();
  vs_band_words();
  if(!(rd16(0x047C) & 0x8000)) {  // the sound request of one of the four cars (table at $83:9AF0)
    const unsigned i = rd16(0x0024) & 3;
    wr16(0x0460, (uint16_t)((rd16((uint32_t)(0x4E35 + 2 * i)) & 0x00F8) | (6 - 2 * i)));
    wr16(0x0461, rd16((uint32_t)(0x4E2D + 2 * i)));
    wr8(0x045B, (uint8_t)(rd8(0x045B) | 0x20));
  }
  nmi_vs_tail();
}

// TG3000_ROUTINE($85:83CE) nmi_race_full
//
// The race NMI's full frame (the race NMI $85:8377 dispatches here through $0008), then
// its tail. The frame has a DMA budget at $00FC (3.5 KB).
// - Every second request ($00B8 bit 0; $00B4 flips the double buffers): the VRAM
//   transfers described at $1DFE-$1E04 and $1E52-$1E56, 400 bytes from $7E:230C, one
//   from the table at $86:C8AC picked by $1F24, and the copies of the per-frame values.
// - The sprite table (from $1576 or $1796 by $00B4), two palette blocks and colour 0.
// - The race VRAM queue, colours 7, $21, $31, the HUD when the budget is still whole.
// - HDMA: the window shape (slot 4, indirect from $7E:4CCC), BG1 and BG2 scroll (slots 5
//   and 6, tables at $1E46 / $1E48 in bank $83); the streams the raster IRQ sends from
//   (slots 1-3: colours from $7E:$C8 / $83:8D35 / $7E:$CC; slot 7: the BG mode bytes).
// - Windows and colour math, the race clock in BCD ($5A-$5C; the original adds in decimal
//   mode, so the clock must hold valid BCD), the bands' raster IRQ, every fourth frame
//   race_cycle_blocks, the sound request.
void nmi_race_full(void) {
  wr16(RACE_DMA_BUDGET, 0x0E00);
  uint16_t req = rd16(0x00B8);
  wr16(0x00B8, (uint16_t)(req & ~0x0001));
  if(req & 0x0001) {
    wr16(0x00B4, (uint16_t)(rd16(0x00B4) ^ 0x0001));
    uint16_t scroll = rd16(0x1E4E);
    wr16(0x4CD0, scroll);
    wr16(0x4CD3, (uint16_t)(scroll + 0x01B0));
    wr16(0x00C8, rd16(0x016C));
    wr16(0x00CC, rd16(0x0170));
    wr16(0x00BE, rd16(0x00C0));
    dma_hdma_set_table(5, rd16(0x1E46));
    dma_hdma_set_table(6, rd16(0x1E48));
    uint16_t size = rd16(0x1E02);
    if(size) {
      wr16(RACE_DMA_BUDGET, (uint16_t)(rd16(RACE_DMA_BUDGET) - size));
      video_upload_vram(rd16(0x1E00), (uint32_t)rd8(0x1E04) << 16 | rd16(0x1DFE), size);
    }
    video_upload_vram(0x57A0, 0x7E230C, 0x0190);
    wr16(RACE_DMA_BUDGET, (uint16_t)(rd16(RACE_DMA_BUDGET) - 0x0190));
    size = rd16(0x1E54);
    if(size) {
      wr16(RACE_DMA_BUDGET, (uint16_t)(rd16(RACE_DMA_BUDGET) - size));
      video_upload_vram(0x1000, (uint32_t)rd8(0x1E56) << 16 | rd16(0x1E52), size);
    }
    uint16_t pick = rd16(0x1F24);
    if(!(pick & 0x8000)) {
      size = rom16(0x86C8AE + (uint32_t)pick);
      wr16(RACE_DMA_BUDGET, (uint16_t)(rd16(RACE_DMA_BUDGET) - size));
      video_upload_vram(0x1380, (uint32_t)rom8(0x86C8B0 + (uint32_t)pick) << 16 | rom16(0x86C8AC + (uint32_t)pick),
                        size ? size : 0x10000);
    }
    wr16(0x00EC, rd16(0x0174));
    wr16(0x00EE, rd16(0x0176));
    wr16(0x00F4, rd16(0x017C));
    wr16(0x00F6, rd16(0x017E));
    wr16(0x01CC, rd16(0x01CE));
    wr16(0x01B0, rd16(0x01B6));
    wr16(0x01B2, rd16(0x01B8));
    wr16(0x01B4, rd16(0x01BA));
    wr16(0x00E2, rd16(0x01DC));
  }
  uint16_t sprites = rd16(0x00B4) ? 0x1576 : 0x1796;
  video_set_oam_address(0x0000);
  video_upload_oam(0x0000, 0x810000 | sprites, 0x0220);
  video_set_obj_select(0x00);
  video_set_oam_address(0x0000);
  uint16_t pal = rd16(0x024A);
  video_upload_cgram(0xF1, (uint32_t)((pal & 0x8000) ? 0x93 : 0x7E) << 16 | pal, 0x001E);
  video_upload_cgram(0x71, 0x7E6A35, 0x00FE);
  video_set_color(0, rd16(0x6A33));
  wr16(0x024A, 0x2140);
  race_upload_queue_drain_long();
  video_set_color(7, rd16(0x00A0));
  uint16_t c = rd16(0x003A);
  video_set_color(0x21, c);
  video_set_color(0x31, c);
  if(!((uint16_t)(rd16(RACE_DMA_BUDGET) - 0x0E00) & 0x8000)) {
    uint16_t b8 = rd16(0x00B8);
    wr16(0x00B8, (uint16_t)(b8 & ~0x0004));
    if(b8 & 0x0004) race_hud_draw();
  }
  dma_hdma_set_mode(5, 0x43, 0x0D);
  dma_hdma_set_mode(6, 0x43, 0x0F);
  dma_hdma_setup(4, 0x44, 0x26, 0x7E4CCC);
  dma_stream_setup(1, 0x00, 0x22, 0x7E0000 | rd16(0x00C8));
  dma_stream_setup(2, 0x08, 0x21, 0x838D35);
  dma_stream_setup(3, 0x00, 0x22, 0x7E0000 | rd16(0x00CC));
  dma_stream_setup(7, 0x00, 0x05, rd16(0x00B4) ? 0x7E047E : 0x7E0480);
  dma_hdma_set_bank(5, 0x83);
  dma_hdma_set_bank(6, 0x83);
  dma_hdma_set_indirect_bank(5, 0x83);
  dma_hdma_set_indirect_bank(6, 0x83);
  dma_hdma_set_indirect_bank(4, 0x7E);
  video_hdma_enable(0x70);
  func_838989_long();
  video_set_window_bg12(rd8(0x01B0));
  video_set_window_obj(rd8(0x01B1));
  video_set_fixed_color(rd8(0x01B4));
  video_set_fixed_color(rd8(0x01B5));
  video_set_color_math(rd8(0x01B2));  // a 16-bit write: $2131 and $2132
  video_set_fixed_color(rd8(0x01B3));
  if(!(rd16(0x0026) & 0x0010) && (rd16(0x0244) & 0x0008)) {  // the race clock
    uint16_t tick = (uint16_t)(rd16(0x0060) - 1);
    wr16(0x0060, tick);
    uint16_t step = 2;
    if(tick == 0) {
      wr16(0x0060, 3);
      step = 1;
    }
    uint16_t clock = bcd_add(rd16(0x005A), step);
    wr16(0x005A, clock);
    if(clock >= 0x6000) {
      wr16(0x005A, (uint16_t)(clock - 0x6000));
      wr16(0x005C, (uint16_t)(rd16(0x005C) + 1));
    }
  }
  race_bands_start();
  if(!(rd16(0x0024) & 0x0003)) race_cycle_blocks();
  if(!(rd16(0x047C) & 0x8000)) {  // the sound request
    wr16(0x0460, (uint16_t)((rd16(0x4E35) & 0x00F8) | 0x0006));
    wr16(0x0461, rd16(0x4E2D));
    wr8(0x045B, (uint8_t)(rd8(0x045B) | 0x20));
  }
  nmi_race_tail();
}
