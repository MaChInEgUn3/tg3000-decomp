// SPDX-License-Identifier: GPL-3.0-or-later
// Registry of ported routines for the differential test: the adapters map the original's
// registers to the C function's arguments. This mapping lives here, never in src/.
//
// Flag rules used by the adapters: N/Z follow the value the original's last flag-setting
// instruction produced, so they are derived from the C's returned value; flags the original
// does not touch are left as they were on entry (the adapter starts from a copy of `in`).
#include "routines.h"

#include <stddef.h>

#include "rng.h"
#include "scroll_tables.h"
#include "sound.h"
#include "scale.h"
#include "line_tables.h"
#include "decompress.h"
#include "dsp4.h"
#include "frame.h"
#include "pads.h"
#include "race_frame.h"
#include "shop.h"
#include "hud.h"
#include "misc.h"
#include "sprites.h"
#include "track.h"
#include "stars.h"
#include "text.h"
#include "tg3000.h"
#include "util.h"
#include "video_dma.h"
#include "vram_queue.h"
#include "platform.h"

static void setNZ16(Regs* r, uint16_t v) { r->n = (v >> 15) & 1; r->z = v == 0; }

// ---- rng_next ($80:87E9, $8A:988C) ----
// State at D+$10, 16-bit A; returns A = new low word, C = the bit shifted out.
static void call_rng_next(const Regs* in, Regs* out) {
  bool carry;
  out->a = rng_next((uint16_t)(in->d + 0x10), &carry);
  out->c = carry;
  setNZ16(out, out->a);
}

static void fuzz_rng_next(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint32_t s = rnd();
  for(int b = 0; b < 4; b++) ram[(uint16_t)(r->d + 0x10 + b)] = (uint8_t)(s >> (8 * b));
}

// ---- vram_queue_push ($80:83FC) ----
// A = size, Y = VRAM dest, X = source, D+$14 = source bank word. Exits with A = new head,
// X = old head + 2 (unmasked: INX INX before the AND), N/Z from the AND.
static void call_vram_queue_push(const Regs* in, Regs* out) {
  uint16_t oldHead = rd16(VRAM_QUEUE_HEAD);
  out->a = vram_queue_push(in->a, in->y, in->x, rd16((uint16_t)(in->d + 0x14)));
  out->x = (uint16_t)(oldHead + 2);
  setNZ16(out, out->a);
}

static void fuzz_vram_queue_push(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint16_t head = rnd() & 0x3E;  // the game keeps the head even and below $40
  ram[VRAM_QUEUE_HEAD] = (uint8_t)head;
  ram[VRAM_QUEUE_HEAD + 1] = 0;
  uint16_t bank = (uint16_t)rnd();
  ram[(uint16_t)(r->d + 0x14)] = (uint8_t)bank;
  ram[(uint16_t)(r->d + 0x15)] = (uint8_t)(bank >> 8);
}

// ---- take_slot ($8A:A25D) ----
// Counter at D+$8C; returns Y = old value; N/Z from the second INC (value + 2).
static void call_take_slot(const Regs* in, Regs* out) {
  out->y = take_slot((uint16_t)(in->d + 0x8C));
  setNZ16(out, (uint16_t)(out->y + 2));
}

static void fuzz_take_slot(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint16_t v = (uint16_t)rnd();
  ram[(uint16_t)(r->d + 0x8C)] = (uint8_t)v;
  ram[(uint16_t)(r->d + 0x8D)] = (uint8_t)(v >> 8);
}

// ---- lsr_n ($8A:B15E) ----
// A = value, Y = count; exits with Y = 0, Z = 1, N = 0 (from DEY, or from CPY #0).
static void call_lsr_n(const Regs* in, Regs* out) {
  bool carry;
  out->a = lsr_n(in->a, in->y, &carry);
  out->y = 0;
  out->c = carry;
  out->n = false;
  out->z = true;
}

static void fuzz_lsr_n(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)ram;
  r->y = rnd() % 20;  // includes 0 and counts beyond 16
}

// ---- inverse_scale ($80:F709) ----
// Inputs/outputs in the direct page (D+$7A in, D+$14 scratch, D+$38 out). Exits with
// A = result and N/Z from it; X = 0 after the division loop (the early $100 exit leaves X).
// C and V are whatever the last shift/subtract left: they are only compared if the flag
// liveness check finds a caller that reads them.
static void call_inverse_scale(const Regs* in, Regs* out) {
  bool early = rd16((uint16_t)(in->d + SCALE_DIVISOR)) == 0x0100;
  out->a = inverse_scale(in->d);
  if(!early) out->x = 0;
  setNZ16(out, out->a);
}

static void fuzz_inverse_scale(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint32_t k = rnd();
  uint16_t divisor = (k & 7) == 0 ? 0x0100 : (k & 7) == 1 ? 0 : (k & 7) == 2 ? (uint16_t)(rnd() & 0x1FF) : (uint16_t)rnd();
  ram[(uint16_t)(r->d + SCALE_DIVISOR)] = (uint8_t)divisor;
  ram[(uint16_t)(r->d + SCALE_DIVISOR + 1)] = (uint8_t)(divisor >> 8);
}

// ---- mode7_zoom_view ($80:F38D) ----  exits with A's low byte = the high byte of the
// vertical offset (A's high byte kept) and N V Z C from that byte's SBC #$00: N = its top
// bit, V clear, Z = zero, C = no borrow.
static void call_mode7_zoom_view(const Regs* in, Regs* out) {
  uint8_t hi = (uint8_t)(mode7_zoom_view(in->d) >> 8);
  out->a = (uint16_t)((in->a & 0xFF00) | hi);
  out->n = hi & 0x80;
  out->v = false;
  out->z = out->c = hi == 0;
}

// the centre on both sides of the offsets' borrow, and any scale
static void fuzz_mode7_zoom_view(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint32_t k = rnd();
  ram[(uint16_t)(r->d + ZOOM_SCALE)] = (uint8_t)k;
  ram[(uint16_t)(r->d + ZOOM_SCALE + 1)] = (uint8_t)(k >> 8);
  ram[(uint16_t)(r->d + ZOOM_CENTRE_X)] = (k >> 16) & 1 ? (uint8_t)(0x7F + (rnd() & 3)) : (uint8_t)rnd();
  ram[(uint16_t)(r->d + ZOOM_CENTRE_Y)] = (k >> 17) & 1 ? (uint8_t)(0x6E + (rnd() & 3)) : (uint8_t)rnd();
}

// ---- mode7_view ($8A:8B0A) ----  exits with X = 8 * the scene index and A = D+$62,
// N Z C from CMP #$FFC0 on it, except after the vertical scroll write of the set-bit path
// (8-bit loads of D+$62, D+$63: A's low byte = D+$63, N Z from it); V from the
// ADC #$0200 on the scroll.
static void call_mode7_view(const Regs* in, Regs* out) {
  uint16_t scroll = rd16((uint16_t)(in->d + VIEW_SCROLL));
  uint16_t scene = (uint16_t)(rd16(VIEW_SCENE) << 3);
  bool follow = bus_rd16(0x8A9351 + scene) & 0x0004;
  uint16_t y = mode7_view(in->d);
  bool keep = !((uint16_t)(y + 0x40) & 0x8000);
  uint16_t sum = (uint16_t)(scroll + 0x0200);
  out->x = scene;
  out->a = y;
  out->c = y >= 0xFFC0;
  out->v = ((scroll ^ sum) & (0x0200 ^ sum) & 0x8000) != 0;
  if(follow && keep) {
    uint8_t hi = (uint8_t)(y >> 8);
    out->a = (uint16_t)(hi << 8 | hi);
    out->n = hi & 0x80;
    out->z = hi == 0;
  } else {
    out->n = !keep;
    out->z = y == 0xFFC0;
  }
}

// both values of the scene bit (the index kept below $1195 so the table read stays in ROM
// or the bank $8B WRAM mirror, away from the lines the routine writes first), D+$62 around both edges of the $0D0A test, any matrix
static void fuzz_mode7_view(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  for(int i = 0; i < 4; i++) {
    uint32_t k = rnd();
    ram[(uint16_t)(r->d + VIEW_MATRIX + 2 * i)] = (uint8_t)k;
    ram[(uint16_t)(r->d + VIEW_MATRIX + 2 * i + 1)] = (uint8_t)(k >> 8);
  }
  uint16_t words[3] = {VIEW_SCROLL, VIEW_LINE_A, VIEW_LINE_B};
  for(int i = 0; i < 3; i++) {
    uint32_t k = rnd();
    if(i == 0 && (k & 0x30000) == 0) k = (k & 0x01FF) | 0x7E00;  // the ADC's overflow
    ram[(uint16_t)(r->d + words[i])] = (uint8_t)k;
    ram[(uint16_t)(r->d + words[i] + 1)] = (uint8_t)(k >> 8);
  }
  uint32_t k = rnd();
  uint16_t scene = (k & 3) ? (uint16_t)(rnd() % 0x200) : (uint16_t)(rnd() % 0x1195);
  if(scene >= 0x0F30 && scene < 0x0F40) scene = 0;  // its entry would be the $0D00 lines just written
  ram[VIEW_SCENE] = (uint8_t)scene;
  ram[VIEW_SCENE + 1] = (uint8_t)(scene >> 8);
  uint16_t y = (k >> 2) % 3 == 0 ? (uint16_t)(0xFFBE + (rnd() & 3)) : (k >> 2) % 3 == 1 ? (uint16_t)(0x7FBE + (rnd() & 3)) : (uint16_t)rnd();
  ram[(uint16_t)(r->d + VIEW_BG2_Y)] = (uint8_t)y;
  ram[(uint16_t)(r->d + VIEW_BG2_Y + 1)] = (uint8_t)(y >> 8);
}

// ---- draw_meter ($80:DA5F) ----  exits with Y = 0, N = 0, Z = 1 (the last DEY or TYA);
// A = 0 if the ten tiles were used up before the empty fill, else $0848 (the empty tile);
// X = 0 after the full-tile loop, or $00C2 (the VRAM address) if there were none;
// C, V: after a partial tile those of its ADC (both clear), else C = bit 2 of the length
// (the last LSR) and V as on entry.
static void call_draw_meter(const Regs* in, Regs* out) {
  uint16_t length = draw_meter(in->d);
  uint16_t full = length >> 3;
  bool part = length & 7;
  uint16_t left = (uint16_t)(METER_TILES - full - (part ? 1 : 0));
  out->y = 0;
  out->x = full ? 0 : METER_VRAM;
  out->a = left ? METER_EMPTY : 0;
  out->n = false;
  out->z = true;
  out->c = part ? false : (length >> 2) & 1;
  if(part) out->v = false;
}

// values whose bar fits in the ten tiles (-51 to 2022: longer ones run the empty fill
// round the 16-bit counter, 65,000-odd words), with the edges of each tile
static void fuzz_draw_meter(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint32_t k = rnd();
  int16_t v = (k & 3) == 0 ? (int16_t)(-51 + (int)(rnd() % 80)) : (k & 3) == 1 ? (int16_t)(1990 + (int)(rnd() % 33))
            : (int16_t)(-51 + (int)(rnd() % 2074));
  ram[(uint16_t)(r->d + METER_VALUE)] = (uint8_t)v;
  ram[(uint16_t)(r->d + METER_VALUE + 1)] = (uint8_t)((uint16_t)v >> 8);
}

// ---- upload_vram_now ($80:83AD, long call) ----
// A = size, X = source, D+$14 = source bank, Y = VRAM word address. Exits with
// A = $1801 (the DMA mode word, then LDA #$01 in 8-bit), N = Z = 0; X, Y kept.
static void call_upload_vram_now(const Regs* in, Regs* out) {
  upload_vram_now(in->y, rd8((uint16_t)(in->d + 0x14)), in->x, in->a);
  out->a = 0x1801;
  out->n = out->z = false;
}

// Random but plausible uploads: source in ROM or WRAM, sizes up to 2 KB.
static void fuzz_upload(Regs* r, uint8_t* ram, uint32_t (*rnd)(void), bool bankInA) {
  static const uint8_t banks[] = {0x7E, 0x7F, 0x80, 0x85, 0x8A, 0x9F, 0xC0};
  uint8_t bank = banks[rnd() % sizeof banks];
  uint16_t size = (uint16_t)(1 + rnd() % 0x800);
  if(bankInA) {  // upload_cgram_now: A = bank:index, X = source, Y = size
    r->a = (uint16_t)((bank << 8) | (rnd() & 0xFF));
    r->y = size;
  } else {       // upload_vram_now: A = size, D+$14 = bank, Y = VRAM word address
    r->a = size;
    ram[(uint16_t)(r->d + 0x14)] = bank;
    r->y = (uint16_t)(rnd() & 0x7FFF);
  }
  r->x = (uint16_t)rnd();
}

static void fuzz_upload_vram_now(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { fuzz_upload(r, ram, rnd, false); }
static void fuzz_upload_cgram_now(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { fuzz_upload(r, ram, rnd, true); }

// ---- upload_cgram_now ($80:83CB, long call) ----
// A = bank:index, X = source, Y = size. Exits with X = $2202 (the DMA mode word),
// A = index:$01 (XBA, then LDA #$01 in 8-bit), N = Z = 0.
static void call_upload_cgram_now(const Regs* in, Regs* out) {
  upload_cgram_now((uint8_t)in->a, (uint8_t)(in->a >> 8), in->x, in->y);
  out->x = 0x2202;
  out->a = (uint16_t)(((in->a & 0xFF) << 8) | 0x01);
  out->n = out->z = false;
}

static void fuzz_rng_state(uint8_t* ram, uint32_t (*rnd)(void)) {
  uint32_t s = rnd();
  for(int b = 0; b < 4; b++) ram[RNG_STATE + b] = (uint8_t)(s >> (8 * b));
}

// ---- rng_next_long ($80:87DF, long call) ----
// PHD / D = $0100 / JSR rng_next / PLD / RTL: A = result, N/Z from the PLD (entry D).
static void call_rng_next_long(const Regs* in, Regs* out) {
  bool carry;
  out->a = rng_next_long(&carry);
  out->c = carry;
  setNZ16(out, in->d);
}

static void fuzz_rng_next_long(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { (void)r; fuzz_rng_state(ram, rnd); }

// ---- star_spawn ($80:FA3F, $8A:9863) ----  Y = slot; exits with A = $7F00.
static void call_star_spawn(const Regs* in, Regs* out) {
  out->a = star_spawn(in->y);
  setNZ16(out, out->a);
}

static void fuzz_star_spawn(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_rng_state(ram, rnd);
  r->y = (uint16_t)(6 * (rnd() % 40));
}

// ---- star_random_y ($80:E166) ----  X = slot; exits with A = the stored y, N/Z from
// AND, C = 0 (the loop only ends when CMP #$DC00 leaves the carry clear).
static void call_star_random_y(const Regs* in, Regs* out) {
  out->a = star_random_y(in->x);
  out->c = false;
  setNZ16(out, out->a);
}

static void fuzz_star_random_y(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_rng_state(ram, rnd);
  r->x = (uint16_t)(6 * (rnd() % 40));
}

// ---- upload_7f_to_3000 ($8A:BA7D) ----  leaves A = $1801, X = 0, Y = $3000, N = Z = 0.
static void call_upload_7f_to_3000(const Regs* in, Regs* out) {
  (void)in;
  upload_7f_to_3000();
  out->a = 0x1801; out->x = 0; out->y = 0x3000;
  out->n = out->z = false;
}

// ---- upload_tiles_* ($85:8245, $85:8260) ----
// X = table entry, A = offset; X is kept (PHX/PLX, so N/Z come from it), A = $1801,
// Y = the VRAM address.
// C and V are those of the last ADC (table word + offset, carry clear), which the upload
// leaves alone; the sum itself is checked through the upload effect.
#define TILES_ADAPTER(fn, vram) \
  static void call_##fn(const Regs* in, Regs* out) { \
    fn(in->x, in->a); \
    out->a = 0x1801; out->y = vram; \
    setNZ16(out, in->x); \
    uint16_t t = rom16(0x8384B1 + in->x); \
    uint32_t sum = (uint32_t)t + in->a; \
    out->c = sum > 0xFFFF; \
    out->v = (~(t ^ in->a) & (t ^ sum) & 0x8000) != 0; \
  }
TILES_ADAPTER(upload_tiles_3300, 0x3300)
TILES_ADAPTER(upload_tiles_3700, 0x3700)
TILES_ADAPTER(upload_tiles_7700, 0x7700)
TILES_ADAPTER(upload_tiles_7b00, 0x7B00)

// $8A:8D81 / $8A:8DA0: the table word + $20 (ADC #$20), then + A: C and V of that second ADC
#define TILES_ADAPTER_20(fn, vram) \
  static void call_##fn(const Regs* in, Regs* out) { \
    fn(in->x, in->a); \
    out->a = 0x1801; out->y = vram; \
    setNZ16(out, in->x); \
    uint16_t t = (uint16_t)(rom16(0x8384B1 + in->x) + 0x20); \
    uint32_t sum = (uint32_t)t + in->a; \
    out->c = sum > 0xFFFF; \
    out->v = (~(t ^ in->a) & (t ^ sum) & 0x8000) != 0; \
  }
TILES_ADAPTER_20(upload_tiles_c280, 0xC280)
TILES_ADAPTER_20(upload_tiles_c680, 0xC680)

static void setSbc16(Regs* r, uint16_t a, uint16_t b) {  // C and V of A - b, carry set
  uint16_t d = (uint16_t)(a - b);
  r->c = a >= b;
  r->v = ((a ^ b) & (a ^ d) & 0x8000) != 0;
}

static void setAdc16(Regs* r, uint16_t a, uint16_t b) {  // C and V of A + b, carry clear
  uint32_t sum = (uint32_t)a + b;
  r->c = sum > 0xFFFF;
  r->v = (~(a ^ b) & (a ^ sum) & 0x8000) != 0;
}

static void wr16ram(uint8_t* ram, uint32_t adr, uint16_t v) { ram[adr] = (uint8_t)v; ram[adr + 1] = (uint8_t)(v >> 8); }

// ---- interrupt_vectors_init ($80:80E4) ----  A = $8934 (N of its LDA)
static void call_interrupt_vectors_init(const Regs* in, Regs* out) {
  (void)in;
  interrupt_vectors_init();
  out->a = 0x8934;
  setNZ16(out, 0x8934);
}
static void fuzz_interrupt_vectors_init(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(uint32_t a = 0; a < 0x0C; a++) ram[a] = (uint8_t)rnd();
  for(uint32_t a = 0x0026; a < 0x002C; a++) ram[a] = (uint8_t)rnd();
  wr16ram(ram, 0x019A, (uint16_t)rnd());
}

// ---- hdma_table_0d00_init ($8A:9C86) ----  A low = $FF (B kept), X = 42, Y = 0 (its DEY)
static void call_hdma_table_0d00_init(const Regs* in, Regs* out) {
  hdma_table_0d00_init();
  out->a = (uint16_t)((in->a & 0xFF00) | 0x00FF);
  out->x = 42;
  out->y = 0;
  setNZ16(out, 0);
}
static void fuzz_hdma_table_0d00_init(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(uint32_t a = 0x0D00; a < 0x0D40; a++) ram[a] = (uint8_t)rnd();
}

// ---- string_space_fours ($8A:BF2A) ----  A low = 0 (B kept), X = bytes read, Y = written;
// N Z C of the CMP #0 that ended it
static void call_string_space_fours(const Regs* in, Regs* out) {
  uint16_t x = 0, y = 0;
  for(;;) {
    uint8_t c = rd8(0x0D40 + (uint32_t)x);
    x++;
    y++;
    if((x & 3) == 0) y++;
    if(c == 0) break;
  }
  string_space_fours();
  out->a = (uint16_t)(in->a & 0xFF00);
  out->x = x;
  out->y = y;
  out->n = false;
  out->z = true;
  out->c = true;
}
// a string of 0-30 non-zero bytes, then 0
static void fuzz_string_space_fours(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint32_t n = rnd() % 31;
  for(uint32_t i = 0; i < 0x20; i++) ram[0x0D40 + i] = (uint8_t)(1 + rnd() % 255);
  ram[0x0D40 + n] = 0;
  for(uint32_t i = 0; i < 0x30; i++) ram[0x0D60 + i] = (uint8_t)rnd();
}

// ---- table_062a_fill ($8A:8AE7) ----  A = the old D+$3C (PLA), X + 24, Y kept (PLY: N Z),
// C V of the last ADC #$10
static void call_table_062a_fill(const Regs* in, Regs* out) {
  uint16_t first = rd16(DP + 0x3C);
  table_062a_fill(in->x);
  out->a = first;
  out->x = (uint16_t)(in->x + 24);
  out->y = in->y;
  setNZ16(out, in->y);
  setAdc16(out, (uint16_t)(first + 0xB0), 0x10);
}
static void fuzz_table_062a_fill(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)((rnd() % 0x68) * 2);
  wr16ram(ram, DP + 0x3C, (uint16_t)rnd());
  wr16ram(ram, DP + 0x3E, (uint16_t)rnd());
}

// ---- text_type_step ($80:B59D) ----  no register outputs (its callers reload them)
static void call_text_type_step(const Regs* in, Regs* out) { (void)in; (void)out; text_type_step(); }
// a text in WRAM ($7E:1C00, 64 bytes; sometimes copied to the end of the bank, to cross into
// $7F): letters, spaces and marks, with the codes $80, $83,
// $84 (a count of 1-3 after it), $85, $86, and rarely $81 or a forward $82; the speed and
// its sum, the map base and cursor, a width of 1-32, the blink and wait counts, the queue
// (empty slots or map words)
static void fuzz_text_type_step(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  static const uint8_t marks[] = {0x26, 0x23, 0x5B, 0x2E, 0x2B, 0x3D, 0x7B, 0x7D, 0x3F, 0x21, 0x2D, 0x7E, 0x5D};
  static const uint8_t codes[] = {0x80, 0x83, 0x84, 0x85, 0x86};
  for(uint32_t i = 0; i < 0x40; i++) {
    uint32_t k = rnd() % 16;
    uint8_t c = k < 8 ? (uint8_t)(0x30 + rnd() % 0x4B) : k < 10 ? 0x20 : k < 11 ? marks[rnd() % 13] : codes[rnd() % 5];
    if(k >= 11 && (rnd() & 15) == 0) c = 0x81;
    ram[0x1C00 + i] = c;
    if(c == 0x84 && i + 2 < 0x40) {
      ram[0x1C00 + ++i] = (uint8_t)(1 + rnd() % 3);
      ram[0x1C00 + ++i] = 0;
    }
  }
  if((rnd() & 15) == 0) {  // a jump forward
    uint32_t i = rnd() % 0x20;
    ram[0x1C00 + i] = 0x82;
    wr16ram(ram, 0x1C01 + i, (uint16_t)(0x1C00 + i + 3 + rnd() % 0x10));
  }
  wr16ram(ram, DP + 0xCC, (uint16_t)(0x1C00 + rnd() % 0x20));
  ram[DP + 0xCE] = 0x7E;
  bool cross = (rnd() & 3) == 0;  // the text at the end of bank $7E, running on into $7F:0000
  if(cross) {
    for(uint32_t i = 0; i < 0x40; i++) ram[0xFFE0 + i] = ram[0x1C00 + i];
    wr16ram(ram, DP + 0xCC, (uint16_t)(0xFFFC + rnd() % 4));
  }
  wr16ram(ram, DP + 0xE5, (uint16_t)(rnd() % 0x400));
  wr16ram(ram, DP + 0xC6, (uint16_t)(rnd() % 0x400));
  if(cross) wr16ram(ram, DP + 0xE5, (uint16_t)(0x0400 + rnd() % 0x400));  // several steps: the crossing happens
  wr16ram(ram, DP + 0xC8, (uint16_t)((rnd() % 0x1000) * 2));
  wr16ram(ram, DP + 0xCA, (uint16_t)((rnd() % 0x100) * 2));
  wr16ram(ram, DP + 0xF9, (uint16_t)(1 + rnd() % 32));
  wr16ram(ram, DP + 0xF6, (uint16_t)rnd());
  wr16ram(ram, DP + 0xF8, (uint16_t)rnd());
  wr16ram(ram, DP + 0xE1, (uint16_t)(1 + rnd() % 16));
  wr16ram(ram, DP + 0xE3, (uint16_t)(rnd() % 4));
  wr16ram(ram, DP + 0xDF, (uint16_t)rnd());
  for(uint32_t a = 0xCF; a <= 0xDD; a += 2)
    wr16ram(ram, DP + a, (rnd() & 1) ? 0xFFFF : (uint16_t)((rnd() % 0x2000) * 2));
}

// ---- password_pack ($8A:BB99) ----  A = the bit count (N Z of its LDA), X = the table's
// end offset, Y = the last field's width (restored by func_8ABBF4); C and V of the last
// $0192 += $0DCA (func_8ABC32)
static void call_password_pack(const Regs* in, Regs* out) {
  (void)in;
  uint16_t x = 0, y = in->y;
  for(; rom16(0x8ACF61 + (uint32_t)x); x = (uint16_t)(x + 6)) y = (uint16_t)(rom16(0x8ACF65 + (uint32_t)x) >> 8);
  out->a = password_pack();
  out->x = x;
  out->y = y;
  setNZ16(out, out->a);
  uint16_t last = rd16(0x0DCA);
  setAdc16(out, (uint16_t)(rd16(DP + 0x92) - last), last);
}
// every field the table names, where it is in WRAM, randomised
static void fuzz_password_pack(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(uint32_t x = 0; rom16(0x8ACF61 + x); x += 6) {
    uint32_t at = (uint32_t)rom16(0x8ACF63 + x) << 16 | rom16(0x8ACF61 + x);
    uint8_t bank = (uint8_t)(at >> 16);
    uint32_t w = bank == 0x7E || bank == 0x7F ? (at & 0x1FFFF) : ((bank & 0x7F) < 0x40 && (at & 0xFFFF) < 0x2000) ? (at & 0x1FFF) : 0xFFFFFFFF;
    if(w != 0xFFFFFFFF && w + 1 < TG3000_WRAM_SIZE) {
      ram[w] = (uint8_t)rnd();
      ram[w + 1] = (uint8_t)rnd();
    }
  }
}

// ---- hdma_table_0d00_step ($8A:9C27) ----  no register outputs (its caller reloads them)
static void call_hdma_table_0d00_step(const Regs* in, Regs* out) { (void)in; (void)out; hdma_table_0d00_step(); }
static void fuzz_hdma_table_0d00_step(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x6C, (uint16_t)rnd());
  wr16ram(ram, DP + 0x6E, (uint16_t)((rnd() % 0x16) * 2));
  for(uint32_t a = 0x0D00; a < 0x0D30; a++) ram[a] = (uint8_t)((rnd() & 3) == 0 ? 0xE0 : rnd());
}

// ---- race_lights_draw ($80:DAED) ----  no register outputs (its caller reloads them); the
// last CMP #$0C matched (N clear, Z and C set)
static void call_race_lights_draw(const Regs* in, Regs* out) {
  (void)in;
  race_lights_draw();
  out->n = false;
  out->z = true;
  out->c = true;
}
// any count lit (0-13), the blink count and D+$26 either way, func_80D6F3's state as its
// own fuzz leaves it, slots in the buffer
static void fuzz_func_80D6F3(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
static void fuzz_race_lights_draw(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_func_80D6F3(r, ram, rnd);
  wr16ram(ram, 0x0368, (uint16_t)(rnd() % 14));
  wr16ram(ram, DP + 0x3C, (rnd() & 3) == 0 ? 0 : (rnd() & 1) ? 1 : (uint16_t)(rnd() % 0x40));
  wr16ram(ram, DP + 0x26, (rnd() & 1) ? 0 : (uint16_t)rnd());
  wr16ram(ram, DP + 0x74, (uint16_t)((rnd() % 0x60) * 4));
}

// ---- race_order_sort ($85:A066, long) ----  A = $03A8 | $03D7, X and Y where the search
// stopped (N Z of the ORA when both cars were found, else of the DEY that reached 0); DB = $83; C set by the CPY #$28 of the listing loop, V of the
// sort's last SBC (the high words of its last pair, with the middle words' borrow)
static void call_race_order_sort(const Regs* in, Regs* out) {
  (void)in;
  uint16_t order[20];
  for(int i = 0; i < 20; i++) order[i] = (uint16_t)(2 * i);
  uint16_t lastA = 0, lastB = 0;
  for(int end = 19; end > 0; end--) {
    for(int i = 0; i < end; i++) {
      uint16_t a = order[i], b = order[i + 1];
      lastA = a;
      lastB = b;
      uint64_t ta = (uint64_t)rd16(0x69E1 + (uint32_t)a) << 32 | (uint64_t)rd16(0x69B9 + (uint32_t)a) << 16 | rd16(0x6991 + (uint32_t)a);
      uint64_t tb = (uint64_t)rd16(0x69E1 + (uint32_t)b) << 32 | (uint64_t)rd16(0x69B9 + (uint32_t)b) << 16 | rd16(0x6991 + (uint32_t)b);
      if(ta < tb) {
        order[i] = b;
        order[i + 1] = a;
      }
    }
  }
  uint32_t low = (uint32_t)rd16(0x69B9 + (uint32_t)lastA) << 16 | rd16(0x6991 + (uint32_t)lastA);
  uint32_t lowB = (uint32_t)rd16(0x69B9 + (uint32_t)lastB) << 16 | rd16(0x6991 + (uint32_t)lastB);
  uint16_t ha = rd16(0x69E1 + (uint32_t)lastA), hb = rd16(0x69E1 + (uint32_t)lastB);
  uint16_t borrow = low < lowB;
  uint16_t diff = (uint16_t)(ha - hb - borrow);
  out->a = race_order_sort(&out->x, &out->y);
  setNZ16(out, out->y ? out->a : 0);  // both found: the ORA's flags; ten places run out: the DEY's
  out->db = 0x83;
  out->c = true;
  out->v = ((ha ^ hb) & (ha ^ diff) & 0x8000) != 0;
}
// totals equal or apart, any car out of the order (bit 2), the list's count 0-$20
static void fuzz_race_order_sort(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(uint32_t c = 0; c < 40; c += 2) {
    uint16_t hi = (uint16_t)((rnd() & 3) == 0 ? 0 : rnd() % 4);
    wr16ram(ram, 0x69E1 + c, hi);
    wr16ram(ram, 0x69B9 + c, (uint16_t)((rnd() & 1) ? rnd() % 4 : rnd()));
    wr16ram(ram, 0x6991 + c, (uint16_t)rnd());
    wr16ram(ram, 0x6A09 + c, (uint16_t)((rnd() & 7) == 0 ? 0x0004 : (rnd() & ~4u)));
  }
  wr16ram(ram, 0x6A31, (uint16_t)(rnd() % 0x20));
  for(uint32_t a = 0x0446; a < 0x0470; a++) ram[a] = (uint8_t)(rnd() % 22);
}

// ---- hdma_table_0d00_settle ($8A:9C48) ----  no register outputs (its caller reloads them);
// N clear and Z set by the last DEY, C of the last entry's compare ($FF: set; else >= $E0)
static void call_hdma_table_0d00_settle(const Regs* in, Regs* out) {
  (void)in;
  // the step runs first: on the last entry ($6E = $28) it may lower that word by one
  uint16_t count = (uint16_t)(rd16(DP + 0x6C) + 1), word = rd16(0x0D29);
  uint8_t last = (uint8_t)word;
  if(!(count & 3) && rd16(DP + 0x6E) == 0x0028 && word != 0x00E0) last = (uint8_t)(word - 1);
  hdma_table_0d00_settle();
  out->n = false;
  out->z = true;
  out->c = last == 0xFF || last >= 0xE0;
}

// ---- steer_state_init ($80:D6A0) ----  A = the rounded value; N Z of its SBC, C set and V
// clear (the remainder is at most the value, both small)
static void call_steer_state_init(const Regs* in, Regs* out) {
  (void)in;
  steer_state_init();
  out->a = rd16(DP + 0x22);
  setNZ16(out, out->a);
  out->c = true;
  out->v = false;
}
static void fuzz_steer_state_init(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  // below $80 (clamped to 0), within the range, or anywhere
  uint32_t k = rnd() % 3;
  wr16ram(ram, 0x0804, (uint16_t)(k == 0 ? rnd() % 0x80 : k == 1 ? 0x0080 + rnd() % 0x100 : rnd()));
}
static void fuzz_hdma_table_0d00_step(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));

// ---- hdma_gradient_build ($8A:8FA4, long) ----  no register outputs; the last DEC $70
// (N clear, Z set), C of the last compare and V of the last ADC (blue's last step)
static void call_hdma_gradient_build(const Regs* in, Regs* out) {
  (void)in;
  hdma_gradient_build();
  uint16_t next = rd16(0x152C), step = rd16(0x1530), now = (uint16_t)(next - step);
  out->n = false;
  out->z = true;
  setAdc16(out, now, step);  // V of the ADC; C below is the CMP's
  out->c = (now & 0xFF00) >= (next & 0xFF00);
}
// any three colours and patterns; the table in bank $7F
static void fuzz_hdma_gradient_build(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(uint32_t a = 0x34; a <= 0x3C; a += 2) wr16ram(ram, DP + a, (uint16_t)rnd());
  wr16ram(ram, DP + 0x68, (uint16_t)(0x4000 + (rnd() % 0x100) * 4));
  ram[DP + 0x6A] = 0x7F;
  wr16ram(ram, DP + 0x6C, (uint16_t)((rnd() % 0x40) * 4));
}

// ---- flag_03dc_from_52 ($8A:9F64) ----  A = $1000 (N clear), Z of the TSB/TRB's test
static void call_flag_03dc_from_52(const Regs* in, Regs* out) {
  (void)in;
  bool was = (rd16(0x03DC) & 0x1000) != 0;
  flag_03dc_from_52();
  out->a = 0x1000;
  out->n = false;
  out->z = !was;
}
static void fuzz_flag_03dc_from_52(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x52, (rnd() & 1) ? 0 : (uint16_t)rnd());
  wr16ram(ram, 0x03DC, (uint16_t)rnd());
}

// ---- timer_0d7e_tick ($8A:9F76) ----  counting: A kept, N Z of the DEC; at 0: A = $03DC &
// 7, C of its CMP #2, N Z of the INC of D+$6C
static void call_timer_0d7e_tick(const Regs* in, Regs* out) {
  bool fires = rd16(0x0D7E) == 1;
  out->a = timer_0d7e_tick(in->a);
  if(!fires) {
    setNZ16(out, rd16(0x0D7E));
    return;
  }
  out->c = out->a >= 2;
  setNZ16(out, rd16(DP + 0x6C));
}
// the count often at 1 (so the never-run branch and its random pick run), the flags word,
// the random state
static void fuzz_timer_0d7e_tick(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x0D7E, (rnd() % 3) == 0 ? 1 : (uint16_t)rnd());
  wr16ram(ram, 0x03DC, (uint16_t)rnd());
  wr16ram(ram, DP + 0x6C, (uint16_t)rnd());
  for(uint32_t a = 0x0110; a < 0x0114; a++) ram[a] = (uint8_t)rnd();
}

// ---- text_line_put ($8A:9B85) ----  A kept (N Z of its PLA), X = the line's old D+$54, Y =
// the length; C and V of func_8A9BAA's ADC
static void call_text_line_put(const Regs* in, Regs* out) {
  uint16_t old = rd16(DP + 0x54);
  out->y = text_line_put(in->x, in->y);
  out->a = in->a;
  out->x = old;
  setNZ16(out, in->a);
  setAdc16(out, old, 0x10);
}
// a string of 0-30 non-zero bytes in WRAM ($7E:1C00), the line anywhere in the map
static void fuzz_text_line_put(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint32_t n = rnd() % 31;
  for(uint32_t i = 0; i < n; i++) ram[0x1C00 + i] = (uint8_t)(1 + rnd() % 255);
  ram[0x1C00 + n] = 0;
  r->x = 0x1C00;
  r->y = 0x007E;
  wr16ram(ram, DP + 0x54, (uint16_t)(rnd() % 0x600));
  wr16ram(ram, DP + 0x56, (uint16_t)rnd());
}

// ---- text_centred_line ($82:B18B) ----  no register outputs (its caller reloads them); the
// final LDA of the zero (N clear, Z set); C clear after any character or palette byte (an
// ASL), else of the start's ADC; V of the last SBC (a digit or letter), else of that ADC
static void call_text_centred_line(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t start = rom16(0x83A006 + (uint32_t)rd16(DP + 0x3C));
  uint16_t free = 13;
  for(uint16_t s = start; bus_rd8(0x830000 | s); s++) if(!(bus_rd8(0x830000 | s) & 0x80)) free--;
  setAdc16(out, (uint16_t)(free & 0xFFFE), rd16(DP + 0x18));
  for(uint16_t s = start; bus_rd8(0x830000 | s); s++) {
    uint8_t c = bus_rd8(0x830000 | s);
    out->c = false;
    if(!(c & 0x80) && c != 0x20 && c != 0x21 && c != 0x2E && c != 0x2B) out->v = false;  // a small SBC
  }
  text_centred_line();
  out->n = false;
  out->z = true;
}
// any string of the table ($83:A006, by D+$3C: its 79 entries; six of them, $7A-$84, point
// at WRAM: there, random strings of letters, digits, marks, '.' and palette bytes), the
// palette and the line's place in WRAM
static void fuzz_text_centred_line(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x3C, (rnd() & 1) ? (uint16_t)(0x7A + (rnd() % 6) * 2) : (uint16_t)((rnd() % 0x4F) * 2));
  static const uint8_t marks[] = {0x20, 0x21, 0x2E, 0x2B, 0x80, 0x83, 0x87};
  static const uint16_t wram[] = {0x037E, 0x03AD, 0x03E4, 0x03F1, 0x03FE, 0x040B};
  for(unsigned k = 0; k < 6; k++) {
    uint32_t n = rnd() % 13;
    for(uint32_t i = 0; i < n; i++) {
      uint32_t t = rnd() % 4;
      ram[wram[k] + i] = t == 0 ? (uint8_t)('A' + rnd() % 26) : t == 1 ? (uint8_t)('0' + rnd() % 10) : marks[rnd() % 7];
    }
    ram[wram[k] + n] = 0;
  }
  wr16ram(ram, DP + 0x3A, (uint16_t)((rnd() % 8) << 10));
  wr16ram(ram, DP + 0x18, (uint16_t)(0x2000 + (rnd() % 0x800) * 2));
}

