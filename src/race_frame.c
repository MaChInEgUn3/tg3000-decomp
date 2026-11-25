// SPDX-License-Identifier: GPL-3.0-or-later
#include "race_frame.h"

#include "frame.h"
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