// ---- shop_list_label ($80:B2F2) ----  X = the new index; Y as sprite_text_centered leaves
// it (the slot) when drawn, else kept, then x / 2 past the end check; A and flags by the exit
static void call_shop_list_label(const Regs* in, Regs* out) {
  int16_t row = (int16_t)rd16(DP + 0x22);
  bool drawn = (int16_t)(row - 0x1000) >= 0 && (int16_t)(row - 0x5800) < 0;
  uint16_t row0 = rd16(DP + 0x22);
  out->x = shop_list_label(in->x);
  out->y = drawn ? rd16(DP + 0x74) : in->y;
  uint16_t moved = rd16(DP + 0x22), x2 = (uint16_t)(in->x + 2);
  setAdc16(out, row0, 0x0800);  // V (C is replaced below)
  if(x2 == 0x0020) {  // CPX #$20 matched
    out->a = moved;
    out->n = false;
    out->z = true;
    out->c = true;
    return;
  }
  out->y = (uint16_t)(x2 >> 1);
  out->c = false;  // the LSR of an even index
  uint8_t count = rd8(0x036E + (uint32_t)(x2 >> 1));
  if(count != 0) {
    out->a = count;
    out->n = false;
    out->z = false;
    return;
  }
  out->a = (uint16_t)(x2 >> 1);
  out->n = false;  // LDX #$1E
  out->z = false;
}
// a list 0-14 (x even), the row anywhere around the window, list counts zero or not
static void fuzz_shop_list_label(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)((rnd() % 0x10) * 2);
  wr16ram(ram, DP + 0x22, (uint16_t)(0x0800 * (rnd() % 14) - 0x0800 + (rnd() & 0x07FF)));
  wr16ram(ram, DP + 0xE7, (uint16_t)rnd());
  for(uint32_t i = 0; i < 0x10; i++) ram[0x036E + i] = (rnd() & 3) == 0 ? 0 : (uint8_t)(1 + rnd() % 5);
  wr16ram(ram, DP + 0x74, (uint16_t)((rnd() % 0x40) * 4));
}

// ---- race_clear_0242_0208 ($82:ECEA) ---- / race_countdown ($82:AB9D): no register outputs
// (flags by liveness)
static void call_race_clear_0242_0208(const Regs* in, Regs* out) { (void)in; (void)out; race_clear_0242_0208(); }
static void call_race_countdown(const Regs* in, Regs* out) { (void)in; (void)out; race_countdown(); }
static void fuzz_race_clear_0242_0208(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x0242, (uint16_t)rnd());
  wr16ram(ram, 0x0208, (uint16_t)rnd());
}
// the count around each mark, the step, negative or 0 sometimes; the flags it reads
static void fuzz_race_countdown(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  static const uint16_t marks[] = {0x00D2, 0x008C, 0x0046, 0x0002};
  uint32_t k = rnd() % 8;
  uint16_t count = k == 0 ? (uint16_t)(0x8000 | rnd()) : k == 1 ? 0 : (uint16_t)(marks[rnd() % 4] + rnd() % 8);
  wr16ram(ram, 0x1ECE, count);
  wr16ram(ram, 0x1E9E, (uint16_t)(rnd() % 12));
  wr16ram(ram, 0x047C, (uint16_t)rnd());
  wr16ram(ram, 0x03DC, (uint16_t)rnd());
  for(uint32_t a = 0x0244; a < 0x0246; a++) ram[a] = (uint8_t)rnd();
}

// ---- race_scroll_commit ($82:ECF1) ----  A = the new $1EF6; N Z C V of its ADC (with the
// low words' carry)
static void call_race_scroll_commit(const Regs* in, Regs* out) {
  (void)in;
  uint16_t lo = rd16(0x0204), hi = rd16(0x0206), tlo = rd16(0x1EF4), thi = rd16(0x1EF6);
  bool carry = (uint32_t)lo + tlo > 0xFFFF;
  out->a = race_scroll_commit();
  setNZ16(out, out->a);
  uint32_t sum = (uint32_t)hi + thi + carry;
  out->c = sum > 0xFFFF;
  out->v = (~(hi ^ thi) & (hi ^ sum) & 0x8000) != 0;
}
static void fuzz_race_scroll_commit(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(uint32_t a = 0x0200; a < 0x020E; a++) ram[a] = (uint8_t)rnd();
  for(uint32_t a = 0x1EEE; a < 0x1EF8; a++) ram[a] = (uint8_t)rnd();
}

// ---- car_lap_check ($83:F9D1) ----  no register outputs (flags by liveness)
static void call_car_lap_check(const Regs* in, Regs* out) { (void)out; car_lap_check(in->d); }
// the frame reached or not, the count just before 0 or the finish, the counters around
static void fuzz_car_lap_check(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint16_t dp = r->d, frame = (uint16_t)rnd();
  wr16ram(ram, 0x019A, frame);
  wr16ram(ram, (uint16_t)(dp + 0x26), (rnd() & 3) ? (uint16_t)(frame + rnd() % 4) : (uint16_t)rnd());
  uint16_t laps = (uint16_t)(1 + rnd() % 8);
  wr16ram(ram, 0x1F06, laps);
  uint32_t k = rnd() % 4;
  wr16ram(ram, (uint16_t)(dp + 0x35), k == 0 ? 0xFFFF : k == 1 ? (uint16_t)(laps - 1) : (uint16_t)rnd());
  wr16ram(ram, (uint16_t)(dp + 0x31), (rnd() & 1) ? 0 : (uint16_t)rnd());
  wr16ram(ram, (uint16_t)(dp + 0x7C), (uint16_t)rnd());
  wr16ram(ram, (uint16_t)(dp + 0x44), (uint16_t)rnd());
  wr16ram(ram, 0x1EBA, (uint16_t)((rnd() % 0x20) * 2));
  wr16ram(ram, 0x1F08, (uint16_t)(rnd() % 0x14));
  wr16ram(ram, 0x1ECA, (uint16_t)rnd());
}

// ---- star_layers_move ($80:DACF) ----  A = X = $200; the last CPX sets Z and C, the
// last ADC #8 clears V
static void call_star_layers_move(const Regs* in, Regs* out) {
  (void)in;
  star_layers_move();
  out->a = 0x0200;
  out->x = 0x0200;
  out->n = false;
  out->z = true;
  out->c = true;
  out->v = false;
}
static void fuzz_star_layers_move(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(uint32_t a = 0x0F00; a < 0x1100; a++) ram[a] = (uint8_t)rnd();
}

// ---- the fade to black ($8A:BAE3, $8A:9675, $8A:8181) ----
// waiting: A = 0 from the last LDA $0196
static void call_fade_out_wait(const Regs* in, Regs* out) {
  (void)in;
  fade_out_wait();
  out->a = 0;
  setNZ16(out, 0);
}
// started: A = $FF80 (N set); not: A = $C0C0, N from its LDA, Z from the TRB that found nothing
static void call_fade_out_on_confirm(const Regs* in, Regs* out) {
  (void)in;
  bool started = fade_out_on_confirm();
  out->a = started ? 0xFF80 : 0xC0C0;
  out->n = true;
  out->z = !started;
}
// already counted: A = D+$8C (N Z of its LDA); started: A = $FF80, N Z of the INC
static void call_fade_out_on_confirm_once(const Regs* in, Regs* out) {
  (void)in;
  uint16_t count = rd16(DP + 0x8C);
  if(count) {
    fade_out_on_confirm_once();
    out->a = count;
    setNZ16(out, count);
    return;
  }
  fade_out_on_confirm_once();
  if(rd16(DP + 0x8C)) {
    out->a = 0xFF80;
    setNZ16(out, rd16(DP + 0x8C));
  } else {
    out->a = 0xC0C0;
    out->n = true;
    out->z = true;
  }
}
// a confirm press on either pad or neither (other bits random); D+$8C 0 or not
static void fuzz_fade_out_on_confirm(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(uint32_t pad = 0x000E; pad <= 0x0012; pad += 4) {
    uint16_t bits = (uint16_t)(rnd() & ~0xC0C0);
    if((rnd() & 3) == 0) bits = (uint16_t)(bits | (0x4000 >> (rnd() % 2 * 8)) << (rnd() & 1));
    wr16ram(ram, pad, bits);
  }
  wr16ram(ram, DP + 0x8C, (rnd() & 1) ? 0 : (uint16_t)rnd());
  wr16ram(ram, 0x0196, (uint16_t)rnd());
  wr16ram(ram, 0x0198, (uint16_t)rnd());
}

// ---- func_8A99D0, func_8A96C2 ----
static void call_func_8A99D0(const Regs* in, Regs* out) {
  (void)in;
  func_8A99D0();
  out->a = 0x0028;
  setNZ16(out, 0x0028);
}
// A = D+$40 + 1 (N Z of its LDA), C clear
static void call_func_8A96C2(const Regs* in, Regs* out) {
  (void)in;
  out->a = func_8A96C2();
  setNZ16(out, out->a);
  out->c = false;
}
static void fuzz_dp_40_66(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x40, (uint16_t)rnd());
  wr16ram(ram, DP + 0x66, (uint16_t)rnd());
}


// ---- func_80B537 ----  A = the new index, X = old index + 2, N Z C V of the final ADC.
static void call_func_80B537(const Regs* in, Regs* out) {
  uint16_t i = rd16((uint16_t)(in->d + 0x8C)), base = rd16((uint16_t)(in->d + 0x8A));
  out->a = func_80B537(in->d, in->a, in->x, in->y);
  out->x = (uint16_t)(i + 2);
  setNZ16(out, out->a);
  setAdc16(out, (uint16_t)((i + 2) & 0x3F), base);
}

// the index anywhere the four tables stay in low WRAM below the fuzz's stack ($1F00
// down); any ring base
static void fuzz_func_80B537(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  wr16ram(ram, (uint16_t)(r->d + 0x8C), (uint16_t)(rnd() % 0x500));
  wr16ram(ram, (uint16_t)(r->d + 0x8A), (rnd() & 1) ? (uint16_t)(rnd() & 0xC0) : (uint16_t)rnd());
}


// ---- star_layers_init ($80:E0D6) ----  exits with A = X = $200 and the flags of
// CPX #$200 (Z = C = 1, N = 0) and of the last ADC #8 (V = 0).
static void call_star_layers_init(const Regs* in, Regs* out) {
  (void)in;
  out->a = out->x = star_layers_init();
  out->z = out->c = true;
  out->n = out->v = false;
}

static void fuzz_star_layers_init(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { (void)r; fuzz_rng_state(ram, rnd); }

// ---- func_81F251 ($81:F251): RTS ----
static void call_func_81F251(const Regs* in, Regs* out) { (void)in; (void)out; func_81F251(); }

static void fuzz_nothing(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { (void)r; (void)ram; (void)rnd; }

// ---- clear_dp_scratch ($8A:8001, long call) ----  exits with X = $FFFE, N = 1, Z = 0.
static void call_clear_dp_scratch(const Regs* in, Regs* out) {
  (void)in;
  clear_dp_scratch();
  out->x = 0xFFFE;
  out->n = true;
  out->z = false;
}

// ---- func_81F1FC ($81:F1FC) ----  A = the first entry; X/Y kept (N/Z from PLX);
// C = 1 from the CPX #$26 that ends the loop.
static void call_func_81F1FC(const Regs* in, Regs* out) {
  out->a = func_81F1FC();
  out->c = true;
  setNZ16(out, in->x);
}

static void fuzz_func_81F1FC(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(int i = 0; i < 20; i++) wr16ram(ram, 0x1E6A + 2 * i, (uint16_t)(2 * (rnd() % 20)));
}

// ---- func_8AA216 ($8A:A216) ----  X = entry; exits with A = the new $0D60 value (flags of
// its ADC #8), Y = the second slot.
static void call_func_8AA216(const Regs* in, Regs* out) {
  uint16_t oldX = rd16(0x0D60 + in->x);
  func_8AA216(in->x);
  out->a = rd16(0x0D60 + in->x);
  out->y = (uint16_t)(rd16(DP + 0x8C) - 2);
  setNZ16(out, out->a);
  setAdc16(out, oldX, 8);
}

static void fuzz_func_8AA216(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)(2 * (rnd() % 4));
  wr16ram(ram, 0x0D60 + r->x, (uint16_t)rnd());
  wr16ram(ram, 0x0D68 + r->x, (uint16_t)rnd());
  wr16ram(ram, DP + 0x8C, (uint16_t)(2 * (rnd() % 64)));
  wr16ram(ram, DP + 0x4C, (uint16_t)rnd());
}

// ---- func_8A92CF ($8A:92CF) ----  X = slot; exits with X = slot + 4, A = $017C (N/Z from
// the PLA), C/V of x + $10.
static void call_func_8A92CF(const Regs* in, Regs* out) {
  uint16_t x = rd16(DP + 0x7A);
  out->x = func_8A92CF(in->x);
  out->a = rd16(DP + 0x7C);
  setNZ16(out, out->a);
  setAdc16(out, x, 0x10);
}

static void fuzz_func_8A92CF(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)(4 * (rnd() % 32));
  wr16ram(ram, DP + 0x7A, (uint16_t)rnd());
  wr16ram(ram, DP + 0x7C, (uint16_t)rnd());
}

static void fuzz_upload_tiles(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)ram;
  r->x = (uint16_t)(2 * (rnd() % 64));
  r->a = (uint16_t)(rnd() & 0x1FFF);
}

// ---- func_80F745 ($80:F745) ----  exits with A = the old back offset, X = the new one
// (LDX: N/Z from it); C/V depend on the path and are left to the liveness check.
static void call_func_80F745(const Regs* in, Regs* out) {
  (void)in;
  out->a = func_80F745();
  out->x = rd16(DP + 0x34);
  setNZ16(out, out->x);
}

static void fuzz_func_80F745(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint32_t k = rnd();
  wr16ram(ram, DP + 0x34, (k & 1) ? 0x0000 : 0x0020);  // the game alternates two buffers
  wr16ram(ram, DP + 0x36, (k & 1) ? 0x0020 : 0x0000);
  wr16ram(ram, DP + 0x38, (uint16_t)(rnd() & 0x0FFF));
  wr16ram(ram, DP + 0x3A, (k & 6) == 0 ? 0 : (uint16_t)rnd());
  wr16ram(ram, DP + 0x3C, (uint16_t)rnd());
  wr16ram(ram, DP + 0x32, (k & 8) ? (uint16_t)(rnd() & 0x0F00) : 0);
  wr16ram(ram, DP + 0x14, (uint16_t)rnd());
  wr16ram(ram, DP + 0x16, (uint16_t)rnd());
}

// ---- tilemap_offset ($8A:B909) ----  X = offset, A kept (PHA/PLA: N/Z from it);
// C/V from the final ADC.
static void call_tilemap_offset(const Regs* in, Regs* out) {
  uint16_t row = (uint16_t)(rd16(DP + 0x38) << 6);
  out->x = tilemap_offset();
  setNZ16(out, in->a);
  setAdc16(out, row, rd16(DP + 0x36));
}

static void fuzz_tilemap_offset(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x36, (uint16_t)rnd());
  wr16ram(ram, DP + 0x38, (uint16_t)rnd());
}

// ---- func_8AB700 ($8A:B700) ----  exits with A = 4 or 0, N/Z from the PLY (Y kept).
static void call_func_8AB700(const Regs* in, Regs* out) {
  func_8AB700();
  out->a = (rd16(DP + 0x5A) & 1) ? 4 : 0;
  setNZ16(out, in->y);
  setSbc16(out, rd16(DP + 0x3C), 10);  // C/V of its SEC/SBC #10
}

static void fuzz_func_8AB700(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x3C, (uint16_t)(10 + rnd() % 0x40));  // >= 10: see the C's comment
  wr16ram(ram, DP + 0x5A, (uint16_t)rnd());
}

// ---- text_put_char ($8A:B83C) ----  entered with 8-bit A = the character. PHP/PLP keep
// V/C, and N/Z of the LDX $0D7C before the PHP; exits with A = the bottom tile written,
// X = the map offset, Y kept.
static void call_text_put_char(const Regs* in, Regs* out) {
  uint16_t at = text_put_char((uint8_t)in->a);
  out->x = at;
  out->a = rd16(0x10040 + at);
  setNZ16(out, rd16(TEXT_FONT_SEL));  // the PHP comes after LDX $0D7C
}

static void fuzz_text_put_char(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint32_t k = rnd();
  r->a = (uint16_t)((r->a & 0xFF00) | (0x20 + rnd() % 0x60));
  wr16ram(ram, TEXT_FONT_SEL, (k & 3) == 0 ? 0 : 1);
  wr16ram(ram, DP + 0x3A, (uint16_t)(rnd() % 32));
  wr16ram(ram, DP + 0x3C, (uint16_t)(rnd() % 16));
}

// ---- char_to_tile_index ($8A:C2A2) ----  A in, A out; N/Z from the result. C/V are those
// of the last CMP (an equal match: C = 1, V kept) or of the final SEC/SBC (#$16 or #$41).

static void call_char_to_tile_index(const Regs* in, Regs* out) {
  out->a = char_to_tile_index(in->a);
  setNZ16(out, out->a);
  uint16_t ch = in->a;
  if(ch == 0x20 || ch == 0x2E || ch == 0x21) out->c = true;
  else setSbc16(out, ch, (int16_t)(ch - 0x41) < 0 ? 0x16 : 0x41);
}

static void fuzz_char_to_tile_index(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)ram;
  static const uint16_t special[] = {0x20, 0x2E, 0x21, 0x41, 0x40, 0x5A};
  uint32_t k = rnd();
  r->a = (k & 3) == 0 ? special[rnd() % 6] : (uint16_t)(rnd() & 0x7F);
}

// ---- sprite_frame ($8A:A0F9) ----  X = entry; exits with A = the entry's x (PLA), X =
// entry (N/Z from the PLX), Y = the last slot, C/V of the last ADC (y + row + row).
static void call_sprite_frame(const Regs* in, Regs* out) {
  sprite_frame(in->x);
  out->a = rd16(0x0D60 + in->x);
  out->y = (uint16_t)(rd16(DP + 0x8C) - 2);
  setNZ16(out, in->x);
  uint16_t y = rd16(0x0D68 + in->x), row = rd16(DP + 0x4C);
  setAdc16(out, (uint16_t)(y + row), row);
}

static void fuzz_sprite_frame(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)(2 * (rnd() % 4));
  for(uint16_t t = 0x0D60; t <= 0x0D90; t += 8) wr16ram(ram, t + r->x, (uint16_t)rnd());
  // the game's widths are small (a frame of a few columns); so are the step counts
  wr16ram(ram, 0x0D70 + r->x, (uint16_t)(rnd() % 0x60));
  wr16ram(ram, 0x0D78 + r->x, (uint16_t)(rnd() % 0x60));
  uint16_t y = (uint16_t)rnd();
  wr16ram(ram, 0x0D68 + r->x, y);
  wr16ram(ram, 0x0D90 + r->x, (uint16_t)(y + (int16_t)(rnd() % 25) - 12));
  wr16ram(ram, DP + 0x7A, (uint16_t)(rnd() % 12));
  wr16ram(ram, DP + 0x7C, (uint16_t)(rnd() % 12));
  wr16ram(ram, DP + 0x7E, (uint16_t)(rnd() % 12));
  wr16ram(ram, DP + 0x4C, (uint16_t)(rnd() % 0x20));
  wr16ram(ram, DP + 0x8C, (uint16_t)(2 * (rnd() % 32)));
}

// ---- func_8A9FA1 ($8A:9FA1) ----  exits with A = $FFFF (the terminator), X/Y restored
// (N/Z from the last PLX), C = 1 from the CMP that found the terminator.
static void call_func_8A9FA1(const Regs* in, Regs* out) {
  func_8A9FA1();
  out->a = 0xFFFF;
  setNZ16(out, in->x);
  out->c = true;
}

static void fuzz_func_8A9FA1(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x58, (uint16_t)(rnd() % 6));
  for(uint16_t i = 0; i < 0x20; i += 2) wr16ram(ram, 0x1516 + i, (uint16_t)rnd());
}

// ---- clear_tilemap_7f ($8A:B981) ----  exits with A = $3D, X = $1000, Y = 0 (Z from DEY).
static void call_clear_tilemap_7f(const Regs* in, Regs* out) {
  (void)in;
  clear_tilemap_7f();
  out->a = 0x003D;
  out->x = 0x1000;
  out->y = 0;
  setNZ16(out, 0);
}

// ---- func_8ABC32 ($8A:BC32) ----  A kept (N/Z from the PLA), C/V of the ADC.
static void call_func_8ABC32(const Regs* in, Regs* out) {
  uint16_t old = rd16(DP + 0x92);
  func_8ABC32();
  setNZ16(out, in->a);
  setAdc16(out, old, rd16(0x0DCA));
}

static void fuzz_func_8ABC32(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x92, (uint16_t)rnd());
  wr16ram(ram, 0x0DCA, (uint16_t)rnd());
}

// ---- func_8ABC7F ($8A:BC7F) ----  exits with X = $FFFE (N from the last DEX).
static void call_func_8ABC7F(const Regs* in, Regs* out) {
  (void)in;
  func_8ABC7F();
  out->x = 0xFFFE;
  setNZ16(out, out->x);
}

// ---- func_82EC09 ($82:EC09) ----  X = entry; exits with A = the difference, N/Z from the
// ASL of $011E, C = 0 (a byte doubled), V of the SBC.
static void call_func_82EC09(const Regs* in, Regs* out) {
  uint16_t b = rd8(0x8CB5 + (uint32_t)in->x);
  out->a = func_82EC09(in->x);
  setSbc16(out, (uint16_t)(out->a + b), b);
  setNZ16(out, rd16(DP + 0x1E));
  out->c = false;
}

static void fuzz_func_82EC09(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)(rnd() % 0x200);
  wr16ram(ram, 0x8CB3 + r->x, (uint16_t)rnd());
  wr16ram(ram, 0x8CB5 + r->x, (uint16_t)rnd());
}

// ---- text_print_tiles ($8A:C13F) ----  A = string (bank DB); exits with A = 0, Y = length,
// Z = 1, C of the last CMP #$80 (unchanged for an empty string).
static void call_text_print_tiles(const Regs* in, Regs* out) {
  uint32_t str = ((uint32_t)in->db << 16) | in->a;
  out->y = text_print_tiles(str);
  out->a = 0;
  setNZ16(out, 0);
  if(out->y > 0) out->c = bus_rd8(((uint32_t)in->db << 16) + (uint16_t)(in->a) + out->y - 1) >= 0x80;
}

static void fuzz_text_print_tiles(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  // DB is the code's bank ($8A); its $0000-$1FFF is WRAM, so the string goes at $1C00
  r->a = 0x1C00;
  int n = (int)(rnd() % 12);
  for(int i = 0; i < n; i++) ram[0x1C00 + i] = (rnd() % 6 == 0) ? 0x80 : (uint8_t)(0x20 + rnd() % 0x3B);
  ram[0x1C00 + n] = 0;
  wr16ram(ram, DP + 0x3A, (uint16_t)(rnd() % 16));
  wr16ram(ram, DP + 0x3C, (uint16_t)(rnd() % 8));
  wr16ram(ram, DP + 0x90, (uint16_t)(rnd() & 1));
}

// ---- wipe_columns ($8A:B91A) ----  exits with X = 0 (N/Z of the last DEX), Y = $0174 + 2
// (func_8AB942's), A = the last wait's target and its C = 1 / V, or, when the last step
// changed nothing, A = 0 and the C/V of func_8AB942's ADC with $0170 = 0.
static void call_wipe_columns(const Regs* in, Regs* out) {
  (void)in;
  out->a = wipe_columns();
  out->x = 0;
  out->y = (uint16_t)(rd16(DP + 0x74) + 2);
  setNZ16(out, 0);
  if(out->a != 0) {
    out->c = true;
    out->v = (uint16_t)(out->a - 1) == 0x7FFF;
  } else {
    setAdc16(out, 27 * 0x40, 0x40);
  }
}

// ---- func_8AA48D, func_8AA49E ----  A kept (N/Z from the PLA).
static void call_func_8AA48D(const Regs* in, Regs* out) { func_8AA48D(); setNZ16(out, in->a); }
static void call_func_8AA49E(const Regs* in, Regs* out) { func_8AA49E(); setNZ16(out, in->a); }

// ---- clear_sprites ($8A:8F02, long) ----  REP #$30 first; exits with A = $E000, X = 0
// (Z from the last DEX), Y = $200.
static void call_clear_sprites(const Regs* in, Regs* out) {
  (void)in;
  clear_sprites();
  out->m8 = out->x8 = false;
  out->a = 0xE000;
  out->x = 0;
  out->y = 0x0200;
  setNZ16(out, 0);
}

// ---- text_print_tiles_at ($8A:C137) ----  X = column, Y = row, A = string (bank DB).
static void call_text_print_tiles_at(const Regs* in, Regs* out) {
  Regs moved = *in;
  wr16(TEXT_CURSOR_X, in->x);
  wr16(TEXT_CURSOR_Y, in->y);
  moved.y = 0;
  call_text_print_tiles(&moved, out);
  out->x = in->x;
}

// ---- func_8AC0F2 ----  as text_print_tiles_at of its last line ($8A:C49B at (6, 26))
static void call_func_8AC0F2(const Regs* in, Regs* out) {
  out->y = func_8AC0F2();
  out->a = 0;
  out->x = 6;
  setNZ16(out, 0);
  if(out->y > 0) out->c = rom8(0x8AC49B + out->y - 1) >= 0x80;
  (void)in;
}

static void fuzz_text_print_tiles_at(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_text_print_tiles(r, ram, rnd);
  r->x = (uint16_t)(rnd() % 16);
  r->y = (uint16_t)(rnd() % 8);
}

// ---- step_toward ($8A:A264, $8A:A27D, $8A:A296) ----  X = entry, Y = steps; exits with
// Y = steps left, A = the last difference, Z = 1 and N = 0 (from the equal SBC or the
// final DEY), C/V of that last SBC (target - the value before the last step).
#define STEP_ADAPTER(fn, value, target) \
  static void call_##fn(const Regs* in, Regs* out) { \
    uint16_t diff; \
    out->y = step_toward((uint16_t)(value + in->x), (uint16_t)(target + in->x), in->y, &diff); \
    out->a = diff; \
    out->z = true; \
    out->n = false; \
    uint16_t t = rd16((uint16_t)(target + in->x)), prior = (uint16_t)(t - diff); \
    out->c = t >= prior; \
    out->v = ((t ^ prior) & (t ^ diff) & 0x8000) != 0; \
  } \
  static void fuzz_##fn(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { \
    r->x = (uint16_t)(2 * (rnd() % 4)); \
    uint16_t v = (uint16_t)rnd(); \
    wr16ram(ram, value + r->x, v); \
    wr16ram(ram, target + r->x, (uint16_t)(v + (int16_t)(rnd() % 25) - 12)); \
    r->y = (uint16_t)(1 + rnd() % 12); \
  }
STEP_ADAPTER(step_toward_0d80, 0x0D80, 0x0D88)
STEP_ADAPTER(step_toward_0d70, 0x0D70, 0x0D78)
STEP_ADAPTER(step_toward_0d68, 0x0D68, 0x0D90)

// ---- wait_frames ($80:8373, $8A:BA90 long) ----  A = frames; exits with A = the target,
// Z = C = 1 and N = 0 (the final equal CMP), V of the ADC that computed the target.
static void call_wait_frames(const Regs* in, Regs* out) {
  uint16_t now = rd16(FRAME_COUNTER);
  out->a = wait_frames(in->a);
  out->z = out->c = true;
  out->n = false;
  uint32_t sum = (uint32_t)in->a + now;
  out->v = (~(in->a ^ now) & (in->a ^ sum) & 0x8000) != 0;
}

// ---- text_print_centered ($8A:B6AA) ----  A = string address in bank DB. Exits with
// X kept (N/Z from the PLX), Y = the length, A = $0186 or func_8AB700's 4/0.
// C/V: those of the ADC that computes the start column ((width/2 - len/2) + left), which
// text_put_char's PHP/PLP preserve; when func_8AB700 runs, those of its SEC/SBC #10.
static void call_text_print_centered(const Regs* in, Regs* out) {
  text_print_centered(((uint32_t)in->db << 16) | in->a);
  out->y = rd16(DP + 0x5A);
  uint16_t f = rd16(DP + 0x86);
  out->a = f ? f : ((rd16(DP + 0x5A) & 1) ? 4 : 0);
  setNZ16(out, in->x);
  // (half width recomputed: text_put_char reuses $0142 as its palette scratch)
  uint16_t half = (uint16_t)((rd16(DP + 0x40) - rd16(DP + 0x3E)) >> 1);
  if(f) setAdc16(out, (uint16_t)(half - rd16(DP + 0x46)), rd16(DP + 0x3E));
  else setSbc16(out, rd16(DP + 0x3C), 10);
}

static void fuzz_text_print_centered(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->a = 0x1C00;  // a string in low WRAM, reached through DB = $8A's mirror
  int len = (int)(rnd() % 21);
  for(int i = 0; i < len; i++) ram[0x1C00 + i] = (uint8_t)(0x20 + rnd() % 0x3B);
  ram[0x1C00 + len] = 0;
  uint16_t left = (uint16_t)(rnd() % 8);
  wr16ram(ram, DP + 0x3E, left);
  wr16ram(ram, DP + 0x40, (uint16_t)(left + 8 + rnd() % 20));
  wr16ram(ram, DP + 0x3C, (uint16_t)(10 + rnd() % 6));
  ram[DP + 0x68] = (rnd() & 3) == 0;
  wr16ram(ram, DP + 0x86, (rnd() & 1) ? 0 : 1);
  wr16ram(ram, TEXT_FONT_SEL, (rnd() & 3) == 0 ? 0 : 1);
}

// ---- text_put_tile_char ($8A:C278) ----  8-bit char in A's low byte; PHP/PLP, PHX/PLX and
// PHY/PLY keep flags and X/Y; A = the tile written.
static void call_text_put_tile_char(const Regs* in, Regs* out) {
  out->a = text_put_tile_char((uint8_t)in->a);
}

static void fuzz_text_put_tile_char(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->a = (uint16_t)((r->a & 0xFF00) | (0x20 + rnd() % 0x3B));
  wr16ram(ram, DP + 0x3A, (uint16_t)(rnd() % 32));
  wr16ram(ram, DP + 0x3C, (uint16_t)(rnd() % 16));
  wr16ram(ram, DP + 0x90, (uint16_t)(rnd() & 1));
}

// ---- byte_buffer_push ($8A:9C19) ----  8-bit A; X kept; N/Z from the 8-bit INC $0160.
static void call_byte_buffer_push(const Regs* in, Regs* out) {
  byte_buffer_push((uint8_t)in->a);
  uint8_t c = rd8(DP + 0x60);
  out->n = (c & 0x80) != 0;
  out->z = c == 0;
}

// ---- byte_buffer_space ($8A:9A99) ----  A kept (N Z from its PLA); C V as on entry
static void call_byte_buffer_space(const Regs* in, Regs* out) {
  byte_buffer_space();
  setNZ16(out, in->a);
}

static void fuzz_byte_buffer_push(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x58, (uint16_t)(rnd() % 0x200));
  ram[DP + 0x60] = (uint8_t)rnd();
}

// ---- byte_buffer_pad ($8A:99B4) ----  A kept; N Z C of the final CMP (equal); V as on entry
static void call_byte_buffer_pad(const Regs* in, Regs* out) {
  byte_buffer_pad(in->a);
  out->n = false;
  out->z = out->c = true;
}

// a target 0-24 bytes ahead of the count, without wrapping its low byte
static void fuzz_byte_buffer_pad(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  wr16ram(ram, DP + 0x58, (uint16_t)(rnd() % 0x200));
  uint8_t count = (uint8_t)(rnd() % 0xE0);
  ram[DP + 0x60] = count;
  r->a = (uint16_t)(ram[DP + 0x61] << 8 | (count + rnd() % 25));
}

// ---- byte_buffer_end ($8A:99D6) ----  A = $80 in its low byte; N Z from the count byte
// after the push (byte_buffer_push), before it is reset; C V as on entry
static void call_byte_buffer_end(const Regs* in, Regs* out) {
  uint8_t count = (uint8_t)(rd8(DP + 0x60) + 1);
  byte_buffer_end();
  out->a = (uint16_t)((in->a & 0xFF00) | 0x80);
  out->n = count & 0x80;
  out->z = count == 0;
}

// ---- byte_buffer_decimal ($8A:9A33) ----  A X Y kept (N Z from the PLA); C = 1 (CMP #0,
// or the final CPX #10); V clear after the digit loop (its last SBC is 0 - 1), else as on entry
static void call_byte_buffer_decimal(const Regs* in, Regs* out) {
  byte_buffer_decimal(in->a);
  setNZ16(out, in->a);
  out->c = true;
  if(in->a != 0) out->v = false;
}

static void fuzz_byte_buffer_decimal(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_byte_buffer_push(r, ram, rnd);
  uint32_t k = rnd();
  r->a = (k & 7) == 0 ? 0 : (k & 7) == 1 ? 0xFFFF : (k & 7) < 4 ? (uint16_t)(rnd() % 100) : (uint16_t)rnd();
}

// ---- func_80E4C5 / func_80E4DB (long) ----  A kept, N Z from its PLA
static void call_func_80E4C5(const Regs* in, Regs* out) { func_80E4C5(); setNZ16(out, in->a); }
static void call_func_80E4DB(const Regs* in, Regs* out) { func_80E4DB(); setNZ16(out, in->a); }

// ---- func_80B58D ----  A = 9 (N Z from the LDA)
static void call_func_80B58D(const Regs* in, Regs* out) {
  func_80B58D((uint8_t)in->a, in->x);
  out->a = 9;
  setNZ16(out, 9);
}

// ---- func_8ABFBC ----  A = the result, C = lsr_n's carry, N Z from the PLY (Y kept)
static void call_func_8ABFBC(const Regs* in, Regs* out) {
  bool carry;
  out->a = func_8ABFBC(in->a, &carry);
  out->c = carry;
  setNZ16(out, in->y);
}

// ---- bcd_to_binary ($8A:BF9E) ----  A as it leaves it, X = 10, Y = 0, N Z C of the final
// CPX #10 (equal); V of the last ADC of a weight (the digits are the nibbles of $0144 and
// the low nibble of $0146), or as on entry if every digit was 0.
// bcd_to_binary's last weight ADC: its C and V into `out` (left alone if every digit is 0)
static void bcd_to_binary_cv(Regs* out, uint16_t lo, uint16_t hi) {
  uint16_t sum = 0;
  for(int i = 0; i < 5; i++) {
    uint16_t digit = i < 4 ? (lo >> (4 * i)) & 0xF : hi & 0xF, w = rom16(0x8ACABA + 2 * i);
    for(; digit != 0; digit--) {
      setAdc16(out, sum, w);
      sum = (uint16_t)(sum + w);
    }
  }
}

static void call_bcd_to_binary(const Regs* in, Regs* out) {
  bcd_to_binary_cv(out, rd16(DP + 0x44), rd16(DP + 0x46));
  out->a = bcd_to_binary(in->a);
  out->x = 10;
  out->y = 0;
  out->n = false;
  out->z = out->c = true;
}

// ---- func_80899F ----  A = 0 (Z) when the byte is 0, else A = X = the new index (N Z from
// its AND #7); C V as on entry
static void call_func_80899F(const Regs* in, Regs* out) {
  uint8_t b = (uint8_t)in->a;
  out->x = func_80899F(b, in->x);
  out->a = b ? out->x : 0;
  setNZ16(out, out->a);
}

static void fuzz_func_80899F(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)ram;
  uint32_t k = rnd();
  r->x = (k & 3) ? (uint16_t)(rnd() & 7) : (uint16_t)(rnd() % 0x4C0);
  if((k & 0x30) == 0) r->a &= 0xFF00;
}

// ---- func_8AA022 ----  A = the new $0158, N Z C V of its ADC
static void call_func_8AA022(const Regs* in, Regs* out) {
  uint16_t old = rd16(DP + 0x58);
  out->a = func_8AA022(in->a);
  setNZ16(out, out->a);
  setAdc16(out, (uint16_t)((in->a ^ 1) & 1), old);
}

static void fuzz_func_8AA022(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x56, (uint16_t)rnd());
  wr16ram(ram, DP + 0x58, (rnd() & 1) ? 0xFFFF : (uint16_t)rnd());
}

// ---- func_80B7BB ----  A = $01DD, X = 0, Y = $FFFF, N Z of the last DEX (zero), C of the
// last CMP (with $01CF, before it is cleared); V as on entry
static void call_func_80B7BB(const Regs* in, Regs* out) {
  (void)in;
  uint16_t v = rd16(DP + 0xDD), cf = rd16(DP + 0xCF);
  func_80B7BB();
  out->a = v;
  out->x = 0;
  out->y = 0xFFFF;
  out->n = false;
  out->z = true;
  out->c = v >= cf;
}

// words drawn from a few values so that some equal $01DD
static void fuzz_func_80B7BB(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint16_t pick[3] = {(uint16_t)rnd(), (uint16_t)rnd(), 0xFFFF};
  for(uint16_t a = 0xCF; a <= 0xDD; a = (uint16_t)(a + 2)) wr16ram(ram, DP + a, pick[rnd() % 3]);
}

// ---- byte_buffer_decimal2 ($8A:9A81) ----  as byte_buffer_decimal of $015E (A = it)
static void call_byte_buffer_decimal2(const Regs* in, Regs* out) {
  uint16_t v = rd16(DP + 0x5E);
  byte_buffer_decimal2();
  out->a = v;
  setNZ16(out, v);
  out->c = true;
  if(v != 0) out->v = false;
  (void)in;
}

static void fuzz_byte_buffer_decimal2(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_byte_buffer_push(r, ram, rnd);
  uint32_t k = rnd();
  uint16_t v = (k & 3) == 0 ? (uint16_t)(rnd() % 10) : (k & 3) == 1 ? (uint16_t)(0x8000 + rnd() % 12) : (uint16_t)rnd();
  wr16ram(ram, DP + 0x5E, (k & 4) ? (uint16_t)(rnd() % 100) : v);
}

// ---- func_80894E ----  ring empty: A = $00FF (the TRB mask), X = the write index, N Z C
// of the CPX (equal); else A = X = the new read index (N Z from its AND #7), C of the CPX
// (write index >= read index). V as on entry.
static void call_func_80894E(const Regs* in, Regs* out) {
  (void)in;
  uint16_t r = rd16(0x1B26);
  func_80894E();
  uint16_t w = rd16(0x1B24);
  out->n = false;
  if(w == r) {
    out->a = 0x00FF;
    out->x = w;
    out->z = out->c = true;
  } else {
    out->a = out->x = (uint16_t)((r + 1) & 7);
    out->z = out->a == 0;
    out->c = w >= r;
  }
}

static void fuzz_func_80894E(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  static const uint16_t sources[4] = {0x01FB, 0x01FC, 0x02FB, 0x02FC};
  for(int i = 0; i < 4; i++) ram[sources[i]] = (rnd() & 1) ? 0 : (uint8_t)rnd();
  wr16ram(ram, 0x1B24, (uint16_t)(rnd() & 7));
  wr16ram(ram, 0x1B26, (rnd() & 1) ? (uint16_t)(rnd() & 7) : ram[0x1B24]);
}

// ---- binary_to_bcd ($80:86C9) ----  A = the low word (N Z from its LDA), X = the high
// word, C = 0 (the last ROR shifts out a bit of the zeros it started with), V as on entry
static void call_binary_to_bcd(const Regs* in, Regs* out) {
  uint32_t bcd = binary_to_bcd(in->a);
  out->a = (uint16_t)bcd;
  out->x = (uint16_t)(bcd >> 16);
  setNZ16(out, out->a);
  out->c = false;
}

static void fuzz_binary_to_bcd(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)ram;
  uint32_t k = rnd();
  if((k & 3) == 0) r->a = (uint16_t)(rnd() % 100);
  if((k & 0xC) == 0) r->a = 0xFFFF;
}

// ---- func_8AA005 ----  as func_8AA022 after the last of its four calls
static void call_func_8AA005(const Regs* in, Regs* out) {
  (void)in;
  uint16_t b[4] = {rd16(0x0018), rd16(0x0014), rd16(0x0010), rd16(0x000C)};
  uint16_t before = 0;
  for(int i = 0; i < 3; i++) before = (uint16_t)(before + ((b[i] ^ 1) & 1));
  out->a = func_8AA005();
  setNZ16(out, out->a);
  setAdc16(out, (uint16_t)((b[3] ^ 1) & 1), before);
}

// ---- func_80E0C8 ----  A = the result, N Z C V of its ADC (the carry in from the CMP)
static void call_func_80E0C8(const Regs* in, Regs* out) {
  uint16_t d = (uint16_t)(in->a << 1);
  out->a = func_80E0C8(in->a);
  setNZ16(out, out->a);
  if(d < 0x40) setAdc16(out, d, 0xB990);
  else {
    uint32_t sum = (uint32_t)d + 0x23FF + 1;
    out->c = sum > 0xFFFF;
    out->v = (~(d ^ 0x23FF) & (d ^ sum) & 0x8000) != 0;
  }
}

static void fuzz_func_80E0C8(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)ram;
  if(rnd() & 1) r->a = (uint16_t)(rnd() % 0x40);
}

// ---- oam_clear ($80:83E8) ----  A = $E0E0, X = 0 (N Z from the last DEX); C V as on entry
static void call_oam_clear(const Regs* in, Regs* out) {
  (void)in;
  oam_clear();
  out->a = 0xE0E0;
  out->x = 0;
  out->n = false;
  out->z = true;
}

// $80:FBD8: JSR oam_clear, RTL
static void call_oam_clear_long(const Regs* in, Regs* out) {
  oam_clear_long();
  out->a = 0xE0E0;
  out->x = 0;
  out->n = false;
  out->z = true;
  (void)in;
}

// ---- wait_flag_76 ($80:B950) ----  A = 8; N V Z of the BIT that saw bit 3 (bits 15, 14 of
// D+$76, not zero); C as on entry
static void call_wait_flag_76(const Regs* in, Regs* out) {
  uint16_t v = wait_flag_76(in->d);
  out->a = 0x0008;
  out->n = v >> 15;
  out->v = (v >> 14) & 1;
  out->z = false;
}

// bit 3 already set (with interrupts off nothing else would set it)
static void fuzz_wait_flag_76(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  wr16ram(ram, (uint16_t)(r->d + 0x76), (uint16_t)(rnd() | 0x0008));
}

// ---- func_808FFC ----  A = the new D+$74, X = the old one, N Z C V of its ADC #8
static void call_func_808FFC(const Regs* in, Regs* out) {
  (void)in;
  uint16_t x = rd16(DP + 0x74);
  out->a = func_808FFC();
  out->x = x;
  setNZ16(out, out->a);
  setAdc16(out, x, 8);
}

static void fuzz_func_808FFC(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x74, (rnd() & 7) ? (uint16_t)(4 * (rnd() % 126)) : (uint16_t)(0xFFF8 + (rnd() & 7)));
  wr16ram(ram, DP + 0xEE, (rnd() & 1) ? 0 : (uint16_t)rnd());
  wr16ram(ram, DP + 0xE7, (uint16_t)rnd());
}

// ---- wait_one_frame ($8A:B212) ----  A kept (N Z from its PLA), C V of wait_frames(1)
static void call_wait_one_frame(const Regs* in, Regs* out) {
  uint16_t now = rd16(FRAME_COUNTER);
  wait_one_frame();
  setNZ16(out, in->a);
  out->c = true;
  uint32_t sum = 1u + now;
  out->v = (~(1u ^ now) & (1u ^ sum) & 0x8000) != 0;
}

// ---- func_8A8232 ----  A = the result, N Z C V of its ADC
static void call_func_8A8232(const Regs* in, Regs* out) {
  (void)in;
  uint16_t q = (uint16_t)(rd16(DP + 0x36) >> 2);
  out->a = func_8A8232();
  setNZ16(out, out->a);
  setAdc16(out, q, 0x0108);
}

static void fuzz_func_8A8232(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x36, (uint16_t)rnd());
}

// ---- func_82AD12 ----  registers kept; N V Z of the BIT on $047C
static void call_func_82AD12(const Regs* in, Regs* out) {
  uint16_t m = rd16(0x047C);
  func_82AD12(in->a, in->x);
  out->n = m >> 15;
  out->v = (m >> 14) & 1;
  out->z = (in->a & m) == 0;
}

static void fuzz_func_82AD12(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x047C, (uint16_t)rnd());
}

// ---- binary_to_bcd5 ($8A:BFD0) ----  A = $FFFF (the last SBC, 0 - 1), X = $FFFE, Y = 0,
// N Z of the last DEY (zero), C = 0 (the last ROR takes a bit of the low nibble's zeros),
// V = 0 (of that SBC)
static void call_binary_to_bcd5(const Regs* in, Regs* out) {
  (void)in;
  binary_to_bcd5();
  out->a = 0xFFFF;
  out->x = 0xFFFE;
  out->y = 0;
  out->n = false;
  out->z = true;
  out->c = out->v = false;
}

static void fuzz_binary_to_bcd5(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint32_t k = rnd();
  wr16ram(ram, DP + 0x42, (k & 3) == 0 ? (uint16_t)(rnd() % 100) : (k & 3) == 1 ? 0xFFFF : (uint16_t)rnd());
}

// ---- func_82F380 / func_82FB1C ----  RTS: nothing changes
static void call_func_82F380(const Regs* in, Regs* out) { (void)in; (void)out; func_82F380(); }
static void call_func_82FB1C(const Regs* in, Regs* out) { (void)in; (void)out; func_82FB1C(); }

// ---- func_8AC03D / func_8AC024 ----  Y = 0, N Z of the last DEY, C = the last bit out
static void call_func_8AC03D(const Regs* in, Regs* out) {
  (void)in;
  out->c = func_8AC03D();
  out->y = 0;
  out->n = false;
  out->z = true;
}

static void call_func_8AC024(const Regs* in, Regs* out) {
  (void)in;
  out->c = func_8AC024();
  out->y = 0;
  out->n = false;
  out->z = true;
}

// ---- func_8AA32B ----  A = the byte stored (A's high byte 0 from its LDA #0), Y kept (N Z
// from its PLY), C V of the 8-bit SBC #10 on D+$3C
static void call_func_8AA32B(const Regs* in, Regs* out) {
  uint8_t d = rd8(DP + 0x3C), r = (uint8_t)(d - 10);
  out->a = func_8AA32B(in->x);
  setNZ16(out, in->y);
  out->c = d >= 10;
  out->v = ((d ^ 10) & (d ^ r) & 0x80) != 0;
}

// all three flag cases, any D+$3C and D+$6E, an object index as the game's (a word index)
static void fuzz_func_8AA32B(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)(2 * (rnd() % 32));
  uint32_t k = rnd();
  ram[(uint16_t)(0x1516 + r->x)] = (uint8_t)((k & 3) == 0 ? 0x80 | rnd() : (k & 3) == 1 ? rnd() & 0x3F : 0x40 | (rnd() & 0x3F));
  ram[DP + 0x3C] = (uint8_t)rnd();
  ram[DP + 0x6E] = (uint8_t)rnd();
}

// ---- func_8AA381 ----  nothing found: A = $0400, N = 0, Z = 1. Bit 10: Y kept (N Z from the
// PLY), A = the word after the call ($1518 + x) or 0 when it was skipped. Bit 11: A = D+$4A
// (N Z from the PLA), C = whether the word before it was $FFFF (CMP #$FFFF). C V otherwise
// as on entry.
static void call_func_8AA381(const Regs* in, Regs* out) {
  uint16_t p = rd16(DP + 0x4A);
  uint16_t before = bus_rd16(0x8A0000 | (uint16_t)(p - 2)), after = bus_rd16((0x8A0000 | p) + 8);
  int bit = func_8AA381(in->x);
  if(bit == 0) {
    out->a = 0x0400;
    out->n = false;
    out->z = true;
  } else if(bit == 10) {
    out->a = after != 0 ? rd16((uint16_t)(0x1518 + in->x)) : 0;
    setNZ16(out, in->y);
  } else {
    out->a = p;
    setNZ16(out, p);
    out->c = before == 0xFFFF;
  }
}

// the four words with bits 10/11 sometimes set; D+$4A pointing into bank $8A's low-WRAM
// mirror at $1C02, with the word before it sometimes $FFFF and the one 8 after sometimes 0
static void fuzz_func_8AA381(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  static const uint16_t words[4] = {0x000E, 0x0012, 0x0016, 0x001A};
  uint32_t k = rnd();
  for(int i = 0; i < 4; i++) {
    uint16_t w = (uint16_t)(rnd() & ~0x0C00);
    if((k >> (2 * i)) % 4 == 0) w |= 0x0800;
    if((k >> (2 * i + 8)) % 4 == 0) w |= 0x0400;
    wr16ram(ram, words[i], w);
  }
  wr16ram(ram, DP + 0x4A, 0x1C02);
  wr16ram(ram, 0x1C00, (rnd() & 1) ? 0xFFFF : (uint16_t)rnd());
  wr16ram(ram, 0x1C0A, (rnd() & 1) ? 0 : (uint16_t)rnd());
  r->x = (uint16_t)(2 * (rnd() % 32));
}

// ---- func_8A8290 ----  D+$54 = 0: A = 0 (N Z of its LDA). Count not a multiple of 8: A =
// its low 3 bits (N Z of the AND). Else A = the word, X = the new index, N Z C of the
// CPX #12 (or of the LDX #0 after it, C = 1 from the CPX). V as on entry.
static void call_func_8A8290(const Regs* in, Regs* out) {
  (void)in;
  uint16_t on = rd16(DP + 0x54), count = (uint16_t)(rd16(DP + 0x52) + 1), x = (uint16_t)(rd16(DP + 0x50) + 2);
  out->a = func_8A8290();
  if(on == 0 || (count & 7)) {
    setNZ16(out, out->a);
    return;
  }
  if(x == 12) {
    out->x = 0;
    out->n = false;
    out->z = out->c = true;
  } else {
    out->x = x;
    setNZ16(out, (uint16_t)(x - 12));
    out->c = x >= 12;
  }
}

// D+$54 on and off, the count often one short of a multiple of 8, the index in the cycle
static void fuzz_func_8A8290(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint32_t k = rnd();
  wr16ram(ram, DP + 0x54, (k & 3) == 0 ? 0 : (uint16_t)rnd());
  wr16ram(ram, DP + 0x52, (k & 4) ? (uint16_t)(8 * rnd() + 7) : (uint16_t)rnd());
  wr16ram(ram, DP + 0x50, (k & 0x18) ? (uint16_t)(2 * (rnd() % 6)) : (uint16_t)(rnd() % 0x100));
}

// ---- func_85CDAF (long) ----  Y = the entry index, N Z C of the CPY that ended it
// (equal). A = the walk's last index (= Y), or entry y's $F733 word when it did not run.
static void call_func_85CDAF(const Regs* in, Regs* out) {
  uint16_t b = rd16((uint16_t)(0xF733 + in->y));
  out->a = func_85CDAF(in->y) ? in->y : b;
  out->n = false;
  out->z = out->c = true;
}

// an even entry index below $100 and a start index, the tables drawn from a few values so
// that copies occur
static void fuzz_func_85CDAF(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->y = (uint16_t)(2 * (rnd() % 128));
  wr16ram(ram, 0x00FE, (rnd() & 7) ? (uint16_t)(2 * (rnd() % 128)) : r->y);
  uint16_t pick[2][3] = {{(uint16_t)rnd(), (uint16_t)rnd(), (uint16_t)rnd()}, {(uint16_t)rnd(), (uint16_t)rnd(), (uint16_t)rnd()}};
  for(uint16_t i = 0; i < 0x100; i = (uint16_t)(i + 2)) {
    wr16ram(ram, (uint32_t)(0xF533 + i), pick[0][rnd() % 3]);
    wr16ram(ram, (uint32_t)(0xF733 + i), pick[1][rnd() % 3]);
  }
}

// ---- func_8A869A / func_8A823D / func_8A8325 ----  A = the (last) result, N Z C V of its ADC
static void call_func_8A869A(const Regs* in, Regs* out) {
  (void)in;
  uint16_t q = (uint16_t)(-(rd16(DP + 0x3C) >> 4));
  out->a = func_8A869A();
  setNZ16(out, out->a);
  setAdc16(out, q, 0x2A);
}

static void call_func_8A823D(const Regs* in, Regs* out) {
  (void)in;
  uint16_t t = (uint16_t)((rd16(DP + 0x38) >> 2) + 0xFFB7), m = rd16(DP + 0x64);
  out->a = func_8A823D();
  setNZ16(out, out->a);
  setAdc16(out, t, m);
}

static void call_func_8A8325(const Regs* in, Regs* out) {
  (void)in;
  uint16_t q = (uint16_t)(rd16(DP + 0x3E) >> 2);
  out->a = func_8A8325();
  setNZ16(out, out->a);
  setAdc16(out, q, 0x1B);
}

static void fuzz_dp_words(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  static const uint8_t words[5] = {0x38, 0x3C, 0x3E, 0x64, 0x00};
  for(int i = 0; i < 4; i++) wr16ram(ram, DP + words[i], (uint16_t)rnd());
}

// ---- func_80B556 ----  A = $19 (N Z of its LDA)
static void call_func_80B556(const Regs* in, Regs* out) {
  func_80B556((uint8_t)in->a, in->x, in->y);
  out->a = 0x0019;
  setNZ16(out, 0x0019);
}

// ---- func_8ABF76 ----  as func_8AC03D after it: Y = 0, N = 0, Z = 1, C = its carry; A =
// the second number's high word, X = $FFFE (binary_to_bcd5), V = 0 (binary_to_bcd5's SBC)
static void call_func_8ABF76(const Regs* in, Regs* out) {
  (void)in;
  out->c = func_8ABF76();
  out->a = rd16(DP + 0x46);
  out->x = 0xFFFE;
  out->y = 0;
  out->n = out->v = false;
  out->z = true;
}

// ---- func_8ABF4E ----  A = the second result (N Z of its LDA), X = 10, Y = 0, C = 1 (the
// last CPX), V of the last weight ADC of either conversion (else as on entry)
static void call_func_8ABF4E(const Regs* in, Regs* out) {
  (void)in;
  uint32_t a = (uint32_t)rd16(0x0390) << 16 | rd16(0x038E), b = (uint32_t)rd16(0x03BF) << 16 | rd16(0x03BD);
  a >>= 12, b >>= 12;  // func_8AC024 (the bits above 32 are zero)
  bcd_to_binary_cv(out, (uint16_t)a, (uint16_t)(a >> 16));
  bcd_to_binary_cv(out, (uint16_t)b, (uint16_t)(b >> 16));
  out->a = func_8ABF4E();
  setNZ16(out, out->a);
  out->x = 10;
  out->y = 0;
  out->c = true;
}

static void fuzz_func_8ABF4E(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  static const uint16_t words[4] = {0x038E, 0x0390, 0x03BD, 0x03BF};
  for(int i = 0; i < 4; i++) wr16ram(ram, words[i], (rnd() & 3) ? (uint16_t)rnd() : 0);
}

// ---- func_8A9202 ----  X = x + $14, A = D+$7C (N Z of its PLA), C V of the ADC #$14
static void call_func_8A9202(const Regs* in, Regs* out) {
  uint16_t y = rd16(DP + 0x7C);
  out->x = func_8A9202(in->x);
  out->a = y;
  setNZ16(out, y);
  setAdc16(out, in->x, 0x14);
}

static void fuzz_func_8A9202(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)(0x14 * (rnd() % 8));
  wr16ram(ram, DP + 0x7A, (rnd() & 3) ? (uint16_t)rnd() : (uint16_t)(0xFFC0 + rnd() % 0x40));
  wr16ram(ram, DP + 0x7C, (uint16_t)rnd());
}

// ---- func_80D6F3 ----  A = the result (N Z of its last LDA/INC), C of the CMP |d_y| vs
// |d_x|, V of the SBC D+$36
static void call_func_80D6F3(const Regs* in, Regs* out) {
  uint16_t dx = (uint16_t)(((in->a & 0xFF) << 8) - rd16(DP + 0x34));
  uint16_t ax = (dx & 0x8000) ? (uint16_t)-dx : dx;
  uint16_t hy = (uint16_t)(in->a & 0xFF00), y0 = rd16(DP + 0x36), dy = (uint16_t)(hy - y0);
  uint16_t ay = (dy & 0x8000) ? (uint16_t)-dy : dy;
  out->a = func_80D6F3(in->a);
  setNZ16(out, out->a);
  out->c = ay >= ax;
  out->v = ((hy ^ y0) & (hy ^ dy) & 0x8000) != 0;
}

// both signs of each difference, the two axes either way, and small longer axes (a zero
// divisor)
static void fuzz_func_80D6F3(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint32_t k = rnd();
  uint16_t x = (uint16_t)rnd(), y = (uint16_t)rnd();
  wr16ram(ram, DP + 0x34, x);
  wr16ram(ram, DP + 0x36, y);
  if((k & 3) == 0) r->a = (uint16_t)(((y + (rnd() % 0x200) - 0x100) & 0xFF00) | ((x >> 8) + rnd() % 3 - 1) % 256);
}

// ---- scroll_tables_build ($85:BC20, long) ----  A = the last value stored (N Z C V of its
// SBC #$70), Y = the new $00E0, X = $0168
static void call_scroll_tables_build(const Regs* in, Regs* out) {
  (void)in;
  uint16_t t = (uint16_t)(0xBB - rd16(0x0237));
  out->a = scroll_tables_build();
  out->y = rd16(0x00E0);
  out->x = rd16(0x0168);
  setNZ16(out, out->a);
  setSbc16(out, t, 0x70);
}

// buffers in low WRAM away from the direct page, both signs of $0242/$1EF2 and of the two
// $0237 bands, zero height bytes
static void fuzz_scroll_tables_build(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint32_t k = rnd();
  wr16ram(ram, 0x00E0, (uint16_t)(0x20 * (rnd() % 4)));
  wr16ram(ram, 0x00E4, (uint16_t)(0x20 * (rnd() % 4)));
  static const uint16_t heads[6] = {0x1E2E, 0x1E32, 0x1E36, 0x1E30, 0x1E34, 0x1E38};
  for(int i = 0; i < 6; i++) wr16ram(ram, heads[i], (uint16_t)(0x0600 + 0x40 * i + 0x30 + rnd() % 8));
  wr16ram(ram, 0x0242, (k & 1) ? (uint16_t)-(int)(rnd() % 0x120) : (uint16_t)(rnd() & 0x7FFF));
  wr16ram(ram, 0x1EF2, (k & 2) ? (uint16_t)-(int)(rnd() % 0x120) : (uint16_t)(rnd() & 0x7FFF));
  uint16_t i1 = (uint16_t)(rnd() % 0x40), i2 = (uint16_t)(rnd() % 0x40);
  wr16ram(ram, 0x0164, i1);
  wr16ram(ram, 0x0168, i2);
  // a height byte of 0 (counts as $DC), 1-3 (goes negative, then 0) or anything
  uint32_t h = rnd();
  ram[0x4CFE + i1] = (h & 3) == 0 ? 0 : (h & 3) == 1 ? (uint8_t)(1 + rnd() % 3) : (uint8_t)rnd();
  ram[0x4D4A + i2] = (h & 12) == 0 ? 0 : (h & 12) == 4 ? (uint8_t)(1 + rnd() % 3) : (uint8_t)rnd();
  wr16ram(ram, 0x0237, (k & 0x30) == 0 ? (uint16_t)(0xFF00 + rnd() % 0x100) : (uint16_t)(rnd() % 0x100 - 0x40));
  wr16ram(ram, 0x1E3E, (uint16_t)rnd());
  wr16ram(ram, 0x1E40, (uint16_t)rnd());
}

// ---- sound_fade_step ($86:DFAE) ----  A = high byte of 2 * fade : the new volume; N Z of
// the ADC (or of the LDA #$7F / #$00 that clamps it), C V of the 8-bit ADC
static void call_sound_fade_step(const Regs* in, Regs* out) {
  (void)in;
  uint16_t fade = rd16(SOUND_FADE);
  uint8_t x = (uint8_t)(fade << 1), m = rd8(SOUND_VOLUME);
  uint16_t sum = (uint16_t)(x + m);
  uint8_t r = (uint8_t)sum;
  uint8_t v = sound_fade_step();
  out->a = (uint16_t)(((uint16_t)(fade << 1) & 0xFF00) | v);
  out->n = false;
  out->z = v == 0;
  out->c = sum > 0xFF;
  out->v = (~(x ^ m) & (x ^ r) & 0x80) != 0;
}

static void fuzz_sound_fade_step(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint32_t k = rnd();
  wr16ram(ram, SOUND_FADE, (k & 3) == 0 ? 0 : (k & 4) ? (uint16_t)(rnd() % 8) : (uint16_t)-(int)(rnd() % 8));
  ram[SOUND_VOLUME] = (uint8_t)(rnd() & 0x7F);
}

// ---- sound_send_message ($86:DFFB) ----  PHP/PLP: flags as on entry; A = its high byte :
// $FF, X = byte $0469, Y = byte $046B (8-bit loads under SEP #$30)
static void call_sound_send_message(const Regs* in, Regs* out) {
  uint8_t x = rd8(0x0469), y = rd8(0x046B);
  sound_send_message();
  out->a = (uint16_t)((in->a & 0xFF00) | 0xFF);
  out->x = x;
  out->y = y;
}

// ---- sound_frame ($86:DFE1, long) ----  PHP/PLP: flags as on entry. A = the word $046D;
// with bit 15 set, its high byte : $77 (the readiness test), or : $FF and X, Y as
// sound_send_message when it sent
static void call_sound_frame(const Regs* in, Regs* out) {
  (void)in;
  uint16_t en = rd16(SOUND_ENABLE);
  uint8_t x = rd8(0x0469), y = rd8(0x046B);
  bool sent = sound_frame();
  out->a = !(en & 0x8000) ? en : (uint16_t)((en & 0xFF00) | (sent ? 0xFF : 0x77));
  if(sent) { out->x = x; out->y = y; }
}

// Negative test: the message with the two request groups swapped (the APU effects must
// see it).
static void call_sound_send_message_wrong(const Regs* in, Regs* out) {
  uint8_t msg[16] = {
    rd8(0x045C), rd8(0x045D), rd8(0x045F), (uint8_t)((video_is_pal() ? 0x02 : 0) | rd8(SOUND_FLAGS) | 0x80),
    rd8(0x0461), rd8(0x045E), rd8(0x0460), (uint8_t)(rd8(0x0462) & 0x7F),
    rd8(0x0469), rd8(0x046C), rd8(0x046B), (uint8_t)(rd8(SOUND_REQUEST_B) | 0x80),
    rd8(0x0464), rd8(0x0467), rd8(0x0466), (uint8_t)(rd8(SOUND_REQUEST_A) & 0x7F),
  };
  uint8_t x = rd8(0x0469), y = rd8(0x046B);
  sound_send_frame(msg);
  wr8(SOUND_REQUEST_A, 0);
  wr8(SOUND_REQUEST_B, 0);
  wr8(0x045F, 0xFF);
  out->a = (uint16_t)((in->a & 0xFF00) | 0xFF);
  out->x = x;
  out->y = y;
}

// ---- pads_read_pair ($83:87C1, 8-bit A) ----  A = its high byte : $0022's low byte (the
// last ORA, N Z from it), C = 0 (ROL of 0), X = A's high byte : (17th read ^ 3) >> 2 (TAX
// with 16-bit index registers takes A's high byte too)
static void call_pads_read_pair(const Regs* in, Regs* out) {
  pads_read_pair(0);
  uint8_t lo = rd8(0x0022);
  int tail = difftest_input_peek(0x4017, 16);
  out->a = (uint16_t)((in->a & 0xFF00) | lo);
  out->x = (uint16_t)((in->a & 0xFF00) | ((uint8_t)((tail < 0 ? 0 : tail) ^ 3) >> 2));
  out->n = lo & 0x80;
  out->z = lo == 0;
  out->c = false;
}

// ---- pads_read ($83:86FF) / pads_read_long ($83:86FB) ----  A = $001C, or 1 if that pad
// read as absent (N Z: of the LDA $1C with Z = 1 from the BIT, or of the LDA #1); X = the
// last pair's second pad (its TAX); C = 0 (the pair read's last ROL); V below
// V: bit 6 of $4213, from the 16-bit BIT $4212 that waited for the automatic read (the
// last value read; nothing after changes V)
static void pads_read_v(Regs* out) {
  int last = -1;
  for(int n = 0, v; (v = difftest_input_peek(0x4213, n)) >= 0; n++) last = v;
  if(last >= 0) out->v = (last >> 6) & 1;
}

static void pads_read_outputs(Regs* out) {
  pads_read_v(out);
  uint16_t d = rd16(0x001C);
  out->x = rd16(0x0022);
  if(d == 1 && rd16(0x001E) == 1 && (rd16(0x0022) & 0xF)) {
    out->a = 1;
    out->n = out->z = false;
  } else {
    out->a = d;
    out->n = d >> 15;
    out->z = true;
  }
  out->c = false;
}

// Negative test: pads_read with the multitap pairs taken in the other order.
static void call_pads_read_wrong(const Regs* in, Regs* out) {
  (void)in;
  uint16_t old[10];
  for(int i = 0; i < 10; i++) old[i] = rd16((uint16_t)(0x000C + 2 * i));
  pads_read();
  for(int i = 2; i < 6; i++) wr16((uint16_t)(0x000C + 2 * i), old[i]);  // drop pads A and B's update
  pads_read_outputs(out);
}

static void call_pads_read(const Regs* in, Regs* out) { (void)in; pads_read(); pads_read_outputs(out); }
static void call_pads_read_long(const Regs* in, Regs* out) { (void)in; pads_read_long(); pads_read_outputs(out); }
static void call_pads_read_long_2(const Regs* in, Regs* out) { (void)in; pads_read_long_2(); pads_read_outputs(out); }

static void fuzz_pads(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(uint16_t a = 0x000C; a < 0x0024; a++) ram[a] = (uint8_t)rnd();
}

// Interrupt handlers return with RTI: P comes back from the interrupt frame (at SP + 1 on
// entry), with its M and X widths; the registers they save they restore.
static void restorePAt(const Regs* in, Regs* out, unsigned frame) {
  uint8_t p = rd8((uint16_t)(in->sp + frame + 1u));
  out->n = p & 0x80; out->v = p & 0x40; out->z = p & 0x02; out->c = p & 0x01;
  out->m8 = p & 0x20; out->x8 = p & 0x10;
  if(out->x8) { out->x &= 0xFF; out->y &= 0xFF; }  // 8-bit index registers drop their high bytes
}

static void restoreP(const Regs* in, Regs* out) { restorePAt(in, out, 0); }

// The common NMI prologue's saves, above which the interrupt frame sits: D, Y, X, A (words,
// last pushed lowest), DB; the tail pulls them back before its RTI.
static void restoreNmiFrame(const Regs* in, Regs* out) {
  uint16_t sp = in->sp;
  out->d = rd16((uint16_t)(sp + 1));
  out->y = rd16((uint16_t)(sp + 3));
  out->x = rd16((uint16_t)(sp + 5));
  out->a = rd16((uint16_t)(sp + 7));
  out->db = rd8((uint16_t)(sp + 9));
  restorePAt(in, out, 9);
}

// Negative test: the tail without counting the frame.
static void call_nmi_tail_wrong(const Regs* in, Regs* out) {
  uint16_t f = rd16(FRAME_COUNTER);
  nmi_tail();
  wr16(FRAME_COUNTER, f);
  restoreNmiFrame(in, out);
}

// ---- nmi_target_80FBD1 / nmi_target_80FC5F ----  as nmi_tail
static void call_nmi_target_80FBD1(const Regs* in, Regs* out) { nmi_target_80FBD1(); restoreNmiFrame(in, out); }
static void call_nmi_target_80FC5F(const Regs* in, Regs* out) { nmi_target_80FC5F(); restoreNmiFrame(in, out); }
static void call_nmi_target_80E28C(const Regs* in, Regs* out) { nmi_target_80E28C(); restoreNmiFrame(in, out); }
static void call_nmi_target_80E175(const Regs* in, Regs* out) { nmi_target_80E175(); restoreNmiFrame(in, out); }
static void call_nmi_target_80D784(const Regs* in, Regs* out) { nmi_target_80D784(); restoreNmiFrame(in, out); }

// ---- race_upload_queue_drain ($83:8644) / func_838989 ($83:8989) and their long forms ----
// no register outputs: the race NMI overwrites A, X and Y after both
// (after a flag word the original's PEA/PLB pair leaves DB = $83)
static void call_race_upload_queue_drain(const Regs* in, Regs* out) { (void)in; if(race_upload_queue_drain()) out->db = 0x83; }
static void call_race_upload_queue_drain_long(const Regs* in, Regs* out) { (void)in; if(race_upload_queue_drain_long()) out->db = 0x83; }
static void call_func_838989(const Regs* in, Regs* out) { (void)in; (void)out; func_838989(); }
static void call_func_838989_long(const Regs* in, Regs* out) { (void)in; (void)out; func_838989_long(); }

// ---- fade_in_start ($8A:BAF2) / hdma_window_tables_init ($8A:A09F) ----  A ends as the
// last value loaded ($0080; B:$E0 in 8-bit), its flags
static void call_fade_in_start(const Regs* in, Regs* out) {
  (void)in;
  fade_in_start();
  out->a = 0x0080;
  out->n = false;
  out->z = false;
}
static void call_hdma_window_tables_init(const Regs* in, Regs* out) {
  hdma_window_tables_init();
  out->a = (uint16_t)((in->a & 0xFF00) | 0xE0);
  out->n = true;
  out->z = false;
}

// ---- video_load_screen ($80:81CC) ----  A = image bank (low byte), X = image address; A's
// low byte ends as the image's last byte; PLB leaves N and Z from the caller's DB
static void call_video_load_screen(const Regs* in, Regs* out) {
  uint32_t image = (uint32_t)(in->a & 0xFF) << 16 | in->x;
  video_load_screen(image);
  out->a = (uint16_t)((in->a & 0xFF00) | bus_rd8(image + 0x43));
  out->n = in->db & 0x80;
  out->z = in->db == 0;
}

static void call_video_load_screen_long(const Regs* in, Regs* out) {
  uint32_t image = (uint32_t)(in->a & 0xFF) << 16 | in->x;
  video_load_screen_long(image);
  out->a = (uint16_t)((in->a & 0xFF00) | bus_rd8(image + 0x43));
  out->n = in->db & 0x80;
  out->z = in->db == 0;
}

// Negative test: the window positions shifted by one register
static void call_video_load_screen_wrong(const Regs* in, Regs* out) {
  call_video_load_screen(in, out);
  uint32_t image = (uint32_t)(in->a & 0xFF) << 16 | in->x;
  video_set_window_position(0, bus_rd8(image + 0x31));
}

// an image of random bytes in low WRAM through bank $00 or $80 (the original's data bank is
// the image's, and its register writes need a bank that maps the I/O registers), with no
// interrupt enabled ($4200: the fuzz runs without interrupts)
static void fuzz_video_load_screen(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->a = (uint16_t)((r->a & 0xFF00) | ((rnd() & 1) ? 0x80 : 0x00));
  r->x = (uint16_t)(0x0C00 + (rnd() & 0x0700));
  for(int i = 0; i < 0x44; i++) ram[r->x + i] = (uint8_t)rnd();
  ram[r->x + 0x3E] &= 0x01;
}

// ---- race_hud_draw ($85:8814) ----  no register outputs (the race NMI loads A and X next)
static void call_race_hud_draw(const Regs* in, Regs* out) { (void)in; (void)out; race_hud_draw(); }

// Negative test: the five lights one place off
static void call_race_hud_draw_wrong(const Regs* in, Regs* out) {
  (void)in;
  (void)out;
  wr16(0x014C, (uint16_t)(rd16(0x014C) + 1));
  race_hud_draw();
  wr16(0x014C, (uint16_t)(rd16(0x014C) - 1));
}

// the bar, the gauge and its count in range (the original loops for ever past them), the
// lights 0-5 (5: the never-run fifth light), the strip accumulator either sign
static void fuzz_race_hud_draw(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x003E, (uint16_t)(rnd() % 15));
  wr16ram(ram, 0x014C, (uint16_t)(rnd() % 6));
  wr16ram(ram, 0x025B, (uint16_t)rnd());
  wr16ram(ram, 0x025C, (uint16_t)(rnd() % 11));
  if((rnd() & 3) == 0) ram[0x025C] |= 0x80;  // the word at $025B negative: the other gauge
  wr16ram(ram, 0x0235, (uint16_t)((rnd() & 3) == 0 ? 0x8000 | rnd() : rnd() % 12));
  wr16ram(ram, 0x1F06, (uint16_t)(1 + rnd() % 10));
  for(uint32_t a = 0x0042; a <= 0x00AA; a++) ram[a] = (uint8_t)rnd();
  ram[0x4E4D] = (uint8_t)rnd();
  ram[0x4E4E] = (uint8_t)rnd();
}

// ---- irq_race_band ($85:87BA) ----  saves and restores DB, A, X itself; P from the frame
static void call_irq_race_band(const Regs* in, Regs* out) { irq_race_band(); restoreP(in, out); }

// Negative test: the colours one CGRAM entry off
static void call_irq_race_band_wrong(const Regs* in, Regs* out) {
  uint16_t band = rd16(0x00BC);
  platform_set_irq_h_low(0xFF);
  dma_stream_size(2, 1);
  dma_stream_size(7, 1);
  dma_stream_size(1, 8);
  dma_stream_size(3, 8);
  video_set_cgram_address(0x22);
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
  restoreP(in, out);
}

// ---- race_cycle_blocks ($85:9147) ----  no register outputs (the race NMI's next
// instructions overwrite A and the flags, and the tail restores X and Y)
static void call_race_cycle_blocks(const Regs* in, Regs* out) { (void)in; (void)out; race_cycle_blocks(); }
static void fuzz_race_cycle_blocks(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(uint32_t i = 0x2000; i < 0x2140; i++) ram[i] = (uint8_t)rnd();
}

// a queue of up to 23 transfers (some skipped, some with a flag word) and a budget that
// is spent, short, or already gone
static void fuzz_race_upload_queue(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x00FC, (rnd() & 7) == 0 ? (uint16_t)(0x10000 - (rnd() & 0xFF)) : (uint16_t)(rnd() % 0x1800));
  uint16_t tail = (uint16_t)(rnd() & 0xFE), n = (uint16_t)(rnd() % 24);
  wr16ram(ram, 0x00FE, tail);
  wr16ram(ram, 0x0100, (uint16_t)((tail + 2 * n) & 0xFE));
  for(uint16_t i = 0; i < n; i++) {
    uint16_t x = (uint16_t)((tail + 2 * i) & 0xFE);
    wr16ram(ram, (uint32_t)(0xF433 + x), (uint16_t)((rnd() & 31) == 0 ? 0 : 2 + (rnd() % 0x400) * 2));
    wr16ram(ram, (uint32_t)(0xF533 + x), (uint16_t)rnd());
    wr16ram(ram, (uint32_t)(0xF633 + x), (uint16_t)(rnd() & 0x7FFF));
    wr16ram(ram, (uint32_t)(0xF733 + x), (uint16_t)(((rnd() & 7) == 0 ? 0x8000 : 0) | (rnd() & 0xFF)));
    wr16ram(ram, (uint32_t)(0xF833 + x), (uint16_t)((rnd() & 3) == 0 ? 0x2000 + (rnd() & 0x0FFE) : 0));
    wr16ram(ram, (uint32_t)(0xF933 + x), (uint16_t)rnd());
  }
  // a queue of only skipped entries sets channel 7's mode and sends nothing: the harness
  // does not model a DMA set up and never started (never seen live; the race NMI rewrites
  // channel 7 before its next kick), so keep one entry live
  bool any = false;
  for(uint16_t i = 0; i < n; i++) any |= !(ram[0xF733 + ((tail + 2 * i) & 0xFE) + 1] & 0x80);
  if(n && !any) ram[0xF733 + tail + 1] &= 0x7F;
}

static void fuzz_func_838989(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x0026, (uint16_t)rnd());
  wr16ram(ram, 0x01CC, (uint16_t)rnd());
  wr16ram(ram, 0x00D4, (uint16_t)rnd());
  ram[0x00D6] = (uint8_t)rnd();
  wr16ram(ram, 0x1F32, (uint16_t)rnd());
  wr16ram(ram, 0x00E2, (uint16_t)(rnd() & 0x1E));
}

// ---- nmi_prologue ($80:8807) / nmi_race_prologue ($85:8377) ----  rows that end at the
// dispatch (endAt): the targets start with D = 0 and the handler's data bank
static void call_nmi_prologue(const Regs* in, Regs* out) { (void)in; nmi_prologue(); out->d = 0; out->db = 0x81; }
static void call_nmi_race_prologue(const Regs* in, Regs* out) { (void)in; nmi_race_prologue(); out->d = 0; out->db = 0x85; }

// Negative test: the sprite table sent a second time, one word further into OAM
static void call_nmi_prologue_wrong(const Regs* in, Regs* out) {
  (void)in;
  bool oam = rd16(0x0026) & 0x0002;
  nmi_prologue();
  if(oam) video_upload_oam(0x0001, 0x810900, 0x0220);
  out->d = 0;
  out->db = 0x81;
}

// the palette and sprite flags, the fade, and a queue of up to 8 transfers against the
// budget the two uploads leave
static void fuzz_nmi_prologue(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x0026, (uint16_t)rnd());
  uint16_t target = (uint16_t)(rnd() & 0x0F00), level = (uint16_t)(rnd() & 0x0FFF);
  uint16_t step = (rnd() & 3) == 0 ? 0 : (uint16_t)((rnd() & 1) ? (rnd() & 0x1FF) : -(int)(rnd() & 0x1FF));
  if((rnd() & 3) == 0) level = (uint16_t)(target - step);
  wr16ram(ram, 0x0196, step);
  wr16ram(ram, 0x0198, target);
  wr16ram(ram, 0x019A, level);
  if((rnd() & 3) == 0) ram[0x019B] = 0;
  uint16_t tail = (uint16_t)(rnd() & 0x3E), n = (uint16_t)(rnd() % 9);
  wr16ram(ram, 0x002A, tail);
  wr16ram(ram, 0x0028, (uint16_t)((tail + 2 * n) & 0x3E));
  for(uint16_t i = 0; i < n; i++) {
    uint16_t x = (uint16_t)((tail + 2 * i) & 0x3E);
    wr16ram(ram, (uint32_t)(0x1900 + x), (uint16_t)((rnd() & 31) == 0 ? 0x2000 : 2 + (rnd() % 0x400) * 2));
    wr16ram(ram, (uint32_t)(0x1940 + x), (uint16_t)rnd());
    wr16ram(ram, (uint32_t)(0x1980 + x), (uint16_t)(rnd() & 0xFF));
    wr16ram(ram, (uint32_t)(0x19C0 + x), (uint16_t)(rnd() & 0x7FFF));
  }
}

// ---- nmi_race_tail ($85:838A) ----  the race NMI's prologue pushed the same frame
static void call_nmi_race_tail(const Regs* in, Regs* out) { nmi_race_tail(); restoreNmiFrame(in, out); }

// Negative test: brightness 0 written as is, without the forced blank (the $2100 byte)
static void call_nmi_race_tail_wrong(const Regs* in, Regs* out) {
  nmi_race_tail();
  if(rd8(0x014B) == 0) video_set_display(0x00);
  restoreNmiFrame(in, out);
}

// the fade on (either direction, sometimes landing on the target) or off; brightness 0 often
static void fuzz_nmi_quiet(uint8_t* ram, uint32_t (*rnd)(void));
static void fuzz_nmi_race_tail(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  fuzz_nmi_quiet(ram, rnd);
  uint16_t target = (uint16_t)(rnd() & 0x0F00), level = (uint16_t)(rnd() & 0x0FFF);
  uint16_t step = (rnd() & 3) == 0 ? 0 : (uint16_t)((rnd() & 1) ? (rnd() & 0x1FF) : -(int)(rnd() & 0x1FF));
  if((rnd() & 3) == 0) level = (uint16_t)(target - step);
  wr16ram(ram, 0x0146, step);
  wr16ram(ram, 0x0148, target);
  wr16ram(ram, 0x014A, level);
  if((rnd() & 3) == 0) ram[0x014B] = 0;
}

// NMI rows can fuzz when the tail sends no sound message (bit 15 of $046D clear): then
// nothing waits for the sound CPU
static void fuzz_func_8AA050(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));

static void fuzz_nmi_quiet(uint8_t* ram, uint32_t (*rnd)(void)) {
  ram[0x046D] = (uint8_t)rnd();
  ram[0x046E] = (uint8_t)(rnd() & 0x7F);
}

// ---- nmi_race_full ($85:83CE) ----  then its tail, which restores the NMI frame
static void call_nmi_race_full(const Regs* in, Regs* out) { nmi_race_full(); restoreNmiFrame(in, out); }

// Negative test: the window table's indirect bank wrong
static void call_nmi_race_full_wrong(const Regs* in, Regs* out) {
  nmi_race_full();
  dma_hdma_set_indirect_bank(4, 0x7F);
  restoreNmiFrame(in, out);
}

// Everything the full frame reads: both halves of the request, the transfers, the queue,
// the HUD, the clock (valid BCD only: the original adds in decimal mode, and LakeSnes's
// result for other digits is the only reference), the band table (first band at line 0,
// 1-4: the never-run path, the middle, or below the last raster line), the sound request.
static void fuzz_nmi_race_full(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_nmi_race_tail(r, ram, rnd);
  fuzz_race_upload_queue(r, ram, rnd);
  fuzz_race_hud_draw(r, ram, rnd);
  fuzz_func_838989(r, ram, rnd);
  wr16ram(ram, 0x00B8, (uint16_t)rnd());
  wr16ram(ram, 0x00B4, (uint16_t)(rnd() & 1));
  wr16ram(ram, 0x1E02, (uint16_t)((rnd() & 3) == 0 ? 0 : (rnd() % 0x200) * 2));
  wr16ram(ram, 0x1E54, (uint16_t)((rnd() & 3) == 0 ? 0 : (rnd() % 0x200) * 2));
  static const uint16_t picks[5] = {0x8000, 0, 6, 12, 18};
  wr16ram(ram, 0x1F24, picks[rnd() % 5]);
  wr16ram(ram, 0x024A, (uint16_t)rnd());
  wr16ram(ram, 0x0244, (uint16_t)rnd());
  wr16ram(ram, 0x0060, (uint16_t)(1 + rnd() % 3));
  uint16_t clock = 0;
  for(int d = 0; d < 4; d++) clock |= (uint16_t)((rnd() % 10) << (4 * d));
  if((rnd() & 3) == 0) clock = (uint16_t)(0x5998 + (rnd() & 1));
  wr16ram(ram, 0x005A, clock);
  for(int i = 0; i < 2; i++) {  // the band index the request copies, and the current one
    uint16_t band = (uint16_t)(rnd() % 0x20);
    wr16ram(ram, i ? 0x00BE : 0x00C0, band);
    static const uint8_t firsts[6] = {0, 1, 4, 5, 0xDB, 0xDC};
    ram[0x0486 + band] = (rnd() & 1) ? firsts[rnd() % 6] : (uint8_t)rnd();
    ram[0x0486 + band + 1] = (rnd() & 3) == 0 ? 0 : (uint8_t)rnd();
    static const uint8_t slots[4] = {0x02, 0x04, 0x08, 0x80};
    uint8_t mask = 0;
    for(int s2 = 0; s2 < 4; s2++) if(rnd() & 1) mask |= slots[s2];
    ram[0x0596 + band] = mask;
  }
  wr16ram(ram, 0x047C, (uint16_t)rnd());
}

static void fuzz_nmi_tail(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  fuzz_nmi_quiet(ram, rnd);
  wr16ram(ram, 0x0026, (uint16_t)rnd());
}

static void fuzz_nmi_target_80FC5F(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_func_8AA050(r, ram, rnd);
  fuzz_nmi_quiet(ram, rnd);
}


// the band height on every value up to $3A (the paths at $34-$37 and $3A), opening or done
static void fuzz_nmi_target_80E175(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  fuzz_nmi_quiet(ram, rnd);
  wr16ram(ram, 0x0114, (uint16_t)(rnd() % 0x3B));
  wr16ram(ram, 0x0116, (rnd() & 7) ? 0 : 1);
  wr16ram(ram, 0x0118, (uint16_t)(rnd() % 0x20));
}

// ---- nmi_tail ($80:8934) ----
static void call_nmi_tail(const Regs* in, Regs* out) {
  nmi_tail();
  restoreNmiFrame(in, out);
}

// ---- irq_split_top ($80:E3F7) ----  saves and restores DB, A, X, Y itself; P from the frame
static void call_irq_split_top(const Regs* in, Regs* out) {
  irq_split_top();
  restoreP(in, out);
}

// Negative test: colour 0 one step off (the CGRAM colour effect must see it); when the split
// is off it writes no colour, so those samples pass
static void call_irq_split_top_wrong(const Regs* in, Regs* out) {
  bool on = rd16(0x017A) < 0x64;
  irq_split_top();
  if(on) video_set_color(0, 0x7CC7);
  restoreP(in, out);
}

// ---- irq_split_bottom ($80:E48A) ----  saves and restores DB, A, X, Y itself (PHB..PLB);
// P from the frame
static void call_irq_split_bottom(const Regs* in, Regs* out) {
  irq_split_bottom();
  restoreP(in, out);
}

// Negative test: one layer off the main screen (the byte-register comparison must see it).
static void call_irq_split_bottom_wrong(const Regs* in, Regs* out) {
  irq_split_bottom();
  video_set_main_screen(0x13);
  restoreP(in, out);
}

// ---- irq_ack_only ($85:95BD) ----
static void call_irq_ack_only(const Regs* in, Regs* out) {
  irq_ack_only();
  restoreP(in, out);
}

// Negative test: irq_ack_only that leaves a byte behind (the RTI row must see it).
static void call_irq_ack_only_wrong(const Regs* in, Regs* out) {
  wr8(0x1F80, (uint8_t)(rd8(0x1F80) + 1));
  restoreP(in, out);
}

static void call_func_8A8000(const Regs* in, Regs* out) { (void)in; (void)out; func_8A8000(); }
static void call_func_8A987F(const Regs* in, Regs* out) { (void)in; (void)out; func_8A987F(); }

static void fuzz_func_8ABF76(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x038E, (rnd() & 3) ? (uint16_t)rnd() : (uint16_t)(rnd() % 100));
  wr16ram(ram, 0x03BD, (rnd() & 3) ? (uint16_t)rnd() : 0xFFFF);
}

// ---- oam_from_objects ($8A:8E1B, long) ----  X = $100, Y = the new end (N Z of its LDY); A =
// $E000 after hiding leftovers (C = 1 from the loop's last CPY), else the end with C of the
// CMP end vs the last end; V as on entry
static void call_oam_from_objects(const Regs* in, Regs* out) {
  (void)in;
  uint16_t last = rd16(0x0130);
  uint16_t end = oam_from_objects();
  bool hid = end != last && ((uint16_t)(end - last) & 0x8000);
  out->x = 0x0100;
  out->y = end;
  setNZ16(out, end);
  out->a = hid ? 0xE000 : end;
  out->c = hid ? true : end >= last;
}

// slots in use or not, x across the $100 boundary, y on both sides of $F0 and negative
static void fuzz_objects(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(uint16_t x = 0; x < 0x100; x = (uint16_t)(x + 2)) {
    uint32_t k = rnd();
    wr16ram(ram, (uint16_t)(0x0500 + x), (k & 3) == 0 ? 0 : (k & 4) ? (uint16_t)(0x8000 | rnd()) : (uint16_t)(1 + rnd() % 0x7FFF));
    wr16ram(ram, (uint16_t)(0x0600 + x), (uint16_t)rnd());
    wr16ram(ram, (uint16_t)(0x0700 + x), (k & 0x18) == 0 ? (uint16_t)(0x00E8 + rnd() % 16) : (k & 0x18) == 8 ? (uint16_t)(0x8000 | rnd()) : (uint16_t)(rnd() % 0xF0));
    wr16ram(ram, (uint16_t)(0x0800 + x), (uint16_t)rnd());
  }
  wr16ram(ram, 0x0130, (uint16_t)(4 * (rnd() % 129)));
}

// ---- func_8AA050 (long) ----  as func_8AA005 after it; with D+$8E set, X = $100 and Y = the
// new sprite end (oam_from_objects's)
static void call_func_8AA050(const Regs* in, Regs* out) {
  (void)in;
  bool on = rd16(DP + 0x8E) != 0;
  uint16_t b[4] = {rd16(0x0018), rd16(0x0014), rd16(0x0010), rd16(0x000C)};
  uint16_t before = 0;
  for(int i = 0; i < 3; i++) before = (uint16_t)(before + ((b[i] ^ 1) & 1));
  out->a = func_8AA050();
  setNZ16(out, out->a);
  setAdc16(out, (uint16_t)((b[3] ^ 1) & 1), before);
  if(on) { out->x = 0x0100; out->y = rd16(0x012A); }
}

static void fuzz_func_8AA050(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_objects(r, ram, rnd);
  wr16ram(ram, DP + 0x8E, (rnd() & 1) ? 0 : (uint16_t)rnd());
}

static void fuzz_shift32(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  static const uint16_t words[4] = {0x038E, 0x0390, 0x03BD, 0x03BF};
  for(int i = 0; i < 4; i++) wr16ram(ram, words[i], (uint16_t)rnd());
}

static void fuzz_func_8ABFBC(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x44, (uint16_t)rnd());
  wr16ram(ram, DP + 0x46, (uint16_t)rnd());
}

// ---- text_cursor_down3 ($8A:A37A) ----  N/Z of the new row.
static void call_text_cursor_down3(const Regs* in, Regs* out) {
  (void)in;
  text_cursor_down3();
  setNZ16(out, rd16(TEXT_CURSOR_Y));
}

static void fuzz_text_cursor_down3(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, TEXT_CURSOR_Y, (uint16_t)rnd());
}

// ---- func_8AB942 ($8A:B942) ----  X kept; A = the restored pointer; Y = the column used
// last ($0174 before its final -2); N/Z of the new $0174; C/V of the last ADC #$40.
static void call_func_8AB942(const Regs* in, Regs* out) {
  uint16_t base = rd16(DP + 0x70);
  func_8AB942();
  out->a = base;
  out->y = (uint16_t)(rd16(DP + 0x74) + 2);
  setNZ16(out, rd16(DP + 0x74));
  setAdc16(out, (uint16_t)(base + 27 * 0x40), 0x40);
}

static void fuzz_func_8AB942(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x70, (uint16_t)(2 * (rnd() % 0x3000)));
  ram[DP + 0x72] = 0x7F;
  wr16ram(ram, DP + 0x74, (uint16_t)(2 * (rnd() % 32)));
  wr16ram(ram, DP + 0x76, (uint16_t)(2 * (rnd() % 32)));
  wr16ram(ram, DP + 0x78, (uint16_t)rnd());
}

// ---- decompress ($82:FB70, long call) ----  A = output bank << 8 | input bank, X = input,
// Y = output; D = its scratch page. Exits with A = bytes written (N/Z, and C/V of the
// final ADC: ~start + end + 1), X = the input after the end byte, Y = the output end.
static void call_decompress(const Regs* in, Regs* out) {
  uint16_t end;
  uint16_t len = decompress(in->d, (uint8_t)in->a, in->x, (uint8_t)(in->a >> 8), in->y, &end);
  out->a = len;
  out->x = end;
  out->y = (uint16_t)(in->y + len);
  setNZ16(out, len);
  uint16_t a = (uint16_t)~in->y;
  uint32_t sum = (uint32_t)a + out->y + 1;
  out->c = sum > 0xFFFF;
  out->v = (~(a ^ out->y) & (a ^ sum) & 0x8000) != 0;
}

// A random but valid stream at $7E:4000 unpacking to $7F:2100, after 8 KB of random
// bytes the back references can reach.
// A random stream at $7E:4000 unpacking from `y`; without back references (commands
// $80-$FF) when the destination starts at 0, where they would reach before the buffer.
static void fuzz_decompress_stream(uint8_t* ram, uint32_t (*rnd)(void), uint32_t y, bool backrefs) {
  uint32_t p = 0x4000, end = y + 0x1700;
  int commands = 4 + (int)(rnd() % 24);
  for(int c = 0; c < commands && y < end; c++) {
    // from 0, the first command is a literal run: the repeats re-read the bytes before them
    switch((!backrefs && c == 0) ? 0 : rnd() % (backrefs ? 8 : 5)) {
      case 0: {
        uint8_t n = (uint8_t)((!backrefs && c == 0 ? 4 : 1) + rnd() % 0x3C);
        ram[p++] = n;
        for(int i = 0; i < n; i++) ram[p++] = (uint8_t)rnd();
        y += n;
        break;
      }
      case 1: { uint8_t k = (uint8_t)(rnd() & 15); ram[p++] = (uint8_t)(0x40 | k); y += k + 3; break; }
      case 2: { uint8_t k = (uint8_t)(rnd() & 15); ram[p++] = (uint8_t)(0x50 | k); y += 2 * (k + 2); break; }
      case 3: { uint8_t k = (uint8_t)(rnd() & 15); ram[p++] = (uint8_t)(0x60 | k); y += k + 3; break; }
      case 4: { uint8_t k = (uint8_t)(rnd() & 15); ram[p++] = (uint8_t)(0x70 | k); y += 2 * (k + 2); break; }
      case 5: { ram[p++] = (uint8_t)(0x80 | (rnd() & 0x3F)); y += 3; break; }
      case 6: {
        ram[p++] = (uint8_t)(0xC0 | (rnd() & 0x1F));
        ram[p++] = (uint8_t)rnd();
        y += 11;
        break;
      }
      default: {
        uint8_t k = (rnd() & 1) ? (uint8_t)(rnd() & 7) : (uint8_t)(rnd() & 0x1F);  // short distances too
        ram[p++] = (uint8_t)(0xE0 | k);
        ram[p++] = (uint8_t)rnd();
        uint8_t l = (rnd() & 1) ? (uint8_t)(rnd() & 3) : (uint8_t)rnd();         // lengths 5-8 and longer
        ram[p++] = l;
        y += l + 5;
        break;
      }
    }
  }
  ram[p] = 0;
}

static void fuzz_decompress(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  for(int i = 0; i < 0x2100; i++) ram[0x10000 + i] = (uint8_t)rnd();
  fuzz_decompress_stream(ram, rnd, 0x2100, true);
  r->a = 0x7F7E;
  r->x = 0x4000;
  r->y = 0x2100;
}

// ---- decompress_near ($80:FBA0) ----  JSL decompress / RTS: same registers and flags.
static void call_decompress_near(const Regs* in, Regs* out) { call_decompress(in, out); }

// ---- queue_unpacked_upload ($80:B95D) ----  A = the length, X = the input end, Y = $7F
// (N Z from that LDY), C V of decompress.
static void call_queue_unpacked_upload(const Regs* in, Regs* out) {
  uint16_t dst = rd16((uint16_t)(in->d + 0x5E)), end;
  uint16_t len = queue_unpacked_upload(in->d, (uint8_t)in->a, in->x, in->y, &end);
  out->a = len;
  out->x = end;
  out->y = 0x007F;
  out->n = out->z = false;
  uint16_t a = (uint16_t)~dst, y = (uint16_t)(dst + len);
  uint32_t sum = (uint32_t)a + y + 1;
  out->c = sum > 0xFFFF;
  out->v = (~(a ^ y) & (a ^ sum) & 0x8000) != 0;
}

// the slot free (with interrupts off a busy slot would never clear), a stream as for
// decompress unpacking to $7F:2100; A's high byte below $80 (the destination bank is
// A | $7F00: from $80 up it would be $FF, ROM; the game passes 0)
static void fuzz_queue_unpacked_upload(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_decompress(r, ram, rnd);
  wr16ram(ram, (uint16_t)(r->d + 0x64), 0);
  wr16ram(ram, (uint16_t)(r->d + 0x5E), 0x2100);
  r->a = (uint16_t)((rnd() & 0x7F00) | 0x7E);
  r->y = (uint16_t)rnd();
}

// ---- clear_object_slots ($8A:A47E) ----  X kept (N/Z from PLX).
static void call_clear_object_slots(const Regs* in, Regs* out) {
  clear_object_slots();
  setNZ16(out, in->x);
}

// ---- func_8AA469 ($8A:A469) ----  A = $0D78's new value (N/Z of the last ASL),
// C = the bit shifted out of the last ASL.
static void call_func_8AA469(const Regs* in, Regs* out) {
  (void)in;
  func_8AA469();
  out->a = rd16(0x0D78);
  setNZ16(out, out->a);
  out->c = (rd16(DP + 0x5A) >> 13) & 1;
  Regs t;
  setSbc16(&t, (uint16_t)(rd16(DP + 0x3C) << 3), 0x1C);  // V of the SEC/SBC #$1C (ASL keeps V)
  out->v = t.v;
}

static void fuzz_8aa469(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x3C, (uint16_t)rnd());
  wr16ram(ram, DP + 0x5A, (uint16_t)rnd());
}

// ---- string_list_nth ($8A:A4CF) ----  A = list address (bank DB), n from ($1516,X & $3C)
// >> 2. Exits with A = the address, X/Y kept (N/Z from the PLX).
static void call_string_list_nth(const Regs* in, Regs* out) {
  uint16_t n = (uint16_t)((rd16(0x1516 + in->x) & 0x3C) >> 2);
  out->a = string_list_nth(((uint32_t)in->db << 16) | in->a, n);
  setNZ16(out, in->x);
}

static void fuzz_string_list_nth(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)(2 * (rnd() % 8));
  wr16ram(ram, 0x1516 + r->x, (uint16_t)rnd());
  // strings in low WRAM (bank $8A's mirror), sometimes ending right at a page boundary
  uint16_t base = (rnd() & 3) == 0 ? (uint16_t)(0x1BF0 - rnd() % 8) : 0x1A00;
  uint16_t p = base;
  for(int k = 0; k < 20 && p < 0x1CF0; k++) {
    int len = (int)(rnd() % 12);
    for(int i = 0; i < len; i++) ram[p++] = (uint8_t)(0x20 + rnd() % 0x5F);
    ram[p++] = 0;
  }
  r->a = base;
}

// ---- take_nibble ($80:8542) ----  A = the nibble; N = 0, Z from it, C = 0 (the last ROL
// shifts A's clear bit 15 out).
static void call_take_nibble(const Regs* in, Regs* out) {
  (void)in;
  out->a = take_nibble();
  setNZ16(out, out->a);
  out->c = false;
}

static void fuzz_take_nibble(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x18, (uint16_t)rnd());
  wr16ram(ram, DP + 0x1A, (uint16_t)rnd());
}

// ---- func_83FE9A ($83:FE9A) ----  A, X, Y in; C = refused. N/Z/V: from BIT $047C when
// it refuses that way, else N/Z of A (the PLA) and V of the SBC if the cooldown ran
// (else BIT's V).
static void call_func_83FE9A(const Regs* in, Regs* out) {
  uint16_t flags = rd16(0x047C), cool = rd16((uint16_t)(in->d + 0x23)), step = rd16(0x1E9E);
  out->c = func_83FE9A(in->d, in->a, in->x, in->y);
  out->v = (flags & 0x4000) != 0;
  if(flags & 0x8000) {
    out->n = true;
    out->z = (in->a & flags) == 0;
    return;
  }
  setNZ16(out, in->a);
  if(cool != 0) {
    Regs t;
    setSbc16(&t, cool, step);
    out->v = t.v;
  }
}

static void fuzz_func_83FE9A(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint32_t k = rnd();
  wr16ram(ram, 0x047C, (uint16_t)((k & 3) == 0 ? (rnd() | 0x8000) : (rnd() & 0x7FFF)));
  wr16ram(ram, (uint16_t)(r->d + 0x23), (k & 4) ? 0 : (uint16_t)rnd());
  wr16ram(ram, 0x1E9E, (uint16_t)rnd());
}

// ---- sprite_text ($80:841A) ----  A's low byte = the string's bank, X = its address,
// Y = start x. Exits with Y = final x (N/Z from that LDY), X = the address of the
// terminator, A = 0 when a glyph was laid out (its 16-bit LDA #0 cleared B), else B kept.
static void call_sprite_text(const Regs* in, Regs* out) {
  uint32_t str = ((uint32_t)(in->a & 0xFF) << 16) | in->x;
  uint16_t slot = rd16(DP + 0x74);
  out->y = sprite_text(str, in->y);
  uint16_t n = 0;
  while(bus_rd8((str & 0xFF0000) | (uint16_t)(str + n)) != 0) n++;
  out->x = (uint16_t)(in->x + n);
  out->a = rd16(DP + 0x74) != slot ? 0 : (uint16_t)(in->a & 0xFF00);
  setNZ16(out, out->y);
}

static void fuzz_sprite_text(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  static const char pool[] = "AZaz09 .!?&#[+={}-~]_@*,\x85\xC1";
  int len = (int)(rnd() % 16);
  for(int i = 0; i < len; i++) {
    uint32_t k = rnd();
    ram[0x1C00 + i] = (k & 1) ? (uint8_t)pool[rnd() % (sizeof pool - 1)] : (uint8_t)(0x20 + rnd() % 0x60);
  }
  ram[0x1C00 + len] = 0;
  r->a = (uint16_t)((r->a & 0xFF00) | 0x7E);  // the string in bank $7E
  r->x = 0x1C00;
  r->y = (uint16_t)rnd();
  wr16ram(ram, DP + 0x74, (uint16_t)(4 * (rnd() % 32)));
  wr16ram(ram, DP + 0x14, (uint16_t)rnd());
}

// ---- sprite_text_centered ($80:84BE) ----  as sprite_text (A = bank, X = string, Y = x);
// exits with Y = the end slot, A = shift high byte : the last entry's new x byte, Z = C =
// 1 from the final CPY, V of the last 8-bit ADC.
static void call_sprite_text_centered(const Regs* in, Regs* out) {
  uint32_t str = ((uint32_t)(in->a & 0xFF) << 16) | in->x;
  uint16_t n = 0;
  while(bus_rd8((str & 0xFF0000) | (uint16_t)(str + n)) != 0) n++;
  uint16_t shift = sprite_text_centered(str, in->y);
  uint16_t end = rd16(DP + 0x74);
  uint8_t last = rd8(0x0900 + (uint16_t)(end - 4)), before = (uint8_t)(last - shift);
  out->a = (uint16_t)((shift & 0xFF00) | last);
  out->x = (uint16_t)(in->x + n);
  out->y = end;
  out->z = out->c = true;
  out->n = false;
  out->v = ((~(before ^ (uint8_t)shift)) & (before ^ last) & 0x80) != 0;
}

static void fuzz_sprite_text_centered(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_sprite_text(r, ram, rnd);
  // the game never centres a string without a glyph ("", only spaces or bytes with bit 7
  // set): there the original's do-while walks all of memory
  bool glyph = false;
  for(int i = 0; ram[0x1C00 + i] != 0; i++) if(ram[0x1C00 + i] != 0x20 && ram[0x1C00 + i] < 0x80) glyph = true;
  if(!glyph) { ram[0x1C00] = 'A'; ram[0x1C01] = 0; }
}

// ---- sprite_number ($80:84F0) ----  A = low word, X = high word, Y = x; exits with Y =
// the final x (N/Z from the closing LDY), A = 0 (a ninth nibble), X = $FFFF ($FFFE for 0),
// C of the last CPX, V of the last width ADC.
static void call_sprite_number(const Regs* in, Regs* out) {
  uint32_t value = ((uint32_t)in->x << 16) | in->a;
  out->y = sprite_number(value, in->y);
  out->a = 0;
  out->x = value == 0 ? 0xFFFE : 0xFFFF;
  setNZ16(out, out->y);
  out->c = value == 0;
  uint16_t w = rom8(0x8085E5 + 0x34 + (value & 0xF));
  setAdc16(out, w, (uint16_t)(out->y - w));
}

// ---- sprite_number_right ($80:8698) ----  value in X:A, x in Y. Exits with Y = D+$74
// (the shift loop's end), X as sprite_number leaves it, N Z C of the final CPY (equal),
// A = high byte of the shift : the last shifted byte, V of that 8-bit ADC.
static void call_sprite_number_right(const Regs* in, Regs* out) {
  uint32_t value = ((uint32_t)in->x << 16) | in->a;
  uint16_t shift = sprite_number_right(value, in->y);
  uint16_t end = rd16(DP + 0x74);
  uint8_t post = rd8((uint16_t)(0x0900 + end - 4)), pre = (uint8_t)(post - (uint8_t)shift);
  out->y = end;
  out->x = value == 0 ? 0xFFFE : 0xFFFF;
  out->a = (uint16_t)((shift & 0xFF00) | post);
  out->n = false;
  out->z = out->c = true;
  out->v = (~(pre ^ (uint8_t)shift) & (pre ^ post) & 0x80) != 0;
}

// ---- sprite_number_centered ($80:8635) ----  as sprite_number_right's adapter
static void call_sprite_number_centered(const Regs* in, Regs* out) {
  uint32_t value = ((uint32_t)in->x << 16) | in->a;
  uint16_t shift = sprite_number_centered(value, in->y);
  uint16_t end = rd16(DP + 0x74);
  uint8_t post = rd8((uint16_t)(0x0900 + end - 4)), pre = (uint8_t)(post - (uint8_t)shift);
  out->y = end;
  out->x = value == 0 ? 0xFFFE : 0xFFFF;
  out->a = (uint16_t)((shift & 0xFF00) | post);
  out->n = false;
  out->z = out->c = true;
  out->v = (~(pre ^ (uint8_t)shift) & (pre ^ post) & 0x80) != 0;
}

static void fuzz_sprite_number(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));

static void fuzz_sprite_number(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint32_t k = rnd();
  r->a = (uint16_t)rnd();
  r->x = (k & 3) == 0 ? 0 : (uint16_t)rnd();  // short numbers too
  if((k & 12) == 0) r->a = (uint16_t)(r->a & 0xF);
  if((k & 48) == 0) r->a = 0, r->x = 0;
  r->y = (uint16_t)rnd();
  wr16ram(ram, DP + 0x74, (uint16_t)(4 * (rnd() % 32)));
  wr16ram(ram, DP + 0x14, (uint16_t)rnd());
}

// ---- func_809DD0 ----  as sprite_number of its number
static void call_func_809DD0(const Regs* in, Regs* out) {
  (void)in;
  uint16_t i = rd16(DP + 0x94);
  uint32_t value = (uint32_t)rd16((uint16_t)(0x0390 + i)) << 16 | rd16((uint16_t)(0x038E + i));
  out->y = func_809DD0();
  out->a = 0;
  out->x = value == 0 ? 0xFFFE : 0xFFFF;
  setNZ16(out, out->y);
  out->c = value == 0;
  uint16_t w = rom8(0x8085E5 + 0x34 + (value & 0xF));
  setAdc16(out, w, (uint16_t)(out->y - w));
}

// both strings (bits 1 and 2 of D+$76), any number, a word index into the numbers
static void fuzz_func_809DD0(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_sprite_number(r, ram, rnd);
  wr16ram(ram, DP + 0x76, (uint16_t)rnd());
  uint16_t i = (uint16_t)(4 * (rnd() % 16));
  wr16ram(ram, DP + 0x94, i);
  uint32_t k = rnd();
  wr16ram(ram, (uint16_t)(0x038E + i), (k & 3) == 0 ? 0 : (uint16_t)rnd());
  wr16ram(ram, (uint16_t)(0x0390 + i), (k & 4) ? 0 : (uint16_t)rnd());
  wr16ram(ram, DP + 0xE7, (uint16_t)rnd());
}

// ---- sprite_group ($80:B49E) ----  A low byte = table bank, X = table address; D = $0100
// or $0200. Exits with A = 0 (the last terminator), X = the new slot, Y = the second
// terminator's address, N/Z from the PLB (the caller's DB), C/V of the last item's
// ADC #6 (or as on entry if both lists are empty).
// sprite_group's exit registers: the table is walked before the call (it may be in WRAM)
typedef struct { bool any; uint16_t lastY; } GroupWalk;

static GroupWalk sprite_group_walk(uint32_t table) {
  uint32_t bank = table & 0xFF0000, at = table;
  GroupWalk w = {false, 0};
  for(int list = 0; list < 2; list++) {
    while(bus_rd16(at) != 0) { w.any = true; w.lastY = (uint16_t)at; at = bank | (uint16_t)(at + 6); }
    if(list == 0) at = bank | (uint16_t)(at + 2);
  }
  return w;
}

static void sprite_group_outputs(Regs* out, const Regs* in, GroupWalk w, uint32_t end) {
  out->a = 0;
  out->x = rd16((uint16_t)(in->d + 0x74));
  out->y = (uint16_t)end;
  out->n = (in->db & 0x80) != 0;
  out->z = in->db == 0;
  if(w.any) setAdc16(out, w.lastY, 6);
}

static void call_sprite_group(const Regs* in, Regs* out) {
  uint32_t table = ((uint32_t)(in->a & 0xFF) << 16) | in->x;
  GroupWalk w = sprite_group_walk(table);
  sprite_group_outputs(out, in, w, sprite_group(in->d, table));
}

// ---- func_80EB66 ----  sprite_group's exit registers (its table at $81:CBB8)
static void call_func_80EB66(const Regs* in, Regs* out) {
  GroupWalk w = sprite_group_walk(0x81CBB8);
  sprite_group_outputs(out, in, w, func_80EB66(in->d));
}

static void fuzz_func_80EB66(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  static const uint8_t words[5] = {0x22, 0x24, 0x7C, 0x7E, 0xE8};
  for(int i = 0; i < 5; i++) wr16ram(ram, (uint16_t)(r->d + words[i]), (uint16_t)rnd());
}

static void fuzz_sprite_group(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint16_t at = 0x1C00;
  for(int list = 0; list < 2; list++) {
    int n = (int)(rnd() % 5);
    for(int i = 0; i < n; i++) {
      uint16_t tile = (uint16_t)rnd();
      if(tile == 0) tile = 1;
      wr16ram(ram, at, tile);
      wr16ram(ram, at + 2, (uint16_t)rnd());
      wr16ram(ram, at + 4, (uint16_t)((rnd() & 1) ? rnd() : rnd() % 0x100));
      at = (uint16_t)(at + 6);
    }
    wr16ram(ram, at, 0);
    at = (uint16_t)(at + 2);
  }
  r->a = (uint16_t)((r->a & 0xFF00) | 0x7E);  // the table in bank $7E
  r->x = 0x1C00;
  for(int o = 0x14; o <= 0x18; o += 2) wr16ram(ram, (uint16_t)(r->d + o), (uint16_t)rnd());
  wr16ram(ram, (uint16_t)(r->d + 0x74), (uint16_t)(4 * (rnd() % 32)));
}

// ---- func_83EA9B ($83:EA9B) ----  X = entry offset; exits with A = the $1EA0 value.
static void call_func_83EA9B(const Regs* in, Regs* out) {
  out->a = func_83EA9B(in->x);
  setNZ16(out, out->a);
}

static void fuzz_func_83EA9B(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)ram;
  r->x = (uint16_t)(2 * (rnd() % 41));
}

// ---- oam_high_table ($81:8101; $81:80F6) ----  exits with M = 16 and either A = $74 - $72
// (C = 1, no sprites to hide), X = the table - $200, or A = $E0E0, X = the end of the hidden
// run, Y = 0 (Z from the DEY), C = 0; V of the SBC either way.
static void oam_high_table_outputs_from(Regs* out, uint16_t x, uint16_t end, uint16_t last) {
  uint16_t diff = (uint16_t)(end - last);
  out->m8 = false;
  out->v = ((end ^ last) & (end ^ diff) & 0x8000) != 0;
  if(end >= last) {
    out->a = diff;
    out->x = x;
    out->c = true;
    setNZ16(out, diff);
  } else {
    uint16_t n = (uint16_t)(last - end) >> 2;
    out->a = 0xE0E0;
    out->x = (uint16_t)(end + 4 * n);
    out->y = 0;
    out->c = ((uint16_t)(last - end) >> 1) & 1;
    setNZ16(out, 0);
  }
}

static void oam_high_table_outputs(const Regs* in, Regs* out, uint16_t x) {
  oam_high_table_outputs_from(out, x, rd16((uint16_t)(in->d + 0x74)), rd16((uint16_t)(in->d + 0x72)));
}

static void fuzz_oam_tables(Regs* r, uint8_t* ram, uint32_t (*rnd)(void), uint16_t table);

// ---- oam_finish_0b00 ($81:80D0, long) ----  the packer's exit registers, then A = the old
// D+$74 (N/Z from its LDA).
static void run_oam_finish_0b00(const Regs* in, Regs* out, void (*fn)(uint16_t)) {
  uint16_t end = rd16((uint16_t)(in->d + 0x74)), last = rd16((uint16_t)(in->d + 0x72));
  fn(in->d);
  oam_high_table_outputs_from(out, 0x0900, end, last);
  out->a = end;
  setNZ16(out, end);
}

static void call_oam_finish_0b00(const Regs* in, Regs* out) { run_oam_finish_0b00(in, out, oam_finish_0b00); }
// $80:89D1: JSL to it, RTS: the same outputs
static void call_oam_finish_0b00_near(const Regs* in, Regs* out) { run_oam_finish_0b00(in, out, oam_finish_0b00_near); }

static void fuzz_oam_finish_0b00(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_oam_tables(r, ram, rnd, 0x0B00);
}

// ---- oam_finish ($81:80DA, long) ----  exits with X = the old D+$6C, A = the old D+$6E
// (N/Z from its LDA), C/V of its SBC #$0900, Y as the packer left it.
static void run_oam_finish(const Regs* in, Regs* out, void (*fn)(uint16_t)) {
  uint16_t built = rd16((uint16_t)(in->d + 0x6C)), other = rd16((uint16_t)(in->d + 0x6E));
  uint16_t end = rd16((uint16_t)(in->d + 0x74)), last = rd16((uint16_t)(in->d + 0x72));
  fn(in->d);
  oam_high_table_outputs_from(out, built, end, last);
  out->x = built;
  out->a = other;
  setNZ16(out, other);
  setSbc16(out, other, 0x0900);
}

static void call_oam_finish(const Regs* in, Regs* out) { run_oam_finish(in, out, oam_finish); }
// $80:89D6: JSL to it, RTS: the same outputs
static void call_oam_finish_near(const Regs* in, Regs* out) { run_oam_finish(in, out, oam_finish_near); }

static void fuzz_oam_finish(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint16_t built = (rnd() & 1) ? 0x0700 : 0x0900, other = (uint16_t)(0x1000 - built);
  wr16ram(ram, (uint16_t)(r->d + 0x6C), built);
  fuzz_oam_tables(r, ram, rnd, (uint16_t)(built + 0x200));
  wr16ram(ram, (uint16_t)(r->d + 0x6C), built);
  wr16ram(ram, (uint16_t)(r->d + 0x6E), (rnd() & 3) ? other : (uint16_t)rnd());
  wr16ram(ram, (uint16_t)(r->d + 0x70), (uint16_t)(4 * (rnd() % 129)));
}

static void call_oam_high_table(const Regs* in, Regs* out) {
  uint16_t x = rd16((uint16_t)(in->d + 0x6C));
  oam_high_table(in->d, (uint16_t)(x + 0x200));
  oam_high_table_outputs(in, out, x);
}

static void call_oam_high_table_0b00(const Regs* in, Regs* out) {
  oam_high_table(in->d, 0x0B00);
  oam_high_table_outputs(in, out, 0x0900);
}

static void fuzz_oam_tables(Regs* r, uint8_t* ram, uint32_t (*rnd)(void), uint16_t table) {
  for(int i = 0; i < 512; i++) ram[(uint16_t)(table + i)] = (uint8_t)rnd();
  wr16ram(ram, (uint16_t)(r->d + 0x74), (uint16_t)(4 * (rnd() % 129)));
  wr16ram(ram, (uint16_t)(r->d + 0x72), (uint16_t)(4 * (rnd() % 129)));
}

static void fuzz_oam_high_table(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint16_t x = (rnd() & 1) ? 0x0700 : (uint16_t)(0x0200 + 0x100 * (rnd() % 0x10));
  wr16ram(ram, (uint16_t)(r->d + 0x6C), x);
  fuzz_oam_tables(r, ram, rnd, (uint16_t)(x + 0x200));
  wr16ram(ram, (uint16_t)(r->d + 0x6C), x);  // in case the table covered it
}

static void fuzz_oam_high_table_0b00(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_oam_tables(r, ram, rnd, 0x0B00);
}

// ---- func_8ABB5C ($8A:BB5C) ----  Y = count, C = first bit in; exits with A = the bits
// shifted out, Y kept (N/Z from the PLY), C = the last bit taken from $0146.
static void call_func_8ABB5C(const Regs* in, Regs* out) {
  bool carry = in->c;
  out->a = func_8ABB5C(in->y, &carry);
  out->c = carry;
  setNZ16(out, in->y);
}

// ---- password_encode ($8A:BC8A) ----  A = the number of bits; X, Y and A out; N and Z
// from the last INX
// The V flag the original leaves: the signed overflow of its count division's last `- 5`
// (the only instruction there that writes V; the C has no flags).
static bool password_count_overflow(uint16_t bits) {
  uint16_t rest = bits, before;
  do {
    before = rest;
    rest = (uint16_t)(rest - 5);
  } while(!(rest & 0x8000));
  return ((before ^ 5) & (before ^ rest) & 0x8000) != 0;
}

static void call_password_encode(const Regs* in, Regs* out) {
  out->a = password_encode(in->a, &out->x, &out->y);
  setNZ16(out, out->x);
  out->c = false;  // the tail's last LSR shifts out bit 4 of a value masked with $03E0
  out->v = password_count_overflow(in->a);
}

static void fuzz_password_encode(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));

static void fuzz_big_number(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  for(int i = 0; i < 32; i++) ram[0x0DAC + i] = (uint8_t)rnd();
  r->y = (uint16_t)(1 + rnd() % 24);
  wr16ram(ram, DP + 0x46, (uint16_t)rnd());
  wr16ram(ram, DP + 0x84, (uint16_t)rnd());
  wr16ram(ram, DP + 0x92, (uint16_t)rnd());
}

// the bits to encode (up to 160), the big number, the feed word, the two tail fields
static void fuzz_password_encode(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_big_number(r, ram, rnd);
  r->a = (uint16_t)(rnd() % 161);
}

// ---- func_8ABBF4 ($8A:BBF4) ----  A = bits, Y = count; exits with A and Y kept (N/Z from
// the PLY), C/V of the last func_8ABC32 ADC ($0192 + $0DCA).
static void call_func_8ABBF4(const Regs* in, Regs* out) {
  func_8ABBF4(in->a, in->y);
  setNZ16(out, in->y);
  uint16_t low = rd16(0x0DCA);
  setAdc16(out, (uint16_t)(rd16(DP + 0x92) - low), low);
}

// ---- func_81FBB1 ($81:FBB1) ----  Y = car offset (DB = $7E). Exits with X = the car's
// segment record, and: first call (flag $40 was clear): A = position - record start (C = 0,
// N/Z of that or of it - $3F); later calls: A = the position and N/Z/C of its compare with
// the jump point, or, after a jump, A = the flags (C/V of the jump's ADC). V otherwise from
// the last position - start.
static void call_func_81FBB1(const Regs* in, Regs* out) {
  uint16_t car = in->y;
  bool first = !(rd16(CAR_FLAGS + (uint32_t)car) & 0x40);
  bool jumped = func_81FBB1(car);
  uint16_t seg = rd16(CAR_SEG + (uint32_t)car), pos = rd16(CAR_POS + (uint32_t)car);
  uint16_t start = bus_rd16(0x840000 + (uint32_t)seg);
  out->x = seg;
  if(jumped) {
    uint16_t skip = rd16(CAR_SKIP + (uint32_t)car);
    out->a = rd16(CAR_FLAGS + (uint32_t)car);
    setNZ16(out, out->a);
    setAdc16(out, (uint16_t)(pos - skip), skip);
    return;
  }
  setSbc16(out, pos, start);  // V (and C, overwritten below) of the last position - start
  out->c = false;
  if(first) {
    uint16_t d = (uint16_t)(pos - start);
    out->a = d;
    setNZ16(out, pos < start ? d : (uint16_t)(d - 0x3F));
  } else {
    out->a = pos;
    setNZ16(out, (uint16_t)(pos - rd16(CAR_LIMIT + (uint32_t)car)));
  }
}

static void fuzz_func_81FBB1(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  // a segment table in low WRAM, which bank $84's $0000-$1FFF mirrors
  uint16_t start = (uint16_t)(rnd() % 0x400);
  uint16_t starts[10];
  for(int i = 0; i < 10; i++) {
    uint16_t rec = (uint16_t)(0x1800 + 0x14 * i);
    starts[i] = start;
    for(int b = 0; b < 0x14; b++) ram[rec + b] = (uint8_t)rnd();
    wr16ram(ram, rec, start);
    wr16ram(ram, rec + 0x10, (uint16_t)(rnd() % 0x80));
    wr16ram(ram, rec + 0x12, (uint16_t)(rnd() % 0x200));
    start = (uint16_t)(start + 0x20 + rnd() % 0x80);
  }
  wr16ram(ram, 0x1800 + 0x14 * 10, 0xFFFF);  // past the end: never reached
  uint16_t car = (uint16_t)(0x35 * (rnd() % 20));
  int k = (int)(rnd() % 5);
  uint16_t pos = (uint16_t)(starts[k] + rnd() % 0x100 - 0x40);
  if(pos >= 0xF000) pos = starts[k];
  r->y = car;
  wr16ram(ram, 0xFAE0 + car, (uint16_t)(0x1800 + 0x14 * k));
  wr16ram(ram, 0xFAB6 + car, pos);
  wr16ram(ram, 0xFADE + car, (uint16_t)(rnd() & 0xF0));
  wr16ram(ram, 0xFAE2 + car, (rnd() & 1) ? 0x7000 : (uint16_t)(pos + rnd() % 0x200 - 0x80));
  wr16ram(ram, 0xFAE4 + car, (uint16_t)(rnd() % 0x200));
  fuzz_rng_state(ram, rnd);
}

// ---- car_road_step ($81:F887) ----  Y = the car; no register outputs (its caller loads A,
// X and Y next)
static void call_car_road_step(const Regs* in, Regs* out) { (void)out; car_road_step(in->y); }

// Negative test: the road rows' right widths one step wider while the C reads them (the
// table restored after, so only the edges and what follows from them differ)
static void call_car_road_step_wrong(const Regs* in, Regs* out) {
  (void)out;
  for(uint32_t a = 0xD1B3; a < 0xD1B3 + 0x1600; a += 2) wr8(a, (uint8_t)(rd8(a) + 1));
  car_road_step(in->y);
  for(uint32_t a = 0xD1B3; a < 0xD1B3 + 0x1600; a += 2) wr8(a, (uint8_t)(rd8(a) - 1));
}

// a car on a segment table (as func_81FBB1's fuzz), the frame step, speeds and drift, the
// car ahead, the road rows around (any words: the bench answers $0A with dsp4_edges)
static void fuzz_car_road_step(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_func_81FBB1(r, ram, rnd);
  uint16_t car = r->y;
  wr16ram(ram, 0x1ECE, (rnd() & 7) == 0 ? 1 : 0);
  wr16ram(ram, 0x1E9E, (uint16_t)(rnd() % 0x300));
  static const uint16_t fields[] = {0xFAB4, 0xFAB9, 0xFABB, 0xFAC9, 0xFACB, 0xFAD7, 0xFAD9, 0xFADB, 0xFAE9};
  for(unsigned i = 0; i < sizeof fields / sizeof fields[0]; i++) wr16ram(ram, (uint32_t)(fields[i] + car), (uint16_t)rnd());
  wr16ram(ram, 0xFACB + (uint32_t)car, (uint16_t)((int16_t)(rnd() % 0x800) - 0x200));
  wr16ram(ram, 0xFAE6 + (uint32_t)car, (rnd() & 1) ? 0 : (uint16_t)(rnd() % 0x400));
  wr16ram(ram, 0xFAE8 + (uint32_t)car, (rnd() & 1) ? 0 : (uint16_t)(rnd() % 0x400));
  uint16_t ahead = (uint16_t)(0x35 * (rnd() % 20));
  wr16ram(ram, 0xFACD + (uint32_t)car, ahead);
  if(rnd() & 1) wr16ram(ram, 0xFAB6 + (uint32_t)ahead, (uint16_t)rnd());
  for(uint32_t a = 0xD1B3; a < 0xD1B3 + 0x1600; a++) ram[a] = (uint8_t)rnd();
}

// ---- player_road_limits ($83:E8AB) ----  no register outputs (it restores DB itself)
static void call_player_road_limits(const Regs* in, Regs* out) { (void)in; (void)out; player_road_limits(); }

// Negative test: the bend flag's bit read as bit 1
static void call_player_road_limits_wrong(const Regs* in, Regs* out) {
  (void)in;
  (void)out;
  uint16_t bend = rd16(0x1F2C);
  wr16(0x1F2C, (uint16_t)(bend ^ 0x0001));
  player_road_limits();
  wr16(0x1F2C, bend);
}

// the rows (any words: the bench answers $0A with dsp4_edges), the car in front, the side
// and push flags (the never-run push and side-flag paths too), the bend
static void fuzz_player_road_limits(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x0224, (uint16_t)rnd());
  wr16ram(ram, 0x0226, (uint16_t)(rnd() % 0x800));
  for(uint32_t a = 0xD1B3; a < 0xD1C5 + 0x1000; a++) ram[a] = (uint8_t)rnd();
  wr16ram(ram, 0x1ECE, (rnd() & 3) == 0 ? 1 : 0);
  wr16ram(ram, 0x1E9C, (uint16_t)rnd());
  uint16_t car = (uint16_t)(0x35 * (rnd() % 20)), ahead = (uint16_t)(0x35 * (rnd() % 20));
  wr16ram(ram, 0x1EBA, car);
  wr16ram(ram, 0xFACD + (uint32_t)car, ahead);
  wr16ram(ram, 0xFABB + (uint32_t)ahead, (uint16_t)((int16_t)(rnd() % 0x400) - 0x200));
  wr16ram(ram, 0x0244, (uint16_t)rnd());
  wr16ram(ram, 0x1F31, (uint16_t)rnd());
  wr16ram(ram, 0x021A, (uint16_t)((rnd() & 1) ? rnd() % 8 : rnd()));
  ram[0x0253] = (rnd() & 1) ? 0 : (uint8_t)rnd();
  ram[0x02B1] = (uint8_t)rnd();
  wr16ram(ram, 0x1F2C, (uint16_t)rnd());
  uint16_t lane = (uint16_t)((rnd() % 14) * 2);
  wr16ram(ram, 0x1EBC, lane);
  wr16ram(ram, 0x3DEC + lane, (uint16_t)((int16_t)(rnd() % 0x100) - 0x80));
  wr16ram(ram, 0x3B08 + lane, (rnd() & 1) ? 0 : (uint16_t)rnd());
  wr16ram(ram, 0x3AEC + lane, (rnd() & 1) ? 0 : (uint16_t)rnd());
  wr16ram(ram, 0x0206, (uint16_t)((int16_t)(rnd() % 0x400) - 0x200));
  wr16ram(ram, 0x0214, (uint16_t)rnd());
  wr16ram(ram, 0x0216, (uint16_t)rnd());
}

// ---- the shop's leaves ($80:98xx-$80:B2xx) ----
// part levels: Y = the car's offset; A = the level + 1 (N/Z from the INC)
#define SHOP_LEVEL(fn) \
  static void call_##fn(const Regs* in, Regs* out) { out->a = fn(in->y); setNZ16(out, out->a); }
SHOP_LEVEL(shop_part_level_2)
SHOP_LEVEL(shop_part_level_3)
SHOP_LEVEL(shop_part_level_4)
SHOP_LEVEL(shop_part_level_5)
SHOP_LEVEL(shop_part_level_6)
SHOP_LEVEL(shop_part_level_7)
static void fuzz_shop_part_level(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->y = (uint16_t)(rnd() % 0x40);
  for(int i = 0; i < 2; i++) wr16ram(ram, (uint32_t)(0x0392 + r->y + 2 * i), (uint16_t)rnd());
  for(uint32_t a = 0x0392; a < 0x03E0; a++) ram[a] = (uint8_t)rnd();
}

// A = 4 (Z from the TSB: bit 2 of $76 before), X = the car
static void call_shop_money_take(const Regs* in, Regs* out) {
  (void)in;
  bool was = rd16(DP + 0x76) & 0x0004;
  shop_money_take();
  out->a = 0x0004;
  out->x = rd16(DP + 0x94);
  out->n = false;
  out->z = !was;
}
static void fuzz_shop_money_take(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint16_t car = (uint16_t)(rnd() % 0x40);
  wr16ram(ram, DP + 0x94, car);
  for(uint32_t a = 0x038E; a < 0x03E0; a++) ram[a] = (uint8_t)rnd();
  wr16ram(ram, DP + 0x76, (uint16_t)rnd());
}

// A kept (PLA: N/Z from it)
static void call_shop_flag_fb(const Regs* in, Regs* out) { shop_flag_fb(); setNZ16(out, in->a); }

// as func_80B537's adapter, with its inputs set here (Y ends as $5C + $0400)
static void call_shop_panel_upload(const Regs* in, Regs* out) {
  (void)in;
  uint16_t i = rd16(DP + 0x8C), base = rd16(DP + 0x8A);
  out->y = (uint16_t)(rd16(DP + 0x5C) + 0x0400);
  out->a = shop_panel_upload();
  out->x = (uint16_t)(i + 2);
  setNZ16(out, out->a);
  setAdc16(out, (uint16_t)((i + 2) & 0x3F), base);
}
static void fuzz_shop_panel_upload(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x5C, (uint16_t)rnd());
  wr16ram(ram, DP + 0x5E, (uint16_t)rnd());
  wr16ram(ram, DP + 0x8A, (uint16_t)(rnd() & 0x00FF));
  wr16ram(ram, DP + 0x8C, (uint16_t)(rnd() & 0x003E));
}

// the shop's item table: list $48 (1-6), row 0-4
static uint16_t shopEntry(uint16_t row) {
  return (uint16_t)((row << 4) + rom16(0x81A892 + (uint32_t)(uint16_t)(rd16(DP + 0x48) << 1)));
}
// lists 1-5 have 5 rows; list 6 has one (its rows 1-4 read past the table: texts of up to
// 64 KB that the MVN would copy over the I/O registers)
static void fuzz_shop_item(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint16_t list = (uint16_t)(1 + rnd() % 6), rows = list == 6 ? 1 : 5;
  wr16ram(ram, DP + 0x48, list);
  wr16ram(ram, DP + 0x4A, (uint16_t)(rnd() % rows));
  wr16ram(ram, DP + 0x4C, (uint16_t)(rnd() % rows));
  wr16ram(ram, DP + 0xC4, (uint16_t)(rnd() & 0x03FE));
}

// as func_80B58D's adapter: A = 9; X = the picture's table word
static void call_shop_item_picture(const Regs* in, Regs* out) {
  (void)in;
  out->x = rom16(0x81A89D + (uint32_t)shopEntry(rd16(DP + 0x4C)));
  shop_item_picture();
  out->a = 9;
  setNZ16(out, 9);
}

// after the MVN: A = $FFFF, X and Y past the block; PLB of $81 sets N
static void call_shop_item_text(const Regs* in, Regs* out) {
  (void)in;
  uint16_t text = rom16(0x81A895 + (uint32_t)shopEntry(rd16(DP + 0x4A)));
  uint16_t size = rom16(0x81A894 + (uint32_t)text), src = rom16(0x81A892 + (uint32_t)text);
  uint16_t dest = (uint16_t)(rd16(DP + 0xC4) + 0x0100);
  shop_item_text();
  out->a = 0xFFFF;
  out->x = (uint16_t)(src + size);
  out->y = (uint16_t)(dest + size);
  out->n = true;
  out->z = false;
}

// A = the new slot (flags of its ADC); X (cap) or Y (column) the old slot
static void call_shop_bar_cap(const Regs* in, Regs* out) {
  (void)in;
  uint16_t slot = rd16(DP + 0x74);
  out->a = shop_bar_cap();
  out->x = slot;
  setNZ16(out, out->a);
  setAdc16(out, slot, 0x0010);
}
static void call_shop_bar_column(const Regs* in, Regs* out) {
  (void)in;
  uint16_t slot = rd16(DP + 0x74);
  out->a = shop_bar_column();
  out->y = slot;
  setNZ16(out, out->a);
  setAdc16(out, slot, 0x000C);
}
static void fuzz_shop_bar(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x74, (uint16_t)((rnd() % 0x70) * 4));
  static const uint32_t fields[] = {0x7A, 0xE7, 0xE8, 0x80, 0x14, 0x7E, 0x84};
  for(unsigned i = 0; i < sizeof fields / sizeof fields[0]; i++) wr16ram(ram, DP + fields[i], (uint16_t)rnd());
}

// the bar routines: no register outputs (their callers reload A, X and Y)
static void call_shop_bar_levels(const Regs* in, Regs* out) { (void)in; (void)out; shop_bar_levels(); }
static void call_shop_bar_segments(const Regs* in, Regs* out) { (void)in; (void)out; shop_bar_segments(); }
static void call_shop_bar_segments_long(const Regs* in, Regs* out) { (void)in; (void)out; shop_bar_segments_long(); }
static void call_shop_bar_double(const Regs* in, Regs* out) { (void)in; (void)out; shop_bar_double(); }
static void call_shop_gauge(const Regs* in, Regs* out) { (void)in; (void)out; shop_gauge(); }

// A = the old D+$14 (N/Z of its PLA); X and Y as sprite_number_centered leaves them, with
// C and V from its last ADC
static void call_shop_item_price(const Regs* in, Regs* out) {
  (void)in;
  uint16_t base = rd16(DP + 0x14);
  uint16_t price = rom16(0x81A89B + (uint32_t)shopEntry(rd16(DP + 0x4A)));
  uint32_t value = (uint32_t)rom16(0x81A88E + (uint32_t)price) << 16 | rom16(0x81A890 + (uint32_t)price);
  uint16_t shift = shop_item_price();
  uint16_t end = rd16(DP + 0x74);
  uint8_t post = rd8((uint16_t)(0x0900 + end - 4)), pre = (uint8_t)(post - (uint8_t)shift);
  out->y = end;
  out->x = value == 0 ? 0xFFFE : 0xFFFF;
  out->a = base;
  setNZ16(out, base);
  out->c = true;
  out->v = (~(pre ^ (uint8_t)shift) & (pre ^ post) & 0x80) != 0;
}
static void fuzz_shop_item_price(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_shop_item(r, ram, rnd);
  wr16ram(ram, DP + 0x3E, (uint16_t)(rnd() % 0x60));
  wr16ram(ram, DP + 0x74, (uint16_t)((rnd() % 0x40) * 4));
  wr16ram(ram, DP + 0xE7, (uint16_t)rnd());
  wr16ram(ram, DP + 0x14, (uint16_t)rnd());
  wr16ram(ram, DP + 0x16, (uint16_t)rnd());
  for(uint32_t a = 0x4DCA; a < 0x4DCA + 0x20; a++) ram[a] = (uint8_t)rnd();
}

// the item-view handlers: no register outputs (the dispatcher's caller reloads them)
static void call_shop_item_view_a(const Regs* in, Regs* out) { (void)out; shop_item_view_a(in->y); }
static void call_shop_item_view_b(const Regs* in, Regs* out) { (void)out; shop_item_view_b(in->y); }
static void call_shop_item_view_c(const Regs* in, Regs* out) { (void)out; shop_item_view_c(in->y); }
static void call_shop_item_view_d(const Regs* in, Regs* out) { (void)out; shop_item_view_d(in->y); }
static void call_shop_item_view_e(const Regs* in, Regs* out) { (void)out; shop_item_view_e(in->y); }
static void call_shop_item_view_g(const Regs* in, Regs* out) { (void)out; shop_item_view_g(in->y); }
// lists 6-8's views and their helpers: only the view dispatch reaches them (rows for when a
// movie does); no register outputs
static void call_shop_item_view_h(const Regs* in, Regs* out) { (void)out; shop_item_view_h(in->y); }
static void call_shop_item_view_i(const Regs* in, Regs* out) { (void)out; shop_item_view_i(in->y); }
static void call_shop_item_view_j(const Regs* in, Regs* out) { (void)out; shop_item_view_j(in->y); }
static void call_shop_level_blocks(const Regs* in, Regs* out) { (void)in; (void)out; shop_level_blocks(); }
static void call_shop_gauge_lines(const Regs* in, Regs* out) { (void)in; (void)out; shop_gauge_lines(); }
static void call_shop_gauge_line(const Regs* in, Regs* out) { (void)out; shop_gauge_line((uint8_t)in->a, in->y, in->x); }
static void call_shop_item_dispatch_view(const Regs* in, Regs* out) { (void)in; (void)out; shop_item_dispatch_view(); }
// the panel dispatch: X = the entry (its PLX: N, Z); C and V of the last queue ADC (one
// upload for lists 0 and 6-8, two for the rest)
static void call_shop_item_dispatch_panel(const Regs* in, Regs* out) {
  (void)in;
  uint16_t list = rd16(DP + 0x48);
  uint16_t entry = (uint16_t)((rd16(DP + 0x4A) << 4) + rom16(0x81A892 + (uint32_t)(uint16_t)(list << 1)));
  uint16_t i = rd16(DP + 0x8C), base = rd16(DP + 0x8A);
  if(list >= 1 && list <= 5) i = (uint16_t)(((i + 2) & 0x3F) + base);  // the first upload's slot
  shop_item_dispatch_panel();
  setAdc16(out, (uint16_t)((i + 2) & 0x3F), base);
  out->x = entry;
  setNZ16(out, entry);
}
// the slides: no register outputs (their callers return or reload them); the last CMP
// matched: N clear, Z and C set
static void slideFlags(Regs* out) {
  out->n = false;
  out->z = true;
  out->c = true;
}
static void call_shop_item_slide_row(const Regs* in, Regs* out) { (void)in; shop_item_slide_row(); slideFlags(out); }
static void call_shop_item_slide_list(const Regs* in, Regs* out) { (void)in; shop_item_slide_list(); slideFlags(out); }
static void call_shop_item_slide_list_in(const Regs* in, Regs* out) { (void)in; shop_item_slide_list_in(); slideFlags(out); }
// the list moves: no register outputs. A slide's exit flags (N clear, Z and C set); leaving,
// the PLA of the routine's own return address (N, Z), C still set by the panel's CMP
static void shopItemMoveOut(const Regs* in, Regs* out, bool left) {
  out->leftCaller = left;
  if(left) {
    setNZ16(out, rd16((uint16_t)(in->sp + 1)));
    out->c = true;
  } else {
    slideFlags(out);
  }
}
static void call_shop_item_down(const Regs* in, Regs* out) { shopItemMoveOut(in, out, shop_item_down()); }
static void call_shop_item_up(const Regs* in, Regs* out) { shopItemMoveOut(in, out, shop_item_up()); }
// the icon's show and settle: no register outputs; show ends on its LDA of the step (N set),
// settle on the TRB (Z of $8000 & D+$76: clear, the bit was set)
static void call_shop_item_show_icon(const Regs* in, Regs* out) {
  (void)in;
  shop_item_show_icon();
  setNZ16(out, rd16(DP + 0x44));
}
static void call_shop_item_icon_settle(const Regs* in, Regs* out) {
  (void)in;
  bool set = (rd16(DP + 0x76) & 0x8000) != 0;
  shop_item_icon_settle();
  out->n = true;  // LDA #$8000 before the TRB
  out->z = !set;
  out->c = true;  // the CMP #$8007 that matched
}
// a 16-bit SBC in decimal mode, as the bench's CPU computes it (result and V)
static uint16_t sbcDecimal16(uint16_t a, uint16_t b, bool carry, bool* v) {
  uint16_t value = (uint16_t)(b ^ 0xFFFF);
  int result = (a & 0xF) + (value & 0xF) + carry;
  if(result < 0x10) result = (result - 0x6) & ((result - 0x6 < 0) ? 0xF : 0x1F);
  result = (a & 0xF0) + (value & 0xF0) + result;
  if(result < 0x100) result = (result - 0x60) & ((result - 0x60 < 0) ? 0xFF : 0x1FF);
  result = (a & 0xF00) + (value & 0xF00) + result;
  if(result < 0x1000) result = (result - 0x600) & ((result - 0x600 < 0) ? 0xFFF : 0x1FFF);
  result = (a & 0xF000) + (value & 0xF000) + result;
  *v = (a & 0x8000) == (value & 0x8000) && (value & 0x8000) != (result & 0x8000);
  if(result < 0x10000) result -= 0x6000;
  return (uint16_t)result;
}
// the price check: C = can pay; short, N clear and Z set by the CPX #$20 that ended the
// flashes
// can pay: N, V, Z of the high word's decimal SBC (its borrow from the low word's CMP)
static void call_shop_item_affordable(const Regs* in, Regs* out) {
  (void)in;
  uint16_t entry = (uint16_t)((rd16(DP + 0x4A) << 4) + rom16(0x81A892 + (uint32_t)(uint16_t)(rd16(DP + 0x48) << 1)));
  const uint32_t price = 0x81A892 + (uint32_t)rom16(0x81A89B + (uint32_t)entry);
  bool v, low = rd16(DP + 0x52) >= rom16(price - 2);
  uint16_t high = sbcDecimal16(rd16(DP + 0x54), rom16(price - 4), low, &v);
  out->c = shop_item_affordable();
  if(out->c) {
    setNZ16(out, high);
    out->v = v;
  } else {
    out->n = false;
    out->z = true;
  }
}
// the money (BCD) at or above the item's price: the fuzz has no frames to wait through
static void fuzz_shop_item_affordable(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint16_t list = (uint16_t)(rnd() % 9);
  wr16ram(ram, DP + 0x48, list);
  wr16ram(ram, DP + 0x4A, (uint16_t)(list == 0 || list >= 6 ? 0 : rnd() % 5));
  uint16_t entry = (uint16_t)((ram[DP + 0x4A] << 4) + rom16(0x81A892 + (uint32_t)(uint16_t)(list << 1)));
  const uint32_t price = 0x81A892 + (uint32_t)rom16(0x81A89B + (uint32_t)entry);
  uint32_t cost = (uint32_t)rom16(price - 4) << 16 | rom16(price - 2);
  uint32_t money;
  do {  // a random BCD amount, kept if it pays
    money = 0;
    for(int d = 0; d < 8; d++) money = money << 4 | (rnd() % 10);
    if((rnd() & 3) == 0) money = cost;
  } while(money < cost);
  wr16ram(ram, DP + 0x52, (uint16_t)money);
  wr16ram(ram, DP + 0x54, (uint16_t)(money >> 16));
}
// the frame: no register outputs (every caller reloads them)
static void call_shop_item_redraw(const Regs* in, Regs* out) { (void)in; (void)out; shop_item_redraw(); }
// no register outputs: every caller reloads A, X and Y ($80:9E49, $80:9B99, LDA #2)
static void call_shop_item_icon_draw(const Regs* in, Regs* out) { (void)out; shop_item_icon_draw(in->x); }
// A = $B149 (N set, Z clear), X = D+$5E + 16, Y = 0; C and V kept
static void call_shop_panel_blank(const Regs* in, Regs* out) {
  (void)in;
  uint16_t x = rd16(DP + 0x5E);
  shop_panel_blank();
  out->a = 0xB149;
  out->x = (uint16_t)(x + 16);
  out->y = 0;
  setNZ16(out, 0xB149);
}
// A = 0, X = D+$5E + 18, Y = 0 (N clear, Z set by the DEY), C set by the CMP that matched
static void call_shop_panel_clear(const Regs* in, Regs* out) {
  (void)in;
  shop_panel_clear();
  out->a = 0;
  out->x = (uint16_t)(rd16(DP + 0x5E) + 18);
  out->y = 0;
  setNZ16(out, 0);
  out->c = true;
}
// the map offset anywhere its rows stay in bank $7F; the queue empty or not
static void fuzz_shop_panel(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x5E, (uint16_t)((rnd() % 0x3000) & ~1u));
  wr16ram(ram, DP + 0x4E, (uint16_t)rnd());
  wr16ram(ram, DP + 0xEC, (uint16_t)rnd());
}
// the queue already empty: the fuzz has no interrupts to replay
static void fuzz_shop_panel_clear(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_shop_panel(r, ram, rnd);
  uint16_t i = (uint16_t)rnd();
  wr16ram(ram, DP + 0x8C, i);
  wr16ram(ram, DP + 0x8E, i);
}

// A = the key (V of its ADC), C = locked; N Z of the CMP #$1800 (or of the ADC when negative)
static void shopListLockedOut(Regs* out) {
  uint16_t a = (uint16_t)(rd16(DP + 0x48) << 3);
  a = (uint16_t)((a << 8) | (a >> 8));
  uint16_t key = (uint16_t)(a + rd16(DP + 0x4A));
  out->a = key;
  setAdc16(out, a, rd16(DP + 0x4A));
  out->c = shop_list_locked();
  if(key & 0x8000) setNZ16(out, key);
  else setNZ16(out, (uint16_t)(key - 0x1800));
}
static void call_shop_list_locked(const Regs* in, Regs* out) { (void)in; shopListLockedOut(out); }
// flags only (the callers reload A): locked, as shop_list_locked; V of its ADC throughout.
// prev: N Z of the old D+$48 (its LDA; a step's DEC is in memory and the PLA restores it), C clear.
// next: at D+$F0, N Z C of the CMP; a step, N Z of the PLA'd old D+$48, C of the CMP.
static void call_shop_list_prev(const Regs* in, Regs* out) {
  (void)in;
  shopListLockedOut(out);
  uint16_t list = rd16(DP + 0x48);
  bool locked = out->c;
  shop_list_prev();
  if(locked) return;
  setNZ16(out, list);
}
static void call_shop_list_next(const Regs* in, Regs* out) {
  (void)in;
  shopListLockedOut(out);
  uint16_t list = rd16(DP + 0x48), limit = rd16(DP + 0xF0);
  bool locked = out->c;
  shop_list_next();
  if(locked) return;
  setNZ16(out, list == limit ? 0 : list);
  out->c = list >= limit;
}
// lists 0-7 and rows 0-4 mostly, the limit D+$F0 near them
static void fuzz_shop_list(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  bool wild = (rnd() & 7) == 0;
  wr16ram(ram, DP + 0x48, wild ? (uint16_t)rnd() : (uint16_t)(rnd() % 8));
  wr16ram(ram, DP + 0x4A, wild ? (uint16_t)rnd() : (uint16_t)(rnd() % 5));
  wr16ram(ram, DP + 0xF0, (uint16_t)(rnd() % 8));
  wr16ram(ram, DP + 0x4E, (uint16_t)rnd());
}

// no register outputs: func_809DD0 follows at every call
static void call_shop_level_marks(const Regs* in, Regs* out) { (void)in; (void)out; shop_level_marks(); }
// lists 0-9 (the getter table), 1-8 marks, any level, cursor and position, slots in the
// buffer
static void fuzz_shop_level_marks(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint16_t list = (uint16_t)(rnd() % 10), car = (uint16_t)(rnd() & 0x1F);
  wr16ram(ram, DP + 0x48, list);
  wr16ram(ram, DP + 0x94, car);
  ram[0x036E + list] = (uint8_t)(1 + rnd() % 8);
  for(uint32_t a = 0x0392; a < 0x039E; a++) ram[a + car] = (uint8_t)rnd();
  wr16ram(ram, DP + 0x4A, (uint16_t)(rnd() % 5));
  wr16ram(ram, DP + 0xEE, (rnd() & 1) ? (uint16_t)(0x14 + (rnd() % 0x50)) : (uint16_t)rnd());
  wr16ram(ram, DP + 0xE7, (uint16_t)rnd());
  wr16ram(ram, DP + 0x74, (uint16_t)((rnd() % 0x60) * 4));
}
// carry set: the queue's ADC sets V and C, the PLY of the old Y sets N and Z; carry clear:
// V from the icon's last group item ADC #6 (as on entry if the group is empty), or from
// BIT D+$76 at step 7
static void shopItemIconOut(const Regs* in, Regs* out, void (*fn)(bool), uint16_t field, uint16_t icons) {
  uint16_t i = rd16(DP + 0x8C), base = rd16(DP + 0x8A), step = rd16(DP + 0x44);
  uint16_t level = (uint16_t)(rd16(0x0393 + (uint32_t)field + rd16(DP + 0x94)) & 0x007F);
  GroupWalk w = sprite_group_walk(0x7E0000 | rom16(0x810000 + (uint32_t)icons + (uint32_t)(uint16_t)(level << 1)));
  fn(in->c);
  if(in->c) {
    setAdc16(out, (uint16_t)((i + 2) & 0x3F), base);
    setNZ16(out, in->y);
    return;
  }
  if(w.any) setAdc16(out, w.lastY, 6);
  if(step == 0x0007) out->v = (rd16(DP + 0x76) & 0x4000) != 0;
}
static void call_shop_item_icon_1(const Regs* in, Regs* out) { shopItemIconOut(in, out, shop_item_icon_1, 0, 0xBC2E); }
// the dispatches: lists 1-5's handlers as their rows; lists 6-8 likewise on fields 5-7; list 0
// a fixed icon. No upload (list 0, or a level-0 part of lists 6-8): C set by the SEC, V of
// the entry's ADC. The upload dispatch ends with PLX of the entry (X, N, Z).
static void shopItemDispatchOut(const Regs* in, Regs* out, bool upload) {
  static const uint16_t icons[8] = {0xBC2E, 0xBC52, 0xBC76, 0xBCBE, 0xBC9A, 0xBCFA, 0xBCF6, 0xBCF2};
  uint16_t list = rd16(DP + 0x48);
  uint16_t entry = (uint16_t)((rd16(DP + 0x4A) << 4) + rom16(0x81A892 + (uint32_t)(uint16_t)(list << 1)));
  uint16_t i = rd16(DP + 0x8C), base = rd16(DP + 0x8A), step = rd16(DP + 0x44);
  uint16_t field = (uint16_t)(list - 1);
  uint16_t level = list ? (uint16_t)(rd16(0x0393 + (uint32_t)field + rd16(DP + 0x94)) & 0x007F) : 0;
  uint16_t group = list ? rom16(0x810000 + (uint32_t)icons[field] + (uint32_t)(uint16_t)(level << 1)) : 0x4F6A;
  GroupWalk w = sprite_group_walk(0x7E0000 | group);
  if(upload) shop_item_dispatch_upload();
  else shop_item_dispatch_icon();
  if(upload) {
    if(list == 0 || (list >= 6 && level == 0)) {  // no upload: V of the entry's ADC, C of the SEC
      setAdc16(out, (uint16_t)(rd16(DP + 0x4A) << 4), rom16(0x81A892 + (uint32_t)(uint16_t)(list << 1)));
      out->c = true;
    }
    else setAdc16(out, (uint16_t)((i + 2) & 0x3F), base);
    out->x = entry;
    setNZ16(out, entry);
    return;
  }
  if(w.any) setAdc16(out, w.lastY, 6);
  if(step == 0x0007) out->v = (rd16(DP + 0x76) & 0x4000) != 0;
}
static void call_shop_item_dispatch_icon(const Regs* in, Regs* out) { shopItemDispatchOut(in, out, false); }
static void call_shop_item_dispatch_upload(const Regs* in, Regs* out) { shopItemDispatchOut(in, out, true); }
static void call_shop_item_icon_2(const Regs* in, Regs* out) { shopItemIconOut(in, out, shop_item_icon_2, 1, 0xBC52); }
static void call_shop_item_icon_3(const Regs* in, Regs* out) { shopItemIconOut(in, out, shop_item_icon_3, 2, 0xBC76); }
static void call_shop_item_icon_4(const Regs* in, Regs* out) { shopItemIconOut(in, out, shop_item_icon_4, 3, 0xBCBE); }
static void call_shop_item_icon_5(const Regs* in, Regs* out) { shopItemIconOut(in, out, shop_item_icon_5, 4, 0xBC9A); }
static void call_shop_item_view_f(const Regs* in, Regs* out) { (void)out; shop_item_view_f(in->y); }
// the ring: no register outputs (its view reloads them); its pieces: A = 0, X = the frame
// x 6, Y = slot + 4 (N/Z of the INY), C and V of the y position's ADC
static void call_shop_ring(const Regs* in, Regs* out) { (void)in; (void)out; shop_ring(); }
static void ringPieceOut(const Regs* in, Regs* out, uint32_t table, uint16_t slot) {
  uint16_t x = (uint16_t)(((in->a >> 8) & 0x000F) * 6);
  out->a = 0;
  out->x = x;
  out->y = slot;
  setNZ16(out, slot);
  (void)table;
}
static void call_shop_ring_piece_front(const Regs* in, Regs* out) {
  uint16_t base = rd16(DP + 0x18), add = rd16(0x4D0E + (uint32_t)(((in->a >> 8) & 0x000F) * 6));
  ringPieceOut(in, out, 0x4D0A, shop_ring_piece_front(in->a, in->y));
  setAdc16(out, base, add);
}
static void call_shop_ring_piece_back(const Regs* in, Regs* out) {
  uint16_t base = rd16(DP + 0x18), add = rd16(0x4D6E + (uint32_t)(((in->a >> 8) & 0x000F) * 6));
  ringPieceOut(in, out, 0x4D6A, shop_ring_piece_back(in->a, in->y));
  setAdc16(out, base, add);
}
static void fuzz_shop_ring(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->y = (uint16_t)((rnd() % 0x40) * 4);
  wr16ram(ram, DP + 0x74, r->y);
  static const uint32_t fields[] = {0x14, 0x18, 0x1A, 0x40, 0x7A, 0xE8};
  for(unsigned i = 0; i < sizeof fields / sizeof fields[0]; i++) wr16ram(ram, DP + fields[i], (uint16_t)rnd());
  for(uint32_t a = 0x4D0A; a < 0x4DCA; a++) ram[a] = (uint8_t)rnd();
}
static void fuzz_shop_bars(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
// an item whose view is this handler (its word at $81:A897 + the entry): its bar entry in Y
// (from $81:A8A1), the stage sometimes at $7FFF (the never-run stop), the bars' inputs
static void fuzz_shop_item_view_of(Regs* r, uint8_t* ram, uint32_t (*rnd)(void), uint16_t view) {
  fuzz_shop_bars(r, ram, rnd);
  for(int tries = 0; tries < 64; tries++) {
    fuzz_shop_item(r, ram, rnd);
    uint16_t list = (uint16_t)(ram[DP + 0x48] | ram[DP + 0x49] << 8), row = (uint16_t)(ram[DP + 0x4A] | ram[DP + 0x4B] << 8);
    uint16_t entry = (uint16_t)((row << 4) + rom16(0x81A892 + (uint32_t)(uint16_t)(list << 1)));
    r->y = rom16(0x81A8A1 + (uint32_t)entry);
    if(rom16(0x81A897 + (uint32_t)entry) == view) break;
  }
  wr16ram(ram, DP + 0x16, (uint16_t)rnd());
  wr16ram(ram, DP + 0xC4, (uint16_t)(0x0100 + (rnd() % 0x10) * 0x40));  // as fuzz_shop_bars: off the direct page
  if((rnd() & 7) == 0) wr16ram(ram, DP + 0x3E, 0x7FFF);
}
static void fuzz_shop_item_view_a(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { fuzz_shop_item_view_of(r, ram, rnd, 0xA2CB); }
static void fuzz_shop_item_view_b(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { fuzz_shop_item_view_of(r, ram, rnd, 0xA831); }
static void fuzz_shop_item_view_c(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { fuzz_shop_item_view_of(r, ram, rnd, 0xAA15); }
static void fuzz_shop_item_view_d(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { fuzz_shop_item_view_of(r, ram, rnd, 0xA717); }
// the view dispatch: any list 0-8 (0 and 6-8 one row each), the bars' inputs as the views',
// the stage sometimes at its stop, and every table a view reads: the wave ($7E:4E1A), the
// ring ($7E:4D0A), the gauge's palette ($7E:4F5A) and counter (D+$46), the spin (D+$40)
static void fuzz_shop_item_dispatch_view(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_shop_bars(r, ram, rnd);
  uint16_t list = (uint16_t)(rnd() % 9);
  wr16ram(ram, DP + 0x48, list);
  wr16ram(ram, DP + 0x4A, (uint16_t)(list == 0 || list >= 6 ? 0 : rnd() % 5));
  wr16ram(ram, DP + 0x16, (uint16_t)rnd());
  wr16ram(ram, DP + 0xC4, (uint16_t)(0x0100 + (rnd() % 0x10) * 0x40));
  if((rnd() & 7) == 0) wr16ram(ram, DP + 0x3E, 0x7FFF);
  wr16ram(ram, DP + 0x40, (uint16_t)rnd());
  wr16ram(ram, DP + 0x46, (uint16_t)rnd());
  for(uint32_t a = 0x4D0A; a < 0x4DCA; a++) ram[a] = (uint8_t)rnd();
  for(uint32_t a = 0x4E1A; a < 0x4EDA; a++) ram[a] = (uint8_t)rnd();
  for(uint32_t a = 0x4F5A; a < 0x4F62; a++) ram[a] = (uint8_t)rnd();
}
// the panel dispatch: any list 0-8 and row, the queue as func_80B537's fuzz
static void fuzz_shop_item_dispatch_panel(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint16_t list = (uint16_t)(rnd() % 9);
  wr16ram(ram, DP + 0x48, list);
  wr16ram(ram, DP + 0x4A, (uint16_t)(list == 0 || list >= 6 ? 0 : rnd() % 5));
  wr16ram(ram, DP + 0x5C, (uint16_t)rnd());
  wr16ram(ram, DP + 0x5E, (uint16_t)rnd());
  // the ring's base a multiple of $40 (as the game's): a second upload's slot is base + 0-$3E
  wr16ram(ram, DP + 0x8A, (uint16_t)((rnd() % 0x14) * 0x40));
  wr16ram(ram, DP + 0x8C, (uint16_t)(rnd() % 0x500));
}
static void fuzz_text_type_step(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
// the frame: the view dispatch's inputs, a typewriter text, the upload queue empty or not,
// and the parts' levels within their icon tables (lists 1-5: 0-5; 6-8: 0-3), the icon's
// group being a list walked to its end
static void fuzz_shop_item_redraw(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_shop_item_dispatch_view(r, ram, rnd);
  fuzz_text_type_step(r, ram, rnd);
  uint16_t car = (uint16_t)(rnd() & 0x1F);
  wr16ram(ram, DP + 0x94, car);
  for(uint32_t f = 0; f < 8; f++) ram[0x0393 + car + f] = (uint8_t)((rnd() % (f < 5 ? 6 : 4)) | (rnd() & 0x80));
  uint16_t list = (uint16_t)(ram[DP + 0x48] | ram[DP + 0x49] << 8);
  ram[0x036E + list] = (uint8_t)(1 + rnd() % 8);
  uint16_t q = (uint16_t)((rnd() % 0x20) * 2);
  wr16ram(ram, DP + 0x8C, q);
  wr16ram(ram, DP + 0x8E, (rnd() & 1) ? q : (uint16_t)(q + 2));
  wr16ram(ram, DP + 0x8A, (uint16_t)(rnd() & 0xC0));
}
// list 0's view: its one row
static void fuzz_shop_item_view_g(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_shop_item_view_of(r, ram, rnd, 0xA2CB);  // the bars and the rest
  wr16ram(ram, DP + 0x48, 0);
  wr16ram(ram, DP + 0x4A, 0);
  r->y = rom16(0x81A8A1 + (uint32_t)rom16(0x81A892));
}

// the icon: its step anywhere in 0-15 or done (bit 15), the position, D+$76 and the colour
// tables; the palette copy off the direct page as the views'; the panel queue as
// func_80B537's fuzz; the five part levels 0-5 (the tables' rows), bit 7 either way
static void fuzz_shop_item_icon(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->c = rnd() & 1;
  wr16ram(ram, DP + 0x44, (rnd() & 3) == 0 ? (uint16_t)(0x8000 | (rnd() & 0x7FFF)) : (uint16_t)(rnd() % 16));
  wr16ram(ram, DP + 0x42, (uint16_t)rnd());
  wr16ram(ram, DP + 0xE8, (uint16_t)rnd());
  wr16ram(ram, DP + 0x76, (uint16_t)rnd());
  wr16ram(ram, DP + 0xC4, (uint16_t)(0x0100 + (rnd() % 0x10) * 0x40));
  for(uint32_t a = 0x4EDA; a < 0x4F5A; a++) ram[a] = (uint8_t)rnd();
  wr16ram(ram, DP + 0x5C, (uint16_t)rnd());
  wr16ram(ram, DP + 0x5E, (uint16_t)rnd());
  wr16ram(ram, DP + 0x8A, (rnd() & 1) ? (uint16_t)(rnd() & 0xC0) : (uint16_t)rnd());
  wr16ram(ram, DP + 0x8C, (uint16_t)(rnd() % 0x500));
  uint16_t p = (uint16_t)(rnd() & 0x1F);
  wr16ram(ram, DP + 0x94, p);
  for(uint32_t f = 0; f < 5; f++) ram[0x0393 + p + f] = (uint8_t)((rnd() % 6) | (rnd() & 0x80));
}
// a dispatch: lists 0-8 (0 and 6-8 one row each), as fuzz_shop_item_icon; lists 6-8's
// levels 0-3
static void fuzz_shop_item_dispatch(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_shop_item_icon(r, ram, rnd);
  uint16_t list = (uint16_t)(rnd() % 9);
  wr16ram(ram, DP + 0x48, list);
  wr16ram(ram, DP + 0x4A, (uint16_t)(list == 0 || list >= 6 ? 0 : rnd() % 5));
  uint16_t car = (uint16_t)(ram[DP + 0x94] | ram[DP + 0x95] << 8);
  for(uint32_t f = 0; f < 3; f++) ram[0x0398 + car + f] = (uint8_t)((rnd() % 4) | (rnd() & 0x80));
}
// the icon alone: X = a list's icon for a level 0-5
static void fuzz_shop_item_icon_draw(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  static const uint16_t icons[] = {0xBC2E, 0xBC52, 0xBC76, 0xBCBE, 0xBC9A};
  fuzz_shop_item_icon(r, ram, rnd);
  r->x = rom16(0x810000 + (uint32_t)icons[rnd() % 5] + (uint32_t)((rnd() % 6) << 1));
}
static void fuzz_shop_item_view_f(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_shop_item_view_of(r, ram, rnd, 0xA333);
  wr16ram(ram, DP + 0x40, (uint16_t)rnd());
  for(uint32_t a = 0x4D0A; a < 0x4DCA; a++) ram[a] = (uint8_t)rnd();
}
static void fuzz_shop_item_view_e(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_shop_item_view_of(r, ram, rnd, 0xAB54);
  wr16ram(ram, DP + 0x40, (uint16_t)rnd());
  for(uint32_t a = 0x4E1A; a < 0x4EDA; a++) ram[a] = (uint8_t)rnd();
}

// the stage 0-$5F, the bar table index (live values), the rest as fuzz_shop_bar
static void fuzz_shop_bars(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_shop_bar(r, ram, rnd);
  wr16ram(ram, DP + 0x74, (uint16_t)((rnd() % 0x40) * 4));
  wr16ram(ram, DP + 0x3E, (uint16_t)(rnd() % 0x60));
  wr16ram(ram, DP + 0x7C, (uint16_t)((rnd() % 6) * 2));
  // the level tiles go to $012E-$013F + D+$C4: keep them off the direct page's own
  // variables (a tile on $01C4 or $013E changes where the next ones go)
  wr16ram(ram, DP + 0xC4, (uint16_t)(0x0100 + (rnd() % 0x10) * 0x40));
}

// A = Y = slot + $10 (flags of the ADC)
static void call_shop_sprite_block(const Regs* in, Regs* out) {
  out->a = out->y = shop_sprite_block(in->a, in->y);
  setNZ16(out, out->a);
  setAdc16(out, in->y, 0x0010);
}
static void fuzz_shop_sprite_block(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->y = (uint16_t)((rnd() % 0x20) * 0x10);
  wr16ram(ram, DP + 0x7E, (uint16_t)rnd());
  wr16ram(ram, DP + 0x16, (uint16_t)rnd());
}

// ---- take_nibble16 ($80:85CA) ----  A = the nibble (N/Z from the last ROL), C = 0.
static void call_take_nibble16(const Regs* in, Regs* out) {
  (void)in;
  out->a = take_nibble16();
  setNZ16(out, out->a);
  out->c = false;
}

static void fuzz_take_nibble16(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x1A, (uint16_t)rnd());
}

// ---- func_82B78D ($82:B78D) ----  exits with A = $0158 (N/Z from the AND), X = the entry
// offset, C/V of the second SBC #2.
static void call_func_82B78D(const Regs* in, Regs* out) {
  uint16_t x = (uint16_t)(in->a >> 2) & 0xFFFC;
  uint16_t word = rd16(0x4F32 + (uint32_t)x);
  out->a = func_82B78D(in->a);
  out->x = x;
  setNZ16(out, out->a);
  setSbc16(out, word, 2);
}

static void fuzz_func_82B78D(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->a = (uint16_t)rnd();
  uint16_t x = (uint16_t)(r->a >> 2) & 0xFFFC;
  for(int i = 0; i < 3; i++) ram[0x4F31 + x + i] = (uint8_t)rnd();
}

// ---- func_82B7B0 ($82:B7B0) ----  exits with A = the last AND, X = the block offset, Y =
// the mask pointer, C = 0 (an ASL of $0156 & 7), V of the ADC that found the offset.
static void call_func_82B7B0(const Regs* in, Regs* out) {
  (void)in;
  uint16_t column = rd16(0x0156), row = rd16(0x0158);
  uint16_t base = rom16(0x838C65 + (column & 0xFFFE));
  uint16_t at = (uint16_t)(2 * row + base), mask = rom16(0x838C55 + 2 * (column & 7));
  func_82B7B0();
  out->a = rd16(0x230C + (uint32_t)at + 0x50 + 8);
  out->x = at;
  out->y = mask;
  setNZ16(out, out->a);
  setAdc16(out, (uint16_t)(2 * row), base);
  out->c = false;
}

static void fuzz_func_82B7B0(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x0156, (uint16_t)(rnd() % 0x40));
  wr16ram(ram, 0x0158, (uint16_t)(rnd() % 0x40));
  for(int i = 0; i < 0x400; i++) ram[0x230C + i] = (uint8_t)rnd();
}

// ---- func_82B838 ($82:B838) ----  X = block offset; exits with A = the last word copied,
// N = 1 / Z = 0 from the final PLB of $83.
static void call_func_82B838(const Regs* in, Regs* out) {
  out->a = func_82B838(in->x);
  out->n = true;
  out->z = false;
}

static void fuzz_func_82B838(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)(2 * (rnd() % 0x100));
  for(int i = 0; i < 0x80; i++) ram[0x217C + r->x + i] = (uint8_t)rnd();
}

// ---- func_80DC5A ($80:DC5A) ----  A = value, X = table (bank DB); exits with X = the
// final byte's offset, A = $00B4 (N = Z = 0), C/V of the SBC $4D - H.
static void call_func_80DC5A(const Regs* in, Regs* out) {
  uint32_t end = func_80DC5A(((uint32_t)in->db << 16) + in->x, in->a);
  out->x = (uint16_t)end;
  out->a = 0x00B4;
  setNZ16(out, out->a);
  setSbc16(out, 0x4D, (uint16_t)(in->a >> 8));
}

static void fuzz_func_80DC5A(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)ram;
  r->a = (uint16_t)rnd();
  if(rnd() & 1) r->a &= 0x00FF;
  r->x = (uint16_t)(rnd() % 0x8000);
}

// ---- func_8A9BB5 ($8A:9BB5) ----  exits with X = the slot, C = 0 and A = $FFFF when one
// was taken, else C = 1, A = 0 ($0168) and X = $0166; Y kept (N/Z from the PLY).
static void call_func_8A9BB5(const Regs* in, Regs* out) {
  uint16_t slot;
  bool taken = func_8A9BB5(&slot);
  out->x = slot;
  out->c = !taken;
  out->a = taken ? 0xFFFF : 0;
  setNZ16(out, in->y);
}

static void fuzz_func_8A9BB5(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint16_t n = (uint16_t)(1 + rnd() % 20);
  wr16ram(ram, DP + 0x66, (uint16_t)(2 * n));
  wr16ram(ram, DP + 0x68, (uint16_t)(rnd() % 8));
  for(uint16_t i = 0; i < n; i++) {
    wr16ram(ram, 0x17FBC + 2 * i, (rnd() % 3) ? 0 : 0xFFFF);
    wr16ram(ram, 0x041C + 2 * i, (uint16_t)(rnd() % 8));
  }
}

// ---- func_8A8EEF ($8A:8EEF, long) ----  exits as clear_sprites does (A = $E000, X = 0,
// Y = $200, Z = 1, 16-bit registers).
static void call_func_8A8EEF(const Regs* in, Regs* out) {
  (void)in;
  func_8A8EEF();
  out->m8 = out->x8 = false;
  out->a = 0xE000;
  out->x = 0;
  out->y = 0x0200;
  setNZ16(out, 0);
}

// ---- func_8A9BAA ($8A:9BAA) ----  A = the new $0154 (C/V of its ADC), N/Z of the DEC $0156.
static void call_func_8A9BAA(const Regs* in, Regs* out) {
  (void)in;
  uint16_t old = rd16(DP + 0x54);
  func_8A9BAA();
  out->a = rd16(DP + 0x54);
  setAdc16(out, old, 0x10);
  setNZ16(out, rd16(DP + 0x56));
}

static void fuzz_func_8A9BAA(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x54, (uint16_t)rnd());
  wr16ram(ram, DP + 0x56, (uint16_t)rnd());
}

// ---- func_8A9BE2 ($8A:9BE2) ----  A = the kind of slot 0 (the last loaded), X = $FFFE
// (N = 1 from the DEX), C of the last CMP with the running best.
static void call_func_8A9BE2(const Regs* in, Regs* out) {
  (void)in;
  uint16_t best = 0;
  for(int x = 0x26; x > 0; x -= 2) {
    uint16_t k = bus_rd16(0x81041C + (uint32_t)x);
    if(!((uint16_t)(k - best) & 0x8000)) best = k;
  }
  uint16_t k0 = bus_rd16(0x81041C);
  func_8A9BE2();
  out->a = k0;
  out->x = 0xFFFE;
  setNZ16(out, out->x);
  out->c = k0 >= best;
}

static void fuzz_func_8A9BE2(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(int i = 0; i < 20; i++) wr16ram(ram, 0x041C + 2 * i, (uint16_t)((rnd() & 3) ? rnd() % 16 : rnd()));
}

// ---- func_8AB680 ($8A:B680) ----  A = list - 4 (bank DB); exits with A = 0, X/Y kept (N/Z
// from the PLX), C/V of the last pointer ADC #8 (as on entry for an empty list).
static void call_func_8AB680(const Regs* in, Regs* out) {
  uint32_t bank = (uint32_t)in->db << 16;
  uint16_t at = (uint16_t)(in->a + 4);
  func_8AB680(bank | at);
  uint16_t end = rd16(DP + 0x60);
  out->a = 0;
  setNZ16(out, in->x);
  if(end != at) setAdc16(out, (uint16_t)(end - 8), 8);
}

static void fuzz_func_8AB680(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->a = 0x1BFC;  // the list at $1C00 of DB's low-WRAM mirror
  int n = (int)(rnd() % 6);
  for(int i = 0; i < n; i++) {
    uint16_t rec = (uint16_t)(0x1C00 + 8 * i);
    for(int b = 0; b < 8; b++) ram[rec + b] = (uint8_t)rnd();
    if(ram[rec] == 0 && ram[rec + 1] == 0) ram[rec] = 1;
    wr16ram(ram, rec + 4, (uint16_t)(2 * (rnd() % 32)));
  }
  wr16ram(ram, (uint16_t)(0x1C00 + 8 * n), 0);
}

// ---- func_8AA4BE ----  A = 0, N = 0, Z = 1 (the terminating word), Y = 2 * the entries,
// X as func_8AB680 leaves it (kept); C V of the last func_8AB680 (its ADC when its list
// was not empty), else as on entry.
static void call_func_8AA4BE(const Regs* in, Regs* out) {
  uint32_t table = ((uint32_t)in->db << 16) | in->a;
  uint16_t n = 0, last = 0;
  for(uint16_t p; (p = bus_rd16(table + 2u * n)) != 0; n++) last = p;
  func_8AA4BE(table);
  out->a = 0;
  out->y = (uint16_t)(2 * n);
  out->n = false;
  out->z = true;
  uint16_t end = rd16(DP + 0x60), at = (uint16_t)(last + 4);
  if(n != 0 && end != at) setAdc16(out, (uint16_t)(end - 8), 8);
}

// a table at $1E00 of 0-3 pointers to the list func_8AB680's fuzz builds
static void fuzz_func_8AA4BE(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_func_8AB680(r, ram, rnd);
  int n = (int)(rnd() % 4);
  for(int i = 0; i < n; i++) wr16ram(ram, (uint16_t)(0x1E00 + 2 * i), 0x1BFC);
  wr16ram(ram, (uint16_t)(0x1E00 + 2 * n), 0);
  r->a = 0x1E00;
}

// ---- func_83FC05 ($83:FC05) ----  D = $0200. Exits with C = the result, X = $011C, A =
// the last difference (vertical, or horizontal once the first test passed), N/Z of the
// last CMP, V of the last SBC.
static void call_func_83FC05(const Regs* in, Regs* out) {
  uint16_t x = rd16(0x011A), kind = rd16(0x011C);
  uint16_t v = (uint16_t)(rd8(0x8CB3 + (uint32_t)x) << 4);
  if(v & 0x0800) v |= 0xF000;
  uint16_t base = rd16((uint16_t)(in->d + 0x06)), d = (uint16_t)(v - base);
  bool first = !(d & 0x8000) ? ((uint16_t)(d - bus_rd16(0x8380CE + kind)) & 0x8000) != 0
                              : !((uint16_t)(d - bus_rd16(0x8380CC + kind)) & 0x8000);
  uint16_t h = (uint16_t)(-(uint16_t)((rd8(0x8CB5 + (uint32_t)x) & 0xF0) >> 1));
  uint16_t sub = bus_rd16(0x81012C), e = (uint16_t)(h - sub);
  bool hit = func_83FC05(in->d);
  out->x = kind;
  if(!first) {
    out->a = d;
    setSbc16(out, v, base);
    setNZ16(out, (uint16_t)(d - bus_rd16((d & 0x8000) ? 0x8380CC + kind : 0x8380CE + kind)));
  } else {
    out->a = e;
    setSbc16(out, h, sub);
    setNZ16(out, (uint16_t)(e - bus_rd16((e & 0x8000) ? 0x83826C + kind : 0x83826E + kind)));
  }
  out->c = hit;
}

static void fuzz_func_83FC05(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint16_t x = (uint16_t)(rnd() % 0x200), kind = (uint16_t)(4 * (rnd() % 16));
  wr16ram(ram, 0x011A, x);
  wr16ram(ram, 0x011C, kind);
  ram[0x8CB3 + x] = (uint8_t)rnd();
  ram[0x8CB5 + x] = (uint8_t)rnd();
  // aim both differences near the limits (within +-48) most of the time: the limits are
  // a few dozen units wide, so uniform values almost never land inside them
  uint16_t v = (uint16_t)(ram[0x8CB3 + x] << 4);
  if(v & 0x0800) v |= 0xF000;
  uint16_t h = (uint16_t)(-(uint16_t)((ram[0x8CB5 + x] & 0xF0) >> 1));
  bool aim = rnd() % 4 != 0;
  wr16ram(ram, (uint16_t)(r->d + 0x06), aim ? (uint16_t)(v - (int16_t)(rnd() % 97 - 48)) : (uint16_t)rnd());
  wr16ram(ram, 0x012C, aim ? (uint16_t)(h - (int16_t)(rnd() % 97 - 48)) : (uint16_t)rnd());
}

// ---- func_83FD8A ($83:FD8A) ----  D = $0200, A = position. Exits:
//   $0120 bit 15 set: A kept, N/V/Z of the BIT, C = 0;
//   a limit not met: A = the difference tested, X = $011C, N/Z of its CMP, V of its SBC, C = 0;
//   a hit: A = the old D+$18, X = $1EBE, Y = 0, N/Z of the final ROR of D+$16, C = 1, V of
//   the D+$5A ADC (or of the BIT $1F31 when the multiply was skipped).
static void call_func_83FD8A(const Regs* in, Regs* out) {
  uint16_t dp = in->d, a = in->a, f120 = rd16(0x0120), kind = rd16(0x011C);
  uint16_t base = rd16((uint16_t)(dp + 0x06)), d = (uint16_t)(a - base);
  uint16_t pos = rd16(0x012A), sub = bus_rd16(0x81012C), e = (uint16_t)(pos - sub);
  uint16_t old18 = rd16((uint16_t)(dp + 0x18)), x1EBE = rd16(0x1EBE), f1F31 = rd16(0x1F31);
  uint16_t old5A = rd16((uint16_t)(dp + 0x5A));
  uint32_t p = (uint32_t)(int32_t)(int16_t)(rd16((uint16_t)(dp + 0x0F)) >> 1) * (int32_t)(int16_t)rd16(0x1F30);
  bool hit = func_83FD8A(dp, a);
  out->c = hit;
  if(f120 & 0x8000) {
    out->n = true;
    out->v = (f120 & 0x4000) != 0;
    out->z = (a & f120) == 0;
    return;
  }
  if(!hit) {
    out->x = kind;
    bool firstFails = !(d & 0x8000) ? !((uint16_t)(d - bus_rd16(0x8380CE + kind)) & 0x8000)
                                     : ((uint16_t)(d - bus_rd16(0x8380CC + kind)) & 0x8000) != 0;
    Regs t;
    if(firstFails) {
      out->a = d;
      setSbc16(&t, a, base);
      setNZ16(out, (uint16_t)(d - bus_rd16((d & 0x8000) ? 0x8380CC + kind : 0x8380CE + kind)));
    } else {
      out->a = e;
      setSbc16(&t, pos, sub);
      setNZ16(out, (uint16_t)(e - bus_rd16((e & 0x8000) ? 0x83826C + kind : 0x83826E + kind)));
    }
    out->v = t.v;
    out->c = false;
    return;
  }
  out->a = old18;
  out->x = x1EBE;
  out->y = 0;
  setNZ16(out, rd16((uint16_t)(dp + 0x16)));
  if(!(f1F31 & 0x8000)) {
    uint16_t top = (uint16_t)((p >> 15) & 0xFFFF), cin = (uint16_t)(p >> 31);
    uint32_t sum = (uint32_t)top + old5A + cin;
    out->v = (~(top ^ old5A) & (top ^ (uint16_t)sum) & 0x8000) != 0;
  } else {
    out->v = (f1F31 & 0x4000) != 0;
  }
  out->c = true;
}

static void fuzz_func_83FD8A(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint16_t kind = (uint16_t)(4 * (rnd() % 16));
  wr16ram(ram, 0x011C, kind);
  wr16ram(ram, 0x0120, (uint16_t)((rnd() % 8 == 0) ? rnd() : rnd() & 0x7FFF));
  uint16_t base = (uint16_t)rnd(), sub = (uint16_t)rnd();
  bool aim = rnd() % 4 != 0;  // near the limits most of the time (they are a few dozen wide)
  wr16ram(ram, (uint16_t)(r->d + 0x06), base);
  r->a = aim ? (uint16_t)(base + (int16_t)(rnd() % 97 - 48)) : (uint16_t)rnd();
  wr16ram(ram, 0x012C, sub);
  wr16ram(ram, 0x012A, aim ? (uint16_t)(sub + (int16_t)(rnd() % 97 - 48)) : (uint16_t)rnd());
  for(int o = 0x0F; o <= 0x1C; o++) ram[(uint16_t)(r->d + o)] = (uint8_t)rnd();
  if(rnd() & 1) wr16ram(ram, (uint16_t)(r->d + 0x1A), 0);              // "no side yet"
  if(rnd() & 1) wr16ram(ram, (uint16_t)(r->d + 0x10), (uint16_t)(rnd() % 0x100));  // near $60
  wr16ram(ram, (uint16_t)(r->d + 0x5A), (uint16_t)rnd());
  wr16ram(ram, (uint16_t)(r->d + 0x7C), (uint16_t)rnd());
  wr16ram(ram, (uint16_t)(r->d + 0x23), (uint16_t)((rnd() & 1) ? 0 : rnd() % 0x40));
  wr16ram(ram, 0x047C, (uint16_t)((rnd() % 4 == 0) ? rnd() : rnd() & 0x7FFF));
  wr16ram(ram, 0x1E9E, (uint16_t)(rnd() % 0x20));
  wr16ram(ram, 0x1EBE, (uint16_t)rnd());
  wr16ram(ram, 0x1F30, (uint16_t)rnd());
  ram[0x1F32] = (uint8_t)rnd();
  wr16ram(ram, 0x0128, (uint16_t)rnd());
}

// ---- func_83FDCD ($83:FDCD) ----  A kept, N/V/Z of BIT $0120, C = 0.
static void call_func_83FDCD(const Regs* in, Regs* out) {
  uint16_t f = rd16(0x0120);
  out->c = func_83FDCD();
  out->n = (f & 0x8000) != 0;
  out->v = (f & 0x4000) != 0;
  out->z = (in->a & f) == 0;
}

static void fuzz_func_83FDCD(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x0120, (uint16_t)rnd());
}

// ---- func_82EA49 ($82:EA49) ----  A = value; exits with A = the bank-$85 word (N/Z of its
// LDA), X = its offset, Y = the value, C/V of the ADC that found the offset.
static void call_func_82EA49(const Regs* in, Regs* out) {
  uint16_t a = in->a;
  uint16_t lo = (uint16_t)dsp4_multiply((int16_t)((a >> 6) & 0xFF), 10);
  uint32_t at = (uint32_t)lo + rd16(DP + 0x98);
  uint16_t e = bus_rd16(0x840002 + (uint32_t)(uint16_t)at), part = (uint16_t)(a & 0x3E);
  uint32_t entry = (uint32_t)part + e + (at >> 16);
  out->a = func_82EA49(a);
  out->x = (uint16_t)entry;
  out->y = a;
  setNZ16(out, out->a);
  out->c = entry > 0xFFFF;
  out->v = (~(part ^ e) & (part ^ (uint16_t)entry) & 0x8000) != 0;
}

static void fuzz_func_82EA49(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  // the game's offsets land in ROM in both banks ($84 and $85, $8000-$FFFF); random ones
  // can land on I/O ($2000-$7FFF), which the routine never reads: retry until both are ROM
  for(int tries = 0; tries < 64; tries++) {
    r->a = (uint16_t)rnd();
    uint16_t base = (uint16_t)(0x8000 + rnd() % 0x7000);
    uint32_t at = (uint32_t)(uint16_t)dsp4_multiply((int16_t)((r->a >> 6) & 0xFF), 10) + base;
    uint16_t e = rom16(0x840002 + (uint32_t)(uint16_t)at);
    uint32_t entry = (uint32_t)(r->a & 0x3E) + e + (at >> 16);
    wr16ram(ram, DP + 0x98, base);
    if((uint16_t)at >= 0x8000 && (uint16_t)entry >= 0x8000 && (uint16_t)at < 0xFFFE && (uint16_t)entry < 0xFFFE) return;
  }
}

// ---- func_82AB3F ($82:AB3F) ----  X, Y = entry offsets (DB = $7E); exits with A = $0118
// (N/Z of its SBC, C/V of that SBC #$0C), X/Y kept.
static void call_func_82AB3F(const Regs* in, Regs* out) {
  out->a = func_82AB3F(in->x, in->y);
  uint16_t b = (uint16_t)(out->a + 0x0C);
  setNZ16(out, out->a);
  setSbc16(out, b, 0x0C);
}

static void fuzz_func_82AB3F(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)(2 * (rnd() % 0x100));
  r->y = (uint16_t)(2 * (rnd() % 0x80));
  for(int i = 0; i < 10; i++) ram[0xFAB4 + r->x + i] = (uint8_t)rnd();
  for(int i = 0; i < 4; i++) {
    ram[0x34EC + r->y + i] = (uint8_t)rnd();
    ram[0x36EC + r->y + i] = (uint8_t)rnd();
  }
}

// ---- func_82A9B7 ($82:A9B7) ----  exits with A = the new $028D, DB = $83 (N = 1, Z = 0
// from the final PLB), C of the last CMP, V of the last ADC (both on the $028D step); X/Y
// = $0281/$0283 unless $0281 was negative.
static void call_func_82A9B7(const Regs* in, Regs* out) {
  bool skip = (rd16(0x0281) & 0x8000) != 0;
  uint16_t x = rd16(0x0281), y = rd16(0x0283), cur = rd16(0x028D);
  out->a = func_82A9B7();
  uint16_t target = rd16(DP + 0x18), step = (uint16_t)(rd16(0x1E9E) << 2), t;
  if((uint16_t)(target - cur) & 0x8000) {
    t = (uint16_t)(cur - step);
    uint16_t neg = (uint16_t)~step;  // EOR #$FFFF / SEC / ADC: V of ~step + cur + 1
    uint16_t sum = (uint16_t)(neg + cur + 1);
    out->v = (~(neg ^ cur) & (neg ^ sum) & 0x8000) != 0;
  } else {
    t = (uint16_t)(cur + step);
    setAdc16(out, step, cur);
  }
  out->c = t >= target;
  out->n = true;
  out->z = false;
  out->db = 0x83;
  if(!skip) { out->x = x; out->y = y; }
}

static void fuzz_func_82A9B7(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint16_t x = (uint16_t)(2 * (rnd() % 0x100)), y = (uint16_t)(2 * (rnd() % 0x80));
  wr16ram(ram, 0x0281, (rnd() % 8 == 0) ? (uint16_t)(0x8000 | rnd()) : x);
  wr16ram(ram, 0x0283, y);
  for(int i = 0; i < 10; i++) ram[0xFAB4 + x + i] = (uint8_t)rnd();
  for(int i = 0; i < 4; i++) {
    ram[0x34EC + y + i] = (uint8_t)rnd();
    ram[0x36EC + y + i] = (uint8_t)rnd();
  }
  wr16ram(ram, 0x0293, (uint16_t)rnd());
  wr16ram(ram, 0x1E9E, (uint16_t)(rnd() % 0x40));
  wr16ram(ram, 0x028B, (uint16_t)rnd());
  wr16ram(ram, 0x028D, (uint16_t)rnd());
}

// the first $7E:4E2D value: bits 8-23 of the product - $B00, at most $3000 (by sign)
static uint16_t cap_after_first(uint16_t pv) {
  uint16_t v = (uint16_t)(pv - 0x0B00);
  return ((uint16_t)(v - 0x3000) & 0x8000) ? v : 0x3000;
}

// ---- func_82AC52 ($82:AC52) ----  three exits: $0244 bit 0 clear (A = $0244 >> 1, C = 0,
// V of the first SBC #$B00); bit 2 clear (A = $0244, N of it, Z = 1, C/V of SBC $14); else
// A = the stored $7E:4E2D (N/Z of its CMP or of LDA #$3000, C of the CMP, V of SBC #$B00).
static void call_func_82AC52(const Regs* in, Regs* out) {
  (void)in;
  uint32_t p = (uint32_t)dsp4_multiply((int16_t)rd16(0x0213), 0x0300);
  uint16_t pv = (uint16_t)(p >> 8), flags = rd16(0x0244);
  uint32_t a = ((uint32_t)rd16(0xFAB6) << 16) | rd16(0xFAB4), b = ((uint32_t)rd16(0x0226) << 16) | rd16(0x0224);
  uint16_t far = (uint16_t)((a - b) >> 16);
  if(far & 0x8000) far = 0;
  uint16_t far2 = (uint16_t)(far << 1);
  func_82AC52();
  Regs t;
  if(!(flags & 1)) {
    out->a = (uint16_t)(flags >> 1);
    setNZ16(out, out->a);
    out->c = false;
    setSbc16(&t, pv, 0x0B00);
    out->v = t.v;
    return;
  }
  if(!(flags & 4)) {
    out->a = flags;
    out->n = (flags & 0x8000) != 0;
    out->z = true;
    setSbc16(out, 0xB0, far2);
    return;
  }
  uint16_t stored = rd16(0x4E2D);
  // the quotient's value before the clamp, from the first $7E:4E2D value
  uint16_t w0 = (uint16_t)(cap_after_first(pv) + 0x0B00);
  if(w0 & 0x8000) w0 = 0;
  uint16_t d = (uint16_t)(rd16(0xFAB6) - rd16(0x0226));
  if(d & 0x8000) d = 0;
  d = (uint16_t)(d + 0x10);
  if(d >= 0xD0) d = 0xD0;
  uint16_t q4 = (uint16_t)((uint16_t)(w0 / (uint8_t)d) << 4);
  uint16_t qv = (uint16_t)(q4 - 0x0B00);
  out->a = stored;
  setSbc16(&t, q4, 0x0B00);
  out->v = t.v;
  out->c = qv >= 0x3000;
  if((uint16_t)(qv - 0x3000) & 0x8000) { out->n = true; out->z = false; }
  else { out->n = false; out->z = false; }
}

static void fuzz_func_82AC52(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x0213, (uint16_t)rnd());
  wr16ram(ram, 0x0244, (uint16_t)rnd());
  uint16_t hi = (uint16_t)rnd();
  wr16ram(ram, 0x0226, hi);
  wr16ram(ram, 0x0224, (uint16_t)rnd());
  wr16ram(ram, 0xFAB6, (uint16_t)(hi + (int16_t)(rnd() % 0x200 - 0x40)));  // near, mostly ahead
  wr16ram(ram, 0xFAB4, (uint16_t)rnd());
}

// ---- decompress_to_vram ($80:8381; $80:837D long) ----  A low byte = source bank, X =
// source, Y = VRAM word. Exits with A = Y's high byte : $01 (the last LDA #$01, N = Z = 0),
// X = the end of the stream, Y = 0, and decompress's C/V for a destination of 0: those of
// ~0 + length + 1, i.e. C = 1, V = 0.
static void call_decompress_to_vram(const Regs* in, Regs* out) {
  out->x = decompress_to_vram(in->d, (uint8_t)in->a, in->x, in->y);
  out->a = (uint16_t)((in->y & 0xFF00) | 0x01);
  out->y = 0;
  out->n = out->z = false;
  out->c = true;
  out->v = false;
}

// ---- func_8ABA55 ----  as upload_cgram_now (A = $0001: colour 0 in the high byte, X = $2202)
static void call_func_8ABA55(const Regs* in, Regs* out) {
  func_8ABA55(in->d);
  out->x = 0x2202;
  out->a = 0x0001;
  out->y = 0x0200;
  out->n = out->z = false;
  out->c = true;
  out->v = false;
}

// ---- func_8AB994 ----  as decompress_to_vram's second call (Y = $1000)
static void call_func_8AB994(const Regs* in, Regs* out) {
  out->x = func_8AB994(in->d);
  out->a = 0x1001;
  out->y = 0;
  out->n = out->z = false;
  out->c = true;
  out->v = false;
}

static void fuzz_decompress_to_vram(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_decompress_stream(ram, rnd, 0, false);  // unpacks to $7F:0000
  // the original ORs A with $7F00 for the destination bank: the game's A high byte is
  // below $80 (bank $7F); $80 or more would make it bank $FF
  r->a = (uint16_t)((rnd() & 0x7F00) | 0x7E);
  r->x = 0x4000;
  r->y = (uint16_t)(rnd() & 0x7FFF);
}

// ---- clear_vram ($8A:8DEC, long) ----  SEP #$20 / REP #$10 first, REP #$20 last; exits
// with A = its high byte : $01 (N = Z = 0 from the 8-bit LDA #$01), X = 0, 16-bit registers.
static void call_clear_vram(const Regs* in, Regs* out) {
  clear_vram();
  out->a = (uint16_t)((in->a & 0xFF00) | 0x01);
  out->x = 0;
  out->m8 = out->x8 = false;
  out->n = out->z = false;
}

// V as func_80F745 leaves it (dead for its own callers, live here): from its last SBC/ADC —
// ($013A & $FF00) - ($0138 & $FF00); then n - $7F if n = that difference's high byte + 1 is
// $80 or more; then $0114 + $FF01 if $0132's high byte is set (a run no movie reaches,
// decoded from ROM); then, if $0138's high byte m is set, (2m - 1) - $0116, and $0114 +
// $FF01 when that is not 0. None of these inputs is written by the routine. When $013A is
// 0 it returns early and V is what it was before (`before`).
//
// V as inverse_scale leaves it: that of its last SBC $017A in the division loop, or as on
// entry if the loop never subtracts (or the divisor is $100).
static bool v_after_inverse_scale(bool before) {
  uint16_t divisor = rd16(DP + 0x7A);
  if(divisor == 0x0100) return before;
  uint16_t rem = 0x0200;
  bool v = before;
  for(int bit = 0; bit < 8; bit++) {
    if(rem >= divisor) {
      Regs t;
      setSbc16(&t, rem, divisor);
      v = t.v;
      rem = (uint16_t)(rem - divisor);
    }
    rem = (uint16_t)(rem << 1);
  }
  return v;
}
static bool v_after_func_80F745(bool before) {
  if(rd16(DP + 0x3A) == 0) return before;  // its early exit leaves V alone
  uint16_t s38 = rd16(DP + 0x38), s3A = rd16(DP + 0x3A), s32 = rd16(DP + 0x32);
  uint16_t s14 = rd16(DP + 0x14), s16 = rd16(DP + 0x16);
  Regs t;
  setSbc16(&t, (uint16_t)(s3A & 0xFF00), (uint16_t)(s38 & 0xFF00));
  bool v = t.v;
  uint16_t n = (uint16_t)((((uint16_t)((s3A & 0xFF00) - (s38 & 0xFF00))) >> 8) & 0xFF) + 1;
  if(n >= 0x80) { setSbc16(&t, n, 0x7F); v = t.v; }
  if(s32 & 0xFF00) { setAdc16(&t, s14, 0xFF01); v = t.v; }
  uint16_t m = (uint16_t)((s38 >> 8) & 0xFF);
  if(m != 0) {
    uint16_t k = (uint16_t)(2 * m - 1);
    setSbc16(&t, k, s16);
    v = t.v;
    if((uint16_t)(k - s16) != 0) { setAdc16(&t, s14, 0xFF01); v = t.v; }
  }
  return v;
}

// ---- func_80F3C4 / func_80F3F4 ----  as func_80F745 leaves them (A = its result, X = $0134,
// N/Z of X, V as above), then C = 0 from the CLC.
static void call_func_80F3C4(const Regs* in, Regs* out) {
  out->a = func_80F3C4();
  out->x = rd16(DP + 0x34);
  setNZ16(out, out->x);
  out->v = v_after_func_80F745(v_after_inverse_scale(in->v));
  out->c = false;
}

static void call_func_80F3F4(const Regs* in, Regs* out) {
  out->a = func_80F3F4();
  out->x = rd16(DP + 0x34);
  setNZ16(out, out->x);
  out->v = v_after_func_80F745(v_after_inverse_scale(in->v));
  out->c = false;
}

static void fuzz_upload_half(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_inverse_scale(r, ram, rnd);
  fuzz_func_80F745(r, ram, rnd);
  wr16ram(ram, DP + 0x3E, (uint16_t)rnd());
}

// ---- upload_slice ($80:F4AD + 12 * k) ----  as func_80F3C4, with Y = the slice's VRAM word.
#define SLICE_ADAPTER(k) \
  static void call_upload_slice_##k(const Regs* in, Regs* out) { \
    out->a = upload_slice(k); \
    out->x = rd16(DP + 0x34); \
    out->y = (uint16_t)((k) * 0x030A); \
    setNZ16(out, out->x); \
    out->v = v_after_func_80F745(v_after_inverse_scale(in->v)); \
    out->c = false; \
  }
SLICE_ADAPTER(0) SLICE_ADAPTER(1) SLICE_ADAPTER(2) SLICE_ADAPTER(3) SLICE_ADAPTER(4) SLICE_ADAPTER(5)
SLICE_ADAPTER(6) SLICE_ADAPTER(7) SLICE_ADAPTER(8) SLICE_ADAPTER(9) SLICE_ADAPTER(10) SLICE_ADAPTER(11)
SLICE_ADAPTER(12) SLICE_ADAPTER(13) SLICE_ADAPTER(14) SLICE_ADAPTER(15)

// Negative test: mode7_zoom_view with the two offsets swapped (the PPU field comparison
// must see it).
static void call_mode7_zoom_view_wrong(const Regs* in, Regs* out) {
  int16_t scale = (int16_t)rd16((uint16_t)(in->d + ZOOM_SCALE));
  video_set_m7a(scale);
  video_set_m7d(scale);
  uint16_t h = (uint16_t)(0x80 - rd8((uint16_t)(in->d + ZOOM_CENTRE_X)));
  uint16_t v = (uint16_t)(0x6F - rd8((uint16_t)(in->d + ZOOM_CENTRE_Y)));
  video_set_bg1_hofs(v);
  video_set_bg1_vofs(h);
  uint8_t hi = (uint8_t)(v >> 8);
  out->a = (uint16_t)((in->a & 0xFF00) | hi);
  out->n = hi & 0x80;
  out->v = false;
  out->z = out->c = hi == 0;
}

// Negative test: draw_meter with the partial tile one eighth short (the VRAM word
// comparison must see it).
static void call_draw_meter_wrong(const Regs* in, Regs* out) {
  int16_t value = (int16_t)rd16((uint16_t)(in->d + METER_VALUE));
  video_set_m7a(value);
  video_set_m7b((int16_t)(0x0A00 | (uint8_t)((uint16_t)value >> 8)));
  uint16_t length = (uint16_t)(((int32_t)value * 10) >> 8) + 2;
  wr16((uint16_t)(in->d + METER_SCRATCH), length);
  uint16_t left = METER_TILES, vram = METER_VRAM;
  for(uint16_t full = length >> 3; full != 0; full--, left--) video_write_vram_word(vram++, METER_FULL);
  if(length & 7) {
    video_write_vram_word(vram++, (uint16_t)(METER_PART + (length & 7) - 1));
    left--;
  }
  for(; left != 0; left--) video_write_vram_word(vram++, METER_EMPTY);
  bool part = length & 7;
  out->y = 0;
  out->x = (length >> 3) ? 0 : METER_VRAM;
  out->a = (uint16_t)(METER_TILES - (length >> 3) - (part ? 1 : 0)) ? METER_EMPTY : 0;
  out->n = false;
  out->z = true;
  out->c = part ? false : (length >> 2) & 1;
  if(part) out->v = false;
}

// Negative test: rng_next with one XOR constant off. --difftest-negative must FAIL.
static uint16_t rng_next_wrong(uint16_t state, bool* carry) {
  uint32_t s = rd32(state);
  bool top = (s >> 31) & 1;
  s <<= 1;
  if(!top) s ^= 0x1D872B44;
  wr32(state, s);
  *carry = top;
  return (uint16_t)s;
}

static void call_rng_next_wrong(const Regs* in, Regs* out) {
  bool carry;
  out->a = rng_next_wrong((uint16_t)(in->d + 0x10), &carry);
  out->c = carry;
  setNZ16(out, out->a);
}

// Negative test for the interrupt replay: waiting one frame too many must FAIL.
static void call_wait_frames_wrong(const Regs* in, Regs* out) {
  out->a = wait_frames((uint16_t)(in->a + 1));
  out->z = out->c = true;
  out->n = false;
}

// Negative test for the effect comparison: an upload one byte too long must FAIL.
static void call_upload_vram_now_wrong(const Regs* in, Regs* out) {
  video_upload_vram(in->y, ((uint32_t)rd8((uint16_t)(in->d + 0x14)) << 16) | in->x, (in->a ? in->a : 0x10000) + 1u);
  out->a = 0x1801;
  out->n = out->z = false;
}

#define ALL_OUTPUTS .compareA = true, .compareX = true, .compareY = true

const Routine routines[] = {
  {.name = "rng_next", .entry = 0x8087E9, .end = 0x808807, .source = "src/rng.c",
   .needM16 = true, .call = call_rng_next, ALL_OUTPUTS, .fuzz = fuzz_rng_next},
  {.name = "rng_next_8a", .entry = 0x8A988C, .end = 0x8A98AA, .source = "src/rng.c",
   .needM16 = true, .call = call_rng_next, ALL_OUTPUTS, .fuzz = fuzz_rng_next},
  {.name = "vram_queue_push", .entry = 0x8083FC, .end = 0x80841A, .source = "src/vram_queue.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_vram_queue_push, ALL_OUTPUTS,
   .fuzz = fuzz_vram_queue_push},
  {.name = "take_slot", .entry = 0x8AA25D, .end = 0x8AA264, .source = "src/util.c",
   .needM16 = true, .needX16 = true, .call = call_take_slot, ALL_OUTPUTS, .fuzz = fuzz_take_slot},
  {.name = "lsr_n", .entry = 0x8AB15E, .end = 0x8AB168, .source = "src/util.c",
   .needM16 = true, .needX16 = true, .call = call_lsr_n, ALL_OUTPUTS, .fuzz = fuzz_lsr_n},
  {.name = "inverse_scale", .entry = 0x80F709, .end = 0x80F745, .source = "src/scale.c",
   .needM16 = true, .needX16 = true, .call = call_inverse_scale, ALL_OUTPUTS, .fuzz = fuzz_inverse_scale},
  {.name = "mode7_zoom_view", .entry = 0x80F38D, .end = 0x80F3C4, .source = "src/scale.c", .needM16 = true,
   .fixedD = true, .call = call_mode7_zoom_view, ALL_OUTPUTS, .fuzz = fuzz_mode7_zoom_view},
  {.name = "mode7_view", .entry = 0x8A8B0A, .end = 0x8A8BB6, .source = "src/scale.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_mode7_view, ALL_OUTPUTS, .fuzz = fuzz_mode7_view},
  {.name = "draw_meter", .entry = 0x80DA5F, .end = 0x80DAAE, .source = "src/hud.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_draw_meter, ALL_OUTPUTS, .fuzz = fuzz_draw_meter},
  {.name = "func_8A8232", .entry = 0x8A8232, .end = 0x8A823D, .source = "src/misc.c", .needM16 = true,
   .fixedD = true, .call = call_func_8A8232, ALL_OUTPUTS, .fuzz = fuzz_func_8A8232},
  {.name = "func_82AD12", .entry = 0x82AD12, .end = 0x82AD21, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_func_82AD12, ALL_OUTPUTS, .fuzz = fuzz_func_82AD12},
  {.name = "binary_to_bcd5", .entry = 0x8ABFD0, .end = 0x8AC004, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_binary_to_bcd5, ALL_OUTPUTS, .fuzz = fuzz_binary_to_bcd5},
  {.name = "pads_read_pair", .entry = 0x8387C1, .end = 0x83886A, .source = "src/pads.c", .lowWramDb = true,
   .call = call_pads_read_pair, ALL_OUTPUTS, .fuzz = fuzz_pads},
  {.name = "pads_read", .entry = 0x8386FF, .end = 0x8387C1, .source = "src/pads.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_pads_read, ALL_OUTPUTS, .fuzz = fuzz_pads},
  {.name = "pads_read_long", .entry = 0x8386FB, .end = 0x8386FF, .rtl = true, .source = "src/pads.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_pads_read_long, ALL_OUTPUTS, .fuzz = fuzz_pads},
  {.name = "pads_read_long_2", .entry = 0x8396CD, .end = 0x8396D1, .rtl = true, .source = "src/pads.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_pads_read_long_2, ALL_OUTPUTS, .fuzz = fuzz_pads},
  {.name = "race_upload_queue_drain", .entry = 0x838644, .end = 0x8386FB, .source = "src/race_frame.c",
   .needM16 = true, .needX16 = true, .call = call_race_upload_queue_drain, .fuzz = fuzz_race_upload_queue},
  {.name = "race_upload_queue_drain_long", .entry = 0x8396D1, .end = 0x8396D5, .rtl = true, .source = "src/race_frame.c",
   .needM16 = true, .needX16 = true, .call = call_race_upload_queue_drain_long, .fuzz = fuzz_race_upload_queue},
  {.name = "func_838989", .entry = 0x838989, .end = 0x8389D1, .source = "src/race_frame.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_func_838989, .fuzz = fuzz_func_838989},
  {.name = "func_838989_long", .entry = 0x8396D5, .end = 0x8396D9, .rtl = true, .source = "src/race_frame.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_func_838989_long, .fuzz = fuzz_func_838989},
  {.name = "video_load_screen", .entry = 0x8081CC, .end = 0x808373, .source = "src/video_dma.c",
   .call = call_video_load_screen, ALL_OUTPUTS, .fuzz = fuzz_video_load_screen},
  {.name = "video_load_screen_long", .entry = 0x8081C8, .end = 0x8081CC, .rtl = true, .source = "src/video_dma.c",
   .call = call_video_load_screen_long, ALL_OUTPUTS, .fuzz = fuzz_video_load_screen},
  {.name = "fade_in_start", .entry = 0x8ABAF2, .end = 0x8ABAFF, .source = "src/misc.c", .needM16 = true,
   .lowWramDb = true, .call = call_fade_in_start, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "hdma_window_tables_init", .entry = 0x8AA09F, .end = 0x8AA0F9, .source = "src/misc.c", .lowWramDb = true,
   .call = call_hdma_window_tables_init, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "race_hud_draw", .entry = 0x858814, .end = 0x858AAE, .rtl = true, .source = "src/race_frame.c",
   .needM16 = true, .needX16 = true, .call = call_race_hud_draw, .fuzz = fuzz_race_hud_draw},
  {.name = "irq_race_band", .entry = 0x8587BA, .end = 0x858812, .rti = true, .source = "src/race_frame.c",
   .call = call_irq_race_band, ALL_OUTPUTS},  // no fuzz: it waits (WAI) for an interrupt the fuzz never raises
  {.name = "race_cycle_blocks", .entry = 0x859147, .end = 0x8593A7, .source = "src/race_frame.c",
   .needM16 = true, .needX16 = true, .call = call_race_cycle_blocks, .fuzz = fuzz_race_cycle_blocks},
  {.name = "sound_fade_step", .entry = 0x86DFAE, .end = 0x86DFE1, .source = "src/sound.c", .needM16 = true,
   .lowWramDb = true, .call = call_sound_fade_step, ALL_OUTPUTS, .fuzz = fuzz_sound_fade_step},
  // no fuzz: the fuzz runs the CPU alone, so the sound CPU never answers the handshake
  {.name = "sound_send_message", .entry = 0x86DFFB, .end = 0x86E096, .source = "src/sound.c", .lowWramDb = true,
   .call = call_sound_send_message, ALL_OUTPUTS},
  {.name = "sound_frame", .entry = 0x86DFE1, .end = 0x86DFFB, .rtl = true, .source = "src/sound.c", .needM16 = true,
   .lowWramDb = true, .call = call_sound_frame, ALL_OUTPUTS},
  {.name = "oam_from_objects", .entry = 0x8A8E1B, .end = 0x8A8EEF, .rtl = true, .source = "src/sprites.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_oam_from_objects, ALL_OUTPUTS,
   .fuzz = fuzz_objects},
  {.name = "func_8AA050", .entry = 0x8AA050, .end = 0x8AA06D, .rtl = true, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_func_8AA050, ALL_OUTPUTS, .fuzz = fuzz_func_8AA050},
  // NMI rows fuzz with the sound message off (fuzz_nmi_quiet)
  {.name = "nmi_target_80FBD1", .entry = 0x80FBD1, .end = 0x80FBD8, .rti = true, .rtiFrame = 9, .source = "src/frame.c",
   .needM16 = true, .needX16 = true, .call = call_nmi_target_80FBD1, ALL_OUTPUTS, .alsoIn = {{0x808934, 0x80894E}},
   .fuzz = fuzz_nmi_tail},
  {.name = "nmi_target_80FC5F", .entry = 0x80FC5F, .end = 0x80FC7D, .rti = true, .rtiFrame = 9, .source = "src/frame.c",
   .needM16 = true, .needX16 = true, .call = call_nmi_target_80FC5F, ALL_OUTPUTS, .alsoIn = {{0x808934, 0x80894E}},
   .fuzz = fuzz_nmi_target_80FC5F},
  {.name = "nmi_target_80E28C", .entry = 0x80E28C, .end = 0x80E3F7, .rti = true, .rtiFrame = 9, .source = "src/frame.c",
   // no fuzz: it enables the NMI itself ($4200 = $B1), which then interrupts the case
   .needM16 = true, .needX16 = true, .call = call_nmi_target_80E28C, ALL_OUTPUTS, .alsoIn = {{0x808934, 0x80894E}}},
  {.name = "nmi_target_80D784", .entry = 0x80D784, .end = 0x80D823, .rti = true, .rtiFrame = 9, .source = "src/frame.c",
   .needM16 = true, .needX16 = true, .call = call_nmi_target_80D784, ALL_OUTPUTS, .alsoIn = {{0x808934, 0x80894E}},
   .fuzz = fuzz_nmi_tail},
  {.name = "nmi_race_full", .entry = 0x8583CE, .end = 0x8587BA, .rti = true, .rtiFrame = 9, .source = "src/race_frame.c",
   .needM16 = true, .needX16 = true, .call = call_nmi_race_full, ALL_OUTPUTS, .alsoIn = {{0x85838A, 0x8583CE}},
   .fuzz = fuzz_nmi_race_full},
  {.name = "nmi_race_tail", .entry = 0x85838A, .end = 0x8583CE, .rti = true, .rtiFrame = 9, .source = "src/frame.c",
   .needM16 = true, .needX16 = true, .call = call_nmi_race_tail, ALL_OUTPUTS, .fuzz = fuzz_nmi_race_tail},
  {.name = "nmi_target_80E175", .entry = 0x80E175, .end = 0x80E28C, .rti = true, .rtiFrame = 9, .source = "src/frame.c",
   .needM16 = true, .needX16 = true, .call = call_nmi_target_80E175, ALL_OUTPUTS, .alsoIn = {{0x808934, 0x80894E}},
   .fuzz = fuzz_nmi_target_80E175},
  {.name = "nmi_prologue", .entry = 0x808807, .end = 0x808931, .rti = true, .endAt = 0x808931, .source = "src/frame.c",
   .call = call_nmi_prologue, .fuzz = fuzz_nmi_prologue},
  {.name = "nmi_race_prologue", .entry = 0x858377, .end = 0x858387, .rti = true, .endAt = 0x858387, .source = "src/frame.c",
   .call = call_nmi_race_prologue, .fuzz = fuzz_nmi_prologue},
  {.name = "nmi_tail", .entry = 0x808934, .end = 0x80894E, .rti = true, .rtiFrame = 9, .source = "src/frame.c",
   .needM16 = true, .needX16 = true, .fixedD = false, .call = call_nmi_tail, ALL_OUTPUTS, .fuzz = fuzz_nmi_tail},
  // no fuzz: it waits (WAI) for the next interrupt, which the fuzz never raises
  {.name = "irq_split_top", .entry = 0x80E3F7, .end = 0x80E48A, .rti = true, .source = "src/frame.c",
   .call = call_irq_split_top, ALL_OUTPUTS},
  {.name = "irq_split_bottom", .entry = 0x80E48A, .end = 0x80E4C5, .rti = true, .source = "src/frame.c",
   .call = call_irq_split_bottom, ALL_OUTPUTS},
  {.name = "irq_ack_only", .entry = 0x8595BD, .end = 0x8595C7, .rti = true, .source = "src/frame.c",
   .call = call_irq_ack_only, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "scroll_tables_build", .entry = 0x85BC20, .end = 0x85BED1, .rtl = true, .source = "src/scroll_tables.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x83, .call = call_scroll_tables_build, ALL_OUTPUTS,
   .fuzz = fuzz_scroll_tables_build},
  {.name = "func_80D6F3", .entry = 0x80D6F3, .end = 0x80D784, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_func_80D6F3, ALL_OUTPUTS, .fuzz = fuzz_func_80D6F3},
  {.name = "func_8ABF4E", .entry = 0x8ABF4E, .end = 0x8ABF76, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_func_8ABF4E, ALL_OUTPUTS, .fuzz = fuzz_func_8ABF4E},
  {.name = "func_8A9202", .entry = 0x8A9202, .end = 0x8A92CF, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .lowWramDb = true, .call = call_func_8A9202, ALL_OUTPUTS, .fuzz = fuzz_func_8A9202},
  {.name = "func_8A8000", .entry = 0x8A8000, .end = 0x8A8001, .rtl = true, .source = "src/misc.c",
   .call = call_func_8A8000, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "func_8A987F", .entry = 0x8A987F, .end = 0x8A9880, .rtl = true, .source = "src/misc.c",
   .call = call_func_8A987F, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "func_85CDAF", .entry = 0x85CDAF, .end = 0x85CDE5, .rtl = true, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x7E, .call = call_func_85CDAF, ALL_OUTPUTS, .fuzz = fuzz_func_85CDAF},
  {.name = "func_8A869A", .entry = 0x8A869A, .end = 0x8A86AB, .source = "src/misc.c", .needM16 = true,
   .fixedD = true, .call = call_func_8A869A, ALL_OUTPUTS, .fuzz = fuzz_dp_words},
  {.name = "func_8A823D", .entry = 0x8A823D, .end = 0x8A824B, .source = "src/misc.c", .needM16 = true,
   .fixedD = true, .call = call_func_8A823D, ALL_OUTPUTS, .fuzz = fuzz_dp_words},
  {.name = "func_8A8325", .entry = 0x8A8325, .end = 0x8A833C, .source = "src/misc.c", .needM16 = true,
   .fixedD = true, .lowWramDb = true, .call = call_func_8A8325, ALL_OUTPUTS, .fuzz = fuzz_dp_words},
  {.name = "func_80B556", .entry = 0x80B556, .end = 0x80B58D, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_func_80B556, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "func_8ABF76", .entry = 0x8ABF76, .end = 0x8ABF9E, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_func_8ABF76, ALL_OUTPUTS, .fuzz = fuzz_func_8ABF76},
  {.name = "func_8A8290", .entry = 0x8A8290, .end = 0x8A82B4, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_func_8A8290, ALL_OUTPUTS, .fuzz = fuzz_func_8A8290},
  {.name = "func_809DD0", .entry = 0x809DD0, .end = 0x809E3A, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_func_809DD0, ALL_OUTPUTS, .fuzz = fuzz_func_809DD0},
  {.name = "func_8AA32B", .entry = 0x8AA32B, .end = 0x8AA37A, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_func_8AA32B, ALL_OUTPUTS, .fuzz = fuzz_func_8AA32B},
  {.name = "func_8AA381", .entry = 0x8AA381, .end = 0x8AA419, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_func_8AA381, ALL_OUTPUTS, .fuzz = fuzz_func_8AA381},
  {.name = "func_82FB1C", .entry = 0x82FB1C, .end = 0x82FB1D, .source = "src/misc.c",
   .call = call_func_82FB1C, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "func_8AC03D", .entry = 0x8AC03D, .end = 0x8AC056, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_func_8AC03D, ALL_OUTPUTS, .fuzz = fuzz_shift32},
  {.name = "func_8AC024", .entry = 0x8AC024, .end = 0x8AC03D, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_func_8AC024, ALL_OUTPUTS, .fuzz = fuzz_shift32},
  {.name = "func_8ABA55", .entry = 0x8ABA55, .end = 0x8ABA7D, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_func_8ABA55, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "func_82F380", .entry = 0x82F380, .end = 0x82F381, .source = "src/misc.c",
   .call = call_func_82F380, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "upload_tiles_c280", .entry = 0x8A8D81, .end = 0x8A8DA0, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_upload_tiles_c280, ALL_OUTPUTS, .fuzz = fuzz_upload_tiles},
  {.name = "upload_tiles_c680", .entry = 0x8A8DA0, .end = 0x8A8DBF, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_upload_tiles_c680, ALL_OUTPUTS, .fuzz = fuzz_upload_tiles},
  {.name = "oam_clear_long", .entry = 0x80FBD8, .end = 0x80FBDC, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_oam_clear_long, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "func_808FFC", .entry = 0x808FFC, .end = 0x809039, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_func_808FFC, ALL_OUTPUTS, .fuzz = fuzz_func_808FFC},
  {.name = "func_8AA4BE", .entry = 0x8AA4BE, .end = 0x8AA4CF, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_func_8AA4BE, ALL_OUTPUTS, .fuzz = fuzz_func_8AA4BE},
  {.name = "func_8AC0F2", .entry = 0x8AC0F2, .end = 0x8AC137, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_func_8AC0F2, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "func_8AB994", .entry = 0x8AB994, .end = 0x8AB9AF, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_func_8AB994, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "wait_one_frame", .entry = 0x8AB212, .end = 0x8AB21C, .source = "src/frame.c", .needM16 = true,
   .lowWramDb = true, .waitsForInterrupts = true, .call = call_wait_one_frame, ALL_OUTPUTS},
  {.name = "func_80894E", .entry = 0x80894E, .end = 0x80899F, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_func_80894E, ALL_OUTPUTS, .fuzz = fuzz_func_80894E},
  {.name = "binary_to_bcd", .entry = 0x8086C9, .end = 0x808704, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_binary_to_bcd, ALL_OUTPUTS, .fuzz = fuzz_binary_to_bcd},
  {.name = "func_8AA005", .entry = 0x8AA005, .end = 0x8AA022, .source = "src/misc.c", .needM16 = true,
   .fixedD = true, .lowWramDb = true, .call = call_func_8AA005, ALL_OUTPUTS, .fuzz = fuzz_func_8AA022},
  {.name = "func_80E0C8", .entry = 0x80E0C8, .end = 0x80E0D6, .source = "src/misc.c", .needM16 = true,
   .call = call_func_80E0C8, ALL_OUTPUTS, .fuzz = fuzz_func_80E0C8},
  {.name = "oam_clear", .entry = 0x8083E8, .end = 0x8083FC, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_oam_clear, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "wait_flag_76", .entry = 0x80B950, .end = 0x80B95D, .source = "src/frame.c", .needM16 = true,
   .waitsForInterrupts = true, .call = call_wait_flag_76, ALL_OUTPUTS, .fuzz = fuzz_wait_flag_76},
  {.name = "bcd_to_binary", .entry = 0x8ABF9E, .end = 0x8ABFBC, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_bcd_to_binary, ALL_OUTPUTS, .fuzz = fuzz_func_8ABFBC},
  {.name = "func_80899F", .entry = 0x80899F, .end = 0x8089B2, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_func_80899F, ALL_OUTPUTS, .fuzz = fuzz_func_80899F},
  {.name = "func_8AA022", .entry = 0x8AA022, .end = 0x8AA03C, .source = "src/misc.c", .needM16 = true,
   .fixedD = true, .call = call_func_8AA022, ALL_OUTPUTS, .fuzz = fuzz_func_8AA022},
  {.name = "func_80B7BB", .entry = 0x80B7BB, .end = 0x80B7CE, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_func_80B7BB, ALL_OUTPUTS, .fuzz = fuzz_func_80B7BB},
  {.name = "byte_buffer_decimal2", .entry = 0x8A9A81, .end = 0x8A9A99, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_byte_buffer_decimal2, ALL_OUTPUTS,
   .fuzz = fuzz_byte_buffer_decimal2},
  {.name = "wait_frames_82", .entry = 0x82841E, .end = 0x828428, .source = "src/frame.c", .needM16 = true,
   .lowWramDb = true, .waitsForInterrupts = true, .call = call_wait_frames, ALL_OUTPUTS},
  {.name = "byte_buffer_pad", .entry = 0x8A99B4, .end = 0x8A99BE, .source = "src/misc.c", .needM16 = true,
   .fixedD = true, .call = call_byte_buffer_pad, ALL_OUTPUTS, .fuzz = fuzz_byte_buffer_pad},
  {.name = "byte_buffer_end", .entry = 0x8A99D6, .end = 0x8A99E2, .source = "src/misc.c", .needM16 = true,
   .fixedD = true, .call = call_byte_buffer_end, ALL_OUTPUTS, .fuzz = fuzz_byte_buffer_push},
  {.name = "byte_buffer_decimal", .entry = 0x8A9A33, .end = 0x8A9A81, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_byte_buffer_decimal, ALL_OUTPUTS,
   .fuzz = fuzz_byte_buffer_decimal},
  {.name = "func_80E4C5", .entry = 0x80E4C5, .end = 0x80E4D0, .rtl = true, .source = "src/misc.c", .needM16 = true,
   .fixedD = true, .call = call_func_80E4C5, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "func_80E4DB", .entry = 0x80E4DB, .end = 0x80E4E6, .rtl = true, .source = "src/misc.c", .needM16 = true,
   .fixedD = true, .call = call_func_80E4DB, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "func_80B58D", .entry = 0x80B58D, .end = 0x80B59D, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_func_80B58D, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "func_8ABFBC", .entry = 0x8ABFBC, .end = 0x8ABFD0, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_func_8ABFBC, ALL_OUTPUTS, .fuzz = fuzz_func_8ABFBC},
  {.name = "queue_unpacked_upload", .entry = 0x80B95D, .end = 0x80B97B, .source = "src/decompress.c",
   .needM16 = true, .needX16 = true, .waitsForInterrupts = true, .call = call_queue_unpacked_upload, ALL_OUTPUTS,
   .fuzz = fuzz_queue_unpacked_upload},
  {.name = "func_80B537", .entry = 0x80B537, .end = 0x80B556, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_func_80B537, ALL_OUTPUTS, .fuzz = fuzz_func_80B537},
  {.name = "byte_buffer_space", .entry = 0x8A9A99, .end = 0x8A9AA5, .source = "src/misc.c", .needM16 = true,
   .fixedD = true, .call = call_byte_buffer_space, ALL_OUTPUTS, .fuzz = fuzz_byte_buffer_push},
  {.name = "sprite_number_right", .entry = 0x808698, .end = 0x8086C9, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_sprite_number_right, ALL_OUTPUTS,
   .fuzz = fuzz_sprite_number},
  {.name = "func_80EB66", .entry = 0x80EB66, .end = 0x80EBBB, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .call = call_func_80EB66, ALL_OUTPUTS, .fuzz = fuzz_func_80EB66},
  {.name = "oam_finish_0b00_near", .entry = 0x8089D1, .end = 0x8089D6, .source = "src/sprites.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_oam_finish_0b00_near, ALL_OUTPUTS, .fuzz = fuzz_oam_finish_0b00},
  {.name = "oam_finish_near", .entry = 0x8089D6, .end = 0x8089DB, .source = "src/sprites.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_oam_finish_near, ALL_OUTPUTS, .fuzz = fuzz_oam_finish},
  {.name = "upload_vram_now", .entry = 0x8083AD, .end = 0x8083CB, .rtl = true, .source = "src/video_dma.c",
   .needM16 = true, .needX16 = true, .call = call_upload_vram_now, ALL_OUTPUTS, .fuzz = fuzz_upload_vram_now},
  {.name = "upload_cgram_now", .entry = 0x8083CB, .end = 0x8083E8, .rtl = true, .source = "src/video_dma.c",
   .needM16 = true, .needX16 = true, .call = call_upload_cgram_now, ALL_OUTPUTS, .fuzz = fuzz_upload_cgram_now},
  {.name = "rng_next_long", .entry = 0x8087DF, .end = 0x8087E9, .rtl = true, .source = "src/rng.c",
   .needM16 = true, .call = call_rng_next_long, ALL_OUTPUTS, .fuzz = fuzz_rng_next_long},
  {.name = "star_spawn", .entry = 0x80FA3F, .end = 0x80FA5B, .source = "src/stars.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_star_spawn, ALL_OUTPUTS, .fuzz = fuzz_star_spawn},
  {.name = "star_spawn_8a", .entry = 0x8A9863, .end = 0x8A987F, .source = "src/stars.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_star_spawn, ALL_OUTPUTS, .fuzz = fuzz_star_spawn},
  {.name = "star_random_y", .entry = 0x80E166, .end = 0x80E175, .source = "src/stars.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_star_random_y, ALL_OUTPUTS, .fuzz = fuzz_star_random_y},
  {.name = "upload_7f_to_3000", .entry = 0x8ABA7D, .end = 0x8ABA90, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_upload_7f_to_3000, ALL_OUTPUTS, .fuzz = fuzz_upload_tiles},
  {.name = "upload_tiles_3300", .entry = 0x858245, .end = 0x858260, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_upload_tiles_3300, ALL_OUTPUTS, .fuzz = fuzz_upload_tiles},
  {.name = "upload_tiles_3700", .entry = 0x858260, .end = 0x85827B, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_upload_tiles_3700, ALL_OUTPUTS, .fuzz = fuzz_upload_tiles},
  {.name = "upload_tiles_7700", .entry = 0x85827B, .end = 0x858296, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_upload_tiles_7700, ALL_OUTPUTS, .fuzz = fuzz_upload_tiles},
  {.name = "upload_tiles_7b00", .entry = 0x858296, .end = 0x8582B1, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_upload_tiles_7b00, ALL_OUTPUTS, .fuzz = fuzz_upload_tiles},
  {.name = "star_layers_init", .entry = 0x80E0D6, .end = 0x80E166, .source = "src/stars.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_star_layers_init, ALL_OUTPUTS, .fuzz = fuzz_star_layers_init},
  {.name = "func_81F251", .entry = 0x81F251, .end = 0x81F252, .source = "src/misc.c",
   .call = call_func_81F251, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "clear_dp_scratch", .entry = 0x8A8001, .end = 0x8A800B, .rtl = true, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_clear_dp_scratch, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "func_81F1FC", .entry = 0x81F1FC, .end = 0x81F21E, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x7E, .call = call_func_81F1FC, ALL_OUTPUTS, .fuzz = fuzz_func_81F1FC},
  {.name = "func_8AA216", .entry = 0x8AA216, .end = 0x8AA25D, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_func_8AA216, ALL_OUTPUTS, .fuzz = fuzz_func_8AA216},
  {.name = "sprite_frame", .entry = 0x8AA0F9, .end = 0x8AA216, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_sprite_frame, ALL_OUTPUTS, .fuzz = fuzz_sprite_frame},
  {.name = "func_8A9FA1", .entry = 0x8A9FA1, .end = 0x8A9FE9, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x8A, .fixedD = true, .call = call_func_8A9FA1, ALL_OUTPUTS, .fuzz = fuzz_func_8A9FA1},
  {.name = "clear_tilemap_7f", .entry = 0x8AB981, .end = 0x8AB994, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .call = call_clear_tilemap_7f, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "func_8ABC32", .entry = 0x8ABC32, .end = 0x8ABC3D, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_func_8ABC32, ALL_OUTPUTS, .fuzz = fuzz_func_8ABC32},
  {.name = "func_8ABC7F", .entry = 0x8ABC7F, .end = 0x8ABC8A, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_func_8ABC7F, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "func_82EC09", .entry = 0x82EC09, .end = 0x82EC2B, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_func_82EC09, ALL_OUTPUTS, .fuzz = fuzz_func_82EC09},
  {.name = "text_print_tiles", .entry = 0x8AC13F, .end = 0x8AC15D, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_text_print_tiles, ALL_OUTPUTS, .fuzz = fuzz_text_print_tiles},
  {.name = "wipe_columns", .entry = 0x8AB91A, .end = 0x8AB942, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .waitsForInterrupts = true, .call = call_wipe_columns, ALL_OUTPUTS},
  {.name = "func_8AA48D", .entry = 0x8AA48D, .end = 0x8AA49E, .source = "src/misc.c", .needM16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_func_8AA48D, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "func_8AA49E", .entry = 0x8AA49E, .end = 0x8AA4AF, .source = "src/misc.c", .needM16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_func_8AA49E, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "clear_sprites", .entry = 0x8A8F02, .end = 0x8A8F18, .rtl = true, .source = "src/misc.c",
   .lowWramDb = true, .call = call_clear_sprites, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "text_print_tiles_at", .entry = 0x8AC137, .end = 0x8AC13F, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_text_print_tiles_at, ALL_OUTPUTS, .fuzz = fuzz_text_print_tiles_at},
  {.name = "func_8A92CF", .entry = 0x8A92CF, .end = 0x8A9307, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_func_8A92CF, ALL_OUTPUTS, .fuzz = fuzz_func_8A92CF},
  {.name = "func_80F745", .entry = 0x80F745, .end = 0x80F8F8, .source = "src/line_tables.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_func_80F745, ALL_OUTPUTS, .fuzz = fuzz_func_80F745},
  {.name = "tilemap_offset", .entry = 0x8AB909, .end = 0x8AB91A, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_tilemap_offset, ALL_OUTPUTS, .fuzz = fuzz_tilemap_offset},
  {.name = "func_8AB700", .entry = 0x8AB700, .end = 0x8AB721, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_func_8AB700, ALL_OUTPUTS, .fuzz = fuzz_func_8AB700},
  {.name = "tilemap_offset_c2cc", .entry = 0x8AC2CC, .end = 0x8AC2DD, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_tilemap_offset, ALL_OUTPUTS, .fuzz = fuzz_tilemap_offset},
  {.name = "text_put_char", .entry = 0x8AB83C, .end = 0x8AB8FA, .source = "src/text.c", .needX16 = true,
   .lowWramDb = true, .fixedD = true, .db = 0x8A, .call = call_text_put_char, ALL_OUTPUTS, .fuzz = fuzz_text_put_char},
  {.name = "char_to_tile_index", .entry = 0x8AC2A2, .end = 0x8AC2CC, .source = "src/text.c", .needM16 = true,
   .call = call_char_to_tile_index, ALL_OUTPUTS, .fuzz = fuzz_char_to_tile_index},
  {.name = "step_toward_0d80", .entry = 0x8AA264, .end = 0x8AA27D, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_step_toward_0d80, ALL_OUTPUTS, .fuzz = fuzz_step_toward_0d80},
  {.name = "step_toward_0d70", .entry = 0x8AA27D, .end = 0x8AA296, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_step_toward_0d70, ALL_OUTPUTS, .fuzz = fuzz_step_toward_0d70},
  {.name = "step_toward_0d68", .entry = 0x8AA296, .end = 0x8AA2AF, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_step_toward_0d68, ALL_OUTPUTS, .fuzz = fuzz_step_toward_0d68},
  {.name = "wait_frames", .entry = 0x808373, .end = 0x80837D, .source = "src/frame.c", .needM16 = true,
   .lowWramDb = true, .waitsForInterrupts = true, .call = call_wait_frames, ALL_OUTPUTS},
  {.name = "wait_frames_long", .entry = 0x8ABA90, .end = 0x8ABA9C, .rtl = true, .source = "src/frame.c",
   .needM16 = true, .waitsForInterrupts = true, .call = call_wait_frames, ALL_OUTPUTS},
  {.name = "text_print_centered", .entry = 0x8AB6AA, .end = 0x8AB700, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_text_print_centered, ALL_OUTPUTS, .fuzz = fuzz_text_print_centered},
  {.name = "text_put_tile_char", .entry = 0x8AC278, .end = 0x8AC2A2, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_text_put_tile_char, ALL_OUTPUTS, .fuzz = fuzz_text_put_tile_char},
  {.name = "byte_buffer_push", .entry = 0x8A9C19, .end = 0x8A9C27, .source = "src/misc.c", .needX16 = true,
   .fixedD = true, .call = call_byte_buffer_push, ALL_OUTPUTS, .fuzz = fuzz_byte_buffer_push},
  {.name = "text_cursor_down3", .entry = 0x8AA37A, .end = 0x8AA381, .source = "src/text.c", .needM16 = true,
   .fixedD = true, .call = call_text_cursor_down3, ALL_OUTPUTS, .fuzz = fuzz_text_cursor_down3},
  {.name = "func_8AB942", .entry = 0x8AB942, .end = 0x8AB981, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_func_8AB942, ALL_OUTPUTS, .fuzz = fuzz_func_8AB942},
  {.name = "decompress", .entry = 0x82FB70, .end = 0x82FFAB, .rtl = true, .source = "src/decompress.c",
   .needM16 = true, .needX16 = true, .call = call_decompress, ALL_OUTPUTS, .fuzz = fuzz_decompress,
   .holes = {{0x82FB8B, 0x82FC8B}, {0x82FD58, 0x82FD78}, {0x82FDD7, 0x82FDF7}, {0x82FE99, 0x82FEB9}}},
  {.name = "decompress_near", .entry = 0x80FBA0, .end = 0x80FBA5, .source = "src/decompress.c",
   .needM16 = true, .needX16 = true, .call = call_decompress_near, ALL_OUTPUTS, .fuzz = fuzz_decompress},
  {.name = "clear_object_slots", .entry = 0x8AA47E, .end = 0x8AA48D, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_clear_object_slots, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "func_8AA469", .entry = 0x8AA469, .end = 0x8AA47E, .source = "src/misc.c", .needM16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_func_8AA469, ALL_OUTPUTS, .fuzz = fuzz_8aa469},
  {.name = "string_list_nth", .entry = 0x8AA4CF, .end = 0x8AA4FB, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_string_list_nth, ALL_OUTPUTS, .fuzz = fuzz_string_list_nth},
  {.name = "take_nibble", .entry = 0x808542, .end = 0x80855A, .source = "src/misc.c", .needM16 = true,
   .fixedD = true, .call = call_take_nibble, ALL_OUTPUTS, .fuzz = fuzz_take_nibble},
  {.name = "func_83FE9A", .entry = 0x83FE9A, .end = 0x83FEBD, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_func_83FE9A, ALL_OUTPUTS, .fuzz = fuzz_func_83FE9A},
  {.name = "sprite_text", .entry = 0x80841A, .end = 0x8084B0, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_sprite_text, ALL_OUTPUTS, .fuzz = fuzz_sprite_text,
   .m8At = {0x808435}},
  {.name = "sprite_text_centered", .entry = 0x8084BE, .end = 0x8084F0, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_sprite_text_centered, ALL_OUTPUTS,
   .fuzz = fuzz_sprite_text_centered},
  {.name = "sprite_number", .entry = 0x8084F0, .end = 0x808542, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_sprite_number, ALL_OUTPUTS,
   .fuzz = fuzz_sprite_number},
  {.name = "sprite_group", .entry = 0x80B49E, .end = 0x80B535, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .call = call_sprite_group, ALL_OUTPUTS, .fuzz = fuzz_sprite_group},
  {.name = "func_83EA9B", .entry = 0x83EA9B, .end = 0x83EAC6, .source = "src/misc.c", .needM16 = true,
   .lowWramDb = true, .call = call_func_83EA9B, ALL_OUTPUTS, .fuzz = fuzz_func_83EA9B},
  {.name = "oam_high_table", .entry = 0x818101, .end = 0x818487, .source = "src/sprites.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_oam_high_table, ALL_OUTPUTS, .fuzz = fuzz_oam_high_table},
  // enters the packer above after its own prologue: the other entry's prologue is a hole
  {.name = "oam_high_table_0b00", .entry = 0x8180F6, .end = 0x818487, .source = "src/sprites.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_oam_high_table_0b00, ALL_OUTPUTS, .fuzz = fuzz_oam_high_table_0b00,
   .holes = {{0x818101, 0x81810A}}},
  {.name = "func_8ABB5C", .entry = 0x8ABB5C, .end = 0x8ABB99, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_func_8ABB5C, ALL_OUTPUTS, .fuzz = fuzz_big_number},
  {.name = "password_encode", .entry = 0x8ABC8A, .end = 0x8ABCEA, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .db = 0x8A, .call = call_password_encode, ALL_OUTPUTS,
   .fuzz = fuzz_password_encode},
  {.name = "func_8ABBF4", .entry = 0x8ABBF4, .end = 0x8ABC32, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_func_8ABBF4, ALL_OUTPUTS, .fuzz = fuzz_big_number},
  {.name = "func_81FBB1", .entry = 0x81FBB1, .end = 0x81FC72, .source = "src/track.c", .needM16 = true,
   .needX16 = true, .db = 0x7E, .fixedD = true, .call = call_func_81FBB1, ALL_OUTPUTS, .fuzz = fuzz_func_81FBB1},
  {.name = "car_road_step", .entry = 0x81F887, .end = 0x81FA8F, .source = "src/track.c", .needM16 = true,
   .needX16 = true, .db = 0x7E, .fixedD = true, .call = call_car_road_step, .fuzz = fuzz_car_road_step},
  {.name = "player_road_limits", .entry = 0x83E8AB, .end = 0x83EA9B, .source = "src/track.c", .needM16 = true,
   .needX16 = true, .call = call_player_road_limits, .fuzz = fuzz_player_road_limits},
  {.name = "shop_part_level_2", .entry = 0x809F23, .end = 0x809F28, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .call = call_shop_part_level_2, ALL_OUTPUTS, .fuzz = fuzz_shop_part_level},
  {.name = "shop_part_level_3", .entry = 0x809F0A, .end = 0x809F0F, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .call = call_shop_part_level_3, ALL_OUTPUTS, .fuzz = fuzz_shop_part_level},
  {.name = "shop_part_level_4", .entry = 0x809F0F, .end = 0x809F14, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .call = call_shop_part_level_4, ALL_OUTPUTS, .fuzz = fuzz_shop_part_level},
  {.name = "shop_part_level_5", .entry = 0x809F14, .end = 0x809F19, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .call = call_shop_part_level_5, ALL_OUTPUTS, .fuzz = fuzz_shop_part_level},
  {.name = "shop_part_level_6", .entry = 0x809F19, .end = 0x809F1E, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .call = call_shop_part_level_6, ALL_OUTPUTS, .fuzz = fuzz_shop_part_level},
  {.name = "shop_part_level_7", .entry = 0x809F1E, .end = 0x809F23, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .call = call_shop_part_level_7, ALL_OUTPUTS, .fuzz = fuzz_shop_part_level},
  {.name = "shop_money_take", .entry = 0x80993E, .end = 0x809957, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_shop_money_take, ALL_OUTPUTS, .fuzz = fuzz_shop_money_take},
  {.name = "shop_flag_fb", .entry = 0x80E4D0, .end = 0x80E4DB, .rtl = true, .source = "src/shop.c", .needM16 = true,
   .fixedD = true, .call = call_shop_flag_fb, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "shop_panel_upload", .entry = 0x809DBC, .end = 0x809DD0, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_shop_panel_upload, ALL_OUTPUTS, .alsoIn = {{0x80B537, 0x80B556}},
   .fuzz = fuzz_shop_panel_upload},
  {.name = "shop_item_picture", .entry = 0x809BF2, .end = 0x809C0B, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_picture, ALL_OUTPUTS, .alsoIn = {{0x80B58D, 0x80B59D}},
   .fuzz = fuzz_shop_item},
  {.name = "shop_item_text", .entry = 0x80B096, .end = 0x80B0C0, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_text, ALL_OUTPUTS, .fuzz = fuzz_shop_item},
  {.name = "shop_sprite_block", .entry = 0x80A7E4, .end = 0x80A831, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_shop_sprite_block, ALL_OUTPUTS, .fuzz = fuzz_shop_sprite_block},
  {.name = "sprite_number_centered", .entry = 0x808635, .end = 0x808667, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_sprite_number_centered, ALL_OUTPUTS,
   .fuzz = fuzz_sprite_number},
  {.name = "shop_bar_cap", .entry = 0x80AAF1, .end = 0x80AB54, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_shop_bar_cap, ALL_OUTPUTS, .fuzz = fuzz_shop_bar},
  {.name = "shop_bar_column", .entry = 0x80A9BD, .end = 0x80AA15, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_shop_bar_column, ALL_OUTPUTS, .fuzz = fuzz_shop_bar},
  {.name = "shop_bar_levels", .entry = 0x80A403, .end = 0x80A4B8, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_bar_levels, .fuzz = fuzz_shop_bars},
  {.name = "shop_bar_segments", .entry = 0x80A4B8, .end = 0x80A579, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_shop_bar_segments, .fuzz = fuzz_shop_bars},
  {.name = "shop_bar_segments_long", .entry = 0x80A87D, .end = 0x80A91E, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_shop_bar_segments_long, .fuzz = fuzz_shop_bars,
   .holes = {{0x80A8A8, 0x80A8AD}}},  // dead bytes after its JMP (never decoded as code)
  {.name = "shop_bar_double", .entry = 0x80A91E, .end = 0x80A9BD, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_bar_double, .fuzz = fuzz_shop_bars},
  {.name = "shop_gauge", .entry = 0x80AA59, .end = 0x80AAF1, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_shop_gauge, .fuzz = fuzz_shop_bars},
  {.name = "shop_item_price", .entry = 0x80A26E, .end = 0x80A2CB, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_price, ALL_OUTPUTS, .fuzz = fuzz_shop_item_price},
  {.name = "shop_item_view_a", .entry = 0x80A2CB, .end = 0x80A333, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_view_a, .fuzz = fuzz_shop_item_view_a},
  {.name = "shop_item_view_b", .entry = 0x80A831, .end = 0x80A87D, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_view_b, .fuzz = fuzz_shop_item_view_b},
  {.name = "shop_item_view_c", .entry = 0x80AA15, .end = 0x80AA59, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_view_c, .fuzz = fuzz_shop_item_view_c},
  {.name = "shop_item_view_d", .entry = 0x80A717, .end = 0x80A7E4, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_view_d, .fuzz = fuzz_shop_item_view_d},
  {.name = "shop_item_view_e", .entry = 0x80AB54, .end = 0x80ABF6, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_view_e, .fuzz = fuzz_shop_item_view_e},
  {.name = "shop_item_icon_draw", .entry = 0x80A1E4, .end = 0x80A251, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_icon_draw, .fuzz = fuzz_shop_item_icon_draw},
  {.name = "shop_item_icon_1", .entry = 0x809F5B, .end = 0x809F9A, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_icon_1, .alsoIn = {{0x80A1E4, 0x80A251}}, .fuzz = fuzz_shop_item_icon},
  {.name = "shop_item_icon_2", .entry = 0x809F9A, .end = 0x809FD9, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_icon_2, .alsoIn = {{0x80A1E4, 0x80A251}}, .fuzz = fuzz_shop_item_icon},
  {.name = "shop_item_icon_3", .entry = 0x809FD9, .end = 0x80A018, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_icon_3, .alsoIn = {{0x80A1E4, 0x80A251}}, .fuzz = fuzz_shop_item_icon},
  {.name = "shop_item_icon_4", .entry = 0x80A057, .end = 0x80A096, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_icon_4, .alsoIn = {{0x80A1E4, 0x80A251}}, .fuzz = fuzz_shop_item_icon},
  {.name = "shop_item_icon_5", .entry = 0x80A018, .end = 0x80A057, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_icon_5, .alsoIn = {{0x80A1E4, 0x80A251}}, .fuzz = fuzz_shop_item_icon},
  {.name = "shop_item_dispatch_icon", .entry = 0x809F40, .end = 0x809F5B, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_shop_item_dispatch_icon, .fuzz = fuzz_shop_item_dispatch},
  {.name = "shop_item_dispatch_upload", .entry = 0x809C0B, .end = 0x809C28, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_shop_item_dispatch_upload, .compareX = true,
   .fuzz = fuzz_shop_item_dispatch},
  {.name = "shop_level_marks", .entry = 0x809E49, .end = 0x809EEA, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_level_marks, .fuzz = fuzz_shop_level_marks},
  {.name = "shop_panel_blank", .entry = 0x80B0C0, .end = 0x80B10C, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .call = call_shop_panel_blank, ALL_OUTPUTS, .fuzz = fuzz_shop_panel},
  {.name = "shop_panel_clear", .entry = 0x80B10C, .end = 0x80B149, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .waitsForInterrupts = true, .call = call_shop_panel_clear, ALL_OUTPUTS, .fuzz = fuzz_shop_panel_clear},
  {.name = "shop_list_locked", .entry = 0x80B359, .end = 0x80B371, .source = "src/shop.c", .needM16 = true,
   .fixedD = true, .call = call_shop_list_locked, ALL_OUTPUTS, .fuzz = fuzz_shop_list},
  {.name = "shop_list_prev", .entry = 0x80B333, .end = 0x80B345, .source = "src/shop.c", .needM16 = true,
   .fixedD = true, .call = call_shop_list_prev, .fuzz = fuzz_shop_list},
  {.name = "shop_list_next", .entry = 0x80B345, .end = 0x80B359, .source = "src/shop.c", .needM16 = true,
   .fixedD = true, .call = call_shop_list_next, .fuzz = fuzz_shop_list},
  {.name = "star_layers_move", .entry = 0x80DACF, .end = 0x80DAED, .source = "src/stars.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_star_layers_move, ALL_OUTPUTS, .fuzz = fuzz_star_layers_move},
  {.name = "fade_out_wait", .entry = 0x8ABAE3, .end = 0x8ABAF2, .source = "src/misc.c", .needM16 = true,
   .lowWramDb = true, .waitsForInterrupts = true, .call = call_fade_out_wait, ALL_OUTPUTS},
  {.name = "fade_out_on_confirm", .entry = 0x8A9675, .end = 0x8A968E, .source = "src/misc.c", .needM16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_fade_out_on_confirm, ALL_OUTPUTS, .fuzz = fuzz_fade_out_on_confirm},
  {.name = "fade_out_on_confirm_once", .entry = 0x8A8181, .end = 0x8A8196, .alsoIn = {{0x8A81B0, 0x8A81BA}}, .source = "src/misc.c", .needM16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_fade_out_on_confirm_once, ALL_OUTPUTS, .fuzz = fuzz_fade_out_on_confirm},
  {.name = "func_8A99D0", .entry = 0x8A99D0, .end = 0x8A99D6, .source = "src/misc.c", .needM16 = true,
   .fixedD = true, .call = call_func_8A99D0, ALL_OUTPUTS, .fuzz = fuzz_dp_40_66},
  {.name = "func_8A96C2", .entry = 0x8A96C2, .end = 0x8A96C8, .source = "src/misc.c", .needM16 = true,
   .fixedD = true, .call = call_func_8A96C2, ALL_OUTPUTS, .fuzz = fuzz_dp_40_66},
  {.name = "shop_item_view_g", .entry = 0x80ABF6, .end = 0x80AC37, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_view_g, .fuzz = fuzz_shop_item_view_g},
  {.name = "interrupt_vectors_init", .entry = 0x8080E4, .end = 0x808119, .source = "src/frame.c", .needM16 = true,
   .lowWramDb = true, .call = call_interrupt_vectors_init, ALL_OUTPUTS, .fuzz = fuzz_interrupt_vectors_init},
  {.name = "hdma_table_0d00_init", .entry = 0x8A9C86, .end = 0x8A9CB0, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_hdma_table_0d00_init, ALL_OUTPUTS, .fuzz = fuzz_hdma_table_0d00_init},
  {.name = "string_space_fours", .entry = 0x8ABF2A, .end = 0x8ABF4E, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_string_space_fours, ALL_OUTPUTS, .fuzz = fuzz_string_space_fours},
  {.name = "table_062a_fill", .entry = 0x8A8AE7, .end = 0x8A8B0A, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_table_062a_fill, ALL_OUTPUTS, .fuzz = fuzz_table_062a_fill},
  {.name = "shop_item_view_f", .entry = 0x80A333, .end = 0x80A39B, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_view_f, .fuzz = fuzz_shop_item_view_f},
  {.name = "shop_ring", .entry = 0x80A579, .end = 0x80A5F0, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_shop_ring, .fuzz = fuzz_shop_ring},
  {.name = "shop_ring_piece_front", .entry = 0x80A69F, .end = 0x80A6DB, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_shop_ring_piece_front, ALL_OUTPUTS, .fuzz = fuzz_shop_ring},
  {.name = "shop_ring_piece_back", .entry = 0x80A6DB, .end = 0x80A717, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_shop_ring_piece_back, ALL_OUTPUTS, .fuzz = fuzz_shop_ring},
  {.name = "shop_item_dispatch_view", .entry = 0x80A251, .end = 0x80A26E, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_shop_item_dispatch_view, .fuzz = fuzz_shop_item_dispatch_view},
  {.name = "shop_item_dispatch_panel", .entry = 0x809C28, .end = 0x809C47, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_shop_item_dispatch_panel, .compareX = true,
   .fuzz = fuzz_shop_item_dispatch_panel},
  {.name = "shop_item_redraw", .entry = 0x809847, .end = 0x809870, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_shop_item_redraw, .fuzz = fuzz_shop_item_redraw},
  // the slides wait for frames (no fuzz: the waits need the movie's interrupts)
  {.name = "shop_item_slide_row", .entry = 0x809AC8, .end = 0x809B5D, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .waitsForInterrupts = true, .call = call_shop_item_slide_row},
  {.name = "shop_item_slide_list", .entry = 0x809B5D, .end = 0x809B99, .alsoIn = {{0x809B99, 0x809BF2}},
   .source = "src/shop.c", .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .waitsForInterrupts = true,
   .call = call_shop_item_slide_list},
  {.name = "shop_item_slide_list_in", .entry = 0x809B99, .end = 0x809BF2, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .waitsForInterrupts = true, .call = call_shop_item_slide_list_in},
  {.name = "shop_item_down", .entry = 0x808FA1, .end = 0x808FD4, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .waitsForInterrupts = true, .mayLeaveCaller = true,
   .call = call_shop_item_down},
  {.name = "shop_item_up", .entry = 0x808FD4, .end = 0x808FFC, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .waitsForInterrupts = true, .mayLeaveCaller = true,
   .call = call_shop_item_up},
  {.name = "shop_item_show_icon", .entry = 0x809832, .end = 0x809847, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .waitsForInterrupts = true, .call = call_shop_item_show_icon},
  // its never-run branches ($80:9892, $80:98B5: an icon below step 8) are left out of the
  // paths: no movie runs them and a waiting routine has no fuzz, so that C is unchecked
  {.name = "shop_item_icon_settle", .entry = 0x809870, .end = 0x8098C4, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .waitsForInterrupts = true, .call = call_shop_item_icon_settle,
   .holes = {{0x809892, 0x8098AE}, {0x8098B5, 0x8098B6}}},
  {.name = "shop_item_affordable", .entry = 0x8098C4, .end = 0x80990D, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .waitsForInterrupts = true, .call = call_shop_item_affordable,
   .fuzz = fuzz_shop_item_affordable},
  {.name = "shop_item_view_h", .entry = 0x80AEBE, .end = 0x80AF36, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_view_h},
  {.name = "shop_item_view_i", .entry = 0x80AE4E, .end = 0x80AEBE, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_view_i},
  {.name = "shop_item_view_j", .entry = 0x80AC37, .end = 0x80ACB6, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_view_j},
  {.name = "shop_level_blocks", .entry = 0x80AF36, .end = 0x80AFE1, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .call = call_shop_level_blocks},
  {.name = "shop_gauge_lines", .entry = 0x80ACB6, .end = 0x80ADD9, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .call = call_shop_gauge_lines},
  {.name = "shop_gauge_line", .entry = 0x80ADD9, .end = 0x80AE4E, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x7E, .call = call_shop_gauge_line},
  // the control codes' handlers follow the code table ($80:B6BC-B6C9, data)
  {.name = "text_type_step", .entry = 0x80B59D, .end = 0x80B6BC, .alsoIn = {{0x80B6CA, 0x80B7BB}, {0x80B7CE, 0x80B7D9}},
   .source = "src/text.c", .needM16 = true, .needX16 = true, .fixedD = true, .call = call_text_type_step,
   .fuzz = fuzz_text_type_step, .m8At = {0x80B645, 0x80B65E}},
  {.name = "password_pack", .entry = 0x8ABB99, .end = 0x8ABBD7, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .db = 0x8A, .call = call_password_pack, ALL_OUTPUTS,
   .fuzz = fuzz_password_pack},
  {.name = "hdma_table_0d00_step", .entry = 0x8A9C27, .end = 0x8A9C48, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_hdma_table_0d00_step,
   .fuzz = fuzz_hdma_table_0d00_step},
  {.name = "race_lights_draw", .entry = 0x80DAED, .end = 0x80DB7D, .source = "src/hud.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_race_lights_draw, .fuzz = fuzz_race_lights_draw},
  {.name = "race_order_sort", .entry = 0x85A066, .end = 0x85A118, .rtl = true, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_race_order_sort, ALL_OUTPUTS, .fuzz = fuzz_race_order_sort},
  {.name = "hdma_table_0d00_settle", .entry = 0x8A9C48, .end = 0x8A9C69, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_hdma_table_0d00_settle,
   .fuzz = fuzz_hdma_table_0d00_step},
  {.name = "steer_state_init", .entry = 0x80D6A0, .end = 0x80D6F3, .source = "src/misc.c", .needM16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_steer_state_init, .compareA = true, .fuzz = fuzz_steer_state_init},
  {.name = "hdma_gradient_build", .entry = 0x8A8FA4, .end = 0x8A9202, .rtl = true, .source = "src/misc.c",
   .lowWramDb = true, .fixedD = true, .call = call_hdma_gradient_build, .fuzz = fuzz_hdma_gradient_build},
  {.name = "flag_03dc_from_52", .entry = 0x8A9F64, .end = 0x8A9F76, .source = "src/misc.c", .needM16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_flag_03dc_from_52, ALL_OUTPUTS, .fuzz = fuzz_flag_03dc_from_52},
  // its never-run branch calls $8A:AE9B (rng_next's third copy, also never run): the fuzz
  // runs both, so they are compared through this row
  {.name = "timer_0d7e_tick", .entry = 0x8A9F76, .end = 0x8A9FA1, .source = "src/misc.c", .needM16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_timer_0d7e_tick, ALL_OUTPUTS, .fuzz = fuzz_timer_0d7e_tick},
  {.name = "text_line_put", .entry = 0x8A9B85, .end = 0x8A9BAA, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_text_line_put, ALL_OUTPUTS, .fuzz = fuzz_text_line_put},
  {.name = "text_centred_line", .entry = 0x82B18B, .end = 0x82B234, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x7E, .call = call_text_centred_line, .fuzz = fuzz_text_centred_line},
  {.name = "shop_list_label", .entry = 0x80B2F2, .end = 0x80B333, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_shop_list_label, ALL_OUTPUTS, .fuzz = fuzz_shop_list_label},
  {.name = "race_clear_0242_0208", .entry = 0x82ECEA, .end = 0x82ECF1, .source = "src/misc.c", .needM16 = true,
   .lowWramDb = true, .call = call_race_clear_0242_0208, .fuzz = fuzz_race_clear_0242_0208},
  {.name = "race_countdown", .entry = 0x82AB9D, .end = 0x82AC52, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_race_countdown, .fuzz = fuzz_race_countdown},
  {.name = "race_scroll_commit", .entry = 0x82ECF1, .end = 0x82ED17, .source = "src/misc.c", .needM16 = true,
   .lowWramDb = true, .call = call_race_scroll_commit, .compareA = true, .fuzz = fuzz_race_scroll_commit},
  {.name = "car_lap_check", .entry = 0x83F9D1, .end = 0x83FA32, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_car_lap_check, .fuzz = fuzz_car_lap_check},
  {.name = "oam_finish_0b00", .entry = 0x8180D0, .end = 0x8180DA, .rtl = true, .source = "src/sprites.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_oam_finish_0b00, ALL_OUTPUTS,
   .fuzz = fuzz_oam_finish_0b00},
  {.name = "oam_finish", .entry = 0x8180DA, .end = 0x8180F6, .rtl = true, .source = "src/sprites.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_oam_finish, ALL_OUTPUTS, .fuzz = fuzz_oam_finish},
  {.name = "take_nibble16", .entry = 0x8085CA, .end = 0x8085DA, .source = "src/misc.c", .needM16 = true,
   .fixedD = true, .call = call_take_nibble16, ALL_OUTPUTS, .fuzz = fuzz_take_nibble16},
  {.name = "func_82B78D", .entry = 0x82B78D, .end = 0x82B7B0, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_func_82B78D, ALL_OUTPUTS, .fuzz = fuzz_func_82B78D},
  {.name = "func_82B7B0", .entry = 0x82B7B0, .end = 0x82B838, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x83, .fixedD = true, .call = call_func_82B7B0, ALL_OUTPUTS, .fuzz = fuzz_func_82B7B0},
  {.name = "func_82B838", .entry = 0x82B838, .end = 0x82B87A, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .call = call_func_82B838, ALL_OUTPUTS, .fuzz = fuzz_func_82B838},
  {.name = "func_80DC5A", .entry = 0x80DC5A, .end = 0x80DCBB, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x7F, .fixedD = true, .call = call_func_80DC5A, ALL_OUTPUTS, .fuzz = fuzz_func_80DC5A},
  {.name = "func_8A9BB5", .entry = 0x8A9BB5, .end = 0x8A9BE2, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_func_8A9BB5, ALL_OUTPUTS, .fuzz = fuzz_func_8A9BB5},
  {.name = "func_8A8EEF", .entry = 0x8A8EEF, .end = 0x8A8F02, .rtl = true, .source = "src/misc.c",
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_func_8A8EEF, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "func_8A9BAA", .entry = 0x8A9BAA, .end = 0x8A9BB5, .source = "src/misc.c", .needM16 = true,
   .fixedD = true, .call = call_func_8A9BAA, ALL_OUTPUTS, .fuzz = fuzz_func_8A9BAA},
  {.name = "func_8A9BE2", .entry = 0x8A9BE2, .end = 0x8A9BF6, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_func_8A9BE2, ALL_OUTPUTS, .fuzz = fuzz_func_8A9BE2},
  {.name = "func_8AB680", .entry = 0x8AB680, .end = 0x8AB6AA, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x8A, .fixedD = true, .call = call_func_8AB680, ALL_OUTPUTS, .fuzz = fuzz_func_8AB680},
  {.name = "func_83FC05", .entry = 0x83FC05, .end = 0x83FC65, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_func_83FC05, ALL_OUTPUTS, .fuzz = fuzz_func_83FC05},
  // its hit tail is at $83:FE16; $83:FDCD-$83:FE15 (another routine and unknown bytes) is a hole
  {.name = "func_83FD8A", .entry = 0x83FD8A, .end = 0x83FE9A, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_func_83FD8A, ALL_OUTPUTS, .fuzz = fuzz_func_83FD8A,
   .holes = {{0x83FDCD, 0x83FE16}}},
  // the bytes after its BRA are never run (a disabled test): a hole
  {.name = "func_83FDCD", .entry = 0x83FDCD, .end = 0x83FDF7, .source = "src/misc.c", .needM16 = true,
   .lowWramDb = true, .call = call_func_83FDCD, ALL_OUTPUTS, .fuzz = fuzz_func_83FDCD,
   .holes = {{0x83FDD4, 0x83FDF5}}},
  {.name = "func_82EA49", .entry = 0x82EA49, .end = 0x82EA80, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_func_82EA49, ALL_OUTPUTS, .fuzz = fuzz_func_82EA49},
  {.name = "func_82AB3F", .entry = 0x82AB3F, .end = 0x82AB9D, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x7E, .fixedD = true, .call = call_func_82AB3F, ALL_OUTPUTS, .fuzz = fuzz_func_82AB3F},
  {.name = "func_82A9B7", .entry = 0x82A9B7, .end = 0x82AA7B, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_func_82A9B7, ALL_OUTPUTS, .fuzz = fuzz_func_82A9B7},
  {.name = "func_82AC52", .entry = 0x82AC52, .end = 0x82AD0E, .source = "src/misc.c", .needM16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_func_82AC52, ALL_OUTPUTS, .fuzz = fuzz_func_82AC52},
  {.name = "decompress_to_vram", .entry = 0x808381, .end = 0x8083AD, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .call = call_decompress_to_vram, ALL_OUTPUTS, .fuzz = fuzz_decompress_to_vram},
  {.name = "decompress_to_vram_long", .entry = 0x80837D, .end = 0x808381, .rtl = true, .source = "src/video_dma.c",
   .needM16 = true, .needX16 = true, .call = call_decompress_to_vram, ALL_OUTPUTS, .fuzz = fuzz_decompress_to_vram},
  {.name = "clear_vram", .entry = 0x8A8DEC, .end = 0x8A8E19, .rtl = true, .source = "src/video_dma.c",
   .call = call_clear_vram, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "func_80F3C4", .entry = 0x80F3C4, .end = 0x80F3F4, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_func_80F3C4, ALL_OUTPUTS, .fuzz = fuzz_upload_half},
  {.name = "func_80F3F4", .entry = 0x80F3F4, .end = 0x80F424, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_func_80F3F4, ALL_OUTPUTS, .fuzz = fuzz_upload_half},
  // 16 entries, 12 bytes apart, sharing the tail at $80:F56D: the entries after each one are a hole
  {.name = "upload_slice_0", .entry = 0x80F4AD, .end = 0x80F595, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_upload_slice_0, ALL_OUTPUTS, .fuzz = fuzz_upload_half, .holes = {{0x80F4B9, 0x80F56D}}},
  {.name = "upload_slice_1", .entry = 0x80F4B9, .end = 0x80F595, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_upload_slice_1, ALL_OUTPUTS, .fuzz = fuzz_upload_half, .holes = {{0x80F4C5, 0x80F56D}}},
  {.name = "upload_slice_2", .entry = 0x80F4C5, .end = 0x80F595, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_upload_slice_2, ALL_OUTPUTS, .fuzz = fuzz_upload_half, .holes = {{0x80F4D1, 0x80F56D}}},
  {.name = "upload_slice_3", .entry = 0x80F4D1, .end = 0x80F595, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_upload_slice_3, ALL_OUTPUTS, .fuzz = fuzz_upload_half, .holes = {{0x80F4DD, 0x80F56D}}},
  {.name = "upload_slice_4", .entry = 0x80F4DD, .end = 0x80F595, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_upload_slice_4, ALL_OUTPUTS, .fuzz = fuzz_upload_half, .holes = {{0x80F4E9, 0x80F56D}}},
  {.name = "upload_slice_5", .entry = 0x80F4E9, .end = 0x80F595, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_upload_slice_5, ALL_OUTPUTS, .fuzz = fuzz_upload_half, .holes = {{0x80F4F5, 0x80F56D}}},
  {.name = "upload_slice_6", .entry = 0x80F4F5, .end = 0x80F595, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_upload_slice_6, ALL_OUTPUTS, .fuzz = fuzz_upload_half, .holes = {{0x80F501, 0x80F56D}}},
  {.name = "upload_slice_7", .entry = 0x80F501, .end = 0x80F595, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_upload_slice_7, ALL_OUTPUTS, .fuzz = fuzz_upload_half, .holes = {{0x80F50D, 0x80F56D}}},
  {.name = "upload_slice_8", .entry = 0x80F50D, .end = 0x80F595, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_upload_slice_8, ALL_OUTPUTS, .fuzz = fuzz_upload_half, .holes = {{0x80F519, 0x80F56D}}},
  {.name = "upload_slice_9", .entry = 0x80F519, .end = 0x80F595, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_upload_slice_9, ALL_OUTPUTS, .fuzz = fuzz_upload_half, .holes = {{0x80F525, 0x80F56D}}},
  {.name = "upload_slice_10", .entry = 0x80F525, .end = 0x80F595, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_upload_slice_10, ALL_OUTPUTS, .fuzz = fuzz_upload_half, .holes = {{0x80F531, 0x80F56D}}},
  {.name = "upload_slice_11", .entry = 0x80F531, .end = 0x80F595, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_upload_slice_11, ALL_OUTPUTS, .fuzz = fuzz_upload_half, .holes = {{0x80F53D, 0x80F56D}}},
  {.name = "upload_slice_12", .entry = 0x80F53D, .end = 0x80F595, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_upload_slice_12, ALL_OUTPUTS, .fuzz = fuzz_upload_half, .holes = {{0x80F549, 0x80F56D}}},
  {.name = "upload_slice_13", .entry = 0x80F549, .end = 0x80F595, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_upload_slice_13, ALL_OUTPUTS, .fuzz = fuzz_upload_half, .holes = {{0x80F555, 0x80F56D}}},
  {.name = "upload_slice_14", .entry = 0x80F555, .end = 0x80F595, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_upload_slice_14, ALL_OUTPUTS, .fuzz = fuzz_upload_half, .holes = {{0x80F561, 0x80F56D}}},
  {.name = "upload_slice_15", .entry = 0x80F561, .end = 0x80F595, .source = "src/video_dma.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_upload_slice_15, ALL_OUTPUTS, .fuzz = fuzz_upload_half},
};
const int routineCount = sizeof routines / sizeof routines[0];

const Routine negativeRoutines[] = {
  {.name = "rng_next_wrong", .entry = 0x8087E9, .end = 0x808807, .source = "src/rng.c",
   .needM16 = true, .call = call_rng_next_wrong, ALL_OUTPUTS, .fuzz = fuzz_rng_next},
  {.name = "upload_vram_now_wrong", .entry = 0x8083AD, .end = 0x8083CB, .rtl = true, .source = "src/video_dma.c",
   .needM16 = true, .needX16 = true, .call = call_upload_vram_now_wrong, ALL_OUTPUTS, .fuzz = fuzz_upload_vram_now},
  {.name = "wait_frames_wrong", .entry = 0x808373, .end = 0x80837D, .source = "src/frame.c", .needM16 = true,
   .lowWramDb = true, .waitsForInterrupts = true, .call = call_wait_frames_wrong, ALL_OUTPUTS},
  {.name = "mode7_zoom_view_wrong", .entry = 0x80F38D, .end = 0x80F3C4, .source = "src/scale.c", .needM16 = true,
   .fixedD = true, .call = call_mode7_zoom_view_wrong, ALL_OUTPUTS, .fuzz = fuzz_mode7_zoom_view},
  {.name = "draw_meter_wrong", .entry = 0x80DA5F, .end = 0x80DAAE, .source = "src/hud.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_draw_meter_wrong, ALL_OUTPUTS, .fuzz = fuzz_draw_meter},
  {.name = "irq_ack_only_wrong", .entry = 0x8595BD, .end = 0x8595C7, .rti = true, .source = "src/frame.c",
   .call = call_irq_ack_only_wrong, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "sound_send_message_wrong", .entry = 0x86DFFB, .end = 0x86E096, .source = "src/sound.c", .lowWramDb = true,
   .call = call_sound_send_message_wrong, ALL_OUTPUTS},
  {.name = "pads_read_wrong", .entry = 0x8386FF, .end = 0x8387C1, .source = "src/pads.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_pads_read_wrong, ALL_OUTPUTS, .fuzz = fuzz_pads},
  {.name = "nmi_prologue_wrong", .entry = 0x808807, .end = 0x808931, .rti = true, .endAt = 0x808931, .source = "src/frame.c",
   .call = call_nmi_prologue_wrong, .fuzz = fuzz_nmi_prologue},
  {.name = "race_hud_draw_wrong", .entry = 0x858814, .end = 0x858AAE, .rtl = true, .source = "src/race_frame.c",
   .needM16 = true, .needX16 = true, .call = call_race_hud_draw_wrong, .fuzz = fuzz_race_hud_draw},
  {.name = "irq_race_band_wrong", .entry = 0x8587BA, .end = 0x858812, .rti = true, .source = "src/race_frame.c",
   .call = call_irq_race_band_wrong, ALL_OUTPUTS},
  {.name = "nmi_race_full_wrong", .entry = 0x8583CE, .end = 0x8587BA, .rti = true, .rtiFrame = 9, .source = "src/race_frame.c",
   .needM16 = true, .needX16 = true, .call = call_nmi_race_full_wrong, ALL_OUTPUTS, .alsoIn = {{0x85838A, 0x8583CE}},
   .fuzz = fuzz_nmi_race_full},
  {.name = "video_load_screen_wrong", .entry = 0x8081CC, .end = 0x808373, .source = "src/video_dma.c",
   .call = call_video_load_screen_wrong, ALL_OUTPUTS, .fuzz = fuzz_video_load_screen},
  {.name = "car_road_step_wrong", .entry = 0x81F887, .end = 0x81FA8F, .source = "src/track.c", .needM16 = true,
   .needX16 = true, .db = 0x7E, .fixedD = true, .call = call_car_road_step_wrong, .fuzz = fuzz_car_road_step},
  {.name = "player_road_limits_wrong", .entry = 0x83E8AB, .end = 0x83EA9B, .source = "src/track.c", .needM16 = true,
   .needX16 = true, .call = call_player_road_limits_wrong, .fuzz = fuzz_player_road_limits},
  {.name = "nmi_race_tail_wrong", .entry = 0x85838A, .end = 0x8583CE, .rti = true, .rtiFrame = 9, .source = "src/frame.c",
   .needM16 = true, .needX16 = true, .call = call_nmi_race_tail_wrong, ALL_OUTPUTS, .fuzz = fuzz_nmi_race_tail},
  {.name = "nmi_tail_wrong", .entry = 0x808934, .end = 0x80894E, .rti = true, .rtiFrame = 9, .source = "src/frame.c",
   .needM16 = true, .needX16 = true, .call = call_nmi_tail_wrong, ALL_OUTPUTS},
  {.name = "irq_split_bottom_wrong", .entry = 0x80E48A, .end = 0x80E4C5, .rti = true, .source = "src/frame.c",
   .call = call_irq_split_bottom_wrong, ALL_OUTPUTS},
  {.name = "irq_split_top_wrong", .entry = 0x80E3F7, .end = 0x80E48A, .rti = true, .source = "src/frame.c",
   .call = call_irq_split_top_wrong, ALL_OUTPUTS},
};
const int negativeRoutineCount = sizeof negativeRoutines / sizeof negativeRoutines[0];
