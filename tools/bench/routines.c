// SPDX-License-Identifier: GPL-3.0-or-later
// Registry of ported routines for the differential test: the adapters map the original's
// registers to the C function's arguments. This mapping lives here, never in src/.
//
// Flag rules used by the adapters: N/Z follow the value the original's last flag-setting
// instruction produced, so they are derived from the C's returned value; flags the original
// does not touch are left as they were on entry (the adapter starts from a copy of `in`).
#include "routines.h"

#include <stddef.h>
#include <string.h>

#include "rng.h"
#include "scroll_tables.h"
#include "sound.h"
#include "boot.h"
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
#include "password.h"
#include "vs.h"
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

// ---- asl_n ($8A:B152) ----  Y kept (its PLY: N/Z of Y), C the last bit out (or set by the
// CPY #0 with no shift), V kept
static void call_asl_n(const Regs* in, Regs* out) {
  bool carry;
  out->a = asl_n(in->a, in->y, &carry);
  out->y = in->y;
  out->c = carry;
  setNZ16(out, in->y);
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
static void call_text_type_step(const Regs* in, Regs* out) { (void)in; (void)out; text_type_step(in->d); }
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
  race_lights_draw(in->d);
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
// (C and V from the string D+$3C and palette D+$18, before the call: textCentredLineCV)
static void textCentredLineCVFont(Regs* out, uint16_t string, uint16_t palette, bool vs) {
  const uint16_t start = rom16(0x83A006 + (uint32_t)string);
  uint16_t free = 13;
  for(uint16_t s = start; bus_rd8(0x830000 | s); s++) if(!(bus_rd8(0x830000 | s) & 0x80)) free--;
  setAdc16(out, (uint16_t)(free & 0xFFFE), palette);
  for(uint16_t s = start; bus_rd8(0x830000 | s); s++) {
    uint8_t c = bus_rd8(0x830000 | s);
    out->c = false;
    if(!(c & 0x80) && c != 0x20 && c != 0x21 && (vs || c != 0x2E) && c != 0x2B) out->v = false;  // a small SBC
  }
}
static void textCentredLineCV(Regs* out, uint16_t string, uint16_t palette) { textCentredLineCVFont(out, string, palette, false); }
// vs_centred_line ($82:A134): as text_centred_line, in the VS font (vs-race-11)
static void call_vs_centred_line(const Regs* in, Regs* out) {
  (void)in;
  textCentredLineCVFont(out, rd16(DP + 0x3C), rd16(DP + 0x18), true);
  vs_centred_line();
  out->n = false;
  out->z = true;
}
static void call_text_centred_line(const Regs* in, Regs* out) {
  (void)in;
  textCentredLineCV(out, rd16(DP + 0x3C), rd16(DP + 0x18));
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
  out->x = shop_list_label(in->d, in->x);
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

// ---- race_message_update ($82:B11F) ----  no register outputs; its final PLB of $83 sets
// N and clears Z; DB = $83
static void call_race_message_update(const Regs* in, Regs* out) {
  (void)in;
  race_message_update();
  out->db = 0x83;
  out->n = true;
  out->z = false;
}
static void call_vs_message_update(const Regs* in, Regs* out) {  // (vs-race-12) as race_message_update
  (void)in;
  vs_message_update();
  out->db = 0x83;
  out->n = true;
  out->z = false;
}
// the shown and new messages (the race's: 0-$4E, by D+$3C's table), their times, the step
static void fuzz_race_message_update(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint16_t shown = (uint16_t)(rnd() % 0x4F), next = (rnd() & 1) ? shown : (uint16_t)(rnd() % 0x4F);
  wr16ram(ram, DP + 0x14, shown);
  wr16ram(ram, DP + 0x16, next);
  wr16ram(ram, DP + 0x1A, (rnd() & 3) == 0 ? 0 : (uint16_t)(rnd() % 0x40));
  wr16ram(ram, 0x1E9E, (uint16_t)(rnd() % 8));
  wr16ram(ram, DP + 0x18, (uint16_t)(0x2000 + (rnd() % 0x800) * 2));
  wr16ram(ram, DP + 0x3A, (uint16_t)((rnd() % 8) << 10));
}

// ---- func_82B769 ($82:B769) ----  as func_82B7B0 leaves the registers
static void call_func_82B7B0(const Regs* in, Regs* out);
static void call_func_82B769(const Regs* in, Regs* out) {
  uint16_t a = in->a;
  if((rom16(0x82DC68 + (uint32_t)rd16(0x047B)) & 0x8000) && in->y != 0) a = (in->y & 0x8000) ? 0 : (uint16_t)(rd16(0x019A) - 1);
  func_82B78D(a);
  call_func_82B7B0(in, out);  // the last call's outputs are the routine's
}
static void fuzz_func_82B7B0(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
static void fuzz_func_82B769(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_func_82B7B0(r, ram, rnd);
  r->a = (uint16_t)((rnd() % 0x40) << 4);
  r->y = (rnd() & 1) ? 0 : (uint16_t)rnd();
  wr16ram(ram, 0x019A, (uint16_t)rnd());
  // $047B (a word index into $82:DC68): half the time an entry whose word is negative
  static const uint16_t negative[] = {17, 27, 47};
  wr16ram(ram, 0x047B, (rnd() & 1) ? negative[rnd() % 3] : (uint16_t)(rnd() % 48));
}

// ---- empty_80B94F: a lone RTS, nothing changes
static void call_empty(const Regs* in, Regs* out) { (void)in; (void)out; }

// ---- sprite_number_point ($80:855A) ----  A = 0 (the fifth digit read: the value is all
// shifted out), X = 0, Y = the x after the number (N Z of that LDY), C clear (take_nibble16)
static void call_sprite_number_point(const Regs* in, Regs* out) {
  out->y = sprite_number_point(in->d, in->a, in->y);
  out->a = 0;
  out->x = 0;
  setNZ16(out, out->y);
  out->c = false;
}
static void fuzz_sprite_number_point(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->a = (uint16_t)((rnd() & 3) == 0 ? (rnd() % 0x1000) : rnd());  // a leading zero, often
  r->y = (uint16_t)(rnd() & 0x01FF);
  wr16ram(ram, DP + 0x14, (uint16_t)rnd());
  wr16ram(ram, DP + 0x74, (uint16_t)((rnd() % 0x60) * 4));
}

// ---- sprite_bar8 ($80:C475) ----  A = X = slot + $20 (N Z C V of that ADC)
static void call_sprite_bar8(const Regs* in, Regs* out) {
  out->x = sprite_bar8(in->d, in->x);
  out->a = out->x;
  setNZ16(out, out->x);
  setAdc16(out, in->x, 0x0020);
}
static void fuzz_sprite_bar8(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)((rnd() % 0x60) * 4);
  for(uint32_t a = 0x14; a <= 0x1A; a += 2) wr16ram(ram, DP + a, (uint16_t)rnd());
}

// ---- clc_82DA46, clc_83FA32: CLC, RTS
static void call_clc(const Regs* in, Regs* out) { (void)in; out->c = false; }

// ---- mode7_zoom_step ($80:F226) ----  as func_80F745 leaves A, X and N Z; C clear (CLC)
static void call_func_80F745(const Regs* in, Regs* out);
static void call_mode7_zoom_step(const Regs* in, Regs* out) {
  mode7_zoom_view(in->d);
  wr16((uint16_t)(in->d + 0x40), (uint16_t)(rd16((uint16_t)(in->d + 0x40)) + 1));
  if(rd16((uint16_t)(in->d + 0x40)) >= 0x0320) {
    uint16_t v = (uint16_t)(rd16((uint16_t)(in->d + 0x7C)) - 0x0010);
    wr16((uint16_t)(in->d + 0x7C), (v & 0x8000) ? 0 : v);
  }
  inverse_scale(in->d);
  call_func_80F745(in, out);
  out->c = false;
}
// the frame count just below or past $320, D+$7C near 0; the rest as the zoom's and the
// line tables' fuzz
static void fuzz_mode7_zoom_view(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
static void fuzz_func_80F745(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
static void fuzz_mode7_zoom_step(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_func_80F745(r, ram, rnd);  // the line tables' buffers, as the title screen keeps them
  fuzz_mode7_zoom_view(r, ram, rnd);
  wr16ram(ram, (uint16_t)(r->d + 0x40), (uint16_t)(0x0300 + rnd() % 0x40));
  wr16ram(ram, (uint16_t)(r->d + 0x7C), (uint16_t)(rnd() % 0x40));
}

static bool v_after_inverse_scale(bool before);
static bool v_after_func_80F745(bool before);
// ---- the title zoom's steps ($80:F2A0, $80:F2EE, $80:F595) ----  as func_80F3C4: what
// func_80F745 leaves (V by v_after_func_80F745), C clear; title_zoom_pan's early stop: A = $1C00 (its last AND/CMP),
// X kept, N Z of the INC of D+$3E
static void call_title_zoom_pan(const Regs* in, Regs* out) {
  bool done;
  uint16_t r = title_zoom_pan(&done);
  if(done) {
    out->a = 0x1C00;
    setNZ16(out, rd16(DP + 0x3E));
  } else {
    out->a = r;
    out->x = rd16(DP + 0x34);
    setNZ16(out, out->x);
    out->v = v_after_func_80F745(v_after_inverse_scale(in->v));
  }
  out->c = false;
}
static void call_title_zoom_cycle(const Regs* in, Regs* out) {
  out->a = title_zoom_cycle();
  out->x = rd16(DP + 0x34);
  setNZ16(out, out->x);
  out->v = v_after_func_80F745(v_after_inverse_scale(in->v));
  out->c = false;
}
static void call_title_zoom_drop(const Regs* in, Regs* out) {
  out->a = title_zoom_drop();
  out->x = rd16(DP + 0x34);
  setNZ16(out, out->x);
  out->v = v_after_func_80F745(v_after_inverse_scale(in->v));
  out->c = false;
}
// the line tables' buffers, the counters near their turns, the pan near its stop
static void fuzz_title_zoom(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_func_80F745(r, ram, rnd);
  wr16ram(ram, DP + 0x30, (uint16_t)(0x000C + rnd() % 0x18));
  uint32_t k = rnd() % 4;
  wr16ram(ram, DP + 0x3C, k == 0 ? (uint16_t)(0xC000 | (rnd() & 0xFF)) : (uint16_t)rnd());
  wr16ram(ram, DP + 0x3A, k == 0 && (rnd() & 1) ? (uint16_t)(0x1C00 | (rnd() & 0xFF)) : (uint16_t)rnd());
  wr16ram(ram, DP + 0x40, (rnd() & 1) ? 0x0800 : 0xF800);
  wr16ram(ram, DP + 0x7E, (uint16_t)(0xFFB0 + rnd() % 0x20));
  wr16ram(ram, DP + 0x3E, (uint16_t)rnd());
}

// ---- far_7fff_4 ($81:E7CE) ----  A = $7FFF (N Z of its LDA)
static void call_far_7fff_4(const Regs* in, Regs* out) {
  (void)in;
  far_7fff_4();
  out->a = 0x7FFF;
  setNZ16(out, 0x7FFF);
}

// ---- window_shape_a/b ($80:DB7D, $80:DBCB) ----  the mirror's last copy: A = the word, X
// its target, Y its source; N set and Z clear by the final PLB ($81); C = D+$1E's bit 0
// (the LSR), V of the last line's 8-bit SBC (right - width)
static void windowShapeOut(const Regs* in, Regs* out, uint32_t widths, void (*fn)(uint16_t, uint16_t, uint16_t)) {
  // where the lines start (func_80DC5A, run once more here: the C writes the same bytes)
  uint16_t start = (uint16_t)((uint16_t)func_80DC5A(0x7F0000 | in->x, in->a) + 0x0033);
  uint16_t e = rd16(DP + 0x1E), row0 = rom16(0x81C852 + (uint32_t)(e & 0xFFFE)), lines = (uint16_t)((e >> 1) + 0x0C);
  uint8_t w = rom8(widths + (uint16_t)(row0 + lines - 1));
  uint8_t right = (uint8_t)((w >> 1) + rd8(DP + 0x2E) - rd8(DP + 0x20));
  uint8_t d = (uint8_t)(right - w);
  fn(in->d, in->a, in->x);
  // $19 copies, or the odd one first and $18 more: the target ends $32 below either way
  uint16_t to = (uint16_t)(start - 0x32), from = (e & 1) ? (uint16_t)(start + 0x30) : (uint16_t)(start + 0x32);
  out->a = rd16(0x10000 + (uint32_t)to);
  out->x = to;
  out->y = from;
  out->n = true;
  out->z = false;
  out->c = (e & 1) != 0;
  out->v = ((right ^ w) & (right ^ d) & 0x80) != 0;
  out->db = 0x81;
}
static void call_window_shape_a(const Regs* in, Regs* out) { windowShapeOut(in, out, 0x81C872, window_shape_a); }
static void call_window_shape_b(const Regs* in, Regs* out) { windowShapeOut(in, out, 0x81C9AA, window_shape_b); }
static void fuzz_func_80DC5A(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
// the head's A as func_80DC5A's fuzz draws it, the table high in bank $7F, rows 0-31, the
// edges anywhere
static void fuzz_window_shape(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_func_80DC5A(r, ram, rnd);
  r->x = (uint16_t)(0x4000 + (rnd() % 0x2000));
  wr16ram(ram, DP + 0x1E, (uint16_t)(rnd() % 0x20));
  wr16ram(ram, DP + 0x2E, (uint16_t)rnd());
  wr16ram(ram, DP + 0x20, (uint16_t)rnd());
}

// ---- sprite_number_point_right ($80:8667) ----  A = D+$14's high byte : the last moved x
// (8-bit ADC: V), X = 0, Y = the slot after (CPY: Z and C set, N clear)
static void call_sprite_number_point_right(const Regs* in, Regs* out) {
  out->y = sprite_number_point_right(in->d, in->a, in->y);
  uint16_t shift = rd16(DP + 0x14);
  uint8_t moved = rd8(0x0900 + (uint32_t)(uint16_t)(out->y - 4)), before = (uint8_t)(moved - (uint8_t)shift);
  out->a = (uint16_t)((shift & 0xFF00) | moved);
  out->x = 0;
  out->n = false;
  out->z = true;
  out->c = true;
  out->v = (~(before ^ (uint8_t)shift) & (before ^ moved) & 0x80) != 0;
}

// ---- car_message_request ($83:FEBD) ----  A kept (its PLA: N Z), X = 2 * message, C of
// the priorities' CMP (current >= new)
static void call_car_message_request(const Regs* in, Regs* out) {
  uint16_t current = rom16(0x83A0A4 + (uint32_t)(uint16_t)(rd16((uint16_t)(in->d + 0x33)) << 1));
  uint16_t wanted = rom16(0x83A0A4 + (uint32_t)(uint16_t)(in->a << 1));
  car_message_request(in->d, in->a);
  out->a = in->a;
  out->x = (uint16_t)(in->a << 1);
  setNZ16(out, in->a);
  out->c = current >= wanted;
}
static void fuzz_car_message_request(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->a = (uint16_t)(rnd() % 0x4F);
  wr16ram(ram, (uint16_t)(r->d + 0x33), (uint16_t)(rnd() % 0x4F));
}

// ---- race_message_request_0233 / _02e6 ($82:9BC9, $82:9BE5) ----  as car_message_request
static void raceMessageOut(const Regs* in, Regs* out, uint32_t slot, void (*fn)(uint16_t)) {
  uint16_t current = rom16(0x83A0A4 + (uint32_t)(uint16_t)(rd16(slot) << 1));
  uint16_t wanted = rom16(0x83A0A4 + (uint32_t)(uint16_t)(in->a << 1));
  fn(in->a);
  out->a = in->a;
  out->x = (uint16_t)(in->a << 1);
  setNZ16(out, in->a);
  out->c = current >= wanted;
}
static void call_race_message_request_0233(const Regs* in, Regs* out) { raceMessageOut(in, out, 0x0233, race_message_request_0233); }
static void call_race_message_request_02e6(const Regs* in, Regs* out) { raceMessageOut(in, out, 0x02E6, race_message_request_02e6); }
static void call_race_message_request_0295(const Regs* in, Regs* out) { raceMessageOut(in, out, 0x0295, race_message_request_0295); }
static void call_race_message_request_0348(const Regs* in, Regs* out) { raceMessageOut(in, out, 0x0348, race_message_request_0348); }
static void fuzz_race_message_request(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->a = (uint16_t)(rnd() % 0x4F);
  wr16ram(ram, 0x0233, (uint16_t)(rnd() % 0x4F));
  wr16ram(ram, 0x02E6, (uint16_t)(rnd() % 0x4F));
}
// (the third and fourth cars' slots, vs-race-3)
static void fuzz_race_message_request_34(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->a = (uint16_t)(rnd() % 0x4F);
  wr16ram(ram, 0x0295, (uint16_t)(rnd() % 0x4F));
  wr16ram(ram, 0x0348, (uint16_t)(rnd() % 0x4F));
}

// ---- track_entry_bounds ($81:E7DA) ----  A = D+$26 (N Z C V of its ADC), X = 4 * the pair
static void call_track_entry_bounds(const Regs* in, Regs* out) {
  (void)in;
  const uint32_t e = rd16(DP + 0x14);
  uint16_t idx = (uint16_t)(rd8(0x8CB4 + e) << 2);
  track_entry_bounds();
  out->a = rd16(DP + 0x26);
  out->x = idx;
  setNZ16(out, out->a);
  setAdc16(out, rom16(0x8380D2 + (uint32_t)idx), rd16(DP + 0x28));
}
static void fuzz_track_entry_bounds(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint16_t e = (uint16_t)(rnd() % 0x200);
  wr16ram(ram, DP + 0x14, e);
  ram[0x8CB3 + e] = (uint8_t)rnd();
  ram[0x8CB4 + e] = (uint8_t)(rnd() % 0x40);
}

// ---- spark_step ($80:EA77) ----  no register outputs (flags by liveness)
static void call_spark_step(const Regs* in, Regs* out) { (void)out; spark_step(in->d); }
// the sparks' lives and places, either buffer, the count around the laying window
static void fuzz_spark_step(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint16_t buffer = (rnd() & 1) ? 0x8000 : 0x0000;
  wr16ram(ram, DP + 0x5E, buffer);
  wr16ram(ram, DP + 0x3E, (uint16_t)((rnd() % 0x20) * 4));
  for(uint32_t i = 0; i < 0x80; i += 2) {
    uint32_t at = 0x11000 + buffer + i;
    ram[at] = (uint8_t)(i % 4 == 0 ? rnd() % 0x40 : rnd());
    ram[at + 1] = (uint8_t)(i % 4 == 0 ? 0 : rnd());
  }
  wr16ram(ram, DP + 0x86, (uint16_t)(0x0060 + rnd() % 0x28));
  wr16ram(ram, DP + 0xE7, (uint16_t)rnd());
  wr16ram(ram, DP + 0x74, (uint16_t)((rnd() % 0x40) * 4));
}

// ---- car_clear_53 ($83:FD3C) ----  A's low byte = 0 (its 8-bit LDA: N clear, Z set)
static void call_car_clear_53(const Regs* in, Regs* out) {
  car_clear_53(in->d);
  out->a = (uint16_t)(in->a & 0xFF00);
  out->n = false;
  out->z = true;
}

// ---- block_copy_7f2000 ($80:E8B5) ----  A = the length word's high byte : the last byte,
// X and Y past the block; N clear and Z set by the last DEC
static void call_block_copy_7f2000(const Regs* in, Regs* out) {
  uint16_t len = bus_rd16((0x81D419 + (uint32_t)in->y) & 0xFFFFFF);
  out->x = in->x;
  out->y = in->y;
  block_copy_7f2000(in->d, &out->x, &out->y);
  out->a = (uint16_t)((len & 0xFF00) | bus_rd8((0x81D419 + (uint32_t)(uint16_t)(out->y - 1)) & 0xFFFFFF));
  out->n = false;
  out->z = true;
}
// any block of the table's live start offsets (as the movie's, X where the copy stays in $7F)
static void fuzz_block_copy_7f2000(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)ram;
  r->x = (uint16_t)(rnd() % 0x4000);
  r->y = (uint16_t)(rnd() % 0x2800);  // within the ROM bank (the table and what follows)
}

// ---- text_line_push ($8A:99E2) ----  X kept (its PLX: N Z); C of the last ASL that made
// the line's offset (bit 13 of X: byte_buffer_push leaves C alone)
static void call_text_line_push(const Regs* in, Regs* out) {
  text_line_push(in->x);
  out->x = in->x;
  setNZ16(out, in->x);
  out->c = (in->x >> 13) & 1;
}
static void fuzz_text_line_push(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)((rnd() % 0x40) * 2);
  uint32_t at = 0x17918 + (uint32_t)(uint16_t)((r->x >> 1) << 4);
  uint32_t n = rnd() % 15;
  for(uint32_t i = 0; i < n; i++) ram[at + i] = (uint8_t)(1 + rnd() % 255);
  ram[at + n] = 0;
  wr16ram(ram, DP + 0x60, (uint16_t)(rnd() % 0x20));
  // the text buffer's index (byte_buffer_push: STA $7F7D00,X) as the game keeps it ($0003 to
  // $020B at every live entry, the 60k and shop-buy movies): from $8300 up the long store
  // runs into bank $80, low WRAM and the stack (a paused race leaves such a word there)
  wr16ram(ram, DP + 0x58, (uint16_t)(rnd() % 0x210));
}

// ---- track_object_test / _bump / _money ($83:FA34, $83:FA6A, $83:FAD7) ----  called
// through $83:F7F9's handler table, whose caller reloads A, X and Y and sets every flag
// before reading them: no register outputs. Too few returns are watched to see that, so
// the exit flags are modelled: mostly the last callee's, through its adapter (calleeRun).
// When set, the adapters of func_83FE9A, func_83FC05, func_83FD8A and func_83FDCD run this
// instead of their routine: a caller's adapter whose exit flags are the callee's sets it
// to its whole routine (which makes that call once), so the callee's flag model sees the
// state at the call. Returns what the callee's call returns.
static bool (*calleeRun)(uint16_t dp) = NULL;

static void call_func_83FE9A(const Regs* in, Regs* out);
static void call_func_83FC05(const Regs* in, Regs* out);
static void call_func_83FD8A(const Regs* in, Regs* out);
static void call_func_83FDCD(const Regs* in, Regs* out);
static uint16_t objectX(void) {
  uint16_t v = (uint16_t)(rd8(0x8CB3 + (uint32_t)rd16(0x011A)) << 4);
  return (v & 0x0800) ? (uint16_t)(v | 0xF000) : v;
}
// the test: the callee's adapter reads $012A, which the routine clears first; it is set
// for that read and restored before the routine runs, so the C makes its own writes
static uint16_t heldPos, heldAt;
static bool run_track_object_test(uint16_t dp) {
  wr16(0x012A, heldPos);
  wr16(0x0128, heldAt);
  return track_object_test(dp);
}
static void call_track_object_test(const Regs* in, Regs* out) {
  uint16_t v = objectX();
  heldPos = rd16(0x012A);
  heldAt = rd16(0x0128);
  wr16(0x012A, 0);
  wr16(0x0128, v);
  Regs t = *in;
  t.a = v;  // both calls are made with A = $0128
  calleeRun = run_track_object_test;
  if(rd8(0x8CB5 + (uint32_t)rd16(0x011A)) & 0x30) call_func_83FDCD(&t, out);
  else call_func_83FD8A(&t, out);
  calleeRun = NULL;
}
// the bump: C = 0 (its CLC); otherwise the flags of the compare that missed, of LDA D+$1A,
// or of func_83FE9A; V of the SBC D+$06 unless func_83FE9A ran
static bool run_track_object_bump(uint16_t dp) { track_object_bump(dp); return false; }
static void call_track_object_bump(const Regs* in, Regs* out) {
  uint16_t v = objectX(), base = rd16((uint16_t)(in->d + 0x06)), kind = rd16(0x011C);
  uint16_t d = (uint16_t)(v - base);
  uint16_t c = (uint16_t)(d - bus_rd16(((d & 0x8000) ? 0x8380CC : 0x8380CE) + (uint32_t)kind));
  bool within = (d & 0x8000) ? !(c & 0x8000) : (c & 0x8000) != 0;
  uint16_t side = rd16((uint16_t)(in->d + 0x1A));
  Regs s;
  setSbc16(&s, v, base);
  if(within && side == 0) {
    Regs t = *in;
    t.a = 0xFF1D;
    t.x = rd16(0x1EBE);
    t.y = 0;
    calleeRun = run_track_object_bump;
    call_func_83FE9A(&t, out);
    calleeRun = NULL;
  } else {
    track_object_bump(in->d);
    setNZ16(out, within ? side : c);
    out->v = s.v;
  }
  out->c = false;
}
// the money: a miss leaves func_83FC05's flags; a hit N and Z clear (LDA #$47), C and V of
// func_83FE9A
static bool run_track_object_money(uint16_t dp) { return track_object_money(dp); }
static void call_track_object_money(const Regs* in, Regs* out) {
  uint16_t at = rd16(0x0128);
  bool hit = func_83FC05(in->d);  // which way it goes (it only writes $0128, put back)
  wr16(0x0128, at);
  calleeRun = run_track_object_money;
  if(!hit) call_func_83FC05(in, out);
  else {
    Regs t = *in;
    t.a = 0xFF19;
    t.x = rd16(0x1EBE);
    t.y = 0;
    call_func_83FE9A(&t, out);
    out->n = false;
    out->z = false;
  }
  calleeRun = NULL;
}
static void fuzz_func_83FC05(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
static void fuzz_func_83FD8A(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
static void fuzz_func_83FE9A(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
static uint16_t bcdRandom(uint32_t (*rnd)(void)) {
  uint16_t v = 0;
  for(int shift = 0; shift < 16; shift += 4) v |= (uint16_t)((rnd() % 10) << shift);
  return v;
}
// the callees' inputs (the hit tests near their limits, the effect request), then the
// object entry; $012A is 0 on the test, so $012C near 0 half the time; money in BCD,
// near a carry into the high word half the time
static void fuzz_track_object(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_func_83FD8A(r, ram, rnd);
  fuzz_func_83FE9A(r, ram, rnd);
  fuzz_func_83FC05(r, ram, rnd);
  if(rnd() & 1) wr16ram(ram, 0x012C, (uint16_t)(int16_t)(rnd() % 97 - 48));
  if(rnd() & 1) wr16ram(ram, (uint16_t)(r->d + 0x1A), 0);
  uint16_t car = (uint16_t)(2 * (rnd() % 8));
  wr16ram(ram, 0x1ECC, car);
  wr16ram(ram, (uint16_t)(0x038E + car), (rnd() & 1) ? (uint16_t)((bcdRandom(rnd) & 0x0FFF) | ((6 + rnd() % 4) << 12)) : bcdRandom(rnd));
  wr16ram(ram, (uint16_t)(0x0390 + car), bcdRandom(rnd));
}

// ---- car_set_53 ($83:FD26, $83:FD31) ----  A = the new dp+$B1 (N Z of its ORA), X = $011A;
// C and V as on entry
static void carSet53Out(const Regs* in, Regs* out, uint8_t value) {
  car_set_53(in->d, value);
  out->a = rd16((uint16_t)(in->d + 0xB1));
  out->x = rd16(0x011A);
  setNZ16(out, out->a);
}
static void call_car_set_53_1(const Regs* in, Regs* out) { carSet53Out(in, out, 1); }
static void call_car_set_53_2(const Regs* in, Regs* out) { carSet53Out(in, out, 2); }
// the two's shared tail ($83:FD45, measure-2): D+$53 already set (rewritten unchanged)
static void call_car_set_53_tail(const Regs* in, Regs* out) { carSet53Out(in, out, rd8((uint16_t)(in->d + 0x53))); }
static void fuzz_car_set_53(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint16_t x = (uint16_t)(rnd() % 0x200);
  wr16ram(ram, 0x011A, x);
  for(int i = 0; i < 3; i++) ram[0x8CB3 + x + i] = (uint8_t)rnd();
}

// ---- variant_lines / _5a / _66 ($8A:9AB1, $8A:98F0, $8A:99BE) ----  A = the value stored
// (N and Z clear, from its LDA #), C of the CMP #2 of the variant, V as on entry
static void variantOut(const Regs* in, Regs* out, void (*fn)(void), uint16_t at) {
  (void)in;
  bool c = (rd8(0x03DC) & 0x0F) >= 2;
  fn();
  out->a = rd16(DP + at);
  out->n = out->z = false;
  out->c = c;
}
static void call_variant_lines(const Regs* in, Regs* out) { variantOut(in, out, variant_lines, 0x56); }
static void call_variant_5a(const Regs* in, Regs* out) { variantOut(in, out, variant_5a, 0x5A); }
static void call_variant_66(const Regs* in, Regs* out) { variantOut(in, out, variant_66, 0x66); }
// the variant, 2 a third of the time, other high bits random
static void fuzz_variant(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  ram[0x03DC] = (uint8_t)((rnd() & 0xF0) | ((rnd() % 3 == 0) ? 2 : rnd() % 16));
}

// ---- text_lines_copy ($8A:9AC9) ----  with lines: A = the new D+$54 (C V of its ADC),
// X = the last line's start + its length, Y = past the last zero, N clear and Z set by
// the last DEC; none: A = 0 (N clear, Z set by LDA), X Y C V as on entry
static void call_text_lines_copy(const Regs* in, Regs* out) {
  uint16_t n = rd16(DP + 0x56), y = 0, x = 0, last = 0;
  for(uint16_t k = 0, p = rd16(DP + 0x54); k < n; k++, p = (uint16_t)(p + 0x10)) {
    x = p;
    while(rom8(0x8A9DEE + (uint32_t)y) != 0) { x++; y++; }
    y++;
    last = p;
  }
  text_lines_copy();
  if(n == 0) {
    out->a = 0;
    out->n = false;
    out->z = true;
    return;
  }
  out->a = rd16(DP + 0x54);
  out->x = x;
  out->y = y;
  setAdc16(out, last, 0x0010);
  out->n = false;
  out->z = true;
}
static void fuzz_text_lines_copy(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x56, (uint16_t)(rnd() % 0x18));
  wr16ram(ram, DP + 0x54, (uint16_t)(rnd() % 0x400));
}

// ---- track_object_count ($83:FB15) ----  as track_object_money; a full count leaves
// func_83FC05's V with N clear, Z and C set (its CMP #$0A)
static bool run_track_object_count(uint16_t dp) { return track_object_count(dp); }
static void call_track_object_count(const Regs* in, Regs* out) {
  uint16_t at = rd16(0x0128);
  bool hit = func_83FC05(in->d);  // which way it goes (it only writes $0128, put back)
  wr16(0x0128, at);
  calleeRun = run_track_object_count;
  if(!hit) call_func_83FC05(in, out);
  else if(rd8((uint16_t)(in->d + 0x4E)) == 0x0A) {
    call_func_83FC05(in, out);
    out->n = false;
    out->z = out->c = true;
  } else {
    Regs t = *in;
    t.a = 0xFF19;
    t.x = rd16(0x1EBE);
    t.y = 0;
    call_func_83FE9A(&t, out);
    out->n = out->z = false;
  }
  calleeRun = NULL;
}
static void fuzz_track_object_count(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_track_object(r, ram, rnd);
  wr16ram(ram, (uint16_t)(r->d + 0x4E), (uint16_t)((rnd() & 0xFF00) | ((rnd() & 1) ? 0x0A : rnd() % 12)));
}

// ---- race_section_seek_1 / _2 ($85:9D39, $85:9D5A) ----  A = the last difference, X =
// D+$9C + the offset, C clear (both exits are BCC), N Z of the SBC or of the CMP #$40,
// V of the SBC
static void sectionSeekOut(Regs* out, uint16_t position, uint16_t offset, void (*fn)(void)) {
  fn();
  uint16_t x = (uint16_t)(rd16(DP + 0x9C) + rd16(DP + offset));
  uint16_t pos = rd16(position), entry = bus_rd16(0x840000 + (uint32_t)x);
  out->x = x;
  setSbc16(out, pos, entry);
  out->a = (uint16_t)(pos - entry);
  setNZ16(out, pos < entry ? out->a : (uint16_t)(out->a - 0x0040));
  out->c = false;
}
static void call_race_section_seek_1(const Regs* in, Regs* out) { (void)in; sectionSeekOut(out, 0x023E, 0xA2, race_section_seek_1); }
static void call_race_section_seek_2(const Regs* in, Regs* out) { (void)in; sectionSeekOut(out, 0x02F1, 0xA6, race_section_seek_2); }
// near the live state: the offsets a few entries back; the positions within $200 of
// theirs, or (half the time, when the table is in ROM) just past the current entry, so
// the offset moves on at least once
static void fuzz_race_section_seek(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  static const uint16_t positions[2] = {0x023E, 0x02F1}, offsets[2] = {DP + 0xA2, DP + 0xA6};
  uint16_t base = (uint16_t)(ram[DP + 0x9C] | ram[DP + 0x9D] << 8);
  for(int k = 0; k < 2; k++) {
    uint16_t o = (uint16_t)(ram[offsets[k]] | ram[offsets[k] + 1] << 8), back = (uint16_t)(0x14 * (rnd() % 4));
    if(o >= back) o = (uint16_t)(o - back);
    wr16ram(ram, offsets[k], o);
    uint16_t x = (uint16_t)(base + o), p = (uint16_t)(ram[positions[k]] | ram[positions[k] + 1] << 8);
    if((rnd() & 1) && x >= 0x8000 && x <= 0xFFFE) p = (uint16_t)(rom16(0x840000 + (uint32_t)x) + 0x40 + rnd() % 0x80);
    else p = (uint16_t)(p + rnd() % 0x400 - 0x200);
    wr16ram(ram, positions[k], p);
  }
}

// ---- row_copy_6a33 / _6b33 ($85:82D8, $85:82FA) ----  A = the last word, X kept (N Z of
// its PLX), Y = $20, C set (CPY), V of the ADC that found the row
static void rowCopyOut(const Regs* in, Regs* out, void (*fn)(uint16_t, uint16_t)) {
  uint16_t base = bus_rd16(0x8384B1 + (uint32_t)in->x), from = (uint16_t)(in->a + base);
  fn(in->a, in->x);
  out->a = bus_rd16(0x7F0000 + (uint32_t)(uint16_t)(from + 0x1E));
  out->x = in->x;
  out->y = 0x0020;
  setAdc16(out, in->a, base);
  setNZ16(out, in->x);
  out->c = true;
}
static void call_row_copy_6a33(const Regs* in, Regs* out) { rowCopyOut(in, out, row_copy_6a33); }
static void call_row_copy_6b33(const Regs* in, Regs* out) { rowCopyOut(in, out, row_copy_6b33); }
static void fuzz_row_copy(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)ram;
  r->a = (uint16_t)rnd();
  r->x = (uint16_t)(2 * (rnd() % 16));
}

// ---- tile_block_5x5 ($82:AD99) ----  always ends at X = A = $0A (the SBC #$13E: C set,
// V clear), Y = 0, N clear and Z set by the last DEC
static void call_tile_block_5x5(const Regs* in, Regs* out) {
  tile_block_5x5(in->a);
  out->a = out->x = 0x000A;
  out->y = 0;
  out->n = out->v = false;
  out->z = out->c = true;
}
static void fuzz_tile_block_5x5(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)ram;
  r->a = (uint16_t)rnd();
}

// ---- track_settings_load ($85:831C) ----  A = the $1F34 word (N Z of its LDA), X = 2 *
// track, C = bit 15 of the track (its ASL), V as on entry
static void call_track_settings_load(const Regs* in, Regs* out) {
  (void)in;
  uint16_t track = rd16(0x047B);
  track_settings_load();
  out->a = rd16(0x1F34);
  out->x = (uint16_t)(track << 1);
  setNZ16(out, out->a);
  out->c = (track & 0x8000) != 0;
}
// any of the tracks (and beyond), either player's flag set
static void fuzz_track_settings_load(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x047B, (uint16_t)(rnd() % 0x40));
  ram[0x039C] = (rnd() & 1) ? (uint8_t)rnd() : 0;
  ram[0x03CB] = (rnd() & 1) ? (uint8_t)rnd() : 0;
}

// ---- rows_7f_copy8 ($86:CE4A) ----  after the last MVN: A = $FFFF, X and Y past it; PLB
// of the caller's bank sets N and Z; C and V of the last ADC #8
static void call_rows_7f_copy8(const Regs* in, Regs* out) {
  out->x = in->x;
  out->y = in->y;
  rows_7f_copy8(&out->x, &out->y);
  out->a = 0xFFFF;
  Regs t;
  setAdc16(&t, (uint16_t)(out->y - 0x38 - 8), 0x0008);
  out->c = t.c;
  out->v = t.v;
  out->n = (in->db & 0x80) != 0;
  out->z = in->db == 0;
}
static void fuzz_rows_7f_copy8(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)ram;
  r->x = (uint16_t)(rnd() % 0xF000);
  r->y = (uint16_t)(rnd() % 0xF000);
}

// ---- block_c4_build ($80:C31A) ----  A = $40 (N Z clear), X = $B2F2 and Y = D+$C4 (the
// second MVN's end and its LDY), C V of the ADC #$60
static void call_block_c4_build(const Regs* in, Regs* out) {
  uint16_t at = rd16((uint16_t)(in->d + 0xC4));
  block_c4_build(in->d);
  out->a = 0x0040;
  out->x = 0xB2F2;
  out->y = at;
  setAdc16(out, at, 0x0060);
  out->n = out->z = false;
}
// the block anywhere in low WRAM above the direct pages and below the stack
static void fuzz_block_c4_build(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  wr16ram(ram, (uint16_t)(r->d + 0xC4), (uint16_t)(0x0400 + rnd() % 0x1200));  // clear of D+$C4 itself and the stack
}

// ---- queue_7f_init ($80:EBCE) ----  A = $FFFE (N set, Z clear), X at the $FFFE word,
// Y = 0; C V as on entry
static void call_queue_7f_init(const Regs* in, Regs* out) {
  uint16_t x = rd16((uint16_t)(in->d + 0x5E));
  queue_7f_init(in->d);
  out->a = 0xFFFE;
  out->x = (uint16_t)(x + 0x38);
  out->y = 0;
  out->n = true;
  out->z = false;
}
static void fuzz_queue_7f_init(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  wr16ram(ram, (uint16_t)(r->d + 0x5E), (uint16_t)(2 * (rnd() % 0x7F00)));
}

// ---- text_map_clear ($8A:9BF6) ----  A = 0, X = $FFFE (N set, Z clear by the last DEX)
static void call_text_map_clear(const Regs* in, Regs* out) {
  (void)in;
  text_map_clear();
  out->a = 0;
  out->x = 0xFFFE;
  out->n = true;
  out->z = false;
}

// ---- variant_offsets_add ($8A:98B4) ----  A = the last sum (C V of its ADC), X = the
// count, Y = 2 * the last entry's byte; N clear and Z set by the last DEC
static void call_variant_offsets_add(const Regs* in, Regs* out) {
  (void)in;
  bool v2 = (rd8(0x03DC) & 0x0F) == 2;
  uint16_t count = v2 ? 4 : 0x0A;
  uint32_t table = v2 ? 0x8A9DB5 : 0x8A9D8D;
  variant_offsets_add();
  uint16_t last = (uint16_t)(count - 1), y = (uint16_t)(rd8(0x0446 + (uint32_t)last) << 1);
  uint16_t add = rom16(table + (uint32_t)(uint16_t)(last << 1)), sum = rd16(0x041C + (uint32_t)y);
  out->a = sum;
  out->x = count;
  out->y = y;
  setAdc16(out, (uint16_t)(sum - add), add);
  out->n = false;
  out->z = true;
}
// the variant (2 a third of the time), entry bytes that keep the sums clear of $0446
static void fuzz_variant_offsets_add(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  ram[0x03DC] = (uint8_t)((rnd() & 0xF0) | ((rnd() % 3 == 0) ? 2 : rnd() % 16));
  for(int i = 0; i < 0x0A; i++) ram[0x0446 + i] = (uint8_t)(rnd() % 0x14);
  for(int i = 0; i < 0x28; i += 2) wr16ram(ram, (uint16_t)(0x041C + i), (uint16_t)rnd());
}

// ---- palettes_load_8a ($8A:81BA) ----  the last upload_cgram_now's exit: X = $2202, A =
// $A001 (N Z clear), Y = $20; C of the last ASL of $1536 (bit 13), V as on entry
static void call_palettes_load_8a(const Regs* in, Regs* out) {
  (void)in;
  bool c = (rd16(0x1536) & 0x2000) != 0;
  palettes_load_8a();
  out->x = 0x2202;
  out->a = 0xA001;
  out->y = 0x0020;
  out->n = out->z = false;
  out->c = c;
}
// the table's 16 entries (a larger $1536 reads past bank $8A into the I/O registers)
static void fuzz_palettes_load_8a(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x1536, (uint16_t)(rnd() % 16));
}

// ---- track_map_load ($85:9D7B) ----  filled: A = $3FFF, X = $1040 (C Z set by the CPX, N
// clear), V as on entry; unpacked: decompress's exit (as call_decompress)
static void call_track_map_load(const Regs* in, Regs* out) {
  bool filled = (bus_rd16(0x85A439 + (uint32_t)rd16(0x047B)) & 0x8000) && rd8(0x039C);
  uint16_t end = 0, len = track_map_load(in->d, &end);
  if(filled) {
    out->a = 0x3FFF;
    out->x = 0x1040;
    out->n = false;
    out->z = out->c = true;
    return;
  }
  Regs t = *in;
  t.y = 0x6C33;
  out->a = len;
  out->x = end;
  out->y = (uint16_t)(t.y + len);
  setNZ16(out, len);
  uint16_t a = (uint16_t)~t.y;
  uint32_t sum = (uint32_t)a + out->y + 1;
  out->c = sum > 0xFFFF;
  out->v = (~(a ^ out->y) & (a ^ sum) & 0x8000) != 0;
}
// the tracks (both kinds of entry), the player flag set half the time
static void fuzz_track_map_load(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x047B, (uint16_t)(rnd() % 0x40));
  ram[0x039C] = (rnd() & 1) ? (uint8_t)(1 + rnd() % 255) : 0;
}

// ---- track_segments_load ($82:A954) ----  A = the second field's last sum (C V of its
// ADC), X = 4 * the entry count, Y = 0 (N clear and Z set by the last DEY)
static void call_track_segments_load(const Regs* in, Regs* out) {
  track_segments_load(in->d);
  uint16_t count = (uint16_t)(bus_rd16(0x85A008 + (uint32_t)(uint16_t)(rd16(0x047B) << 1)) >> 2);
  uint16_t last = (uint16_t)(4 * (count - 1)), sum = rd16(0x4F33 + (uint32_t)last);
  uint16_t prev = count > 1 ? rd16(0x4F33 + (uint32_t)(uint16_t)(last - 4)) : 0;
  out->a = sum;
  out->x = (uint16_t)(4 * count);
  out->y = 0;
  setAdc16(out, prev, (uint16_t)(sum - prev));
  out->n = false;
  out->z = true;
}
// any of the 47 tracks (every length in $85:A008 is at least 4)
static void fuzz_track_segments_load(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x047B, (uint16_t)(rnd() % 47));
}

// ---- wait_03e0 ($84:A4CD) ----  A = 0 (N clear, Z set by its LDA)
static void call_wait_03e0(const Regs* in, Regs* out) {
  (void)in;
  out->a = wait_03e0();
  out->n = false;
  out->z = true;
}

// ---- variant_flags_apply ($8A:B0E1) ----  A = the new $1526 (N Z of its ORA), Y = 0,
// C of lsr_n's last shift, V as on entry
static void call_variant_flags_apply(const Regs* in, Regs* out) {
  (void)in;
  out->c = variant_flags_apply();
  out->a = rd16(0x1526);
  out->y = 0;
  setNZ16(out, out->a);
}
static void fuzz_variant_flags_apply(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x03DC, (uint16_t)rnd());
  wr16ram(ram, 0x1526, (uint16_t)rnd());
  wr16ram(ram, 0x1528, (uint16_t)rnd());
}

// ---- sprite_slots_six ($8A:C19C) ----  A = 1, X = $0C (C Z set by the CPX, N clear),
// V of the last ADC #$10
static void call_sprite_slots_six(const Regs* in, Regs* out) {
  (void)in;
  uint16_t first = rd16(DP + 0x42);
  sprite_slots_six();
  out->a = 0x0001;
  out->x = 0x000C;
  Regs t;
  setAdc16(&t, (uint16_t)(first + 0x50), 0x0010);
  out->v = t.v;
  out->n = false;
  out->z = out->c = true;
}
static void fuzz_sprite_slots_six(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x42, (uint16_t)rnd());
  wr16ram(ram, DP + 0x44, (uint16_t)rnd());
}

// ---- tiles_planar_upload ($8A:8CB7) ----  upload_vram_now's exit (A = $1801), then X =
// the entry X (N Z of its PLX), Y = $6FE0; C V of the ADC that found the graphic
static void tilesPlanarOut(const Regs* in, Regs* out, void (*fn)(uint16_t, uint16_t), uint16_t vram) {
  uint16_t base = bus_rd16(0x8384B1 + (uint32_t)in->x);
  fn(in->a, in->x);
  out->a = 0x1801;
  out->x = in->x;
  out->y = vram;
  setAdc16(out, base, in->a);
  setNZ16(out, in->x);
}
static void call_tiles_planar_upload(const Regs* in, Regs* out) { tilesPlanarOut(in, out, tiles_planar_upload, 0x6FE0); }
static void call_tiles_planar_upload_5fe0(const Regs* in, Regs* out) {
  tilesPlanarOut(in, out, tiles_planar_upload_5fe0, 0x5FE0);
}
// one of the first 20 graphics (all at least one tile), anywhere low in bank $7F
static void fuzz_tiles_planar_upload(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)ram;
  r->x = (uint16_t)(2 * (rnd() % 20));
  r->a = (uint16_t)(rnd() % 0x6000);
}

// ---- track_backdrop_load ($82:A1D3) ----  the last upload_vram_now's exit: A = $1801 (N Z
// clear), X = the second map, Y = $7800; C V of the last ADC #$400 of the map words
static void call_track_backdrop_load(const Regs* in, Regs* out) {
  track_backdrop_load(in->d);
  const uint32_t x = (uint16_t)((bus_rd8(0x84A66C + (uint32_t)rd16(0x047B)) << 2) << 1);
  const uint16_t map1 = bus_rd16(0x85A34A + x), map2 = bus_rd16(0x85A34C + x);
  const uint16_t last = rd16(0x10000 + (uint32_t)(uint16_t)(map1 + 2 * 0x5FF));
  out->a = 0x1801;
  out->x = map2;
  out->y = 0x7800;
  setAdc16(out, (uint16_t)(last - 0x0400), 0x0400);
  out->n = out->z = false;
}
static void fuzz_track_backdrop_load(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x047B, (uint16_t)(rnd() % 47));
}

// ---- race_positions_update ($82:AD2C) ----  ends with PLB of the $83 its PEA $837E pushed
// (DB = $83, N set, Z clear) and Y = 0; V clear (the ADCs only add small numbers). Player 2's position unchanged: A =
// it, C set (its CMP was equal); changed: A = its message, C = race_message_request_02e6's.
// X = the last message requested << 1, else $35 * 20.
static void call_race_positions_update(const Regs* in, Regs* out) {
  (void)in;
  uint16_t old1 = rd16(0x1ED2), old2 = rd16(0x1ED4), slot2 = rd16(0x02E6);
  race_positions_update();
  uint16_t p1 = rd16(0x0142), p2 = rd16(0x0144);
  out->x = 0x0424;
  if(p1 != old1) out->x = (uint16_t)((p1 + 0x25) << 1);
  if(p2 != old2) {
    out->a = (uint16_t)(p2 + 0x25);
    out->x = (uint16_t)(out->a << 1);
    out->c = rom16(0x83A0A4 + (uint32_t)(uint16_t)(slot2 << 1)) >= rom16(0x83A0A4 + (uint32_t)out->x);
  } else {
    out->a = p2;
    out->c = true;
  }
  out->y = 0;
  out->v = false;
  out->db = 0x83;
  out->n = true;
  out->z = false;
}
// random progress for the 20 cars (often close to a player's), the last positions shown
// right or wrong, the message slots
static void fuzz_race_positions_update(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(uint16_t x = 0; x < 0x35 * 20; x = (uint16_t)(x + 0x35)) {
    bool near = rnd() % 3 != 0;
    uint16_t lo = (uint16_t)rnd(), mid = near ? (uint16_t)(rnd() % 4) : (uint16_t)rnd(), hi = near ? 0 : (uint16_t)rnd();
    wr16ram(ram, (uint16_t)(0xFAB5 + x), lo);
    wr16ram(ram, (uint16_t)(0xFAB7 + x), mid);
    wr16ram(ram, (uint16_t)(0xFAD3 + x), hi);
  }
  wr16ram(ram, 0x1ED2, (uint16_t)(rnd() % 22));
  wr16ram(ram, 0x1ED4, (uint16_t)(rnd() % 22));
  wr16ram(ram, 0x0233, (uint16_t)(rnd() % 0x4F));
  wr16ram(ram, 0x02E6, (uint16_t)(rnd() % 0x4F));
}

// ---- race_bands_merge ($85:B1FF) ----  A = $00B4's high byte : 0 (the 8-bit loads keep B),
// X = D+$16 (its last LDX), Y = the final Y; N clear, Z and C set (CMP #0 of the end);
// V as on entry
static void bandsMergeOut(Regs* out, uint16_t split, uint16_t (*fn)(void)) {
  uint16_t hi = (uint16_t)(rd16(split) & 0xFF00);
  out->y = fn();
  out->a = hi;
  out->x = rd16(DP + 0x16);
  out->n = false;
  out->z = out->c = true;
}
static void call_race_bands_merge(const Regs* in, Regs* out) { (void)in; bandsMergeOut(out, 0x00B4, race_bands_merge); }
static void call_race_bands_merge2(const Regs* in, Regs* out) { (void)in; bandsMergeOut(out, 0x00B6, race_bands_merge2); }
static void call_vs_bands_merge(const Regs* in, Regs* out) { (void)in; bandsMergeOut(out, 0x00B6, vs_bands_merge); }
// two zero-ended ascending lists, often sharing lines, sometimes one empty; either start
static void fuzzBandsMerge(uint8_t* ram, uint32_t (*rnd)(void), int view) {
  for(int list = 0; list < 2; list++) {
    static const uint16_t bases[2][2] = {{0x4CFE, 0x4D4A}, {0x4D96, 0x4DE2}}, ats[2][2] = {{0x0164, 0x0168}, {0x0166, 0x016A}};
    uint16_t at = (uint16_t)(rnd() % 0x40), base = bases[view][list];
    wr16ram(ram, ats[view][list], at);
    int n = (int)(rnd() % 10);
    uint8_t line = 0;
    for(int i = 0; i < n; i++) {
      line = (uint8_t)(line + 1 + rnd() % ((rnd() & 1) ? 4 : 24));  // close lines meet often
      if(line == 0) break;
      ram[base + at + i] = line;
      if(i == n - 1 || line > 0xE0) { n = i + 1; break; }
    }
    ram[base + at + n] = 0;
  }
  wr16ram(ram, view ? 0x00B6 : 0x00B4, (rnd() & 1) ? 0 : (uint16_t)rnd());
}
static void fuzz_race_bands_merge(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  fuzzBandsMerge(ram, rnd, 0);
}
static void fuzz_race_bands_merge2(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  fuzzBandsMerge(ram, rnd, 1);
}
static void fuzz_vs_bands_merge(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  fuzzBandsMerge(ram, rnd, 0);
  fuzzBandsMerge(ram, rnd, 1);
}

// ---- hdma_gradients_init ($8A:8F18) ----  the last hdma_gradient_build's exit flags (as its
// adapter: from $152C and $1530 afterwards); registers not compared, as for that row
static void call_hdma_gradients_init(const Regs* in, Regs* out) {
  (void)in;
  hdma_gradients_init();
  uint16_t next = rd16(0x152C), step = rd16(0x1530), now = (uint16_t)(next - step);
  out->n = false;
  out->z = true;
  setAdc16(out, now, step);
  out->c = (now & 0xFF00) >= (next & 0xFF00);
}
static void fuzz_hdma_gradients_init(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x1536, (uint16_t)(rnd() % 16));
  wr16ram(ram, DP + 0x72, (uint16_t)rnd());
}

// ---- player_frame_setup ($83:E2D7) ----  registers not compared (as player_road_limits'
// row). Without the controls: N Z clear (the PLD of $0100), C V as player_road_limits
// leaves them. With them: N clear, Z of the item's byte (its AND #$FF), V = bit 14 of $000C
// (the last BIT of a memory word), C of the last CPX #5 when $03A3 was pressed, else of the
// ASL of the item when $03A1 was pressed or held, else clear (the ADC #0).
static void call_player_frame_setup(const Regs* in, Regs* out) {
  (void)in;
  bool controls = !(rd16(0x0244) & 0x0004) && (rd16(0x03DC) & 0x0010);
  uint16_t pad = rd16(0x000C), m = rd16(0x000E), item = rd16(0x014C);
  uint16_t a1 = rd16(0x03A1), a5 = rd16(0x03A5), a3 = rd16(0x03A3);
  bool c = false;
  if((a1 & m) || (a1 & pad)) c = (item & 0x8000) != 0;
  m = (uint16_t)(m & ~a1);
  if(a5 & m) {
    for(uint16_t i = item;;) {
      i = (uint16_t)(i - 1);
      if(i == 0) { item = i; break; }
      if(i & 0x8000) break;
      if(rd8(0x0397 + (uint32_t)i)) { item = i; break; }
    }
  }
  m = (uint16_t)(m & ~a5);
  if(a3 & m) {
    uint16_t i = item;
    for(;;) {
      i = (uint16_t)(i + 1);
      if(i == 5) break;
      if(rd8(0x0397 + (uint32_t)i)) break;
    }
    c = i >= 5;
  }
  player_frame_setup();
  if(!controls) {
    out->n = out->z = false;
    return;
  }
  out->n = false;
  out->z = rd16(0x025C) == 0;
  out->v = (pad & 0x4000) != 0;
  out->c = c;
}
// the controls read half the time (else player_road_limits): buttons held and pressed, the masks (one button each,
// sometimes the same), the item and the items carried
static void fuzz_player_frame_setup(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  if(rnd() & 1) {
    wr16ram(ram, 0x0244, (uint16_t)(rnd() & ~0x0004));
    wr16ram(ram, 0x03DC, (uint16_t)(rnd() | 0x0010));
  } else {  // the other path, player_road_limits (the short race only reads the controls)
    wr16ram(ram, 0x0244, (uint16_t)(rnd() | 0x0004));
  }
  wr16ram(ram, 0x000C, (uint16_t)rnd());
  wr16ram(ram, 0x000E, (uint16_t)rnd());
  static const uint16_t masks[5] = {0x039D, 0x039F, 0x03A1, 0x03A3, 0x03A5};
  for(int k = 0; k < 5; k++) wr16ram(ram, masks[k], (uint16_t)(1u << (rnd() % 16)));
  wr16ram(ram, 0x014C, (uint16_t)(rnd() % 5));
  for(int i = 0; i < 6; i++) ram[0x0397 + i] = (rnd() & 1) ? (uint8_t)rnd() : 0;
  ram[0x0393] = (uint8_t)(rnd() % 6);
  ram[0x0394] = (uint8_t)(rnd() % 6);
  ram[0x0396] = (uint8_t)(rnd() % 6);
}

// ---- scene_objects_place ($8A:86AB) ----  not placing: A = 0 (Z set, N clear), X = 8 *
// $1536, C = bit 13 of $1536 (the last ASL). Placing: func_8A92CF's exit for the last
// object (A = D+$7C, N Z of it, C V of D+$7A + $10, X its slot)
static void call_scene_objects_place(const Regs* in, Regs* out) {
  (void)in;
  uint16_t sel = rd16(0x1536), x8 = (uint16_t)(sel << 3);
  bool placing = (bus_rd16(0x8A9351 + (uint32_t)x8) & 0x0001) != 0;
  // the last object's D+$7A, from the state before (the callees may move D+$7A on)
  uint16_t y84 = (uint16_t)-rd16(DP + 0x62);
  uint16_t last7a = (uint16_t)(((rd16(DP + 0x3C) >> 6) >> 1) + 0x0140 + (y84 >= 0x000B));
  out->x = scene_objects_place();
  if(!placing) {
    out->a = 0;
    out->n = false;
    out->z = true;
    out->c = (sel & 0x2000) != 0;
    return;
  }
  out->a = rd16(DP + 0x7C);
  setNZ16(out, out->a);
  setAdc16(out, last7a, 0x0010);
}
static void fuzz_scene_objects_place(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x1536, (uint16_t)(rnd() % 16));
  wr16ram(ram, DP + 0x3C, (uint16_t)rnd());
  wr16ram(ram, DP + 0x62, (uint16_t)((rnd() & 1) ? rnd() % 0x20 : rnd()));
}

// ---- result_slot_step / result_tally ($84:A31B, $84:A34B) ----  X = $0444 first. The byte
// clear: A = 0 (Z set, N clear); C of the CMP #$100 unless variant 0 (then as on entry). Set:
// the step leaves A = $036C (N Z, C V of its ADC #4); the tally A = X = the table word at
// $85:D590 + offset + $036C (C V of that ADC), N Z of the INC. Y = the tally's offset.
// (The second entries, $84:A31E and $84:A34E, take X from their caller.)
static void resultOut(const Regs* in, Regs* out, bool tally, bool at) {
  uint16_t v = (uint16_t)(rd16(0x03DC) & 0x0300), x0 = at ? in->x : rd16(0x0444);
  uint16_t off = v == 0 ? 0 : v == 0x0100 ? 0x001C : 0x0044;
  uint32_t t = v == 0 ? 0x85D501 : v == 0x0100 ? 0x85D530 : 0x85D55F;
  bool set = bus_rd8(t + (uint32_t)x0) != 0;
  uint16_t base = rd16(0x036C);
  if(at) {
    if(tally) result_tally_at(x0); else result_slot_step_at(x0);
  } else {
    if(tally) result_tally(); else result_slot_step();
  }
  out->x = x0;
  if(tally) out->y = off;
  if(v != 0) out->c = true;  // CMP #$100 of $100, $200 or $300
  if(!set) {
    out->a = 0;
    out->n = false;
    out->z = true;
    return;
  }
  if(!tally) {
    out->a = rd16(0x036C);
    setNZ16(out, out->a);
    setAdc16(out, base, 0x0004);
    return;
  }
  setAdc16(out, off, base);
  out->a = out->x = bus_rd16(0x85D590 + (uint32_t)(uint16_t)(off + base));  // its LDA, then TAX
  setNZ16(out, rd16(0x036E + (uint32_t)out->x));
}
static void call_result_slot_step(const Regs* in, Regs* out) { resultOut(in, out, false, false); }
static void call_result_tally(const Regs* in, Regs* out) { resultOut(in, out, true, false); }
static void call_result_slot_step_at(const Regs* in, Regs* out) { resultOut(in, out, false, true); }
static void call_result_tally_at(const Regs* in, Regs* out) { resultOut(in, out, true, true); }
// every variant, the entry and the slot within the tables
static void fuzz_result(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x03DC, (uint16_t)rnd());
  wr16ram(ram, 0x0444, (uint16_t)(rnd() % 0x2F));
  wr16ram(ram, 0x036C, (uint16_t)(4 * (rnd() % 7)));
}
// the second entries: X a track below $0444 (the caller's count), or any of the table's
static void fuzz_result_at(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_result(r, ram, rnd);
  r->x = (uint16_t)(rnd() % 0x2F);
}

// ---- clear_041c ($8A:9D1E) ----  A = 0, X = $FFFE (N set, Z clear by the last DEX)
static void call_clear_041c(const Regs* in, Regs* out) {
  (void)in;
  clear_041c();
  out->a = 0;
  out->x = 0xFFFE;
  out->n = true;
  out->z = false;
}

// ---- name_copy_7f2000 ($80:E8CE) ----  A = $1400 (N Z clear), X past the words, Y at the
// zero; C V as on entry
static void call_name_copy_7f2000(const Regs* in, Regs* out) {
  out->x = in->x;
  name_copy_7f2000(in->d, &out->x, &out->y);
  out->a = 0x1400;
  out->n = out->z = false;
}
// a name of up to 12 bytes for either player
static void fuzz_name_copy_7f2000(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint16_t p = (rnd() & 1) ? 0x002F : 0x0000;
  wr16ram(ram, DP + 0x94, p);
  int n = (int)(rnd() % 12);
  for(int i = 0; i < n; i++) ram[0x037E + p + i] = (uint8_t)(1 + rnd() % 255);
  ram[0x037E + p + n] = 0;
  r->x = (uint16_t)(rnd() % 0x4000);
}

// ---- car_blocks_7f2000 / car_block_7f2000 ($80:E8F4, $80:E911) ----  the last
// block_copy_7f2000's exit: A = its length word's high byte : its last byte, N clear, Z set;
// C clear (the ASL of the car, which block_copy_7f2000 leaves alone), except with no car in
// $80:E8F4 (its tail jump: C as on entry); V as on entry
static void carBlocksOut(const Regs* in, Regs* out, void (*fn)(uint16_t, uint16_t*, uint16_t*)) {
  out->x = in->x;
  out->y = in->y;
  fn(in->d, &out->x, &out->y);
  // the last block's start, from the car
  uint16_t car = rd16(0x03A8 + (uint32_t)rd16((uint16_t)(in->d + 0x94))), start;
  if(fn == car_blocks_7f2000) start = (car & 0x8000) ? 0x00A8 : 0x00BE;
  else start = rom16(0x81D3F1 + (uint32_t)(uint16_t)(((car & 0x8000) ? 0x0013 : car) << 1));
  uint16_t len = bus_rd16((0x81D419 + (uint32_t)start) & 0xFFFFFF);
  out->a = (uint16_t)((len & 0xFF00) | bus_rd8((0x81D419 + (uint32_t)(uint16_t)(out->y - 1)) & 0xFFFFFF));
  out->n = false;
  out->z = true;
  if(!(fn == car_blocks_7f2000 && (car & 0x8000))) out->c = false;
}
static void call_car_blocks_7f2000(const Regs* in, Regs* out) { carBlocksOut(in, out, car_blocks_7f2000); }
static void call_car_block_7f2000(const Regs* in, Regs* out) { carBlocksOut(in, out, car_block_7f2000); }
// either player, a car of the tables or none, the copy within bank $7F
static void fuzz_car_blocks(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint16_t p = (rnd() & 1) ? 0x002F : 0x0000;
  wr16ram(ram, DP + 0x94, p);
  wr16ram(ram, (uint16_t)(0x03A8 + p), (rnd() % 4 == 0) ? (uint16_t)(0x8000 | rnd()) : (uint16_t)(rnd() % 0x14));
  r->x = (uint16_t)(rnd() % 0x4000);
}

// ---- track_next ($84:A41B) ----  A = the new $0444 (N Z of its AND), X = its index, C
// set (the CMP that matched), V as on entry
static void call_track_next(const Regs* in, Regs* out) {
  (void)in;
  out->x = track_next();
  out->a = rd16(0x0444);
  setNZ16(out, out->a);
  out->c = true;
}
// every variant, $0444 one of the first 20 tracks of its list
static void fuzz_track_next(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint16_t v = (uint16_t)rnd();
  wr16ram(ram, 0x03DC, v);
  uint32_t list = (v & 0x0300) == 0x0100 ? 0x8ACADE : (v & 0x0300) == 0x0200 ? 0x8ACB01 : 0x8ACAC4;
  wr16ram(ram, 0x0444, rom8(list + rnd() % 20));
}

// ---- results_init ($84:A4ED) ----  X = $14, Y = 0, DB = $81 (the MVNs'); A = $03E2 (N of
// its LDA, Z of the BIT #2), C V clear (the last ADC #$202); with bit 1, A's low byte = 1
// (N Z clear)
static void call_results_init(const Regs* in, Regs* out) {
  (void)in;
  uint16_t f = rd16(0x03E2);
  results_init();
  out->x = 0x0014;
  out->y = 0;
  out->db = 0x81;
  out->c = out->v = false;
  if(f & 0x0002) {
    out->a = (uint16_t)((f & 0xFF00) | 0x01);
    out->n = out->z = false;
  } else {
    out->a = f;
    out->n = (f & 0x8000) != 0;
    out->z = true;
  }
}
static void fuzz_results_init(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x03E2, (uint16_t)rnd());
}

// ---- prize_money_award ($84:A388) ----  player 2 placed: A = 0 (its bonus word, Z set, N
// clear), X = 4 * the bonus steps; else A = $03D7 (N set) and X as player 1 left it (4 * its
// steps, or as on entry). C and V as the C returns them (the last decimal ADC's, as the CPU
// computes V)
static uint16_t prizeSteps(uint16_t bits) {
  uint16_t n = 1;
  while(bits >>= 1) n++;
  return n;
}
static void call_prize_money_award(const Regs* in, Regs* out) {
  uint16_t p1 = rd16(0x03A8), p2 = rd16(0x03D7), b1 = rd16(0x03AB), b2 = rd16(0x03DA);
  bool v = in->v;
  out->c = prize_money_award(in->c, &v);
  out->v = v;
  out->x = in->x;
  if(!(p1 & 0x8000)) out->x = (uint16_t)(4 * prizeSteps(b1));
  if(!(p2 & 0x8000)) {
    out->x = (uint16_t)(4 * prizeSteps(b2));
    out->a = 0;
    out->n = false;
    out->z = true;
  } else {
    out->a = p2;
    out->n = true;
    out->z = false;
  }
}
// places 0-9 or none, BCD money (often near a carry), bonus bits
static uint16_t bcdWord(uint32_t (*rnd)(void)) {
  uint16_t v = 0;
  for(int shift = 0; shift < 16; shift += 4) v |= (uint16_t)((rnd() % 10) << shift);
  return v;
}
static void fuzz_prize_money_award(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  static const uint16_t place[2] = {0x03A8, 0x03D7}, money[2] = {0x038E, 0x03BD}, bonus[2] = {0x03AB, 0x03DA};
  for(int k = 0; k < 2; k++) {
    wr16ram(ram, place[k], (rnd() % 4 == 0) ? (uint16_t)(0x8000 | rnd()) : (uint16_t)(rnd() % 10));
    wr16ram(ram, money[k], (rnd() & 1) ? (uint16_t)(0x9900 | (bcdWord(rnd) & 0x00FF)) : bcdWord(rnd));
    wr16ram(ram, (uint16_t)(money[k] + 2), (rnd() & 1) ? 0x9999 : bcdWord(rnd));
    wr16ram(ram, bonus[k], (rnd() & 1) ? (uint16_t)(rnd() & 0x00FF) : (uint16_t)rnd());
  }
}

// ---- title_zoom_count ($80:F5BF) ----  as title_zoom_cycle: A = func_80F745's result, X =
// D+$34 (N Z of it), V by v_after_func_80F745, C clear (its CLC)
static void call_title_zoom_count(const Regs* in, Regs* out) {
  out->a = title_zoom_count();
  out->x = rd16(DP + 0x34);
  setNZ16(out, out->x);
  out->v = v_after_func_80F745(v_after_inverse_scale(in->v));
  out->c = false;
}
// title_zoom's state, the counter often about to reach 0
static void fuzz_title_zoom_count(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_title_zoom(r, ram, rnd);
  wr16ram(ram, DP + 0x30, (uint16_t)((rnd() & 1) ? 1 : rnd() % 0x40));
}

// ---- slot_order_fill ($8A:9CF8) ----  the last func_8A9BB5 found none: A = 0, X = D+$66
// (the slots' end), C set; N Z of the final Y; V as on entry
static void call_slot_order_fill(const Regs* in, Regs* out) {
  (void)in;
  out->y = slot_order_fill();
  out->a = 0;
  out->x = rd16(DP + 0x66);
  out->c = true;
  setNZ16(out, out->y);
}
static void fuzz_slot_order_fill(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(uint16_t i = 0; i < 20; i++) wr16ram(ram, (uint16_t)(0x041C + 2 * i), (uint16_t)(rnd() % 8));
}

// ---- player_names_put ($8A:9AF1) ----  the exit of the last text_line_put (A kept, X = the
// D+$54 before it, Y = the string's length, N Z of A, C V of D+$54 + $10), followed through
// the calls; without any: the CMP #2's flags (variants 3-15), or the last LSR's (variant 1)
static void namePutModel(Regs* out, uint32_t at, uint16_t a, uint16_t* line) {
  uint16_t n = 0;
  while(bus_rd8((at + n) & 0xFFFFFF)) n++;
  out->a = a;
  out->x = *line;
  out->y = n;
  setNZ16(out, a);
  setAdc16(out, *line, 0x0010);
  *line = (uint16_t)(*line + 0x10);
}
static void call_player_names_put(const Regs* in, Regs* out) {
  (void)in;
  uint16_t dc = rd16(0x03DC), v = (uint16_t)(dc & 0x000F), line = rd16(DP + 0x54);
  if(v == 0) namePutModel(out, 0x81037E, 0, &line);
  else if(v == 2) {
    static const uint16_t at[4] = {0x03E4, 0x03F1, 0x03FE, 0x040B};
    for(int k = 0; k < 4; k++) namePutModel(out, 0x810000 + at[k], 2, &line);
  } else if(v != 1) {
    out->a = v;
    out->n = false;
    out->z = false;
    out->c = true;  // CMP #2 of 3-15
  } else if((dc & 0x0030) == 0x0010) {
    namePutModel(out, 0x81037E, 0x0010, &line);
    namePutModel(out, 0x8A9B7D, 0x0010, &line);
  } else {
    if(dc & 0x0010) namePutModel(out, 0x81037E, (uint16_t)(dc >> 5), &line);
    if(dc & 0x0020) namePutModel(out, 0x8103AD, (uint16_t)(dc >> 6), &line);
    else {  // the second LSR found bit 5 clear
      out->a = (uint16_t)(dc >> 6);
      out->n = false;
      out->z = out->a == 0;
      out->c = false;
    }
  }
  player_names_put();
}
// the variants 0-3 (bits 4-5 any), short names in every slot, the line anywhere in the map
static void fuzz_player_names_put(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x03DC, (uint16_t)((rnd() & 0xFFC0) | (rnd() & 0x30) | (rnd() % 4)));
  static const uint16_t at[6] = {0x037E, 0x03AD, 0x03E4, 0x03F1, 0x03FE, 0x040B};
  for(int k = 0; k < 6; k++) {
    int n = (int)(rnd() % 12);
    for(int i = 0; i < n; i++) ram[at[k] + i] = (uint8_t)(0x20 + rnd() % 0x5F);
    ram[at[k] + n] = 0;
  }
  wr16ram(ram, DP + 0x54, (uint16_t)(rnd() % 0x400));
}

// ---- shake_step ($82:8F8F) ----  by path: off, A = $8000; start, the CMP #4 of the byte
// $0397 with X = the new $1F18; running, the last compare (A = $B0 or $98 when stored, X =
// the old $1F18 & $FFF8, V of the ADC); ending, A = the table word, X = the pick, C of the
// CMP #$B0
static void call_shake_step(const Regs* in, Regs* out) {
  (void)in;
  uint16_t on = rd16(0x0231), s = rd16(0x1F18), b = rd8(0x0397), step2 = (uint16_t)(rd16(0x1E9E) << 1);
  shake_step();
  if(on == 0) {
    out->a = 0x8000;
    out->n = true;
    out->z = false;
    return;
  }
  if(s & 0x8000) {
    out->a = b;
    out->x = rd16(0x1F18);
    out->c = b >= 4;
    setNZ16(out, (uint16_t)(b - 4));
    return;
  }
  if(s < 0x0098) {
    uint16_t before = (uint16_t)(s & 0xFFF8), step = (uint16_t)((uint16_t)(step2 + s) & 0xFFF8);
    out->x = before;
    setAdc16(out, step2, s);
    if(step >= 0x0098) {
      out->a = 0x00B0;
      out->n = out->z = false;
      out->c = true;
    } else if(step < 0x0030) {
      out->a = step;
      out->c = false;
      setNZ16(out, (uint16_t)(step - 0x0030));
    } else if(before >= 0x0030) {
      out->a = step;
      out->c = true;
      setNZ16(out, (uint16_t)(before - 0x0030));
    } else {
      out->a = 0x0098;
      out->n = out->z = false;
      out->c = false;
    }
    return;
  }
  out->a = rd16(0x1F18);
  setNZ16(out, out->a);
  out->c = s >= 0x00B0;
  // X = the pick: the table word's offset (both tables are 3 words; find it)
  uint32_t t = s >= 0x00B0 ? 0x82913F : 0x829139;
  for(uint16_t k = 0; k < 6; k += 2) if(rom16(t + k) == out->a) { out->x = k; break; }
}
// every path: off, start (either side of 4), running near $30 and $98, ending
static void fuzz_shake_step(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x0231, (rnd() % 6 == 0) ? 0 : (uint16_t)(1 + rnd() % 100));
  static const uint16_t starts[8] = {0x8000, 0xFFFF, 0x0000, 0x0028, 0x002E, 0x0090, 0x0098, 0x00B0};
  uint16_t s = starts[rnd() % 8];
  if(s < 0x0098) s = (uint16_t)(s + rnd() % 8);
  else if(!(s & 0x8000)) s = (uint16_t)(s + rnd() % 0x20);
  wr16ram(ram, 0x1F18, s);
  ram[0x0397] = (uint8_t)(rnd() % 8);
  wr16ram(ram, 0x1E9E, (uint16_t)(rnd() % 8));
  fuzz_rng_state(ram, rnd);
}

// ---- track_objects_stream ($82:AF33) ----  ends on the CMP that found a record not reached:
// A = $0226, C clear, N Z of the difference, X = that record's index; Y = the last
// upload's VRAM word (its LDY D+$18), V of its ADC; with no upload Y and V as on entry. The
// records' choices read only state the routine does not change, so they are followed here
static void call_track_objects_stream(const Regs* in, Regs* out) {
  uint16_t pos = rd16(0x0226), x = rd16(DP + 0xA0);
  bool first = true;
  for(;;) {
    uint16_t at = rd16(0x5B91 + (uint32_t)x);
    if(pos < at) break;
    first = false;
    uint16_t kind = rd16(0x5B94 + (uint32_t)x), slot = rd8(0x5B93 + (uint32_t)x);
    if(kind) {
      bool high = (kind & 0xFF00) != 0;
      uint16_t t = bus_rd16((high ? 0x86AC36 : 0x86AE36) + (uint32_t)(uint16_t)(kind << 1));
      if(!t) x = (uint16_t)(kind << 1);  // the empty entry's quirk
      if(t) {
        uint16_t off = bus_rd16((high ? 0x860022 : 0x86001E) + (uint32_t)t);
        uint16_t base = bus_rd16(0x860000 + (uint32_t)t);
        Regs v;
        setAdc16(&v, rd16(0x6591 + (uint32_t)base), off);
        out->v = v.v;
        out->y = (uint16_t)(slot << 5);
      }
    }
    x = (uint16_t)(x + 5);
  }
  (void)first;
  track_objects_stream();
  out->a = pos;
  out->x = x;
  out->c = false;
  setNZ16(out, (uint16_t)(pos - rd16(0x5B91 + (uint32_t)x)));
  (void)in;
}
// a few records from D+$A0 on (the kinds within the tables, low or high), the car between
// their starts, a record far ahead to end on
static void fuzz_track_objects_stream(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint16_t x = (uint16_t)(5 * (rnd() % 8)), at = (uint16_t)(rnd() % 0x100);
  wr16ram(ram, DP + 0xA0, x);
  int n = (int)(1 + rnd() % 5);
  bool quirk = false;
  for(int k = 0; k < n && !quirk; k++, x = (uint16_t)(x + 5)) {
    wr16ram(ram, (uint16_t)(0x5B91 + x), at);
    ram[0x5B93 + x] = (uint8_t)(rnd() % 0x40);
    // a kind whose entry is empty or points into ROM (others would read I/O space)
    uint16_t kind = 0;
    for(int tries = 0; tries < 8 && rnd() % 4 != 0; tries++) {
      bool high = rnd() & 1;
      // high kinds below $29: $86:AC36 + 2 * (k << 8) stays in bank $86
      uint16_t k = (uint16_t)(1 + rnd() % (high ? 0x28 : 0x30));
      uint16_t t = rom16((high ? 0x86AC36 : 0x86AE36) + (uint32_t)(uint16_t)((high ? (uint16_t)(k << 8) : k) << 1));
      // an empty entry (the quirk: the next record is counted from 2 * kind) only for low
      // kinds, with a record there that ends it; a high kind's 2 * kind is far away
      if(t >= 0x8000 || (t == 0 && !high)) {
        kind = high ? (uint16_t)(k << 8) : k;
        quirk = t == 0;  // then the last record, so nothing overwrites the one that ends it
        break;
      }
    }
    wr16ram(ram, (uint16_t)(0x5B94 + x), kind);
    if(quirk) {
      wr16ram(ram, (uint16_t)(0x5B91 + (uint16_t)(2 * kind + 5)), 0xFFFF);
      continue;
    }
    if(kind) {  // the source kept inside bank $7F, as the game's data keeps it (no DMA wrap)
      bool high = (kind & 0xFF00) != 0;
      uint16_t t = rom16((high ? 0x86AC36 : 0x86AE36) + (uint32_t)(uint16_t)(kind << 1));
      if(t) {
        uint16_t off = rom16((high ? 0x860022 : 0x86001E) + (uint32_t)t);
        uint16_t size = rom16((high ? 0x860024 : 0x860020) + (uint32_t)t);
        uint16_t base = rom16(0x860000 + (uint32_t)t);
        uint32_t room = size ? 0x10000u - size : 0;
        uint16_t src = (uint16_t)(room ? rnd() % room : 0);
        wr16ram(ram, (uint16_t)(0x6591 + base), (uint16_t)(src - off));
      }
    }
    at = (uint16_t)(at + 1 + rnd() % 0x40);
  }
  wr16ram(ram, (uint16_t)(0x5B91 + x), 0xFFFF);
  wr16ram(ram, 0x0226, (uint16_t)(rnd() % (uint32_t)(at + 1)));
}

// ---- results_text_build ($8A:9AA5) ----  text_lines_copy's exit, from the state after: A =
// D+$54 (C V of the last line's ADC #$10), N clear, Z set; X = the last line's start + its
// length, Y = past its zero (the lines from $8A:9DEE). Their count is variant_lines' less
// one per name player_names_put wrote (each counts D+$56 down). None left (variant 2: four
// lines, four names): A = 0, N clear, Z set, the rest as the last name's text_line_put left
static void call_results_text_build(const Regs* in, Regs* out) {
  uint16_t dc = rd16(0x03DC), v = (uint16_t)(dc & 0x000F), names = 0;
  if(v == 0) names = 1;
  else if(v == 2) names = 4;
  else if(v == 1) names = (dc & 0x0030) == 0x0010 ? 2 : (uint16_t)(((dc >> 4) & 1) + ((dc >> 5) & 1));
  uint16_t lines = (uint16_t)((v == 2 ? 4 : 0x14) - names);
  results_text_build();
  if(lines == 0) {  // after the four names (D+$54 from 0): the last put's exit
    out->x = 0x0030;
    uint16_t n = 0;
    while(rd8(0x040B + (uint32_t)n)) n++;
    out->y = n;
    setAdc16(out, 0x0030, 0x0010);
    out->a = 0;
    out->n = false;
    out->z = true;
    return;
  }
  (void)in;
  uint16_t y = 0, len = 0;
  for(uint16_t k = 0; k < lines; k++) {
    len = 0;
    while(rom8(0x8A9DEE + (uint32_t)(uint16_t)(y + len))) len++;
    y = (uint16_t)(y + len + 1);
  }
  out->a = rd16(DP + 0x54);
  out->x = (uint16_t)(out->a - 0x10 + len);
  out->y = y;
  setAdc16(out, (uint16_t)(out->a - 0x10), 0x0010);
  out->n = false;
  out->z = true;
}
static void fuzz_results_text_build(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_player_names_put(r, ram, rnd);
  wr16ram(ram, DP + 0x56, (uint16_t)rnd());
}

// ---- player_in_top5 ($8A:9CB0) ----  clear_041c's exit (A = 0, X = $FFFE, N set, Z clear),
// C = not found; V as on entry; Y (slot_order_fill's count) not compared
static void call_player_in_top5(const Regs* in, Regs* out) {
  (void)in;
  out->c = player_in_top5();
  out->a = 0;
  out->x = 0xFFFE;
  out->n = true;
  out->z = false;
}
// every variant (both two-player kinds), the slots' kinds
static void fuzz_player_in_top5(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x03DC, (uint16_t)((rnd() & 0xFFC0) | (rnd() & 0x30) | (rnd() % 4)));
  for(uint16_t i = 0; i < 20; i++) wr16ram(ram, (uint16_t)(0x041C + 2 * i), (uint16_t)(rnd() % 8));
}

// ---- title_scroll_start ($80:F6FD) ----  A = $FF80 (its LDA #), N Z of the INC of D+$3E;
// C V as on entry
static void call_title_scroll_start(const Regs* in, Regs* out) {
  (void)in;
  title_scroll_start();
  out->a = 0xFF80;
  setNZ16(out, rd16(DP + 0x3E));
}
static void fuzz_title_scroll_start(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x3E, (uint16_t)((rnd() & 1) ? 0xFFFF : rnd()));
}

// ---- save_data_seal ($80:8199) ----  after the MVN: A = $FFFF, X = $03E0, Y = $FAB5; the
// PLB of the caller's bank sets N Z; C V of the last ADC of the sum
static void call_save_data_seal(const Regs* in, Regs* out) {
  uint16_t sum = 0, last = 0;
  for(uint32_t x = 0x78; x != 0; x--) {
    last = sum;
    sum = (uint16_t)(sum + rd8(0x0367 + x));
  }
  uint16_t lastByte = rd8(0x0368);
  save_data_seal();
  out->a = 0xFFFF;
  out->x = 0x03E0;
  out->y = 0xFAB5;
  setAdc16(out, last, lastByte);
  out->n = (in->db & 0x80) != 0;
  out->z = in->db == 0;
}
static void fuzz_save_data_seal(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(int i = 0; i < 0x78; i++) ram[0x0368 + i] = (uint8_t)rnd();
}

// ---- text_centred_line_at ($82:8911) ----  C and V as text_centred_line's adapter models them
// (from the string X and palette A); N/Z of the final PLB ($83)
static void call_text_centred_line_at(const Regs* in, Regs* out) {
  textCentredLineCV(out, in->x, in->a);
  text_centred_line_at(in->a, in->x);
  out->n = true;  // its last PLB: the $83 left on the stack
  out->z = false;
}
static void fuzz_text_centred_line_at(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_text_centred_line(r, ram, rnd);
  r->x = (uint16_t)(ram[DP + 0x3C] | ram[DP + 0x3D] << 8);
  r->a = (uint16_t)rnd();
}

// ---- race_pause ($82:884B) ----  a waiting row. Resumed: the last PLX (N/Z of the entry X),
// C/V of the last text_centred_line_at (the line string $0233 * 2, palette $4E61), A $10 (its
// LDA before the TRB of $0026). Quitting is never run (a hole: it leaves the caller)
static void call_race_pause(const Regs* in, Regs* out) {
  const bool quit = race_pause();
  (void)quit;
  textCentredLineCV(out, (uint16_t)(rd16(0x0233) << 1), 0x4E61);
  out->a = 0x0010;
  setNZ16(out, in->x);
}

// ---- shop_panel_frame ($80:B149) ----  no register outputs (an indirect callback). Text still
// typing: the CMP of D+$8C with D+$8E; typed: N/Z of the INC of D+$4E
static void call_shop_panel_frame(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t a = rd16(DP + 0x8C), b = rd16(DP + 0x8E);
  shop_panel_frame(in->d);
  if(a != b) {
    setNZ16(out, (uint16_t)(a - b));
    out->c = a >= b;
  } else {
    setNZ16(out, rd16(DP + 0x4E));
  }
}

// ---- money_skip_bonus ($8A:C355) ----  $03DC bit 12 clear: its LDA's N, the BIT's Z; set:
// the decimal adds (N/Z of the new $0390, their carry, V of the last), A that word
static uint16_t bcdValue(uint16_t v) {
  return (uint16_t)((v >> 12) * 1000 + ((v >> 8) & 15) * 100 + ((v >> 4) & 15) * 10 + (v & 15));
}
static void call_money_skip_bonus(const Regs* in, Regs* out) {
  const uint16_t flags = rd16(0x03DC), lo = rd16(0x038E), hi = rd16(0x0390);
  bool v = in->v;
  money_skip_bonus(&v);
  if(!(flags & 0x1000)) {
    out->a = flags;
    out->n = (flags & 0x8000) != 0;
    out->z = true;
    return;
  }
  const bool carry = bcdValue(lo) + 5000 >= 10000;
  out->a = rd16(0x0390);
  setNZ16(out, out->a);
  out->c = bcdValue(hi) + 7 + carry >= 10000;
  out->v = v;
}
static uint16_t bcd4(uint32_t (*rnd)(void));
static void fuzz_money_skip_bonus(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x03DC, (uint16_t)rnd());
  wr16ram(ram, 0x038E, bcd4(rnd));
  wr16ram(ram, 0x0390, (rnd() & 3) ? bcd4(rnd) : (uint16_t)(0x9990 + rnd() % 10));
}

// ---- track_snap ($8A:C2DD) ----  A = the new $0444 (N clear, Z of it: its AND #$FF), X =
// its index, C of the CMP that stopped (the track at or above the old $0444); V of the last
// money_skip_bonus add, else as on entry
static void call_track_snap(const Regs* in, Regs* out) {
  uint16_t old = rd16(0x0444);
  bool v = in->v;
  out->x = track_snap(&v);
  out->v = v;
  out->a = rd16(0x0444);
  out->n = false;
  out->z = out->a == 0;
  out->c = out->a >= old;
}
static uint16_t bcd4(uint32_t (*rnd)(void)) {
  uint16_t v = 0;
  for(int shift = 0; shift < 16; shift += 4) v |= (uint16_t)((rnd() % 10) << shift);
  return v;
}
// every variant; $0444 at or between the list's tracks (so it is passed, the search ends
// in the list); the bonus bit; BCD money near a carry
static void fuzz_track_snap(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint16_t dc = (uint16_t)rnd();
  wr16ram(ram, 0x03DC, dc);
  uint32_t list = (dc & 0x0300) == 0x0100 ? 0x8ACADE : (dc & 0x0300) == 0x0200 ? 0x8ACB01 : 0x8ACAC4;
  uint16_t pick = rom8(list + rnd() % 20);
  wr16ram(ram, 0x0444, (uint16_t)(pick - ((rnd() & 1) ? rnd() % 3 : 0)));
  wr16ram(ram, 0x038E, (rnd() & 1) ? (uint16_t)(0x5000 | (bcd4(rnd) & 0x0FFF)) : bcd4(rnd));
  wr16ram(ram, 0x0390, bcd4(rnd));
}

// ---- effect_request_p1 ($81:F48B) ----  $047C bit 15: the BIT's flags (N set, V = bit 14,
// Z of A & $047C), C as on entry. Else N Z of the PLA'd A, and with no cooldown V of the
// BIT, C as on entry; with one, C and V of its SBC (the keep exit is PLA, RTS: no SEC)
static void call_effect_request_p1(const Regs* in, Regs* out) {
  uint16_t flags = rd16(0x047C), cool = rd16(0x0223), step = rd16(0x1E9E);
  effect_request_p1(in->a, in->x, in->y);
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
    out->c = cool >= step;
  }
}
// the request blocked or not, a cooldown running out or not (it never runs in the movies)
static void fuzz_effect_request_p1(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint32_t k = rnd();
  wr16ram(ram, 0x047C, (uint16_t)((k & 3) == 0 ? (rnd() | 0x8000) : (rnd() & 0x7FFF)));
  wr16ram(ram, 0x0223, (k & 4) ? 0 : (uint16_t)rnd());
  wr16ram(ram, 0x1E9E, (uint16_t)rnd());
  r->a = (uint16_t)rnd();
}

// ---- tile_anim_step ($8A:85AF) ----  D+$42 = 0: A = 0 (Z set, N clear). A frame shown: A =
// its last word (N Z of the LDA), X = the frame, C set (the CMP that chose it, or mode > 1).
// Mode 1 without a frame: A = the count, flags of its CMP #5. V as on entry
static void call_tile_anim_step(const Regs* in, Regs* out) {
  (void)in;
  uint16_t mode = rd16(DP + 0x42), f = rd16(DP + 0x40);
  tile_anim_step();
  if(mode == 0) {
    out->a = 0;
    out->n = false;
    out->z = true;
    return;
  }
  uint16_t frame = 0xFFFF;
  if(mode == 1) {
    f = (uint16_t)((f + 1) & 7);
    frame = f == 1 ? 0x005A : f == 3 ? 0x0078 : f == 5 ? 0x003C : 0xFFFF;
    if(frame == 0xFFFF) {
      out->a = f;
      out->c = f >= 5;
      setNZ16(out, (uint16_t)(f - 5));
      return;
    }
  } else {
    f = (uint16_t)((f + 1) & 0x0F);
    frame = (f & 2) ? 0x0000 : 0x001E;
  }
  out->x = frame;
  out->a = rd16(0x8000 + (uint32_t)(uint16_t)(frame + 0x1C));
  setNZ16(out, out->a);
  out->c = true;
}
// every mode and count; the frames' words
static void fuzz_tile_anim_step(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x42, (uint16_t)(rnd() % 4 == 0 ? 0 : rnd() % 4 == 0 ? 1 : 1 + rnd() % 5));
  wr16ram(ram, DP + 0x40, (uint16_t)rnd());
  for(int i = 0; i < 0xA0; i++) ram[0x8000 + i] = (uint8_t)rnd();
}

// ---- track_rows_build ($81:E631) ----  after the MVN: A = $FFFF, X = $D1C3, Y = its end;
// the PLB of the caller's bank sets N Z; C V of the ADC #$D1B3 that found the destination
static void call_track_rows_build(const Regs* in, Regs* out) {
  uint16_t n2 = (uint16_t)(rd16(0x019A) << 1);
  track_rows_build();
  out->a = 0xFFFF;
  out->x = 0xD1C3;
  out->y = (uint16_t)(n2 + 0xD1B3 + 0x10);
  setAdc16(out, n2, 0xD1B3);
  out->n = (in->db & 0x80) != 0;
  out->z = in->db == 0;
}
// rows $10-$3F, the track table in ROM, the limits, edges' objects of the valid kinds (all
// but 9, whose handler is $81:0000), some rows ending on the $F8F3 marker
static void fuzz_track_rows_build(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint16_t n = (uint16_t)(0x10 + rnd() % 0x30);
  wr16ram(ram, 0x019A, n);
  // half the time the first row lands on a $F8F3 marker of the track table (its other
  // limits, $1F2A); the start index is D+$98 - 10 (the first row's multiply by 10 of -1)
  static uint16_t marker = 0;
  if(!marker)
    for(uint32_t a = 0x848000; a < 0x84FFF0 && !marker; a++)
      if(rom16(a + 4) == 0xF8F3) marker = (uint16_t)a;
  wr16ram(ram, DP + 0x98, (rnd() & 1) && marker ? (uint16_t)(marker + 10) : (uint16_t)(0x9000 + (rnd() % 0x400) * 10));
  wr16ram(ram, 0x1F28, (uint16_t)(rnd() % 0x200));
  wr16ram(ram, 0x1F2A, (uint16_t)(rnd() % 0x200));
  // the entries, 4 bytes apart from $7E:8CB3 (x, kind, ...); every kind a valid one
  for(uint32_t i = 0; i < 4u * n + 8; i++) ram[0x8CB3 + i] = (uint8_t)rnd();
  for(uint32_t e = 0; e < 4u * n + 4; e += 4) {
    uint32_t k = rnd() % 4 == 0 ? 1 + rnd() % 63 : 0;
    if(k == 9) k = 10;
    ram[0x8CB4 + e] = (uint8_t)k;
  }
}

// ---- track_block_triple ($82:ADC2) ----  after the last MVN: A = $FFFF, X = $230C, Y =
// $262C; the PLB of the caller's bank sets N Z; C V clear (the ADC of the product to 0)
static void call_track_block_triple(const Regs* in, Regs* out) {
  track_block_triple(in->d);
  out->a = 0xFFFF;
  out->x = 0x230C;
  out->y = 0x262C;
  out->c = out->v = false;
  out->n = (in->db & 0x80) != 0;
  out->z = in->db == 0;
}
// any of the tracks (the product reaches into the unpacked data)
static void fuzz_track_block_triple(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x047B, (uint16_t)(rnd() % 0x40));
}

// ---- track_palettes_load ($82:A672) ----  A = Y = $E0 (the last row step: C V clear),
// X = player 2's last palette pick; the PLB of the $83 its PEA pushed (DB $83, N set, Z
// clear)
static void call_track_palettes_load(const Regs* in, Regs* out) {
  track_palettes_load(in->d);
  out->a = out->y = 0x00E0;
  out->x = (uint16_t)(rd16((uint16_t)(in->d + 0x1C)) - 0x10);
  out->c = out->v = false;
  out->db = 0x83;
  out->n = true;
  out->z = false;
}
// any track; $01E4 negative, 0 or above; either player's flag
static void fuzz_track_palettes_load(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x047B, (uint16_t)(rnd() % 0x40));
  wr16ram(ram, 0x01E4, (rnd() % 3 == 0) ? 0 : (rnd() & 1) ? (uint16_t)(0x8000 | rnd()) : (uint16_t)(1 + rnd() % 8));
  ram[0x039C] = (rnd() & 1) ? (uint8_t)(1 + rnd() % 255) : 0;
  ram[0x03CB] = (rnd() & 1) ? (uint8_t)(1 + rnd() % 255) : 0;
}

// ---- title_zoom_settle / title_stars_spread ($80:F303, $80:F247) ----  as title_zoom_cycle:
// A = func_80F745's result, X = D+$34 (N Z of it), C clear; V through inverse_scale and
// func_80F745 from the V before them (the settle's SBC of the product; the stars' last ADC
// #6, clear)
static void call_title_zoom_settle(const Regs* in, Regs* out) {
  (void)in;
  // the V before inverse_scale: the SBC of 2E00 - p, p from D+$86 after the step
  uint16_t s = (uint16_t)(rd16(DP + 0x86) >> 3);
  if(s >= 0x00D0) s = 0x00D0;
  uint16_t w = (uint16_t)(rd16(DP + 0x86) - (uint16_t)(s + 1));
  if(w & 0x8000) w = 0;
  Regs t;
  setSbc16(&t, 0x2E00, (uint16_t)(0x80 * (uint8_t)(w >> 8)));
  out->a = title_zoom_settle();
  out->x = rd16(DP + 0x34);
  setNZ16(out, out->x);
  out->v = v_after_func_80F745(v_after_inverse_scale(t.v));
  out->c = false;
}
static void call_title_stars_spread(const Regs* in, Regs* out) {
  out->a = title_stars_spread(in->c);
  out->y = 0x0300;  // past the 128 stars, 6 bytes each
  out->x = rd16(DP + 0x34);
  setNZ16(out, out->x);
  out->v = v_after_func_80F745(v_after_inverse_scale(false));
  out->c = false;
}
// title_zoom's state, the scale around $E2 and $100, D+$86 around its floor, the counter
// at its turns
static void fuzz_title_zoom_settle(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_title_zoom(r, ram, rnd);
  wr16ram(ram, DP + 0x7A, (uint16_t)((rnd() & 1) ? 0x00E0 + rnd() % 0x500 : rnd()));
  wr16ram(ram, DP + 0x86, (uint16_t)((rnd() & 1) ? rnd() % 0x200 : rnd()));
  static const uint16_t counts[4] = {0x000F, 0x001F, 0x000E, 0x0003};
  wr16ram(ram, DP + 0x30, counts[rnd() % 4]);
}
// the stars' table (speeds and positions)
static void fuzz_title_stars_spread(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_title_zoom(r, ram, rnd);
  for(int i = 0; i < 0x300; i++) ram[0x0F00 + i] = (uint8_t)rnd();
}

// ---- stars_draw / stars_draw_8a ($80:F950, $8A:9773) ----  A = Y = $300, X = the next OAM
// slot (C Z set by the CPY #$300, N clear, V of its ADC #6: clear)
static void starsDrawOut(const Regs* in, Regs* out, void (*fn)(uint16_t, uint16_t)) {
  fn(in->a, in->x);
  out->a = out->y = 0x0300;
  out->x = rd16(DP + 0x74);
  out->n = out->v = false;
  out->z = out->c = true;
}
static void call_stars_draw(const Regs* in, Regs* out) { starsDrawOut(in, out, stars_draw); }
static void call_stars_draw_8a(const Regs* in, Regs* out) { starsDrawOut(in, out, stars_draw_8a); }
// the stars (speeds from $40, some 0: the divider's $FFFF), a step that cannot make a
// spawned star too near forever, a small multiplier (a slow star under a large one is
// spawned again dozens of times, past the bench's instruction limit), the OAM slot
static void fuzz_stars_draw(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_rng_state(ram, rnd);
  for(uint32_t y = 0; y < 0x300; y += 6) {
    for(int k = 0; k < 5; k++) ram[0x0F00 + y + (uint32_t)k] = (uint8_t)rnd();
    ram[0x0F05 + y] = (uint8_t)(rnd() % 16 == 0 ? 0 : 0x40 + rnd() % 0xC0);
  }
  r->a = (uint16_t)(rnd() % 0x1000);
  r->x = (uint16_t)((rnd() & 0xFF00) | (rnd() % 0x40));
  wr16ram(ram, DP + 0x74, (uint16_t)(4 * (rnd() % 16)));
  wr16ram(ram, DP + 0x80, (uint16_t)rnd());
}

// ---- window_curve_build ($80:FA5B) ----  A = $DD under the high byte of the last word
// copied ($1503), X = 0, Y = $170, N set Z clear (LDA #$DD); C = the position past $8000
// (the CMP that ended the lines, or clear when they ran out first); V of the last add
static void call_window_curve_build(const Regs* in, Regs* out) {
  (void)in;
  window_curve_build();
  const uint16_t p = rd16(DP + 0x14), q = rd16(DP + 0x17), prev = (uint16_t)(p - q);
  out->a = (uint16_t)(rd8(0x1504) << 8 | 0xDD);
  out->x = 0;
  out->y = 0x0170;
  out->n = true;
  out->z = false;
  out->c = p >= 0x8000;
  out->v = ((prev ^ p) & (q ^ p) & 0x8000) != 0;
}
// any r (0: the divider's $FFFF; negative: a signed product), small ones often
static void fuzz_window_curve_build(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  ram[DP + 0x42] = (uint8_t)((rnd() & 1) ? rnd() % 0x10 : rnd());
}

// ---- palette_fade_60 ($80:FAEE) ----  A = $0001 (LDA #$01 under the size's 0: N Z clear),
// X = $20, Y = 0; C V of the last blue: V of its SBC, C the bit its second ASL shifted out
static void call_palette_fade_60(const Regs* in, Regs* out) {
  palette_fade_60(in->a);
  const uint16_t s = rd16(DP + 0x14), b = bus_rd16(0x81DB8C + 0x1E);
  Regs t;
  setSbc16(&t, b, s);
  const uint16_t v = b >= s ? (uint16_t)(b - s) : 0;
  out->a = 0x0001;
  out->x = 0x0020;
  out->y = 0;
  out->n = out->z = false;
  out->v = t.v;
  out->c = ((uint16_t)(v << 8 | v >> 8) & 0x4000) != 0;
}
// any of the table's 80 steps
static void fuzz_palette_fade_60(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)ram;
  r->a = (uint16_t)(rnd() % 0x50);
}

// ---- stars_scatter_8a ($8A:971B) ----  A = X = $300, Y kept; the CPX that ends the loop
// (Z C set, N clear), V of its ADC #6 (clear)
static void call_stars_scatter_8a(const Regs* in, Regs* out) {
  stars_scatter_8a();
  out->a = out->x = 0x0300;
  out->y = in->y;
  out->n = out->v = false;
  out->z = out->c = true;
}
static void fuzz_stars_scatter_8a(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  fuzz_rng_state(ram, rnd);
}

// ---- hdma_coldata_setup ($8A:9C69) ----  A = $0D00, X = $81 (LDX #$81 with X 8-bit: N
// set Z clear), Y = 0 from hdma_table_0d00_init; C V kept
static void call_hdma_coldata_setup(const Regs* in, Regs* out) {
  hdma_coldata_setup();
  out->a = 0x0D00;
  out->x = 0x0081;
  out->y = 0;
  out->n = true;
  out->z = false;
  out->c = in->c;
  out->v = in->v;
}

// ---- hdma_title_setup ($8A:8787) ----  A = $0DA1 (LDA #$A1 under the table's $0D: N set Z
// clear), X Y past the second MVN ($891C, $0D2D); C V kept; DB restored by its PLB
static void call_hdma_title_setup(const Regs* in, Regs* out) {
  hdma_title_setup();
  out->a = 0x0DA1;
  out->x = 0x891C;
  out->y = 0x0D2D;
  out->n = true;
  out->z = false;
  out->c = in->c;
  out->v = in->v;
}
// the two tables' old bytes
static void fuzz_hdma_title_setup(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(uint32_t a = 0x0D00; a < 0x0D30; a++) ram[a] = (uint8_t)rnd();
}

// ---- palette_load_70 ($8A:8DBF) ----  as upload_cgram_now (A = $7001), X kept (PHX/PLX:
// N Z of it), Y = $20; C V kept
static void call_palette_load_70(const Regs* in, Regs* out) {
  palette_load_70(in->x);
  out->a = 0x7001;
  out->x = in->x;
  out->y = 0x0020;
  setNZ16(out, in->x);
  out->c = in->c;
  out->v = in->v;
}
// any of the table's first 32 picks
static void fuzz_palette_load_70(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)ram;
  r->x = (uint16_t)(2 * (rnd() % 32));
}

// ---- text_screen_init ($8A:C1C6) ----  A = the last tile masked, X = $180, Y = 0 (the
// DEY that ended the mask: Z set, N clear); C V from text_print_centered (D+$86 = 1: the
// ADC of its left edge)
static void call_text_screen_init(const Regs* in, Regs* out) {
  text_screen_init(in->x);
  out->a = rd16(0x1017E);
  out->x = 0x0180;
  out->y = 0;
  out->n = false;
  out->z = true;
  const uint16_t half = (uint16_t)((rd16(DP + 0x40) - rd16(DP + 0x3E)) >> 1);
  if(rd16(DP + 0x86)) {
    Regs t;
    setAdc16(&t, (uint16_t)(half - rd16(DP + 0x46)), rd16(DP + 0x3E));
    out->c = t.c;
    out->v = t.v;
  } else {
    Regs t;
    setSbc16(&t, rd16(DP + 0x3C), 10);
    out->c = t.c;
    out->v = t.v;
  }
}
// text_print_centered's string (its address in X)
static void fuzz_text_print_centered(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
static void fuzz_text_screen_init(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_text_print_centered(r, ram, rnd);
  r->x = r->a;
}

// ---- title_mode7_setup ($80:F424) ----  as title_zoom_cycle: A = func_80F745's result,
// X = D+$34 (N Z of it), Y kept, C clear; V through inverse_scale and func_80F745 from the
// entry's (nothing before them sets it)
static void call_title_mode7_setup(const Regs* in, Regs* out) {
  out->a = title_mode7_setup();
  out->x = rd16(DP + 0x34);
  out->y = in->y;
  setNZ16(out, out->x);
  out->v = v_after_func_80F745(v_after_inverse_scale(in->v));
  out->c = false;
}

// ---- hdma_window_setup ($8A:A06D) ----  A = $0D20, X = $81 (LDX #$81 with X 8-bit: N set,
// Z clear), Y's low byte (SEP #$10 clears the high bytes); C V kept
static void call_hdma_window_setup(const Regs* in, Regs* out) {
  hdma_window_setup();
  out->a = 0x0D20;
  out->x = 0x0081;
  out->y = in->y & 0x00FF;
  out->n = true;
  out->z = false;
  out->c = in->c;
  out->v = in->v;
}

// ---- text_tiles_put_0 ($8A:BACB) ----  A = the last word, X = $FFFF and Y = $C2 (the DEX
// that ended it: N set, Z clear), C V kept
static void call_text_tiles_put_0(const Regs* in, Regs* out) {
  text_tiles_put_0();
  out->a = bus_rd16(0x8AC8E4 + 0xC0);
  out->x = 0xFFFF;
  out->y = 0x00C2;
  out->n = true;
  out->z = false;
  out->c = in->c;
  out->v = in->v;
}

// ---- text_palettes_load ($8A:C24B) ----  as upload_cgram_now: A = $6001, X = $2202, Y =
// $40, N Z clear; C V kept
static void call_text_palettes_load(const Regs* in, Regs* out) {
  text_palettes_load();
  out->a = 0x6001;
  out->x = 0x2202;
  out->y = 0x0040;
  out->n = out->z = false;
  out->c = in->c;
  out->v = in->v;
}

// ---- car_pair_contact ($81:F252) ----  No contact: A = the value tested, X Y kept, C clear;
// N Z of the BIT's ORA (Z clear) or of the CMP that ended it, V kept or of that word's SBC.
// Contact: X Y kept (PLY/PLX: N Z of X). Pushed beside: A = x's new +$06, C clear, V of
// its ADC/SBC #$40 or of the $029C add. Swapped: C set, A = $FF24 when effect_request_p1
// ran (V of its BIT or of its cooldown SBC) or 4; else V of the last 32-bit SBC (the
// drop from $0225, or the drop itself).
static bool sbcV(uint16_t a, uint16_t b, bool c) {
  const uint16_t d = (uint16_t)(a - b - (c ? 0 : 1));
  return ((a ^ b) & (a ^ d) & 0x8000) != 0;
}
static bool adcV(uint16_t a, uint16_t b) {
  const uint16_t s = (uint16_t)(a + b);
  return (~(a ^ b) & (a ^ s) & 0x8000) != 0;
}
// When set, car_pair_contact's adapter runs this at its contact call instead of the routine
// (a caller whose exit flags are that call's: see car_contacts_chain).
static void (*carContactRun)(void) = NULL;
static void call_car_pair_contact(const Regs* in, Regs* out) {
  const uint16_t x = in->x, y = in->y;
  out->x = x;
  out->y = y;
  out->c = false;
  const uint16_t o = (uint16_t)(rd16(0xFADE + (uint32_t)y) | rd16(0xFADE + (uint32_t)x));
  if(o & 0x0088) {
    car_pair_contact(x, y);
    out->a = o;
    out->n = (o & 0x8000) != 0;
    out->z = false;
    out->v = in->v;
    return;
  }
  static const uint16_t gate[2][2] = {{0xFABF, 0x0018}, {0xFABB, 0x0040}};
  for(int k = 0; k < 2; k++) {
    const uint16_t at = gate[k][0], lim = gate[k][1];
    const uint16_t a = rd16(at + (uint32_t)y), b = rd16(at + (uint32_t)x), d = (uint16_t)(a - b);
    const uint16_t t = (d & 0x8000) ? (uint16_t)(d - (uint16_t)-lim) : (uint16_t)(d - lim);
    if((d & 0x8000) ? (t & 0x8000) != 0 : !(t & 0x8000)) {
      car_pair_contact(x, y);
      out->a = d;
      out->n = (t & 0x8000) != 0;
      out->z = t == 0;
      out->v = sbcV(a, b, true);
      return;
    }
  }
  // contact: what the flags need, before the C changes it
  const uint16_t mode = rd16(0x03DC) & 0x0003;
  const uint16_t sx = mode ? bus_rd16(0x81FE16 + (uint32_t)(uint16_t)((x / 0x35) << 1)) : (x ? 0xFFFF : 0);
  const uint32_t gap = (rd16(0xFAC1 + (uint32_t)y) | (uint32_t)rd16(0xFAC3 + (uint32_t)y) << 16) -
                       (rd16(0xFAC1 + (uint32_t)x) | (uint32_t)rd16(0xFAC3 + (uint32_t)x) << 16);
  const uint16_t fy = rd16(0xFABB + (uint32_t)y);
  const bool below = ((uint16_t)(rd16(0xFAD5 + (uint32_t)y) - rd16(0xFAD5 + (uint32_t)x)) & 0x8000) != 0;
  const uint16_t was5 = rd16(0xFAB5 + (uint32_t)x), was7 = rd16(0xFAB7 + (uint32_t)x);
  const uint16_t flags = rd16(0x047C), cool = rd16(0x0223), step = rd16(0x1E9E);
  uint16_t old225 = 0, old227 = 0;
  if(!(sx & 0x8000)) {
    old225 = rd16(0x0225 + (uint32_t)sx);
    old227 = rd16(0x0227 + (uint32_t)sx);
  }
  if(carContactRun) carContactRun();
  else car_pair_contact(x, y);
  setNZ16(out, x);
  if((gap - 0x60) & 0x80000000u) {
    const uint16_t side = (uint16_t)(fy + (below ? 0x0040 : (uint16_t)-0x0040));
    out->a = side;
    out->v = below ? adcV(fy, 0x0040) : sbcV(fy, 0x0040, true);
    if(!(sx & 0x8000) && !(rd16(0x0244 + (uint32_t)sx) & 0x0004)) out->v = adcV(side, rd16(0x029C + (uint32_t)sx));
    return;
  }
  out->c = true;
  const uint16_t now5 = rd16(0xFAB5 + (uint32_t)x), now7 = rd16(0xFAB7 + (uint32_t)x);
  out->v = sbcV(was7, now7, was5 >= now5);
  const uint16_t d1c = rd16(DP + 0x1C), d1e = rd16(DP + 0x1E);
  if(!(sx & 0x8000) && !(rd16(0x0244 + (uint32_t)sx) & 0x0001)) out->v = sbcV(old227, d1e, old225 >= d1c);
  const uint16_t sy = rd16(DP + 0x1A);
  uint16_t m = 0x0004;
  if(!(sx & 0x8000)) m &= rd16(0x0244 + (uint32_t)sx);
  if(!(sy & 0x8000)) m &= rd16(0x0244 + (uint32_t)sy);
  if(m & 0x0004) {
    out->a = 0x0004;
    return;
  }
  out->a = 0xFF24;
  if(!(flags & 0x8000) && cool != 0) out->v = sbcV(cool, step, true);
  else out->v = (flags & 0x4000) != 0;
}
// two distinct objects (stride $35) close enough to reach every exit: flags, the +$0A and
// +$06 gates, the $60 gap; the two-player mode's slot table now and then; the cars' slots
// and their flags, divisors, the effect's flags and cooldown
static void fuzz_car_pair_contact(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  const uint16_t x = (uint16_t)(0x35 * (rnd() % 4 == 0 ? 0 : rnd() % 20));
  uint16_t y;
  do y = (uint16_t)(0x35 * (rnd() % 20)); while(y == x);
  r->x = x;
  r->y = y;
  for(uint32_t i = 0; i < 0x35; i++) {
    ram[0xFAB5 + x + i] = (uint8_t)rnd();
    ram[0xFAB5 + y + i] = (uint8_t)rnd();
  }
  wr16ram(ram, 0xFADE + x, (uint16_t)(rnd() % 8 == 0 ? rnd() : rnd() & 0xFF77));
  wr16ram(ram, 0xFADE + y, (uint16_t)(rnd() % 8 == 0 ? rnd() : rnd() & 0xFF77));
  const uint16_t bx = (uint16_t)rnd(), cx = (uint16_t)rnd();
  wr16ram(ram, 0xFABF + x, bx);
  wr16ram(ram, 0xFABF + y, (uint16_t)(bx + (rnd() % 0x40) - 0x20));
  wr16ram(ram, 0xFABB + x, cx);
  wr16ram(ram, 0xFABB + y, (uint16_t)(cx + (rnd() % 0x90) - 0x48));
  const uint32_t px = rnd();
  const uint32_t py = px + (uint32_t)(rnd() % 0x100) - 0x40;
  wr16ram(ram, 0xFAC1 + x, (uint16_t)px);
  wr16ram(ram, 0xFAC3 + x, (uint16_t)(px >> 16));
  wr16ram(ram, 0xFAC1 + y, (uint16_t)py);
  wr16ram(ram, 0xFAC3 + y, (uint16_t)(py >> 16));
  // y's +$00 near x's +$0C, so the drop sometimes reaches below x's floor
  const uint32_t ay = px + (uint32_t)(rnd() % 0x100);
  wr16ram(ram, 0xFAB5 + y, (uint16_t)ay);
  wr16ram(ram, 0xFAB7 + y, (uint16_t)(ay >> 16));
  wr16ram(ram, 0x03DC, (uint16_t)(rnd() % 4 == 0 ? 1 + rnd() % 3 : 0));
  for(uint32_t i = 0x0200; i < 0x0300; i++) ram[i] = (uint8_t)rnd();
  wr16ram(ram, 0x0223, (rnd() & 1) ? 0 : (uint16_t)rnd());
}

// ---- track_object_test_sides ($83:FC65) ----  as track_object_test: no register outputs,
// the exit flags those of the last test made (the first hit, or the fourth). A miss writes
// nothing, so the adapter finds that test from func_83FD8A's conditions and hands its
// adapter the state at that call (A = $0128, X = the jump index, Y = $011A).
static bool fd8aHits(uint16_t dp, uint16_t a) {
  if(rd16(0x0120) & 0x8000) return false;
  const uint16_t kind = rd16(0x011C), d = (uint16_t)(a - rd16((uint16_t)(dp + 0x06)));
  if(!(d & 0x8000) ? !((uint16_t)(d - bus_rd16(0x8380CE + kind)) & 0x8000)
                   : ((uint16_t)(d - bus_rd16(0x8380CC + kind)) & 0x8000) != 0)
    return false;
  const uint16_t e = (uint16_t)(rd16(0x012A) - bus_rd16(0x81012C));
  return !(e & 0x8000) ? ((uint16_t)(e - bus_rd16(0x83826E + kind)) & 0x8000) != 0
                       : !((uint16_t)(e - bus_rd16(0x83826C + kind)) & 0x8000);
}
static bool run_track_object_test_sides(uint16_t dp) {
  wr16(0x012A, heldPos);
  wr16(0x0128, heldAt);
  return track_object_test_sides(dp);
}
static void call_track_object_test_sides(const Regs* in, Regs* out) {
  const uint16_t x = objectX(), obj = rd16(0x011A);
  const uint16_t sel = (uint16_t)((rd8(0x8CB5 + (uint32_t)obj) & 0x30) >> 4);
  heldPos = rd16(0x012A);
  heldAt = rd16(0x0128);
  wr16(0x012A, 0);
  const uint16_t nx = (uint16_t)-x;
  uint16_t at[4] = {x, nx, (uint16_t)(nx - 0x80), (uint16_t)(x + 0x80)};
  bool fd[4] = {true, true, true, true};
  if(sel == 1) {
    at[1] = x;
    at[2] = at[3] = nx;
    fd[1] = fd[3] = false;
  } else if(sel == 2) {
    fd[0] = fd[1] = fd[2] = fd[3] = false;
  }
  int last = 3;
  for(int i = 0; i < 4; i++)
    if(fd[i] && fd8aHits(in->d, at[i])) {
      last = i;
      break;
    }
  wr16(0x0128, at[last]);
  Regs t = *in;
  t.a = at[last];
  t.x = (uint16_t)(sel << 1);
  t.y = obj;
  calleeRun = run_track_object_test_sides;
  if(fd[last]) call_func_83FD8A(&t, out);
  else call_func_83FDCD(&t, out);
  calleeRun = NULL;
}
// track_object_test's state, with any jump index; x near the limits on either side
static void fuzz_track_object_test_sides(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_track_object(r, ram, rnd);
  const uint32_t obj = (uint32_t)(ram[0x011A] | ram[0x011B] << 8);
  ram[0x8CB5 + obj] = (uint8_t)((ram[0x8CB5 + obj] & 0xCF) | (rnd() & 0x30));
}

// ---- ticker_draw ($80:EBF7) ----  A = the text's bank (its DB, put back by PLB), X = the
// x, Y = the text. Both callers reload every register and set every flag they read after
// it: no outputs (the bench judges the flags from the watched returns).
static void call_ticker_draw(const Regs* in, Regs* out) {
  (void)out;
  ticker_draw(in->d, (uint8_t)in->a, in->x, in->y);
}
// a text in low WRAM (bank $7F's mirror is not used: bank $81's is): capitals, digits,
// spaces, a few marks, codes 1, 2 (with a count), 3 and 5, ending in code 0; the queue,
// its counters and the end marker's countdown
static void fuzz_ticker_draw(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->a = 0x0081;
  r->x = (uint16_t)(rnd() % 0x100);
  r->y = 0x1C00;
  uint32_t at = 0x1C00;
  const int n = (int)(rnd() % 24);
  for(int i = 0; i < n; i++) {
    const uint32_t k = rnd() % 16;
    if(k < 6) ram[at++] = (uint8_t)(0x41 + rnd() % 26);
    else if(k < 9) ram[at++] = (uint8_t)(0x30 + rnd() % 10);
    else if(k < 11) ram[at++] = 0x20;
    else if(k == 11) ram[at++] = (uint8_t)"!?.,-'"[rnd() % 6];
    else if(k == 12) {
      ram[at++] = 0;
      ram[at++] = 1;
      ram[at++] = (uint8_t)rnd();
      ram[at++] = 0;
    } else if(k == 13) {
      ram[at++] = 0;
      ram[at++] = 2;
      ram[at++] = (uint8_t)(1 + rnd() % 3);
      ram[at++] = 0;
    } else if(k == 14) {
      ram[at++] = 0;
      ram[at++] = (rnd() & 1) ? 3 : 5;
    } else ram[at++] = (uint8_t)(0x41 + rnd() % 26);
  }
  ram[at++] = 0;
  ram[at++] = 0;
  const uint16_t q = (uint16_t)(2 * (rnd() % 0x40));
  wr16ram(ram, DP + 0x5E, q);
  for(uint32_t i = 0; i < 0x40; i += 2)
    wr16ram(ram, 0x10000 + q + i, (rnd() % 3 == 0) ? 0xFFFF : (uint16_t)(rnd() % 0x20));
  wr16ram(ram, DP + 0xC6, (uint16_t)(rnd() % 0x300));
  wr16ram(ram, DP + 0xE5, (uint16_t)(rnd() % 0x180));
  wr16ram(ram, DP + 0xCC, (uint16_t)(rnd() % 0x14));
  wr16ram(ram, DP + 0xD1, (uint16_t)(rnd() % 3));
  wr16ram(ram, DP + 0x74, (uint16_t)(4 * (rnd() % 16)));
  const uint32_t e = rnd() % 4;
  wr16ram(ram, DP + 0xE1, e == 0 ? (uint16_t)(0x8000 | rnd()) : (uint16_t)(rnd() % 0x3C));
}

// ---- race_hud_sprites ($80:D823) ----  both callers go on to oam_finish_0b00_near, which sets
// A and X before reading them and never reads Y: no register outputs (the flags judged by
// the bench from the watched returns)
static void call_race_hud_sprites(const Regs* in, Regs* out) {
  (void)in;
  (void)out;
  race_hud_sprites(in->d);
}
// the window shapes' and the lights' state; the speeds, readouts and the camera around the
// screen; names in range; the $0700 list (a few entries, then 0) and the $0F00 table
static void fuzz_race_hud_sprites(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_window_shape(r, ram, rnd);
  fuzz_race_lights_draw(r, ram, rnd);
  // the slot ($0174 is D+$74): small, or near the end so the OAM loops stop early
  wr16ram(ram, DP + 0x74, (uint16_t)(4 * ((rnd() & 1) ? rnd() % 0x10 : rnd() % 0x84)));
  wr16ram(ram, DP + 0x22, (uint16_t)((rnd() % 0x140) - 0x20));
  wr16ram(ram, DP + 0x28, (uint16_t)(rnd() % 0x400));
  wr16ram(ram, DP + 0x32, (uint16_t)(rnd() % 0x400));
  wr16ram(ram, 0x0444, (uint16_t)(rnd() % 8));
  const uint32_t n = 1 + rnd() % 4;
  for(uint32_t i = 0; i < 10 * (n + 1); i += 2) ram[0x0700 + i] = ram[0x0701 + i] = 0;
  for(uint32_t i = 0; i < n; i++) {
    const uint32_t e = 0x0700 + 10 * i;
    wr16ram(ram, e, (uint16_t)(1 + rnd() % 0x100));
    wr16ram(ram, e + 4, (rnd() & 1) ? 0xC272 : 0xC2F2);
    wr16ram(ram, e + 6, (uint16_t)(rnd() % 0x180));
    wr16ram(ram, e + 8, (uint16_t)(rnd() % 0x1000));
  }
  if(rnd() & 1) wr16ram(ram, 0x0700, 0);
  for(uint32_t i = 0; i < 0x200; i++) ram[0x0F00 + i] = (uint8_t)rnd();
}

// ---- text_buffer_screen ($8A:C15D) ----  it ends in text_screen_init: its exit registers and
// flags, which that adapter reads from the state after it (the same here)
static void call_text_buffer_screen(const Regs* in, Regs* out) {
  (void)in;
  text_buffer_screen();
  out->a = rd16(0x1017E);
  out->x = 0x0180;
  out->y = 0;
  out->n = false;
  out->z = true;
  const uint16_t half = (uint16_t)((rd16(DP + 0x40) - rd16(DP + 0x3E)) >> 1);
  Regs t;
  if(rd16(DP + 0x86)) setAdc16(&t, (uint16_t)(half - rd16(DP + 0x46)), rd16(DP + 0x3E));
  else setSbc16(&t, rd16(DP + 0x3C), 10);
  out->c = t.c;
  out->v = t.v;
}
// text_print_centered's state; a buffer of a few rows of printable bytes, $80 between rows
// and an end; the cursor; $1B37's bit 15 now and then
static void fuzz_text_buffer_screen(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_text_print_centered(r, ram, rnd);
  uint32_t at = 0x17D00;
  const uint32_t rows = rnd() % 8;
  for(uint32_t i = 0; i < rows; i++) {
    const uint32_t n = rnd() % 16;
    for(uint32_t k = 0; k < n; k++) ram[at++] = (uint8_t)(0x20 + rnd() % 0x3B);
    ram[at++] = 0x80;
  }
  ram[at] = 0;
  wr16ram(ram, DP + 0x3A, (uint16_t)(rnd() % 16));
  wr16ram(ram, DP + 0x3C, (uint16_t)(rnd() % 12));
  wr16ram(ram, DP + 0x90, (uint16_t)(rnd() & 1));
  wr16ram(ram, 0x1B37, (rnd() % 4 == 0) ? (uint16_t)(0x8000 | rnd()) : (uint16_t)(rnd() & 0x7FFF));
}

// ---- scenery_build ($80:DCBB) ----  its caller goes on to star_layers_init, which sets
// every register: no register outputs. Five entries, so every flag is modelled: N Z of the
// PLY of the record that ended it; at a 0 kind C clear and V of the ADC #$0E that reached
// it (or as on entry for the first record); at an $8000 kind C = the bit ASL D+$88 shifted
// out and V of the last kind-x add. The adapter walks the records from the ROM.
static bool sceneryAdcV(uint16_t a, uint16_t b) {
  const uint16_t s = (uint16_t)(a + b);
  return (~(a ^ b) & (a ^ s) & 0x8000) != 0;
}
static void call_scenery_build(const Regs* in, Regs* out) {
  uint16_t y = bus_rd16(0x81C3D0 + (uint32_t)(uint16_t)(bus_rd8(0x84A240 + (uint32_t)rd16(0x0444)) << 1));
  const uint16_t mask = rd16(0x036A);
  uint16_t bit = 1;
  bool c = false, v = in->v;
  for(;;) {
    const uint16_t kind = bus_rd16(0x81C3E8 + (uint32_t)y);
    if(kind == 0) break;
    if(kind & 0x8000) {
      const uint16_t col8 = (uint16_t)(bus_rd16(0x81C3EA + (uint32_t)y) << 3);
      v = sceneryAdcV(col8, bus_rd16(0x81CAE2 + (uint32_t)(kind & 0x7FFF)));
      c = (bit & 0x8000) != 0;
      bit = (uint16_t)(bit << 1);
      (void)mask;
      if(!(kind & 0x7FFF)) break;
    }
    c = false;
    v = sceneryAdcV(y, 0x000E);
    y = (uint16_t)(y + 0x0E);
  }
  scenery_build(in->d);
  out->c = c;
  out->v = v;
  setNZ16(out, y);
}
// any of the first tracks, or now and then $0444 = 225: its byte at $84:A240 is 41, whose
// list is one kind-2 record at column 6 (the never-run map row below $20); which scenery
// bits are hidden; the source tiles in $7F
static void fuzz_scenery_build(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x0444, (uint16_t)(rnd() % 8 == 0 ? 225 : rnd() % 8));
  wr16ram(ram, 0x036A, (uint16_t)rnd());
  for(uint32_t i = 0x1BC50; i < 0x1BD10; i++) ram[i] = (uint8_t)rnd();
}

// ---- car_contacts_chain ($81:F21E) ----  y $60 or more ahead: A = the difference, C set,
// N Z of the CMP #$60, V of its SBC, X Y kept. Else D+$3C's counted-up value in A (PLA), X
// Y kept (PLY/PLX: N Z of X); on a swap C set and V of car_pair_contact (its adapter, at
// that call, through carContactRun; the walk finds that call from its gates, a miss writing
// nothing and a push aside only x's +$06); else C clear and V of the last chain step's SBC.
static bool chainGatesPass(uint16_t x, uint16_t y) {
  if((rd16(0xFADE + (uint32_t)y) | rd16(0xFADE + (uint32_t)x)) & 0x0088) return false;
  static const uint16_t gate[2][2] = {{0xFABF, 0x0018}, {0xFABB, 0x0040}};
  for(int k = 0; k < 2; k++) {
    const uint16_t d = (uint16_t)(rd16(gate[k][0] + (uint32_t)y) - rd16(gate[k][0] + (uint32_t)x));
    const uint16_t t = (d & 0x8000) ? (uint16_t)(d - (uint16_t)-gate[k][1]) : (uint16_t)(d - gate[k][1]);
    if((d & 0x8000) ? (t & 0x8000) != 0 : !(t & 0x8000)) return false;
  }
  return true;
}
static uint16_t chainX, chainY, chainBB;
static void run_car_contacts_chain(void) {
  wr16(0xFABB + (uint32_t)chainX, chainBB);
  car_contacts_chain(chainX, chainY);
}
static void call_car_contacts_chain(const Regs* in, Regs* out) {
  const uint16_t x = in->x, y = in->y;
  out->x = x;
  out->y = y;
  const uint16_t a0 = rd16(0xFAB5 + (uint32_t)y), b0 = rd16(0xFAB5 + (uint32_t)x), d0 = (uint16_t)(a0 - b0);
  if(d0 >= 0x0060) {
    car_contacts_chain(x, y);
    out->a = d0;
    setNZ16(out, (uint16_t)(d0 - 0x0060));
    out->c = true;
    out->v = sbcV(a0, b0, true);
    return;
  }
  // walk the chain as the routine will, to find the call that swaps (a push aside writes
  // x's +$06, kept here, and goes on with the carry clear)
  uint16_t n = (uint16_t)(rd16(DP + 0x3C) + 1), at = y, hitAt = 0, xbb = rd16(0xFABB + (uint32_t)x);
  const uint16_t realBB = xbb;
  bool hit = false, v = false;
  for(;;) {
    wr16(0xFABB + (uint32_t)x, xbb);
    const bool pass = chainGatesPass(x, at);
    wr16(0xFABB + (uint32_t)x, realBB);
    if(pass) {
      const uint32_t gap = (rd16(0xFAC1 + (uint32_t)at) | (uint32_t)rd16(0xFAC3 + (uint32_t)at) << 16) -
                           (rd16(0xFAC1 + (uint32_t)x) | (uint32_t)rd16(0xFAC3 + (uint32_t)x) << 16);
      if(!((gap - 0x60) & 0x80000000u)) {
        hit = true;
        hitAt = at;
        break;
      }
      const bool below = ((uint16_t)(rd16(0xFAD5 + (uint32_t)at) - rd16(0xFAD5 + (uint32_t)x)) & 0x8000) != 0;
      xbb = (uint16_t)(rd16(0xFABB + (uint32_t)at) + (below ? 0x0040 : (uint16_t)-0x0040));
    }
    at = rd16(0xFACD + (uint32_t)at);
    const uint16_t a = rd16(0xFAB5 + (uint32_t)at);
    v = sbcV(a, b0, true);
    if((uint16_t)(a - b0) >= 0x0060) break;
    n = (uint16_t)(n - 1);
    if(n == 0) break;
  }
  const uint16_t count = (uint16_t)(rd16(DP + 0x3C) + 1);
  if(hit) {
    Regs t = *in;
    t.y = hitAt;
    chainX = x;
    chainY = y;
    chainBB = realBB;
    wr16(0xFABB + (uint32_t)x, xbb);  // x's +$06 as that call sees it; put back before the C
    carContactRun = run_car_contacts_chain;
    Regs o = *in;
    call_car_pair_contact(&t, &o);
    carContactRun = NULL;
    out->v = o.v;
    out->c = true;
  } else {
    car_contacts_chain(x, y);
    out->v = v;
    out->c = false;
  }
  out->a = count;
  setNZ16(out, x);
}
// car_pair_contact's state for x and a first y, a chain of links among objects close
// behind and ahead, the count
static void fuzz_car_contacts_chain(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_car_pair_contact(r, ram, rnd);
  const uint16_t x = r->x;
  const uint16_t base = (uint16_t)(ram[0xFAB5 + x] | ram[0xFAB6 + x] << 8);
  for(uint32_t i = 0; i < 20; i++) {
    const uint32_t o = 0x35 * i;
    if(o == x || o == r->y) continue;
    for(uint32_t k = 0; k < 0x35; k++) ram[0xFAB5 + o + k] = (uint8_t)rnd();
    wr16ram(ram, 0xFAB5 + o, (uint16_t)(base + rnd() % 0x80));
    wr16ram(ram, 0xFABF + o, (uint16_t)(ram[0xFABF + x] | ram[0xFAC0 + x] << 8) + (uint16_t)(rnd() % 0x40) - 0x20);
    wr16ram(ram, 0xFABB + o, (uint16_t)(ram[0xFABB + x] | ram[0xFABC + x] << 8) + (uint16_t)(rnd() % 0x90) - 0x48);
    wr16ram(ram, 0xFADE + o, (uint16_t)(rnd() % 4 == 0 ? 0x0088 : rnd() & 0xFF77));
  }
  for(uint32_t i = 0; i < 20; i++) wr16ram(ram, 0xFACD + 0x35 * i, (uint16_t)(0x35 * (rnd() % 20)));
  wr16ram(ram, 0xFAB5 + r->y, (uint16_t)(base + rnd() % 0x70));
  wr16ram(ram, DP + 0x3C, (uint16_t)(rnd() % 5));
}

// ---- ticker_scene_step ($80:E925) ----  both callers load A and test it with BIT #, then
// wait_frames: no register outputs. C is read after it: with D+$46 negative at the end (as
// the steps leave it, simulated here), that of its CMP #$FFFF; else oam_finish_near's (its
// SBC #$0900 of the old D+$6E, which is D+$6C after it).
static void call_ticker_scene_step(const Regs* in, Regs* out) {
  (void)in;
  out->c = ticker_scene_step(in->d);
}
// the sparks', the sprites' and the ticker's state; a valid line at $7F:2000 + D+$5E; the
// camera and view words; D+$46 counting either way (and $FFFF), D+$48/$4A keeping the list
// index within its five lines; the car's $03A8
static void fuzz_func_80EB66(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
static void fuzz_oam_finish(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
static void fuzz_ticker_scene_step(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_oam_finish(r, ram, rnd);  // the OAM buffers ($6C, $6E): the fuzz starts from a race's RAM
  fuzz_spark_step(r, ram, rnd);
  fuzz_func_80EB66(r, ram, rnd);
  // D+$5E is the $7F double buffer (0 or $8000) of both the sparks and the ticker: the
  // ticker's queue and line go to the sparks' buffer
  const uint32_t buffer = (uint32_t)(ram[DP + 0x5E] | ram[DP + 0x5F] << 8);
  fuzz_ticker_draw(r, ram, rnd);
  const uint32_t q = (uint32_t)(ram[DP + 0x5E] | ram[DP + 0x5F] << 8);
  for(uint32_t i = 0; i < 0x40; i++) ram[0x10000 + buffer + i] = ram[0x10000 + q + i];
  wr16ram(ram, DP + 0x5E, (uint16_t)buffer);
  for(uint32_t i = 0; i < 0x80; i++) ram[0x12000 + buffer + i] = ram[0x1C00 + i];
  static const uint16_t words[] = {0x80, 0x82, 0x7B, 0x7D, 0x7E, 0x84, 0x22, 0x24, 0xE8, 0x2C, 0xE7};
  for(unsigned i = 0; i < sizeof words / sizeof words[0]; i++) wr16ram(ram, DP + words[i], (uint16_t)rnd());
  const uint32_t d = rnd() % 4;
  wr16ram(ram, DP + 0x82, (uint16_t)(d == 0 ? 0x8000 | rnd() : d == 1 ? 0xFFE8 + rnd() % 0x18 : rnd() % 0x100));
  wr16ram(ram, DP + 0xE7, (uint16_t)(rnd() % 0x100));
  const uint32_t k = rnd() % 5;
  wr16ram(ram, DP + 0x46, k == 0 ? 0xFFFF : k == 1 ? (uint16_t)(1 + rnd() % 3) : (uint16_t)(0x8000 | (rnd() % 4)));
  wr16ram(ram, DP + 0x48, (uint16_t)(rnd() % 4 == 0 ? 0 : 1u << (rnd() % 3)));
  wr16ram(ram, DP + 0x4A, (uint16_t)(2 * (rnd() % 2)));
  wr16ram(ram, DP + 0x4C, (uint16_t)(1 + rnd() % 3));
  const uint16_t car = (uint16_t)(2 * (rnd() % 8));
  wr16ram(ram, DP + 0x94, car);
  wr16ram(ram, 0x03A8 + car, (uint16_t)(rnd() % 3 == 0 ? 0 : rnd() % 2 ? 0x8000 | rnd() : 1 + rnd() % 0x100));
}

// ---- title_frame_step / title_fade_step ($80:F5DE, $80:F6AD) ----  both end with SEC.
// After decompress_near (input Y = 0): X = the input end, Y = A = the length (N Z of it in
// the fade), V clear. The frame step: A = the new D+$40, N Z of its CMP #$17C or of the
// INC D+$3E at $17C. The fade without an unpack: A = D+$42 (N Z of its CMP #$3F), X Y and V
// as window_curve_build leaves them (X = 0, Y = $170; V of its last add).
// the input end of the stream at $7E:`src`, from an unpack made before the C and undone
// (WRAM put back), so the WRAM compared holds the C's writes only
static uint8_t titleProbeRam[TG3000_WRAM_SIZE];
static uint16_t titleUnpackEnd(uint16_t src) {
  memcpy(titleProbeRam, g_ram, sizeof titleProbeRam);
  uint16_t end;
  decompress_near(DP, 0x7E, src, 0x7F, 0x0000, &end);
  memcpy(g_ram, titleProbeRam, sizeof titleProbeRam);
  return end;
}
static void call_title_frame_step(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t src = rd16(0xA920 + (uint32_t)(rd16(DP + 0x40) & 0xFFFE));
  const uint16_t end = titleUnpackEnd(src);
  const uint16_t len = title_frame_step();
  out->a = rd16(DP + 0x40);
  out->x = end;
  out->y = len;
  if(out->a == 0x017C) setNZ16(out, rd16(DP + 0x3E));
  else setNZ16(out, (uint16_t)(out->a - 0x017C));
  out->c = true;
  out->v = false;
}
static void call_title_fade_step(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t end = (uint16_t)(rd16(DP + 0x42) + 1) == 0x003F ? titleUnpackEnd(0xC730) : 0;
  const uint16_t len = title_fade_step();
  out->c = true;
  if(rd16(DP + 0x42) == 0x003F) {
    out->a = out->y = len;
    out->x = end;
    setNZ16(out, len);
    out->v = false;
    return;
  }
  out->a = rd16(DP + 0x42);
  setNZ16(out, (uint16_t)(out->a - 0x003F));
  out->x = 0;
  out->y = 0x0170;
  const uint16_t p = rd16(DP + 0x14), q = rd16(DP + 0x17), prev = (uint16_t)(p - q);
  out->v = ((prev ^ p) & (q ^ p) & 0x8000) != 0;
}
// a compressed stream from the ROM ($88:929A, the palettes' picture) copied to WRAM at
// $7E:`at`, as the title's pictures lie there
static void fuzzTitleStream(uint8_t* ram, uint32_t at) {
  for(uint32_t i = 0; i < 0x1000; i++) ram[at + i] = rom8(0x88929A + i);
}
// the frames around the uploads and the curve's start, the table pointing at a stream
static void fuzz_title_frame_step(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_window_curve_build(r, ram, rnd);
  const uint32_t k = rnd() % 3;
  wr16ram(ram, DP + 0x40, (uint16_t)(k == 0 ? 0x013A + 2 * (rnd() % 4) : k == 1 ? 0x015A + 2 * (rnd() % 0x12) : 0x0100 + 2 * (rnd() % 0x3E)));
  fuzzTitleStream(ram, 0xC000);
  for(uint32_t i = 0; i < 0x80; i += 2) wr16ram(ram, 0xA920 + 0x0100 + i, 0xC000);
  for(uint32_t i = 0; i < 0x100; i += 2) wr16ram(ram, 0xA920 + i, 0xC000);
  for(uint32_t i = 0x100; i < 0x180; i += 2) wr16ram(ram, 0xA920 + i, 0xC000);
}
// the count around its turns ($20, $3F, $7F)
static void fuzz_title_fade_step(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_window_curve_build(r, ram, rnd);
  static const uint16_t near[] = {0x001E, 0x001F, 0x0020, 0x003D, 0x003E, 0x003F, 0x0040, 0x007D, 0x007E, 0x007F};
  wr16ram(ram, DP + 0x42, (rnd() & 1) ? near[rnd() % 10] : (uint16_t)(rnd() % 0x90));
  fuzzTitleStream(ram, 0xC730);
}

// ---- title_frame_begin ($80:F5CD) ----  title_frame_step's exit from $80:F670: A = D+$40
// (2), N Z of its CMP #$17C, X = the input end, Y = the length, C set, V clear
static void call_title_frame_begin(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t end = titleUnpackEnd(rd16(0xA920));
  const uint16_t len = title_frame_begin();
  out->a = rd16(DP + 0x40);
  out->x = end;
  out->y = len;
  setNZ16(out, (uint16_t)(out->a - 0x017C));
  out->c = true;
  out->v = false;
}

// ---- menu_bar_draw ($80:C35E) ----  both callers go on to oam_finish_near: no register
// outputs (the bench judges the flags from the watched returns)
static void call_menu_bar_draw(const Regs* in, Regs* out) {
  (void)in;
  (void)out;
  menu_bar_draw(in->d);
}
// the bar at the keys, between them and now and then past the last; any item; the slot
static void fuzz_menu_bar_draw(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  static const uint8_t keys[4] = {0x00, 0x0B, 0x16, 0x21};
  const uint32_t k = rnd() % 4;
  const uint8_t hi = k == 0 ? keys[rnd() % 4] : k == 3 ? (uint8_t)(0x21 + rnd() % 0x10) : (uint8_t)(rnd() % 0x22);
  wr16ram(ram, DP + 0x4A, (uint16_t)(hi << 8 | (rnd() & 0xFF)));
  wr16ram(ram, DP + 0x48, (uint16_t)(rnd() % 4));
  wr16ram(ram, DP + 0xE7, (uint16_t)(rnd() % 0x100));
  wr16ram(ram, DP + 0x74, (uint16_t)(4 * (rnd() % 0x30)));
}

// ---- track_tiles_load ($85:8000, long) ----  row_copy_6b33's exit (its input A = 0 or
// $20, X = the kind k): A = the last word it copied, X = k (N Z of it), Y = $20, C set, V
// of its ADC of the base
static void call_track_tiles_load(const Regs* in, Regs* out) {
  const uint16_t k = bus_rd8(0x838410 + (uint32_t)in->a);
  const uint16_t handler = bus_rd16(0x858025 + (uint32_t)(uint16_t)(rd16(0x047B) << 1));
  const bool rows = handler == 0x80F7 || handler == 0x8139;
  const uint16_t a6b = (rows && rd8(0x03CB)) ? 0x0020 : 0;
  track_tiles_load(in->a);
  const uint16_t base = bus_rd16(0x8384B1 + (uint32_t)k), from = (uint16_t)(a6b + base);
  out->a = bus_rd16(0x7F0000 + (uint32_t)(uint16_t)(from + 0x1E));
  out->x = k;
  out->y = 0x0020;
  setAdc16(out, a6b, base);
  setNZ16(out, k);
  out->c = true;
}
// any track (A) and any layout ($047B over its 55 entries), the players' flags
static void fuzz_track_tiles_load(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->a = (uint16_t)(rnd() % 0x37);
  wr16ram(ram, 0x047B, (uint16_t)(rnd() % 0x37));
  ram[0x039C] = (rnd() & 1) ? (uint8_t)(1 + rnd() % 255) : 0;
  ram[0x03CB] = (rnd() & 1) ? (uint8_t)(1 + rnd() % 255) : 0;
}

// ---- track_tiles_preview ($8A:8BC2) ----  palette_load_70's exit (A = $7001, X = the kind
// k: N Z of it, Y = $20), which keeps C and V: those of upload_tiles_c680's ADC (its table
// word + $20, plus the layout's offset)
static void call_track_tiles_preview(const Regs* in, Regs* out) {
  const uint16_t k = bus_rd8(0x838410 + (uint32_t)in->a);
  const uint16_t handler = bus_rd16(0x8A8BED + (uint32_t)(uint16_t)(rd16(0x0444) << 1));
  const uint16_t off = (handler == 0x8C5B || handler == 0x8C72) ? 0x0200 : 0x0220;
  track_tiles_preview(in->a);
  out->a = 0x7001;
  out->x = k;
  out->y = 0x0020;
  setNZ16(out, k);
  const uint16_t t = (uint16_t)(rom16(0x8384B1 + (uint32_t)k) + 0x20);
  const uint32_t sum = (uint32_t)t + off;
  out->c = sum > 0xFFFF;
  out->v = (~(t ^ off) & (t ^ sum) & 0x8000) != 0;
}
// any track (A), any layout ($0444 over its 55 entries)
static void fuzz_track_tiles_preview(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->a = (uint16_t)(rnd() % 0x37);
  wr16ram(ram, 0x0444, (uint16_t)(rnd() % 0x37));
}

// ---- title_card_show ($8A:C056) ----  A = the handler put back (N Z of its PLA), X = 0, Y
// = func_8AC0F2's ($14: its last line's, fixed by the ROM); C and V of the last
// wait_frames(1) (C set; V when it started at $7FFF)
static void call_title_card_show(const Regs* in, Regs* out) {
  const uint16_t handler = rd16(0x0008);
  const uint16_t frame = title_card_show();
  out->a = handler;
  out->x = 0;
  out->y = 0x0014;
  setNZ16(out, handler);
  out->c = true;
  out->v = frame == 0x8000;
  (void)in;
}

// ---- vram_planes_build ($8A:8828) ----  A = $0680 (its row address went to $0608; LDA #$80
// with A 8-bit: N set, Z clear), X = 0, Y = $0F, C V of the last ADC #$80 (clear)
static void call_vram_planes_build(const Regs* in, Regs* out) {
  (void)in;
  vram_planes_build();
  out->a = 0x0680;
  out->x = 0;
  out->y = 0x000F;
  out->n = true;
  out->z = false;
  out->c = out->v = false;
}
// deliberately wrong: the second upload one word off (the raw DMA's VRAM address checked)
static void call_vram_planes_build_wrong(const Regs* in, Regs* out) {
  (void)in;
  video_set_vram_address(0x0000);
  dma_stream_setup(0, 0x00, 0x19, 0x7E8633);
  dma_stream_size(0, 0x1200);
  dma_stream_kick(0x01);
  video_set_vram_address(0x1201);  // wrong: $1200
  dma_stream_setup(0, 0x00, 0x19, 0x7E9833);
  dma_stream_size(0, 0x06C0);
  dma_stream_kick(0x01);
  video_set_vram_address(0x0000);
  video_set_vram_increment(0x00);
  dma_stream_setup(0, 0x08, 0x18, 0x8A88DD);
  dma_stream_size(0, 0x4000);
  dma_stream_kick(0x01);
  for(uint32_t i = 0; i < 6; i++) {
    dma_stream_setup(0, 0x00, 0x18, 0x7E80BA + 15 * i);
    dma_stream_size(0, 0x000F);
    video_set_vram_address((uint16_t)(0x0308 + 0x80 * i));
    dma_stream_kick(0x01);
  }
  video_set_vram_increment(0x80);
  out->a = 0x0680;
  out->x = 0;
  out->y = 0x000F;
  out->n = true;
  out->z = false;
  out->c = out->v = false;
}

// ---- road_shift_apply ($82:F1C1) ----  A = the new $1EF6, N Z C V of its ADC (the old
// $1EF6 + $0206 + the low half's carry), X = D+$16, Y kept
static void call_road_shift_apply(const Regs* in, Regs* out) {
  const uint16_t lo = rd16(0x1EF4), hi = rd16(0x1EF6);
  road_shift_apply();
  const uint16_t at = rd16(0x0204), atHi = rd16(0x0206);
  const uint32_t c0 = ((uint32_t)lo + at) >> 16;
  const uint32_t sum = (uint32_t)atHi + hi + c0;
  out->a = (uint16_t)sum;
  out->x = rd16(DP + 0x16);
  out->y = in->y;
  setNZ16(out, (uint16_t)sum);
  out->c = sum > 0xFFFF;
  out->v = (~(atHi ^ hi) & (atHi ^ (uint16_t)sum) & 0x8000) != 0;
}
// a row of the tables (flagged or not), the step D+$14, the car's words and the offset's
// sign
static void fuzz_road_shift_apply(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  const uint16_t x = (uint16_t)(2 * (rnd() % 0x40));
  wr16ram(ram, DP + 0x16, x);
  wr16ram(ram, DP + 0x14, (uint16_t)rnd());
  for(uint32_t t = 0; t < 3; t++) wr16ram(ram, 0x3DEC + 2 * t + x, (uint16_t)rnd());
  wr16ram(ram, 0x3D6C + x, (uint16_t)(rnd() % 3 == 0 ? 0x8000 | (rnd() & 0x7FFD) : rnd()));
  ram[0x3AEF + x] = (uint8_t)rnd();
  static const uint16_t words[] = {0x0200, 0x0202, 0x0204, 0x0206, 0x020A, 0x020C, 0x01CE, 0x1EF4, 0x1EF6, 0x1EC2, 0x1EC4};
  for(unsigned i = 0; i < sizeof words / sizeof words[0]; i++) wr16ram(ram, words[i], (uint16_t)rnd());
  wr16ram(ram, 0x1E66, (uint16_t)rnd());
}

// ---- road_shift_settle ($82:EFDA) ----  as road_shift_apply (the same tail)
static void call_road_shift_settle(const Regs* in, Regs* out) {
  const uint16_t lo = rd16(0x1EF4), hi = rd16(0x1EF6);
  road_shift_settle();
  const uint16_t at = rd16(0x0204), atHi = rd16(0x0206);
  const uint32_t c0 = ((uint32_t)lo + at) >> 16;
  const uint32_t sum = (uint32_t)atHi + hi + c0;
  out->a = (uint16_t)sum;
  out->x = rd16(DP + 0x16);
  out->y = in->y;
  setNZ16(out, (uint16_t)sum);
  out->c = sum > 0xFFFF;
  out->v = (~(atHi ^ hi) & (atHi ^ (uint16_t)sum) & 0x8000) != 0;
}
// road_shift_apply's, with the car's and the gap's words small so every way is taken
static void fuzz_road_shift_settle(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_road_shift_apply(r, ram, rnd);
  if(rnd() & 1) wr16ram(ram, 0x1EF6, (uint16_t)((rnd() % 0x400) - 0x200));
  if(rnd() & 1) wr16ram(ram, 0x0206, (uint16_t)((rnd() % 0x400) - 0x200));
}

// ---- follow_car_step ($83:F73E) ----  nothing to follow (never run): A = 1, the BIT of D+$44
// (Z set, N V its bits 15 and 14), X Y C kept. Else X = $1EBA, Y = $1E9E and: still behind:
// A = the gap's two words ORed (N Z of it), C set, V of the high word's SBC #0; arrived: A
// = $1ECE (or $118: N Z clear, C set) with N Z C of its CMP #$8000, V of the snap's high
// SBC #0
static void call_follow_car_step(const Regs* in, Regs* out) {
  const uint16_t dp = in->d, flags = rd16((uint16_t)(dp + 0x44));
  const uint16_t car = rd16(0x1EBA), lo5 = rd16(0xFAB5 + (uint32_t)car), hi7 = rd16(0xFAB7 + (uint32_t)car);
  const uint16_t count = rd16(0x1ECE);
  follow_car_step(dp);
  out->x = in->x;
  out->y = in->y;
  if(!(flags & 1)) {
    out->a = 0x0001;
    out->z = true;
    out->n = (flags & 0x8000) != 0;
    out->v = (flags & 0x4000) != 0;
    return;
  }
  out->x = car;
  out->y = rd16(0x1E9E);
  if(rd16((uint16_t)(dp + 0x44)) & 1) {  // still behind
    const uint16_t lo2 = rd16(0x0114), hi2 = rd16(0x0116);
    const uint16_t lo1 = (uint16_t)(lo2 + 0x40);
    const bool borrow = lo1 < 0x40;
    const uint16_t hi1 = (uint16_t)(hi2 + borrow);
    out->a = (uint16_t)(hi2 | lo2);
    setNZ16(out, out->a);
    out->c = true;
    out->v = (hi1 & (hi1 ^ hi2) & 0x8000) != 0;
    return;
  }
  const uint16_t hi = (uint16_t)(hi7 - (lo5 < 0x40));
  out->v = (hi7 & (hi7 ^ hi) & 0x8000) != 0;
  out->a = rd16(0x1ECE);
  if((uint16_t)(count - 1) == 0x8000) {
    out->n = out->z = false;
    out->c = true;
    return;
  }
  setNZ16(out, (uint16_t)(out->a - 0x8000));
  out->c = out->a >= 0x8000;
}
// a car (its stride $35) and the view near it, behind, at or past it; a speed; the count
// near its wrap; bit 0 of D+$44 now and then clear
static void fuzz_follow_car_step(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  const uint16_t dp = r->d;
  const uint16_t car = (uint16_t)(0x35 * (rnd() % 20));
  wr16ram(ram, 0x1EBA, car);
  const uint32_t at = rnd();
  wr16ram(ram, 0xFAB5 + car, (uint16_t)at);
  wr16ram(ram, 0xFAB7 + car, (uint16_t)(at >> 16));
  const uint32_t view = at - 0x40 - (rnd() % 4 == 0 ? rnd() % 0x10 : rnd() % 0x2000) + (rnd() % 4 == 0 ? 0x20 : 0);
  wr16ram(ram, (uint16_t)(dp + 0x25), (uint16_t)view);
  wr16ram(ram, (uint16_t)(dp + 0x27), (uint16_t)(view >> 16));
  wr16ram(ram, 0x1E9E, (uint16_t)(rnd() % 0x400));
  if(rnd() & 1) {  // a few units behind at a crawl: it lands in the last $40 and arrives
    // (an overshoot wraps the 32-bit gap past $40 and counts as still behind)
    const uint32_t near = at - 0x40 - rnd() % 0x10;
    wr16ram(ram, (uint16_t)(dp + 0x25), (uint16_t)near);
    wr16ram(ram, (uint16_t)(dp + 0x27), (uint16_t)(near >> 16));
    wr16ram(ram, 0x1E9E, (uint16_t)(rnd() % 9));
  }
  wr16ram(ram, (uint16_t)(dp + 0x44), (uint16_t)(rnd() % 8 == 0 ? rnd() & 0xFFFE : rnd() | 1));
  wr16ram(ram, 0x1ECE, ((rnd() >> 20) % 3 == 0) ? 0x8001 : (uint16_t)(1 + rnd() % 0x200));  // (high bits: the low ones pair with the arrival's)
}

// ---- track_event_clear ($82:EADA) ----  A = the $3D6C word (N Z of its ORA), X = D+$1A
static void call_track_event_clear(const Regs* in, Regs* out) {
  (void)in;
  out->a = track_event_clear();
  out->x = rd16(DP + 0x1A);
  setNZ16(out, out->a);
}
static void fuzz_track_event_clear(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint16_t x = (uint16_t)((rnd() % 0x20) * 2);
  wr16ram(ram, DP + 0x1A, x);
  wr16ram(ram, 0x3D6C + x, (uint16_t)rnd());
}

// ---- track_event_set ($82:EA81 ... $82:EAD4, body $82:EB1A) ----  A = the $3D6C word (N Z
// of its ORA), X = D+$1A, Y kept; C and V of the position + D+$1E
static void trackEventSetOut(const Regs* in, Regs* out, uint16_t pattern) {
  out->a = track_event_set(pattern, in->y);
  out->x = rd16(DP + 0x1A);
  setAdc16(out, rd16(0x1F0E), rd16(DP + 0x1E));
  setNZ16(out, out->a);
}
static void call_track_event_2007(const Regs* in, Regs* out) { trackEventSetOut(in, out, 0x2007); }
static void call_track_event_2047(const Regs* in, Regs* out) { trackEventSetOut(in, out, 0x2047); }
static void call_track_event_2087(const Regs* in, Regs* out) { trackEventSetOut(in, out, 0x2087); }
static void call_track_event_2107(const Regs* in, Regs* out) { trackEventSetOut(in, out, 0x2107); }
static void call_track_event_c2e7(const Regs* in, Regs* out) { trackEventSetOut(in, out, 0xC2E7); }
// the five's shared tail ($82:EB1A, measure-2): entered by their JMP with the pattern in A
static void call_track_event_tail(const Regs* in, Regs* out) { trackEventSetOut(in, out, in->a); }
static void fuzz_func_82EC09(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
static void fuzz_track_event_set(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_func_82EC09(r, ram, rnd);
  r->y = r->x;
  fuzz_track_event_clear(r, ram, rnd);
}
// the tail's: the pattern any word
static void fuzz_track_event_tail(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_track_event_set(r, ram, rnd);
  r->a = (uint16_t)rnd();
}

// ---- title_sprites_place ($8A:824B) ----  placed: A = $070A (N Z), C V of $060A + $10;
// skipped: A = D+$62, flags of its CMP #$FFFF
static void call_title_sprites_place(const Regs* in, Regs* out) {
  (void)in;
  uint16_t flag = rd16(DP + 0x62);
  title_sprites_place();
  if(flag != 0xFFFF) {
    out->a = flag;
    setNZ16(out, (uint16_t)(flag - 0xFFFF));
    out->c = false;
    return;
  }
  out->a = rd16(0x070A);
  setNZ16(out, out->a);
  setAdc16(out, (uint16_t)(rd16(0x060C) - 0x0010), 0x0010);
}
static void fuzz_title_sprites_place(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x62, (rnd() & 3) ? 0xFFFF : (uint16_t)rnd());
  for(uint32_t a = 0x4A; a <= 0x4C; a += 2) wr16ram(ram, DP + a, (uint16_t)rnd());
  for(uint32_t a = 0x74; a <= 0x76; a += 2) wr16ram(ram, DP + a, (uint16_t)rnd());
}

// ---- rotation_from_angle ($8A:867C) ----  A = -sin (N Z of its INC), X = the table offset,
// C = bit 2 of the angle (the third LSR)
static void call_rotation_from_angle(const Regs* in, Regs* out) {
  (void)in;
  uint16_t angle = rd16(DP + 0x34);
  rotation_from_angle();
  out->a = rd16(DP + 0x18);
  out->x = (uint16_t)((angle >> 3) & 0x03FE);
  setNZ16(out, out->a);
  out->c = (angle >> 2) & 1;
}
static void fuzz_rotation_from_angle(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x34, (uint16_t)rnd());
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
// password_hdma_off ($8A:B18C, vs-race-35): a frame's wait, then the HDMA enable and the ROM speed
// off (its 16-bit STZ). The flags are the wait's (as call_wait_frames: Z and C set, N clear, V
// the add's); A its target. (password_hdma_on, $8A:B197, runs only inside other waiting
// samples, which keep the interrupts: it can have no sample of its own)
static void hdmaPairOut(Regs* out, uint16_t now, bool on) {
  const uint32_t sum = 1u + now;
  out->a = on ? 0x0082 : (uint16_t)sum;
  out->z = !on;
  out->c = true;
  out->n = false;
  out->v = (~(1u ^ now) & (1u ^ sum) & 0x8000) != 0;
}
static void call_password_hdma_off(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t now = rd16(FRAME_COUNTER);
  password_hdma_off();
  hdmaPairOut(out, now, false);
}
// vs_names_absent ($8A:A6F6, vs-race-35): X ends $FFFE (N set, Z clear); C set by a copy's CMP
static void call_vs_names_absent(const Regs* in, Regs* out) {
  bool copied = false;
  for(int x = 6; x >= 0; x -= 2) if(rd16((uint16_t)(0x0DF4 + x)) == 0) copied = true;
  vs_names_absent();
  out->n = true;
  out->z = false;
  out->c = copied ? true : in->c;
}
// the four players' names present or absent ($0DF4-$0DFB words, 0 now and then)
static void fuzz_vs_names_absent(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(int x = 0; x < 8; x += 2) wr16ram(ram, 0x0DF4 + (uint32_t)x, (rnd() & 1) ? 0 : (uint16_t)(1 + rnd() % 0xFFFF));
}
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
  uint32_t bcd = binary_to_bcd(in->d, in->a);
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
// ---- sound_boot_upload ($86:E096) / sound_boot ($86:DD85): the driver's upload at start-up ----
// (port request 4) its pointer is the long one at D+$E0; no register outputs (the callers set
// theirs)
static void call_sound_boot_upload(const Regs* in, Regs* out) {
  (void)out;
  sound_boot_upload((uint32_t)(rd8(in->d + 0xE0) | rd8(in->d + 0xE1) << 8 | rd8(in->d + 0xE2) << 16));
}
// sound_load_set ($86:DDEF): A is the set; its last PLB and the ready test give N, Z and C
static void call_sound_load_set(const Regs* in, Regs* out) {
  sound_load_set(in->d, in->a);
  out->n = (in->db >> 7) & 1;  // its last PLB
  out->z = in->db == 0;
  out->c = 1;
}
// sound_load_menu ($84:A4B7): its last LDA #$7F (N, Z clear); C from the loader. (The race's
// and the VS race's helpers, $84:A487 and $84:A4A3, run only inside the race pieces below,
// whose rows hold them: a waiting row inside another gets no samples.)
static void soundHelperFlags(Regs* out) {
  out->n = false;
  out->z = false;
  out->c = 1;
}
static void call_sound_load_menu(const Regs* in, Regs* out) { sound_load_menu(in->d, in->a); soundHelperFlags(out); }
// the race pieces ($82:854B, $82:932D): a demo skips them (N from LDA $047C); else the last
// store's value sets N and Z, C from the loader
static void call_race_sound(const Regs* in, Regs* out) {
  const bool demo = rd16(0x047C) & 0x8000;
  race_sound();
  if(demo) { out->n = true; out->z = false; return; }
  out->n = false;  // the EOR #$01 result, 0 or 1
  out->z = rd8(0x045F) == 0;
  out->c = 0;  // its two LSRs
}
static void call_vs_race_sound(const Regs* in, Regs* out) {
  (void)in;
  const bool demo = rd16(0x047C) & 0x8000;
  vs_race_sound();
  if(demo) { out->n = true; out->z = false; return; }
  out->n = false;
  out->z = true;
  out->c = 1;
}
// game_boot ($80:8002, port request C): from the reset vector's code to its call of the top loop,
// in two rows around the CPU's set-up (its stack switch, $80:802E-$80:803B)
static void call_game_boot_head(const Regs* in, Regs* out) {
  (void)in;
  game_boot_head();
  out->db = 0x81;  // the CPU's set-up (PHA/PLB), native mode, 8-bit registers, C from the XCE
  out->e = false;
  out->m8 = out->x8 = true;
  out->c = 1;
  out->n = true;  // its last wait's LDA $4212 (vblank: bit 7)
  out->z = false;
}
static void call_game_boot_tail(const Regs* in, Regs* out) {
  game_boot_tail();
  out->n = (in->db >> 7) & 1;  // its last PLB (the entry's data bank)
  out->z = in->db == 0;
}
static void call_game_boot_body(const Regs* in, Regs* out) {
  (void)in;
  game_boot_body();
  out->db = 0x00;  // as its callees leave it
  out->n = true;   // its last LDA #$81 (the value for $4200)
  out->z = false;
}
// the top loop's pieces between its calls (port request C)
static void call_options_entry_head(const Regs* in, Regs* out) {
  (void)in;
  options_entry_head();
  out->db = 0x8A;  // the CPU's (PEA/PLB, TCD)
  out->d = 0x0100;
}
static void call_options_entry_tail(const Regs* in, Regs* out) {
  out->db = rd8((uint16_t)(in->sp + 1));  // its PLB: the data bank pushed at $80:FC37
  options_entry_tail();
  wr8((uint16_t)(in->sp + 1), 0x80);  // its JSL's bank byte lands where that was (the CPU's)
}
static void call_top_volumes(const Regs* in, Regs* out) { (void)in; (void)out; top_volumes(); }
static void call_top_after_options(const Regs* in, Regs* out) {
  (void)in;
  out->atEnd2 = top_after_options();
  const uint16_t mode = rd16(0x03DC) & 0x000F;  // its last CMP #$0002
  out->c = mode >= 2;
  out->z = mode == 2;
  out->n = ((uint16_t)(mode - 2) & 0x8000) != 0;
}
static void call_top_tallies(const Regs* in, Regs* out) { (void)in; (void)out; top_tallies(); }
static void call_top_race_prep(const Regs* in, Regs* out) { (void)in; (void)out; top_race_prep(); }
static void call_top_after_race(const Regs* in, Regs* out) { (void)in; (void)out; top_after_race(); }
static void call_top_after_ticker(const Regs* in, Regs* out) { (void)in; (void)out; top_after_ticker(); }
static void call_top_after_title(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t group = rom8(0x84A240 + (uint32_t)rd16(0x0444)), was = rd16(0x0368);
  const bool two = rd16(0x03E2) & 0x0001;
  out->atEnd2 = !top_after_title();
  // C: the CMP's (the same group, or $03E2 bit 0 skipping the test), else player_in_top5's (clear)
  out->c = (out->atEnd2 || two) ? group >= was : false;
}
// the two-task handlers' parts after their switch (port request D): the NMI's ends at its JMP to
// nmi_tail with D = 0 (its PLD) and the data bank pushed at entry (its PLB); the IRQ's at its
// PLD with D = $0200 and DB = $81
static void call_nmi_menu_frame_tail(const Regs* in, Regs* out) {
  nmi_menu_frame_tail();
  out->d = 0x0000;
  out->db = in->db;
}
static void call_irq_menu_frame_tail(const Regs* in, Regs* out) {
  (void)in;
  irq_menu_frame_tail();
  out->d = 0x0200;
  out->db = 0x81;
}
static void call_sound_boot(const Regs* in, Regs* out) {
  sound_boot(in->d);
  setNZ16(out, in->d);  // its last PLD; C from the ready test, V clear from the upload's end
  out->c = 1;
  out->v = 0;
}

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
static void call_nmi_target_80FC14(const Regs* in, Regs* out) { nmi_target_80FC14(); restoreNmiFrame(in, out); }
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
// vs_hud_draw ($83:9B49, vs-race-19): its caller goes on to vs_view_scrolls, which loads X and A
// the C's strip copy seeded with the words the original read back, in its order (port request H),
// when the original drew the HUD (four reads)
static void vsStripSeed(void) {
  static const uint16_t pick[4] = {0x00A4, 0x00AC, 0x00A6, 0x00AE};
  if(difftest_vram_reads_left() < 4) return;
  for(int i = 0; i < 4; i++) vs_strip_set(rd16(pick[i]), difftest_vram_read());
}
static void call_vs_hud_draw(const Regs* in, Regs* out) {
  (void)in;
  (void)out;
  vsStripSeed();
  vs_hud_draw();
}
// the four players: bars 0-12, counts 0-5 (with a high byte: the original masks it), the words,
// the read-back addresses clear of every word written, accumulator pairs (negative now and
// then), the strips' sources
static void fuzz_vs_hud_draw(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  static const uint16_t bar[4] = {0x003E, 0x0040, 0x0064, 0x0068}, cnt[4] = {0x024E, 0x0301, 0x02B0, 0x0363},
                        pick[4] = {0x00A4, 0x00AC, 0x00A6, 0x00AE}, acc[4] = {0x4E4D, 0x4E4F, 0x4E51, 0x4E53},
                        w[12] = {0x42, 0x44, 0x46, 0x4E, 0x50, 0x52, 0x48, 0x4A, 0x4C, 0x54, 0x56, 0x58};
  (void)r;
  for(int i = 0; i < 4; i++) {
    wr16ram(ram, bar[i], (uint16_t)(rnd() % 13));
    wr16ram(ram, cnt[i], (uint16_t)((rnd() & 0xFF00) | rnd() % 6));
    wr16ram(ram, pick[i], (uint16_t)(0x57C0 + rnd() % 0xA0));  // in the progress strip, as in play (port request H)
    ram[acc[i]] = (uint8_t)rnd();
    ram[acc[i] + 1] = (uint8_t)rnd();
  }
  for(int k = 0; k < 12; k++) wr16ram(ram, w[k], (uint16_t)rnd());
  for(uint32_t a = 0x4E61; a < 0x4F31; a++) ram[a] = (uint8_t)rnd();
}
// (vs-race-4) its caller, the VS split screen's interrupt handler, reloads A, X and Y next
static void call_vs_view_scrolls(const Regs* in, Regs* out) { (void)in; (void)out; vs_view_scrolls(); }
static void fuzz_vs_view_scrolls(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x0026, (uint16_t)rnd());
  for(int i = 0; i < 4; i++) {
    static const uint16_t steps[4] = {0x01CC, 0x01D4, 0x01D0, 0x01D8};
    wr16ram(ram, steps[i], (rnd() & 1) ? (uint16_t)(rnd() % 0x400 - 0x200) : (uint16_t)rnd());
    wr16ram(ram, (uint16_t)(0x00D4 + 3 * i), (uint16_t)rnd());
  }
  ram[0x00DF] = (uint8_t)rnd();
  wr16ram(ram, 0x00E2, (uint16_t)(rnd() & 0x0E));
}

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
// (vs-race-21) the VS race's: as irq_race_band
static void call_irq_vs_band(const Regs* in, Regs* out) { irq_vs_band(); restoreP(in, out); }
static void call_irq_vs_band2(const Regs* in, Regs* out) { irq_vs_band2(); restoreP(in, out); }
static void call_irq_vs_split(const Regs* in, Regs* out) { irq_vs_split(); restoreP(in, out); }  // (vs-race-22)

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

// the menus' NMI frame, up to its stack switch (endAt $80:BC54): D = $0100 (its TCD), DB as
// the prologue left it ($81); N/Z of the brightness byte it wrote ($80 when $019B is 0)
static void call_nmi_menu_frame(const Regs* in, Regs* out) {
  (void)in;
  nmi_menu_frame();
  const uint8_t b = rd8(0x019B) ? rd8(0x019B) : 0x80;
  out->n = (b & 0x80) != 0;
  out->z = false;
  out->d = 0x0100;
  out->db = 0x81;
}
// both tasks' rings (tail, head and up to 4 entries; one time in 32 an entry over the
// budget), the fade, the pads, the counter's parity, the shadowed registers, the large
// upload (one time in four; over $1C00 now and then), the brightness. The transfers stay
// small enough for a fuzz case, which starts anywhere in a frame, to end before the next
// NMI (a case the NMI interrupts is lost)
static void fuzz_nmi_menu_frame(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(uint16_t ring = 0; ring <= 0x0100; ring = (uint16_t)(ring + 0x0100)) {
    const uint16_t tail = (uint16_t)((rnd() & 0x3E) + ring), n = (uint16_t)(rnd() % 5);
    wr16ram(ram, ring ? 0x028E : 0x018E, tail);
    wr16ram(ram, ring ? 0x028C : 0x018C, (uint16_t)(((tail + 2 * n) & 0x3F) + ring));
    for(uint16_t i = 0; i < n; i++) {
      const uint16_t x = (uint16_t)(((tail + 2 * i) & 0x3F) + ring);
      wr16ram(ram, (uint32_t)(0x1900 + x), (uint16_t)((rnd() & 31) == 0 ? 0x0D00 : 2 + (rnd() % 0x100) * 2));
      wr16ram(ram, (uint32_t)(0x1940 + x), (uint16_t)rnd());
      wr16ram(ram, (uint32_t)(0x1980 + x), (uint16_t)(0x7E + (rnd() & 1)));
      wr16ram(ram, (uint32_t)(0x19C0 + x), (uint16_t)(rnd() & 0x7FFF));
    }
  }
  const uint16_t target = (uint16_t)(rnd() & 0x0F00);
  const uint16_t step = (rnd() & 3) == 0 ? 0 : (uint16_t)((rnd() & 1) ? (rnd() & 0x1FF) : -(int)(rnd() & 0x1FF));
  wr16ram(ram, 0x0296, step);
  wr16ram(ram, 0x0298, target);
  wr16ram(ram, 0x029A, (rnd() & 3) ? (uint16_t)(rnd() & 0x0FFF) : (uint16_t)(target - step));
  for(uint32_t a = 0x000C; a < 0x0012; a++) ram[a] = (uint8_t)rnd();
  for(uint32_t a = 0x010C; a < 0x0110; a++) ram[a] = (uint8_t)rnd();
  for(uint32_t a = 0x020C; a < 0x0210; a++) ram[a] = (uint8_t)rnd();
  wr16ram(ram, 0x0024, (uint16_t)rnd());
  for(uint32_t a = 0x019C; a < 0x01C6; a++) ram[a] = (uint8_t)rnd();
  ram[0x01A8] &= 0x0F;  // a BG mode the PPU model takes
  wr16ram(ram, 0x016E, (uint16_t)(0x0900 + (rnd() % 4) * 0x220));
  if((rnd() & 3) == 0) {
    wr16ram(ram, 0x0164, (uint16_t)((rnd() & 1) ? 0x1C00 + (rnd() % 0x100) * 2 : (rnd() % 0x400) * 2));
    wr16ram(ram, 0x0166, (uint16_t)rnd());
    wr16ram(ram, 0x0168, (uint16_t)(0x7E + (rnd() & 1)));
    wr16ram(ram, 0x016A, (uint16_t)(rnd() & 0x7FFF));
  } else {
    wr16ram(ram, 0x0164, 0);
  }
  ram[0x019B] = (rnd() & 3) ? (uint8_t)(rnd() & 0x0F) : 0;
}

// the second task's frame, up to its stack switch (endAt $80:BDC1): D = $4300 (its last TCD),
// DB = $80 (PHK, PLB); N/Z of its LDA #$B1. No fuzz: it waits (WAI) for the line interrupt,
// and the fuzz keeps interrupts off
static void call_irq_menu_frame(const Regs* in, Regs* out) {
  (void)in;
  irq_menu_frame();
  out->n = true;
  out->z = false;
  out->d = 0x4300;
  out->db = 0x80;
}

// the results screen, cut at its first wait (advisor): the load (endAt $8A:95EB, not a
// waiting row: the NMIs that land in its long decompressions are its interruptions, and its
// samples are replayed from their entry state) ends after upload_7f_to_3000 (its exit)
// the ticker screen, cut around its first wait (split_screen_setup, its own row): the load ends after
// the STA $0248 (A = $03DA, its N/Z; X = 0 and Y = $11 from the row uploads, C clear from
// their last ADC); the show returns after the 16-bit STZ $420B (A = $89B2: N set; V from
// wait_flag_76's BIT of D+$76; C as the show returns it)
static void call_ticker_screen(const Regs* in, Regs* out) {
  (void)in;
  ticker_screen_load();
  out->a = rd16(0x03DA);
  out->x = 0;
  out->y = 0x0011;
  out->n = out->a >> 15;
  out->z = out->a == 0;
  out->c = false;
}
// menu_button_assign, X = the buttons. On the exit line (C set by its CMP #5): an ignored
// press leaves A = X (N of X, Z of its BIT); the exit leaves the caller too, with its PLA of
// the JSR's word (A, N Z of it). Else a swap ends on PLY (N Z of Y; A the line's old button),
// a new button on TXA (A = X), both with C and V of the CLC / ADC of D+$94. (Not one bit
// exactly: never run.)
static void call_menu_button_assign(const Regs* in, Regs* out) {
  const uint16_t button = in->x, line = rd16(DP + 0x48), p94 = rd16(DP + 0x94);
  const uint16_t mine = (uint16_t)((line << 1) + p94);
  uint16_t found = 0xFFFF;
  if(line != 5) {
    for(uint16_t y = p94; y != 0x000A; y = (uint16_t)(y + 2))
      if(bus_rd16(0x810000 | (uint16_t)(0x039D + y)) == button) { found = y; break; }
  }
  const uint16_t old = rd16((uint16_t)(0x039D + mine));
  const uint16_t ret = rd16((uint16_t)(in->sp + 1));
  const bool leave = menu_button_assign(in->d, button);
  if(line == 5) {
    out->c = true;
    if(leave) {
      out->a = ret;
      setNZ16(out, ret);
      out->leftCaller = true;
    } else {
      out->a = button;
      out->n = button >> 15;
      out->z = true;
    }
    return;
  }
  setAdc16(out, (uint16_t)(line << 1), p94);
  if(found != 0xFFFF) {
    out->a = old;
    out->y = found;
    setNZ16(out, found);
  } else {
    out->a = button;
    out->y = mine;
    setNZ16(out, button);
  }
}

// name_entry_key: a letter or a delete ends on LDA #$20 (N Z clear), X = the new D+$40, C
// from its last CMP (a letter: D+$42 below 13, clear; a delete: set); the end with D+$4C set
// on its LDA of it (C set); the end proper leaves the caller with its PLA of the JSR's word
// (N Z of it), X = D+$40, C and V oam_finish_near's
static void call_name_entry_key(const Regs* in, Regs* out) {
  const uint16_t key = rd16(DP + 0x48), ret = rd16((uint16_t)(in->sp + 1));
  const bool leave = name_entry_key(in->d);
  if(key == 0x001F || key == 0x0020) {
    out->c = true;
    if(leave) {
      out->a = ret;
      setNZ16(out, ret);
      out->x = rd16(DP + 0x40);
      setSbc16(out, rd16(DP + 0x6C), 0x0900);
      out->leftCaller = true;
    } else {
      out->a = rd16(DP + 0x4C);
      setNZ16(out, out->a);
    }
    return;
  }
  out->a = 0x0020;
  out->n = out->z = false;
  out->x = rd16(DP + 0x40);
  out->c = key == 0x001D || key == 0x001E;
}

// the controls menu's parts: the opening ends at the window poll (its PHA of the second
// length above the cut; A = $FFFF, X Y C V rows_7f_copy8's, N Z of DB); the lines end at the
// setup's PLA (A = X = D+$5E + $2000, its TAX's N Z and ADC's C V; Y = D+$5C + $1200); the
// setup gets that length (A)
// and ends at the typed-text poll (A the last colour, N Z of it, C V oam_finish_near's); the
// frame ends at its loop's wait (A oam_finish_near's) or returns with the controls menu on EXIT (A = $CC75, the
// JSR's word menu_button_assign pulled: N set, Z clear; C set; X the buttons)
static void call_menu_controls_open(const Regs* in, Regs* out) {
  uint16_t x, y;
  const uint16_t len = menu_controls_open(in->d, &x, &y);
  wr16((uint16_t)(in->sp - 1), len);
  out->x = x;
  out->y = y;
  out->a = 0xFFFF;
  Regs t;
  setAdc16(&t, (uint16_t)(y - 0x38 - 8), 0x0008);
  out->c = t.c;
  out->v = t.v;
  out->n = (in->db & 0x80) != 0;
  out->z = in->db == 0;
}
static void call_menu_controls_lines(const Regs* in, Regs* out) {
  (void)in;
  menu_controls_lines(in->d);
  out->a = out->x = (uint16_t)(rd16(DP + 0x5E) + 0x2000);
  out->y = (uint16_t)(rd16(DP + 0x5C) + 0x1200);
  setNZ16(out, out->x);
  setAdc16(out, rd16(DP + 0x5E), 0x2000);
}
static void call_menu_controls_setup(const Regs* in, Regs* out) {
  menu_controls_setup(in->d, in->a);
  out->a = rom16(0x81C1C3);
  setNZ16(out, out->a);
  setSbc16(out, rd16(DP + 0x6C), 0x0900);
}
static void call_menu_controls_frame(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t buttons = rd16(DP + 0x0E);
  const uint16_t other = rd16(DP + 0x6E);
  if(!menu_controls_frame(in->d)) {
    out->a = other;  // (oam_finish_near's A: the other buffer's end)
  } else {
    out->a = 0xCC75;
    out->n = true;
    out->z = false;
    out->c = true;
    out->x = buttons;
  }
}

// the name entry's parts, as the controls menu's: the opening (its PHA above the cut, X Y C V
// rows_7f_copy8's); the lines (A = X = D+$5E + $2000); the setup ends at the text poll (A =
// $0100: its 8-bit LDA of the name's end under the $01 of $018C; X = D+$40, Y = D+$42; Z set,
// C V oam_finish_near's); the ready part on LDA #$20 (Y = D+$C4); the frame at the key
// handler's test (A = D+$4A, X = the key * 2, Y = D+$C4); the drawing at the loop's wait
static void call_name_entry_open(const Regs* in, Regs* out) {
  uint16_t x, y;
  const uint16_t len = name_entry_open(in->d, &x, &y);
  wr16((uint16_t)(in->sp - 1), len);
  out->x = x;
  out->y = y;
  out->a = 0xFFFF;
  Regs t;
  setAdc16(&t, (uint16_t)(y - 0x38 - 8), 0x0008);
  out->c = t.c;
  out->v = t.v;
  out->n = (in->db & 0x80) != 0;
  out->z = in->db == 0;
}
static void call_name_entry_lines(const Regs* in, Regs* out) {
  (void)in;
  name_entry_lines(in->d);
  out->a = out->x = (uint16_t)(rd16(DP + 0x5E) + 0x2000);
  out->y = (uint16_t)(rd16(DP + 0x5C) + 0x1200);
  setNZ16(out, out->x);
  setAdc16(out, rd16(DP + 0x5E), 0x2000);
}
static void call_name_entry_setup(const Regs* in, Regs* out) {
  name_entry_setup(in->d, in->a);
  out->a = 0x0100;
  out->x = rd16(DP + 0x40);
  out->y = rd16(DP + 0x42);
  out->n = false;
  out->z = true;
  setSbc16(out, rd16(DP + 0x6C), 0x0900);
}
static void call_name_entry_ready(const Regs* in, Regs* out) {
  (void)in;
  name_entry_ready(in->d);
  out->a = 0x0020;
  out->y = rd16(DP + 0xC4);
  out->n = out->z = false;
}
static void call_name_entry_frame(const Regs* in, Regs* out) {
  (void)in;
  name_entry_frame(in->d);
  out->a = rd16(DP + 0x4A);
  out->x = (uint16_t)(rd16(DP + 0x48) << 1);
  out->y = rd16(DP + 0xC4);
}
static void call_name_entry_draw(const Regs* in, Regs* out) { (void)in; (void)out; name_entry_draw(in->d); }

// the championship screen's parts: the start ends at its wait (flags dead after the cut); the
// setups at split_screen_setup (A = $0C; the fresh one's X = 0 from its clear's DEX: Z set);
// the records at the first queued upload (its arguments: A = $87, X = $8190, Y = D+$5C +
// $3000, C V of that ADD); the load at the queue's poll (A = $FFFF, X and Y past the MVN, N Z
// of its PLB of $81, C V of the ADD of 2); the map's queue and the fade in on their last LDA;
// the panel on LDA #$13 (X, C V func_80B537's)
static void call_champ_begin(const Regs* in, Regs* out) { (void)in; (void)out; champ_begin(); }
static void call_champ_setup(const Regs* in, Regs* out) {
  (void)in;
  champ_setup();
  out->a = 0x000C;
  out->n = out->z = false;
}
static void call_champ_setup_fresh(const Regs* in, Regs* out) {
  (void)in;
  champ_setup_fresh();
  out->a = 0x000C;
  out->x = 0;
  out->n = false;
  out->z = true;
}
static void call_champ_records(const Regs* in, Regs* out) {
  champ_records(in->d);
  out->a = 0x0087;
  out->x = 0x8190;
  out->y = (uint16_t)(rd16((uint16_t)(in->d + 0x5C)) + 0x3000);
  out->n = out->z = false;
  setAdc16(out, rd16((uint16_t)(in->d + 0x5C)), 0x3000);
}
static void call_champ_load(const Regs* in, Regs* out) {
  champ_load(in->d);
  out->a = 0xFFFF;
  out->x = (uint16_t)(rd16((uint16_t)(in->d + 0x5E)) + 0x07FE);
  out->y = (uint16_t)(rd16((uint16_t)(in->d + 0x5E)) + 0x0800);
  out->n = true;
  out->z = false;
  setAdc16(out, rd16((uint16_t)(in->d + 0x5E)), 0x0002);
}
static void call_champ_queue_map(const Regs* in, Regs* out) {
  champ_queue_map(in->d);
  out->a = 0x0800;
  out->n = out->z = false;
}
static void call_champ_panel(const Regs* in, Regs* out) {
  const uint16_t i = rd16((uint16_t)(in->d + 0x8C)), base = rd16((uint16_t)(in->d + 0x8A));
  champ_panel(in->d);
  out->a = 0x0013;
  out->x = (uint16_t)(i + 2);
  out->n = out->z = false;
  setAdc16(out, (uint16_t)((i + 2) & 0x3F), base);
}
static void call_champ_fade_in(const Regs* in, Regs* out) {
  champ_fade_in(in->d);
  out->a = 0x0100;
  out->n = out->z = false;
}

// the options menus' interpreter: the drawing parts end at the selection's test or return at
// the list's end; the selection ends at its jump to the action (its four pushes above the
// cut: D+$62, D+$4A, X, Y; A the action word, C set by its CMP) or returns with C clear; the
// action $8A:B174 returns on its INC
static void call_menu_draw(const Regs* in, Regs* out) { (void)out; menu_draw(in->a); }
static void call_menu_draw_rest(const Regs* in, Regs* out) { (void)out; menu_draw_rest(in->c); }
static void call_menu_select(const Regs* in, Regs* out) {
  const uint16_t p62 = rd16(DP + 0x62), p4A = rd16(DP + 0x4A);
  const bool on = rd16(0x0D90) == rd16(0x0D68);
  if(menu_select()) {
    out->a = rd16(DP + 0x5C);  // (the action: D+$5C is $015C, which the jump goes through)
    setNZ16(out, out->a);
    out->c = true;  // (its CMP of $0D68, equal)
    wr16((uint16_t)(in->sp - 1), p62);
    wr16((uint16_t)(in->sp - 3), p4A);
    wr16((uint16_t)(in->sp - 5), in->x);
    wr16((uint16_t)(in->sp - 7), in->y);
  } else {
    out->c = false;
    if(on) {
      out->a = 0xC0C0;
      out->n = true;
      out->z = true;
    }
  }
}
// the selection's cases: the cursor on its item or not, B/Y/A/X on one of the four pads or
// none, the item pointer within the menus' bank
static void fuzz_menu_select(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  const uint16_t at = (uint16_t)rnd();
  wr16ram(ram, 0x0D90, at);
  wr16ram(ram, 0x0D68, (rnd() & 3) ? at : (uint16_t)rnd());
  static const uint16_t pads[4] = {0x000E, 0x0012, 0x0016, 0x001A};
  for(int i = 0; i < 4; i++) wr16ram(ram, pads[i], (uint16_t)(rnd() & ((rnd() & 1) ? 0xC0C0 : 0x3F3F)));
  wr16ram(ram, DP + 0x4A, (uint16_t)(0x8000 | rnd()));
}
static void call_menu_action_rookie(const Regs* in, Regs* out) {
  menu_action_rookie();
  out->a = (uint16_t)((in->a & 0xFF00) | 0x10);
  setNZ16(out, rd16(DP + 0x6C));
}

// the options menus' actions, entered through the jump on $015C (D+$5C): the leaves run
// through menu_action (the word's case); the submenu actions end at their JSR to the
// submenu (A the menu); the submenu's parts end at its waits and its menu (the chosen test:
// at its wipe or back at the loop's head, harness-23); the selection's tail starts after its
// last PLA (A D+$62's saved value)
static void call_menu_action_submenu(const Regs* in, Regs* out) {
  (void)in;
  out->a = menu_action_submenu(rd16(DP + 0x5C));
  setNZ16(out, out->a);
}
static void call_menu_action_difficulty(const Regs* in, Regs* out) {
  (void)in;
  menu_action(rd16(DP + 0x5C));
  out->a = rd16(0x1526);
  setNZ16(out, out->a);
  out->c = false;
}
static void call_menu_action_music(const Regs* in, Regs* out) {
  (void)in;
  out->c = ((rd16(0x1528) >> 2) & 0x000F) == 1;  // (its CMP #1 on the way to 0; else the ASLs')
  menu_action(rd16(DP + 0x5C));
  out->a = rd16(0x1528);
  setNZ16(out, out->a);
}
// VS MODE's player counts: the action ends at its JSR (Y the count); the pads' mask returns
// after lsr_n (X 0, Y 0, N clear, Z set); the setting ends at the VS screen's JSR (its tail
// $8A:A570 runs after the car's choice, past the movies)
static void call_menu_action_players(const Regs* in, Regs* out) {
  (void)in;
  out->y = menu_action_players(rd16(DP + 0x5C));
  setNZ16(out, out->y);
}
static void call_vs_players_mask(const Regs* in, Regs* out) {
  bool carry;
  out->a = vs_players_mask(in->y, &carry);
  out->c = carry;
  out->x = 0;
  out->y = 0;
  out->n = false;
  out->z = true;
}
static void call_vs_players_set(const Regs* in, Regs* out) {
  bool carry;
  out->a = vs_players_set(in->y, &carry);
  out->c = carry;
  out->x = 0;
  out->y = 0;
  setNZ16(out, out->a);
}
// the pads' mask for 1 to 4 players and any pads with at least that many present (else the
// original loops forever)
static void fuzz_vs_players(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->y = (uint16_t)(1 + rnd() % 4);
  uint16_t pads = (uint16_t)(rnd() & ((rnd() & 1) ? 0x000F : 0xFFFF));
  while(__builtin_popcount(pads) < r->y) pads = (uint16_t)(pads | (1u << (rnd() % 8)));
  wr16ram(ram, DP + 0x56, pads);
}
static void call_menu_action_exit(const Regs* in, Regs* out) {
  (void)in;
  menu_action(rd16(DP + 0x5C));
  setNZ16(out, rd16(DP + 0x5E));
}
static void call_menu_options_apply(const Regs* in, Regs* out) {
  (void)in;
  out->c = menu_options_apply();
  out->a = rd16(0x03DC);
  out->y = 0x0008;
  setNZ16(out, out->a);
}
static void call_menu_password_back(const Regs* in, Regs* out) {
  (void)in;
  menu_password_back();
  out->a = rd16(0x0D7C);
  setNZ16(out, out->a);
}
static void call_menu_submenu_open(const Regs* in, Regs* out) { (void)out; menu_submenu_open(in->a); }
static void call_menu_submenu_tick(const Regs* in, Regs* out) {
  (void)in;
  if(menu_submenu_tick()) {
    out->a = rd16(DP + 0x6C);
  } else {
    out->a = 0x0001;
  }
  setNZ16(out, out->a);
}
static void call_menu_submenu_frame(const Regs* in, Regs* out) {
  (void)in;
  func_8A9FA1();
  out->a = rd16(DP + 0x62);
  setNZ16(out, out->a);
  out->c = true;
}
static void call_menu_submenu_chosen(const Regs* in, Regs* out) {
  (void)in;
  out->a = rd16(DP + 0x5E);
  setNZ16(out, out->a);
  out->atEnd2 = out->a == 0;  // none chosen: on to the submenu's tick ($8A:B65E)
  menu_submenu_chosen();
}
static void call_menu_select_restore(const Regs* in, Regs* out) {
  wr16(DP + 0x62, in->a);
  out->c = true;
}
static void call_password_entry_found(const Regs* in, Regs* out) {
  (void)in;
  password_entry_found();
  out->a = rd16(DP + 0x52);
  setNZ16(out, out->a);
}

// VS MODE's screen: the panels' leaves (X the panel: 0, 2, 4, 6)
static void cmpOut(Regs* out, uint16_t a, uint16_t b) {  // A = a, then CMP b
  out->a = a;
  setNZ16(out, (uint16_t)(a - b));
  out->c = a >= b;
}
static void call_vs_palette_load(const Regs* in, Regs* out) {
  (void)in;
  vs_palette_load();
  out->a = 0x0001;
  out->x = 0x2202;
  out->y = 0x0080;
  out->n = out->z = false;
}
static void call_vs_cursor_moving(const Regs* in, Regs* out) {
  const uint16_t x = in->x;
  const bool moving = vs_cursor_moving(x);
  const uint16_t d88 = rd16((uint16_t)(0x0D88 + x)), d78 = rd16((uint16_t)(0x0D78 + x));
  if(d88 != rd16((uint16_t)(0x0D80 + x))) cmpOut(out, d88, rd16((uint16_t)(0x0D80 + x)));
  else if(d78 != rd16((uint16_t)(0x0D70 + x))) cmpOut(out, d78, rd16((uint16_t)(0x0D70 + x)));
  else {
    const uint16_t a = rd16((uint16_t)(0x0D90 + x)), b = rd16((uint16_t)(0x0D68 + x));
    out->a = (uint16_t)(a - b);
    setNZ16(out, out->a);
    setSbc16(out, a, b);
  }
  out->c = moving;
}
static void call_vs_panel_sprite(const Regs* in, Regs* out) {
  vs_panel_sprite(in->x);
  out->a = 0x3002;
  setNZ16(out, out->a);
  setAdc16(out, (uint16_t)(rom16(0x8AACDC + in->x) << 3), 7);
}
static void call_vs_cursor_home(const Regs* in, Regs* out) {
  vs_cursor_home(in->x);
  out->a = 0xFFFF;
  setNZ16(out, out->a);
}
static void call_vs_letter_index(const Regs* in, Regs* out) {
  const uint16_t times = (uint16_t)(rd16((uint16_t)(0x0DB4 + in->x)) * 7);
  out->a = out->y = vs_letter_index(in->x);
  setNZ16(out, out->a);
  setAdc16(out, times, rd16((uint16_t)(0x0DAC + in->x)));
}
static void call_vs_panel_screens(const Regs* in, Regs* out) {
  vs_panel_screens(in->x);
  setNZ16(out, in->a);
  const uint16_t x = in->x;
  out->c = (x == 0 || x == 2 || x == 4 || x == 6) || x >= 6;  // (its last CPX)
}
static void call_vs_letter_next(const Regs* in, Regs* out) {
  const uint16_t was = rd16((uint16_t)(0x0DB4 + in->x));
  // the flags: its steps over again (CMP #4; vs_letter_index's ADC; CMP #$20, CMP #$3C)
  uint16_t col = was, row = rd16((uint16_t)(0x0DAC + in->x));
  for(;;) {
    if(col == 4) {
      out->c = true;
      break;
    }
    col++;
    setAdc16(out, (uint16_t)(col * 7), row);
    const uint8_t c = rom8(0x8AAFF1 + (uint16_t)(col * 7 + row));
    out->c = c >= 0x20;
    if(c == 0x20) continue;
    out->c = c >= 0x3C;
    if(c == 0x3C) {
      row--;
      continue;
    }
    break;
  }
  vs_letter_next(in->x);
  out->a = was;
  setNZ16(out, was);
  if(was != 4) out->y = rd16(DP + 0x4E);  // (the letters' index of its last step)
}
static void call_vs_cursor_column(const Regs* in, Regs* out) {
  vs_cursor_column(in->x);
  out->a = rom16(0x8AAA8A + in->x);
  setNZ16(out, out->a);
  setAdc16(out, (uint16_t)(rd16((uint16_t)(0x0DB4 + in->x)) << 4), rom16(0x8AAA82 + in->x));
}
static void call_vs_caret_blink(const Regs* in, Regs* out) {
  const uint16_t state = rd16((uint16_t)(0x0DDC + in->x));
  vs_caret_blink(in->x);
  out->c = state >= 0xFFFE;
  if(state == 0xFFFE) {
    out->a = 0xFFFE;
    out->z = true;
    out->n = false;
  } else {
    out->a = rd16((uint16_t)(0x05F8 + in->x));
    setNZ16(out, out->a);
  }
}
static void call_vs_name_clear(const Regs* in, Regs* out) {
  const bool once = rd16((uint16_t)(0x0DEC + in->x)) == 0;
  vs_name_clear(in->x);
  if(once) {
    out->y = 0x000C;
    out->c = true;
  }
  setNZ16(out, in->a);
}
// a panel (0-6, even; sometimes another X for the screens' byte), its letter column 0-4 and
// row start
static void fuzz_vs_panel(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)((rnd() % 8) ? (rnd() % 4) * 2 : rnd() % 16);
  const uint16_t x = (uint16_t)(r->x & 6);
  r->x = (r->x & 1) ? r->x : x | (r->x & 8);
  wr16ram(ram, (uint16_t)(0x0DB4 + r->x), (uint16_t)(rnd() % 5));
  wr16ram(ram, (uint16_t)(0x0DAC + r->x), (uint16_t)(rnd() % 32));
}

// (vs_cursor_moving's exit, for its callers' early returns: A, N Z of its last compare)
static void vsMovingOut(Regs* out, uint16_t x) {
  const uint16_t d88 = rd16((uint16_t)(0x0D88 + x)), d78 = rd16((uint16_t)(0x0D78 + x));
  if(d88 != rd16((uint16_t)(0x0D80 + x))) cmpOut(out, d88, rd16((uint16_t)(0x0D80 + x)));
  else if(d78 != rd16((uint16_t)(0x0D70 + x))) cmpOut(out, d78, rd16((uint16_t)(0x0D70 + x)));
  else {
    const uint16_t a = rd16((uint16_t)(0x0D90 + x)), b = rd16((uint16_t)(0x0D68 + x));
    out->a = (uint16_t)(a - b);
    setNZ16(out, out->a);
    setSbc16(out, a, b);
  }
}
static void call_vs_cursor_step(const Regs* in, Regs* out) {
  const uint16_t x = in->x;
  const uint16_t state = rd16((uint16_t)(0x0D38 + x));
  if(state != 0xFFFF) {
    cmpOut(out, state, 0xFFFF);
    vs_cursor_step(x);
    return;
  }
  const bool moving = vs_cursor_moving(x);
  vsMovingOut(out, x);
  out->c = moving;
  const uint16_t pad = rd16((uint16_t)(0x0DCC + x)), col = rd16((uint16_t)(0x0DB4 + x));
  vs_cursor_step(x);
  if(moving) return;
  if(pad & 0x0100) {
    cmpOut(out, col, 0x0003);
    if(col != 3) setNZ16(out, (uint16_t)(col + 1));
  } else if(pad & 0x0200) {
    out->a = col;
    setNZ16(out, col);
    if(col != 0) setNZ16(out, (uint16_t)(col - 1));
  } else {
    out->a = 0;
    out->n = false;
    out->z = true;
  }
}
static void call_vs_letter_find(const Regs* in, Regs* out) {
  uint16_t y = 0;
  while(rom8(0x8AAF57 + y) != (uint8_t)in->a) y = (uint16_t)(y + 3);
  vs_letter_find(in->x, (uint8_t)in->a);
  const uint8_t col = rom8(0x8AAF59 + y);
  out->a = (uint16_t)((in->a & 0xFF00) | col);
  out->y = (uint16_t)(y + 2);
  out->n = (col & 0x80) != 0;
  out->z = col == 0;
  out->c = true;
}
static void call_vs_cursor_target(const Regs* in, Regs* out) {
  vs_cursor_target(in->x);
  out->a = rd16((uint16_t)(0x0D90 + in->x));
  setNZ16(out, out->a);
  setAdc16(out, (uint16_t)(rd16((uint16_t)(0x0DB4 + in->x)) << 4), rd16((uint16_t)(0x0DC4 + in->x)));
}
static void call_vs_all_ready(const Regs* in, Regs* out) {
  vs_all_ready();
  const uint16_t n = rd16(DP + 0x42), ready = rd16(DP + 0x44);
  out->a = n == ready ? 0xFFC0 : n;
  out->c = n >= ready;
  setNZ16(out, in->x);
}
static void call_vs_panel_screens_bg2_off(const Regs* in, Regs* out) {
  vs_panel_screens_bg2_off(in->x);
  setNZ16(out, in->a);
  const uint16_t x = in->x;
  out->c = (x == 0 || x == 2 || x == 4 || x == 6) || x >= 6;
}
static void call_vs_panel_text_base(const Regs* in, Regs* out) {
  vs_panel_text_base(in->x);
  const uint16_t x = in->x;
  if(x <= 6 && !(x & 1)) {
    out->a = rd16(DP + 0x70);
    setNZ16(out, out->a);
    out->c = true;
  } else {
    cmpOut(out, in->a, 0);  // (A kept)
    setNZ16(out, (uint16_t)(x - 6));
    out->c = x >= 6;
  }
}
static void call_vs_text_center(const Regs* in, Regs* out) {
  const uint16_t col = rd16(DP + 0x3A);
  vs_text_center(in->a);
  const uint16_t half = rd16(DP + 0x48);
  out->y = 0;
  while(bus_rd8(0x8A0000 | (uint16_t)(in->a + out->y)) != 0) out->y++;
  setNZ16(out, in->a);
  setAdc16(out, (uint16_t)(6 - half), col);
}
static void call_vs_row_next(const Regs* in, Regs* out) {
  const uint16_t was = rd16((uint16_t)(0x0DAC + in->x));
  uint16_t row = was;
  const uint16_t col7 = (uint16_t)(rd16((uint16_t)(0x0DB4 + in->x)) * 7);
  for(;;) {
    if(row == 6) {
      out->c = true;
      break;
    }
    row++;
    setAdc16(out, col7, row);
    const uint8_t c = rom8(0x8AAFF1 + (uint16_t)(col7 + row));
    out->c = c >= 0x20;
    if(c == 0x20) continue;
    out->c = c >= 0x3C;
    if(c == 0x3C) continue;
    break;
  }
  vs_row_next(in->x);
  out->a = was;
  setNZ16(out, was);
  if(was != 6) out->y = rd16(DP + 0x4E);
}
// a panel and its letter's row start 0-6
static void fuzz_vs_row(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)((rnd() % 4) * 2);
  wr16ram(ram, (uint16_t)(0x0DB4 + r->x), (uint16_t)(rnd() % 5));
  wr16ram(ram, (uint16_t)(0x0DAC + r->x), (uint16_t)(rnd() % 7));
}
// a panel whose cursor has arrived (mostly) or not, Right, Left or neither, its column 0-3
static void fuzz_vs_step(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)((rnd() % 4) * 2);
  const uint16_t x = r->x;
  wr16ram(ram, (uint16_t)(0x0D38 + x), (rnd() % 8) ? 0xFFFF : (uint16_t)rnd());
  const uint16_t at = (uint16_t)rnd(), w = (uint16_t)(rnd() & 0x3F), y = (uint16_t)rnd();
  const bool arrived = (rnd() % 4) != 0;
  wr16ram(ram, (uint16_t)(0x0D80 + x), at);
  wr16ram(ram, (uint16_t)(0x0D88 + x), arrived ? at : (uint16_t)(at + 1 + (rnd() & 1)));
  wr16ram(ram, (uint16_t)(0x0D70 + x), w);
  wr16ram(ram, (uint16_t)(0x0D78 + x), w);
  wr16ram(ram, (uint16_t)(0x0D68 + x), y);
  wr16ram(ram, (uint16_t)(0x0D90 + x), y);
  static const uint16_t pads[4] = {0x0000, 0x0100, 0x0200, 0x0300};
  wr16ram(ram, (uint16_t)(0x0DCC + x), (uint16_t)(pads[rnd() % 4] | (rnd() & 0xFCFF & ((rnd() & 1) ? 0xFFFF : 0))));
  wr16ram(ram, (uint16_t)(0x0DB4 + x), (uint16_t)(rnd() % 4));
}
// the four panels each in or out, ready or not
static void fuzz_vs_ready(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(uint16_t x = 0; x < 8; x += 2) {
    wr16ram(ram, (uint16_t)(0x0DF4 + x), (rnd() % 3) ? 0xFFFF : (uint16_t)(rnd() & 1));
    wr16ram(ram, (uint16_t)(0x0DDC + x), (rnd() % 3) ? 0xFFFD : (uint16_t)rnd());
  }
}
static void call_vs_panel_reset(const Regs* in, Regs* out) {
  out->c = vs_panel_reset(in->x);
  out->a = 0xFFFF;
  setNZ16(out, out->a);
}
static void call_vs_panel_text_clear(const Regs* in, Regs* out) {
  vs_panel_text_clear(in->x);
  out->a = rd16(DP + 0x70);
  setAdc16(out, (uint16_t)(out->a - 0x40), 0x40);
  out->y = 0x001E;
  setNZ16(out, in->x);
}
static void call_vs_ready_check(const Regs* in, Regs* out) {
  (void)in;
  vs_ready_check();
  out->a = 0xFF80;
  out->c = true;
  setNZ16(out, out->a);
  for(uint16_t x = 0; x < 8; x += 2) {
    const uint16_t s = rd16((uint16_t)(0x0DDC + x));
    if(s != 0xFFFD) {
      cmpOut(out, s, 0xFFFD);
      break;
    }
  }
}
static void call_vs_name_default(const Regs* in, Regs* out) {
  const uint16_t name = rom16(0x8AA804 + in->x);
  bool c = in->c;  // (the count loop's last compare)
  uint16_t n = 0;
  for(uint8_t ch; (ch = bus_rd8(0x8A0000 | (uint16_t)(name + n))) != 0; n++) c = ch >= (ch < 0x41 ? 0x41 : 0x5B) ? true : false;
  const uint16_t count = vs_name_default(in->x);
  out->c = c;
  if(count != 0) {
    out->a = count;
    out->y = n;
    setNZ16(out, count);
    return;
  }
  uint16_t m = 0;
  const uint16_t from = rom16(0x8AA814 + in->x);
  while(bus_rd8(0x8A0000 | (uint16_t)(from + m)) != 0) m++;
  out->a = (uint16_t)(from & 0xFF00);
  out->y = m;
  out->n = false;
  out->z = true;
  out->c = true;
}
static void call_vs_letter_width(const Regs* in, Regs* out) {
  vs_letter_width(in->x);
  const uint16_t y = rd16(DP + 0x4E);
  const uint8_t c = rom8(0x8AAFF1 + y);
  out->y = y;
  if(c == 0x73 || c == 0x64 || c == 0x65) {
    out->a = rd16(DP + 0x50);
    setNZ16(out, out->a);
    out->c = true;
  } else {
    cmpOut(out, c, 0x0065);
  }
}
// a panel, its letter index (the grid's 35 cells), its name empty or not
static void fuzz_vs_name(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)((rnd() % 4) * 2);
  wr16ram(ram, DP + 0x4E, (uint16_t)(rnd() % 35));
  const uint16_t name = rom16(0x8AA804 + r->x);
  if(name < 0x2000) {
    const int n = (int)(rnd() % 7);
    for(int i = 0; i < 6; i++) ram[name + i] = i < n ? (uint8_t)((rnd() & 1) ? 0x41 + rnd() % 26 : 0x20 + rnd() % 0x60) : 0;
  }
}
static void call_vs_players_in(const Regs* in, Regs* out) {
  (void)in;
  vs_players_in();
  out->a = (uint16_t)(rd16(0x03DC) >> 8);
  out->x = 0x0008;
  out->n = false;
  out->z = out->c = true;
}
static void call_vs_names_load(const Regs* in, Regs* out) {
  vs_names_load();
  out->a = (uint16_t)(in->a & 0xFF00);
  out->x = rd16(0x0DDA);
  out->n = false;
  out->z = true;
}
static void call_vs_name_put(const Regs* in, Regs* out) {
  vs_name_put(in->x, (uint8_t)in->a);
  setNZ16(out, in->a);
  const uint16_t x = in->x;
  out->c = (x == 0 || x == 2 || x == 4 || x == 6) || x >= 6;
}
static void call_vs_put_char(const Regs* in, Regs* out) {
  out->a = vs_put_char((uint8_t)in->a);  // (P, X and Y kept: its PHP/PLP)
}
static void call_vs_put_sprite_char(const Regs* in, Regs* out) {
  out->a = vs_put_sprite_char(in->x, in->y, (uint8_t)in->a);
  out->x = 0x0001;
  out->n = out->z = false;
}
// a character (mostly the font's printable range), sprite or map mode
static void fuzz_vs_char(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->a = (uint16_t)((rnd() & 0xFF00) | (0x20 + rnd() % 0x3B));
  wr16ram(ram, DP + 0x88, (rnd() & 1) ? 1 : 0);
}
static void call_vs_put_string(const Regs* in, Regs* out) {
  out->a = vs_put_string(in->a);
  setNZ16(out, in->x);  // (its last PLX)
}
static void vsMarksOut(Regs* out, uint16_t panel) {
  out->a = vs_panel_marks(panel);
  out->x = 0x0001;
  out->y = (panel & 2) ? 0x00B8 : 0x0048;
  out->n = out->z = false;
}
static void call_vs_panel_marks_0(const Regs* in, Regs* out) { (void)in; vsMarksOut(out, 0); }
static void call_vs_panel_marks_1(const Regs* in, Regs* out) { (void)in; vsMarksOut(out, 1); }
static void call_vs_panel_marks_2(const Regs* in, Regs* out) { (void)in; vsMarksOut(out, 2); }
static void call_vs_panel_marks_3(const Regs* in, Regs* out) { (void)in; vsMarksOut(out, 3); }
static void call_vs_name_key(const Regs* in, Regs* out) {
  const uint16_t x = in->x;
  const bool press = (rd16((uint16_t)(0x0DCC + x)) & 0xC0C0) != 0;
  const bool end = rom8(0x8AAFF1 + rd16(DP + 0x4E)) == 0x65;
  // (END: vs_cursor_moving's SBC when it gets to compare the rows; then, once arrived,
  // vs_panel_text_clear's ADDs. V is unchanged on the other paths.)
  const bool rowsCompared = rd16((uint16_t)(0x0D88 + x)) == rd16((uint16_t)(0x0D80 + x)) &&
                            rd16((uint16_t)(0x0D78 + x)) == rd16((uint16_t)(0x0D70 + x));
  const uint16_t d90 = rd16((uint16_t)(0x0D90 + x)), d68 = rd16((uint16_t)(0x0D68 + x));
  const bool moving = !rowsCompared || d90 != d68;
  vs_name_key(x);
  if(!press || !end) return;
  if(moving) {
    if(rowsCompared) setSbc16(out, d90, d68);
    return;
  }
  setAdc16(out, (uint16_t)(rd16(DP + 0x70) - 0x40), 0x40);
}
static void call_vs_name_entry_frame(const Regs* in, Regs* out) { (void)out; vs_name_entry_frame(in->x); }
// a panel's name entry: a press or not, its letter (any cell), its name's length 0-12 and
// whether it was cleared, its cursor arrived or not, a player or the computer (its index in
// its name), a delay or not, the pad's directions
static void fuzz_vs_entry(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_vs_step(r, ram, rnd);
  const uint16_t x = r->x;
  wr16ram(ram, (uint16_t)(0x0DCC + x), (uint16_t)(rnd() & ((rnd() & 1) ? 0xC0C0 : 0x0F00) & ((rnd() % 4) ? 0xFFFF : 0x00FF)));
  wr16ram(ram, DP + 0x4E, (uint16_t)(rnd() % 35));
  wr16ram(ram, (uint16_t)(0x0DD4 + x), (uint16_t)(rnd() % 13));
  wr16ram(ram, (uint16_t)(0x0DEC + x), (uint16_t)(rnd() & 1));
  wr16ram(ram, (uint16_t)(0x0DE4 + x), (rnd() % 4) ? 0 : (uint16_t)(rnd() % 40));
  wr16ram(ram, (uint16_t)(0x0DDC + x), (rnd() & 1) ? 0xFFFF : (uint16_t)(rnd() % 7));
  wr16ram(ram, (uint16_t)(0x0DAC + x), (uint16_t)(rnd() % 7));
  wr16ram(ram, (uint16_t)(0x0DB4 + x), (uint16_t)(rnd() % 5));
}
static void call_vs_car_title(const Regs* in, Regs* out) {
  out->a = vs_car_title(in->x);
  setNZ16(out, in->x);  // (vs_put_string's PLX)
}
static void call_vs_grid_draw(const Regs* in, Regs* out) {
  out->a = vs_grid_draw(in->x, in->y);
  setNZ16(out, in->x);  // (vs_put_string's PLX)
}
static void call_vs_panel_name_draw(const Regs* in, Regs* out) {
  out->a = vs_panel_name_draw(in->x);
  if(in->x <= 6 && !(in->x & 1)) out->y = (in->x & 4) ? 0x00B8 : 0x0048;
  setNZ16(out, in->x);
}
static void call_vs_car_confirm(const Regs* in, Regs* out) { (void)out; vs_car_confirm(in->x); }
static void call_vs_ready_draw(const Regs* in, Regs* out) { (void)out; vs_ready_draw(in->x); }
static void call_vs_car_info(const Regs* in, Regs* out) {
  const uint16_t col = rd16((uint16_t)(0x0DB4 + in->x));
  const bool same = col == rd16((uint16_t)(0x0DAC + in->x));
  out->a = vs_car_info(in->x);
  out->n = false;
  out->z = out->c = true;
  if(same) return;
  const uint16_t text = rom16(0x8ACE63 + (uint16_t)(col << 1));
  uint16_t y = 0;
  while(bus_rd8(0x8A0000 | (uint16_t)(text + y)) != 0xFF) y++;
  out->y = y;
}
static void call_vs_panel_frame(const Regs* in, Regs* out) { (void)out; vs_panel_frame(in->x); }
static void call_vs_graphics_load(const Regs* in, Regs* out) {
  (void)in;
  vs_graphics_load();
  out->a = out->x = 0x01C0;
  out->y = 0;
  out->n = out->c = out->v = false;
  out->z = true;
}
// the VS screen's parts: the blank and the setup end at their frame waits (A = 1, LDA #1);
// a frame ends back at the loop's wait (A = 1) or returns (A = $82)
static void call_vs_screen_blank(const Regs* in, Regs* out) {
  (void)in;
  vs_screen_blank();
  out->a = 0x0001;
  setNZ16(out, out->a);
}
static void call_vs_screen_setup(const Regs* in, Regs* out) {
  (void)in;
  vs_screen_setup();
  out->a = 0x0001;
  setNZ16(out, out->a);
}
static void call_vs_screen_frame(const Regs* in, Regs* out) {
  (void)in;
  out->a = vs_screen_frame() ? 0x0001 : 0x0082;
  setNZ16(out, out->a);
}
// a frame that goes on or ends, the four panels in play or not, their pads
static void fuzz_vs_screen_frame(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x019A, (rnd() & 1) ? 0 : (uint16_t)(1 + rnd() % 0x40));
  for(uint16_t p = 0; p < 4; p++) {
    if(p > 0) wr16ram(ram, (uint16_t)(0x0DF4 + 2 * p), (rnd() % 3) ? 0 : 0xFFFF);
    static const uint16_t pads[4] = {0x000E, 0x0012, 0x0016, 0x001A};
    wr16ram(ram, pads[p], (uint16_t)(rnd() & 0x0F00));
  }
}
// the options screens: the clear ends after its PLX (X kept); the opening at its wipe (the
// pushed NMI handler above the cut); the setup at its first frame's wait (A = 1); a frame's
// end back at that wait (A = 1) or at the fade's wait loop (A = $FFC0)
static void call_options_state_clear(const Regs* in, Regs* out) {
  options_state_clear();
  setNZ16(out, in->x);
}
static void call_options_screen_open(const Regs* in, Regs* out) {
  (void)out;
  wr16((uint16_t)(in->sp - 1), options_screen_open());
}
static void call_options_screen_setup(const Regs* in, Regs* out) {
  (void)in;
  options_screen_setup();
  out->a = 0x0001;
  setNZ16(out, out->a);
}
// a frame's end with the menus left (D+$6C) or not, the idle timer far from its end or at it
static void fuzz_options_screen_tick(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x6C, (rnd() & 1) ? 0 : (uint16_t)(1 + rnd() % 3));
  wr16ram(ram, DP + 0x52, (uint16_t)(rnd() & 1));
  wr16ram(ram, 0x0D7E, (rnd() % 8) ? (uint16_t)(2 + rnd() % 0x800) : 1);
}
static void call_options_screen_tick(const Regs* in, Regs* out) {
  (void)in;
  out->atEnd2 = options_screen_tick();  // the menus left: the fade out ($8A:9F5A)
  out->a = out->atEnd2 ? 0xFFC0 : 0x0001;
  setNZ16(out, out->a);
}
// the password shown: its opening ends at the fade out (the pushed handler above the cut);
// the graphics at the wipe; the text at the 8-frame wait (A = 8); the waiting part before its
// PLA (A = 0, the fade's end read)
static void call_password_show_open(const Regs* in, Regs* out) {
  (void)out;
  wr16((uint16_t)(in->sp - 1), password_show_open());
}
static void call_password_show_graphics(const Regs* in, Regs* out) { (void)in; (void)out; password_show_graphics(); }
static void call_password_show_text(const Regs* in, Regs* out) {
  (void)in;
  password_show_text();
  out->a = 0x0008;
  setNZ16(out, out->a);
}
static void call_password_show_wait(const Regs* in, Regs* out) {
  (void)in;
  password_show_wait();
  out->a = 0;
  out->n = false;
  out->z = true;
}
// vs_panel's cases with the panel's name length a real one (0-12): a longer one writes past
// the names, into the hardware registers on the VS race's entry states
static void fuzz_vs_name_len(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_vs_panel(r, ram, rnd);
  wr16ram(ram, (uint16_t)(0x0DD4 + r->x), (uint16_t)(rnd() % 13));
}
// the VS race's cars
static void call_vs_cars_ahead(const Regs* in, Regs* out) {
  uint16_t a;
  bool c, v;
  out->y = vs_cars_ahead(in->d, in->x, &a, &c, &v);
  out->a = a;
  out->c = c;
  out->v = v;
  setNZ16(out, a);
}
// vs_cars_tie ($81:F6AE) / vs_cars_apart ($81:F6F2): A, N V Z C as the RTS leaves them; X and Y
// restored (vs-race-13)
static void vsCarsExitRegs(const VsCarsExit* e, const Regs* in, Regs* out) {
  out->a = e->a;
  out->x = in->x;
  out->y = in->y;
  out->n = e->n;
  out->v = e->v;
  out->z = e->z;
  out->c = e->c;
}
static void call_vs_cars_tie(const Regs* in, Regs* out) {
  const VsCarsExit e = vs_cars_tie(in->x, in->y);
  vsCarsExitRegs(&e, in, out);
}
static void call_vs_cars_apart(const Regs* in, Regs* out) {
  const VsCarsExit e = vs_cars_apart(in->x, in->y);
  vsCarsExitRegs(&e, in, out);
}
// two of the four cars, the ring through all four in a random order. Distances: Y and the others
// within $100 ahead of X or not; sides near or far. The saved $FAC1/$FAC3 near X's either way, so
// the shared tail both pushes apart and collides. The players' records' words the collision
// divides at random. The count $013C small, so the ring walk can run out.
static void fuzz_vs_cars_pair(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  static const uint16_t cars[4] = {0x0000, 0x0035, 0x006A, 0x009F};
  uint16_t order[4] = {0x0000, 0x0035, 0x006A, 0x009F};
  for(int i = 3; i > 0; i--) {
    const int j = (int)(rnd() % (uint32_t)(i + 1));
    const uint16_t t = order[i];
    order[i] = order[j];
    order[j] = t;
  }
  for(int i = 0; i < 4; i++) wr16ram(ram, (uint16_t)(0xFACD + order[i]), order[(i + 1) % 4]);
  const int xi = (int)(rnd() % 4), yi = (xi + 1 + (int)(rnd() % 3)) % 4;
  const uint16_t x = cars[xi], y = cars[yi];
  r->x = x;
  r->y = y;
  const uint16_t px = (uint16_t)rnd(), sx = (uint16_t)rnd(), fx = (uint16_t)rnd(), fh = (uint16_t)rnd();
  for(int i = 0; i < 4; i++) {
    const uint16_t c = cars[i];
    wr16ram(ram, (uint16_t)(0xFAB5 + c), (uint16_t)(px + ((rnd() % 3) ? rnd() % 0x100 : 0x100 + rnd() % 0xFF00)));
    wr16ram(ram, (uint16_t)(0xFABB + c), (uint16_t)(sx + ((rnd() & 1) ? (int)(rnd() % 0x80) - 0x40 : (int)(rnd() % 0x10000))));
    wr16ram(ram, (uint16_t)(0xFAD5 + c), (uint16_t)rnd());
    wr16ram(ram, (uint16_t)(0xFAC1 + c), (uint16_t)(fx + rnd() % 0x400 - 0x200));
    wr16ram(ram, (uint16_t)(0xFAC3 + c), (rnd() & 7) ? fh : (uint16_t)rnd());
  }
  static const uint16_t players[4] = {0x0000, 0x00B3, 0x0062, 0x0115};
  for(int i = 0; i < 4; i++) {
    const uint16_t p = players[i];
    wr16ram(ram, (uint16_t)(0x020E + p), (uint16_t)rnd());
    wr16ram(ram, (uint16_t)(0x0210 + p), (uint16_t)((rnd() & 1) ? rnd() % 8 : rnd()));
    wr16ram(ram, (uint16_t)(0x022B + p), (uint16_t)((rnd() % 8) ? 0x40 + rnd() % 0x200 : rnd()));
    wr16ram(ram, (uint16_t)(0x0244 + p), (uint16_t)rnd());
  }
  wr16ram(ram, (uint16_t)(0xFAB5 + x), px);
  wr16ram(ram, (uint16_t)(0xFABB + x), sx);
  wr16ram(ram, (uint16_t)(0xFAC1 + x), fx);
  wr16ram(ram, 0x013C, (uint16_t)((rnd() & 1) ? rnd() % 4 : rnd()));
}
// vs_cars_order ($81:F4AF, vs-race-14): A the first place's $FAB6 word, X its car, Y 0; the PLB
// of $83: N set, Z clear; C clear (the lap test's BCC), V of its SBC
static void call_vs_cars_order(const Regs* in, Regs* out) {
  (void)in;
  const VsCarsOrderExit e = vs_cars_order();
  out->a = e.a;
  out->x = rd16(0x1E6A);
  out->y = 0;
  out->db = 0x83;
  out->n = true;
  out->z = false;
  out->c = false;
  out->v = e.v;
}
// the pair routines' fuzz (sides, saved words, players' words), then the four cars in a random
// order with distances in 256ths ($FAB6 word) around the lap $019A: under it, past it (one or
// more at the back), or below 0 (one or more at the front); the handlers' kinds as the movie
// left them, or now and then the RTS kind
static void fuzz_vs_cars_order(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_vs_cars_pair(r, ram, rnd);
  uint16_t order[4] = {0x0000, 0x0035, 0x006A, 0x009F};
  for(int i = 3; i > 0; i--) {
    const int j = (int)(rnd() % (uint32_t)(i + 1));
    const uint16_t t = order[i];
    order[i] = order[j];
    order[j] = t;
  }
  for(int i = 0; i < 4; i++) wr16ram(ram, (uint16_t)(0x1E6A + 2 * i), order[i]);
  const int mode = (int)(rnd() % 5);  // 0, 1: all under; 2: some past; 3: some below 0; 4: all below 0
  // (all four below 0 needs a lap length of $8000 or more: the leader is tested against it
  // unsigned first, so a negative leader is otherwise "past the lap")
  const uint16_t lap = mode == 4 ? (uint16_t)(0xC000 + rnd() % 0x3000) : (uint16_t)(0x100 + rnd() % 0x300);
  wr16ram(ram, 0x019A, lap);
  for(int i = 0; i < 4; i++) {
    const uint16_t c = order[i];
    uint16_t u = (uint16_t)(8 + rnd() % (uint16_t)(lap - 8));
    if(mode == 2 && i == 0) u = (uint16_t)(lap + rnd() % (lap / 2));  // one past the lap, as in a race
    // (two or more past it, with cars side by side, can collide and lose laps for ages)
    if(mode == 3 && i > 0 && (rnd() & 1)) u = (uint16_t)(0x10000 - 1 - rnd() % 0x80);  // (one stays ahead: four
    // just below 0 with a short lap would count as past it, lap after lap, for ages)
    if(mode == 4) u = (uint16_t)(0x8000 + 0x0400 * i + rnd() % 0x100);  // apart; a lap added makes it positive
    wr16ram(ram, (uint16_t)(0xFAB5 + c), (uint16_t)((uint8_t)rnd() | ((u & 0xFF) << 8)));
    wr16ram(ram, (uint16_t)(0xFAB7 + c), (uint16_t)((u & 0x8000) ? 0xFF00 | (u >> 8) : (u >> 8)));
    if(!(rnd() % 4)) wr16ram(ram, (uint16_t)(0xFAC5 + c), 0x0006);  // the RTS kind ($F886)
    // (all four below 0: the cars kept apart sideways, their handlers the RTS so that their
    // players' sides do not bring them back together, or the sort and their collisions can
    // cycle for ever on this made-up state)
    if(mode == 4) {
      wr16ram(ram, (uint16_t)(0xFABB + c), (uint16_t)(0x0400 * (uint16_t)i));
      wr16ram(ram, (uint16_t)(0xFAC5 + c), 0x0006);
    }
  }
}
// car_sync_slot_* ($81:FC72...): the handler's caller reloads the registers (vs-race-14)
static void call_car_sync_slot_0200(const Regs* in, Regs* out) { (void)out; car_sync_slot_0200(in->y); }
static void call_car_sync_slot_02b3(const Regs* in, Regs* out) { (void)out; car_sync_slot_02b3(in->y); }
static void call_car_sync_slot_0262(const Regs* in, Regs* out) { (void)out; car_sync_slot_0262(in->y); }
static void call_car_sync_slot_0315(const Regs* in, Regs* out) { (void)out; car_sync_slot_0315(in->y); }
// one of the four cars (Y), its record's words either way of $0244 bit 0
static void fuzz_car_sync_slot(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  static const uint16_t cars[4] = {0x0000, 0x0035, 0x006A, 0x009F}, recs[4] = {0x0000, 0x00B3, 0x0062, 0x0115};
  r->y = cars[rnd() % 4];
  for(int i = 0; i < 4; i++) {
    const uint16_t b = recs[i];
    wr16ram(ram, (uint16_t)(0x0244 + b), (uint16_t)rnd());
    wr16ram(ram, (uint16_t)(0x022D + b), (uint16_t)rnd());
    wr16ram(ram, (uint16_t)(0x022F + b), (uint16_t)rnd());
    wr16ram(ram, (uint16_t)(0x0204 + b), (uint16_t)rnd());
    wr16ram(ram, (uint16_t)(0x0206 + b), (uint16_t)rnd());
    wr16ram(ram, (uint16_t)(0x0210 + b), (uint16_t)rnd());
  }
  wr16ram(ram, 0x1E9E, (uint16_t)rnd());
}
// vs_car_bands ($82:9806): its caller reloads the registers (vs-race-15)
static void call_vs_car_bands(const Regs* in, Regs* out) { (void)in; (void)out; vs_car_bands(); }
// the four players: their +$13 words (near the race's or any), flags (bits 0 and 2 either way), the
// car near the player or anywhere, the speed words
static void fuzz_vs_car_bands(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  static const uint16_t recs[4] = {0x0200, 0x02B3, 0x0262, 0x0315}, cars[4] = {0xFAB4, 0xFAE9, 0xFB1E, 0xFB53};
  for(int i = 0; i < 4; i++) {
    const uint16_t b = recs[i];
    wr16ram(ram, (uint16_t)(b + 0x13), (rnd() & 3) ? (uint16_t)(rnd() % 0x1400) : (uint16_t)rnd());
    wr16ram(ram, (uint16_t)(b + 0x44), (uint16_t)rnd());
    const uint16_t hi = (uint16_t)rnd();
    wr16ram(ram, (uint16_t)(b + 0x24), (uint16_t)rnd());
    wr16ram(ram, (uint16_t)(b + 0x26), hi);
    wr16ram(ram, cars[i], (uint16_t)rnd());
    wr16ram(ram, (uint16_t)(cars[i] + 2), (rnd() & 3) ? (uint16_t)(hi + rnd() % 0x70 - 0x08) : (uint16_t)rnd());
    wr16ram(ram, (uint16_t)(0x4E2D + 2 * i), (uint16_t)rnd());
  }
}
// vs_hud_digits ($82:9C39): its caller reloads the registers (vs-race-16)
static void call_vs_hud_digits(const Regs* in, Regs* out) { (void)in; (void)out; vs_hud_digits(); }
// the four players: flags (bit 2 now and then), +$13 and +$10 on all sides of the bar's and the
// digits' tests (negative now and then), +$40; the message slots and words as race_message_update's
static void fuzz_vs_hud_digits(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  static const uint16_t recs[4] = {0x0200, 0x02B3, 0x0262, 0x0315}, slots[4] = {0x0233, 0x02E6, 0x0295, 0x0348};
  fuzz_race_message_update(r, ram, rnd);
  wr16ram(ram, 0x019E, (uint16_t)((rnd() & 3) ? 1 + rnd() % 0x100 : rnd()));
  for(int i = 0; i < 4; i++) {
    const uint16_t b = recs[i];
    wr16ram(ram, (uint16_t)(b + 0x44), (uint16_t)((rnd() % 6) ? rnd() & 0xFFFB : rnd()));
    wr16ram(ram, (uint16_t)(b + 0x13), (uint16_t)((rnd() % 6) ? rnd() % 0x4000 : rnd()));
    const uint32_t k = rnd() % 5;
    wr16ram(ram, (uint16_t)(b + 0x10), (uint16_t)(k == 0 ? rnd() % 5 : k == 1 ? 5 + rnd() % 45 : k == 2 ? rnd() : rnd() % 0x1000));
    wr16ram(ram, (uint16_t)(b + 0x40), (uint16_t)((rnd() % 6) ? rnd() % 0x4000 : rnd()));
    wr16ram(ram, slots[i], (uint16_t)(rnd() % 0x4F));
    wr16ram(ram, (uint16_t)(0x4E3D + 2 * i), (uint16_t)(rnd() % 0x4F));
    wr16ram(ram, (uint16_t)(0x4E45 + 2 * i), (uint16_t)(rnd() % 0x40));
    wr16ram(ram, (uint16_t)(0x4E4D + 2 * i), (uint16_t)rnd());
  }
}
// vs_track_objects_stream ($82:B902, vs-race-17): ends on the CMP that found a record not
// reached: A = D+$36, C clear, N Z of the difference, X = that record's index; Y = the last
// upload's VRAM word (LDY D+$18; a kind-0 record leaves Y), V of the last ADC of the bias;
// with no record Y and V as on entry
static void call_vs_track_objects_stream(const Regs* in, Regs* out) {
  const uint16_t pos = rd16(DP + 0x36), bias = rd16(DP + 0x34), vram = rd16(DP + 0x3A);
  uint16_t x = rd16(DP + 0x38);
  while(pos >= rd16(0x5B91 + (uint32_t)x)) {
    const uint16_t kind = rd16(0x5B94 + (uint32_t)x), slot = rd8(0x5B93 + (uint32_t)x);
    Regs v;
    setAdc16(&v, (uint16_t)(slot << 1), bias);
    out->v = v.v;
    if(kind) out->y = (uint16_t)((slot << 5) + vram);
    x = (uint16_t)(x + 5);
  }
  vs_track_objects_stream(in->a);
  out->a = pos;
  out->x = x;
  out->c = false;
  setNZ16(out, (uint16_t)(pos - rd16(0x5B91 + (uint32_t)x)));
}
// one of the four views (the callers' parameters); records from the index on, kinds whose table
// entry points into ROM (now and then kind 0), the source kept inside bank $7F as the one-player
// fuzz does; the player before the first record now and then
static void fuzz_vs_track_objects_stream(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  static const uint16_t tabs[4] = {0x0000, 0x00DC, 0x01B8, 0x0294}, bases[4] = {0x0000, 0x0800, 0x1000, 0x1800},
                        biases[4] = {0x0000, 0x0080, 0x0000, 0x0080}, firsts[4] = {0x0100, 0x0180, 0x0100, 0x0180};
  const uint32_t view = rnd() % 4;
  r->a = firsts[view];
  wr16ram(ram, DP + 0x3C, tabs[view]);
  wr16ram(ram, DP + 0x3A, bases[view]);
  wr16ram(ram, DP + 0x34, biases[view]);
  uint16_t x = (uint16_t)(5 * (rnd() % 8)), at = (uint16_t)(rnd() % 0x100);
  const uint16_t firstAt = at;
  wr16ram(ram, DP + 0x38, x);
  const int n = (int)(1 + rnd() % 5);
  for(int k = 0; k < n; k++, x = (uint16_t)(x + 5)) {
    wr16ram(ram, (uint16_t)(0x5B91 + x), at);
    ram[0x5B93 + x] = (uint8_t)(0x80 + rnd() % 0x60);
    uint16_t kind = 0;
    if(rnd() % 8) {
      while(!kind) {  // kinds whose entry points into ROM (others would read I/O space)
        const bool high = rnd() & 1;
        const uint16_t kk = (uint16_t)(1 + rnd() % (high ? 0x28 : 0x30));
        const uint16_t t = rom16((high ? 0x86AC36 : 0x86AE36) + (uint32_t)(uint16_t)((high ? (uint16_t)(kk << 8) : kk) << 1));
        if(t >= 0x8000) kind = high ? (uint16_t)(kk << 8) : kk;
      }
      const bool high = (kind & 0xFF00) != 0;
      const uint16_t t = rom16((high ? 0x86AC36 : 0x86AE36) + (uint32_t)(uint16_t)(kind << 1));
      const uint16_t off = rom16((high ? 0x860022 : 0x86001E) + (uint32_t)t);
      const uint16_t size = rom16((high ? 0x860024 : 0x860020) + (uint32_t)t);
      const uint16_t base = rom16(0x860000 + (uint32_t)t);
      const uint32_t room = size ? 0x10000u - size : 0;
      const uint16_t src = (uint16_t)(room ? rnd() % room : 0);
      wr16ram(ram, (uint16_t)(0x6591 + base), (uint16_t)(src - off));
    }
    wr16ram(ram, (uint16_t)(0x5B94 + x), kind);
    at = (uint16_t)(at + 1 + rnd() % 0x40);
  }
  wr16ram(ram, (uint16_t)(0x5B91 + x), 0xFFFF);
  const uint16_t pos = (rnd() % 8) ? (uint16_t)(firstAt + rnd() % (uint32_t)(at - firstAt)) : (uint16_t)(firstAt ? firstAt - 1 : 0);
  wr16ram(ram, DP + 0x36, pos);
}
// vs_track_objects_view0..3 ($82:B87A...; vs-race-18): as the stream leaves X, Y, C and V; then
// A = the index stored (N Z of it)
static void callVsTrackObjectsView(int view, Regs* out) {
  static const uint16_t vrams[4] = {0x0000, 0x0800, 0x1000, 0x1800}, dists[4] = {0x0226, 0x02D9, 0x0288, 0x033B},
                        biases[4] = {0x0000, 0x0080, 0x0000, 0x0080};
  const uint16_t pos = rd16(dists[view]);
  uint16_t x = rd16((uint16_t)(0x01A0 + 4 * view));
  while(pos >= rd16(0x5B91 + (uint32_t)x)) {
    const uint16_t kind = rd16(0x5B94 + (uint32_t)x), slot = rd8(0x5B93 + (uint32_t)x);
    Regs v;
    setAdc16(&v, (uint16_t)(slot << 1), biases[view]);
    out->v = v.v;
    if(kind) out->y = (uint16_t)((slot << 5) + vrams[view]);
    x = (uint16_t)(x + 5);
  }
  static void (*const fn[4])(void) = {vs_track_objects_view0, vs_track_objects_view1, vs_track_objects_view2,
                                      vs_track_objects_view3};
  fn[view]();
  out->a = x;
  out->x = x;
  out->c = false;
  setNZ16(out, x);
}
static void call_vs_track_objects_view0(const Regs* in, Regs* out) { (void)in; callVsTrackObjectsView(0, out); }
static void call_vs_track_objects_view1(const Regs* in, Regs* out) { (void)in; callVsTrackObjectsView(1, out); }
static void call_vs_track_objects_view2(const Regs* in, Regs* out) { (void)in; callVsTrackObjectsView(2, out); }
static void call_vs_track_objects_view3(const Regs* in, Regs* out) { (void)in; callVsTrackObjectsView(3, out); }
// the stream's records and distances, moved to the words the wrappers read
static void fuzz_vs_track_objects_view(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_vs_track_objects_stream(r, ram, rnd);
  static const uint16_t dists[4] = {0x0226, 0x02D9, 0x0288, 0x033B};
  for(int view = 0; view < 4; view++) {
    wr16ram(ram, dists[view], (uint16_t)(ram[DP + 0x36] | ram[DP + 0x37] << 8));
    wr16ram(ram, (uint16_t)(0x01A0 + 4 * view), (uint16_t)(ram[DP + 0x38] | ram[DP + 0x39] << 8));
  }
}
// vs_race_reset ($82:BA12, vs-race-23): A = $0300 (its last LDA: N Z clear); C and V of the last of
// the four chained SBC #$20 (on the cars' places, not changed after)
static void call_vs_race_reset(const Regs* in, Regs* out) {
  (void)in;
  vs_race_reset();
  static const uint16_t from[4] = {0x0226, 0x02D9, 0x0288, 0x033B};
  unsigned borrow = 0;
  for(int i = 0; i < 4; i++) {
    const uint16_t m = rd16(from[i]);
    const uint32_t r = (uint32_t)m - 0x20u - borrow;
    borrow = (r >> 31) & 1u;
    out->c = !borrow;
    out->v = ((m ^ 0x0020) & (m ^ (uint16_t)r) & 0x8000) != 0;
  }
  out->a = 0x0300;
  out->n = out->z = false;
}
static void fuzz_track_palettes_load(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
// race_wrapper_head ($82:8342) and race_wrapper_tail ($82:83BB), race-setup-3: waiting rows (a frame's
// wait each); the head ends at the mode's JSR (X = the mode * 2), the tail before its PLB/PLD/RTL
static void call_race_wrapper_head(const Regs* in, Regs* out) {
  (void)in;
  race_wrapper_head();
  out->x = (uint16_t)((rd16(0x03DC) & 3) << 1);
  out->a = out->x;
  out->n = false;
  out->z = out->x == 0;
  out->c = false;  // (the ASL of a 2-bit mode)
  out->db = 0x83;
}
static void call_race_wrapper_tail(const Regs* in, Regs* out) {
  (void)in;
  race_wrapper_tail();
  out->a = 0x0080;
  out->n = false;
  out->z = false;
}
// race_load ($82:8428, race-setup-1): the setup's first piece, ending at the $047C test ($82:854B);
// the flags are the last decompression's (as call_decompress_to_vram: C set, N Z V clear)
static void call_race_load(const Regs* in, Regs* out) {
  (void)in;
  race_load();
  out->n = out->z = out->v = false;
  out->c = true;
}
// race_track_build ($82:856D, race-setup-2): up to the first wait ($82:85D6; an NMI lands in its
// track map's unpacking, so it is judged by replays): A = 1 (LDA #1), N Z of it, C V by liveness
static void call_race_track_build(const Regs* in, Regs* out) {
  (void)in;
  race_track_build();
}
// race_track_start ($82:85D6): up to the race loop's head ($82:8645): A = the message, N Z V of
// BIT $047C (or of LDA #$43, bit 15 set: never run); C of the last frame wait (set)
static void call_race_track_start(const Regs* in, Regs* out) {
  (void)in;
  race_track_start();
  out->c = true;
  const uint16_t m = rd16(0x047C);
  out->v = (m & 0x4000) != 0;
  if(m & 0x8000) {
    out->a = 0x0043;
    out->n = out->z = false;
  } else {
    out->a = 0x003D;
    out->n = false;
    out->z = (m & 0x003D) == 0;
  }
}
// The race loop (race-loop-1): its head up to the wait (an NMI may land in it: judged by replays),
// the wait (a waiting row), the body after it (judged as it comes; its way out at $82:8776)
static void call_race_frame_head(const Regs* in, Regs* out) { (void)in; (void)out; race_frame_head(); }
static void call_race_frame_wait(const Regs* in, Regs* out) { (void)in; (void)out; race_frame_wait(); }
static void call_race_frame_body(const Regs* in, Regs* out) { (void)in; (void)out; race_frame_body(); }
// race_frame_rest ($82:867B, harness-37): the body's short rest, after the frame drawn: the NMI
// rarely lands in it, so its race-end paths (one frame a race, timed by the NMI's frame count)
// are compared live
static void call_race_frame_rest(const Regs* in, Regs* out) {
  (void)in;
  out->atEnd2 = race_frame_rest();  // the race over: on to race_end ($82:8776)
}
// race_end ($82:8776, race-loop-2): the flags of the frame wait (as call_wait_frames)
static void call_race_end(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t now = rd16(FRAME_COUNTER);
  race_end();
  const uint32_t sum = 1u + now;
  out->a = (uint16_t)sum;
  out->z = out->c = true;
  out->n = false;
  out->v = (~(1u ^ now) & (1u ^ sum) & 0x8000) != 0;
}
// vs_race_load ($82:92BA, vs-race-24): the setup's first piece, ending at the $047C test ($82:932D);
// the setup goes on with the registers it loads there; the flags are the last decompression's
// (as call_decompress_to_vram: C set, N Z V clear)
static void call_vs_race_load(const Regs* in, Regs* out) {
  (void)in;
  vs_race_load();
  out->n = out->z = out->v = false;
  out->c = true;
}
// vs_race_frame_body ($82:94B2, vs-race-33): up to the frame's wait ($82:95E8; the NMI lands in it,
// so it is judged by replays from its entry, interrupts off): A = $00B8's last test; V from BIT
// $047C (the race-over path, never run, would test it again), C from CMP $1F08 with 4
static void call_vs_race_frame_body(const Regs* in, Regs* out) {
  (void)in;
  vs_race_frame_body();
  out->v = (rd16(0x047C) & 0x4000) != 0;
  out->c = rd16(0x1F08) >= 0x0004;
}
// vs_race_frame_tail ($82:95E8, vs-race-33): the wait, up to the jump back ($82:963F); V from BIT
// $047C (bit 14); C as it came
static void call_vs_race_frame_tail(const Regs* in, Regs* out) {
  (void)in;
  vs_race_frame_tail();
  out->v = (rd16(0x047C) & 0x4000) != 0;
}
// vs_race_start ($82:9456, vs-race-29): the setup's last piece, ending at the race loop's head
// ($82:94B2): A = $8004 and the flags of BIT $047C, or (bit 15 set) A = $43, N and Z clear; C set
// by the frame wait
static void call_vs_race_start(const Regs* in, Regs* out) {
  (void)in;
  vs_race_start();
  out->c = true;
  const uint16_t m = rd16(0x047C);
  if(m & 0x8000) {
    out->a = 0x0043;
    out->n = out->z = false;
  } else {
    out->a = 0x8004;
    out->n = false;
    out->z = (m & 0x8004) == 0;
  }
  out->v = (m & 0x4000) != 0;
}
// vs_race_track ($82:9348, vs-race-25): the setup's track piece, ending at the JSR to the VS road
// rows ($82:941F): N and Z from the colour's high byte (LDA.L $7E6A34, 8-bit), C and V from the
// last frame wait (C set; V its add's overflow: the counter was $7FFF, so it now reads $8000)
static void call_vs_race_track(const Regs* in, Regs* out) {
  (void)in;
  vs_race_track();
  out->v = rd16(FRAME_COUNTER) == 0x8000;
  const uint8_t hi = bus_rd8(0x7E6A34);
  out->n = (hi & 0x80) != 0;
  out->z = hi == 0;
  out->c = true;
}
// vs_road_rows_view0-3 ($85:BF78, $C316, $C6C2, $CA18; vs-race-26): RTL, DB left $83 (its second PLB)
static void call_vs_road_rows_view0(const Regs* in, Regs* out) { (void)in; vs_road_rows_view0(); out->db = 0x83; }
static void call_vs_road_rows_view1(const Regs* in, Regs* out) { (void)in; vs_road_rows_view1(); out->db = 0x83; }
static void call_vs_road_rows_view2(const Regs* in, Regs* out) { (void)in; vs_road_rows_view2(); out->db = 0x83; }
static void call_vs_road_rows_view3(const Regs* in, Regs* out) { (void)in; vs_road_rows_view3(); out->db = 0x83; }
static uint16_t vsr_w(const uint8_t* ram, uint32_t a) { return (uint16_t)(ram[a] | ram[a + 1] << 8); }
// func_82EA49 on the ROM tables, to know what the tilt will see
static uint16_t vsr_height(const uint8_t* ram, uint16_t a) {
  const uint32_t at = (uint32_t)((a >> 6) & 0xFF) * 10 + vsr_w(ram, 0x100 + 0x98);
  const uint32_t e = (uint32_t)(a & 0x3E) + rom16(0x840002 + (uint16_t)at) + (at >> 16);
  return rom16(0x850000 + (uint16_t)e);
}
// every row built (prev + $20 .. now + $20): its list's first pair must reach it (else a trap)
static bool vsr_rows_ok(const uint8_t* ram, uint16_t prev, uint16_t now, uint16_t laps) {
  uint16_t r = (uint16_t)(prev + 0x20);
  if(r >= laps) r = (uint16_t)(r - laps);
  for(uint16_t n = (uint16_t)(now - prev); n; n--) {
    const uint16_t entry = (uint16_t)(((uint16_t)(r << 2) >> 8) * 10 + vsr_w(ram, 0x100 + 0x98));
    if(rom16(0x850002 + rom16(0x840004 + entry)) < (uint16_t)((r << 1) & 0x3E)) return false;
    if(++r == laps) r = 0;
  }
  return true;
}
// 0-3 rows crossed anywhere on the track; the object stream rewound to an entry (the spawns run:
// views 2 and 3 also two in one frame); views 1-3 also at the lap's ends. Views 0 and 1 keep to
// states that stay out of their never-run paths (checked here by the row lists and the paths'
// conditions: view 1 one object a frame and no road-kind scan; view 0 all of its paths).
static void fuzz_vs_road_rows(Regs* r, uint8_t* ram, uint32_t (*rnd)(void), int view) {
  (void)r;
  static const uint16_t recs[4] = {0x0200, 0x02B3, 0x0262, 0x0315};
  static const uint8_t curs[4] = {0xA0, 0xA4, 0xA8, 0xAC};
  const uint32_t R = recs[view];
  const bool strict = view < 2;
  const uint16_t laps = vsr_w(ram, 0x019A);
  if(laps < 0x60) return;
  const uint16_t seg0 = rom16(0x840000 + vsr_w(ram, 0x100 + 0x9C));
  uint32_t n = 0;  // the stream's entries in order (places rising, below the lap)
  for(uint16_t last = 0; n < 64; n++) {
    const uint16_t place = vsr_w(ram, 0x5B91 + 5 * n);
    if(place < last || place >= laps) break;
    last = place;
  }
  for(int tries = 0; tries < 64; tries++) {
    const uint32_t scenario = rnd() % 3;
    uint16_t at = (uint16_t)(rnd() % (laps - 0x30)), now = (uint16_t)(at + rnd() % 4);
    uint32_t j = 0;
    if(scenario == 1 && n > 0) {
      j = rnd() % n;
      now = vsr_w(ram, 0x5B91 + 5 * j);
      if(!strict && j + 1 < n && (rnd() & 1)) now = vsr_w(ram, 0x5B91 + 5 * (j + 1));  // two objects
      at = (uint16_t)(now >= 3 ? now - rnd() % 4 : now);
      if(strict && (vsr_w(ram, 0x5B91 + 5 * (j + 1)) <= now || (view == 0 && !(vsr_w(ram, 0x5B94 + 5 * j) & 0xFF00))))
        continue;
    } else {
      if(scenario == 2 && view != 0) {
        if(rnd() & 1) {
          at = (uint16_t)(laps - 1 - rnd() % 3);
          now = (uint16_t)(at + rnd() % 4);
        } else {
          at = now = (uint16_t)(rnd() % 3);
        }
      }
      while(j < n && vsr_w(ram, 0x5B91 + 5 * j) <= now) j++;  // the natural cursor
    }
    const uint16_t wrapped = now >= laps ? (uint16_t)(now - laps) : now;
    if(view == 1 && wrapped >= seg0) continue;
    if(view == 0 && (now + 1 >= laps || now < 3 || now >= seg0 || (vsr_height(ram, (uint16_t)(now - 3)) & 0x2000)))
      continue;
    if(!vsr_rows_ok(ram, at, now, laps)) continue;
    wr16ram(ram, R + 0x24, (uint16_t)rnd());
    ram[R + 0x25] = (uint8_t)rnd();
    wr16ram(ram, R + 0x26, now);
    wr16ram(ram, R + 0x3E, at);
    wr16ram(ram, R + 0x3C, (rnd() & 1) ? 0 : (uint16_t)rnd());
    wr16ram(ram, R + 0x40, (uint16_t)rnd());
    wr16ram(ram, R + 0x44, (uint16_t)(rnd() | (view == 0 ? 1 : 0)));  // view 0: bit 0 set (its never-run path)
    wr16ram(ram, 0x100 + curs[view], (uint16_t)(5 * j));
    return;
  }
}
static void fuzz_vs_road_rows_view0(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { fuzz_vs_road_rows(r, ram, rnd, 0); }
// vs_road_rows ($82:DF91, vs-race-27): the four in turn, DB left $83; each player's fuzz in turn
static void call_vs_road_rows(const Regs* in, Regs* out) { (void)in; vs_road_rows(); out->db = 0x83; }
static void fuzz_vs_road_rows_all(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  for(int v = 0; v < 4; v++) fuzz_vs_road_rows(r, ram, rnd, v);
}
// vs_race_sums_clear ($82:9422, vs-race-27): the setup's piece up to the JSL to $85:A471
// (STZ leaves the flags and registers as they came)
static void call_vs_race_sums_clear(const Regs* in, Regs* out) { (void)in; (void)out; vs_race_sums_clear(); }
static void fuzz_vs_road_rows_view1(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { fuzz_vs_road_rows(r, ram, rnd, 1); }
static void fuzz_vs_road_rows_view2(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { fuzz_vs_road_rows(r, ram, rnd, 2); }
static void fuzz_vs_road_rows_view3(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { fuzz_vs_road_rows(r, ram, rnd, 3); }
// track_palettes_load's inputs, $019A and $1F06 (the divide; a zero product now and then), $1F32
static void fuzz_vs_race_reset(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_track_palettes_load(r, ram, rnd);
  wr16ram(ram, 0x019A, (uint16_t)((rnd() % 8) ? 0x40 + rnd() % 0x400 : rnd()));
  wr16ram(ram, 0x1F06, (uint16_t)((rnd() % 8) ? rnd() % 0x2000 : 0));
  wr16ram(ram, 0x1F32, (uint16_t)rnd());
}
// vs_places_update ($82:9AF4): its caller reloads the registers (vs-race-8)
static void fuzz_vs_cars_ahead(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
static void call_vs_places_update(const Regs* in, Regs* out) { (void)in; (void)out; vs_places_update(); }
// the four cars' distances near each other (vs_cars_ahead's), the last counts and the slots at random
static void fuzz_vs_places_update(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_vs_cars_ahead(r, ram, rnd);
  for(int i = 0; i < 4; i++) wr16ram(ram, (uint16_t)(0x1ED2 + 2 * i), (uint16_t)(rnd() % 5));
  static const uint16_t slots[4] = {0x0233, 0x02E6, 0x0295, 0x0348};
  for(int i = 0; i < 4; i++) wr16ram(ram, slots[i], (uint16_t)(rnd() % 0x4F));
}
static void call_vs_cars_ring(const Regs* in, Regs* out) {
  vs_cars_ring();
  out->a = rd16(0x1E6A);
  setNZ16(out, in->x);
}
// vs_cars_link ($82:97BE): A the first car, X the last, Y 0; N Z of the first car
static void call_vs_cars_link(const Regs* in, Regs* out) {
  (void)in;
  out->x = vs_cars_link();
  out->a = rd16(0x1E6A);
  out->y = 0;
  setNZ16(out, out->a);
}
// the four cars: the players' records' offsets in any order, sometimes any word
static void fuzz_vs_cars_link(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  static const uint16_t cars[4] = {0x0000, 0x0062, 0x00B3, 0x0115};
  for(int i = 0; i < 4; i++) wr16ram(ram, (uint16_t)(0x1E6A + 2 * i), (rnd() & 7) ? cars[rnd() % 4] : (uint16_t)(rnd() & 0x03FE));
}
// vs_car_parts ($83:E878): A the last word loaded ($1EB6), X the tyre index; N Z of A
static void call_vs_car_parts(const Regs* in, Regs* out) {
  out->x = vs_car_parts(in->d);
  out->a = rd16(0x1EB6);
  setNZ16(out, out->a);
}
// vs_grid_setup ($82:BD0E): A the end record ($D4), X $20, Y 0; DEY's Z set, its N clear, the
// last ADC's C and V clear
static void call_vs_grid_setup(const Regs* in, Regs* out) {
  vs_grid_setup(in->d);
  out->a = 0x00D4;
  out->x = 0x0020;
  out->y = 0;
  out->n = false;
  out->z = true;
  out->c = false;
  out->v = false;
}
// vs_step_share ($82:BDDA): A the share; N set on entry ends at its LDA (N Z; C V as they
// came), N clear at its SBC from 0 (N Z C V)
static void shareFlags(bool negative, uint16_t r, const Regs* rin, Regs* out) {
  out->a = r;
  setNZ16(out, r);
  if(negative) {
    out->c = rin->c;
    out->v = rin->v;
  } else {
    const uint16_t q = (uint16_t)(0 - r);
    out->c = q == 0;
    out->v = q == 0x8000;
  }
}
static void call_vs_step_share(const Regs* in, Regs* out) { shareFlags(in->n, vs_step_share(in->a, in->n), in, out); }
static void call_vs_steps_share(const Regs* in, Regs* out) {
  const bool negative = (rd16(0x01DA) & 0x8000) != 0;
  shareFlags(negative, vs_steps_share(), in, out);
}
// vs_playerN_frame_setup ($83:E597...): the callers reload the registers (vs-race-7)
static void call_vs_player1_frame_setup(const Regs* in, Regs* out) { (void)in; (void)out; vs_player1_frame_setup(); }
static void call_vs_player2_frame_setup(const Regs* in, Regs* out) { (void)in; (void)out; vs_player2_frame_setup(); }
static void call_vs_player3_frame_setup(const Regs* in, Regs* out) { (void)in; (void)out; vs_player3_frame_setup(); }
static void call_vs_player4_frame_setup(const Regs* in, Regs* out) { (void)in; (void)out; vs_player4_frame_setup(); }
// half the time the controls' path (the record's +$44 bit 2 clear, every player's $03DC bit
// set), with the pads, the masks and the choices at random
static void fuzz_vs_player_frame_setup(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  static const uint16_t recs[4] = {0x0200, 0x02B3, 0x0262, 0x0315};
  const bool controls = rnd() & 1;
  for(int i = 0; i < 4; i++) {
    const uint16_t f = (uint16_t)rnd();
    wr16ram(ram, (uint16_t)(recs[i] + 0x44), controls ? (uint16_t)(f & ~0x0004) : (uint16_t)(f | 0x0004));
    ram[0x0418 + i] = (uint8_t)(rnd() % 8);
    wr16ram(ram, (uint16_t)(0x000C + 4 * i), (uint16_t)rnd());
    wr16ram(ram, (uint16_t)(0x000E + 4 * i), (uint16_t)rnd());
  }
  wr16ram(ram, 0x03DC, controls ? (uint16_t)(rnd() | 0x00F0) : (uint16_t)rnd());
  static const uint16_t masks[6] = {0x039D, 0x039F, 0x03A1, 0x03CC, 0x03CE, 0x03D0};
  for(int k = 0; k < 6; k++) wr16ram(ram, masks[k], (uint16_t)(1u << (rnd() % 16)));
  for(int i = 0; i < 16; i++) ram[0x0390 + i] = (rnd() & 1) ? (uint8_t)rnd() : 0;
}
// vs_cars_frame ($83:E28C), as player1_frame: DB $83, PLD's flags of D = $0100 (vs-race-9)
static void call_vs_cars_frame(const Regs* in, Regs* out) {
  (void)in;
  vs_cars_frame();
  out->db = 0x83;
  out->n = out->z = false;
}
// the players' setup at random as their own fuzz does, the cars as the movie left them
// (player_car_frame's own fuzz, which moves a car anywhere, runs past the budget four cars
// at a time on the VS race's RAM)
static void fuzz_vs_cars_frame(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_vs_player_frame_setup(r, ram, rnd);
}
// the words and the step count: small steps, sometimes any word; the count sometimes 0
static void fuzz_vs_steps_share(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->a = (rnd() & 1) ? (uint16_t)(rnd() % 0x1000 - 0x800) : (uint16_t)rnd();
  static const uint16_t words[4] = {0x01CE, 0x01D2, 0x01D6, 0x01DA};
  for(int i = 0; i < 4; i++) wr16ram(ram, words[i], (rnd() & 1) ? (uint16_t)(rnd() % 0x1000 - 0x800) : (uint16_t)rnd());
  ram[0x1E9E] = (uint8_t)((rnd() % 8) ? 1 + rnd() % 8 : rnd());
}
static void fuzz_vs_grid_setup(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x019A, (uint16_t)((rnd() & 1) ? 0x40 + rnd() % 0x200 : rnd()));
}
// the car's entry D+$14: one of the tables' 16 entries, sometimes any byte
static void fuzz_vs_car_parts(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  wr16ram(ram, (uint16_t)(r->d + 0x14), (uint16_t)((rnd() & 3) ? rnd() % 16 : rnd() & 0xFF));
}
// the player's index 1-4 and the four cars' distances near the record's
static void fuzz_vs_cars_ahead(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)(1 + rnd() % 4);
  static const uint16_t cars[4] = {0x0225, 0x02D8, 0x0287, 0x033A};
  const uint16_t base25 = (uint16_t)rnd(), base27 = (uint16_t)rnd(), base35 = (uint16_t)rnd();
  for(int i = 0; i < 4; i++) {
    wr16ram(ram, cars[i], (rnd() & 1) ? base25 : (uint16_t)(base25 + (rnd() & 0xFF) - 0x80));
    wr16ram(ram, (uint16_t)(cars[i] + 2), (rnd() & 3) ? base27 : (uint16_t)rnd());
    wr16ram(ram, (uint16_t)(cars[i] + 0x10), (rnd() & 3) ? base35 : (uint16_t)rnd());
  }
}
// a letter from the table (its first byte of a random entry)
static void fuzz_vs_letter(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)ram;
  r->x = (uint16_t)((rnd() % 4) * 2);
  r->a = (uint16_t)((r->a & 0xFF00) | rom8(0x8AAF57 + (rnd() % 8) * 3));
}

// the championship menu's pass ends at its button test: C V oam_finish_near's, or with down
// pressed its CMP #3's C (the cursor at the bottom)
// champ_menu_pads ($80:C2DD, harness-37): the pass's up/down test, cut apart from its drawing: the
// presses reach D+$0E through the NMI, so a replay of the whole pass (interrupts off) never sees
// one; the short test is compared live. Its flags as the whole pass's
static void call_champ_menu_pads(const Regs* in, Regs* out) {
  (void)in;
  const bool down = rd16(DP + 0x0E) & 0x0400;
  const uint16_t line = rd16(DP + 0x48);
  champ_menu_pads(in->d);
  setSbc16(out, rd16(DP + 0x6C), 0x0900);
  if(down) out->c = line >= 0x0003;
}
static void call_champ_menu_draw(const Regs* in, Regs* out) {
  (void)in;
  (void)out;
  champ_menu_draw(in->d);
}

// the championship screen's opening animation: the slide step returns its offset in X
static void call_champ_slide_step(const Regs* in, Regs* out) { out->x = champ_slide_step(in->d, in->x); }
static void call_champ_clear_panels(const Regs* in, Regs* out) { (void)out; champ_clear_panels(in->d); }
static void call_champ_noise(const Regs* in, Regs* out) { (void)out; champ_noise(in->d); }
// (the static's panels end after func_80B537: its C and V, the ADD of its queue index, now
// D+$8C, to D+$8A)
static void call_champ_noise_panels(const Regs* in, Regs* out) {
  champ_noise_panels(in->d);
  setAdc16(out, (uint16_t)(rd16((uint16_t)(in->d + 0x8C)) & 0x3F), rd16((uint16_t)(in->d + 0x8A)));
}
static void call_champ_menu_ready(const Regs* in, Regs* out) { (void)out; champ_menu_ready(in->d); }

// the race choice's parts: the close ends at the bar's poll (A = $C5E0: N set); the bar's
// lines at the $4212 poll (A = $DB40 in 8 bits, the poll's mask: its LDA #$40); the load at the
// D+$16 poll (its PHA of the second length above the cut; A = $E0E0, X = 0 from oam_clear);
// the setup at its PLA (A = $7F, X = $1000, Y = $6000: N Z of the LDY); its end at the frame
// poll (A = $0028, X = $0444; C V func_80D6F3's)
// the pre-race frame ends at its fade test (the raster IRQ moves the fade mid-frame, after
// the C's frame: the test and the end are apart); its end returns after the bit search's BIT
// (N V of the track's word, Z clear; C clear from an ASL, else the entry's; A the bit, X =
// D+$2C, Y its number)
static void call_race_start_frame(const Regs* in, Regs* out) {
  (void)in;
  (void)out;
  race_start_frame_draw(in->d);
}
static void call_race_start_end(const Regs* in, Regs* out) {
  (void)in;
  race_start_end(in->d);
  const uint16_t bits = rd16((uint16_t)(0x0808 + rd16(DP + 0x2C)));
  uint16_t a = 1, n = 0;
  while(!(bits & a)) {
    a = (uint16_t)(a << 1);
    n++;
  }
  out->a = a;
  out->x = rd16(DP + 0x2C);
  out->y = n;
  out->n = bits >> 15;
  out->v = (bits >> 14) & 1;
  out->z = false;
  if(n > 0) out->c = false;
}
static void call_race_start_close(const Regs* in, Regs* out) {
  race_start_close(in->d);
  out->a = 0xC5E0;
  out->n = true;
  out->z = false;
}
static void call_race_start_bar(const Regs* in, Regs* out) {
  (void)in;
  race_start_bar();
  out->a = 0xDB40;
  out->m8 = true;
  out->n = out->z = false;
}
static void call_race_start_load(const Regs* in, Regs* out) {
  const uint16_t len = race_start_load(in->d);
  wr16((uint16_t)(in->sp - 1), len);
  out->a = 0xE0E0;
  out->x = 0;
  out->n = false;
  out->z = true;
}
static void call_race_start_setup(const Regs* in, Regs* out) {
  (void)in;
  race_start_setup();
  out->a = 0x007F;
  out->x = 0x1000;
  out->y = 0x6000;
  out->n = out->z = false;
}
static void call_race_start_setup_end(const Regs* in, Regs* out) {
  const uint16_t arg = rom16(0x81CB14 + (uint32_t)(uint16_t)(rd16(0x0368) << 1));
  race_start_setup_end(in->a);
  uint16_t dx = (uint16_t)(((arg & 0xFF) << 8) - rd16(DP + 0x34));
  uint16_t ax = (dx & 0x8000) ? (uint16_t)-dx : dx;
  uint16_t hy = (uint16_t)(arg & 0xFF00), y0 = rd16(DP + 0x36), dy = (uint16_t)(hy - y0);
  uint16_t ay = (dy & 0x8000) ? (uint16_t)-dy : dy;
  out->c = ay >= ax;
  out->v = ((hy ^ y0) & (hy ^ dy) & 0x8000) != 0;
  out->a = rd16(0x0028);
  out->x = rd16(0x0444);
  setNZ16(out, out->a);
}

// the menu choice's parts: the dispatch ends at its JSR (A = D+$4A, N Z of it; X the offset,
// C its ASL's; D+$48 and D+$4A pushed above the cut); back from a choice at the window poll
// (its PHA above the cut; A = 0, X = D+$5E + 24, Y = 0 from the clear's DEY: Z set; C V
// rows_7f_copy8's); the lines at the panels' PLA; the panels at the third func_80B537's JSR
// (A = $200, X = D+$5E + $BE08's, Y = D+$5C + $60); the redraw at the text poll (A, C V
// oam_finish_near's); the end after LDA #2 (C set by the poll's CMP)
static void call_menu_choice_begin(const Regs* in, Regs* out) {
  const uint16_t p48 = rd16(DP + 0x48), p4A = rd16(DP + 0x4A);
  out->x = menu_choice_begin(in->d);
  wr16((uint16_t)(in->sp - 1), p48);
  wr16((uint16_t)(in->sp - 3), p4A);
  out->a = p4A;
  setNZ16(out, p4A);
  out->c = p48 >> 15;
}
static void call_menu_choice_back(const Regs* in, Regs* out) {
  uint16_t x, y;
  const uint16_t len = menu_choice_back(in->d, &x, &y);
  wr16((uint16_t)(in->sp - 1), len);
  Regs t;
  setAdc16(&t, (uint16_t)(y - 0x38 - 8), 0x0008);
  out->c = t.c;
  out->v = t.v;
  out->a = 0;
  out->x = (uint16_t)(rd16(DP + 0x5E) + 0x18);
  out->y = 0;
  out->n = false;
  out->z = true;
}
static void call_menu_choice_lines(const Regs* in, Regs* out) {
  (void)in;
  menu_choice_lines(in->d);
  out->a = out->x = (uint16_t)(rd16(DP + 0x5E) + 0x2800);
  out->y = (uint16_t)(rd16(DP + 0x5C) + 0x1200);
  setNZ16(out, out->x);
  setAdc16(out, rd16(DP + 0x5E), 0x2800);
}
static void call_menu_choice_panels(const Regs* in, Regs* out) {
  menu_choice_panels(in->d, in->a);
  const uint16_t off = rom16(0x81BE08);
  out->a = 0x0200;
  out->x = (uint16_t)(rd16(DP + 0x5E) + off);
  out->y = (uint16_t)(rd16(DP + 0x5C) + 0x0060);
  setNZ16(out, 0x0200);
  setAdc16(out, rd16(DP + 0x5E), off);
}
static void call_menu_choice_redraw(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t other = rd16(DP + 0x6E);
  menu_choice_redraw(in->d);
  out->a = other;
  setNZ16(out, other);
  setSbc16(out, rd16(DP + 0x6C), 0x0900);
}
static void call_menu_choice_done(const Regs* in, Regs* out) {
  (void)in;
  menu_choice_done(in->d);
  out->a = 0x0002;
  out->n = out->z = false;
  out->c = true;
}

// the third choice's parts: the opening pushes both lengths above the cut (the second, then
// the third); the lines and the first panel end at their PLAs (A = X = D+$5E + the next
// graphic's offset); the setup at the text poll (A, C V oam_finish_near's); the frame at its
// loop's wait, or returns on a press (A = 5, N Z clear), C V oam_finish_near's either way; at
// the wait N Z are its LDA #$C0C0 / TRB's, or the LDA of D+$4C that keeps it (below 100
// returns, the returns' watches keep every flag live)
static void call_menu_gears_open(const Regs* in, Regs* out) {
  uint16_t a, b, x, y;
  menu_gears_open(in->d, &a, &b, &x, &y);
  wr16((uint16_t)(in->sp - 1), a);
  wr16((uint16_t)(in->sp - 3), b);
  out->x = x;
  out->y = y;
  out->a = 0xFFFF;
  Regs t;
  setAdc16(&t, (uint16_t)(y - 0x38 - 8), 0x0008);
  out->c = t.c;
  out->v = t.v;
  out->n = (in->db & 0x80) != 0;
  out->z = in->db == 0;
}
static void gearsLinesOut(Regs* out, uint16_t xoff, uint16_t yoff) {
  out->a = out->x = (uint16_t)(rd16(DP + 0x5E) + xoff);
  out->y = (uint16_t)(rd16(DP + 0x5C) + yoff);
  setNZ16(out, out->x);
  setAdc16(out, rd16(DP + 0x5E), xoff);
}
static void call_menu_gears_lines(const Regs* in, Regs* out) {
  (void)in;
  menu_gears_lines(in->d);
  gearsLinesOut(out, 0x2000, 0x1200);
}
static void call_menu_gears_panel(const Regs* in, Regs* out) {
  menu_gears_panel(in->d, in->a);
  gearsLinesOut(out, 0x4000, 0x2D00);
}
static void call_menu_gears_setup(const Regs* in, Regs* out) {
  menu_gears_setup(in->d, in->a);
  out->a = rd16(DP + 0x6C);
  setSbc16(out, rd16(DP + 0x6C), 0x0900);
}
static void call_menu_gears_frame(const Regs* in, Regs* out) {
  (void)in;
  const bool pressed = (rd16(DP + 0x0E) & 0xC0C0) != 0;
  const bool leave = menu_gears_frame(in->d);
  setSbc16(out, rd16(DP + 0x6C), 0x0900);
  if(leave) {
    out->a = 0x0005;
    out->n = out->z = false;
  } else if(pressed) {  // (the window still moving: its LDA of D+$4C)
    out->n = rd16(DP + 0x4C) >> 15;
    out->z = rd16(DP + 0x4C) == 0;
  } else {  // its LDA #$C0C0 and TRB of none
    out->n = true;
    out->z = true;
  }
}

// menu_controls_sprites returns after the last colour's LDA (A, N Z of it), X its index
// (D+$4C / 2, even: C its LSR's), Y = D+$C4
static void call_menu_controls_sprites(const Regs* in, Regs* out) {
  (void)in;
  menu_controls_sprites(in->d);
  const uint16_t k = (uint16_t)((rd16(DP + 0x4C) >> 1) & 0xFFFE);
  out->x = k;
  out->y = rd16(DP + 0xC4);
  out->a = rom16(0x81C1B3 + (uint32_t)k);
  setNZ16(out, out->a);
  out->c = rd16(DP + 0x4C) & 1;
}

// menu_bit_word: its SEC / ROR loop (the SEC's 1 shifted in at the top) leaves Y = twice the
// bit's number, C set; A = X = the word (N Z of its TAX)
static void call_menu_bit_word(const Regs* in, Regs* out) {
  uint16_t a = in->a, y = 0;
  bool c = true;
  for(;;) {
    const bool out_bit = a & 1;
    a = (uint16_t)((a >> 1) | (c ? 0x8000 : 0));
    c = out_bit;
    if(c) break;
    y = (uint16_t)(y + 2);
  }
  out->x = menu_bit_word(in->a);
  out->a = out->x;
  out->y = y;
  out->c = true;
  setNZ16(out, out->x);
}
static void fuzz_menu_bit_word(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)ram;
  r->a = (uint16_t)(1u << (rnd() % 16));
  if(rnd() & 1) r->a = (uint16_t)(r->a | (uint16_t)rnd());
}

// the menu bar's line window: the middle lines end at the dispatch's JMP (A = X = D+$4E * 2,
// N Z of the TAX, C the ASL's); step 1 ends at the shared end; the end returns after its two
// SBCs (A, N Z C V of the second)
static void call_menu_bar_lines(const Regs* in, Regs* out) {
  const uint16_t step = rd16((uint16_t)(in->d + 0x4E));
  out->a = out->x = menu_bar_lines(in->d);
  setNZ16(out, out->x);
  out->c = step >> 15;
}
static void call_menu_bar_open_step(const Regs* in, Regs* out) { (void)out; menu_bar_open_step(in->d); }
static void call_menu_bar_close_step(const Regs* in, Regs* out) { (void)out; menu_bar_close_step(in->d); }
static void call_menu_bar_open_colours_step(const Regs* in, Regs* out) { (void)out; menu_bar_open_colours_step(in->d); }
static void call_menu_bar_open_controls_step(const Regs* in, Regs* out) { (void)in; (void)out; menu_bar_open_controls_step(in->d); }
static void call_menu_bar_open_panel_step(const Regs* in, Regs* out) { (void)in; (void)out; menu_bar_open_panel_step(in->d); }
static void call_menu_bar_lines_end(const Regs* in, Regs* out) {
  const uint16_t first = (uint16_t)(0xFFFF - rd16((uint16_t)(in->d + 0xE8)));
  out->a = menu_bar_lines_end(in->d);
  setNZ16(out, out->a);
  setSbc16(out, first, rd16((uint16_t)(in->d + 0x4C)));
  out->m8 = false;
}

// the shop's main screen in parts, between its waits: the graphics end after the VRAM clear
// (X = 0: Z set); the records at the queue_unpacked_upload call (its arguments: A = $87,
// X = $8000, Y = D+$5C); the text words after LDA #2 / STA D+$C0
static void call_shop_screen_begin(const Regs* in, Regs* out) { (void)in; (void)out; shop_screen_begin(); }
static void call_shop_screen_graphics(const Regs* in, Regs* out) {
  (void)in;
  shop_screen_graphics(in->d);
  out->x = 0;
  out->n = false;
  out->z = true;
}
static void call_shop_screen_records(const Regs* in, Regs* out) {
  const uint16_t step = rom16(0x818B38 + (uint32_t)rd16((uint16_t)(in->d + 0x92)));
  shop_screen_records(in->d);
  out->a = 0x0087;
  out->x = 0x8000;
  out->y = rd16((uint16_t)(in->d + 0x5C));
  out->n = out->z = false;
  setAdc16(out, step, 0x0004);  // (C and V of its CLC / ADC #4)
}
static void call_shop_screen_text_setup(const Regs* in, Regs* out) {
  shop_screen_text_setup(in->d);
  out->a = 0x0002;
  out->n = out->z = false;
}
static void call_shop_screen_list_setup(const Regs* in, Regs* out) { (void)in; (void)out; shop_screen_list_setup(in->d); }
// the frame loop's parts: the frame ends after oam_finish_near (its C and V); back from the
// item screen (after its PLA of the saved D+$4A: A), A = $0C69 (N Z clear); the frame's end with its CMP's carry and the BIT of D+$0C's V; the closing after
// LDA #$89B2 (N set), V from wait_flag_76's BIT of D+$76
static void call_shop_screen_frame(const Regs* in, Regs* out) {
  (void)in;
  shop_screen_frame(in->d);
  setSbc16(out, rd16(DP + 0x6C), 0x0900);
}
static void call_shop_screen_after_items(const Regs* in, Regs* out) {
  shop_screen_after_items(in->d, in->a);
  out->a = 0x0C69;
  out->n = out->z = false;
}
static void call_shop_screen_frame_end(const Regs* in, Regs* out) {
  (void)in;
  out->c = shop_screen_frame_end(in->d);
  out->v = (rd16(DP + 0x0C) >> 14) & 1;
}
static void call_shop_screen_close(const Regs* in, Regs* out) {
  (void)in;
  shop_screen_close(in->d);
  out->a = 0x89B2;
  out->n = true;
  out->z = false;
  out->v = (rd16(DP + 0x76) >> 14) & 1;
}

// the screens' bank $80 wrappers: a prologue row (its PHB of the caller's DB, the only
// push left above the cut; DB = $8A, D = $0100) and an epilogue row ending at the RTL
// (PLB: DB back, N Z of it)
static void call_title_screen_run_open(const Regs* in, Regs* out) {
  title_screen_run_open(in->a);
  wr8(in->sp, in->db);
  out->db = 0x8A;
  out->d = 0x0100;
}
static void call_results_screen_run_open(const Regs* in, Regs* out) {
  (void)out;
  wr8(in->sp, in->db);
  out->db = 0x8A;
  out->d = 0x0100;
  out->m8 = out->x8 = false;
}
static void runCloseOut(const Regs* in, Regs* out) {
  out->db = rd8((uint16_t)(in->sp + 1));
  out->n = out->db >> 7;
  out->z = out->db == 0;
}
static void call_title_screen_run_close(const Regs* in, Regs* out) {
  title_screen_run_close();
  runCloseOut(in, out);
}
static void call_results_screen_run_close(const Regs* in, Regs* out) {
  results_screen_run_close();
  runCloseOut(in, out);
}

// the shop's item screen in parts (the frame's pad input and waits call waiting rows, which
// an enclosing waiting row would hide: left out). The opening: the fade-out path returns
// after LDA #$FF80 (N set) with C from its CMP (equal); else it ends at $80:8F3B after
// LDA $0024 / STA D+$4E (N Z of it), C clear from the CMP of D+$4E with 4
static void call_shop_item_screen(const Regs* in, Regs* out) {
  (void)in;
  if(!shop_item_screen_open(in->d)) {
    out->a = 0xFF80;
    out->n = true;
    out->z = false;
    out->c = true;
    return;
  }
  out->a = rd16(DP + 0x4E);
  setNZ16(out, out->a);
  out->c = false;
}
static void call_shop_item_frame_view(const Regs* in, Regs* out) {
  (void)in;
  (void)out;
  shop_item_frame_view(in->d);
}
// the drawing ends after oam_finish_near: its C and V (the SBC of the other buffer's end,
// now D+$6C, and $0900)
static void call_shop_item_frame_draw(const Regs* in, Regs* out) {
  (void)in;
  shop_item_frame_draw(in->d);
  setSbc16(out, rd16(DP + 0x6C), 0x0900);
}
static void call_shop_item_frame_tail(const Regs* in, Regs* out) {
  (void)in;
  (void)out;
  shop_item_frame_tail(in->d);
}

// the intro, cut at its first wait: the load leaves DB = $81; its flags are dead after the cut (harness-17); the
// show returns after LDA #4 / TRB $0026 (N clear, Z from bit 2), C clear and V from the last
// pad read (pads_read_long; the ADC #$FFFF's when it read no $4213)
static void call_intro_screen(const Regs* in, Regs* out) {
  (void)in;
  intro_screen_load();
  out->db = 0x81;  // (its PEA $817F and two PLBs)
}
static void call_intro_screen_show(const Regs* in, Regs* out) {
  (void)in;
  out->z = !intro_screen_show();
  out->a = 0x0004;
  out->n = false;
  out->c = false;
  uint16_t a = (uint16_t)(rd16(DP + 0xF6) - 3);
  if(a & 0x8000) a = 0;
  out->v = (uint16_t)(a << 3) == 0x8000;  // (the ADC #$FFFF's overflow, before the pad read)
  pads_read_v(out);
}

static void call_ticker_screen_wrong_carry(const Regs* in, Regs* out) {  // (negative: C set)
  call_ticker_screen(in, out);
  out->c = true;
}
static void call_ticker_screen_show(const Regs* in, Regs* out) {
  out->c = ticker_screen_show(in->d, in->c);
  out->a = 0x89B2;
  out->n = true;
  out->z = false;
  out->v = (rd16((uint16_t)(in->d + 0x76)) >> 14) & 1;
}

// the title screen, cut at its first two waits: the load ends after its loop clearing $1516..$1534
// (X = -2, N set); the show returns after wait_frames(8) (its CMP: Z and C set, N clear)
static void call_title_screen(const Regs* in, Regs* out) {
  (void)in;
  title_screen_load();
  out->x = 0xFFFE;
  out->n = true;
  out->z = false;
}
// the scene ends after palettes_load_8a (its exit: N, Z clear, C = $1536 bit 13)
static void call_title_screen_scene(const Regs* in, Regs* out) {
  (void)in;
  title_screen_scene();
  out->x = 0x2202;
  out->a = 0xA001;
  out->y = 0x0020;
  out->n = out->z = false;
  out->c = (rd16(0x1536) & 0x2000) != 0;
}
static void call_title_screen_show(const Regs* in, Regs* out) {
  (void)in;
  title_screen_show();
  out->n = false;
  out->z = out->c = true;
}

// the show: a waiting row with a budget; it returns when the fade is 0 (its LDA: Z set, N
// clear; the carry is stars_draw_8a's)
static void call_results_screen_show(const Regs* in, Regs* out) {
  (void)in;
  results_screen_show();
  out->n = false;
  out->z = true;
  out->c = true;
}
static void call_results_screen(const Regs* in, Regs* out) {
  (void)in;
  results_screen_load();
  out->a = 0x1801; out->x = 0; out->y = 0x3000;
  out->n = out->z = false;
}


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

// ---- nmi_vs_prologue ($83:9721) / nmi_vs_full ($83:9777), vs-race-20 ----
static void call_nmi_vs_prologue(const Regs* in, Regs* out) { (void)in; nmi_vs_prologue(); out->d = 0; out->db = 0x83; }
static void vsStripSeed(void);
static void call_nmi_vs_full(const Regs* in, Regs* out) { vsStripSeed(); nmi_vs_full(); restoreNmiFrame(in, out); }
static void fuzz_vs_view_scrolls(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
static void fuzz_vs_hud_draw(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
// What the VS frame reads: the request, the transfer, the queue, the HUD, the views' scrolls,
// the two clocks (valid BCD, near the minute now and then), both views' band tables (every
// kind of first line and entry, the never-run ones included), the sound request
static void fuzz_nmi_vs_full(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_nmi_race_tail(r, ram, rnd);
  fuzz_race_upload_queue(r, ram, rnd);
  fuzz_vs_hud_draw(r, ram, rnd);
  fuzz_vs_view_scrolls(r, ram, rnd);
  wr16ram(ram, 0x00B8, (rnd() & 1) ? (uint16_t)rnd() : 0);
  wr16ram(ram, 0x00B4, (uint16_t)(rnd() & 1));
  wr16ram(ram, 0x1E02, (uint16_t)((rnd() & 3) ? 0 : (rnd() % 0x200) * 2));
  for(uint16_t a = 0x016C; a <= 0x0182; a += 2) wr16ram(ram, a, (uint16_t)rnd());
  wr16ram(ram, 0x5395, (uint16_t)(0x8007 + 2 * (rnd() % 0x2000)));
  for(int c = 0; c < 2; c++) {
    uint16_t v = 0;
    for(int d = 0; d < 4; d++) v |= (uint16_t)((rnd() % 10) << (4 * d));
    if((rnd() & 3) == 0) v = (uint16_t)(0x5998 + (rnd() & 1));
    if((rnd() & 3) == 0) v = (uint16_t)(v & 0x5FFF);
    wr16ram(ram, c ? 0x005D : 0x005A, v);
    wr16ram(ram, c ? 0x0062 : 0x0060, (uint16_t)(1 + rnd() % 3));
  }
  static const uint8_t firsts[8] = {0, 1, 4, 5, 0x30, 0x6B, 0x6C, 0x90};
  static const uint8_t seconds[8] = {0, 0x70, 0x74, 0x75, 0xB0, 0xDB, 0xDC, 0xF8};
  static const uint8_t slots[4] = {0x02, 0x04, 0x08, 0x80};
  for(int i = 0; i < 2; i++) {
    const uint16_t band = (uint16_t)(2 * (rnd() % 8) + 16 * i), at = (uint16_t)(2 * (rnd() % 8) + 16 * i);
    wr16ram(ram, i ? 0x00C0 : 0x00BE, band);
    wr16ram(ram, i ? 0x00C6 : 0x00C4, at);
    ram[0x0486 + band] = (rnd() & 1) ? firsts[rnd() % 8] : (uint8_t)rnd();
    ram[0x0486 + band + 1] = (rnd() & 3) == 0 ? 0 : (uint8_t)rnd();
    uint8_t mask = 0;
    for(int s = 0; s < 4; s++) if(rnd() & 1) mask |= slots[s];
    ram[0x0596 + band] = mask;
    ram[0x050E + at] = (rnd() & 1) ? seconds[rnd() % 8] : (uint8_t)rnd();
    ram[0x050E + at + 1] = (rnd() & 3) == 0 ? 0 : (uint8_t)rnd();
    ram[0x061E + at] = (uint8_t)rnd();
  }
  wr16ram(ram, 0x0024, (uint16_t)rnd());
  wr16ram(ram, 0x047C, (uint16_t)rnd());
  for(uint16_t a = 0x4E2D; a <= 0x4E3C; a++) ram[a] = (uint8_t)rnd();
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

// In turn: the output at $7F:2100 after 8 KB of random bytes; the output near $7F:0000 with
// back references reaching before it (a word read at $FFFF takes its high byte from
// $80:0000, low WRAM: the end of bank $7F and WRAM $0000-$0001 random). An output running
// across $FFFF is not fuzzed (the game has none; the C does not model its writes there)
static void fuzz_decompress(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  static unsigned turn;
  const uint16_t y = (turn++ % 2) ? (uint16_t)(rnd() % 0x20) : 0x2100;
  for(int i = 0; i < 0x2100; i++) ram[0x10000 + i] = (uint8_t)rnd();
  for(int i = 0xC000; i < 0x10000; i++) ram[0x10000 + i] = (uint8_t)rnd();
  ram[0x0000] = (uint8_t)rnd();
  ram[0x0001] = (uint8_t)rnd();
  fuzz_decompress_stream(ram, rnd, y, true);
  r->a = 0x7F7E;
  r->x = 0x4000;
  r->y = y;
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
  out->a = take_nibble(in->d);
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
  out->c = calleeRun ? calleeRun(in->d) : func_83FE9A(in->d, in->a, in->x, in->y);
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
// sprite_text_menu: as sprite_text; A's high byte is the $01 of its LDA #$0100 once a glyph
// was written
static void call_sprite_text_menu(const Regs* in, Regs* out) {
  uint32_t str = ((uint32_t)(in->a & 0xFF) << 16) | in->x;
  uint16_t slot = rd16(DP + 0x74);
  out->y = sprite_text_menu(in->d, str, in->y);
  uint16_t n = 0;
  while(bus_rd8((str & 0xFF0000) | (uint16_t)(str + n)) != 0) n++;
  out->x = (uint16_t)(in->x + n);
  out->a = rd16(DP + 0x74) != slot ? 0x0100 : (uint16_t)(in->a & 0xFF00);
  setNZ16(out, out->y);
}
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
  out->y = sprite_number(in->d, value, in->y);
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
  uint16_t shift = sprite_number_right(in->d, value, in->y);
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
  uint16_t shift = sprite_number_centered(in->d, value, in->y);
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
// menu_panel_sprites returns after its last SBC #8 (C clear: the borrow that ends it), X =
// D+$74, Y = D+$C4; A that SBC's result (the frame's half height - 8 when too short for an
// edge)
static void call_menu_panel_sprites(const Regs* in, Regs* out) {
  (void)in;
  menu_panel_sprites(in->d);
  const uint16_t half = (uint16_t)(rd16(DP + 0x7E) >> 1);
  const uint16_t prev = half < 8 ? half : (uint16_t)(rd16(DP + 0x80) + 8);
  out->x = rd16(DP + 0x74);
  out->y = rd16(DP + 0xC4);  // (its LDY D+$C4 for the colour word)
  out->a = (uint16_t)(prev - 8);
  setNZ16(out, out->a);
  setSbc16(out, prev, 0x0008);
}
// menu_frame_sprites ends in sprite_group's JSR, then RTS: its outputs
static void call_menu_frame_sprites(const Regs* in, Regs* out) {
  const GroupWalk w = sprite_group_walk(0x81BF40);
  sprite_group_outputs(out, in, w, menu_frame_sprites(in->d));
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
  // the slot: the game's (a few sprites in) or, one case in four, from $F500 up, where the
  // stores at $0900/$0B00 + the slot run into the next bank (bank $7F here)
  wr16ram(ram, (uint16_t)(r->d + 0x74), (rnd() % 4 == 0) ? (uint16_t)(0xF500 + 4 * (rnd() % 0x2C0)) : (uint16_t)(4 * (rnd() % 32)));
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
// When set, func_81FBB1's adapter runs this at its call instead of the routine (a caller
// whose exit is that call's: see car_sync_player).
static bool (*fbb1Run)(void) = NULL;
static void call_func_81FBB1(const Regs* in, Regs* out) {
  uint16_t car = in->y;
  bool first = !(rd16(CAR_FLAGS + (uint32_t)car) & 0x40);
  bool jumped = fbb1Run ? fbb1Run() : func_81FBB1(car);
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
static void call_player_road_limits(const Regs* in, Regs* out) { (void)out; player_road_limits(in->d); }

// Negative test: the bend flag's bit read as bit 1
static void call_player_road_limits_wrong(const Regs* in, Regs* out) {
  (void)in;
  (void)out;
  uint16_t bend = rd16(0x1F2C);
  wr16(0x1F2C, (uint16_t)(bend ^ 0x0001));
  player_road_limits(in->d);
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
SHOP_LEVEL(shop_part_level_8)  // lists 6-10's: no +1, the LDA's N/Z
SHOP_LEVEL(shop_part_level_9)
SHOP_LEVEL(shop_part_level_a)
SHOP_LEVEL(shop_part_level_b)
SHOP_LEVEL(shop_part_level_c)
static void fuzz_shop_part_level(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->y = (uint16_t)(rnd() % 0x40);
  for(int i = 0; i < 2; i++) wr16ram(ram, (uint32_t)(0x0392 + r->y + 2 * i), (uint16_t)rnd());
  for(uint32_t a = 0x0392; a < 0x03E0; a++) ram[a] = (uint8_t)rnd();
}

// A = 4 (Z from the TSB: bit 2 of $76 before), X = the car
static void call_shop_money_take(const Regs* in, Regs* out) {
  (void)in;
  bool was = rd16(DP + 0x76) & 0x0004;
  shop_money_take(in->d);
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

// ---- shop_money_after_price / shop_money_roll / shop_money_store ($80:9957, $80:99D1,
// $80:9ABB) ----  the buying's money (BCD: the game's money always is, and so is the fuzz's).
// X = D+$94 on every exit. after_price: A = 4 (LDA #4), Y = the digits left unchecked.
// roll: A as the original's last load, Z set and N clear (the final DEC of D+$84 to 0).
// store: A = D+$54 (its LDA: N/Z).
static uint16_t bcd16_random(uint32_t (*rnd)(void)) {
  uint16_t v = 0;
  for(int k = 0; k < 16; k += 4) v = (uint16_t)(v | (uint16_t)((rnd() % 10) << k));
  return v;
}
static void call_shop_money_after_price(const Regs* in, Regs* out) {
  (void)in;
  ShopMoneyFlags f;
  out->y = shop_money_after_price_flags(in->d, &f);
  out->c = f.c;
  out->v = f.v;
  out->a = 4;
  out->x = rd16(DP + 0x94);
  out->n = out->z = false;
}
// the refund (A = the row): its JMP into shop_money_after_price's tail ($80:997E), so that
// tail's exit (shop-27)
static void call_shop_money_after_refund(const Regs* in, Regs* out) {
  ShopMoneyFlags f;
  out->y = shop_money_after_refund_flags(in->d, in->a, &f);
  out->c = f.c;
  out->v = f.v;
  out->a = 4;
  out->x = rd16(DP + 0x94);
  out->n = out->z = false;
}
static void fuzz_shop_money_after_price(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
static void fuzz_shop_money_after_refund(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_shop_money_after_price(r, ram, rnd);
  const uint16_t list = (uint16_t)(ram[DP + 0x48] | ram[DP + 0x49] << 8);
  r->a = (uint16_t)(list == 0 || list >= 6 ? 0 : rnd() % 5);  // a row of the list
}
static void call_shop_money_roll(const Regs* in, Regs* out) {
  ShopMoneyFlags f = {in->c, in->v};
  out->a = shop_money_roll_flags(in->d, &f);
  out->c = f.c;
  out->v = f.v;
  out->x = rd16(DP + 0x94);
  out->y = in->y;
  out->n = false;
  out->z = true;
}
static void call_shop_money_store(const Regs* in, Regs* out) {
  shop_money_store(in->d);
  out->a = rd16(DP + 0x54);
  out->x = rd16(DP + 0x94);
  out->y = in->y;
  setNZ16(out, out->a);
}
// the car's money ($038E/$0390 + D+$94) and the target D+$52/$54 in BCD; for the roll the
// settled digits (D+$56/$58: each digit all or nothing, mostly from the top) and D+$5A 1-4
static void fuzz_shop_money(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  const uint16_t car = (uint16_t)(rnd() % 0x40);
  wr16ram(ram, DP + 0x94, car);
  wr16ram(ram, (uint16_t)(0x038E + car), bcd16_random(rnd));
  wr16ram(ram, (uint16_t)(0x0390 + car), (rnd() & 1) ? bcd16_random(rnd) : (uint16_t)(rnd() % 10));
  wr16ram(ram, DP + 0x52, bcd16_random(rnd));
  wr16ram(ram, DP + 0x54, (rnd() & 1) ? bcd16_random(rnd) : (uint16_t)(rnd() % 10));
  uint32_t settled = 0;
  const int top = (int)(rnd() % 9);
  for(int k = 0; k < 8; k++) if(k < top || (rnd() % 6) == 0) settled |= 0xF0000000u >> (4 * k);
  wr16ram(ram, DP + 0x56, (uint16_t)settled);
  wr16ram(ram, DP + 0x58, (uint16_t)(settled >> 16));
  wr16ram(ram, DP + 0x5A, (uint16_t)(1 + rnd() % 4));
}
// the current item: lists 0-8 (0 and 6-8 one row each), the car's money in BCD
static void fuzz_shop_money_after_price(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_shop_money(r, ram, rnd);
  const uint16_t list = (uint16_t)(rnd() % 9);
  wr16ram(ram, DP + 0x48, list);
  wr16ram(ram, DP + 0x4A, (uint16_t)(list == 0 || list >= 6 ? 0 : rnd() % 5));
}

// ---- the buying handlers ($80:9053 ... $80:95C0, $80:930A) ----  waiting routines (the panel
// moves a frame at a time): no fuzz. Lists 2-5 end in shop_item_icon_settle (JSR, RTS): its
// flags (N set by its LDA #$8000, Z clear: shop_item_show_icon set D+$76 bit 15, C set);
// list 1 ends at its JMP to it (endAt). The money short: shop_flag_fb's PLA of the A
// shop_item_affordable left, the redraw's last (oam_finish: the new D+$6C, $0900 or $0D00:
// N and Z clear), C clear. Short after a refund: shop_money_after_price's exit.
static void call_shop_buy(const Regs* in, Regs* out, uint16_t field, uint16_t height, bool price_first, bool settle) {
  ShopMoneyFlags f = {in->c, in->v};
  switch(shop_buy_part(in->d, field, height, price_first, settle, &f)) {
    case SHOP_BUY_OWNED:
    case SHOP_BUY_BOUGHT:
      out->n = true;
      out->z = false;
      out->c = true;
      break;
    case SHOP_BUY_REFUNDED_SHORT:  // lists 2-5: shop_money_after_price last (A = 4); list 1:
                                   // shop_flag_fb's PLA of that same A
      out->n = out->z = false;
      out->c = f.c;
      out->v = f.v;
      break;
    default:
      setNZ16(out, rd16(DP + 0x6C));
      out->c = false;
      break;
  }
}
// ---- the password screen's grid cursor ($8A:B4D2-B611), caret and lines ----  each exit is
// modelled from the state before the call: A and the flags of the path's last LDA/CMP/ADC/SBC
// (V kept where none of them ran)
static bool pwNotBelow(uint16_t a, uint16_t b) { return !((uint16_t)(a - b) & 0x8000); }
static void pwCmpExit(Regs* out, uint16_t a, uint16_t b) {  // a CMP that ended the path
  out->a = a;
  setNZ16(out, (uint16_t)(a - b));
  out->c = a >= b;
}
static void pwRightExit(Regs* out, uint16_t x, uint16_t y) {
  if(y == 0x00AC && pwNotBelow(x, 0xFFA4)) { pwCmpExit(out, x, 0xFFA4); return; }
  if(!pwNotBelow(x, 0x003C)) { out->a = 6; setNZ16(out, 6); setAdc16(out, x, 0x10); return; }
  if(x == 0x0064) { pwCmpExit(out, x, 0x0064); return; }
  out->a = 0x26;
  setNZ16(out, 0x26);
  setAdc16(out, x, 0x28);
}
static void pwLeftExit(Regs* out, uint16_t x) {
  if(x == 0x0064) { out->a = 6; setNZ16(out, 6); setSbc16(out, x, 0x28); return; }
  if(x == 0xFF8C) { pwCmpExit(out, x, 0xFF8C); return; }
  out->a = (uint16_t)(x - 0x10);
  setNZ16(out, out->a);
  setSbc16(out, x, 0x10);
}
static void pwUpExit(Regs* out, uint16_t y) {
  if(y == 0x0064) { pwCmpExit(out, y, 0x0064); return; }
  out->a = (uint16_t)(y - 0x18);
  setNZ16(out, out->a);
  setSbc16(out, y, 0x18);
}
static void pwDownExit(Regs* out, uint16_t x, uint16_t y) {
  if(y == 0x0094 && pwNotBelow(x, 0xFFB4)) { pwCmpExit(out, x, 0xFFB4); return; }
  if(y == 0x00AC) { pwCmpExit(out, y, 0x00AC); return; }
  out->a = (uint16_t)(y + 0x18);
  setNZ16(out, out->a);
  setAdc16(out, y, 0x18);
}
static void call_password_cursor_right(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t x = rd16(0x0D88), y = rd16(0x0D90);
  password_cursor_right();
  pwRightExit(out, x, y);
}
static void call_password_cursor_left(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t x = rd16(0x0D88);
  password_cursor_left();
  pwLeftExit(out, x);
}
static void call_password_cursor_up(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t y = rd16(0x0D90);
  password_cursor_up();
  pwUpExit(out, y);
}
static void call_password_cursor_down(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t x = rd16(0x0D88), y = rd16(0x0D90);
  password_cursor_down();
  pwDownExit(out, x, y);
}
// a pad handler: no press, the last AND (A = 0: Z); the cursor moving, the CMP of where it
// goes against where it is; else the step's exit
static int pwPadState(uint16_t button, uint16_t to, uint16_t at) {
  if(!(rd16(0x000E) & button) && !(rd16(0x0012) & button) && !(rd16(0x0016) & button) && !(rd16(0x001A) & button))
    return 0;
  return rd16(to) == rd16(at) ? 2 : 1;
}
static void pwPadExit(Regs* out, int state, uint16_t to, uint16_t at) {
  if(state == 0) {
    out->a = 0;
    setNZ16(out, 0);
  } else {
    pwCmpExit(out, to, at);
  }
}
static void call_password_pad_right(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t x = rd16(0x0D88), y = rd16(0x0D90), at = rd16(0x0D80);
  const int s = pwPadState(0x0100, 0x0D88, 0x0D80);
  password_pad_right();
  if(s == 2) pwRightExit(out, x, y);
  else pwPadExit(out, s, x, at);
}
static void call_password_pad_left(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t x = rd16(0x0D88), at = rd16(0x0D80);
  const int s = pwPadState(0x0200, 0x0D88, 0x0D80);
  password_pad_left();
  if(s == 2) pwLeftExit(out, x);
  else pwPadExit(out, s, x, at);
}
static void call_password_pad_up(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t y = rd16(0x0D90), at = rd16(0x0D68);
  const int s = pwPadState(0x0800, 0x0D90, 0x0D68);
  password_pad_up();
  if(s == 2) pwUpExit(out, y);
  else pwPadExit(out, s, y, at);
}
static void call_password_pad_down(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t x = rd16(0x0D88), y = rd16(0x0D90), at = rd16(0x0D68);
  const int s = pwPadState(0x0400, 0x0D90, 0x0D68);
  password_pad_down();
  if(s == 2) pwDownExit(out, x, y);
  else pwPadExit(out, s, y, at);
}
// the typing ($8A:B403-B4D1). After password_text_draw: A and the flags of
// password_caret_place's ADC, X the last text_put_char's tilemap offset (D+$38 * $40 + D+$36,
// the doubled column it leaves), Y kept. A beep after it (func_8AA48D) keeps A (its PLA: N/Z)
static void pwDrawExit(Regs* out) {
  const uint16_t row = (uint16_t)(rd16(DP + 0x3C) << 3);
  out->a = (uint16_t)(row + 0x0F);
  setNZ16(out, out->a);
  setAdc16(out, row, 0x0F);
  out->x = (uint16_t)((rd16(DP + 0x38) << 6) + rd16(DP + 0x36));
}
static void pwRefusalExit(Regs* out) {  // func_8AA4AF: LDA #$0A8C last
  out->a = 0x0A8C;
  setNZ16(out, 0x0A8C);
}
static void pwTypeExit(Regs* out, uint16_t count) {
  if(count == 0x15) {
    pwRefusalExit(out);
    out->c = true;  // the CPY #$15 that matched
    out->y = 0x15;
    return;
  }
  pwDrawExit(out);
  out->y = count;
}
static void pwDeleteExit(Regs* out, uint16_t count) {
  if(count == 0) {
    pwRefusalExit(out);
    return;
  }
  pwDrawExit(out);
  out->y = (uint16_t)(count - 1);
}
static void call_password_type_char(const Regs* in, Regs* out) {
  const uint16_t count = rd16(DP + 0x80);
  password_type_char((uint8_t)in->a);
  pwTypeExit(out, count);
}
static void call_password_delete(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t count = rd16(DP + 0x80);
  password_delete();
  pwDeleteExit(out, count);
}
static void call_password_text_draw(const Regs* in, Regs* out) {
  (void)in;
  password_text_draw();
  pwDrawExit(out);
}
// none: the DEX/BPL that ended the loop (X = $FFFF), SEC; one: the CMP that matched, CLC. A's
// low byte the last character read
static void call_password_no_vowel(const Regs* in, Regs* out) {
  const bool none = password_no_vowel();
  uint16_t i = 3;  // the original reads from the fourth down: the highest vowel
  for(; !none; i--) {
    const uint8_t ch = rd8(0x0D40 + (uint32_t)i);
    if(ch == 'A' || ch == 'E' || ch == 'I' || ch == 'O' || ch == 'U') break;
  }
  out->x = none ? 0xFFFF : i;
  out->a = (uint16_t)((in->a & 0xFF00) | rd8(0x0D40 + (uint32_t)(none ? 0 : i)));
  out->n = none;
  out->z = !none;
  out->c = none;
}
// not on the last column: the CMP #$64 of x; EXIT: func_8AA49E keeps A (y), SEC; DEL and
// SPACE: their exits, CLC; any other row: the CMP #$64 of y
typedef struct { uint16_t x, y, count; } PwLastColumn;  // the state it reads, before the call
static PwLastColumn pwLastColumnBefore(void) {
  return (PwLastColumn){rd16(0x0D80), rd16(0x0D68), rd16(DP + 0x80)};
}
static void pwLastColumnExit(Regs* out, PwLastColumn s, bool exit) {
  if(s.x != 0x0064) {
    pwCmpExit(out, s.x, 0x0064);
  } else if(s.y == 0x0094) {
    out->a = s.y;
    setNZ16(out, s.y);
  } else if(s.y == 0x007C) {
    pwDeleteExit(out, s.count);
  } else if(s.y == 0x0064) {
    pwTypeExit(out, s.count);
  } else {
    pwCmpExit(out, s.y, 0x0064);
  }
  out->c = exit;
}
static void call_password_press_last_column(const Regs* in, Regs* out) {
  (void)in;
  const PwLastColumn s = pwLastColumnBefore();
  pwLastColumnExit(out, s, password_press_last_column());
}
// no press: the last TRB (Z) after LDA #$C0C0 (N), CLC. A press: password_press_last_column's
// exit, and past it the ADC #$74 of x (V) and the path's last load or compare, CLC; on a letter
// password_type_char's exit
// (password_entry_frame ends on the same exit: `fn` is either)
static void pressAOut(Regs* out, bool (*fn)(void)) {
  const bool pressed = ((rd16(0x000E) | rd16(0x0012) | rd16(0x0016) | rd16(0x001A)) & 0xC0C0) != 0;
  const PwLastColumn s = pwLastColumnBefore();
  const uint16_t row = rd16(0x0D90);
  const bool exit = fn();
  if(!pressed) {
    out->a = 0xC0C0;
    out->n = out->z = true;
    out->c = false;
    return;
  }
  pwLastColumnExit(out, s, exit);
  if(exit) return;
  const uint16_t at = (uint16_t)(s.x + 0x74);
  setAdc16(out, s.x, 0x74);
  out->c = false;
  if(at & 0x000F) {
    out->a = at;
    setNZ16(out, at);
    return;
  }
  out->y = (uint16_t)(at >> 4);
  if(pwNotBelow(s.x, 0x003D)) {
    pwCmpExit(out, s.x, 0x003D);
  } else if(row == 0x0064 || row == 0x007C || row == 0x0094 || row == 0x00AC) {
    pwTypeExit(out, s.count);
  } else {
    pwCmpExit(out, row, 0x00AC);
  }
  out->c = false;
}
static void call_password_press_a(const Regs* in, Regs* out) {
  (void)in;
  pressAOut(out, password_press_a);
}
// the password entry's parts: the typing frame ends at its BCS on password_press_a's exit
// (pressAOut); the check at the code search's JSR (C set: the checksum failed) or returns
// after its last STA of $03DC (a valid password: A = $03DC, N Z of its ORA; C func_8ABF76's,
// V clear)
static void call_password_entry_frame(const Regs* in, Regs* out) {
  (void)in;
  pressAOut(out, password_entry_frame);
}
static void call_password_entry_open(const Regs* in, Regs* out) { (void)in; (void)out; password_entry_open(); }
static void call_password_entry_sprites(const Regs* in, Regs* out) { (void)in; (void)out; password_entry_sprites(); }
static void call_password_entry_slide(const Regs* in, Regs* out) { (void)in; (void)out; password_entry_slide(); }
static void call_password_entry_ready(const Regs* in, Regs* out) { (void)in; (void)out; password_entry_ready(); }
static void call_password_entry_pads(const Regs* in, Regs* out) { (void)in; (void)out; password_entry_pads(); }
static void call_password_entry_check(const Regs* in, Regs* out) {
  (void)in;
  bool carry = false;
  if(password_entry_check(&carry)) {
    out->a = rd16(0x03DC);
    setNZ16(out, out->a);
    out->c = carry;
    out->v = false;
  } else {
    out->c = true;
  }
}
// the typed text (the password alphabet, a vowel now and then), its count (full or empty one
// time in four), the text cell's row, the font
static void fuzz_password_typing(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  static const char alphabet[] = "BCDFGHJKLN?PQRSTVWXYZ0123456789M.!AEIOU ";
  const uint32_t k = rnd() % 8;
  const uint16_t count = k == 0 ? 0x15 : k == 1 ? 0 : (uint16_t)(rnd() % 0x15 + 1);
  for(uint32_t i = 0; i < 0x16; i++)
    ram[0x0D40 + i] = i < count ? (uint8_t)alphabet[rnd() % ((rnd() & 3) ? 33 : (sizeof alphabet - 1))] : 0;
  wr16ram(ram, DP + 0x80, count);
  wr16ram(ram, DP + 0x3C, (uint16_t)(rnd() % 16));
  wr16ram(ram, TEXT_FONT_SEL, (rnd() & 3) == 0 ? 0 : 1);
  r->a = (uint16_t)((r->a & 0xFF00) | (uint8_t)alphabet[rnd() % (sizeof alphabet - 1)]);
  const uint32_t at = rnd() % 4;
  wr16ram(ram, 0x0D80, at ? 0x0064 : (uint16_t)rnd());
  wr16ram(ram, 0x0D68, at == 1 ? 0x0094 : at == 2 ? 0x007C : at == 3 ? 0x0064 : (uint16_t)rnd());
}

// the cursor on the grid (three in four) or anywhere; where it is: where it goes, or not
static uint16_t pwGridX(uint32_t (*rnd)(void)) {
  const uint32_t k = rnd() % 16;
  return k < 12 ? (uint16_t)(0xFF8C + 0x10 * k) : k < 14 ? 0x0064 : (uint16_t)rnd();
}
static uint16_t pwGridY(uint32_t (*rnd)(void)) {
  const uint32_t k = rnd() % 6;
  return k < 4 ? (uint16_t)(0x0064 + 0x18 * k) : (uint16_t)rnd();
}
// ---- the password read back ($8A:BCEA, $8A:BD25) ----
static uint16_t pwAlphabetIndex(uint8_t ch) {
  uint16_t y = 0;
  while(bus_rd8(0x8ABC5F + (uint32_t)y) != ch) y++;
  return y;
}
static uint16_t pwTypedLength(void) {
  uint16_t n = 0;
  while(rd8((uint16_t)(0x0D40 + n)) != 0) n++;
  return n;
}
// the last DEX (X below 0: N), A the index of the last character pushed (at 0, or below the
// text when it is shorter than three), Y 5; C/V of func_8ABBF4's last checksum ADC (as its
// adapter models it)
static void call_password_decode(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t start = (uint16_t)(pwTypedLength() - 3);
  const uint16_t last = (start & 0x8000) ? start : 0;
  const uint16_t index = pwAlphabetIndex(rd8((uint16_t)(0x0D40 + last)));
  password_decode();
  out->a = index;
  out->x = (uint16_t)(last - 1);
  out->y = 5;
  setNZ16(out, out->x);
  const uint16_t low = rd16(0x0DCA);
  setAdc16(out, (uint16_t)(rd16(DP + 0x92) - low), low);
}
// A the two characters' value, X where they are, Y the second's index; N/Z of the CMP with
// the checksum, C clear when it matched (CLC), else set (SEC)
static void call_password_checksum_ok(const Regs* in, Regs* out) {
  (void)in;
  const bool ok = password_checksum_ok();
  uint16_t x = 0;
  while(rd8((uint16_t)(0x0D42 + x)) != 0) x++;
  out->a = rd16(DP + 0x42);
  out->x = x;
  out->y = pwAlphabetIndex(rd8((uint16_t)(0x0D41 + x)));
  setNZ16(out, (uint16_t)(out->a - rd16(DP + 0x92)));
  out->c = !ok;
}
// a typed text (the alphabet, the grid's other characters now and then, 0-21 long), the
// checksum D+$92 matching its last two one time in two
static void fuzz_password_read(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  static const char grid[] = "BCDFGHJKLN?PQRSTVWXYZ0123456789MAEIOU.! ";
  const uint16_t len = (uint16_t)(rnd() % 22);
  for(uint32_t i = 0; i < 0x18; i++)
    ram[0x0D40 + i] = i < len ? (uint8_t)grid[rnd() % ((rnd() & 7) ? 32 : (sizeof grid - 1))] : 0;
  for(int i = 0; i < 32; i++) ram[0x0DAC + i] = (uint8_t)rnd();
  wr16ram(ram, DP + 0x84, (uint16_t)rnd());
  wr16ram(ram, DP + 0x48, (uint16_t)rnd());
  uint16_t sum = (uint16_t)rnd();
  if(rnd() & 1) {
    const uint16_t at = len >= 2 ? (uint16_t)(len - 2) : 0;
    sum = (uint16_t)((sum & 0xFC00) | pwAlphabetIndex(ram[0x0D40 + at]) | (uint16_t)(pwAlphabetIndex(ram[0x0D41 + at]) << 5));
  }
  wr16ram(ram, DP + 0x92, sum);
}

// password_unpack: A the last field's word as written, X below the table ($FFFA: N/Z of the
// last DEX), Y the first field's shift, C the last asl_n's, V kept
static void call_password_unpack(const Regs* in, Regs* out) {
  password_unpack(in->c);
  const uint32_t field = (uint32_t)rd8(DP + 0x44) << 16 | rd16(DP + 0x42);
  out->a = bus_rd16(field);
  out->x = 0xFFFA;
  setNZ16(out, out->x);
  const uint16_t shift = (uint16_t)(rom16(0x8ACF65) & 0x00FF);
  out->y = shift;
  // the carry of the last asl_n: the width's mask (func_8ABB5C leaves it in D+$46) shifted
  bool carry;
  const uint16_t width = (uint16_t)(rom16(0x8ACF65) >> 8);
  const uint16_t mask = (uint16_t)(width >= 16 ? 0xFFFF : (1u << width) - 1);
  (void)asl_n(mask, shift, &carry);
  out->c = carry;
}
// the big number, the fields, the entry carry
static void fuzz_password_unpack(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_password_pack(r, ram, rnd);
  for(int i = 0; i < 32; i++) ram[0x0DAC + i] = (uint8_t)rnd();
  wr16ram(ram, DP + 0x46, (uint16_t)rnd());
  r->c = rnd() & 1;
}

// the screen's way in and out: waiting rows. C and V as the last wait leaves them (the ADC of
// 1 and the frame counter: V when the count reaches $8000); N/Z of the last LDA or INC; A
// fade_in_start's $80, kept by wait_one_frame
static bool pwWaitV(void) { return rd16(FRAME_COUNTER) == 0x8000; }
static void call_password_screen_open(const Regs* in, Regs* out) {
  (void)in;
  password_screen_open();
  out->a = 0x0080;
  setNZ16(out, rd16(TEXT_FONT_SEL));
  out->c = true;
  out->v = pwWaitV();
}
static void call_password_screen_close(const Regs* in, Regs* out) {
  (void)in;
  password_screen_close();
  out->a = 0x0080;
  setNZ16(out, rd16(DP + 0x8E));
  out->c = true;
  out->v = pwWaitV();
}

// ---- the password codes ($8A:B35B, $8A:C013) ----
// password_code_find, up to its JMP ($0182) (endAt $8A:B3A4, after the JSR that pushed its
// return $8A:B3A1): a match leaves C set and N/Z of the PLX of the entry's index; no match
// returns by the LDY of the table's 0 (Z), CLC
static void call_password_code_find(const Regs* in, Regs* out) {
  const uint16_t handler = password_code_find();
  if(handler) {
    uint16_t x = 0;  // the index after the matching entry's handler word
    for(;;) {
      const uint16_t h = rom16(0x8ACDAB + (uint32_t)x);
      x = (uint16_t)(x + 2);
      if(h == handler) break;
      while(rom8(0x8ACDAB + (uint32_t)x)) x++;
      x++;
    }
    setNZ16(out, x);
    out->c = true;
    wr16((uint16_t)(in->sp - 1), 0xB3A1);
    return;
  }
  out->n = false;
  out->z = true;
  out->c = false;
}
// a typed text: a code (one time in two, any of them but RELAX's screen), a near miss, or the
// password alphabet
static void fuzz_password_code_find(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_password_typing(r, ram, rnd);
  if(rnd() & 1) {
    uint16_t x = 0, n = (uint16_t)(1 + rnd() % 16);
    for(uint16_t k = 0; k < n; k++) {  // the n-th entry
      x = (uint16_t)(x + 2);
      while(rom8(0x8ACDAB + (uint32_t)x)) x++;
      x++;
    }
    x = (uint16_t)(x + 2);
    uint16_t y = 0;
    for(; rom8(0x8ACDAB + (uint32_t)x + y); y++) ram[0x0D40 + y] = (uint8_t)(rom8(0x8ACDAB + (uint32_t)x + y) ^ 0x9A);
    ram[0x0D40 + y] = 0;
    if((rnd() & 3) == 0 && y > 0) ram[0x0D40 + rnd() % y] ^= 1;  // a near miss
  }
}
// INFODATA's handler: A = 4 (its LDA), Z of the TSB (bit 2 of $03E2 before)
static void call_password_code_infodata(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t before = rd16(0x03E2);
  password_code_infodata();
  out->a = 0x0004;
  out->n = false;
  out->z = (before & 0x0004) == 0;
}
static void fuzz_password_code_infodata(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x03E2, (uint16_t)rnd());
}

// the typing's state, with a button (A, B, X or Y one time in two) on one of the pads
static void fuzz_password_press_a(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_password_typing(r, ram, rnd);
  const uint16_t x = pwGridX(rnd), y = pwGridY(rnd);
  wr16ram(ram, 0x0D80, x);
  wr16ram(ram, 0x0D88, x);
  wr16ram(ram, 0x0D90, y);
  wr16ram(ram, 0x0D68, (rnd() & 3) ? y : (uint16_t)rnd());
  for(uint32_t a = 0x000E; a <= 0x001A; a += 4) wr16ram(ram, a, (uint16_t)(rnd() & 0x3F3F));
  if(rnd() & 1) wr16ram(ram, 0x000E + 4 * (rnd() % 4), (uint16_t)(rnd() | (0x40 << (rnd() % 2)) << (8 * (rnd() % 2))));
}
static void fuzz_password_cursor(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  const uint16_t x = pwGridX(rnd), y = pwGridY(rnd);
  wr16ram(ram, 0x0D88, x);
  wr16ram(ram, 0x0D90, y);
  wr16ram(ram, 0x0D80, (rnd() & 3) ? x : (uint16_t)rnd());
  wr16ram(ram, 0x0D68, (rnd() & 3) ? y : (uint16_t)rnd());
  wr16ram(ram, 0x0D78, (uint16_t)rnd());
  for(uint32_t a = 0x000E; a <= 0x001A; a += 4)  // the four pads' new presses: a direction one time in four
    wr16ram(ram, a, (rnd() & 3) == 0 ? (uint16_t)(0x0100 << (rnd() % 4)) : (uint16_t)(rnd() & 0xF0FF));
}
static void call_password_caret_place(const Regs* in, Regs* out) {
  (void)in;
  password_caret_place();
  const uint16_t row = (uint16_t)(rd16(DP + 0x3C) << 3);
  out->a = (uint16_t)(row + 0x0F);
  setNZ16(out, out->a);
  setAdc16(out, row, 0x0F);
}
static void fuzz_password_caret_place(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x3A, (rnd() & 3) ? (uint16_t)(rnd() % 32) : (uint16_t)rnd());
  wr16ram(ram, DP + 0x3C, (rnd() & 3) ? (uint16_t)(rnd() % 32) : (uint16_t)rnd());
}
// odd: the AND #1 (A = 1); even: LDA #$FFFF
static void call_password_caret_blink(const Regs* in, Regs* out) {
  (void)in;
  password_caret_blink();
  out->a = (rd16(DP + 0x66) & 1) ? 1 : 0xFFFF;
  setNZ16(out, out->a);
}
static void fuzz_password_caret_blink(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x66, (uint16_t)rnd());
}
// the last text_print_centered's exit (as its adapter models it), X kept
static void call_password_lines_print(const Regs* in, Regs* out) {
  password_lines_print();
  out->y = rd16(DP + 0x5A);
  const uint16_t f = rd16(DP + 0x86);
  out->a = f ? f : ((rd16(DP + 0x5A) & 1) ? 4 : 0);
  setNZ16(out, in->x);
  const uint16_t half = (uint16_t)((rd16(DP + 0x40) - rd16(DP + 0x3E)) >> 1);
  if(f) setAdc16(out, (uint16_t)(half - rd16(DP + 0x46)), rd16(DP + 0x3E));
  else setSbc16(out, rd16(DP + 0x3C), 10);
}
static void fuzz_password_lines_print(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  const uint16_t a = r->a;
  fuzz_text_print_centered(r, ram, rnd);
  r->a = a;  // the strings are the ROM's
}

// lists 6-8: as lists 2-5
static void call_shop_buy_upgrade(const Regs* in, Regs* out, uint16_t field) {
  ShopMoneyFlags f = {in->c, in->v};
  switch(shop_buy_upgrade(in->d, field, &f)) {
    case SHOP_BUY_OWNED:
    case SHOP_BUY_BOUGHT:
      out->n = true;
      out->z = false;
      out->c = true;
      break;
    case SHOP_BUY_REFUNDED_SHORT:
      out->n = out->z = false;
      out->c = f.c;
      out->v = f.v;
      break;
    default:
      setNZ16(out, rd16(DP + 0x6C));
      out->c = false;
      break;
  }
}
static void call_shop_buy_list6(const Regs* in, Regs* out) { call_shop_buy_upgrade(in, out, 5); }
static void call_shop_buy_list7(const Regs* in, Regs* out) { call_shop_buy_upgrade(in, out, 6); }
static void call_shop_buy_list8(const Regs* in, Regs* out) { call_shop_buy_upgrade(in, out, 7); }
static void call_shop_buy_list9(const Regs* in, Regs* out) { call_shop_buy_upgrade(in, out, 9); }
// the dispatch, up to its RTS into the handler (endAt): A the handler's address - 1 (N/Z of
// the DEC), X the entry, C/V of the ADC that made it. The switch from the word to a handler
// is not compared here: each handler's row checks its own C
static void call_shop_buy_dispatch(const Regs* in, Regs* out) {
  const uint16_t base = rom16(0x81A892 + (uint32_t)(uint16_t)(rd16(DP + 0x48) << 1));
  const uint16_t rows = (uint16_t)(rd16(DP + 0x4A) << 4);
  out->a = (uint16_t)(shop_buy_handler(in->d) - 1);
  setNZ16(out, out->a);
  // its two pushes, the RTS trick: the return to its own RTS, then the handler's address - 1
  wr16((uint16_t)(in->sp - 1), 0x9051);
  wr16((uint16_t)(in->sp - 3), out->a);
  out->x = (uint16_t)(rows + base);
  setAdc16(out, rows, base);
}
// the list (0-10, the table's) and the row (0-7). Off those, the entry can run past bank $81
// into $82's WRAM and I/O mirror, which the C (rom16) does not model: not a game state
static void fuzz_shop_buy_dispatch(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x48, (uint16_t)(rnd() % 11));
  wr16ram(ram, DP + 0x4A, (uint16_t)(rnd() % 8));
}
// list 1's row ends at its JMP to the settle: owned, shop_item_show_icon's N/Z (D+$44);
// bought, shop_money_store's (its LDA of D+$54); C set and V clear either way (the redraw's
// last SBC #$0900 in oam_finish); short, as the others
static void call_shop_buy_list1(const Regs* in, Regs* out) {
  ShopMoneyFlags f = {in->c, in->v};
  switch(shop_buy_part(in->d, 0, 0x0078, true, false, &f)) {
    case SHOP_BUY_OWNED:
      setNZ16(out, rd16(DP + 0x44));
      out->c = true;
      out->v = false;
      break;
    case SHOP_BUY_BOUGHT:
      setNZ16(out, rd16(DP + 0x54));
      out->c = true;
      out->v = false;
      break;
    case SHOP_BUY_REFUNDED_SHORT:
      out->n = out->z = false;
      out->c = f.c;
      out->v = f.v;
      break;
    default:
      setNZ16(out, rd16(DP + 0x6C));
      out->c = false;
      break;
  }
}
static void call_shop_buy_list2(const Regs* in, Regs* out) { call_shop_buy(in, out, 1, 0x00B4, false, true); }
static void call_shop_buy_list3(const Regs* in, Regs* out) { call_shop_buy(in, out, 2, 0x00B4, false, true); }
static void call_shop_buy_list4(const Regs* in, Regs* out) { call_shop_buy(in, out, 3, 0x00B4, false, true); }
// func_80E4DB's PLA: A as the handler left it, its high byte the entry's and its low byte
// D+$4A's (an 8-bit LDA)
static void call_shop_buy_colour(const Regs* in, Regs* out) {
  shop_buy_colour(in->d);
  setNZ16(out, (uint16_t)((in->a & 0xFF00) | rd8(DP + 0x4A)));
}

// A kept (PLA: N/Z from it)
static void call_shop_flag_fb(const Regs* in, Regs* out) { shop_flag_fb(in->d); setNZ16(out, in->a); }

// as func_80B537's adapter, with its inputs set here (Y ends as $5C + $0400)
static void call_shop_panel_upload(const Regs* in, Regs* out) {
  (void)in;
  uint16_t i = rd16(DP + 0x8C), base = rd16(DP + 0x8A);
  out->y = (uint16_t)(rd16(DP + 0x5C) + 0x0400);
  out->a = shop_panel_upload(in->d);
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
  shop_item_picture(in->d);
  out->a = 9;
  setNZ16(out, 9);
}

// after the MVN: A = $FFFF, X and Y past the block; PLB of $81 sets N
static void call_shop_item_text(const Regs* in, Regs* out) {
  (void)in;
  uint16_t text = rom16(0x81A895 + (uint32_t)shopEntry(rd16(DP + 0x4A)));
  uint16_t size = rom16(0x81A894 + (uint32_t)text), src = rom16(0x81A892 + (uint32_t)text);
  uint16_t dest = (uint16_t)(rd16(DP + 0xC4) + 0x0100);
  shop_item_text(in->d);
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
  out->a = shop_bar_cap(in->d);
  out->x = slot;
  setNZ16(out, out->a);
  setAdc16(out, slot, 0x0010);
}
static void call_shop_bar_column(const Regs* in, Regs* out) {
  (void)in;
  uint16_t slot = rd16(DP + 0x74);
  out->a = shop_bar_column(in->d);
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
static void call_shop_bar_levels(const Regs* in, Regs* out) { (void)in; (void)out; shop_bar_levels(in->d); }
static void call_shop_bar_segments(const Regs* in, Regs* out) { (void)in; (void)out; shop_bar_segments(in->d); }
static void call_shop_bar_segments_long(const Regs* in, Regs* out) { (void)in; (void)out; shop_bar_segments_long(in->d); }
static void call_shop_bar_double(const Regs* in, Regs* out) { (void)in; (void)out; shop_bar_double(in->d); }
static void call_shop_gauge(const Regs* in, Regs* out) { (void)in; (void)out; shop_gauge(in->d); }

// A = the old D+$14 (N/Z of its PLA); X and Y as sprite_number_centered leaves them, with
// C and V from its last ADC
static void call_shop_item_price(const Regs* in, Regs* out) {
  (void)in;
  uint16_t base = rd16(DP + 0x14);
  uint16_t price = rom16(0x81A89B + (uint32_t)shopEntry(rd16(DP + 0x4A)));
  uint32_t value = (uint32_t)rom16(0x81A88E + (uint32_t)price) << 16 | rom16(0x81A890 + (uint32_t)price);
  uint16_t shift = shop_item_price(in->d);
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
static void call_shop_item_view_a(const Regs* in, Regs* out) { (void)out; shop_item_view_a(in->d, in->y); }
static void call_shop_item_view_b(const Regs* in, Regs* out) { (void)out; shop_item_view_b(in->d, in->y); }
static void call_shop_item_view_c(const Regs* in, Regs* out) { (void)out; shop_item_view_c(in->d, in->y); }
static void call_shop_item_view_d(const Regs* in, Regs* out) { (void)out; shop_item_view_d(in->d, in->y); }
static void call_shop_item_view_e(const Regs* in, Regs* out) { (void)out; shop_item_view_e(in->d, in->y); }
static void call_shop_item_view_g(const Regs* in, Regs* out) { (void)out; shop_item_view_g(in->d, in->y); }
// lists 6-8's views and their helpers (only the password movie reaches them, with few
// returns: their exit flags are modelled). A view's tail: D+$40 += $40 (wrapping at `wrap`,
// a SBC, for views h and i), then INC D+$3E (DEC back from $8000): N/Z of D+$3E; C/V of the
// last CMP/SBC or of the ADC
static void viewTail(Regs* out, uint16_t old40, uint16_t wrap) {
  const uint16_t a = (uint16_t)(old40 + 0x0040);
  setNZ16(out, rd16(DP + 0x3E));
  if(wrap == 0 || a < wrap) {
    out->c = wrap != 0 ? false : (uint32_t)old40 + 0x40 > 0xFFFF;  // the CMP's borrow, or the ADC's carry
    out->v = (~(old40 ^ 0x0040) & (old40 ^ a) & 0x8000) != 0;
  } else {  // never run on a movie: SBC wrap (C set by the CMP), its V
    const uint16_t r = (uint16_t)(a - wrap);
    out->c = true;
    out->v = ((a ^ wrap) & (a ^ r) & 0x8000) != 0;
  }
}
static void call_shop_item_view_h(const Regs* in, Regs* out) { const uint16_t o = rd16(DP + 0x40); shop_item_view_h(in->d, in->y); viewTail(out, o, 0x1E00); }
static void call_shop_item_view_i(const Regs* in, Regs* out) { const uint16_t o = rd16(DP + 0x40); shop_item_view_i(in->d, in->y); viewTail(out, o, 0x0F00); }
static void call_shop_item_view_j(const Regs* in, Regs* out) { const uint16_t o = rd16(DP + 0x40); shop_item_view_j(in->d, in->y); viewTail(out, o, 0); }
// shop_level_blocks ends on the CMP #$2D that matched (Z, C set, N clear) after the ADC #6
// from $27 (V clear); or, all seven full with three left over, at the SBC #3 of D+$7E (not
// stored): its N/Z/V, C set
static void call_shop_level_blocks(const Regs* in, Regs* out) {
  (void)in;
  if(shop_level_blocks(in->d)) {
    const uint16_t a = rd16(DP + 0x7E), r = (uint16_t)(a - 3);
    setNZ16(out, r);
    out->c = true;
    out->v = ((a ^ 3) & (a ^ r) & 0x8000) != 0;
    return;
  }
  out->n = false;
  out->z = out->c = true;
  out->v = false;
}
// shop_gauge_line ends past column $48 (the CMP's N/Z/C) or at its string's 0 word (that
// LDA's N/Z, the CMP's C); V of the last ADC #8
static void gaugeLineOut(Regs* out) {
  const uint16_t now = rd16(DP + 0x80), old = (uint16_t)(now - 8);
  out->c = now >= 0x0048;
  out->v = (~(old ^ 0x0008) & (old ^ now) & 0x8000) != 0;
  if(!((uint16_t)(now - 0x0048) & 0x8000)) {
    out->n = false;
    out->z = now == 0x0048;
  } else {
    out->n = false;
    out->z = true;
  }
}
static void call_shop_gauge_line(const Regs* in, Regs* out) { shop_gauge_line(in->d, (uint8_t)in->a, in->y, in->x); gaugeLineOut(out); }
// four gauge lines, then PLB: N/Z of the bank it restores (the entry's), C/V the last line's
static void call_shop_gauge_lines(const Regs* in, Regs* out) {
  shop_gauge_lines(in->d);
  gaugeLineOut(out);
  out->n = (in->db & 0x80) != 0;
  out->z = in->db == 0;
}
static void call_shop_item_dispatch_view(const Regs* in, Regs* out) { (void)in; (void)out; shop_item_dispatch_view(in->d); }
// the panel dispatch: X = the entry (its PLX: N, Z); C and V of the last queue ADC (one
// upload for lists 0 and 6-8, two for the rest)
static void call_shop_item_dispatch_panel(const Regs* in, Regs* out) {
  (void)in;
  uint16_t list = rd16(DP + 0x48);
  uint16_t entry = (uint16_t)((rd16(DP + 0x4A) << 4) + rom16(0x81A892 + (uint32_t)(uint16_t)(list << 1)));
  uint16_t i = rd16(DP + 0x8C), base = rd16(DP + 0x8A);
  if(list >= 1 && list <= 5) i = (uint16_t)(((i + 2) & 0x3F) + base);  // the first upload's slot
  shop_item_dispatch_panel(in->d);
  setAdc16(out, (uint16_t)((i + 2) & 0x3F), base);
  out->x = entry;
  setNZ16(out, entry);
}
// the panel's handlers (shop-26): their last func_80B537's exit: A the next index (N/Z), X the
// index it used + 2, C/V of its ADC, Y D+$5C + the last upload's VRAM offset
static void shopItemPanelOut(const Regs* in, Regs* out, void (*fn)(uint16_t, uint16_t), bool two, uint16_t lastVram) {
  uint16_t i = rd16(DP + 0x8C);
  const uint16_t base = rd16(DP + 0x8A);
  if(two) i = (uint16_t)(((i + 2) & 0x3F) + base);
  fn(in->d, in->y);
  out->a = (uint16_t)(((i + 2) & 0x3F) + base);
  out->x = (uint16_t)(i + 2);
  out->y = (uint16_t)(rd16(DP + 0x5C) + lastVram);
  setNZ16(out, out->a);
  setAdc16(out, (uint16_t)((i + 2) & 0x3F), base);
}
static void call_shop_item_panel_1(const Regs* in, Regs* out) { shopItemPanelOut(in, out, shop_item_panel_1, true, 0x1780); }
static void call_shop_item_panel_2(const Regs* in, Regs* out) { shopItemPanelOut(in, out, shop_item_panel_2, true, 0x1260); }
static void call_shop_item_panel_3(const Regs* in, Regs* out) { shopItemPanelOut(in, out, shop_item_panel_3, true, 0x1240); }
static void call_shop_item_panel_5(const Regs* in, Regs* out) { shopItemPanelOut(in, out, shop_item_panel_5, true, 0x1340); }
static void call_shop_item_panel_4(const Regs* in, Regs* out) { shopItemPanelOut(in, out, shop_item_panel_4, true, 0x1560); }
static void call_shop_item_panel_0(const Regs* in, Regs* out) { shopItemPanelOut(in, out, shop_item_panel_0, false, 0x1000); }
static void call_shop_item_panel_6(const Regs* in, Regs* out) { shopItemPanelOut(in, out, shop_item_panel_6, false, 0x1140); }
// a list's pictures (Y: the entry's word at $81:A89B for lists 0-10), the queue as
// func_80B537's fuzz
static void fuzz_shop_item_panel(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  const uint16_t list = (uint16_t)(rnd() % 11);
  const uint16_t entry = (uint16_t)(((list == 0 || list >= 6) ? 0 : (rnd() % 5) << 4) + rom16(0x81A892 + (uint32_t)(uint16_t)(list << 1)));
  r->y = rom16(0x81A89B + (uint32_t)entry);
  wr16ram(ram, DP + 0x5C, (uint16_t)rnd());
  wr16ram(ram, DP + 0x5E, (uint16_t)rnd());
  // as the panel dispatch's fuzzer: the ring's base a multiple of $40 (the game's)
  wr16ram(ram, DP + 0x8A, (uint16_t)((rnd() % 0x14) * 0x40));
  wr16ram(ram, DP + 0x8C, (uint16_t)(rnd() % 0x500));
}
// the slides: no register outputs (their callers return or reload them); the last CMP
// matched: N clear, Z and C set
static void slideFlags(Regs* out) {
  out->n = false;
  out->z = true;
  out->c = true;
}
static void call_shop_item_slide_row(const Regs* in, Regs* out) { (void)in; shop_item_slide_row(in->d); slideFlags(out); }
static void call_shop_item_slide_list(const Regs* in, Regs* out) { (void)in; shop_item_slide_list(in->d); slideFlags(out); }
static void call_shop_item_slide_list_in(const Regs* in, Regs* out) { (void)in; shop_item_slide_list_in(in->d); slideFlags(out); }
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
static void call_shop_item_down(const Regs* in, Regs* out) { shopItemMoveOut(in, out, shop_item_down(in->d)); }
static void call_shop_item_up(const Regs* in, Regs* out) { shopItemMoveOut(in, out, shop_item_up(in->d)); }
// the icon's show and settle: no register outputs; show ends on its LDA of the step (N set),
// settle on the TRB (Z of $8000 & D+$76: clear, the bit was set)
static void call_shop_item_show_icon(const Regs* in, Regs* out) {
  (void)in;
  shop_item_show_icon(in->d);
  setNZ16(out, rd16(DP + 0x44));
}
static void call_shop_item_icon_settle(const Regs* in, Regs* out) {
  (void)in;
  bool set = (rd16(DP + 0x76) & 0x8000) != 0;
  shop_item_icon_settle(in->d);
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
  out->c = shop_item_affordable(in->d);
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
static void call_shop_item_redraw(const Regs* in, Regs* out) { (void)in; (void)out; shop_item_redraw(in->d); }
// no register outputs: every caller reloads A, X and Y ($80:9E49, $80:9B99, LDA #2)
static void call_shop_item_icon_draw(const Regs* in, Regs* out) { (void)out; shop_item_icon_draw(in->d, in->x); }
// A = $B149 (N set, Z clear), X = D+$5E + 16, Y = 0; C and V kept
static void call_shop_panel_blank(const Regs* in, Regs* out) {
  (void)in;
  uint16_t x = rd16(DP + 0x5E);
  shop_panel_blank(in->d);
  out->a = 0xB149;
  out->x = (uint16_t)(x + 16);
  out->y = 0;
  setNZ16(out, 0xB149);
}
// A = 0, X = D+$5E + 18, Y = 0 (N clear, Z set by the DEY), C set by the CMP that matched
static void call_shop_panel_clear(const Regs* in, Regs* out) {
  (void)in;
  shop_panel_clear(in->d);
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
  out->c = shop_list_locked(DP);
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
  shop_list_prev(in->d);
  if(locked) return;
  setNZ16(out, list);
}
static void call_shop_list_next(const Regs* in, Regs* out) {
  (void)in;
  shopListLockedOut(out);
  uint16_t list = rd16(DP + 0x48), limit = rd16(DP + 0xF0);
  bool locked = out->c;
  shop_list_next(in->d);
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
static void call_shop_level_marks(const Regs* in, Regs* out) { (void)in; (void)out; shop_level_marks(in->d); }
// lists 0-9 (the getter table), 1-8 marks, any level, cursor and position, slots in the
// buffer
static void fuzz_shop_level_marks(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint16_t list = (uint16_t)(rnd() % 11), car = (uint16_t)(rnd() & 0x1F);  // lists 0-10 (shop-23: with 10)
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
static void shopItemIconOut(const Regs* in, Regs* out, void (*fn)(uint16_t, bool), uint16_t field, uint16_t icons) {
  uint16_t i = rd16(DP + 0x8C), base = rd16(DP + 0x8A), step = rd16(DP + 0x44);
  uint16_t level = (uint16_t)(rd16(0x0393 + (uint32_t)field + rd16(DP + 0x94)) & 0x007F);
  GroupWalk w = sprite_group_walk(0x7E0000 | rom16(0x810000 + (uint32_t)icons + (uint32_t)(uint16_t)(level << 1)));
  fn(in->d, in->c);
  if(in->c) {
    setAdc16(out, (uint16_t)((i + 2) & 0x3F), base);
    setNZ16(out, in->y);
    return;
  }
  if(w.any) setAdc16(out, w.lastY, 6);
  if(step == 0x0007) out->v = (rd16(DP + 0x76) & 0x4000) != 0;
}
static void call_shop_item_icon_1(const Regs* in, Regs* out) { shopItemIconOut(in, out, shop_item_icon_1, 0, 0xBC2E); }
// lists 0 and 6-10 (shop-25): as lists 1-5, but list 0 queues nothing (its RTS: flags as on
// entry) and lists 6-10 queue nothing at level 0 (PHY ... PLY: N/Z of Y)
static void shopItemIconOutB(const Regs* in, Regs* out, void (*fn)(uint16_t, bool), int field, uint16_t icons) {
  const uint16_t i = rd16(DP + 0x8C), base = rd16(DP + 0x8A), step = rd16(DP + 0x44);
  const uint16_t level = field < 0 ? 0 : (uint16_t)(rd16(0x0393 + (uint32_t)field + rd16(DP + 0x94)) & 0x007F);
  const uint16_t group = field < 0 ? 0x4F6A : rom16(0x810000 + (uint32_t)icons + (uint32_t)(uint16_t)(level << 1));
  GroupWalk w = sprite_group_walk(0x7E0000 | group);
  fn(in->d, in->c);
  if(in->c) {
    if(field < 0) return;
    if(level != 0) setAdc16(out, (uint16_t)((i + 2) & 0x3F), base);
    setNZ16(out, in->y);
    return;
  }
  // the draw (shop_item_icon_draw), all four flags (these rows have few samples): C and V of
  // the group's last ADC #6, else C of the CMP #$10 of the step * 2; a negative step leaves
  // by its BMI (N set); else N/Z of the new step, C of its CMP #$F / #7, V of BIT D+$76 at 7
  if(w.any) setAdc16(out, w.lastY, 6);
  else out->c = (uint16_t)(step << 1) >= 0x0010;
  if(step & 0x8000) {
    out->n = true;
    out->z = false;
    return;
  }
  setNZ16(out, rd16(DP + 0x44));
  out->c = step == 0x000F || step >= 0x0007;
  if(step == 0x0007) out->v = (rd16(DP + 0x76) & 0x4000) != 0;
}
static void call_shop_item_icon_0(const Regs* in, Regs* out) { shopItemIconOutB(in, out, shop_item_icon_0, -1, 0); }
static void call_shop_item_icon_6(const Regs* in, Regs* out) { shopItemIconOutB(in, out, shop_item_icon_6, 5, 0xBCFA); }
static void call_shop_item_icon_7(const Regs* in, Regs* out) { shopItemIconOutB(in, out, shop_item_icon_7, 6, 0xBCF6); }
static void call_shop_item_icon_8(const Regs* in, Regs* out) { shopItemIconOutB(in, out, shop_item_icon_8, 7, 0xBCF2); }
static void call_shop_item_icon_9(const Regs* in, Regs* out) { shopItemIconOutB(in, out, shop_item_icon_9, 9, 0xBCFE); }
static void call_shop_item_icon_10(const Regs* in, Regs* out) { shopItemIconOutB(in, out, shop_item_icon_10, 8, 0xBD02); }
// the dispatches: lists 1-5's handlers as their rows; lists 6-8 likewise on fields 5-7; list 0
// a fixed icon. No upload (list 0, or a level-0 part of lists 6-8): C set by the SEC, V of
// the entry's ADC. The upload dispatch ends with PLX of the entry (X, N, Z).
static void shopItemDispatchOut(const Regs* in, Regs* out, bool upload) {
  // lists 1-10: their icon tables and level fields (list 9's is $039C, list 10's $039B)
  static const uint16_t icons[10] = {0xBC2E, 0xBC52, 0xBC76, 0xBCBE, 0xBC9A, 0xBCFA, 0xBCF6, 0xBCF2, 0xBCFE, 0xBD02};
  static const uint16_t fields[10] = {0, 1, 2, 3, 4, 5, 6, 7, 9, 8};
  uint16_t list = rd16(DP + 0x48);
  uint16_t entry = (uint16_t)((rd16(DP + 0x4A) << 4) + rom16(0x81A892 + (uint32_t)(uint16_t)(list << 1)));
  uint16_t i = rd16(DP + 0x8C), base = rd16(DP + 0x8A), step = rd16(DP + 0x44);
  uint16_t field = list ? fields[list - 1] : 0;
  uint16_t level = list ? (uint16_t)(rd16(0x0393 + (uint32_t)field + rd16(DP + 0x94)) & 0x007F) : 0;
  uint16_t group = list ? rom16(0x810000 + (uint32_t)icons[list - 1] + (uint32_t)(uint16_t)(level << 1)) : 0x4F6A;
  GroupWalk w = sprite_group_walk(0x7E0000 | group);
  if(upload) shop_item_dispatch_upload(in->d);
  else shop_item_dispatch_icon(in->d);
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
static void call_shop_item_view_f(const Regs* in, Regs* out) { (void)out; shop_item_view_f(in->d, in->y); }
// the ring: no register outputs (its view reloads them); its pieces: A = 0, X = the frame
// x 6, Y = slot + 4 (N/Z of the INY), C and V of the y position's ADC
static void call_shop_ring(const Regs* in, Regs* out) { (void)in; (void)out; shop_ring(in->d); }
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
  ringPieceOut(in, out, 0x4D0A, shop_ring_piece_front(in->d, in->a, in->y));
  setAdc16(out, base, add);
}
static void call_shop_ring_piece_back(const Regs* in, Regs* out) {
  uint16_t base = rd16(DP + 0x18), add = rd16(0x4D6E + (uint32_t)(((in->a >> 8) & 0x000F) * 6));
  ringPieceOut(in, out, 0x4D6A, shop_ring_piece_back(in->d, in->a, in->y));
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
// lists 6-8's views: the view's spin D+$40 near its wrap now and then
static void fuzz_shop_item_view_hij(Regs* r, uint8_t* ram, uint32_t (*rnd)(void), uint16_t view, uint16_t wrap) {
  fuzz_shop_item_view_of(r, ram, rnd, view);
  wr16ram(ram, DP + 0x40, (rnd() & 1) && wrap ? (uint16_t)(wrap - 0x40 + (rnd() % 3) * 0x40 - 0x40) : (uint16_t)(rnd() % 0x2000));
  wr16ram(ram, DP + 0x46, (uint16_t)rnd());
}
static void fuzz_shop_item_view_h(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { fuzz_shop_item_view_hij(r, ram, rnd, 0xAEBE, 0x1E00); }
static void fuzz_shop_item_view_i(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { fuzz_shop_item_view_hij(r, ram, rnd, 0xAE4E, 0x0F00); }
static void fuzz_shop_item_view_j(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { fuzz_shop_item_view_hij(r, ram, rnd, 0xAC37, 0); }
// the stage D+$3E (0-$5F), the sprite slot (a shop's), the view's place words
static void fuzz_shop_level_blocks(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x3E, (uint16_t)(rnd() % 0x60));
  wr16ram(ram, DP + 0x74, (uint16_t)(4 * (rnd() % 32)));
  wr16ram(ram, DP + 0x7A, (uint16_t)(rnd() % 0x40));
  wr16ram(ram, DP + 0xE7, (uint16_t)rnd());
  wr16ram(ram, DP + 0x14, (uint16_t)rnd());
  wr16ram(ram, DP + 0x84, (uint16_t)rnd());
}
// the gauges: their speed word D+$46, the column D+$14 (from $C0 up its high byte is set: the
// sign path), the view's place, the sprite slot
static void fuzz_shop_gauge_lines(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x46, (uint16_t)rnd());
  wr16ram(ram, DP + 0x14, (rnd() & 1) ? (uint16_t)(0x00C0 + rnd() % 0x80) : (uint16_t)(rnd() % 0x100));
  wr16ram(ram, DP + 0x7A, (uint16_t)(rnd() % 0x40));
  wr16ram(ram, DP + 0xE8, (uint16_t)rnd());
  wr16ram(ram, DP + 0x74, (uint16_t)(4 * (rnd() % 32)));
}
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
  // the sprite buffers as the shop keeps them (measured at every live entry, both shop
  // movies): D+$6C/D+$6E the two OAM tables, $0900 and $0D00, swapped every frame by
  // oam_finish; the slot D+$74 = D+$6C - $0900 (0 or $400); the last frames' ends
  // D+$70/D+$72 from $00DC to $05A8. A fuzz base outside the shop (a paused race: $27B4 and
  // $FFBC there) is not a state the shop's screen runs in: oam_high_table's direct page
  // would land on the hardware registers, and its clearing loop (from D+$74 up to D+$72)
  // would run over the whole bank and the stack
  const bool first = rnd() & 1;
  wr16ram(ram, DP + 0x6C, first ? 0x0900 : 0x0D00);
  wr16ram(ram, DP + 0x6E, first ? 0x0D00 : 0x0900);
  wr16ram(ram, DP + 0x74, first ? 0x0000 : 0x0400);
  wr16ram(ram, DP + 0x70, (uint16_t)(0x00DC + 4 * (rnd() % 0x134)));
  wr16ram(ram, DP + 0x72, (uint16_t)(0x00DC + 4 * (rnd() % 0x134)));
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
// a dispatch: lists 0-10 (0 and 6-10 one row each), as fuzz_shop_item_icon; lists 6-10's
// levels 0-3
static void fuzz_shop_item_dispatch(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_shop_item_icon(r, ram, rnd);
  uint16_t list = (uint16_t)(rnd() % 11);
  wr16ram(ram, DP + 0x48, list);
  wr16ram(ram, DP + 0x4A, (uint16_t)(list == 0 || list >= 6 ? 0 : rnd() % 5));
  uint16_t car = (uint16_t)(ram[DP + 0x94] | ram[DP + 0x95] << 8);
  for(uint32_t f = 0; f < 5; f++) ram[0x0398 + car + f] = (uint8_t)((rnd() % 4) | (rnd() & 0x80));
}
// one of lists 0 and 6-10's handlers: their levels 0-3
static void fuzz_shop_item_icon_b(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_shop_item_icon(r, ram, rnd);
  uint16_t car = (uint16_t)(ram[DP + 0x94] | ram[DP + 0x95] << 8);
  for(uint32_t f = 0; f < 5; f++) ram[0x0398 + car + f] = (uint8_t)((rnd() % 4) | (rnd() & 0x80));
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
  out->a = out->y = shop_sprite_block(in->d, in->a, in->y);
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
  out->a = take_nibble16(in->d);
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
  bool hit = calleeRun ? calleeRun(in->d) : func_83FC05(in->d);
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
  bool hit = calleeRun ? calleeRun(dp) : func_83FD8A(dp, a);
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
  out->c = calleeRun ? calleeRun(in->d) : func_83FDCD();
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

// ---- car_sync_player ($81:FA8F) ----  func_81FBB1's exit, through its adapter (fbb1Run),
// given the car's record as it stands at that call: the adapter writes it (the product
// in plain C, no chip traffic) and the hook puts the record back before the C runs.
// Directly, then: A = the car's new flags (N Z of the ORA), V = bit 14 of $1E66 (its BIT);
// C and X stay func_81FBB1's.
static uint8_t syncHeld[0x35];
static uint16_t syncCar;
static bool run_car_sync_player(void) {
  memcpy(&g_ram[0xFAB5 + (uint32_t)syncCar], syncHeld, sizeof syncHeld);
  return car_sync_player(syncCar);
}
static void call_car_sync_player(const Regs* in, Regs* out) {
  const uint16_t car = in->y;
  const uint32_t o = car;
  const bool direct = !(rd16(0x0244) & 0x0001);
  memcpy(syncHeld, &g_ram[0xFAB5 + o], sizeof syncHeld);
  uint32_t pos = rd16(0xFAB5 + o) | (uint32_t)rd16(0xFAB7 + o) << 16;
  if(direct) pos += rd16(0x022D) | (uint32_t)rd16(0x022F) << 16;
  else pos += (uint32_t)((int32_t)(int16_t)rd16(0x1E9E) * (int16_t)rd16(0x0210));
  wr16(0xFAB5 + o, (uint16_t)pos);
  wr16(0xFAB7 + o, (uint16_t)(pos >> 16));
  wr16(0xFAC9 + o, rd16(0x020E));
  wr16(0xFACB + o, rd16(0x0210));
  if(direct) {
    wr16(0xFAB9 + o, rd16(0x0204));
    wr16(0xFABB + o, rd16(0x0206));
  }
  wr16(0xFAC7 + o, 0xFFFF);
  syncCar = car;
  fbb1Run = run_car_sync_player;
  Regs t = *in;
  call_func_81FBB1(&t, out);
  fbb1Run = NULL;
  out->y = car;
  if(direct) {
    out->a = rd16(CAR_FLAGS + o);
    setNZ16(out, out->a);
    out->v = (rd16(0x1E66) & 0x4000) != 0;
  }
}
// func_81FBB1's state for the car, either way of $0244, the player's words
static void fuzz_car_sync_player(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_func_81FBB1(r, ram, rnd);
  wr16ram(ram, 0x0244, (uint16_t)rnd());
  static const uint16_t words[] = {0x022D, 0x022F, 0x020E, 0x0210, 0x0204, 0x0206, 0x1E66};
  for(unsigned i = 0; i < sizeof words / sizeof words[0]; i++) wr16ram(ram, words[i], (uint16_t)rnd());
  wr16ram(ram, 0x022D, (uint16_t)(rnd() % 0x200));
  wr16ram(ram, 0x022F, 0);
  wr16ram(ram, 0x1E9E, (uint16_t)(rnd() % 0x10));
  wr16ram(ram, 0x0210, (uint16_t)(rnd() % 0x20));
}

// ---- race_hud_digits ($82:B4ED) ----  both callers go on to code that sets what it reads
// (one returns, JMP'd to, into the race frame's next step): no register outputs; the
// bench judges the flags from the watched returns
static void call_race_hud_digits(const Regs* in, Regs* out) {
  (void)in;
  (void)out;
  race_hud_digits();
}
// the blocks' and the messages' state; the HUD's inputs on all sides of their tests (the
// never-run ones now and then)
static void fuzz_race_hud_digits(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_func_82B769(r, ram, rnd);
  fuzz_race_message_update(r, ram, rnd);
  wr16ram(ram, 0x4E3D, (uint16_t)(rnd() % 0x4F));
  wr16ram(ram, 0x0233, (uint16_t)(rnd() % 0x4F));
  wr16ram(ram, 0x4E45, (uint16_t)(rnd() % 0x40));
  wr16ram(ram, 0x4E4D, (uint16_t)rnd());
  wr16ram(ram, DP + 0x50, (uint16_t)(2 * (rnd() % 0x40)));
  wr16ram(ram, DP + 0x52, (uint16_t)(2 * (rnd() % 0x40)));
  wr16ram(ram, 0x0226, (uint16_t)((rnd() % 0x40) << 4));
  wr16ram(ram, 0x02D9, (uint16_t)((rnd() % 0x40) << 4));
  wr16ram(ram, 0x0235, (uint16_t)rnd());
  wr16ram(ram, 0x02E8, (uint16_t)rnd());
  wr16ram(ram, 0x0244, (uint16_t)(rnd() % 4 == 0 ? 0x0004 : rnd() & 0xFFFB));
  wr16ram(ram, 0x03DC, (uint16_t)(rnd() % 6 == 0 ? 1 + rnd() % 3 : 0));
  wr16ram(ram, 0x0213, (uint16_t)rnd());
  wr16ram(ram, 0x0210, (uint16_t)(rnd() % 6 == 0 ? 0x8000 | rnd() : rnd() % 0x2000));
  ram[0x03A7] = (uint8_t)(rnd() & 1);
  wr16ram(ram, 0x1E94, (uint16_t)(rnd() % 5 == 0 ? rnd() % 0x800 : 0x8000 | rnd()));
  wr16ram(ram, DP + 0x42, (uint16_t)(rnd() % 30));
  wr16ram(ram, 0x0258, (uint16_t)(rnd() % 0x48));
  wr16ram(ram, 0x0297, (uint16_t)rnd());
  wr16ram(ram, 0x025B, (uint16_t)rnd());
  wr16ram(ram, DP + 0x4C, (uint16_t)(rnd() % 10));
}

// ---- car_track_step ($83:F7F9) ----  its caller PLDs and returns into a routine that loads
// A and X first: no register outputs but DB ($83, its PLB); the bench judges the flags
static void call_car_track_step(const Regs* in, Regs* out) {
  car_track_step(in->d);
  out->db = 0x83;
}
// the handlers' and the lap check's inputs; the car's state bits, speeds and counters;
// a short move forward (a few segments) and objects of every kind the table handles (the
// random pickup, kinds 24 and 93, often) on the segments ahead
static void fuzz_car_track_step(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  const uint16_t dp = r->d;
  const uint16_t frames = (uint16_t)(ram[0x019A] | ram[0x019B] << 8);
  fuzz_track_object(r, ram, rnd);
  fuzz_car_lap_check(r, ram, rnd);
  fuzz_rng_state(ram, rnd);
  uint16_t laps = frames;
  if(laps == 0 || laps > 0x400 || (rnd() & 1)) laps = (uint16_t)(0x40 + rnd() % 0x200);
  wr16ram(ram, 0x019A, laps);
  const uint32_t pos = (uint32_t)(rnd() % ((uint32_t)laps << 8)) | (rnd() % 4 == 0 ? (uint32_t)(rnd() % 4) << 24 : 0);
  wr16ram(ram, (uint16_t)(dp + 0x25), (uint16_t)pos);
  wr16ram(ram, (uint16_t)(dp + 0x27), (uint16_t)(pos >> 16));
  static const uint16_t bits[] = {0x0000, 0x0020, 0x0120, 0x0021, 0x0001, 0x0080, 0x00A0, 0x01A0};
  wr16ram(ram, (uint16_t)(dp + 0x44), (uint16_t)(bits[rnd() % 8] | (rnd() & 0xFE5E)));
  wr16ram(ram, 0x1E9E, (uint16_t)(1 + rnd() % 4));
  // the car moves forward (a backward move crosses tens of thousands of segments, through
  // the table's unused kinds)
  wr16ram(ram, (uint16_t)(dp + 0x10), (uint16_t)(rnd() % 0x200));
  wr16ram(ram, (uint16_t)(dp + 0x70), (uint16_t)((rnd() & 1) ? 0xF0 + rnd() % 0x20 : rnd() % 0x100));
  wr16ram(ram, (uint16_t)(dp + 0x72), (uint16_t)(rnd() % 0x100));
  // D+$5E: with bit 8 a countdown (0 half the time), else the segment to stop at, ahead
  const bool countdown = (ram[dp + 0x45] & 0x01) != 0;
  wr16ram(ram, (uint16_t)(dp + 0x5E), countdown ? ((rnd() & 1) ? 0 : (uint16_t)(rnd() % 8)) : (uint16_t)((pos >> 8) + 1 + rnd() % 8));
  wr16ram(ram, (uint16_t)(dp + 0x0C), (uint16_t)rnd());
  wr16ram(ram, (uint16_t)(dp + 0x4E), (uint16_t)((rnd() & 0xFF00) | (rnd() % 12)));
  const uint16_t car = (uint16_t)(0x35 * (rnd() % 20));
  wr16ram(ram, 0x1EBA, car);
  wr16ram(ram, (uint16_t)(0xFADE + car), (uint16_t)rnd());
  // $012C (from the car's +$0A) within the pickups' height limits (+-32) half the time,
  // and the car's D+$06 inside the 12-bit range the objects' bytes reach
  wr16ram(ram, (uint16_t)(0xFABF + car), (rnd() & 1) ? (uint16_t)(rnd() % 64 - 32) : (uint16_t)rnd());
  const uint16_t across = (uint16_t)(rnd() % 0x400 - 0x200);
  wr16ram(ram, (uint16_t)(dp + 0x06), across);
  for(int i = 0; i < 0x22; i++) wr16ram(ram, (uint16_t)(0x3DEC + 2 * i), (uint16_t)rnd());
  // the segments ahead: kinds whose handler entry is not 0, the random pickup a third of
  // the time
  uint16_t at = (uint16_t)(((pos + 0x100) >> 8) & 0xFFFF);
  for(int i = 0; i < 12; i++, at++) {
    const uint32_t e = 0x8CB3 + (uint32_t)(uint16_t)((uint16_t)(at << 2) % (uint16_t)(laps << 2));
    uint16_t kind;
    do kind = (rnd() % 3 == 0) ? (rnd() & 1 ? 24 : 93) : (uint16_t)(1 + rnd() % 104);
    while(rom16(0x837FFE + 2u * kind) == 0);
    if(rnd() % 4 == 0) wr16ram(ram, (uint16_t)e, 0);
    else {
      ram[e] = (uint8_t)rnd();
      ram[e + 1] = (uint8_t)kind;
      ram[e + 2] = (uint8_t)rnd();
      if(rnd() & 1) {  // aimed at the car: across within +-32, height 0
        ram[e] = (uint8_t)((uint16_t)(across + rnd() % 64 - 32) >> 4);
        ram[e + 2] &= 0x0F;
      }
    }
  }
}

// ---- minimap_car ($82:D8C6) ----  both callers reload A, X and Y next: no register outputs;
// the bench judges the flags
static void call_minimap_car(const Regs* in, Regs* out) {
  (void)out;
  minimap_car(in->a, in->x, in->y);
}
// Negative row: the dot's word with y in the low byte and x in the high one (the
// sprite's two coordinates swapped), which the comparison must catch
static void call_minimap_car_wrong(const Regs* in, Regs* out) {
  call_minimap_car(in, out);
  const uint32_t o = (uint16_t)(rd16(0x0184) - 4);
  const uint16_t w = rd16(0x1576 + o);
  wr16(0x1576 + o, (uint16_t)(w << 8 | w >> 8));
}
// a place in the map table (a multiple of 4 below its end), its bytes, y, the attribute; the
// track-end case ($047B on a negative $82:DC68 entry, D+$14 either sign) half the time; the
// output offset
static void fuzz_minimap_car(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  const uint16_t laps = (uint16_t)(0x40 + rnd() % 0x240);
  wr16ram(ram, 0x019A, laps);
  r->x = (uint16_t)(4 * (rnd() % (laps / 4)));
  r->a = (uint16_t)rnd();
  r->y = (rnd() & 1) ? 0x30FD : (uint16_t)rnd();
  static const uint16_t negative[] = {17, 27, 47};
  wr16ram(ram, 0x047B, (rnd() & 1) ? negative[rnd() % 3] : (uint16_t)(rnd() % 48));
  wr16ram(ram, DP + 0x14, (rnd() % 3 == 0) ? 0 : (uint16_t)rnd());
  for(int i = 0; i < 8; i++) ram[0x4F31 + (uint32_t)r->x + i] = (uint8_t)rnd();
  const uint16_t end = (uint16_t)(((uint16_t)(laps - 1) >> 2) & 0xFFFC);
  ram[0x4F31] = (uint8_t)rnd();
  ram[0x4F32] = (uint8_t)rnd();
  ram[0x4F31 + (uint32_t)end] = (uint8_t)rnd();
  ram[0x4F32 + (uint32_t)end] = (uint8_t)rnd();
  wr16ram(ram, 0x0184, (uint16_t)(4 * (rnd() % 0x40)));
}

// ---- player_segment_step ($82:F2B3) ----  both callers go on to code that reads none of
// its registers: no register outputs; the bench judges the flags
static void call_player_segment_step(const Regs* in, Regs* out) {
  (void)in;
  (void)out;
  player_segment_step();
}
// the race's own segment table ($019C) and cursors, the player's place up to a few segments
// past the first cursor's start, an armed jump reached or not, the handlers' words; now and
// then the cursor moved to a segment of the never-run kinds (6, 8, 10) further on in the
// same table (random tables reach odd kinds, whose table entries jump anywhere)
static void fuzz_player_segment_step(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  // the race's own table, or (half the time) one of the ROM's tables holding kinds 6 and 10
  // (the movies end on a track whose table has no segment within reach)
  uint16_t base = (uint16_t)(ram[0x019C] | ram[0x019D] << 8);
  uint16_t c = (uint16_t)(ram[0x1E5E] | ram[0x1E5F] << 8);
  if(rnd() & 1) {
    static const uint16_t tables[] = {0xC1C1, 0xE171, 0xE675, 0xF257};
    base = tables[rnd() % 4];
    c = (uint16_t)(0x14 * (rnd() % 4));
    wr16ram(ram, 0x019C, base);
  }
  wr16ram(ram, 0x1E5E, c);
  wr16ram(ram, 0x1E60, (uint16_t)(c + 0x14 * (rnd() % 3)));
  const uint16_t start = rom16(0x840000 + (uint32_t)(uint16_t)(base + c));
  // the place stays well before the first segment past the table's end (a kind other than
  // 0-10, even), even after the armed jump and $1EE6 move it on (up to $7F each), so the
  // original never jumps through a stray table entry
  uint16_t place = (uint16_t)(start + rnd() % 0x180);
  for(uint16_t s = c, n = 0; n < 40; s += 0x14, n++) {
    const uint8_t kind = rom8(0x840002 + (uint32_t)(uint16_t)(base + s));
    if(kind <= 10 && !(kind & 1)) continue;
    const uint16_t limit = (uint16_t)(rom16(0x840000 + (uint32_t)(uint16_t)(base + s)) - 0x00C0);
    if((uint16_t)(limit - start) & 0x8000) place = (uint16_t)(start - 0x0100);
    else if((uint16_t)(place - start) > (uint16_t)(limit - start)) place = limit;
    break;
  }
  wr16ram(ram, 0x0226, place);
  wr16ram(ram, 0x1EDE, (rnd() & 1) ? 0x7FFF : (uint16_t)(place - 0x20 + rnd() % 0x40));
  static const uint16_t words[] = {0x1EE0, 0x023E, 0x0206, 0x0204, 0x1EC2, 0x1EC4, 0x1EE6, 0x1E66};
  for(unsigned i = 0; i < sizeof words / sizeof words[0]; i++) wr16ram(ram, words[i], (uint16_t)rnd());
  wr16ram(ram, 0x1EE0, (uint16_t)(rnd() % 0x80));
  wr16ram(ram, 0x1EE6, (uint16_t)(rnd() % 0x80));
  for(int i = 0; i < 0x22; i++) wr16ram(ram, (uint16_t)(0x3DEC + 2 * i), (uint16_t)rnd());
  if(rnd() % 3 == 0) {  // a later segment of a never-run kind, if the table has one
    for(uint16_t s = c, n = 0; n < 200; s += 0x14, n++) {
      const uint8_t kind = rom8(0x840002 + (uint32_t)(uint16_t)(base + s));
      if(kind > 10 || (kind & 1)) break;  // the table's end
      if(kind == 6 || kind == 8 || kind == 10) {
        wr16ram(ram, 0x1E5E, s);
        wr16ram(ram, 0x1E60, s);
        wr16ram(ram, 0x0226, (uint16_t)(rom16(0x840000 + (uint32_t)(uint16_t)(base + s)) + 0x40 + rnd() % 0x40));
        wr16ram(ram, 0x1EDE, 0x7FFF);  // no armed jump to carry the place further
        wr16ram(ram, 0x1EE6, 0);
        break;
      }
    }
  }
}

// ---- player2_segment_step ($82:FA4F) ----  as player_segment_step (the same continuations);
// with only 5 watched returns every flag counts as live: they are the last SBC's, the
// second cursor's segment start against $02D9 + 2 (N set, Z clear, C and V of the SBC)
static void call_player2_segment_step(const Regs* in, Regs* out) {
  (void)in;
  player2_segment_step();
  const uint32_t a = (uint32_t)rd16(0x02D9) + 2;
  const uint16_t start = rom16(0x840000 + (uint32_t)(uint16_t)(rd16(0x019C) + rd16(0x1E64)));
  const uint32_t sum = (a & 0xFFFF) + (uint16_t)~start + (a >> 16);
  const uint16_t r = (uint16_t)sum;
  out->n = true;
  out->z = false;
  out->c = sum > 0xFFFF;
  out->v = (((uint16_t)a ^ start) & ((uint16_t)a ^ r) & 0x8000) != 0;
}
// as player_segment_step's fuzz, with the second player's words (and cursors)
static void fuzz_player2_segment_step(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  uint16_t base = (uint16_t)(ram[0x019C] | ram[0x019D] << 8);
  uint16_t c = (uint16_t)(ram[0x1E62] | ram[0x1E63] << 8);
  if(rnd() & 1) {
    static const uint16_t tables[] = {0xC1C1, 0xE171, 0xE675, 0xF257};
    base = tables[rnd() % 4];
    c = (uint16_t)(0x14 * (rnd() % 4));
    wr16ram(ram, 0x019C, base);
  }
  wr16ram(ram, 0x1E62, c);
  wr16ram(ram, 0x1E64, (uint16_t)(c + 0x14 * (rnd() % 3)));
  const uint16_t start = rom16(0x840000 + (uint32_t)(uint16_t)(base + c));
  uint16_t place = (uint16_t)(start + rnd() % 0x180);
  for(uint16_t s = c, n = 0; n < 40; s += 0x14, n++) {
    const uint8_t kind = rom8(0x840002 + (uint32_t)(uint16_t)(base + s));
    if(kind <= 10 && !(kind & 1)) continue;
    const uint16_t limit = (uint16_t)(rom16(0x840000 + (uint32_t)(uint16_t)(base + s)) - 0x00C0);
    if((uint16_t)(limit - start) & 0x8000) place = (uint16_t)(start - 0x0100);
    else if((uint16_t)(place - start) > (uint16_t)(limit - start)) place = limit;
    break;
  }
  wr16ram(ram, 0x02D9, place);
  wr16ram(ram, 0x1EE2, (rnd() & 1) ? 0x7FFF : (uint16_t)(place - 0x20 + rnd() % 0x40));
  static const uint16_t words[] = {0x02F1, 0x02B9, 0x02B7, 0x1EC6, 0x1EC8, 0x1E68};
  for(unsigned i = 0; i < sizeof words / sizeof words[0]; i++) wr16ram(ram, words[i], (uint16_t)rnd());
  wr16ram(ram, 0x1EE4, (uint16_t)(rnd() % 0x80));
  wr16ram(ram, 0x1EE8, (uint16_t)(rnd() % 0x80));
  for(int i = 0; i < 0x22; i++) wr16ram(ram, (uint16_t)(0x436C + 2 * i), (uint16_t)rnd());
}

// ---- road_shift_by_place ($82:EEB7) ----  its caller pulls and stores its saved words next:
// no register outputs. With 62 watched returns every flag counts as live: N and Z of the
// final PLA ($1E66 as it was, or without bit 0), V of the callees' last ADC ($1EF4:$1EF6
// plus the move, as in their adapters), C of it too after the apply, 0 after the settle
// (the ROL of 0)
static void call_road_shift_by_place(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t lo = rd16(0x1EF4), hi = rd16(0x1EF6), flags = rd16(0x1E66);
  const bool apply = (rd16(0x0226) & 0x0020) != 0;
  road_shift_by_place();
  const uint16_t at = rd16(0x0204), atHi = rd16(0x0206);
  const uint32_t c0 = ((uint32_t)lo + at) >> 16;
  const uint32_t sum = (uint32_t)atHi + hi + c0;
  out->c = apply && sum > 0xFFFF;
  out->v = (~(atHi ^ hi) & (atHi ^ (uint16_t)sum) & 0x8000) != 0;
  setNZ16(out, apply ? flags : (uint16_t)(flags & 0xFFFE));
}
// both callees' inputs (road_shift_settle's fuzz covers road_shift_apply's), bit 5 of
// $0226 either way, $1E66's bits 15 and 0 all four ways
static void fuzz_road_shift_by_place(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_road_shift_settle(r, ram, rnd);
  wr16ram(ram, 0x0226, (uint16_t)rnd());
  wr16ram(ram, 0x1E66, (uint16_t)rnd());
}

// ---- road_shift_frame ($82:EC36) ----  its caller returns into a routine that loads its own
// registers: no register outputs but DB ($83, its PLB); the bench judges the flags
static void call_road_shift_frame(const Regs* in, Regs* out) {
  (void)in;
  road_shift_frame();
  out->db = 0x83;
}
// the handlers' inputs (road_shift_by_place's fuzz covers the shift's), both handler
// indexes over their tables (the never-run handlers among them), the rows the edges read,
// the place, t, $01CE either sign, the step (0 now and then: the divider's $FFFF).
// The row fuzzes on the game's own stack (fuzzStack, entry SP $1FEF as measured live): the
// usual fuzz stack at $1EFE lies over the game's words $1EEE-$1EFD, which its handlers write.
static void fuzz_road_shift_frame(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_road_shift_by_place(r, ram, rnd);
  ram[0x0261] = (uint8_t)(2 * (rnd() % 2));
  ram[0x0260] = (uint8_t)(2 * (rnd() % 6));
  wr16ram(ram, 0x0224, (uint16_t)rnd());
  static const uint16_t rows[] = {0x3C6C, 0x3F6C, 0x3E6C, 0x3DEC};
  for(unsigned k = 0; k < 4; k++)
    for(int i = 0; i < 0x22; i++) wr16ram(ram, (uint16_t)(rows[k] + 2 * i), (uint16_t)(rnd() % 0x400 - 0x200));
  static const uint16_t words[] = {0x01CE, 0x1EF8, 0x1EFA, 0x0202, 0x0204, 0x0206, 0x1EF4, 0x1EF6};
  for(unsigned i = 0; i < sizeof words / sizeof words[0]; i++) wr16ram(ram, words[i], (uint16_t)rnd());
  if(rnd() & 1) wr16ram(ram, 0x0202, (uint16_t)(rnd() % 0x200 - 0x100));
  wr16ram(ram, 0x1E9E, (uint16_t)(rnd() % 16 == 0 ? (rnd() & 0xFF00) : rnd()));
  wr16ram(ram, 0x1EFA, (uint16_t)rnd());
  // the road's place at t, as the routine computes it (the row $3DEC and its slope)
  const uint32_t x = (uint16_t)((ram[0x0226] | ram[0x0227] << 8) << 1) & 0x003E;
  const uint16_t t = (uint16_t)(((ram[0x0224] | ram[0x0225] << 8) >> 1) & 0xFF80);
  const uint16_t row = (uint16_t)(ram[0x3DEC + x] | ram[0x3DED + x] << 8);
  const uint16_t next = (uint16_t)(ram[0x3DEE + x] | ram[0x3DEF + x] << 8);
  const int32_t p = (int32_t)(int16_t)(uint16_t)((uint16_t)(next - row) << 1) * (int16_t)t;
  uint16_t at = (uint16_t)((uint16_t)((uint32_t)p >> 16) + row);
  if(at != 0 && !(at & 0x8000)) at--;
  // half the time the car between the road's two edges (the rows $3C6C below and $3F6C above
  // its place), so the edges are set and the edge shift scales, both handlers paired
  if(rnd() & 1) {
    for(int i = 0; i < 0x22; i++) {
      wr16ram(ram, (uint16_t)(0x3C6C + 2 * i), (uint16_t)(-0x100 + rnd() % 0x20));
      wr16ram(ram, (uint16_t)(0x3F6C + 2 * i), (uint16_t)(0x100 + rnd() % 0x20));
    }
    wr16ram(ram, 0x0202, (uint16_t)-(uint16_t)(at + rnd() % 0x80 - 0x40));
    ram[0x0261] = 2;
    if(rnd() & 1) ram[0x0260] = 2;
  }
}

// ---- road_shift_frame_call ($82:EC32) ----  JSR road_shift_frame; RTS (as it)
static void call_road_shift_frame_call(const Regs* in, Regs* out) {
  (void)in;
  road_shift_frame_call();
  out->db = 0x83;
}

// ---- track_event_none ($82:EA80) ----  RTS: nothing changes
static void call_track_event_none(const Regs* in, Regs* out) { (void)in; (void)out; track_event_none(); }

// ---- player_segment_step_long / player2_segment_step_long ($82:A9AF, $82:A9B3) ----  JSR;
// RTL. The bank-$85 caller goes on to code that reads no register; with 5 watched returns
// every flag counts as live: the callee's last SBC, the second cursor's segment start
// against the place + 2 (N set, Z clear, C and V of the SBC; see player2_segment_step)
static void segmentStepFlags(Regs* out, uint16_t place, uint16_t cursor2) {
  const uint32_t a = (uint32_t)rd16(place) + 2;
  const uint16_t start = rom16(0x840000 + (uint32_t)(uint16_t)(rd16(0x019C) + rd16(cursor2)));
  const uint32_t sum = (a & 0xFFFF) + (uint16_t)~start + (a >> 16);
  out->n = true;
  out->z = false;
  out->c = sum > 0xFFFF;
  out->v = (((uint16_t)a ^ start) & ((uint16_t)a ^ (uint16_t)sum) & 0x8000) != 0;
}
static void call_player_segment_step_long(const Regs* in, Regs* out) {
  (void)in;
  player_segment_step_long();
  segmentStepFlags(out, 0x0226, 0x1E60);
}
static void call_player2_segment_step_long(const Regs* in, Regs* out) {
  (void)in;
  player2_segment_step_long();
  segmentStepFlags(out, 0x02D9, 0x1E64);
}

// ---- track_setup_load ($82:A7F9) ----  its caller goes on to track_rows_build, which loads
// its own registers: no register outputs. With 5 watched returns every flag counts as live:
// N set and Z clear (the final PLB of $83). C: the last object kind's walk (from the table
// the routine unpacked): its ASL clears it, the slot test (CMP #$FFFF) sets it for a free
// slot, whose ADC (the graphics' length plus D+$14) then sets it from the sum; with no kind,
// the variant's CMP #2. V: the last such ADC; else the SBC #$8007 of a bank-$85 entry
// looked up through the header; else 0 (decompress's, from a destination of 0).
static void call_track_setup_load(const Regs* in, Regs* out) {
  (void)in;
  track_setup_load();
  out->n = true;
  out->z = false;
  bool c = (rd16(0x03DC) & 0x0003) >= 2, v = false;
  const uint32_t x2 = (uint16_t)(rd16(0x047B) << 1);
  const bool lookup = (rom16((rd8(0x039C) ? 0x85A3CC : 0x85A35E) + x2) & 0x8000) ||
                      (rom16((rd8(0x03CB) ? 0x85A3CC : 0x85A35E) + x2) & 0x8000);
  if(lookup) {
    const uint16_t w = rd16(0x5395), r = (uint16_t)(w - 0x8007);
    v = ((w ^ 0x8007) & (w ^ r) & 0x8000) != 0;
  }
  // the walk again, over the slots as they ended: a slot is free when first met; the last
  // free slot's ADC wrote the final D+$14 from its place (the slot's value)
  static bool seen[0x10000];
  memset(seen, 0, sizeof seen);
  int32_t lastFree = -1;
  bool lastWasFree = false, anyKind = false;
  for(uint16_t y = (uint16_t)(rd16(0x019A) << 2); y != 0; y = (uint16_t)(y - 4)) {
    const uint8_t kind = rd8(0x8CB0 + (uint32_t)y);
    if(kind == 0) continue;
    anyKind = true;
    lastWasFree = false;
    const uint16_t x = rom16(0x86AE36 + 2u * kind);
    if(x == 0) continue;
    const uint16_t s = rom16(0x860000 + (uint32_t)x);
    if(seen[s]) continue;
    seen[s] = true;
    lastFree = s;
    lastWasFree = true;
  }
  if(anyKind) c = false;
  if(lastFree >= 0) {
    const uint16_t old = rd16(0x6591 + (uint32_t)lastFree), now = rd16(DP + 0x14);
    const uint16_t len = (uint16_t)(now - old);
    v = (~(len ^ old) & (len ^ now) & 0x8000) != 0;
    if(lastWasFree) c = now < old;
  }
  out->c = c;
  out->v = v;
}
// the race's track (its object table and $019A agree), the variant and $01E4 on all sides,
// the bank-$85 tables' selectors
static void fuzz_track_setup_load(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  ram[0x03DC] = (uint8_t)((ram[0x03DC] & 0xFC) | (rnd() % 4));
  static const uint16_t sides[] = {0x8000, 0x0000, 0x0001};
  wr16ram(ram, 0x01E4, (uint16_t)(sides[rnd() % 3] | (rnd() & 0x00F0)));
  ram[0x039C] = (uint8_t)((rnd() & 1) ? 0 : rnd());
  ram[0x03CB] = (uint8_t)((rnd() & 1) ? 0 : rnd());
}

// ---- results_text ($8A:9908) ----  its caller goes on to clear_vram: no register outputs.
// With 4 watched returns every flag counts as live: N Z C of the last CMP #$FF (equal), V
// = bit 14 of $1B37 (its BIT; no callee after it touches V)
static void call_results_text(const Regs* in, Regs* out) {
  (void)in;
  results_text();
  out->n = false;
  out->z = out->c = true;
  out->v = (rd16(0x1B37) & 0x4000) != 0;
}
// results_text_build's inputs (the variant, the lines), the slots' state, the results'
// values and names; $1B37's bit 15 (the never-run variant) and 14 both ways
static void fuzz_results_text_build(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
static void fuzz_func_8A9BB5(Regs* r, uint8_t* ram, uint32_t (*rnd)(void));
static void fuzz_results_text(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_results_text_build(r, ram, rnd);
  fuzz_func_8A9BB5(r, ram, rnd);
  for(uint32_t s = 0; s < 0x168; s += 2) wr16ram(ram, 0x041C + s, (uint16_t)(rnd() % 16));  // small: the slot taker walks the kinds down one by one
  wr16ram(ram, 0x1B37, (uint16_t)rnd());
  // the byte buffer's count at a line's start (D+$60, a word: its INC is 16-bit, and a
  // high byte set makes byte_buffer_pad push ~65536 spaces) and its index (D+$58) small, as
  // in the game: the push is a long store to $7F:7D00 + the index, which past $82FF crosses
  // into bank $80 (low WRAM: the direct page and the stack)
  wr16ram(ram, DP + 0x60, 0);
  wr16ram(ram, DP + 0x58, (uint16_t)(rnd() % 0x100));
  // the slots' names: each text map line (16 bytes at $7F:7918) a short name and its zero,
  // as in the game (a name past the padding column makes byte_buffer_pad wrap its count)
  for(uint32_t line = 0; line < 0xB4; line++) {
    const uint32_t at = 0x17918 + 16 * line, n = 1 + rnd() % 12;
    for(uint32_t i = 0; i < n; i++) ram[at + i] = (uint8_t)(0x41 + rnd() % 26);
    ram[at + n] = 0;
  }
}

// ---- save_data_check ($80:8119) ----  A = $00FF (N clear), Z of the TSB/TRB of $045A (its
// low byte was 0), C = the C's result. The stored copy valid: X = $FAB7, Y = $03E2 (past
// the MVN), V of the first sum's last ADC. The data valid: X = 0 (the loop's end), Y as on
// entry, V of the second sum's last ADC. Else save_data_seal's exit: X = $03E0, Y = $FAB5,
// V of its sum's last ADC (over the defaults now at $0368)
static uint16_t saveSum(uint32_t at) {
  uint16_t sum = 0;
  for(uint32_t x = 0x78; x != 0; x--) sum = (uint16_t)(sum + rd8(at + x));
  return (uint16_t)(sum ^ 0x7645);
}
static bool saveSumV(uint32_t at) {
  uint16_t sum = 0, last = 0;
  for(uint32_t x = 0x78; x != 0; x--) {
    last = sum;
    sum = (uint16_t)(sum + rd8(at + x));
  }
  const uint16_t b = rd8(at + 1);
  return (~(last ^ b) & (last ^ sum) & 0x8000) != 0;
}
static void call_save_data_check(const Regs* in, Regs* out) {
  const uint16_t old = rd16(0x045A);
  const bool copyValid = saveSum(0xFA3C) == rd16(0xFA3B);
  const bool v1 = saveSumV(0xFA3C);
  const bool dataValid = !copyValid && saveSum(0x0367) == rd16(0x0366);
  const bool v2 = saveSumV(0x0367);
  out->c = save_data_check();
  out->a = 0x00FF;
  out->n = false;
  out->z = (old & 0x00FF) == 0;
  if(copyValid) {
    out->x = 0xFAB7;
    out->y = 0x03E2;
    out->v = v1;
  } else if(dataValid) {
    out->x = 0;
    out->y = in->y;
    out->v = v2;
  } else {
    out->x = 0x03E0;
    out->y = 0xFAB5;
    out->v = saveSumV(0x0367);
  }
}
// the saved data random; a third of the time its stored copy, a third the data itself, made
// valid (its checksum written); $045A's low byte 0 now and then
static void fuzz_save_data_check(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(uint32_t i = 0; i < 0x7A; i++) {
    ram[0xFA3B + i] = (uint8_t)rnd();
    ram[0x0366 + i] = (uint8_t)rnd();
  }
  const uint32_t k = rnd() % 3;
  if(k < 2) {
    const uint32_t at = k == 0 ? 0xFA3C : 0x0367;
    uint16_t sum = 0;
    for(uint32_t x = 0x78; x != 0; x--) sum = (uint16_t)(sum + ram[at + x]);
    wr16ram(ram, at - 1, (uint16_t)(sum ^ 0x7645));
  }
  wr16ram(ram, 0x045A, (uint16_t)((rnd() & 1) ? rnd() & 0xFF00 : rnd()));
}

// ---- oam_high_load ($82:AE15) ----  a DSP-4 tape row. DB = $83 (its PLB); V of the ADC of
// the room, N clear, Z set (the ADC's 0, or the fill's last DEX); none left: C of the ADC,
// A = 0, X as on entry, Y = D+$86; else C = the room's bit 1 (its second LSR), A = $E0E0,
// X = 0, Y past the last word filled
static void oamHighOut(const Regs* in, Regs* out, uint16_t d86, uint16_t d8e) {
  const uint16_t e = (uint16_t)~(uint16_t)(d86 - d8e);
  const uint32_t sum = (uint32_t)e + 0x0200 + 1;
  const uint16_t room = (uint16_t)sum;
  out->c = sum > 0xFFFF;
  out->v = (~(e ^ 0x0200) & (e ^ room) & 0x8000) != 0;
  out->db = 0x83;
  out->n = false;
  out->z = true;
  if(room == 0) {
    out->a = 0;
    out->x = in->x;
    out->y = d86;
  } else {
    out->c = (room >> 1) & 1;
    out->a = 0xE0E0;
    out->x = 0;
    out->y = (uint16_t)(d86 + 4 * (uint32_t)(uint16_t)(room >> 2));
  }
}
static void call_oam_high_load(const Regs* in, Regs* out) {
  const uint16_t d86 = rd16(DP + 0x86), d8e = rd16(DP + 0x8E);
  oam_high_load();
  oamHighOut(in, out, d86, d8e);
}
// the two sprite pointers (the high table's place, the sprites used: up to $200 bytes more,
// in steps of 4), the chip's 16 answers scripted (now and then fewer: the rest read 0)
static void fuzz_oam_high_load(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  const uint16_t d8e = (uint16_t)(4 * (rnd() % 0x80));
  wr16ram(ram, DP + 0x8E, d8e);
  wr16ram(ram, DP + 0x86, (uint16_t)(d8e + 4 * (rnd() % 0x81)));
  uint16_t words[16];
  for(int i = 0; i < 16; i++) words[i] = (uint16_t)rnd();
  dsp4_bridge_script(words, (rnd() % 4 == 0) ? (int)(rnd() % 16) : 16);
}
// oam_high_load2 ($82:AEA4, vs-race-31): the same with D+$88 for D+$86; its fuzz the same with D+$88
static void call_oam_high_load2(const Regs* in, Regs* out) {
  const uint16_t d88 = rd16(DP + 0x88), d8e = rd16(DP + 0x8E);
  oam_high_load2();
  oamHighOut(in, out, d88, d8e);
}
static void fuzz_oam_high_load2(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  const uint16_t d8e = (uint16_t)(4 * (rnd() % 0x80));
  wr16ram(ram, DP + 0x8E, d8e);
  wr16ram(ram, DP + 0x88, (uint16_t)(d8e + 4 * (rnd() % 0x81)));
  uint16_t words[16];
  for(int i = 0; i < 16; i++) words[i] = (uint16_t)rnd();
  dsp4_bridge_script(words, (rnd() % 4 == 0) ? (int)(rnd() % 16) : 16);
}
// Negative row: the command word $0007 instead of $0006 (the tape's first write differs).
// One word fewer read (oamHighWrong(false, 15): the tape left unconsumed) was run once as a
// second negative row and failed as it must; two rows cannot share an entry in one run
static void oamHighWrong(bool wrongCommand, int words) {
  dsp4_port_write16(wrongCommand ? 0x0007 : 0x0006);
  const uint32_t y = rd16(DP + 0x8E);
  for(int i = 0; i < words; i++) wr16(0x1776 + y + 2u * (uint32_t)i, dsp4_port_read16());
  const uint16_t room = (uint16_t)(0x0200 - (uint16_t)(rd16(DP + 0x86) - rd16(DP + 0x8E)));
  if(room == 0) return;
  uint16_t n = (uint16_t)(room >> 2), at = rd16(DP + 0x86);
  do {
    wr16(0x1576 + (uint32_t)at, 0xE0E0);
    at = (uint16_t)(at + 4);
    n = (uint16_t)(n - 1);
  } while(n != 0);
}
static void call_oam_high_load_wrong_command(const Regs* in, Regs* out) {
  const uint16_t d86 = rd16(DP + 0x86), d8e = rd16(DP + 0x8E);
  oamHighWrong(true, 16);
  oamHighOut(in, out, d86, d8e);
}

// ---- race_frame_draw ($82:8923) ----  a DSP-4 tape row over its first five callees (their
// native commands are judged as whole commands, harness-5). It ends at its tail jump to
// oam_high_load (endAt, as hud_markers_write: oam_high_load keeps its own row and samples),
// with func_82A9B7's DB = $83, N set and Z clear (its PLB); oam_high_load reloads the flags
// before reading any, and A X Y are not read. Every live entry is interrupted (it takes most
// of a frame): its cases are the two replays and the fuzz, where the stand-in chip answers
static void call_race_frame_draw(const Regs* in, Regs* out) {
  (void)in;
  race_frame_draw_body();
  out->db = 0x83;
  out->n = true;
  out->z = false;
}
// the buffer ($00B4) either way, the shake's and the view targets' inputs as their rows fuzz
// them; the race as the movie left it otherwise
static void fuzz_race_frame_draw(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  wr16ram(ram, 0x00B4, (uint16_t)(rnd() & 1 ? 0x0001 : 0));
  if(rnd() & 1) fuzz_shake_step(r, ram, rnd);
  if(rnd() & 1) fuzz_func_82A9B7(r, ram, rnd);
}

// ---- sprite_put_rows_a / _b ($82:D934, $82:D988) ----  DSP-4 tape rows. C = the buffer
// filled (the callers' BCS); the other flags by liveness; A X Y not read (the callers
// return or reload them)
static void call_sprite_put_rows_a(const Regs* in, Regs* out) { (void)in; out->c = sprite_put_rows_a(); }
static void call_sprite_put_rows_b(const Regs* in, Regs* out) { (void)in; out->c = sprite_put_rows_b(); }
// the race's object (D+$14, D+$24 as it left them: their tables lead into ROM); the parts
// lists written where the routine will look (up to three parts in each, then their 0
// word); the OAM pointer, its limit on a pair now and then (the buffer fills); the chip's
// answers scripted: 40 words, a third of them 0 (each ends a part's pairs)
static void fuzz_sprite_put_rows(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  const uint16_t d14 = (uint16_t)(ram[DP + 0x14] | ram[DP + 0x15] << 8);
  const uint16_t d24 = (uint16_t)(ram[DP + 0x24] | ram[DP + 0x25] << 8);
  const uint16_t slot = rom16(0x860000 + (uint32_t)d24);
  const uint16_t place = (uint16_t)(ram[0x6591 + (uint32_t)slot] | ram[0x6592 + (uint32_t)slot] << 8);
  const uint16_t row = (uint16_t)(ram[0x39EC + (uint32_t)d14] | ram[0x39ED + (uint32_t)d14] << 8);
  const uint16_t set = (uint16_t)(rom16(0x848000 + (uint32_t)row) + d24);
  uint16_t x = (uint16_t)(place + rom16(0x860002 + (uint32_t)set));
  for(int list = 0; list < 2; list++) {
    // the first list empty half the time, so the second one fills the buffer too
    const int parts = (list == 0 && (rnd() & 1)) ? 0 : (int)(rnd() % 4);
    for(int i = 0; i < parts; i++, x = (uint16_t)(x + 6)) {
      wr16ram(ram, 0x10000 + (uint32_t)x, (uint16_t)(1 + rnd() % 0xFFFF));
      wr16ram(ram, 0x10002 + (uint32_t)x, (uint16_t)rnd());
      wr16ram(ram, 0x10004 + (uint32_t)x, (uint16_t)rnd());
    }
    wr16ram(ram, 0x10000 + (uint32_t)x, 0);
    x = (uint16_t)(x + 2);
  }
  const uint16_t at = (uint16_t)(4 * (rnd() % 0x40));
  wr16ram(ram, DP + 0x84, at);
  wr16ram(ram, DP + 0x1A, (rnd() & 1) ? (uint16_t)(at + 4 * (1 + rnd() % 6)) : (uint16_t)(at + 2));
  static const uint16_t words[] = {DP + 0x20, DP + 0x22, DP + 0x26, DP + 0x3A};
  for(unsigned i = 0; i < sizeof words / sizeof words[0]; i++) wr16ram(ram, words[i], (uint16_t)rnd());
  uint16_t script[40];
  for(int i = 0; i < 40; i++) script[i] = (rnd() % 3 == 0) ? 0 : (uint16_t)rnd();
  dsp4_bridge_script(script, 40);
}

// ---- sprite_parts_stream ($82:DDBF, vs-race-36) ----  a DSP-4 tape row as sprite_parts_stream_rom
// (C = the buffer filled): two parts lists written at a random place in bank $7F (up to three
// parts each, the first empty half the time), X their start; the OAM pointer and its limit, the
// answers scripted as for sprite_put_rows_a
static void call_sprite_parts_stream(const Regs* in, Regs* out) { out->c = sprite_parts_stream(in->x); }
static void fuzz_sprite_parts_stream(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint16_t x = (uint16_t)(0x2000 + 2 * (rnd() % 0x6000));
  r->x = x;
  for(int list = 0; list < 2; list++) {
    const int parts = (list == 0 && (rnd() & 1)) ? 0 : (int)(rnd() % 4);
    for(int i = 0; i < parts; i++, x = (uint16_t)(x + 6)) {
      wr16ram(ram, 0x10000 + (uint32_t)x, (uint16_t)(1 + rnd() % 0xFFFF));
      wr16ram(ram, 0x10002 + (uint32_t)x, (uint16_t)rnd());
      wr16ram(ram, 0x10004 + (uint32_t)x, (uint16_t)rnd());
    }
    wr16ram(ram, 0x10000 + (uint32_t)x, 0);
    x = (uint16_t)(x + 2);
  }
  const uint16_t at = (uint16_t)(4 * (rnd() % 0x40));
  wr16ram(ram, DP + 0x84, at);
  wr16ram(ram, DP + 0x1A, (rnd() & 1) ? (uint16_t)(at + 4 * (1 + rnd() % 6)) : (uint16_t)(at + 2));
  uint16_t script[40];
  for(int i = 0; i < 40; i++) script[i] = (rnd() % 3 == 0) ? 0 : (uint16_t)rnd();
  dsp4_bridge_script(script, 40);
}
// ---- sprite_parts_stream_rom ($82:DEA8) ----  a DSP-4 tape row: as sprite_put_rows_a (C =
// the buffer filled; the other flags by liveness; its callers restore X and Y)
static void call_sprite_parts_stream_rom(const Regs* in, Regs* out) { out->c = sprite_parts_stream_rom(in->x); }
// a ROM parts list whose two lists end within 8 parts each, as the game's do (X a random
// offset from $8000 on, searched); the OAM pointer and its limit, the answers scripted as
// for sprite_put_rows_a
static bool romPartsEnd(uint16_t* x) {
  for(int n = 0; n < 8; n++, *x = (uint16_t)(*x + 6)) {
    if(*x < 0x8000) return false;
    if(rom16(0x860000 + (uint32_t)*x) == 0) return true;
  }
  return false;
}
static void fuzz_sprite_parts_stream_rom(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  for(int tries = 0; tries < 4000; tries++) {
    uint16_t x = (uint16_t)(0x8000 + 2 * (rnd() % 0x3FF0)), at = x;
    if(!romPartsEnd(&at)) continue;
    at = (uint16_t)(at + 2);
    if(!romPartsEnd(&at)) continue;
    r->x = x;
    break;
  }
  const uint16_t at = (uint16_t)(4 * (rnd() % 0x40));
  wr16ram(ram, DP + 0x84, at);
  wr16ram(ram, DP + 0x1A, (rnd() & 1) ? (uint16_t)(at + 4 * (1 + rnd() % 6)) : (uint16_t)(at + 2));
  uint16_t script[40];
  for(int i = 0; i < 40; i++) script[i] = (rnd() % 3 == 0) ? 0 : (uint16_t)rnd();
  dsp4_bridge_script(script, 40);
}

// ---- car_sprite_head ($82:D267) ----  a DSP-4 tape row: A = the answer (N Z of its LDA), X
// = Y (its last TYX; the caller reads $FABF,x next), Y kept; C V of the frame's ADC
static void call_car_sprite_head(const Regs* in, Regs* out) {
  const uint16_t a = (uint16_t)((rd16(0xFAB5 + (uint32_t)in->y) >> 1) & 0x007E);
  const uint16_t row = rd16(0x39EC + (uint32_t)rd16(DP + 0x14));
  out->a = car_sprite_head(in->x, in->y);
  out->x = in->y;
  out->y = in->y;
  setNZ16(out, out->a);
  const uint32_t sum = (uint32_t)a + row;
  out->c = sum > 0xFFFF;
  out->v = (~(a ^ row) & (a ^ (uint16_t)sum) & 0x8000) != 0;
}
// the race's object (D+$14 and its row at $39EC: the frame table in bank $83 stays in ROM),
// a row offset X, a car Y (its words random), the height byte either sign, D+$32/$34, one
// scripted answer
static void fuzz_car_sprite_head(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)(2 * (rnd() % 0x40));
  r->y = (uint16_t)(0x35 * (rnd() % 20));
  wr16ram(ram, 0xFAB4 + (uint32_t)r->y, (uint16_t)rnd());
  wr16ram(ram, 0xFAB6 + (uint32_t)r->y, (uint16_t)rnd());
  wr16ram(ram, 0x386A + (uint32_t)r->x, (uint16_t)rnd());
  ram[0x0243] = (uint8_t)rnd();
  wr16ram(ram, DP + 0x32, (uint16_t)rnd());
  wr16ram(ram, DP + 0x34, (uint16_t)rnd());
  const uint16_t answer = (uint16_t)rnd();
  dsp4_bridge_script(&answer, 1);
}

// ---- object_sprite_raised / _flat ($82:DAFD, $82:DB4D) ----  DSP-4 tape rows: as the
// sprite_put_rows they jump to (C = the buffer filled; the other flags by liveness)
static void call_object_sprite_raised(const Regs* in, Regs* out) { out->c = object_sprite_raised(in->x); }
static void call_object_sprite_flat(const Regs* in, Regs* out) { out->c = object_sprite_flat(in->x); }
// a graphics entry X whose word leads into ROM (from $8000), put in D+$24 for the parts
// lists sprite_put_rows' fuzz writes; the object entry (its bytes random, $0024 random)
static void fuzz_object_sprite(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  uint16_t x = 0;
  for(int tries = 0; tries < 200; tries++) {
    x = (uint16_t)(2 * (rnd() % 0x80));
    if(rom16(0x86AF06 + (uint32_t)x) >= 0x8000) break;
  }
  wr16ram(ram, DP + 0x24, rom16(0x86AF06 + (uint32_t)x));
  fuzz_sprite_put_rows(r, ram, rnd);
  r->x = x;
  const uint16_t o = (uint16_t)(4 * (rnd() % 0x100));
  wr16ram(ram, DP + 0x1C, o);
  for(int i = 0; i < 4; i++) ram[0x8CB3 + (uint32_t)o + i] = (uint8_t)rnd();
  wr16ram(ram, 0x0024, (uint16_t)rnd());
}

// ---- object_sprite_group ($82:DB87) ----  a DSP-4 tape row: as object_sprite_flat (C = the
// buffer filled; the other flags by liveness)
static void call_object_sprite_group(const Regs* in, Regs* out) { out->c = object_sprite_group(in->x); }
// sprite_put_rows_c ($82:CAE5) and the VS object handlers vs_object_flat ($82:DD3B) and
// vs_object_group ($82:DD69), vs-race-34: tape rows as their one-player counterparts (C = the
// buffer filled; the other flags by liveness)
static void call_sprite_put_rows_c(const Regs* in, Regs* out) { (void)in; out->c = sprite_put_rows_c(); }
static void call_vs_object_flat(const Regs* in, Regs* out) { out->c = vs_object_flat(in->x); }
static void call_vs_object_group(const Regs* in, Regs* out) { out->c = vs_object_group(in->x); }
// object_sprite_kind0 ($82:DCEC, verify-cover-2): the VS race's table uses it for 15 kinds (the
// one-player's for none the movies run)
static void call_object_sprite_kind0(const Regs* in, Regs* out) { out->c = object_sprite_kind0(in->x); }

// ---- object_sprite_shadowed ($82:DA48) ----  a DSP-4 tape row: C = the buffer filled; N
// clear and Z set (every exit of the stream's tail follows a 0 word read or written); V
// clear (its ADCs, the graphics place plus an offset and the list's +6, stay small in the
// game). race1 reaches it 22 times, so all flags count there
static void call_object_sprite_shadowed(const Regs* in, Regs* out) {
  (void)in;
  out->c = object_sprite_shadowed(in->x);
  out->n = false;
  out->z = true;
  out->v = false;
}
// object_sprite's fuzz, with the parts lists for the second sprite's graphics word ($BC30)
// written first
static void fuzz_object_sprite_shadowed(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  wr16ram(ram, DP + 0x24, 0xBC30);
  fuzz_sprite_put_rows(r, ram, rnd);
  fuzz_object_sprite(r, ram, rnd);
  // the words its second sprite's test reads ($7E:8CB5 + the height row, up to $3E)
  for(uint32_t i = 0; i <= 0x40; i++) ram[0x8CB5 + i] = (uint8_t)rnd();
}

// ---- sprite_part_put ($81:8527) ----  a DSP-4 tape row: Y = past the last pair (the callers
// go on from it), C = the buffer filled, N clear and Z set (the flag's 0, or the CPY's
// equality), V of the ADC of D+$32 and -8
static void call_sprite_part_put(const Regs* in, Regs* out) {
  const uint16_t d32 = rd16(DP + 0x32);
  uint16_t y = in->y;
  out->c = sprite_part_put(in->x, &y);
  out->y = y;
  out->n = false;
  out->z = true;
  const uint16_t sum = (uint16_t)(0xFFF8 + d32);
  out->v = (~(0xFFF8 ^ d32) & (0xFFF8 ^ sum) & 0x8000) != 0;
}
// the part word X, D+$32 and D+$16, the OAM place Y and its limit (on a pair now and then),
// the chip's answers scripted (40 words, a third of them 0)
static void fuzz_sprite_part_put(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->x = (uint16_t)rnd();
  r->y = (uint16_t)(4 * (rnd() % 0x40));
  wr16ram(ram, DP + 0x1A, (rnd() & 1) ? (uint16_t)(r->y + 4 * (1 + rnd() % 6)) : (uint16_t)(r->y + 2));
  wr16ram(ram, DP + 0x32, (uint16_t)rnd());
  wr16ram(ram, DP + 0x16, (uint16_t)rnd());
  uint16_t script[40];
  for(int i = 0; i < 40; i++) script[i] = (rnd() % 3 == 0) ? 0 : (uint16_t)rnd();
  dsp4_bridge_script(script, 40);
}

// ---- sprite_bar_put ($81:8487) ----  a DSP-4 tape row (RTL). Length 0: A = 0 (Z set, N
// clear), C clear, X Y V as on entry. Else A = the old D+$14 (N Z of its PLA), X = $60F6, Y
// = the OAM place, C clear (the end part's), V of that part's ADC (D+$32 - 8)
static void call_sprite_bar_put(const Regs* in, Regs* out) {
  const uint16_t old14 = rd16(DP + 0x14);
  const bool full = sprite_bar_put(in->a, in->x, in->y);
  out->c = full;
  if(in->a == 0) {
    out->a = 0;
    out->n = false;
    out->z = true;
    return;
  }
  out->a = old14;
  setNZ16(out, old14);
  out->x = 0x60F6;
  out->y = rd16(DP + 0x84);
  const uint16_t sum = (uint16_t)(0xFFF8 + in->y);
  out->v = (~(0xFFF8 ^ in->y) & (0xFFF8 ^ sum) & 0x8000) != 0;
}
// a length (0 now and then, up to $60), its centre, D+$32 in Y; the OAM place never on its
// limit (the full buffer's path returns with two words left on the stack: never in a case);
// the chip's answers scripted
static void fuzz_sprite_bar_put(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  r->a = (rnd() % 6 == 0) ? 0 : (uint16_t)(1 + rnd() % 0x60);
  r->x = (uint16_t)rnd();
  r->y = (uint16_t)rnd();
  const uint16_t at = (uint16_t)(4 * (rnd() % 0x40));
  wr16ram(ram, DP + 0x84, at);
  wr16ram(ram, DP + 0x1A, (uint16_t)(at + 2));
  wr16ram(ram, DP + 0x14, (uint16_t)rnd());
  wr16ram(ram, DP + 0x16, (uint16_t)rnd());
  uint16_t script[60];
  for(int i = 0; i < 60; i++) script[i] = (rnd() % 3 == 0) ? 0 : (uint16_t)rnd();
  dsp4_bridge_script(script, 60);
}

// ---- race_grid_setup ($85:9ADB) ----  RTL; its caller reloads A X Y. With 5 entries every
// flag counts: N Z of the last load (the second player's car's +$06), C V of
// player2_segment_step_long's last SBC (nothing after it touches them)
static void call_race_grid_setup(const Regs* in, Regs* out) {
  (void)in;
  race_grid_setup();
  Regs t;
  segmentStepFlags(&t, 0x02D9, 0x1E64);
  out->c = t.c;
  out->v = t.v;
  setNZ16(out, rd16(0xFABB + 0x35));
}
// the race's track (its length, segment table, cursors, car order and tables by $047B stay
// as the race left them: the segment steps at the end walk its table); the variant, the
// players' lanes and the random state
static void fuzz_race_grid_setup(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  fuzz_rng_state(ram, rnd);
  ram[0x03DD] = (uint8_t)((ram[0x03DD] & 0xFC) | (rnd() % 4));
  ram[0x0392] = (uint8_t)(rnd() % 7);
  ram[0x03C1] = (uint8_t)(rnd() % 7);
}

// ---- race_state_reset ($82:A28B) ----  A = $FFFF, X = 0, Y = $140 (past the 40 tiles), N
// clear and Z set (the last DEX), C and V clear (the tiles' last ADC #8), DB = $83
static void call_race_state_reset(const Regs* in, Regs* out) {
  (void)in;
  race_state_reset();
  out->a = 0xFFFF;
  out->x = 0;
  out->y = 0x0140;
  out->n = false;
  out->z = true;
  out->c = false;
  out->v = false;
  out->db = 0x83;
}
// race_grid_setup's and track_palettes_load's inputs; the variant's bits 4-5 (both set now
// and then), the first player's car (5 now and then), the cars' order (a shuffle of 0-19),
// the players' cars (0-6), the palettes' colours
static void fuzz_race_state_reset(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_race_grid_setup(r, ram, rnd);
  fuzz_track_palettes_load(r, ram, rnd);
  ram[0x03DC] = (uint8_t)((ram[0x03DC] & 0xCF) | (rnd() % 4) << 4);
  wr16ram(ram, 0x0393, (uint16_t)((rnd() & 0xFF00) | ((rnd() & 1) ? 5 : rnd() % 8)));
  wr16ram(ram, 0x0395, (uint16_t)rnd());
  uint8_t order[20];
  for(int i = 0; i < 20; i++) order[i] = (uint8_t)i;
  for(int i = 19; i > 0; i--) {
    const int j = (int)(rnd() % (uint32_t)(i + 1));
    const uint8_t t = order[i];
    order[i] = order[j];
    order[j] = t;
  }
  for(int i = 0; i < 20; i++) ram[0x0446 + i] = order[i];
  ram[0x0392] = (uint8_t)(rnd() % 7);
  ram[0x03C1] = (uint8_t)(rnd() % 7);
  wr16ram(ram, 0x6A33, (uint16_t)rnd());
  wr16ram(ram, 0x6B33, (uint16_t)rnd());
}

// ---- road_rows_build ($82:DFAD) ----  its caller returns into code that loads its own
// registers: no register outputs but DB ($83, its last PLB); the bench judges the flags
static void call_road_rows_build(const Regs* in, Regs* out) {
  (void)in;
  road_rows_build();
  out->db = 0x83;
}
// ---- road_rows_build_call ($82:DFA9) ----  JSR road_rows_build; RTS: its outputs
static void call_road_rows_build_call(const Regs* in, Regs* out) {
  (void)in;
  road_rows_build_call();
  out->db = 0x83;
}
// the race's road (its tables, cursors and object stream as the race left them); 0-3 rows
// crossed (past the lap's end now and then), the multipliers' inputs, $1E66's bit 15, the
// armed jump near the rows; the next rows' objects of every kind the event table handles
// (both roads', the never-run ones among them)
static void fuzz_road_rows_build(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  const uint16_t laps = (uint16_t)(ram[0x019A] | ram[0x019B] << 8);
  uint16_t at = (uint16_t)(ram[0x023E] | ram[0x023F] << 8);
  // three kinds of case: (0) a segment table's rows, (1) the race's object stream rewound to
  // one of its entries, the place at it (the spawns run), (2) the lap's ends or the race's
  // own state
  const uint32_t scenario = rnd() % 3;
  bool rewound = false;
  if(scenario == 1) {
    uint32_t n = 0;  // the stream's entries in order (their places rising, below the lap)
    for(uint16_t last = 0; n < 64; n++) {
      const uint16_t place = (uint16_t)(ram[0x5B91 + 5 * n] | ram[0x5B92 + 5 * n] << 8);
      if(place < last || place >= laps) break;
      last = place;
    }
    if(n > 0) {
      const uint32_t j = rnd() % n;
      wr16ram(ram, DP + 0xA0, (uint16_t)(5 * j));
      // the place at that entry, or at the next one (both spawn in one frame)
      const uint32_t k = (j + 1 < n && (rnd() & 1)) ? j + 1 : j;
      at = (uint16_t)(ram[0x5B91 + 5 * k] | ram[0x5B92 + 5 * k] << 8);
      rewound = true;
    }
  }
  // near the lap's end: the rows (32 ahead) reach $019A, or the place itself passes it
  const bool passing = scenario == 2 && laps > 0x30 && rnd() % 3 == 0;
  if(scenario == 2 && laps > 0x30 && !passing && (rnd() & 1)) at = (uint16_t)(laps - 0x21 - rnd() % 3);
  if(passing) at = (uint16_t)(laps - 1 - rnd() % 2);
  static const uint16_t tables[] = {0xC1C1, 0xE171, 0xE675, 0xF257};
  uint16_t base = 0, seg = 0;
  uint32_t s = 0;
  for(int tries = 0; tries < 16; tries++) {  // a table and segment valid for this track (below)
    base = tables[rnd() % 4];
    seg = (uint16_t)(0x14 * (rnd() % 4));
    s = 0x840000 + (uint32_t)(uint16_t)(base + seg);
    bool ok = rom16(s) + 0x60 < laps;
    for(uint32_t k = 0; ok && k < 8 * 0x14; k += 0x14)
      if(rom16(s + k + 4) < 0x8000 || rom16(s + k + 6) < 0x8000 || rom16(s + k + 0x0A) > 2) ok = false;
    if(ok) break;
  }
  // one of the ROM's segment tables (only where its segment's curve lists stay in ROM), the
  // rows near one of its segments
  bool valid = true, exact = false;  // this segment and the next ones it may move on to
  for(uint32_t k = 0; k < 8 * 0x14; k += 0x14)
    if(rom16(s + k + 4) < 0x8000 || rom16(s + k + 6) < 0x8000 || rom16(s + k + 0x0A) > 2) valid = false;
  // (its rows below this track's length, as a row always is: the rows' table entries past
  // it are not the track's)
  if(rom16(s) + 0x60 >= laps) valid = false;
  if(scenario == 0 && valid) {
    wr16ram(ram, DP + 0x9C, base);
    wr16ram(ram, DP + 0xA2, seg);
    wr16ram(ram, 0x1EE6, 0);
    // the row (32 ahead) before, inside, or exactly $3E into the segment (then with bit 15
    // of $1E66 and no armed jump: the $8001 flag)
    exact = rnd() % 3 == 0;
    at = (uint16_t)(rom16(0x840000 + (uint32_t)(uint16_t)(base + seg)) - 0x20 + (exact ? 0x3E : rnd() % 0x80 - 2));
  }
  wr16ram(ram, 0x0224, (uint16_t)rnd());
  ram[0x0225] = (uint8_t)rnd();  // a byte: the word would reach $0226
  const bool lapStart = scenario == 2 && rnd() % 4 == 0;  // the lap's start (the tilt's rows wrap back)
  if(lapStart) at = (uint16_t)(rnd() % 3);
  if(rewound) {  // the place at the entry, 0-3 rows crossed to it
    wr16ram(ram, 0x0226, at);
    at = (uint16_t)(at >= 3 ? at - rnd() % 4 : at);
    wr16ram(ram, 0x023E, at);
  } else {
    wr16ram(ram, 0x023E, at);
    wr16ram(ram, 0x0226, lapStart ? at : passing ? (uint16_t)(at + 1 + rnd() % 3) : (uint16_t)(at + rnd() % 4));
  }
  wr16ram(ram, 0x023C, (rnd() & 1) ? 0 : (uint16_t)rnd());
  wr16ram(ram, 0x1E66, (uint16_t)rnd());
  const uint16_t d1ee6 = (uint16_t)(ram[0x1EE6] | ram[0x1EE7] << 8);
  uint16_t row = (uint16_t)(at + 0x20 + d1ee6);
  if(laps != 0 && row >= laps) row = (uint16_t)(row - laps);
  wr16ram(ram, 0x1EDE, (exact || scenario == 0 || (rnd() & 1)) ? 0x7FFF : (uint16_t)(row + rnd() % 4));
  if(exact) wr16ram(ram, 0x1E66, (uint16_t)(rnd() | 0x8000));
  else if(scenario == 0 && valid && rnd() % 3 != 0) {
    // past a segment with bit 15 clear: each segment passed adds its length to the row; only
    // when that whole chain (up to 32 segments) visits segments whose lists stay in ROM, ends,
    // and keeps the row inside the track
    uint16_t r16 = (uint16_t)(at + 0x20);
    uint32_t k = 0;
    bool fine = true;
    for(; k < 32; k++) {
      const uint32_t sk = s + 0x14 * k;
      if(rom16(sk + 4) < 0x8000 || rom16(sk + 6) < 0x8000 || rom16(sk + 0x0A) > 2 || r16 >= laps) {
        fine = false;
        break;
      }
      const uint16_t d = (uint16_t)(r16 - rom16(sk));
      if((d & 0x8000) || d < 0x40) break;
      r16 = (uint16_t)(r16 + rom16(sk + 0x10));
    }
    if(fine && k < 32 && r16 < laps) wr16ram(ram, 0x1E66, (uint16_t)(rnd() & 0x7FFF));
    else wr16ram(ram, 0x1E66, (uint16_t)(rnd() | 0x8000));
  } else if(scenario == 0 && valid) {
    wr16ram(ram, 0x1E66, (uint16_t)(rnd() | 0x8000));  // no lengths added: the row stays near the checked segments
  }
  // the road object stream's next entry (at D+$A0) now and then one without a kind (its lane
  // to $45EC), its place reached
  if(rnd() % 3 == 0) {
    const uint32_t ox = (uint16_t)(ram[DP + 0xA0] | ram[DP + 0xA1] << 8);
    wr16ram(ram, 0x5B91 + ox, (uint16_t)(ram[0x0226] | ram[0x0227] << 8));
    wr16ram(ram, 0x5B94 + ox, 0);
  }
  static const uint8_t kinds[] = {0, 1, 80, 81, 82, 83, 84, 95, 96, 97, 184, 185, 186, 187, 188, 199, 200, 201};
  for(int i = 0; i < 6; i++) {
    const uint32_t obj = (uint16_t)((uint16_t)(row + i) << 2);
    ram[0x8CB4 + obj] = kinds[rnd() % (sizeof kinds / sizeof kinds[0])];
    ram[0x8CB3 + obj] = (uint8_t)rnd();
    ram[0x8CB5 + obj] = (uint8_t)rnd();
  }
}

// ---- race_cars_frame ($81:E812) ----  RTL: A = the second car of the order ($1E6C), X =
// the last car ($1E6A), Y = 0, DB = $83; N set and Z clear (the PLB of $83), C set (the
// finish loop's CPY), V of the last wrap check's SBC (the leader's +$01 less $019A: its
// place is not touched after it)
static void call_race_cars_frame(const Regs* in, Regs* out) {
  (void)in;
  race_cars_frame();
  out->a = rd16(0x1E6C);
  out->x = rd16(0x1E6A);
  out->y = 0;
  out->c = true;
  out->n = true;
  out->z = false;
  const uint16_t at = rd16(0xFAB6 + (uint32_t)rd16(0x1E90)), lap = rd16(0x019A), d = (uint16_t)(at - lap);
  out->v = ((at ^ lap) & (at ^ d) & 0x8000) != 0;
  out->db = 0x83;
}
// the race as the movie left it, then one of: the order shuffled a little (swaps in the
// sort); two neighbours on one high word (contacts); 1-20 cars from the leader down past the
// lap's end (the forward wrap, its skipped 15th-17th cars and its uncounted last one); 1-19
// cars from the last up behind the line (the backward wrap); finishers (bit 0 without bit
// 1, +$1E = $1F06). Some cars get another handler (each of the table's but the road's), and the
// players' records direct or by the DSP-4. The cars a wrap case moves get the empty
// handler (6): the road step would walk its segment table for millions of instructions
static void fuzz_race_cars_frame(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  const uint16_t lap = (uint16_t)(ram[0x019A] | ram[0x019B] << 8);
  uint16_t order[20];
  for(int i = 0; i < 20; i++) order[i] = (uint16_t)(ram[0x1E6A + 2 * i] | ram[0x1E6B + 2 * i] << 8);
  const unsigned scenario = rnd() % 5;
  switch(scenario) {
    case 0:
      for(int n = 1 + (int)(rnd() % 3); n > 0; n--) {
        const int i = (int)(rnd() % 19);
        const uint16_t t = order[i];
        order[i] = order[i + 1];
        order[i + 1] = t;
      }
      break;
    case 1: {
      const int i = (int)(rnd() % 19);
      const uint32_t a = order[i], b = order[i + 1];
      ram[0xFAB7 + b] = ram[0xFAB7 + a];
      ram[0xFAB8 + b] = ram[0xFAB8 + a];
      ram[0xFAB5 + b] = (uint8_t)rnd();
      ram[0xFAB6 + b] = (uint8_t)rnd();
      break;
    }
    case 2:
      for(int n = rnd() % 3 == 0 ? 20 : 1 + (int)(rnd() % 20), i = 19; n > 0; n--, i--) {
        const uint32_t x = order[i];
        wr16ram(ram, 0xFAB6 + x, (uint16_t)(lap + (rnd() % 4 == 0 ? 0x30 : rnd() % 0x10)));
        ram[0xFAB8 + x] = 0;
        wr16ram(ram, 0xFAC5 + x, 6);  // the empty handler: the road step would walk its segment table a long way
      }
      break;
    case 3:  // up to 19: with the leader behind the line too it wraps forward (unsigned) a hundred times
      for(int n = rnd() % 3 == 0 ? 19 : 1 + (int)(rnd() % 19), i = 0; n > 0; n--, i++) {
        const uint32_t x = order[i];
        wr16ram(ram, 0xFAB6 + x, (uint16_t)(0xFFF0 - rnd() % 0x40));
        ram[0xFAB8 + x] = 0xFF;
        wr16ram(ram, 0xFAC5 + x, 6);
      }
      break;
    default: {
      const uint16_t last = (uint16_t)(rnd() % 4);
      wr16ram(ram, 0x1F06, last);
      wr16ram(ram, 0x1F08, (uint16_t)(rnd() % 4));
      for(int i = 0; i < 20; i++) {
        const uint32_t x = 0x35 * (uint32_t)i;
        if(rnd() % 3 == 0) ram[0xFADE + x] = (uint8_t)((ram[0xFADE + x] & 0x78) | (rnd() & 0x87));  // the wrap and finish bits
        if(rnd() % 2 == 0) wr16ram(ram, 0xFAD3 + x, last);
      }
      break;
    }
  }
  for(int i = 0; i < 20; i++) wr16ram(ram, 0x1E6A + 2 * (uint32_t)i, order[i]);
  if(scenario != 2 && scenario != 3 && rnd() % 3 == 0) {  // not with the moved cars: a player's handler walks
    for(int n = 1 + (int)(rnd() % 4); n > 0; n--) wr16ram(ram, 0xFAC5 + 0x35 * (rnd() % 20), (uint16_t)(2 + 2 * (rnd() % 7)));
    static const uint16_t records[] = {0x0000, 0x00B3, 0x0062, 0x0115};
    for(unsigned k = 0; k < 4; k++) {
      const uint32_t b = records[k];
      ram[0x0244 + b] = (uint8_t)((ram[0x0244 + b] & 0xFE) | (rnd() & 1));
      wr16ram(ram, 0x022D + b, (uint16_t)(rnd() % 0x400));
      wr16ram(ram, 0x022F + b, 0);
      wr16ram(ram, 0x0210 + b, (uint16_t)(rnd() % 0x80 - 0x40));  // the speed: a far jump would wrap the leader many times
    }
  }
}

// ---- dsp4_answers_store ($81:89F5) ----  a DSP-4 tape row. X = the last pair's address, Y =
// 0. A is the last pair's value, or (no pair in the second run) the chip's status word,
// which is not on the tape: not compared (its callers load A). N Z of that value, or Z set
// (the LDY of a 0 count); C V as on entry
static void call_dsp4_answers_store(const Regs* in, Regs* out) {
  uint16_t x = in->x, a = in->a;
  const bool pairs = dsp4_answers_store(&x, &a);
  out->x = x;
  out->y = 0;
  if(pairs) setNZ16(out, a);
  else {
    out->n = false;
    out->z = true;
  }
}
// two runs of 0-6 pairs each (one count often 0), the addresses anywhere in bank $7E from
// $2000 (below are the stack and the direct page), $FFFF now and then
static void fuzz_dsp4_answers_store(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  (void)ram;
  uint16_t script[32];
  int n = 0;
  for(int run = 0; run < 2; run++) {
    const int pairs = rnd() % 3 == 0 ? 0 : 1 + (int)(rnd() % 6);
    script[n++] = (uint16_t)pairs;
    for(int i = 0; i < pairs; i++) {
      script[n++] = rnd() % 16 == 0 ? 0xFFFF : (uint16_t)(0x2000 + rnd() % 0xE000);
      script[n++] = (uint16_t)rnd();
    }
  }
  dsp4_bridge_script(script, n);
}

// ---- road_rows_stream ($81:8564) ----  a DSP-4 tape row (RTL). A = $00EA, X = the fill's
// end (D+$14) or the scan's ($0194), Y = 0 (dsp4_answers_store's), DB = $83; N set and Z
// clear (the PLB of $83), C of the CMP #$A422 and V of the ADC #$0011 on $00EA
static void call_road_rows_stream(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t e = rd16(0x00EA), sum = (uint16_t)(e + 0x0011);
  const bool split = road_rows_stream();
  out->a = rd16(0x00EA);
  out->x = split ? rd16(DP + 0x14) : rd16(0x0194);
  out->y = 0;
  out->n = true;
  out->z = false;
  out->c = sum >= 0xA422;
  out->v = (~(e ^ 0x0011) & (e ^ sum) & 0x8000) != 0;
  out->db = 0x83;
}
// the race's road, then: 1-16 rows ($0190), each row's bit 0 of $7E:3D6E (one or two
// answer runs), the buffer ($00B4), $0242 / $1EF2 negative now and then (the buffer's ends
// moved in, the first answer's fill), the last answer 0 or not, and (a third of the cases)
// the heights $7E:386C above the widths $7E:37EC up to $0194, for the split fill. The chip is
// scripted in the order the routine reads: two products, the fill word, each row's runs
// (mostly empty; pairs from $6000 up, away from the tables), the last answer, two products
static void fuzz_road_rows_stream(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  const int rows = 1 + (int)(rnd() % 16);
  wr16ram(ram, 0x0190, (uint16_t)(2 + 2 * rows));
  wr16ram(ram, 0x00B4, (rnd() & 1) ? 0 : 1);
  if(rnd() % 3 == 0) wr16ram(ram, 0x0242, (uint16_t)(0 - (1 + rnd() % 0xF0)));
  else if(rnd() & 1) wr16ram(ram, 0x0242, (uint16_t)(rnd() % 0x40));
  if(rnd() % 3 == 0) wr16ram(ram, 0x1EF2, (uint16_t)(0 - (1 + rnd() % 0xF0)));
  else if(rnd() & 1) wr16ram(ram, 0x1EF2, (uint16_t)(rnd() % 0x40));
  const uint32_t first = (uint32_t)((ram[0x0226] << 1) & 0x3E);
  for(int i = 0; i < rows + 1; i++) {
    const uint32_t a = 0x3D6E + first + 2 * (uint32_t)i;
    ram[a] = (uint8_t)((ram[a] & 0xFE) | (rnd() & 1));
  }
  const bool split = rnd() % 3 == 0;
  if(split) {
    const uint16_t end = (uint16_t)(4 + 2 * (rnd() % 40));
    wr16ram(ram, 0x0194, end);
    for(uint32_t k = 2; k < end; k += 2) {
      wr16ram(ram, 0x386C + k, (uint16_t)(0x40 + rnd() % 0x20));
      wr16ram(ram, 0x37EC + k, (uint16_t)(0x10 + rnd() % 0x20));
    }
  }
  uint16_t script[256];
  int n = 0;
  for(int i = 0; i < 5; i++) script[n++] = (uint16_t)rnd();
  for(int i = 0; i < rows; i++) {
    const bool own = (ram[0x3D6E + first + 2 * (uint32_t)i] & 1) != 0;
    for(int call = 0; call < (own ? 2 : 1); call++)
      for(int run = 0; run < 2; run++) {
        const int pairs = (rnd() % 4 == 0 && n < 200) ? 1 + (int)(rnd() % 2) : 0;
        script[n++] = (uint16_t)pairs;
        for(int k = 0; k < pairs; k++) {
          script[n++] = (uint16_t)(0x6000 + 2 * (rnd() % 0x4800));
          script[n++] = (uint16_t)rnd();
        }
      }
  }
  script[n++] = (rnd() & 1) ? 0 : (uint16_t)(1 + rnd() % 0xFFFF);
  for(int i = 0; i < 4; i++) script[n++] = (uint16_t)rnd();
  dsp4_bridge_script(script, n);
}

// ---- road_rows_project ($85:B3D7) ----  a DSP-4 tape row (RTL). A = D+$1A, X = the band
// ($23 or $45), Y = $015C, DB = $83; N set and Z clear (the PLB of $83); C of the CMP #4
// ($0242 negative) or of the rows' CMP #$0E00 (set); V of their ADCs (clear)
// vs_road_project ($85:A471, vs-race-28): a DSP-4 tape row (RTL): the bands merge's registers, then
// PLB ($83: N set, Z clear)
static void call_vs_road_project(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t hi = (uint16_t)(rd16(0x00B6) & 0xFF00);
  out->y = vs_road_project();
  out->a = hi;
  out->x = rd16(DP + 0x16);
  out->n = true;
  out->z = false;
  out->c = true;
}
// the chip's answers scripted for all four passes: random products and rows, objects now and
// then. Each pass's count sum is chosen first (0-104) and reached exactly; only the last count may
// end the pass early (CPX $1C). The tail clears the band bytes at +$23 and +$45 before it reads
// the last one written, +$45 (or +$23) minus the sum: a sum of 0, or 34 with the large band, reads
// 0 and takes the never-run clamp (decoded from the ROM). Player 1's pass sums to 104 or 0 (its
// tail's line then 0: its never-run edge pieces stay out of reach). Player 1's pass (the second) also stays out of its other never-run paths: $1F and $27
// not negative (the products steered), the count sum exactly 104 ($14 ends 0), no early exit.
static uint16_t vsp_hi(uint16_t t) { return (uint16_t)(t >> 2); }  // the answer pair for hi << 2 | lo >> 14 == t
static uint16_t vsp_lo(uint16_t t, uint32_t (*rnd)(void)) { return (uint16_t)((t & 3) << 14 | (rnd() & 0x3FFF)); }
static void fuzz_vs_road_project(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  static const struct { uint16_t rec, tab; } v[4] = {{0x0262, 0x3DEC}, {0x0200, 0x3AEC}, {0x0315, 0x436C}, {0x02B3, 0x406C}};
  const bool small = (rnd() & 1) != 0;
  wr16ram(ram, 0x00B4, small ? 1 : 0);
  const uint16_t stop = small ? 0x0004 : 0x01A4;
  static uint16_t script[4096];
  int n = 0;
  for(int b = 0; b < 4; b++) {
    const uint16_t R = v[b].rec;
    ram[R + 0x25] = (uint8_t)rnd();
    wr16ram(ram, R + 0x37, (uint16_t)(b == 1 ? (int)(rnd() % 0x100) - 0x40 : (int)(rnd() % 0x180) - 0xC0));
    uint16_t at = (uint16_t)(0x0080 - ((ram[R + 0x25] >> 1) & 0x7E));
    uint32_t k = (uint32_t)((ram[R + 0x26] << 1) & 0x3E);
    const int base = n;
    for(int i = 0; i < 10; i++) script[n++] = (uint16_t)rnd();  // five products, low word then high
    if(b == 1) {
      const uint16_t t2 = (uint16_t)((ram[R + 8] | ram[R + 9] << 8) - (rnd() & 0x7FFF));
      script[base + 2] = vsp_lo(t2, rnd);
      script[base + 3] = vsp_hi(t2);
      const uint16_t t4 = (uint16_t)(0x8001 + rnd() % 0x7FFF);
      script[base + 6] = vsp_lo(t4, rnd);
      script[base + 7] = vsp_hi(t4);
    }
    int left = (b == 1) ? ((rnd() % 4) ? 104 : 0) : (rnd() % 8 == 0) ? (small ? 0 : 34) * (int)(rnd() & 1) : (int)(rnd() % 105);
    for(;;) {
      k += 2;
      for(int i = 0; i < 4; i++) script[n++] = (uint16_t)rnd();
      const bool lastRow = (uint16_t)(at + 0x80) >= 0x0E00;
      const int count = lastRow ? left : (left && rnd() % 4 == 0) ? 1 + (int)(rnd() % (uint32_t)(left < 8 ? left : 8)) : 0;
      script[n++] = (uint16_t)count;
      if(count) {
        left -= count;
        wr16ram(ram, v[b].tab + 0x200 + k, (uint16_t)(0x8100 + 2 * (rnd() % 0x3000)));
        uint16_t last = 0;
        for(int i = 0; i < count; i++) {
          last = (uint16_t)(b == 1 || !lastRow ? stop + 2 * (rnd() % 0x100) : 2 * (rnd() % 0x180));
          script[n++] = last;
          script[n++] = (uint16_t)rnd();
          script[n++] = (uint16_t)rnd();
        }
        if((int16_t)(last - stop) < 0) break;  // the CPX $1C / BMI exit
      }
      at = (uint16_t)(at + 0x80);
      if(at >= 0x0E00) break;
    }
  }
  dsp4_bridge_script(script, n);
}
static void call_road_rows_project(const Regs* in, Regs* out) {
  (void)in;
  road_rows_project();
  out->a = rd16(DP + 0x1A);
  out->x = rd16(0x00B4) ? 0x0023 : 0x0045;
  out->y = rd16(0x015C);
  out->n = true;
  out->z = false;
  const uint16_t p = rd16(0x0242);
  out->c = (p & 0x8000) ? (uint16_t)(p + 0x00DC) >= 4 : true;
  out->v = false;
  out->db = 0x83;
}
// the race's road, then the buffer ($00B4), the place in the row ($0225's byte), $0242
// negative now and then, $0202, D+$E4's sign (objects from the chip or copied from bank
// $90), each row's edge flags ($7E:3D6A bits 15 and 1) and edge bytes, $020D. The chip is scripted in the order
// the routine reads: six products, then each row's four answers and object count (mostly
// 0; an object's four answers unless copied, and 1-2 (index, word, word) entries in low
// WRAM); an object's bytes in bank $90 come from ROM
static void fuzz_road_rows_project(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x00B4, (rnd() & 1) ? 0 : 1);
  ram[0x0225] = (uint8_t)rnd();
  if(rnd() % 3 == 0) wr16ram(ram, 0x0242, (uint16_t)(0 - (1 + rnd() % 0xE0)));
  else if(rnd() & 1) wr16ram(ram, 0x0242, (uint16_t)(rnd() % 0x40));
  if(rnd() & 1) wr16ram(ram, 0x0202, (rnd() % 4 == 0) ? 0 : (uint16_t)rnd());
  if(rnd() & 1) ram[0x020D] = (uint8_t)rnd();  // the side's high byte, negative too
  const bool copied = (rnd() & 1) != 0;
  wr16ram(ram, DP + 0xE4, (uint16_t)(copied ? 0x8000 | rnd() : rnd() & 0x7FFF));
  uint16_t at = (uint16_t)(0x0080 - ((ram[0x0225] >> 1) & 0x7E));
  uint32_t k = (uint32_t)(((ram[0x0226] | ram[0x0227] << 8) << 1) & 0x3E);
  uint16_t script[256];
  int n = 0;
  for(int i = 0; i < 12; i++) script[n++] = (uint16_t)rnd();
  int objects = 0;
  do {
    k += 2;
    if(rnd() % 3 == 0) {
      ram[0x3D6B + k] = (uint8_t)((ram[0x3D6B + k] & 0x7F) | (rnd() & 0x80));
      ram[0x3D6A + k] = (uint8_t)((ram[0x3D6A + k] & 0xFD) | (rnd() & 0x02));
      ram[0x3AEB + k] = (uint8_t)rnd();  // the edges' bytes, negative too
      ram[0x3AED + k] = (uint8_t)rnd();
    }
    for(int i = 0; i < 4; i++) script[n++] = (uint16_t)rnd();
    const int pairs = (objects < 8 && rnd() % 5 == 0) ? 1 + (int)(rnd() % 2) : 0;
    script[n++] = (uint16_t)pairs;
    if(pairs) {
      objects++;
      wr16ram(ram, 0x3CEA + k, (uint16_t)(0x8100 + 2 * (rnd() % 0x3000)));
      if(!copied)
        for(int i = 0; i < 4; i++) script[n++] = (uint16_t)rnd();
      for(int i = 0; i < pairs; i++) {
        script[n++] = (uint16_t)(2 * (rnd() % 0x180));
        script[n++] = (uint16_t)rnd();
        script[n++] = (uint16_t)rnd();
      }
    }
    at = (uint16_t)(at + 0x0080);
  } while(at < 0x0E00);
  dsp4_bridge_script(script, n);
}

// ---- road_rows_project2 ($85:B828) ----  a DSP-4 tape row (RTL); outputs as
// road_rows_project's: A = D+$1A, X = the band, Y = $0160, N set, Z clear, C of the CMP #4
// ($1EF2 negative, as it computed it) or of the rows' CMP #$0E00, V clear, DB = $83
static void call_road_rows_project2(const Regs* in, Regs* out) {
  (void)in;
  road_rows_project2();
  out->a = rd16(DP + 0x1A);
  out->x = rd16(0x00B4) ? 0x0023 : 0x0045;
  out->y = rd16(0x0160);
  out->n = true;
  out->z = false;
  const uint16_t q = rd16(0x1EF2);
  out->c = (q & 0x8000) ? (uint16_t)(q + 0x00DC) >= 4 : true;
  out->v = false;
  out->db = 0x83;
}
// as road_rows_project's: the buffer, the place, D+$E4's sign; the first product's answer
// chosen so that $1EF2 comes out negative now and then; each row's two answers and object
// count (objects from bank $7E below $7000 or from ROM in bank $90; entries in low WRAM)
static void fuzz_road_rows_project2(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x00B4, (rnd() & 1) ? 0 : 1);
  ram[0x0225] = (uint8_t)rnd();
  const bool copied = (rnd() & 1) != 0;
  wr16ram(ram, DP + 0xE4, (uint16_t)(copied ? 0x8000 | rnd() : rnd() & 0x7FFF));
  uint16_t at = (uint16_t)(0x0080 - ((ram[0x0225] >> 1) & 0x7E));
  uint32_t k = (uint32_t)(((ram[0x0226] | ram[0x0227] << 8) << 1) & 0x3E);
  const uint16_t base = (uint16_t)((ram[0x0242] | ram[0x0243] << 8) + (ram[0x3E6C + k] | ram[0x3E6D + k] << 8));
  const uint16_t q = rnd() % 3 == 0 ? (uint16_t)(0 - (1 + rnd() % 0xE0)) : (uint16_t)(rnd() % 0x40);
  uint16_t script[256];
  int n = 0;
  script[n++] = (uint16_t)rnd();
  script[n++] = (uint16_t)(q - base);  // $1EF2 = $0242 + this + $3E6C
  for(int i = 0; i < 8; i++) script[n++] = (uint16_t)rnd();
  int objects = 0;
  do {
    k += 2;
    for(int i = 0; i < 2; i++) script[n++] = (uint16_t)rnd();
    const int pairs = (objects < 8 && rnd() % 5 == 0) ? 1 + (int)(rnd() % 2) : 0;
    script[n++] = (uint16_t)pairs;
    if(pairs) {
      objects++;
      wr16ram(ram, 0x3FEC + k, (rnd() & 1) ? (uint16_t)(0x2010 + 2 * (rnd() % 0x2800)) : (uint16_t)(0x8110 + 2 * (rnd() % 0x3000)));
      if(!copied)
        for(int i = 0; i < 4; i++) script[n++] = (uint16_t)rnd();
      for(int i = 0; i < pairs; i++) {
        script[n++] = (uint16_t)(2 * (rnd() % 0x300));
        script[n++] = (uint16_t)rnd();
        script[n++] = (uint16_t)rnd();
      }
    }
    at = (uint16_t)(at + 0x0080);
  } while(at < 0x0E00);
  dsp4_bridge_script(script, n);
}

// ---- race_road_frame ($85:B3A9) / race_road_frame_alt ($85:B36F) ----  DSP-4 tape rows (RTL):
// race_bands_merge's outputs (A = $00B4's high byte, X = D+$16, Y its result; N clear, Z C
// set), V of scroll_tables_build's SBC (its $BB - $0237 less $70; nothing after touches V)
static void roadFrameOutputs(Regs* out, uint16_t merged, uint16_t t) {
  out->a = (uint16_t)(rd16(0x00B4) & 0xFF00);
  out->x = rd16(DP + 0x16);
  out->y = merged;
  out->n = false;
  out->z = out->c = true;
  Regs s;
  setSbc16(&s, t, 0x70);
  out->v = s.v;
}
static void call_race_road_frame(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t t = (uint16_t)(0xBB - rd16(0x0237));
  roadFrameOutputs(out, race_road_frame(), t);
}
static void call_race_road_frame_alt(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t t = (uint16_t)(0xBB - rd16(0x0237));
  roadFrameOutputs(out, race_road_frame_alt(), t);
}
// the race's road as the movie left it, with the buffer ($00B4) either way; the chip
// scripted empty (every answer 0: no objects), the two projections having their own rows
static void fuzz_race_road_frame(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x00B4, (rnd() & 1) ? 0 : 1);
  dsp4_bridge_script(NULL, 0);
}

// ---- screen_objects_place ($8A:898F) ----  three ways out. D+$5E set: A = D+$5E (N Z of it),
// the rest as on entry. Past the edge: objects_scatter_init's A = Y = $FF74, X = $20, Z C
// set (its CPX), N V clear. Else A = the right column's last ADC (N Z C V of it), X = $90,
// Y = 0
static void call_screen_objects_place(const Regs* in, Regs* out) {
  (void)in;
  const uint16_t e = rd16(DP + 0x5E), x0 = rd16(DP + 0x3C);
  const bool edge = !((uint16_t)((uint16_t)((x0 >> 2) - 0x0100) - 0x0120) & 0x8000);
  out->a = screen_objects_place();
  if(e != 0) {
    setNZ16(out, out->a);
    return;
  }
  if(edge) {
    out->x = 0x0020;
    out->y = out->a;
    out->n = out->v = false;
    out->z = out->c = true;
    return;
  }
  out->x = 0x0090;
  out->y = 0;
  uint32_t c = 0, s;
  uint16_t v = rd16(0x072A);
  for(int i = 0; i < 3; i++) {
    s = (uint32_t)v + (i == 0 ? 0x10 : 0x08) + c;
    c = s >> 16;
    v = (uint16_t)s;
  }
  v = rd16(0x062A);
  uint16_t before = 0, add = 0;
  for(int i = 0; i < 4; i++) {
    before = v;
    add = i == 0 ? 0x50 : 0x08;
    s = (uint32_t)v + add + c;
    c = s >> 16;
    v = (uint16_t)s;
  }
  setNZ16(out, v);
  out->c = c != 0;
  out->v = (~(before ^ add) & (before ^ v) & 0x8000) != 0;
}
// D+$5E set now and then; the place D+$3C/D+$3E anywhere (both sides of the edge); the
// random state; the 72 kinds at $7E:8152 (4 now and then)
static void fuzz_screen_objects_place(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x5E, rnd() % 6 == 0 ? (uint16_t)(1 + rnd() % 0x20) : 0);
  wr16ram(ram, DP + 0x3C, (uint16_t)(rnd() % 3 == 0 ? rnd() : 0x0400 + rnd() % 0x0480));
  wr16ram(ram, DP + 0x3E, (uint16_t)rnd());
  wr16ram(ram, 0x0110, (uint16_t)rnd());
  wr16ram(ram, 0x0112, (uint16_t)rnd());
  for(uint32_t i = 0; i < 0x48; i++) wr16ram(ram, 0x8152 + 2 * i, rnd() % 4 == 0 ? 4 : (uint16_t)(rnd() % 0x0400));
}

// ---- scene_step ($8A:82C0) ----  no register outputs: its caller goes on to func_8A8232,
// which loads A (and nothing after reads X or Y first)
static void call_scene_step(const Regs* in, Regs* out) {
  (void)in;
  (void)out;
  scene_step();
}
// any of the ten states (D+$44), the wait D+$48 (mostly 0), the counters near their ends
// (D+$46 1-3 or anything; $8361's index even below $24), the speeds near their tests
// (D+$56 around $63/$64/$AA, D+$34 around 0/$100/$1E0), D+$4C both sides of the row test
static void fuzz_scene_step(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  static const uint16_t states[] = {0x82CA, 0x82EC, 0x8322, 0x833D, 0x8361, 0x8393, 0x83D6, 0x8412, 0x843D, 0x856D};
  const uint16_t state = states[rnd() % 10];
  wr16ram(ram, DP + 0x44, state);
  wr16ram(ram, DP + 0x48, rnd() % 5 == 0 ? (uint16_t)(1 + rnd() % 8) : 0);
  wr16ram(ram, DP + 0x46, state == 0x8361 ? (uint16_t)(2 * (rnd() % 0x12)) : rnd() & 1 ? (uint16_t)(1 + rnd() % 3) : (uint16_t)rnd());
  static const uint16_t speeds[] = {0x62, 0x63, 0x64, 0x65, 0xA9, 0xAA, 0, 1};
  wr16ram(ram, DP + 0x56, rnd() & 1 ? speeds[rnd() % 8] : (uint16_t)(rnd() % 0x100));
  static const uint16_t places[] = {0, 1, 0xFFFF, 0x00FF, 0x0100, 0x01DF, 0x01E0, 0x8000};
  wr16ram(ram, DP + 0x34, rnd() & 1 ? places[rnd() % 8] : (uint16_t)rnd());
  wr16ram(ram, DP + 0x4C, rnd() & 1 ? (uint16_t)(0xFFB0 + rnd() % 0x20) : (uint16_t)rnd());
  wr16ram(ram, DP + 0x78, (uint16_t)(2 * (rnd() % 0x100)));
  static const uint16_t words[] = {0x3C, 0x36, 0x38, 0x4E, 0x5C, 0x60, 0x62, 0x74, 0x76, 0x8A, 0x58, 0x54};
  for(unsigned i = 0; i < sizeof words / sizeof words[0]; i++) if(rnd() & 1) wr16ram(ram, DP + words[i], (uint16_t)rnd());
  if(rnd() & 1) wr16ram(ram, 0x0600, (uint16_t)(0x0068 + rnd() % 8));
  if(rnd() & 1) wr16ram(ram, 0x0160, (uint16_t)rnd());
  if(state == 0x8412 && (rnd() & 1)) {  // its end: the 4-frame step lands with D+$46 at 1
    wr16ram(ram, 0x0160, (uint16_t)(4 * (rnd() % 0x400) + 3));
    wr16ram(ram, DP + 0x46, 1);
  }
  if(state == 0x856D && (rnd() & 1)) wr16ram(ram, DP + 0x46, 0x0033);  // its midpoint: D+$46 down to $32
  if(state == 0x8393 && (rnd() & 1)) {  // its end: the 8-frame step lands with D+$46 at 1
    wr16ram(ram, DP + 0x4E, (uint16_t)(8 * (rnd() % 0x200) + 7));
    wr16ram(ram, DP + 0x46, 1);
  }
}

// ---- split_screen_setup ($80:B7E7) ----  it waits a frame (wait_frames). A = $BC9B (N set,
// Z clear), X = 0 (the clearing loop), Y = $02C2 (the second MVN's end), C set and V of
// wait_frames' add (1 + the frame counter); the return address it stores comes from the
// stack, DB from the entry
static void call_split_screen_setup(const Regs* in, Regs* out) {
  const uint16_t now = rd16(FRAME_COUNTER);
  split_screen_setup(rd16((uint32_t)(uint16_t)(in->sp + 1)), in->db);
  out->a = 0xBC9B;
  out->x = 0;
  out->y = 0x02C2;
  out->n = true;
  out->z = false;
  out->c = true;
  const uint16_t sum = (uint16_t)(now + 1);
  out->v = (~(now ^ 1) & (now ^ sum) & 0x8000) != 0;
}

// ---- title_scene_load ($8A:891C) ----  A = 1 (Z N clear), C clear (the LSRs of 0); X and Y
// are screen_objects_place's (its caller, mode7_view next, does not read them); V is its
// right column's last ADC (D+$3C = 0 always takes that path), or with D+$5E set (no
// placing) decompress_to_vram's clear V
static void call_title_scene_load(const Regs* in, Regs* out) {
  (void)in;
  const bool placed = rd16(DP + 0x5E) == 0;
  title_scene_load();
  out->a = 1;
  out->n = out->z = false;
  out->c = false;
  out->v = false;
  if(placed) {
    uint32_t c = 0, s;
    uint16_t v = rd16(0x072A);
    for(int i = 0; i < 3; i++) {
      s = (uint32_t)v + (i == 0 ? 0x10 : 0x08) + c;
      c = s >> 16;
      v = (uint16_t)s;
    }
    v = rd16(0x062A);
    uint16_t before = 0, add = 0;
    for(int i = 0; i < 4; i++) {
      before = v;
      add = i == 0 ? 0x50 : 0x08;
      s = (uint32_t)v + add + c;
      c = s >> 16;
      v = (uint16_t)s;
    }
    out->v = (~(before ^ add) & (before ^ v) & 0x8000) != 0;
  }
}

// ---- track_preview_maybe ($8A:8768) ----  not shown: A = 0 (Z set, N clear), X = $1536 * 8,
// C = its bit 13 (the third ASL's).
// Shown: track_tiles_preview's A X Y C V (as its adapter), N set and Z clear (the PLB of $8A)
static void call_track_preview_maybe(const Regs* in, Regs* out) {
  const uint16_t track = rd16(0x1536);
  const uint16_t k = bus_rd8(0x838410 + (uint32_t)track);
  const uint16_t handler = bus_rd16(0x8A8BED + (uint32_t)(uint16_t)(rd16(0x0444) << 1));
  const uint16_t off = (handler == 0x8C5B || handler == 0x8C72) ? 0x0200 : 0x0220;
  if(!track_preview_maybe()) {
    out->a = 0;
    out->x = (uint16_t)(track << 3);
    out->n = false;
    out->z = true;
    out->c = (track & 0x2000) != 0;
    return;
  }
  out->a = 0x7001;
  out->x = k;
  out->y = 0x0020;
  out->db = in->db;
  out->n = (in->db & 0x80) != 0;
  out->z = in->db == 0;
  const uint16_t t = (uint16_t)(rom16(0x8384B1 + (uint32_t)k) + 0x20);
  const uint32_t sum = (uint32_t)t + off;
  out->c = sum > 0xFFFF;
  out->v = (~(t ^ off) & (t ^ sum) & 0x8000) != 0;
}
// any track ($1536, over the 55 the preview table holds), any layout ($0444)
static void fuzz_track_preview_maybe(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, 0x1536, (uint16_t)(rnd() % 0x37));
  wr16ram(ram, 0x0444, (uint16_t)(rnd() % 0x37));
}

// ---- hud_markers_write ($82:8961) ----  the sample ends at its tail jump to race_hud_digits
// ($82:89B2, which has its own row): hud_markers_put. X = 4, Y = $0157, C set (the CPX #4), V
// as on entry; A = the last load: $03A7's byte (0, Z set) or the word at $83:A502
static void call_hud_markers_write(const Regs* in, Regs* out) {
  (void)in;
  hud_markers_put();
  out->x = 4;
  out->y = 0x0157;
  out->c = true;
  out->a = rd8(0x03A7) ? bus_rd16(0x83A502) : 0;
  setNZ16(out, out->a);
}
// the badge's byte ($03A7) either way; the markers' bytes ($0398-$039B: the level bytes of
// lists 6-8 and the next, 0 or a part bought, $81) either way: a set one's word is stepped
// over by a dead read of $2139 (harness-9)
static void fuzz_hud_markers_write(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  for(uint32_t i = 0; i < 4; i++) ram[0x0398 + i] = (rnd() & 1) ? 0 : (uint8_t)(rnd() & 1 ? 0x81 : 0x01);
  ram[0x03A7] = (rnd() & 1) ? 0 : (uint8_t)(1 + rnd() % 0xFF);
}
// deliberately wrong: the cleared markers written one after another, ignoring the step over
// a set one (the words land one address early)
static void call_hud_markers_write_wrong(const Regs* in, Regs* out) {
  uint16_t at = 0x7C57;
  for(uint16_t i = 0; i < 4; i++)
    if(rd8(0x0398 + i) == 0) video_write_vram_word(at++, 0x0157);
  if(rd8(0x03A7) != 0)
    for(uint16_t i = 0; i < 4; i++) video_write_vram_word((uint16_t)(0x7F45 + i), bus_rd16(0x83A4FC + 2 * (uint32_t)i));
  (void)in;
  out->x = 4;
  out->y = 0x0157;
  out->c = true;
  out->a = rd8(0x03A7) ? bus_rd16(0x83A502) : 0;
  setNZ16(out, out->a);
}

// ---- player_car_frame ($83:EAC6) ----  its caller loads its own registers: DB = $83 (the
// last PLB) is the output; the bench judges the flags
static void call_player_car_frame(const Regs* in, Regs* out) {
  player_car_frame(in->d);
  out->db = 0x83;
}
// the race as the movie left it, then a few of its inputs moved: the steps $1E9E (1-4, never
// 0: the step loop counts it), the pads' bits $1E9C, $1F31/$1F2C's signs, the side D+$06
// and the road's D+$9E near the $280 limits, the side speed D+$21, the engine's state D+$1A
// (its tables' range), the item D+$53 (0-4), D+$60's byte (the $83:F3B9 table's six), the
// counters
static void fuzz_player_car_frame(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  const uint32_t d = r->d;
  wr16ram(ram, 0x1E9E, (uint16_t)(1 + rnd() % 4));
  if(rnd() & 1) wr16ram(ram, 0x1E9C, (uint16_t)(rnd() & 0x00FF));
  if(rnd() % 4 == 0) wr16ram(ram, 0x1F31, (uint16_t)(rnd() & 1 ? 0x8000 : 0));
  if(rnd() % 4 == 0) wr16ram(ram, 0x1F2C, (uint16_t)(rnd() & 0x8001));
  if(rnd() % 4 == 0) wr16ram(ram, d + 0x06, (uint16_t)(rnd() & 1 ? 0x0270 + rnd() % 0x20 : 0xFD70 + rnd() % 0x20));
  if(rnd() % 4 == 0) wr16ram(ram, d + 0x9E, (uint16_t)(rnd() % 0x200 - 0x100));
  if(rnd() % 4 == 0) wr16ram(ram, d + 0x21, (uint16_t)(rnd() % 0x100 - 0x80));
  if(rnd() % 4 == 0) wr16ram(ram, d + 0x1A, (uint16_t)((rnd() % 6) | (rnd() & 1 ? 0x8000 : 0)));
  if(rnd() % 4 == 0) ram[d + 0x53] = (uint8_t)(rnd() % 5);
  if(rnd() % 4 == 0) ram[d + 0x60] = (uint8_t)(2 * (rnd() % 6));
  if(rnd() % 4 == 0) wr16ram(ram, d + 0x1C, (uint16_t)(rnd() % 0x200 - 0x100));
  // the rarer states, two to five a case (the selftest's 200 cases must reach them all)
  for(int n = 2 + (int)(rnd() % 4); n > 0; n--) switch(rnd() % 16) {
    case 0:  // following (not the follow step), the view's distance near its cap of 9
      wr16ram(ram, 0x1ECE, (uint16_t)(1 + rnd() % 0x10));
      wr16ram(ram, d + 0x14, (uint16_t)(rnd() % 12));
      if(rnd() & 1) {  // and near 0 (the drift takes it below)
        wr16ram(ram, d + 0x14, 0);
        wr16ram(ram, d + 0x12, (uint16_t)(rnd() % 0x100));
      }
      break;
    case 1: ram[d + 0x4E] = 0; wr16ram(ram, d + 0x31, 0); break;
    case 2: wr16ram(ram, d + 0x7E, (uint16_t)(0x0050 + rnd() % 8)); break;
    case 3: {  // the drag on a car or a record (EC33)
      static const uint16_t who[] = {0x0000, 0x0035, 0x006A, 0x009F};
      wr16ram(ram, d + 0x81, who[rnd() % 4]);
      ram[d + 0x50] = (uint8_t)(1 + rnd() % 0x7F);
      wr16ram(ram, 0x1E9C, (uint16_t)(ram[0x1E9C] | 0x40));
      ram[0x03DC] = (uint8_t)rnd();
      if(rnd() & 1) {  // a drag that ends below 0 (from a value already negative)
        const uint32_t c = (uint32_t)(ram[d + 0x81] | ram[d + 0x82] << 8);
        wr16ram(ram, 0xFAC9 + c, (uint16_t)rnd());
        wr16ram(ram, 0xFACB + c, 0xFFFF);
        wr16ram(ram, 0x0216, (uint16_t)rnd());
        wr16ram(ram, 0x0218, 0xFFFF);
        wr16ram(ram, 0x0216 + 0xB3, (uint16_t)rnd());
        wr16ram(ram, 0x0218 + 0xB3, 0xFFFF);
      }
      break;
    }
    case 4: wr16ram(ram, d + 0x18, (uint16_t)(rnd() % 0x40)); wr16ram(ram, d + 0x16, (uint16_t)rnd()); break;  // speeds
    case 5: wr16ram(ram, d + 0x58, (uint16_t)(rnd() & 1 ? rnd() % 3 : 0xFFFF - rnd() % 3)); wr16ram(ram, d + 0x56, (uint16_t)rnd()); break;
    case 6:  // the throttle: its delay, a big pedal (its product's high byte past $7F)
      wr16ram(ram, d + 0x48, (uint16_t)(rnd() & 1 ? 0 : rnd() % 0x300));
      wr16ram(ram, d + 0x44, (uint16_t)rnd());
      if(rnd() & 1) {
        wr16ram(ram, 0x1EA6, (uint16_t)(0x4000 + rnd() % 0x3FFF));
        wr16ram(ram, 0x1EA8, (uint16_t)(0x4000 + rnd() % 0x3FFF));
        wr16ram(ram, 0x1E9C, (uint16_t)(ram[0x1E9C] | 0x01));
      }
      break;
    case 7: wr16ram(ram, 0x1EAA, (uint16_t)rnd()); wr16ram(ram, 0x1EAC, (uint16_t)rnd()); break;
    case 8: {
      const uint32_t x = (uint32_t)(ram[0x1EBC] | ram[0x1EBD] << 8);
      ram[0x3D6E + x] ^= 0x08;
      wr16ram(ram, 0x3BEC + x, rnd() & 1 ? 0 : (uint16_t)rnd());
      break;
    }
    case 9: {  // the side speed near its limits, or near 0 (its easing crosses 0)
      static const uint16_t near[] = {1, 2, 0xFFFF, 0xFFFE};
      wr16ram(ram, d + 0x21, rnd() & 1 ? near[rnd() % 4] : (uint16_t)(rnd() & 1 ? 0x0078 + rnd() % 0x10 : 0xFF78 + rnd() % 0x10));
      if(rnd() & 1) {  // the top speed: D+$10 of $80 and over
        wr16ram(ram, 0x1EA0, (uint16_t)(0x4000 + rnd() % 0x3FFF));
        wr16ram(ram, d + 0x18, (uint16_t)(0x20 + rnd() % 0x20));
      }
      break;
    }
    case 10: ram[d + 0x2A] = (uint8_t)rnd(); wr16ram(ram, d + 0x29, (uint16_t)rnd()); wr16ram(ram, d + 0x46, (uint16_t)(rnd() % 4)); break;
    case 11: {  // the $83:F3C5 limit: D+$9E (the slope, recomputed from the road's row) of either sign
      ram[d + 0x60] = 6;
      ram[d + 0x26] = (uint8_t)(0x18 + rnd() % 0x20);
      wr16ram(ram, 0x1F2C, (uint16_t)(rnd() & 0x8001));
      wr16ram(ram, 0x1F31, 0);
      const uint32_t x = (uint32_t)(ram[0x1EBC] | ram[0x1EBD] << 8);
      ram[0x3D6E + x] &= (uint8_t)~0x08;
      wr16ram(ram, 0x3DEC + x, (uint16_t)(rnd() % 0x200 - 0x100));
      break;
    }
    case 12:
      wr16ram(ram, d + 0x1A, (uint16_t)((rnd() % 6) | (rnd() & 1 ? 0x8000 : 0)));
      wr16ram(ram, d + 0x3A, (uint16_t)rnd());
      wr16ram(ram, d + 0x44, (uint16_t)(ram[d + 0x44] & ~(rnd() & 1 ? 0x31 : 0x21)));
      wr16ram(ram, d + 0x46, (uint16_t)(rnd() % 4));
      break;
    case 13: {  // an item under the car
      ram[d + 0x53] = (uint8_t)(1 + rnd() % 4);
      ram[d + 0xB1] = (uint8_t)rnd();
      ram[d + 0xB2] = (uint8_t)rnd();
      wr16ram(ram, d + 0x06, (uint16_t)((int16_t)(int8_t)ram[d + 0xB1] * 16 + (int)(rnd() % 0x20) - 0x10));
      wr16ram(ram, d + 0x1A, (uint16_t)(rnd() % 3));
      wr16ram(ram, 0x1F31, 0);
      wr16ram(ram, d + 0x4C, (uint16_t)(rnd() % 3));
      if(rnd() & 1) ram[0x0463] = 0;
      break;
    }
    case 14: wr16ram(ram, d + 0x97, (uint16_t)(0xF000 + rnd() % 0x1000)); wr16ram(ram, d + 0x58, (uint16_t)(1 + rnd() % 8)); break;
    case 15:
      if(rnd() % 3 == 0) {  // $1F31 negative: the plain $280 limit, either side
        wr16ram(ram, 0x1F31, 0x8000);
        wr16ram(ram, d + 0x06, (uint16_t)(rnd() & 1 ? 0xFD00 + rnd() % 0x60 : 0x02A0 + rnd() % 0x60));
      } else if(rnd() & 1) wr16ram(ram, 0x1EA0, (uint16_t)(1 + rnd() % 0x200));
      else {  // the view's distance near 0 with D+$44 bit 4 clear (EEA0)
        wr16ram(ram, d + 0x44, (uint16_t)(ram[d + 0x44] & ~0x10));
        wr16ram(ram, d + 0x14, 0);
        wr16ram(ram, d + 0x12, (uint16_t)(rnd() % 0x100));
      }
      break;
    default: break;
  }
  // the rarer scenarios, set last (the draws above can undo them) and taken in turn, so that
  // even the selftest's 200 cases reach each: the four items (D+$4C at its end, no sound
  // queued), the side speed's band ($83:F32F), the gear at its table's end, both sides of
  // the $83:F3C5 limit, the steering at its $80 clamps (at the top speed, where the side
  // step is big), the plain $280 limit of $1F31 negative, and none
  static unsigned turn;
  const unsigned scenario = turn++ % 13;
  const uint32_t row = (uint32_t)(ram[0x1EBC] | ram[0x1EBD] << 8);
  if(scenario < 4) {
    ram[d + 0x53] = (uint8_t)(1 + scenario);
    ram[d + 0xB1] = (uint8_t)rnd();
    ram[d + 0xB2] = (uint8_t)rnd();
    wr16ram(ram, d + 0x06, (uint16_t)((int16_t)(int8_t)ram[d + 0xB1] * 16 + (int)(rnd() % 0x20) - 0x10));
    wr16ram(ram, d + 0x1A, (uint16_t)(rnd() % 3 == 0 ? 0 : 2 + rnd() % 3));
    wr16ram(ram, 0x1F31, 0);
    wr16ram(ram, d + 0x4C, (uint16_t)(rnd() % 2));
    if(rnd() % 4 != 0) ram[0x0463] = 0;
    if(rnd() & 1) wr16ram(ram, d + 0x44, (uint16_t)(ram[d + 0x44] & ~0x20));
  } else if(scenario == 4) {  // (bits $20/$80 of $1E9C would set D+$1A; a view far ahead keeps bit 4 off at EEA0)
    wr16ram(ram, d + 0x44, (uint16_t)(ram[d + 0x44] & ~0x31));
    wr16ram(ram, d + 0x1A, (uint16_t)(2 + rnd() % 4));
    wr16ram(ram, d + 0x46, (uint16_t)(rnd() % 3));
    wr16ram(ram, 0x1E9C, (uint16_t)(ram[0x1E9C] & ~0xA0));
    wr16ram(ram, d + 0x14, (uint16_t)(0x4000 + rnd() % 0x1000));
    wr16ram(ram, d + 0x18, (uint16_t)(rnd() % 0x10));
  } else if(scenario == 5) {  // the gear at its table's end, held (DD0F bit 0)
    for(int tries = 0; tries < 8; tries++) {
      const uint16_t k = (uint16_t)(1 + rnd() % 5);
      if(rom16(0x86DD0F + 2 * (uint32_t)k) & 1) {
        wr16ram(ram, d + 0x1A, (uint16_t)(0x8000 | k));
        wr16ram(ram, d + 0x3A, (uint16_t)(rom16(0x86DD03 + 2 * (uint32_t)k) - rnd() % 4));
        wr16ram(ram, d + 0x44, (uint16_t)(ram[d + 0x44] & ~0x40));
        ram[0x03DC] = (uint8_t)(ram[0x03DC] & ~0x03);
        break;
      }
    }
  } else if(scenario < 8) {  // $83:F3C5 taking the side to half the slope D+$9E: $1F2C negative
                             // with D+$9E positive, or $1F2C clear with D+$9E negative
    const bool right = scenario == 6;
    ram[d + 0x60] = 6;
    ram[d + 0x26] = (uint8_t)(0x18 + rnd() % 0x20);
    wr16ram(ram, 0x1F31, 0);
    wr16ram(ram, 0x1F2C, right ? 0x8000 : 0);
    ram[0x3D6E + row] &= (uint8_t)~0x08;
    const uint16_t slope = (uint16_t)(0x0100 + rnd() % 0xF0);
    const uint16_t w = right ? (uint16_t)(0 - slope) : slope;
    wr16ram(ram, 0x3DEC + row, w);
    wr16ram(ram, 0x3DEE + row, w);  // a flat row: the product is 0, D+$9E = -$3DEC
    wr16ram(ram, d + 0x06, right ? (uint16_t)(rnd() % 0x40) : (uint16_t)(0 - (0x0100 + rnd() % 0x80)));
  } else if(scenario < 10) {  // steering right or left from near its clamp
    const bool right = scenario == 8;
    wr16ram(ram, 0x1EA0, (uint16_t)(0x6000 + rnd() % 0x1FFF));
    wr16ram(ram, d + 0x18, (uint16_t)(0x30 + rnd() % 0x10));
    wr16ram(ram, d + 0x21, right ? (uint16_t)(0x0070 + rnd() % 0x10) : (uint16_t)(0xFF80 + rnd() % 0x10));
    wr16ram(ram, 0x1E9C, (uint16_t)((ram[0x1E9C] & ~0x0C) | (right ? 0x08 : 0x04)));
  } else if(scenario < 12) {  // $1F31 negative: the plain $280 limit, either side
    wr16ram(ram, 0x1F31, 0x8000);
    wr16ram(ram, d + 0x06, scenario == 10 ? (uint16_t)(0xFD00 + rnd() % 0x60) : (uint16_t)(0x02A0 + rnd() % 0x60));
  }
}

// ---- player1_frame ($83:E241) ----  RTL after PLD (D back to $0100: N and Z clear); DB = $83
// (car_track_step's); its caller loads its own registers
static void call_player1_frame(const Regs* in, Regs* out) {
  (void)in;
  player1_frame();
  out->db = 0x83;
  out->n = out->z = false;
}
// the player's car as player_car_frame's fuzz moves it (with D = $0200, its record)
static void fuzz_player1_frame(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  Regs inner = *r;
  inner.d = 0x0200;
  fuzz_player_car_frame(&inner, ram, rnd);
}

// ---- object_sprites_stream ($82:CBD4) ----  a DSP-4 tape row with a native prologue
// (commands $03 and $0B, harness-5). Its caller (the race frame) loads its own registers:
// DB = $83, N set and Z clear (its PLB), C clear (three ASLs of a byte, nothing after)
static void call_object_sprites_stream(const Regs* in, Regs* out) {
  (void)in;
  object_sprites_stream();
  out->db = 0x83;
  out->n = true;
  out->z = false;
  out->c = false;
}
// the race as the movie left it; the player car's flags ($0244 bit 0, $021A, $1F18, $021D,
// the item $014C), $0281's sign, $1EF2's, D+$C8; the chip scripted with many 0 answers (the
// parts' lists end early, and some fill the buffer)
// Whether car `c` drawn the rival's way (rival_car_sprites) finds a ROM part list at every
// turn band and every row drawn: its list is $7F:($7F:band + $84:9180[frame]). Row 0's place
// is the one the routine writes ($7E:39EE - $80), often negative.
static bool rival_parts_in_rom(const uint8_t* ram, uint32_t c) {
  const uint32_t rows = (uint32_t)(ram[0x0190] | ram[0x0191] << 8);
  for(uint32_t r = 0; r < rows && r < 0x80; r += 2) {
    const uint16_t place = r == 0 ? (uint16_t)((ram[0x39EE] | ram[0x39EF] << 8) - 0x0080) : (uint16_t)(ram[0x39EC + r] | ram[0x39ED + r] << 8);
    const uint32_t frame = (uint16_t)((((ram[0xFAB5 + c] | ram[0xFAB6 + c] << 8) >> 1) & 0x007E) + place);
    for(uint32_t band = 0; band <= 12; band += 2) {
      const uint16_t parts = (uint16_t)((ram[0x10000 + band] | ram[0x10001 + band] << 8) + rom16(0x849180 + frame));
      if((uint16_t)(ram[0x10000 + parts] | ram[0x10000 + (uint16_t)(parts + 1)] << 8) < 0x8000) return false;
    }
  }
  return true;
}

// A car of the race order ($1E6A, sorted by row $FAB6) brought onto the rows drawn ($0226 + 1
// on, $1B of them: not row 0, whose place is often negative): the one before a car already
// there, moved between its neighbours (onto that car's row when `same`). Returns its offset,
// or $FFFF when no car is in view.
static uint16_t car_into_view(uint8_t* ram, uint32_t (*rnd)(void), bool same) {
  const uint16_t at = (uint16_t)(ram[0x0226] | ram[0x0227] << 8);
  uint16_t car[20], row[20];
  for(int i = 0; i < 20; i++) {
    car[i] = (uint16_t)(ram[0x1E6A + 2 * i] | ram[0x1E6B + 2 * i] << 8);
    if(car[i] >= 0x800) return 0xFFFF;
    row[i] = (uint16_t)(ram[0xFAB6 + car[i]] | ram[0xFAB7 + car[i]] << 8);
  }
  int cand[19], n = 0;
  for(int i = 0; i + 1 < 20; i++)
    if((uint16_t)(row[i + 1] - at - 1) < 0x1B && row[i] <= row[i + 1]) cand[n++] = i;
  if(n == 0) return 0xFFFF;
  const int i = cand[rnd() % (uint32_t)n];
  uint16_t lo = i > 0 ? row[i - 1] : 0;
  if(lo < at + 1) lo = (uint16_t)(at + 1);
  if(lo > row[i + 1]) lo = row[i + 1];
  const uint16_t to = same ? row[i + 1] : (uint16_t)(lo + rnd() % (uint32_t)(row[i + 1] - lo + 1));
  wr16ram(ram, 0xFAB6 + car[i], to);
  return car[i];
}

// vs_sprites_view0-3 ($82:BE18, $C170, $C489, $C7BA; vs-race-30): DSP-4 tape rows. Exit as
// object_sprites_stream's: DB = $83, N set, Z clear (PLB); C as the C returns it
#define VS_SPRITES_CALL(V) \
  static void call_vs_sprites_view##V(const Regs* in, Regs* out) { \
    (void)in; \
    out->c = vs_sprites_view##V(); \
    out->db = 0x83; \
    out->n = true; \
    out->z = false; \
  }
VS_SPRITES_CALL(0)
VS_SPRITES_CALL(1)
VS_SPRITES_CALL(2)
VS_SPRITES_CALL(3)
// the four cars near the view's row, the race order sorted, the record's own-car fields kept out
// of the never-run path (+$31 = 0); objects of the kinds the VS table has handlers for; the side
// byte negative now and then; the chip's answers sparse or dense (the buffer full, the carry exits)
static void fuzz_vs_sprites(Regs* r, uint8_t* ram, uint32_t (*rnd)(void), unsigned view) {
  (void)r;
  static const uint16_t recs[4] = {0x0200, 0x02B3, 0x0262, 0x0315};
  const uint32_t R = recs[view];
  const uint32_t laps = (uint32_t)(ram[0x019A] | ram[0x019B] << 8);
  ram[R + 0x44] = (uint8_t)((ram[R + 0x44] & ~1) | (rnd() & 1));
  wr16ram(ram, R + 0x31, 0);
  if(rnd() % 4 == 0) ram[R + 9] = (uint8_t)rnd();
  wr16ram(ram, 0x1DF6, 0);
  if(laps != 0 && rnd() % 3 == 0) wr16ram(ram, R + 0x26, (uint16_t)(rnd() % laps));
  const uint32_t at = (uint32_t)(ram[R + 0x26] | ram[R + 0x27] << 8);
  uint32_t pos[4], order[4];
  for(unsigned c = 0; c < 4; c++) {  // the player's car on its row, the rivals on the rows after
    // (the view's first row's place is the second's less $80: a rival there reads past its tables)
    // (the player's own car drawn the rival way, +$44 bit 0, also from the row after)
    const bool mine = c == view && !(ram[R + 0x44] & 1);
    const uint32_t row = laps ? (mine ? at : (at + 1 + rnd() % 0x1D) % laps) : 0;
    pos[c] = row << 8 | (rnd() & 0xFF);
    ram[0xFAB5 + 0x35 * c] = (uint8_t)pos[c];
    wr16ram(ram, 0xFAB6 + 0x35 * c, (uint16_t)row);
    ram[0xFAB8 + 0x35 * c] = 0;
    order[c] = c;
  }
  for(int i = 0; i < 4; i++)
    for(int j = i + 1; j < 4; j++)
      if(pos[order[j]] < pos[order[i]]) {
        const uint32_t t = order[i];
        order[i] = order[j];
        order[j] = t;
      }
  for(unsigned c = 0; c < 4; c++) wr16ram(ram, 0x1E6A + 2 * c, (uint16_t)(0x35 * order[c]));
  wr16ram(ram, 0x1E72, (uint16_t)(0x35 * order[3]));
  // the record's place is its car's (drawn its own way); a car drawn the rival way keeps the
  // record's row, its car on a row after
  if(!(ram[R + 0x44] & 1))
    for(unsigned i = 0; i < 4; i++) ram[R + 0x25 + i] = ram[0xFAB5 + 0x35 * view + i];
  static uint8_t kinds[128];
  int nk = 0;
  for(unsigned kd = 1; kd < 0x68; kd++) {  // the VS table's kinds ($82:8000-$80CD; the one-player's follows)
    const uint16_t h = (uint16_t)rom16(0x827FFE + 2 * kd);
    if(h == 0xDD3B || h == 0xDD69 || h == 0xDCEC || h == 0xDA46) kinds[nk++] = (uint8_t)kd;
  }
  static unsigned turn;
  for(uint32_t o = 0; o < 0x400 && nk > 0; o += 4) {
    if(rnd() % 4 != 0) continue;
    ram[0x8CB3 + o] = (uint8_t)(1 + rnd() % 255);
    ram[0x8CB4 + o] = (uint8_t)(kinds[turn++ % (unsigned)nk] | (rnd() & 0x80));
    ram[0x8CB5 + o] = (uint8_t)rnd();
    ram[0x8CB6 + o] = (uint8_t)rnd();
  }
  if(rnd() % 3 == 0) {  // the view's rows' places, from $80 up (its first row is the second's less $80)
    uint16_t p = (uint16_t)(0x0080 + 2 * (rnd() % 0x40) + 0x80 * (rnd() % 8));
    for(uint32_t q = 0x40 * view + 2; q < 0x40 * view + 0x40 && p < 0x0E00; q += 2, p = (uint16_t)(p + 0x0080))
      wr16ram(ram, 0x39EC + q, p);
  }
  const uint16_t base = (rnd() & 1) ? 0x0220 : 0x0000;
  wr16ram(ram, DP + 0x86, base);
  wr16ram(ram, DP + 0x8E, base);
  static uint16_t script[4096];
  const int n = 200 + (int)(rnd() % 600);
  const bool sparse = (rnd() & 1) != 0;
  int nonzero = 0;
  for(int i = 0; i < n; i++) {
    script[i] = (rnd() % 3 != 0 || (sparse && nonzero >= 40)) ? 0 : (uint16_t)rnd();
    if(script[i] != 0) nonzero++;
  }
  dsp4_bridge_script(script, n);
}
static void fuzz_vs_sprites_view0(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { fuzz_vs_sprites(r, ram, rnd, 0); }
static void fuzz_vs_sprites_view1(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { fuzz_vs_sprites(r, ram, rnd, 1); }
static void fuzz_vs_sprites_view2(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { fuzz_vs_sprites(r, ram, rnd, 2); }
static void fuzz_vs_sprites_view3(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) { fuzz_vs_sprites(r, ram, rnd, 3); }
static void fuzz_object_sprites_stream(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  if(rnd() & 1) wr16ram(ram, 0x021A, (uint16_t)(rnd() & 1 ? 0 : 1));
  if(rnd() & 1) wr16ram(ram, 0x1F18, (uint16_t)(rnd() & 1 ? 0x8000 : rnd() % 0x40));
  if(rnd() & 1) wr16ram(ram, 0x021D, (uint16_t)(rnd() & 1 ? 0x8000 : 0));
  if(rnd() & 1) wr16ram(ram, 0x014C, (uint16_t)(rnd() % 4));
  if(rnd() & 1) wr16ram(ram, 0x0281, (uint16_t)(rnd() & 1 ? 0xFFFF : 0));
  if(rnd() & 1) wr16ram(ram, 0x1EF2, (uint16_t)(rnd() % 0x200 - 0x100));
  if(rnd() & 1) wr16ram(ram, DP + 0xC8, (uint16_t)(rnd() & 1));
  // the rows' objects ($7E:8CB3, 4 bytes each): kinds in turn over every handler but kind 9
  // (an empty entry), at random places; the rows' places ($7E:39EC) as road_rows_project
  // writes them (from below $80 by $80 up to $E00: far rows from $442), from a random start
  static unsigned turn;
  for(uint32_t o = 0; o < 0x400; o += 4) {
    if(rnd() % 4 != 0) continue;
    uint16_t kind;
    do kind = (uint16_t)(turn++ % 128); while(kind == 9);
    ram[0x8CB3 + o] = (uint8_t)rnd();
    ram[0x8CB4 + o] = (uint8_t)(kind | (rnd() & 0x80));
    ram[0x8CB5 + o] = (uint8_t)rnd();
  }
  if(rnd() % 3 == 0) {
    uint16_t at = (uint16_t)(0x0002 + 2 * (rnd() % 0x40) + 0x80 * (rnd() % 12));
    for(uint32_t k = 2; k < 0x40 && at < 0x0E00; k += 2, at = (uint16_t)(at + 0x0080)) wr16ram(ram, 0x39EC + k, at);
  }
  // the rarer cases, taken in turn (the selftest's 200 cases must reach each)
  static unsigned scene;
  const uint32_t first = (uint32_t)(ram[0x1E6A] | ram[0x1E6B] << 8);
  switch(scene++ % 7) {
    case 0:  // the second road (the mini-map's dots, no segment length), a negative side byte
      wr16ram(ram, 0x1E66, (uint16_t)(rnd() & 1 ? 0x8000 : 0));
      wr16ram(ram, 0x1E68, (uint16_t)(rnd() & 1 ? 0x8000 : 0));
      ram[0x0209] = (uint8_t)(0x80 | rnd());
      break;
    case 1: {  // a row on the armed jump ($1EDE), by 1-16 rows ($1EE0; the game loads it only
               // in a case never run, and zeroes it otherwise), landing before the lap's end
               // (the rows then wrap with the lap as usual). The second ASL's carry (bit 14
               // set: an odd table index) is not reached
      const uint16_t at = (uint16_t)(ram[0x0226] | ram[0x0227] << 8);
      const uint16_t arm = (uint16_t)(at + 1 + rnd() % 8);
      wr16ram(ram, 0x1EDE, arm);
      const int32_t room = (int32_t)(ram[0x019A] | ram[0x019B] << 8) - arm - 2;
      if(room >= 1) wr16ram(ram, 0x1EE0, (uint16_t)(1 + rnd() % (uint32_t)(room < 16 ? room : 16)));
      break;
    }
    case 2: {  // a row at the segment's end (its start + $3F): a segment of the race's table
               // (its starts rise, $14 bytes each, inside the lap) whose end and jump (its
               // length, +$10) both land inside the lap, so the rows stay in the objects' table
      const uint16_t laps = (uint16_t)(ram[0x019A] | ram[0x019B] << 8), base = (uint16_t)(ram[0x019C] | ram[0x019D] << 8);
      uint16_t offs[32], any[32];
      int n = 0, nAny = 0;
      int32_t prev = -1;
      bool ended = false;  // the table's end: its first start that does not rise
      for(uint16_t off = 0; off < 0x14 * 32; off = (uint16_t)(off + 0x14)) {
        const int32_t start = rom16(0x840000 + (uint16_t)(base + off));
        // any start will do for the zero jump below (race1's table opens with $7000)
        if(off < 0x14 * 16 && start >= 0x10 && start + 0x3F < laps) any[nAny++] = off;
        if(ended || start <= prev || start >= laps) {
          ended = true;
          continue;
        }
        prev = start;
        const int32_t to = start + 0x3F + (int16_t)rom16(0x840010 + (uint16_t)(base + off));
        if(start >= 0x10 && start + 0x3F < laps && to >= 0 && to < laps) offs[n++] = off;
      }
      if(n > 0) {
        const uint16_t off = offs[rnd() % (uint32_t)n];
        wr16ram(ram, 0x1E5E, off);
        wr16ram(ram, 0x0226, (uint16_t)(rom16(0x840000 + (uint16_t)(base + off)) + 0x3F - 1 - rnd() % 8));
        wr16ram(ram, 0x1E66, 0);
      } else if(nAny > 0) {  // no jump lands in the lap (race1's track): the second road's
                             // flag ($1E66 negative) makes the jump 0, the path still runs
        const uint16_t off = any[rnd() % (uint32_t)nAny];
        wr16ram(ram, 0x1E5E, off);
        wr16ram(ram, 0x0226, (uint16_t)(rom16(0x840000 + (uint16_t)(base + off)) + 0x3F - 1 - rnd() % 8));
        wr16ram(ram, 0x1E66, 0x8000);
      }
      break;
    }
    case 3: {  // a rival in view on the branch road ($FADE bit 5 without bit 1)
      const uint16_t c = car_into_view(ram, rnd, false);
      if(c != 0xFFFF) ram[0xFADE + c] = (uint8_t)((ram[0xFADE + c] & ~0x22) | 0x20);
      break;
    }
    case 4:  // two cars on one row in view: the next in the order drawn on the same row
      (void)car_into_view(ram, rnd, true);
      break;
    case 5: {  // kind 0 objects (object_sprite_kind0: no movie runs them) on rows in view
      const uint32_t laps = (uint32_t)(ram[0x019A] | ram[0x019B] << 8);
      const uint32_t at = (uint32_t)(ram[0x0226] | ram[0x0227] << 8);
      for(uint32_t k = 0; k < 0x1C && laps != 0; k++) {
        if(rnd() % 3 != 0) continue;
        const uint32_t o = 4 * ((at + k) % laps);
        ram[0x8CB3 + o] = (uint8_t)(1 + rnd() % 255);  // not an empty entry
        ram[0x8CB4 + o] = (uint8_t)(rnd() & 0x80);     // kind 0, either flip
        ram[0x8CB5 + o] = (uint8_t)rnd();
        ram[0x8CB6 + o] = (uint8_t)rnd();
      }
      // the player's car drawn the rival's way (no bar), so the script below is dense and the
      // buffer fills inside kind 0's handler too
      if(rival_parts_in_rom(ram, 0)) ram[0x0244] |= 1;
      break;
    }
    default: break;
  }
  (void)first;
  // the player's car drawn the rival's way ($0244 bit 0), when its lists are in the ROM at the
  // rows' places as set above
  if(rnd() % 3 == 0 && ((ram[0x0244] & 1) || rival_parts_in_rom(ram, 0))) ram[0x0244] = (uint8_t)(ram[0x0244] ^ 1);
  // no kind 9 object anywhere a row can reach (the lap's table, $7E:8CB3 + 4 per row, plus a
  // margin for the jumps): its handler entry is $0000 (the original would run WRAM as code),
  // and stale entries there do occur once the scenarios move the rows
  {
    uint32_t span = 4u * (uint32_t)(ram[0x019A] | ram[0x019B] << 8) + 0x400;
    if(span > 0x6000) span = 0x6000;
    for(uint32_t o = 0; o + 1 < span; o += 4) {
      const uint8_t kind = ram[0x8CB4 + o] & 0x7F;
      if(kind == 9 && (ram[0x8CB3 + o] | ram[0x8CB4 + o]) != 0) ram[0x8CB3 + o] = ram[0x8CB4 + o] = 0;
    }
  }
  // the sprite buffer as race_frame_draw leaves it: D+$86 = D+$8E, 0 or $220 (equal at every
  // live entry; a paused race leaves the last frame's end there instead)
  {
    const uint16_t base = (rnd() & 1) ? 0x0220 : 0x0000;
    wr16ram(ram, DP + 0x86, base);
    wr16ram(ram, DP + 0x8E, base);
  }
  // The chip's answers. Every sprite stored follows a non-zero flag word, so a script with
  // fewer than 128 of them cannot fill the $200 bytes. That is how it stays whenever the
  // player's car is drawn its own way ($0244 bit 0 clear): the buffer filling inside its bar
  // (sprite_bar_put's full path) makes the original return with two words left on the stack
  // and crash, which no live frame comes near (at least $168 bytes free when the bar starts,
  // at most $20 used). Otherwise the script is dense, and the buffer does fill.
  static uint16_t script[4096];
  const int n = 200 + (int)(rnd() % 1500);
  const bool sparse = !(ram[0x0244] & 1);
  int nonzero = 0;
  for(int i = 0; i < n; i++) {
    script[i] = (rnd() % 3 != 0 || (sparse && nonzero >= 120)) ? 0 : (uint16_t)rnd();
    if(script[i] != 0) nonzero++;
  }
  dsp4_bridge_script(script, n);
}

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
  {.name = "asl_n", .entry = 0x8AB152, .end = 0x8AB15E, .source = "src/util.c",
   .needM16 = true, .needX16 = true, .call = call_asl_n, ALL_OUTPUTS, .fuzz = fuzz_lsr_n},
  {.name = "password_pad_right", .entry = 0x8AB4D2, .end = 0x8AB4FF, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_password_pad_right, ALL_OUTPUTS, .fuzz = fuzz_password_cursor},
  {.name = "password_pad_left", .entry = 0x8AB4FF, .end = 0x8AB52C, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_password_pad_left, ALL_OUTPUTS, .fuzz = fuzz_password_cursor},
  {.name = "password_pad_up", .entry = 0x8AB52C, .end = 0x8AB559, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_password_pad_up, ALL_OUTPUTS, .fuzz = fuzz_password_cursor},
  {.name = "password_pad_down", .entry = 0x8AB559, .end = 0x8AB586, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_password_pad_down, ALL_OUTPUTS, .fuzz = fuzz_password_cursor},
  {.name = "password_cursor_right", .entry = 0x8AB586, .end = 0x8AB5BF, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_password_cursor_right, ALL_OUTPUTS, .fuzz = fuzz_password_cursor},
  {.name = "password_cursor_left", .entry = 0x8AB5BF, .end = 0x8AB5E2, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_password_cursor_left, ALL_OUTPUTS, .fuzz = fuzz_password_cursor},
  {.name = "password_cursor_up", .entry = 0x8AB5E2, .end = 0x8AB5F2, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_password_cursor_up, ALL_OUTPUTS, .fuzz = fuzz_password_cursor},
  {.name = "password_cursor_down", .entry = 0x8AB5F2, .end = 0x8AB612, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_password_cursor_down, ALL_OUTPUTS, .fuzz = fuzz_password_cursor},
  {.name = "password_type_char", .entry = 0x8AB403, .end = 0x8AB420, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_password_type_char, ALL_OUTPUTS,
   .fuzz = fuzz_password_typing},
  {.name = "password_delete", .entry = 0x8AB420, .end = 0x8AB43D, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_password_delete, ALL_OUTPUTS,
   .fuzz = fuzz_password_typing},
  {.name = "password_text_draw", .entry = 0x8AB43D, .end = 0x8AB47C, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_password_text_draw, ALL_OUTPUTS,
   .fuzz = fuzz_password_typing},
  {.name = "password_no_vowel", .entry = 0x8AB47C, .end = 0x8AB4A4, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_password_no_vowel, ALL_OUTPUTS,
   .fuzz = fuzz_password_typing},
  {.name = "password_press_last_column", .entry = 0x8AB4A4, .end = 0x8AB4D2, .source = "src/password.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_password_press_last_column,
   ALL_OUTPUTS, .fuzz = fuzz_password_typing},
  {.name = "password_press_a", .entry = 0x8AB339, .end = 0x8AB403, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_password_press_a, ALL_OUTPUTS,
   .fuzz = fuzz_password_press_a,
   .holes = {{0x8AB35B, 0x8AB3A7}}},  // another routine (the codes' check, $8A:B35B) sits inside
  {.name = "password_decode", .entry = 0x8ABCEA, .end = 0x8ABD25, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_password_decode, ALL_OUTPUTS, .fuzz = fuzz_password_read},
  {.name = "password_checksum_ok", .entry = 0x8ABD25, .end = 0x8ABD6E, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_password_checksum_ok, ALL_OUTPUTS,
   .fuzz = fuzz_password_read},
  {.name = "password_unpack", .entry = 0x8ABAFF, .end = 0x8ABB5C, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .db = 0x8A, .call = call_password_unpack, ALL_OUTPUTS,
   .fuzz = fuzz_password_unpack},
  // the screen's way in and out (waiting rows). Their HDMA steps ($8A:B18C, $8A:B197) are
  // only called inside them, where one interrupt recorder at a time hides their samples: they
  // have no rows and are compared as part of these
  {.name = "password_screen_open", .entry = 0x8AB1A5, .end = 0x8AB1E2, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .waitsForInterrupts = true, .call = call_password_screen_open,
   .compareA = true},
  {.name = "password_screen_close", .entry = 0x8AB1E2, .end = 0x8AB212, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .waitsForInterrupts = true, .maxInsns = 40000000,
   .call = call_password_screen_close, .compareA = true},
  // (its SEC; RTS at $8A:B3A2, after the handler returns, lies past the sample: not in the extent)
  {.name = "password_entry_open", .entry = 0x8AB232, .end = 0x8AB26C, .endAt = 0x8AB26C, .source = "src/password.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_password_entry_open},
  {.name = "password_entry_sprites", .entry = 0x8AB273, .end = 0x8AB27C, .endAt = 0x8AB27C, .source = "src/password.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_password_entry_sprites},
  {.name = "password_entry_slide", .entry = 0x8AB284, .end = 0x8AB28C, .endAt = 0x8AB26C, .source = "src/password.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_password_entry_slide},
  {.name = "password_entry_ready", .entry = 0x8AB28C, .end = 0x8AB295, .endAt = 0x8AB295, .source = "src/password.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_password_entry_ready},
  {.name = "password_entry_frame", .entry = 0x8AB29C, .end = 0x8AB2A8, .endAt = 0x8AB2A8, .source = "src/password.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_password_entry_frame},
  {.name = "password_entry_pads", .entry = 0x8AB2AA, .end = 0x8AB2BC, .endAt = 0x8AB295, .source = "src/password.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_password_entry_pads},
  {.name = "password_entry_check", .entry = 0x8AB2CE, .end = 0x8AB339, .endAt = 0x8AB2E8, .source = "src/password.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_password_entry_check,
   .holes = {{0x8AB2E8, 0x8AB31B}}},  // (the code search and what follows it: not this part)
  {.name = "password_code_find", .entry = 0x8AB35B, .end = 0x8AB3A2, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_password_code_find, .fuzz = fuzz_password_code_find,
   .endAt = 0x8AB3A4},  // its JMP ($0182) into the handler
  {.name = "password_code_infodata", .entry = 0x8AC013, .end = 0x8AC024, .source = "src/password.c", .needM16 = true,
   .lowWramDb = true, .call = call_password_code_infodata, .fuzz = fuzz_password_code_infodata,
   .holes = {{0x8AC018, 0x8AC020}}},  // (verify-cover-1: its tail; between, another routine)
  {.name = "password_lines_print", .entry = 0x8AB612, .end = 0x8AB644, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_password_lines_print, ALL_OUTPUTS,
   .fuzz = fuzz_password_lines_print},
  {.name = "password_caret_place", .entry = 0x8AB644, .end = 0x8AB659, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_password_caret_place, ALL_OUTPUTS, .fuzz = fuzz_password_caret_place},
  {.name = "password_caret_blink", .entry = 0x8AB2BC, .end = 0x8AB2CE, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_password_caret_blink, ALL_OUTPUTS, .fuzz = fuzz_password_caret_blink},
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
  {.name = "vs_hud_draw", .entry = 0x839B49, .end = 0x839EEA, .source = "src/race_frame.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .db = 0x83, .call = call_vs_hud_draw, .fuzz = fuzz_vs_hud_draw,
   .vramDeadAt = {0x839BD3, 0x839CBB, 0x839DA3, 0x839E8B},   // each player's dead read (harness-30)
   .vramLiveAt = {0x839BD6, 0x839CBE, 0x839DA6, 0x839E8E}},  // and the read whose word is ORed and written back
  {.name = "vs_view_scrolls", .entry = 0x83886A, .end = 0x838909, .source = "src/race_frame.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_vs_view_scrolls, .fuzz = fuzz_vs_view_scrolls},
   // (the first step negative, $83:887C, never ran in the movies: the fuzz runs it)
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
  {.name = "irq_vs_band", .entry = 0x838A26, .end = 0x838A84, .rti = true, .source = "src/race_frame.c",
   .call = call_irq_vs_band, ALL_OUTPUTS},   // no fuzz: it waits (WAI), as irq_race_band
  {.name = "irq_vs_split", .entry = 0x838A84, .end = 0x838B8E, .rti = true, .source = "src/race_frame.c",
   .call = call_irq_vs_split, ALL_OUTPUTS,   // no fuzz: it waits (WAI) twice
   .dmaReadAt = {0x838B75}, .dmaReadsFree = true},  // slot 4's table address: the C has it (harness-41)
  {.name = "irq_vs_band2", .entry = 0x838BE3, .end = 0x838C35, .rti = true, .source = "src/race_frame.c",
   .call = call_irq_vs_band2, ALL_OUTPUTS},
  {.name = "race_cycle_blocks", .entry = 0x859147, .end = 0x8593A7, .source = "src/race_frame.c",
   .needM16 = true, .needX16 = true, .call = call_race_cycle_blocks, .fuzz = fuzz_race_cycle_blocks},
  {.name = "sound_fade_step", .entry = 0x86DFAE, .end = 0x86DFE1, .source = "src/sound.c", .needM16 = true,
   .lowWramDb = true, .call = call_sound_fade_step, ALL_OUTPUTS, .fuzz = fuzz_sound_fade_step},
  // no fuzz: the fuzz runs the CPU alone, so the sound CPU never answers the handshake
  {.name = "sound_send_message", .entry = 0x86DFFB, .end = 0x86E096, .source = "src/sound.c", .lowWramDb = true,
   .call = call_sound_send_message, ALL_OUTPUTS},
  {.name = "sound_boot_upload", .entry = 0x86E096, .end = 0x86E0F0, .source = "src/sound.c", .manyEffects = true,
   .call = call_sound_boot_upload, .holes = {{0x86E0EA, 0x86E0EC}}},  // once per movie, at start-up: live only
   // (the hole: a kick of 0 made 4, which this driver's block sizes never need)
  {.name = "sound_load_set", .entry = 0x86DDEF, .end = 0x86DFAE, .rtl = true, .source = "src/sound.c",
   .waitsForInterrupts = true, .maxInsns = 200000000, .manyEffects = true, .call = call_sound_load_set,
   .alsoIn = {{0x86E0F0, 0x86E3AA}},
   .holes = {{0x86DE6F, 0x86DE96}, {0x86E105, 0x86E106}, {0x86E15F, 0x86E16E}, {0x86E274, 0x86E2B4},
             {0x86E2C9, 0x86E2CA}, {0x86E323, 0x86E332}, {0x86E33E, 0x86E346}}},
       // never run: a list of uncompressed blocks (a trap)
       // its senders (port request 4); never run there: the driver's time-out (a BRK), blocks
       // under three bytes, a sample not a multiple of three
  {.name = "sound_load_menu", .entry = 0x84A4B7, .end = 0x84A4CD, .rtl = true, .source = "src/sound.c",
   .waitsForInterrupts = true, .maxInsns = 200000000, .manyEffects = true, .call = call_sound_load_menu},
  {.name = "options_entry_head", .entry = 0x80FC2D, .end = 0x80FC41, .endAt = 0x80FC41, .source = "src/boot.c",
   .call = call_options_entry_head},
  {.name = "options_entry_tail", .entry = 0x80FC45, .end = 0x80FC5E, .endAt = 0x80FC5E, .source = "src/boot.c",
   .needM16 = true, .needX16 = true, .call = call_options_entry_tail},
  {.name = "top_volumes", .entry = 0x8089EB, .end = 0x8089FF, .endAt = 0x8089FF, .source = "src/boot.c",
   .call = call_top_volumes},
  {.name = "top_after_options", .entry = 0x808A2B, .end = 0x808A82, .endAt = 0x808A82, .endAt2 = 0x808A87,
   .source = "src/boot.c", .needM16 = true, .needX16 = true, .call = call_top_after_options,
   .holes = {{0x808A3F, 0x808A53}, {0x808A61, 0x808A71}}},  // never run: bits 15 and 13 of $03DC (traps)
  {.name = "top_tallies", .entry = 0x808A9B, .end = 0x808AB8, .endAt = 0x808AB8, .source = "src/boot.c",
   .needM16 = true, .needX16 = true, .call = call_top_tallies},
  {.name = "top_race_prep", .entry = 0x808AC3, .end = 0x808AD0, .endAt = 0x808AD0, .source = "src/boot.c",
   .needM16 = true, .needX16 = true, .call = call_top_race_prep},
  {.name = "top_after_race", .entry = 0x808ADF, .end = 0x808AEE, .endAt = 0x808AEE, .source = "src/boot.c",
   .needM16 = true, .needX16 = true, .call = call_top_after_race},
  {.name = "top_after_ticker", .entry = 0x808AF1, .end = 0x808B12, .endAt = 0x808B12, .source = "src/boot.c",
   .needM16 = true, .needX16 = true, .call = call_top_after_ticker,
   .holes = {{0x808B09, 0x808B0F}, {0x808B11, 0x808B12}}},  // never run: the two players' test, the RTS
  {.name = "top_after_title", .entry = 0x808B25, .end = 0x808B55, .endAt = 0x808B55, .endAt2 = 0x808B59,
   .source = "src/boot.c", .needM16 = true, .needX16 = true, .call = call_top_after_title,
   .holes = {{0x808B50, 0x808B55}}},  // never run: the player not in the first five
  {.name = "nmi_menu_frame_tail", .entry = 0x80BC60, .end = 0x80BC98, .endAt = 0x80BC98, .source = "src/frame.c",
   .needM16 = true, .needX16 = true, .call = call_nmi_menu_frame_tail, .holes = {{0x80BC74, 0x80BC77}}},
  {.name = "irq_menu_frame_tail", .entry = 0x80BDCD, .end = 0x80BE2C, .endAt = 0x80BE2C, .source = "src/frame.c",
   .needM16 = true, .needX16 = true, .call = call_irq_menu_frame_tail},
  {.name = "game_boot_head", .entry = 0x808002, .end = 0x80802E, .endAt = 0x80802E, .source = "src/boot.c",
   .emuEntry = true, .call = call_game_boot_head},  // once per movie: live only
  {.name = "game_boot_body", .entry = 0x80803C, .end = 0x80807A, .endAt = 0x80807A, .source = "src/boot.c",
   .needM16 = true, .needX16 = true, .manyEffects = true, .call = call_game_boot_body},
  {.name = "game_boot_tail", .entry = 0x80807A, .end = 0x80809B, .endAt = 0x80809B, .source = "src/boot.c",
   .needX16 = true, .waitsForInterrupts = true, .maxInsns = 40000000, .call = call_game_boot_tail},
  {.name = "sound_boot", .entry = 0x86DD85, .end = 0x86DDEF, .rtl = true, .source = "src/sound.c", .manyEffects = true,
   .call = call_sound_boot},
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
  {.name = "nmi_target_80FC14", .entry = 0x80FC14, .end = 0x80FC1B, .rti = true, .rtiFrame = 9, .source = "src/frame.c",
   .needM16 = true, .needX16 = true, .call = call_nmi_target_80FC14, ALL_OUTPUTS,
   .alsoIn = {{0x8A968E, 0x8A96AD}, {0x808934, 0x80894E}}, .fuzz = fuzz_nmi_target_80FC5F},  // (port request 8)
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
  {.name = "nmi_vs_full", .entry = 0x839777, .end = 0x839B49, .rti = true, .rtiFrame = 9, .source = "src/race_frame.c",
   .needM16 = true, .needX16 = true, .call = call_nmi_vs_full, ALL_OUTPUTS, .alsoIn = {{0x839734, 0x839777}},
   .fuzz = fuzz_nmi_vs_full, .holes = {{0x839AF0, 0x839AF8}},  // its jump table (by the frame count)
   .vramDeadAt = {0x839BD3, 0x839CBB, 0x839DA3, 0x839E8B},   // vs_hud_draw's (harness-30)
   .vramLiveAt = {0x839BD6, 0x839CBE, 0x839DA6, 0x839E8E}},
  {.name = "nmi_race_tail", .entry = 0x85838A, .end = 0x8583CE, .rti = true, .rtiFrame = 9, .source = "src/frame.c",
   .needM16 = true, .needX16 = true, .call = call_nmi_race_tail, ALL_OUTPUTS, .fuzz = fuzz_nmi_race_tail},
  {.name = "nmi_target_80E175", .entry = 0x80E175, .end = 0x80E28C, .rti = true, .rtiFrame = 9, .source = "src/frame.c",
   .needM16 = true, .needX16 = true, .call = call_nmi_target_80E175, ALL_OUTPUTS, .alsoIn = {{0x808934, 0x80894E}},
   .fuzz = fuzz_nmi_target_80E175},
  {.name = "nmi_menu_frame", .entry = 0x80B97B, .end = 0x80BC54, .rti = true, .endAt = 0x80BC54, .source = "src/frame.c",
   .call = call_nmi_menu_frame, .fuzz = fuzz_nmi_menu_frame},  // up to the two tasks' stack switch
  // the results screen, cut at its first wait: the load, then the show (one path no movie runs:
  // $1B37 bit 15 with the fade not at full)
  {.name = "results_screen", .entry = 0x8A94C3, .end = 0x8A95EB, .endAt = 0x8A95EB, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareX = true, .compareY = true,
   .call = call_results_screen},
  {.name = "menu_frame_sprites", .entry = 0x80CA97, .end = 0x80CB4A, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_menu_frame_sprites, ALL_OUTPUTS},
  {.name = "menu_panel_sprites", .entry = 0x80D026, .end = 0x80D168, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_menu_panel_sprites, ALL_OUTPUTS,
   .holes = {{0x80D041, 0x80D044}, {0x80D066, 0x80D069}, {0x80D098, 0x80D099}, {0x80D0A2, 0x80D0AA}}},
  {.name = "menu_button_assign", .entry = 0x80CC7E, .end = 0x80CCE1, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .mayLeaveCaller = true, .compareA = true, .compareY = true,
   .call = call_menu_button_assign, .holes = {{0x80CC93, 0x80CC94}, {0x80CCD5, 0x80CCDA}}},
  {.name = "race_start_close", .entry = 0x80D16C, .end = 0x80D178, .endAt = 0x80D178, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .compareA = true, .call = call_race_start_close},
  {.name = "race_start_bar", .entry = 0x80D17F, .end = 0x80D1A1, .endAt = 0x80D1A1, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .call = call_race_start_bar},
  {.name = "race_start_load", .entry = 0x80D1B4, .end = 0x80D241, .endAt = 0x80D241, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true,
   .call = call_race_start_load},
  {.name = "race_start_setup", .entry = 0x80D269, .end = 0x80D3B9, .endAt = 0x80D3B9, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_race_start_setup},
  {.name = "race_start_setup_end", .entry = 0x80D3BA, .end = 0x80D422, .endAt = 0x80D422, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true,
   .call = call_race_start_setup_end},
  {.name = "champ_begin", .entry = 0x80BE32, .end = 0x80BE3E, .endAt = 0x80BE3E, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .call = call_champ_begin},
  {.name = "champ_begin_2", .entry = 0x80BEC6, .end = 0x80BED2, .endAt = 0x80BED2, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .call = call_champ_begin},
  {.name = "champ_setup_fresh", .entry = 0x80BE44, .end = 0x80BEBB, .endAt = 0x80BEBB, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true,
   .call = call_champ_setup_fresh},
  {.name = "champ_setup", .entry = 0x80BED8, .end = 0x80BF0C, .endAt = 0x80BF0C, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .call = call_champ_setup},
  {.name = "champ_records", .entry = 0x80BF0F, .end = 0x80BF3B, .endAt = 0x80BF3B, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_champ_records},
  {.name = "champ_load", .entry = 0x80BF82, .end = 0x80BFE9, .endAt = 0x80BFE9, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_champ_load},
  {.name = "champ_queue_map", .entry = 0x80BFED, .end = 0x80BFFF, .endAt = 0x80BFFF, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .db = 0x81, .compareA = true, .call = call_champ_queue_map},
  {.name = "champ_panel", .entry = 0x80C003, .end = 0x80C02F, .endAt = 0x80C02F, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .db = 0x81, .compareA = true, .compareX = true,
   .call = call_champ_panel},
  {.name = "champ_fade_in", .entry = 0x80C039, .end = 0x80C043, .endAt = 0x80C043, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .db = 0x81, .compareA = true, .call = call_champ_fade_in},
  {.name = "menu_draw", .entry = 0x8AA2AF, .end = 0x8AA31A, .endAt = 0x8AA31A, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_menu_draw, .holes = {{0x8AA2E1, 0x8AA2F7}},
   .alsoIn = {{0x8AA31F, 0x8AA32B}}},  // (the items after one with bit 7: menu_draw_rest's code)
  {.name = "menu_draw_rest", .entry = 0x8AA31D, .end = 0x8AA32B, .endAt = 0x8AA31A, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_menu_draw_rest, .alsoIn = {{0x8AA2C9, 0x8AA31D}},
   .holes = {{0x8AA2E1, 0x8AA2F7}}},
  {.name = "menu_select", .entry = 0x8AA419, .end = 0x8AA459, .endAt = 0x8AA459, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_menu_select, .fuzz = fuzz_menu_select},
  {.name = "menu_action_rookie", .entry = 0x8AB174, .end = 0x8AB180, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_menu_action_rookie},
  {.name = "menu_action_submenu", .entry = 0x8AA4FB, .end = 0x8AA4FE, .endAt = 0x8AA4FE, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_menu_action_submenu},
  {.name = "menu_action_options", .entry = 0x8AB0D3, .end = 0x8AB0D6, .endAt = 0x8AB0D6, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_menu_action_submenu},
  {.name = "menu_action_difficulty", .entry = 0x8AB08C, .end = 0x8AB0AD, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_menu_action_difficulty},
  {.name = "menu_action_vs", .entry = 0x8AA502, .end = 0x8AA505, .endAt = 0x8AA505, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_menu_action_submenu},
  {.name = "menu_action_music", .entry = 0x8AB0AD, .end = 0x8AB0D0, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_menu_action_music},
  {.name = "menu_action_players", .entry = 0x8AA53C, .end = 0x8AA53F, .endAt = 0x8AA53F, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .compareY = true, .call = call_menu_action_players},
  {.name = "vs_players_mask", .entry = 0x8AA509, .end = 0x8AA53C, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareX = true, .compareY = true,
   .call = call_vs_players_mask, .fuzz = fuzz_vs_players},
  {.name = "vs_players_set", .entry = 0x8AA558, .end = 0x8AA56D, .endAt = 0x8AA56D, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareX = true,
   .compareY = true, .call = call_vs_players_set, .fuzz = fuzz_vs_players},
  {.name = "menu_action_exit", .entry = 0x8AB0D0, .end = 0x8AB0D3, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_menu_action_exit},
  {.name = "menu_options_apply", .entry = 0x8AB11A, .end = 0x8AB152, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareY = true, .call = call_menu_options_apply},
  {.name = "menu_password_back", .entry = 0x8AB225, .end = 0x8AB22A, .endAt = 0x8AB22A, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_menu_password_back},
  {.name = "menu_submenu_open", .entry = 0x8AB659, .end = 0x8AB65B, .endAt = 0x8AB65B, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_menu_submenu_open},
  {.name = "menu_submenu_tick", .entry = 0x8AB65E, .end = 0x8AB668, .endAt = 0x8AB668, .alsoIn = {{0x8AB67F, 0x8AB680}}, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_menu_submenu_tick},
  {.name = "menu_submenu_frame", .entry = 0x8AB66C, .end = 0x8AB671, .endAt = 0x8AB671, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_menu_submenu_frame},
  {.name = "menu_submenu_chosen", .entry = 0x8AB674, .end = 0x8AB67C, .endAt = 0x8AB67C, .endAt2 = 0x8AB65E, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_menu_submenu_chosen},
  {.name = "menu_select_restore", .entry = 0x8AA462, .end = 0x8AA466, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_menu_select_restore},
  {.name = "password_entry_found", .entry = 0x8AB2FD, .end = 0x8AB302, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_password_entry_found},
  {.name = "vs_palette_load", .entry = 0x8AA7CD, .end = 0x8AA7DB, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareX = true, .compareY = true, .call = call_vs_palette_load},
  {.name = "vs_cursor_moving", .entry = 0x8AAD58, .end = 0x8AAD74, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_vs_cursor_moving},
  {.name = "vs_panel_sprite", .entry = 0x8AABCF, .end = 0x8AABEF, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_vs_panel_sprite},
  {.name = "vs_cursor_home", .entry = 0x8AAA58, .end = 0x8AAA71, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_vs_cursor_home},
  {.name = "vs_letter_index", .entry = 0x8AACE4, .end = 0x8AACFF, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareY = true, .call = call_vs_letter_index, .fuzz = fuzz_vs_panel},
  {.name = "vs_panel_screens", .entry = 0x8AA72F, .end = 0x8AA764, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_vs_panel_screens, .fuzz = fuzz_vs_panel},
  {.name = "vs_letter_next", .entry = 0x8AAEDA, .end = 0x8AAF08, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareY = true, .call = call_vs_letter_next, .fuzz = fuzz_vs_panel},
  {.name = "vs_cursor_column", .entry = 0x8AAA0E, .end = 0x8AAA23, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_vs_cursor_column},
  {.name = "vs_caret_blink", .entry = 0x8AAAA2, .end = 0x8AAAC1, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_vs_caret_blink},
  {.name = "vs_name_clear", .entry = 0x8AACB2, .end = 0x8AACD4, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareY = true, .call = call_vs_name_clear},
  {.name = "vs_cursor_step", .entry = 0x8AAA23, .end = 0x8AAA58, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_vs_cursor_step, .fuzz = fuzz_vs_step},
  {.name = "vs_letter_find", .entry = 0x8AAE20, .end = 0x8AAE46, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareY = true, .call = call_vs_letter_find, .fuzz = fuzz_vs_letter},
  {.name = "vs_cursor_target", .entry = 0x8AAD34, .end = 0x8AAD58, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_vs_cursor_target},
  {.name = "vs_all_ready", .entry = 0x8AA795, .end = 0x8AA7CD, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_vs_all_ready, .fuzz = fuzz_vs_ready},
  {.name = "vs_panel_screens_bg2_off", .entry = 0x8AA728, .end = 0x8AA72F, .alsoIn = {{0x8AA734, 0x8AA764}}, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_vs_panel_screens_bg2_off, .fuzz = fuzz_vs_panel},
  {.name = "vs_panel_text_base", .entry = 0x8AAE6E, .end = 0x8AAE9B, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_vs_panel_text_base, .fuzz = fuzz_vs_panel},
  {.name = "vs_text_center", .entry = 0x8AA97F, .end = 0x8AA9A1, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareY = true, .call = call_vs_text_center},
  {.name = "vs_row_next", .entry = 0x8AAF08, .end = 0x8AAF31, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareY = true, .call = call_vs_row_next, .fuzz = fuzz_vs_row},
  {.name = "rng_next_8a_2", .entry = 0x8AAE9B, .end = 0x8AAEB9, .source = "src/rng.c",
   .needM16 = true, .call = call_rng_next, ALL_OUTPUTS, .fuzz = fuzz_rng_next},
  {.name = "vs_panel_reset", .entry = 0x8AA9F2, .end = 0x8AAA0E, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_vs_panel_reset, .fuzz = fuzz_vs_step},
  {.name = "vs_panel_text_clear", .entry = 0x8AAE46, .end = 0x8AAE6E, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareY = true, .call = call_vs_panel_text_clear},
  {.name = "vs_ready_check", .entry = 0x8AA768, .end = 0x8AA795, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_vs_ready_check, .fuzz = fuzz_vs_ready},
  {.name = "vs_name_default", .entry = 0x8AA8E5, .end = 0x8AA922, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareY = true, .call = call_vs_name_default, .fuzz = fuzz_vs_name},
  {.name = "vs_letter_width", .entry = 0x8AACFF, .end = 0x8AAD34, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareY = true, .call = call_vs_letter_width, .fuzz = fuzz_vs_name},
  {.name = "vs_players_in", .entry = 0x8AA7DB, .end = 0x8AA804, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareX = true, .call = call_vs_players_in},
  {.name = "vs_names_load", .entry = 0x8AA922, .end = 0x8AA97F, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareX = true, .call = call_vs_names_load},
  {.name = "vs_name_put", .entry = 0x8AAC73, .end = 0x8AACB2, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareX = true, .compareY = true, .call = call_vs_name_put, .fuzz = fuzz_vs_name_len},
  {.name = "vs_put_char", .entry = 0x8AB730, .end = 0x8AB7AC, .source = "src/vs.c", .needM16 = false,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareX = true, .compareY = true, .call = call_vs_put_char, .fuzz = fuzz_vs_char},
  {.name = "vs_put_sprite_char", .entry = 0x8AB721, .end = 0x8AB730, .alsoIn = {{0x8AB730, 0x8AB7AC}}, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareX = true, .compareY = true, .call = call_vs_put_sprite_char, .fuzz = fuzz_vs_char},
  {.name = "vs_put_string", .entry = 0x8AB072, .end = 0x8AB08C, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareX = true, .compareY = true, .call = call_vs_put_string},
  {.name = "vs_panel_marks", .entry = 0x8AAB6B, .end = 0x8AAB84, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareX = true, .compareY = true, .call = call_vs_panel_marks_0},
  {.name = "vs_panel_marks_1", .entry = 0x8AAB84, .end = 0x8AAB9D, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareX = true, .compareY = true, .call = call_vs_panel_marks_1},
  {.name = "vs_panel_marks_2", .entry = 0x8AAB9D, .end = 0x8AABB6, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareX = true, .compareY = true, .call = call_vs_panel_marks_2},
  {.name = "vs_panel_marks_3", .entry = 0x8AABB6, .end = 0x8AABCF, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareX = true, .compareY = true, .call = call_vs_panel_marks_3},
  {.name = "vs_name_key", .entry = 0x8AABEF, .end = 0x8AAC73, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_vs_name_key, .fuzz = fuzz_vs_entry},
  {.name = "vs_name_entry_frame", .entry = 0x8AAD74, .end = 0x8AAE20, .holes = {{0x8AADCC, 0x8AADD4}}, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_vs_name_entry_frame, .fuzz = fuzz_vs_entry},
  {.name = "vs_car_title", .entry = 0x8AAA71, .end = 0x8AAA82, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareX = true, .compareY = true, .call = call_vs_car_title},
  {.name = "vs_grid_draw", .entry = 0x8AB037, .end = 0x8AB072, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareX = true, .compareY = true, .call = call_vs_grid_draw},
  {.name = "vs_panel_name_draw", .entry = 0x8AAAC1, .end = 0x8AAB6B, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareX = true, .compareY = true, .call = call_vs_panel_name_draw, .fuzz = fuzz_vs_panel},
  {.name = "vs_car_confirm", .entry = 0x8AA852, .end = 0x8AA8E5, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_vs_car_confirm, .fuzz = fuzz_vs_entry,
   .holes = {{0x8AA869, 0x8AA8A1}}},  // (verify-cover-1: its tail; between, vs_ready_draw's own row)
  {.name = "vs_ready_draw", .entry = 0x8AA869, .end = 0x8AA8A1, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_vs_ready_draw},
  {.name = "vs_car_info", .entry = 0x8AA9A7, .end = 0x8AA9F2, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareY = true, .call = call_vs_car_info},
  {.name = "vs_panel_frame", .entry = 0x8AA81C, .end = 0x8AA852, .alsoIn = {{0x8AA869, 0x8AA8A1}}, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_vs_panel_frame},
  {.name = "vs_graphics_load", .entry = 0x8AB9AF, .end = 0x8ABA55, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .compareX = true, .compareY = true, .call = call_vs_graphics_load,
   .manyEffects = true},  // (1,920 VRAM words)
  {.name = "vs_screen_blank", .entry = 0x8AA576, .end = 0x8AA57E, .endAt = 0x8AA57E, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_vs_screen_blank},
  {.name = "vs_screen_setup", .entry = 0x8AA582, .end = 0x8AA67E, .endAt = 0x8AA67E, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_vs_screen_setup,
   .manyEffects = true},
  {.name = "vs_screen_frame", .entry = 0x8AA682, .end = 0x8AA6F6, .endAt = 0x8AA67E, .alsoIn = {{0x8AA67B, 0x8AA67E}}, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_vs_screen_frame, .fuzz = fuzz_vs_screen_frame},
  {.name = "options_state_clear", .entry = 0x8A9E5B, .end = 0x8A9E67, .endAt = 0x8A9E67, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .compareX = true, .call = call_options_state_clear},
  {.name = "options_screen_open", .entry = 0x8A9E82, .end = 0x8A9EC2, .endAt = 0x8A9EC2, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_options_screen_open, .manyEffects = true},
  {.name = "options_screen_setup", .entry = 0x8A9EC5, .end = 0x8A9F3D, .endAt = 0x8A9F3D, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_options_screen_setup},
  {.name = "options_screen_tick", .entry = 0x8A9F47, .end = 0x8A9F5A, .endAt = 0x8A9F3D, .endAt2 = 0x8A9F5A,
   .alsoIn = {{0x8A9F3A, 0x8A9F3D}}, .source = "src/misc.c", .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A,
   .compareA = true, .call = call_options_screen_tick, .fuzz = fuzz_options_screen_tick},
  {.name = "password_show_open", .entry = 0x8ABE2E, .end = 0x8ABE55, .endAt = 0x8ABE55, .source = "src/password.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_password_show_open},
  {.name = "password_show_graphics", .entry = 0x8ABE5E, .end = 0x8ABE7D, .endAt = 0x8ABE7D, .source = "src/password.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_password_show_graphics},
  {.name = "password_show_text", .entry = 0x8ABE86, .end = 0x8ABEC8, .endAt = 0x8ABEC8, .source = "src/password.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .call = call_password_show_text},
  {.name = "password_show_wait", .entry = 0x8ABECC, .end = 0x8ABF08, .endAt = 0x8ABF08, .source = "src/password.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .waitsForInterrupts = true,
   .maxInsns = 40000000, .call = call_password_show_wait},
  {.name = "vs_cars_ahead", .entry = 0x829B62, .end = 0x829BBA, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .compareA = true, .compareY = true, .call = call_vs_cars_ahead, .fuzz = fuzz_vs_cars_ahead},
  {.name = "vs_cars_ring", .entry = 0x81F68B, .end = 0x81F6AE, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .lowWramDb = false, .compareA = true, .call = call_vs_cars_ring},
  {.name = "vs_cars_order", .entry = 0x81F4AF, .end = 0x81F68B, .rtl = true, .source = "src/vs.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .call = call_vs_cars_order, ALL_OUTPUTS,
   .fuzz = fuzz_vs_cars_order},
  {.name = "car_sync_slot_0200", .entry = 0x81FC72, .end = 0x81FCD1, .source = "src/track.c", .needM16 = true,
   .needX16 = true, .db = 0x7E, .call = call_car_sync_slot_0200, .fuzz = fuzz_car_sync_slot},
  {.name = "car_sync_slot_02b3", .entry = 0x81FCD1, .end = 0x81FD30, .source = "src/track.c", .needM16 = true,
   .needX16 = true, .db = 0x7E, .call = call_car_sync_slot_02b3, .fuzz = fuzz_car_sync_slot},
  {.name = "car_sync_slot_0262", .entry = 0x81FD30, .end = 0x81FD8F, .source = "src/track.c", .needM16 = true,
   .needX16 = true, .db = 0x7E, .call = call_car_sync_slot_0262, .fuzz = fuzz_car_sync_slot},
  {.name = "car_sync_slot_0315", .entry = 0x81FD8F, .end = 0x81FDEE, .source = "src/track.c", .needM16 = true,
   .needX16 = true, .db = 0x7E, .call = call_car_sync_slot_0315, .fuzz = fuzz_car_sync_slot},
  {.name = "vs_cars_tie", .entry = 0x81F6AE, .end = 0x81F6F2, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x7E, .call = call_vs_cars_tie, ALL_OUTPUTS, .fuzz = fuzz_vs_cars_pair,
   .alsoIn = {{0x81F70B, 0x81F876}}},       // the shared tail and the collision
  {.name = "vs_cars_apart", .entry = 0x81F6F2, .end = 0x81F876, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x7E, .call = call_vs_cars_apart, ALL_OUTPUTS, .fuzz = fuzz_vs_cars_pair},
       // (verify-cover-1: the shared tail in its extent, right after it, so its paths count)
  {.name = "vs_car_bands", .entry = 0x829806, .end = 0x829AF4, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .lowWramDb = true, .call = call_vs_car_bands, .fuzz = fuzz_vs_car_bands},
  {.name = "vs_track_objects_view0", .entry = 0x82B87A, .end = 0x82B89C, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_track_objects_view0, ALL_OUTPUTS,
   .fuzz = fuzz_vs_track_objects_view},
  {.name = "vs_track_objects_view1", .entry = 0x82B89C, .end = 0x82B8BF, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_track_objects_view1, ALL_OUTPUTS,
   .fuzz = fuzz_vs_track_objects_view},
  {.name = "vs_track_objects_view2", .entry = 0x82B8BF, .end = 0x82B8DF, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_track_objects_view2, ALL_OUTPUTS,
   .fuzz = fuzz_vs_track_objects_view},
  {.name = "vs_track_objects_view3", .entry = 0x82B8DF, .end = 0x82B902, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_track_objects_view3, ALL_OUTPUTS,
   .fuzz = fuzz_vs_track_objects_view},
  {.name = "vs_track_objects_stream", .entry = 0x82B902, .end = 0x82BA12, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_track_objects_stream, ALL_OUTPUTS,
   .fuzz = fuzz_vs_track_objects_stream},
  {.name = "vs_hud_digits", .entry = 0x829C39, .end = 0x82A0C6, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_hud_digits, .fuzz = fuzz_vs_hud_digits},
  {.name = "vs_places_update", .entry = 0x829AF4, .end = 0x829B62, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_vs_places_update, .fuzz = fuzz_vs_places_update},
  {.name = "vs_cars_link", .entry = 0x8297BE, .end = 0x829806, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .compareA = true, .compareX = true, .compareY = true, .call = call_vs_cars_link,
   .fuzz = fuzz_vs_cars_link},
  {.name = "vs_car_parts", .entry = 0x83E878, .end = 0x83E8AB, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .compareA = true, .compareX = true, .call = call_vs_car_parts,
   .fuzz = fuzz_vs_car_parts},
  {.name = "race_wrapper_head", .entry = 0x828342, .end = 0x8283B8, .endAt = 0x8283B8, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .waitsForInterrupts = true, .call = call_race_wrapper_head},
  {.name = "race_wrapper_tail", .entry = 0x8283BB, .end = 0x828415, .endAt = 0x828415, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .lowWramDb = true, .waitsForInterrupts = true,
   .call = call_race_wrapper_tail},
  {.name = "race_load", .entry = 0x828428, .end = 0x82854B, .endAt = 0x82854B, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .lowWramDb = true, .call = call_race_load, .manyEffects = true,
   .holes = {{0x8284A8, 0x8284C8}}},  // the other two cars' graphics ($01E4 0 or positive: never in the movies)
  {.name = "race_track_build", .entry = 0x82856D, .end = 0x8285D6, .endAt = 0x8285D6, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .lowWramDb = true, .call = call_race_track_build,
   .manyEffects = true, .dsp4Tape = true, .replays = 32},
  {.name = "race_track_start", .entry = 0x8285D6, .end = 0x828645, .endAt = 0x828645, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .lowWramDb = true, .waitsForInterrupts = true,
   .maxInsns = 40000000, .call = call_race_track_start, .manyEffects = true,  // no fuzz: it waits
   .dsp4Tape = true,  // the road's first frame streams the road and the sprites
   .holes = {{0x82863F, 0x828642}}},  // the demo's message (ported from the ROM's bytes, untested)
  {.name = "race_frame_head", .entry = 0x828645, .end = 0x828656, .endAt = 0x828656, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .lowWramDb = true, .call = call_race_frame_head,
   .manyEffects = true, .replays = 32},
  {.name = "race_frame_wait", .entry = 0x828656, .end = 0x82865B, .endAt = 0x82865B, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .lowWramDb = true, .waitsForInterrupts = true,
   .maxInsns = 40000000, .call = call_race_frame_wait},  // no fuzz: it waits
  {.name = "race_frame_body", .entry = 0x82865B, .end = 0x82867B, .endAt = 0x82867B, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .lowWramDb = true, .call = call_race_frame_body,
   .manyEffects = true, .dsp4Tape = true, .replays = 32},
  {.name = "race_frame_rest", .entry = 0x82867B, .end = 0x828773, .endAt = 0x828773, .endAt2 = 0x828776,
   .source = "src/misc.c", .needM16 = true, .needX16 = true, .fixedD = true, .lowWramDb = true,
   .call = call_race_frame_rest, .manyEffects = true,
   .holes = {{0x828689, 0x8286AB}, {0x8286B3, 0x8286BE}, {0x8286C6, 0x8286D1}, {0x8286F4, 0x8286FA}}},
       // never run: the demo's end, the Start button's pause, the Select button's way out (a trap),
       // a message (ported, untested)
  {.name = "race_end", .entry = 0x828776, .end = 0x82878A, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .lowWramDb = true, .waitsForInterrupts = true, .call = call_race_end},
  {.name = "race_sound", .entry = 0x82854B, .end = 0x82856D, .endAt = 0x82856D, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .lowWramDb = true, .waitsForInterrupts = true,
   .maxInsns = 200000000, .manyEffects = true, .call = call_race_sound, .alsoIn = {{0x84A487, 0x84A4A3}}},
  {.name = "vs_race_sound", .entry = 0x82932D, .end = 0x829348, .endAt = 0x829348, .source = "src/vs.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .lowWramDb = true, .waitsForInterrupts = true,
   .maxInsns = 200000000, .manyEffects = true, .call = call_vs_race_sound, .alsoIn = {{0x84A4A3, 0x84A4B7}}},
  {.name = "vs_race_load", .entry = 0x8292BA, .end = 0x82932D, .endAt = 0x82932D, .source = "src/vs.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .lowWramDb = true, .call = call_vs_race_load, .manyEffects = true},
  {.name = "vs_names_absent", .entry = 0x8AA6F6, .end = 0x8AA721, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_vs_names_absent, .fuzz = fuzz_vs_names_absent},
  {.name = "password_hdma_off", .entry = 0x8AB18C, .end = 0x8AB197, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .waitsForInterrupts = true, .call = call_password_hdma_off},  // no fuzz: it waits
  {.name = "vs_race_frame_body", .entry = 0x8294B2, .end = 0x8295E8, .endAt = 0x8295E8, .source = "src/vs.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .lowWramDb = true, .call = call_vs_race_frame_body,
   .manyEffects = true, .dsp4Tape = true, .replays = 32,  // the NMI lands in every live sample
   .holes = {{0x8294DE, 0x8294E1}, {0x8294ED, 0x8294F5}, {0x8294FF, 0x829502}, {0x8295AD, 0x8295DB}}},
       // never run (the countdown's clamp and its first two messages, the race's end: ported, untested)
  {.name = "vs_race_frame_tail", .entry = 0x8295E8, .end = 0x82963F, .endAt = 0x82963F, .source = "src/vs.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .lowWramDb = true, .waitsForInterrupts = true,
   .maxInsns = 40000000, .call = call_vs_race_frame_tail, .manyEffects = true,  // no fuzz: it waits
   .holes = {{0x8295F5, 0x829617}, {0x82962E, 0x829639}}},  // never run (the demo's end, the Start test)
  {.name = "vs_race_start", .entry = 0x829456, .end = 0x8294B2, .endAt = 0x8294B2, .source = "src/vs.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .lowWramDb = true, .waitsForInterrupts = true,
   .maxInsns = 40000000, .call = call_vs_race_start, .manyEffects = true,  // no fuzz: it waits
   .holes = {{0x8294A3, 0x8294B2}}},  // never run (ported from the ROM's bytes, untested)
  {.name = "vs_race_track", .entry = 0x829348, .end = 0x82941F, .endAt = 0x82941F, .source = "src/vs.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .lowWramDb = true, .waitsForInterrupts = true,
   .maxInsns = 40000000, .call = call_vs_race_track, .manyEffects = true},  // no fuzz: it waits
  {.name = "vs_road_rows", .entry = 0x82DF91, .end = 0x82DFA2, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_road_rows, .fuzz = fuzz_vs_road_rows_all,
   .fuzzStack = 0x1FF3},  // the game's stack, one JSR above the four
  {.name = "vs_race_sums_clear", .entry = 0x829422, .end = 0x829452, .endAt = 0x829452, .source = "src/vs.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .lowWramDb = true, .call = call_vs_race_sums_clear},
  {.name = "vs_road_rows_view0", .entry = 0x85BF78, .end = 0x85C316, .rtl = true, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_road_rows_view0, .fuzz = fuzz_vs_road_rows_view0,
   .fuzzStack = 0x1FF1,
   .holes = {{0x85C05E, 0x85C067}, {0x85C13A, 0x85C14F}, {0x85C18A, 0x85C1ED}, {0x85C25E, 0x85C261},
             {0x85C26D, 0x85C278}, {0x85C28D, 0x85C2A1}, {0x85C2B6, 0x85C2B9}, {0x85C2C7, 0x85C2CA},
             {0x85C2FD, 0x85C300}}},  // its never-run paths (traps in the C)
  {.name = "vs_road_rows_view1", .entry = 0x85C316, .end = 0x85C6C2, .rtl = true, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_road_rows_view1, .fuzz = fuzz_vs_road_rows_view1,
   .fuzzStack = 0x1FF1, .holes = {{0x85C4D8, 0x85C4ED}, {0x85C60A, 0x85C60D}, {0x85C639, 0x85C64D}}},
  {.name = "vs_road_rows_view2", .entry = 0x85C6C2, .end = 0x85CA18, .rtl = true, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_road_rows_view2, .fuzz = fuzz_vs_road_rows_view2,
   .fuzzStack = 0x1FF1, .holes = {{0x85C864, 0x85C879}}},
  {.name = "vs_road_rows_view3", .entry = 0x85CA18, .end = 0x85CD78, .rtl = true, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_road_rows_view3, .fuzz = fuzz_vs_road_rows_view3,
   .fuzzStack = 0x1FF1, .replays = 32, .holes = {{0x85CBBA, 0x85CBCF}}},
  {.name = "vs_race_reset", .entry = 0x82BA12, .end = 0x82BD06, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .lowWramDb = true, .call = call_vs_race_reset, .fuzz = fuzz_vs_race_reset},
  {.name = "vs_grid_setup", .entry = 0x82BD0E, .end = 0x82BDA9, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_vs_grid_setup, ALL_OUTPUTS, .fuzz = fuzz_vs_grid_setup},
  {.name = "vs_player1_frame_setup", .entry = 0x83E597, .end = 0x83E64D, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_player1_frame_setup, .fuzz = fuzz_vs_player_frame_setup},
  {.name = "vs_player2_frame_setup", .entry = 0x83E64D, .end = 0x83E706, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_player2_frame_setup, .fuzz = fuzz_vs_player_frame_setup},
  {.name = "vs_player3_frame_setup", .entry = 0x83E706, .end = 0x83E7BF, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_player3_frame_setup, .fuzz = fuzz_vs_player_frame_setup},
  {.name = "vs_player4_frame_setup", .entry = 0x83E7BF, .end = 0x83E878, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_player4_frame_setup, .fuzz = fuzz_vs_player_frame_setup},
  {.name = "vs_cars_frame", .entry = 0x83E28C, .end = 0x83E2D7, .rtl = true, .source = "src/vs.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_cars_frame,
   .fuzz = fuzz_vs_cars_frame},
  {.name = "vs_step_share", .entry = 0x82BDDA, .end = 0x82BE11, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_vs_step_share, ALL_OUTPUTS, .fuzz = fuzz_vs_steps_share},
  {.name = "vs_steps_share", .entry = 0x82BDA9, .end = 0x82BDDA, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_vs_steps_share, ALL_OUTPUTS, .fuzz = fuzz_vs_steps_share},
  {.name = "champ_menu_frame", .entry = 0x80C2BB, .end = 0x80C2DD, .endAt = 0x80C2DD, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .call = call_champ_menu_draw},
       // (harness-37: its drawing; the up/down test is champ_menu_pads)
  {.name = "champ_menu_pads", .entry = 0x80C2DD, .end = 0x80C306, .endAt = 0x80C306, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .call = call_champ_menu_pads,
   .holes = {{0x80C2E4, 0x80C2F0}}},  // up pressed (never in the movies)
  {.name = "champ_slide_step", .entry = 0x80C05C, .end = 0x80C091, .endAt = 0x80C091, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .db = 0x81, .compareX = true, .call = call_champ_slide_step},
  {.name = "champ_clear_panels", .entry = 0x80C096, .end = 0x80C13E, .endAt = 0x80C13E, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .db = 0x81, .call = call_champ_clear_panels},
  {.name = "champ_noise", .entry = 0x80C144, .end = 0x80C227, .endAt = 0x80C227, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .db = 0x81, .call = call_champ_noise},
  {.name = "champ_noise_panels", .entry = 0x80C22D, .end = 0x80C25A, .endAt = 0x80C25A, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .db = 0x81, .call = call_champ_noise_panels},
  {.name = "champ_menu_ready", .entry = 0x80C261, .end = 0x80C2B1, .endAt = 0x80C2B1, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .db = 0x81, .call = call_champ_menu_ready},
  {.name = "race_start_end", .entry = 0x80D668, .end = 0x80D6A0, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_race_start_end},
  {.name = "race_start_frame_draw", .entry = 0x80D475, .end = 0x80D65D, .endAt = 0x80D65D, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .waitsForInterrupts = true, .call = call_race_start_frame,
   .holes = {{0x80D4D8, 0x80D520}, {0x80D548, 0x80D54D}, {0x80D572, 0x80D57D}, {0x80D5A7, 0x80D5A9}}},
   // (the big animation, $0700 set: no movie runs it; the far camera, the scroll back, the
   // markers' end)
  {.name = "menu_choice_begin", .entry = 0x80C4F9, .end = 0x80C503, .endAt = 0x80C503, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true,
   .call = call_menu_choice_begin},
  {.name = "menu_choice_back", .entry = 0x80C506, .end = 0x80C566, .endAt = 0x80C566, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_menu_choice_back},
  {.name = "menu_choice_lines", .entry = 0x80C56A, .end = 0x80C57D, .endAt = 0x80C57D, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_menu_choice_lines},
  {.name = "menu_choice_panels", .entry = 0x80C57E, .end = 0x80C5B0, .endAt = 0x80C5B0, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_menu_choice_panels},
  {.name = "menu_choice_redraw", .entry = 0x80C5B9, .end = 0x80C5C7, .endAt = 0x80C5C7, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .call = call_menu_choice_redraw},
  {.name = "menu_choice_done", .entry = 0x80C5CD, .end = 0x80C5D5, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .call = call_menu_choice_done},
  {.name = "menu_gears_open", .entry = 0x80CEE2, .end = 0x80CF32, .endAt = 0x80CF32, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_menu_gears_open},
  {.name = "menu_gears_lines", .entry = 0x80CF36, .end = 0x80CF4B, .endAt = 0x80CF4B, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_menu_gears_lines},
  {.name = "menu_gears_panel", .entry = 0x80CF4C, .end = 0x80CF62, .endAt = 0x80CF62, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_menu_gears_panel},
  {.name = "menu_gears_setup", .entry = 0x80CF63, .end = 0x80CFCE, .endAt = 0x80CFCE, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .call = call_menu_gears_setup},
  {.name = "menu_gears_frame", .entry = 0x80CFDF, .end = 0x80D026, .endAt = 0x80CFD9, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .call = call_menu_gears_frame,
   .holes = {{0x80CFEE, 0x80CFFB}}},  // down pressed (the movie only presses up)
  {.name = "name_entry_open", .entry = 0x80C6D4, .end = 0x80C713, .endAt = 0x80C713, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_name_entry_open},
  {.name = "name_entry_lines", .entry = 0x80C717, .end = 0x80C72A, .endAt = 0x80C72A, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_name_entry_lines},
  {.name = "name_entry_setup", .entry = 0x80C72B, .end = 0x80C7A8, .endAt = 0x80C7A8, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_name_entry_setup},
  {.name = "name_entry_ready", .entry = 0x80C7AE, .end = 0x80C7E2, .endAt = 0x80C7E2, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareY = true,
   .call = call_name_entry_ready},
  {.name = "name_entry_frame", .entry = 0x80C7E8, .end = 0x80C972, .endAt = 0x80C972, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_name_entry_frame, .holes = {{0x80C8BD, 0x80C8BF}, {0x80C8C8, 0x80C8CD}, {0x80C8F7, 0x80C91B}, {0x80C95D, 0x80C970}}},
   // (left past the wide keys; right from the last rows' keys: never pressed in the movie; the
   // height's two clamps, one hole with the +2 step between them, for the four-hole limit)
  {.name = "name_entry_draw", .entry = 0x80C97C, .end = 0x80C9A6, .endAt = 0x80C7E2, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .call = call_name_entry_draw},
  {.name = "menu_controls_open", .entry = 0x80CB4A, .end = 0x80CB89, .endAt = 0x80CB89, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_menu_controls_open},
  {.name = "menu_controls_lines", .entry = 0x80CB8D, .end = 0x80CBAF, .endAt = 0x80CBAF, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_menu_controls_lines},
  {.name = "menu_controls_setup", .entry = 0x80CBB0, .end = 0x80CC34, .endAt = 0x80CC34, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .call = call_menu_controls_setup},
  {.name = "menu_controls_frame", .entry = 0x80CC45, .end = 0x80CC7E, .endAt = 0x80CC3F, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .call = call_menu_controls_frame,
   .holes = {{0x80CC4C, 0x80CC56}},  // up pressed (the movie only goes down)
   .alsoIn = {{0x80CCE0, 0x80CCE1}}},  // EXIT: menu_button_assign's RTS after its PLA, at this depth
  {.name = "name_entry_key", .entry = 0x80C9A6, .end = 0x80CA97, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .waitsForInterrupts = true, .mayLeaveCaller = true, .compareA = true,
   .compareX = true, .call = call_name_entry_key, .holes = {{0x80CA05, 0x80CA0A}, {0x80CA2B, 0x80CA30}}},
  {.name = "menu_controls_sprites", .entry = 0x80CCE1, .end = 0x80CE7F, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_menu_controls_sprites, .holes = {{0x80CDAF, 0x80CDB4}, {0x80CDBD, 0x80CDC0}}},  // D+$7A past its target
  {.name = "menu_bit_word", .entry = 0x80CE7F, .end = 0x80CE91, .source = "src/misc.c", .needM16 = true, .needX16 = true,
   .call = call_menu_bit_word, ALL_OUTPUTS, .fuzz = fuzz_menu_bit_word},
  // the menu bar's line window: the dispatch, its six steps (step 0 is the end alone)
  {.name = "menu_bar_lines", .entry = 0x80C5E0, .end = 0x80C633, .endAt = 0x80C633, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .compareA = true, .compareX = true, .call = call_menu_bar_lines},
  {.name = "menu_bar_open_step", .entry = 0x80C642, .end = 0x80C651, .endAt = 0x80C6B9, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_menu_bar_open_step,
   .alsoIn = {{0x80C6B7, 0x80C6B9}}},  // (the STZ of D+$4E)
  {.name = "menu_bar_close_step", .entry = 0x80C651, .end = 0x80C65D, .endAt = 0x80C6B9, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_menu_bar_close_step, .alsoIn = {{0x80C6B7, 0x80C6B9}}},
  {.name = "menu_bar_open_colours_step", .entry = 0x80C65D, .end = 0x80C68D, .endAt = 0x80C6B9, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_menu_bar_open_colours_step,
   .alsoIn = {{0x80C6B7, 0x80C6B9}}},
  {.name = "menu_bar_open_controls_step", .entry = 0x80C68D, .end = 0x80C6A2, .endAt = 0x80C6B9, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .call = call_menu_bar_open_controls_step,
   .alsoIn = {{0x80C6B7, 0x80C6B9}}},
  {.name = "menu_bar_open_panel_step", .entry = 0x80C6A2, .end = 0x80C6B7, .endAt = 0x80C6B9, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .call = call_menu_bar_open_panel_step,
   .alsoIn = {{0x80C6B7, 0x80C6B9}}},
  {.name = "menu_bar_lines_end", .entry = 0x80C6B9, .end = 0x80C6D4, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .compareA = true, .call = call_menu_bar_lines_end},
  // the shop's main screen in parts (its frame loop and waits are not rows yet)
  {.name = "shop_screen_begin", .entry = 0x808BC5, .end = 0x808BD1, .endAt = 0x808BD1, .source = "src/shop.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .call = call_shop_screen_begin},
  {.name = "shop_screen_graphics", .entry = 0x808BD7, .end = 0x808C1F, .endAt = 0x808C1F, .source = "src/shop.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareX = true, .call = call_shop_screen_graphics},
  {.name = "shop_screen_records", .entry = 0x808C22, .end = 0x808CB6, .endAt = 0x808CB6, .source = "src/shop.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_shop_screen_records},
  {.name = "shop_screen_text_setup", .entry = 0x808CB9, .end = 0x808CE1, .endAt = 0x808CE1, .source = "src/shop.c",
   .needM16 = true, .needX16 = true, .db = 0x81, .compareA = true, .call = call_shop_screen_text_setup},
  {.name = "shop_screen_list_setup", .entry = 0x808D1A, .end = 0x808D9A, .endAt = 0x808D9A, .source = "src/shop.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .call = call_shop_screen_list_setup,
   .holes = {{0x808D40, 0x808D52}}},  // $03DC bits 8-9 clear, or both set
  {.name = "shop_screen_frame", .entry = 0x808DA0, .end = 0x808E04, .endAt = 0x808E04, .source = "src/shop.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .call = call_shop_screen_frame},
  {.name = "shop_screen_after_items", .entry = 0x808E12, .end = 0x808E2A, .endAt = 0x808E2A, .source = "src/shop.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .call = call_shop_screen_after_items},
  {.name = "shop_screen_frame_end", .entry = 0x808E2A, .end = 0x808E87, .endAt = 0x808E87, .source = "src/shop.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .call = call_shop_screen_frame_end,
   .holes = {{0x808E53, 0x808E6C}}},  // the held cursor's repeat
  {.name = "shop_screen_close", .entry = 0x808E91, .end = 0x808EB4, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .waitsForInterrupts = true, .call = call_shop_screen_close},
  // the screens' bank $80 wrappers, around their JSLs (the epilogues end at their RTL, which
  // the PLB'd byte keeps the bench from finding as a return: the RTL itself is in no row)
  {.name = "title_screen_run", .entry = 0x80FBA5, .end = 0x80FBC1, .endAt = 0x80FBC1, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .call = call_title_screen_run_open},
  {.name = "title_screen_run_close", .entry = 0x80FBC5, .end = 0x80FBD0, .endAt = 0x80FBD0, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .call = call_title_screen_run_close},
  {.name = "results_screen_run", .entry = 0x80FBDC, .end = 0x80FBE8, .endAt = 0x80FBE8, .source = "src/misc.c",
   .call = call_results_screen_run_open},
  {.name = "results_screen_run_close", .entry = 0x80FBEC, .end = 0x80FBF7, .endAt = 0x80FBF7, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .call = call_results_screen_run_close},
  // the shop's item screen in parts
  {.name = "shop_item_screen", .entry = 0x808EB4, .end = 0x808F3B, .endAt = 0x808F3B, .source = "src/shop.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .waitsForInterrupts = true,
   .maxInsns = 40000000, .call = call_shop_item_screen},
  {.name = "shop_item_frame_view", .entry = 0x808F4D, .end = 0x808F5D, .endAt = 0x808F5D, .source = "src/shop.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .call = call_shop_item_frame_view},
  {.name = "shop_item_frame_draw", .entry = 0x808F64, .end = 0x808F70, .endAt = 0x808F70, .source = "src/shop.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .call = call_shop_item_frame_draw},
  {.name = "shop_item_frame_tail", .entry = 0x808F94, .end = 0x808FA1, .endAt = 0x808F4D, .source = "src/shop.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .call = call_shop_item_frame_tail,
   .alsoIn = {{0x808F4A, 0x808F4D}}},  // (the BRA back to the panel upload)
  // the intro, cut at its first wait
  {.name = "intro_screen", .entry = 0x80EE56, .end = 0x80F115, .endAt = 0x80F115, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .call = call_intro_screen},
  {.name = "intro_screen_show", .entry = 0x80F115, .end = 0x80F1E3, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .waitsForInterrupts = true, .maxInsns = 40000000,
   .call = call_intro_screen_show, .holes = {{0x80F1BD, 0x80F1CA}}},  // a button's fade (the movies let it run)
  // the ticker screen, cut around split_screen_setup; the show's unrun paths: D+$76 bit 5 set,
  // $03A8 from 10 up (two places, and its still view at $80:E88E), D+$84 below $FE3E
  {.name = "ticker_screen", .entry = 0x80E4E6, .end = 0x80E5F7, .endAt = 0x80E5F7, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_ticker_screen},
  {.name = "ticker_screen_show", .entry = 0x80E5FA, .end = 0x80E88E, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x81, .waitsForInterrupts = true, .maxInsns = 40000000,
   .call = call_ticker_screen_show, .holes = {{0x80E6AE, 0x80E6B1}, {0x80E70D, 0x80E71C}, {0x80E734, 0x80E737}, {0x80E7D5, 0x80E7D8}},
   .alsoIn = {{0x80E88E, 0x80E8B5}}},  // the still view past the RTS (never run; not counted)
  // the title screen, cut at its first two waits (one path no movie runs: D+$5A's uploads)
  {.name = "title_screen", .entry = 0x8A800B, .end = 0x8A8042, .endAt = 0x8A8042, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .compareX = true, .call = call_title_screen},
  {.name = "title_screen_scene", .entry = 0x8A8042, .end = 0x8A80C8, .endAt = 0x8A80C8, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .waitsForInterrupts = true, .maxInsns = 40000000,
   .call = call_title_screen_scene},
  {.name = "title_screen_show", .entry = 0x8A80C8, .end = 0x8A8181, .rtl = true, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .waitsForInterrupts = true, .maxInsns = 40000000,
   .call = call_title_screen_show, .holes = {{0x8A80F8, 0x8A8142}}},
  {.name = "results_screen_show", .entry = 0x8A95EB, .end = 0x8A9675, .rtl = true, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .waitsForInterrupts = true, .maxInsns = 40000000,
   .call = call_results_screen_show, .holes = {{0x8A9655, 0x8A965E}}},
  {.name = "irq_menu_frame", .entry = 0x80BC9B, .end = 0x80BDC1, .rti = true, .endAt = 0x80BDC1, .source = "src/frame.c",
   .call = call_irq_menu_frame},  // up to the two tasks' stack switch
  {.name = "nmi_prologue", .entry = 0x808807, .end = 0x808931, .rti = true, .endAt = 0x808931, .source = "src/frame.c",
   .call = call_nmi_prologue, .fuzz = fuzz_nmi_prologue},
  {.name = "nmi_vs_prologue", .entry = 0x839721, .end = 0x839731, .rti = true, .endAt = 0x839731, .source = "src/frame.c",
   .call = call_nmi_vs_prologue, .fuzz = fuzz_nmi_prologue},
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
  {.name = "split_screen_setup", .entry = 0x80B7E7, .end = 0x80B94F, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .waitsForInterrupts = true, .call = call_split_screen_setup,
   ALL_OUTPUTS,  // no fuzz: a case cannot wait for a frame
   .holes = {{0x80B828, 0x80B830}}},  // $03DC's low nibble set (its $30 test): no movie reaches it
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
  {.name = "sprite_text_menu", .entry = 0x808704, .end = 0x80879B, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_sprite_text_menu, ALL_OUTPUTS, .fuzz = fuzz_sprite_text,
   .m8At = {0x80871F}},
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
  {.name = "shop_part_level_8", .entry = 0x809F30, .end = 0x809F34, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .call = call_shop_part_level_8, ALL_OUTPUTS, .fuzz = fuzz_shop_part_level},
  {.name = "shop_part_level_9", .entry = 0x809F2C, .end = 0x809F30, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .call = call_shop_part_level_9, ALL_OUTPUTS, .fuzz = fuzz_shop_part_level},
  {.name = "shop_part_level_a", .entry = 0x809F28, .end = 0x809F2C, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .call = call_shop_part_level_a, ALL_OUTPUTS, .fuzz = fuzz_shop_part_level},
  {.name = "shop_part_level_b", .entry = 0x809F38, .end = 0x809F3C, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .call = call_shop_part_level_b, ALL_OUTPUTS, .fuzz = fuzz_shop_part_level},
  {.name = "shop_part_level_c", .entry = 0x809F34, .end = 0x809F38, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .call = call_shop_part_level_c, ALL_OUTPUTS, .fuzz = fuzz_shop_part_level},
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
  {.name = "shop_buy_list1", .entry = 0x809053, .end = 0x8090FD, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .waitsForInterrupts = true, .call = call_shop_buy_list1,
   .endAt = 0x8090FD,  // its tail jump to shop_item_icon_settle
   // lists 1-3: the money short, after a refund or not: no movie runs it (holes; as list 4's)
   .holes = {{0x809088, 0x809090}, {0x809095, 0x80909A}}},
  {.name = "shop_buy_list2", .entry = 0x809100, .end = 0x8091AE, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .waitsForInterrupts = true, .maxInsns = 40000000, .call = call_shop_buy_list2,
   .holes = {{0x809135, 0x80913D}, {0x809142, 0x809147}}},  // as list 1's
  {.name = "shop_buy_list3", .entry = 0x8091AE, .end = 0x80925C, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .waitsForInterrupts = true, .maxInsns = 40000000, .call = call_shop_buy_list3,
   .holes = {{0x8091E3, 0x8091EB}, {0x8091F0, 0x8091F5}}},  // as list 1's
  // lists 6-8: the password movie buys a first part from each (and owns one in lists 6 and 8).
  // Never run on a movie, and a waiting row has no fuzz: a part bought here sold back
  // (first hole), a level held from elsewhere, the money short, and the panel's rise (the
  // second); list 7's part owned (the third). The rise is shop_buy_part's, run on lists 1-5
  // the dispatch, up to its RTS into the handler (shop-21)
  {.name = "shop_buy_dispatch", .entry = 0x809039, .end = 0x809051, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_buy_dispatch, ALL_OUTPUTS, .fuzz = fuzz_shop_buy_dispatch,
   .endAt = 0x809051},  // its RTS into the handler: the row stops there, so it encloses none
  // list 9: as lists 6-8 (its owned path a hole too, as list 7's)
  {.name = "shop_buy_list9", .entry = 0x80966E, .end = 0x809750, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .waitsForInterrupts = true, .call = call_shop_buy_list9,
   .holes = {{0x809699, 0x8096B2}, {0x8096C6, 0x809702}, {0x80967B, 0x80968E}}},
  {.name = "shop_buy_list8", .entry = 0x80931A, .end = 0x8093FC, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .waitsForInterrupts = true, .call = call_shop_buy_list8,
   .holes = {{0x809345, 0x80935E}, {0x809372, 0x8093AE}}},
  {.name = "shop_buy_list7", .entry = 0x8093FC, .end = 0x8094DE, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .waitsForInterrupts = true, .call = call_shop_buy_list7,
   .holes = {{0x809427, 0x809440}, {0x809454, 0x809490}, {0x809409, 0x80941C}}},
  {.name = "shop_buy_list6", .entry = 0x8094DE, .end = 0x8095C0, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .waitsForInterrupts = true, .call = call_shop_buy_list6,
   .holes = {{0x809509, 0x809522}, {0x809536, 0x809572}}},
  {.name = "shop_buy_list4", .entry = 0x80925C, .end = 0x80930A, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .waitsForInterrupts = true, .maxInsns = 40000000, .call = call_shop_buy_list4,
   .holes = {{0x80929E, 0x8092A3}}},  // the money short with no refund (JSL shop_flag_fb, RTS): no
                                      // movie runs it and a waiting row has no fuzz: unchecked
  {.name = "shop_buy_colour", .entry = 0x80930A, .end = 0x80931A, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_buy_colour, .fuzz = fuzz_shop_money},
  {.name = "shop_money_after_refund", .entry = 0x80990D, .end = 0x80993E, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_shop_money_after_refund, ALL_OUTPUTS,
   .alsoIn = {{0x80997E, 0x8099D1}}, .fuzz = fuzz_shop_money_after_refund},
  {.name = "shop_money_after_price", .entry = 0x809957, .end = 0x8099D1, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_shop_money_after_price, ALL_OUTPUTS,
   .fuzz = fuzz_shop_money_after_price},
  {.name = "shop_money_roll", .entry = 0x8099D1, .end = 0x809ABB, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_shop_money_roll, ALL_OUTPUTS,
   .fuzz = fuzz_shop_money},
  {.name = "shop_money_store", .entry = 0x809ABB, .end = 0x809AC8, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_shop_money_store, ALL_OUTPUTS,
   .fuzz = fuzz_shop_money},
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
  {.name = "shop_item_icon_0", .entry = 0x80A096, .end = 0x80A09F, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_icon_0, .alsoIn = {{0x80A1E4, 0x80A251}}, .fuzz = fuzz_shop_item_icon_b},
  {.name = "shop_item_icon_8", .entry = 0x80A09F, .end = 0x80A0E0, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_icon_8, .alsoIn = {{0x80A1E4, 0x80A251}}, .fuzz = fuzz_shop_item_icon_b},
  {.name = "shop_item_icon_7", .entry = 0x80A0E0, .end = 0x80A121, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_icon_7, .alsoIn = {{0x80A1E4, 0x80A251}}, .fuzz = fuzz_shop_item_icon_b},
  {.name = "shop_item_icon_6", .entry = 0x80A121, .end = 0x80A162, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_icon_6, .alsoIn = {{0x80A1E4, 0x80A251}}, .fuzz = fuzz_shop_item_icon_b},
  {.name = "shop_item_icon_9", .entry = 0x80A162, .end = 0x80A1A3, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_icon_9, .alsoIn = {{0x80A1E4, 0x80A251}}, .fuzz = fuzz_shop_item_icon_b},
  {.name = "shop_item_icon_10", .entry = 0x80A1A3, .end = 0x80A1E4, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_icon_10, .alsoIn = {{0x80A1E4, 0x80A251}}, .fuzz = fuzz_shop_item_icon_b},
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
  {.name = "fade_out_on_confirm_once", .entry = 0x8A8181, .end = 0x8A81BA, .source = "src/misc.c", .needM16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_fade_out_on_confirm_once, ALL_OUTPUTS, .fuzz = fuzz_fade_out_on_confirm,
   .holes = {{0x8A8196, 0x8A81B0}}},  // (verify-cover-1: its tail; between, not its code)
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
  {.name = "scene_step", .entry = 0x8A82C0, .end = 0x8A85AF, .source = "src/misc.c", .needM16 = true, .needX16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_scene_step, .fuzz = fuzz_scene_step,
   .holes = {{0x8A833C, 0x8A833D},    // a lone RTS no state points at
             {0x8A84B9, 0x8A8524}}},  // objects_scatter_init and its rng_next copy (screen_objects_place's)
  {.name = "title_scene_load", .entry = 0x8A891C, .end = 0x8A898F, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_title_scene_load, .compareA = true,
   .fuzz = fuzz_nothing},
  {.name = "screen_objects_place", .entry = 0x8A898F, .end = 0x8A8AE7, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_screen_objects_place, ALL_OUTPUTS,
   .fuzz = fuzz_screen_objects_place, .holes = {{0x8A89E1, 0x8A8A43}}},  // other code (unknown bytes), not reached from here
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
  {.name = "shop_item_panel_1", .entry = 0x809C47, .end = 0x809C85, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .alsoIn = {{0x80B537, 0x80B556}}, .call = call_shop_item_panel_1, ALL_OUTPUTS, .fuzz = fuzz_shop_item_panel},
  {.name = "shop_item_panel_2", .entry = 0x809C85, .end = 0x809CC3, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .alsoIn = {{0x80B537, 0x80B556}}, .call = call_shop_item_panel_2, ALL_OUTPUTS, .fuzz = fuzz_shop_item_panel},
  {.name = "shop_item_panel_3", .entry = 0x809CC3, .end = 0x809D02, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .alsoIn = {{0x80B537, 0x80B556}}, .call = call_shop_item_panel_3, ALL_OUTPUTS, .fuzz = fuzz_shop_item_panel},
  {.name = "shop_item_panel_5", .entry = 0x809D02, .end = 0x809D41, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .alsoIn = {{0x80B537, 0x80B556}}, .call = call_shop_item_panel_5, ALL_OUTPUTS, .fuzz = fuzz_shop_item_panel},
  {.name = "shop_item_panel_4", .entry = 0x809D41, .end = 0x809D80, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .alsoIn = {{0x80B537, 0x80B556}}, .call = call_shop_item_panel_4, ALL_OUTPUTS, .fuzz = fuzz_shop_item_panel},
  {.name = "shop_item_panel_0", .entry = 0x809D80, .end = 0x809D9E, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .alsoIn = {{0x80B537, 0x80B556}}, .call = call_shop_item_panel_0, ALL_OUTPUTS, .fuzz = fuzz_shop_item_panel},
  {.name = "shop_item_panel_6", .entry = 0x809D9E, .end = 0x809DBC, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .alsoIn = {{0x80B537, 0x80B556}}, .call = call_shop_item_panel_6, ALL_OUTPUTS, .fuzz = fuzz_shop_item_panel},
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
   .fixedD = true, .db = 0x81, .call = call_shop_item_view_h, .fuzz = fuzz_shop_item_view_h},
  {.name = "shop_item_view_i", .entry = 0x80AE4E, .end = 0x80AEBE, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_view_i, .fuzz = fuzz_shop_item_view_i},
  {.name = "shop_item_view_j", .entry = 0x80AC37, .end = 0x80ACB6, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x81, .call = call_shop_item_view_j, .fuzz = fuzz_shop_item_view_j},
  {.name = "shop_level_blocks", .entry = 0x80AF36, .end = 0x80AFE1, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .call = call_shop_level_blocks, .fuzz = fuzz_shop_level_blocks},
  {.name = "shop_gauge_lines", .entry = 0x80ACB6, .end = 0x80ADD9, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .call = call_shop_gauge_lines, .fuzz = fuzz_shop_gauge_lines},
  {.name = "shop_gauge_line", .entry = 0x80ADD9, .end = 0x80AE4E, .source = "src/shop.c", .needM16 = true, .needX16 = true,
   .fixedD = true, .db = 0x7E, .call = call_shop_gauge_line},
  // the control codes' handlers follow the code table ($80:B6BC-B6C9, data)
  {.name = "text_type_step", .entry = 0x80B59D, .end = 0x80B7D9, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_text_type_step, .fuzz = fuzz_text_type_step,
   .m8At = {0x80B645, 0x80B65E},
   .holes = {{0x80B6BC, 0x80B6CA}, {0x80B7BB, 0x80B7CE}}},  // (verify-cover-1: its tail jumps' targets
       // in its extent, so their paths count; between them another routine and the gap before it)
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
  // the race paused: a waiting row (it encloses wait_frames_82's samples while it waits). One
  // path no movie runs: quitting (L and R held: it leaves its caller)
  {.name = "race_pause", .entry = 0x82884B, .end = 0x828911, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .waitsForInterrupts = true, .maxInsns = 40000000, .call = call_race_pause,
   .compareA = true, .holes = {{0x8288D8, 0x8288F2}}},  // the quit; $03E2 bit 2 runs on code_infodata.txt
  {.name = "shop_panel_frame", .entry = 0x80B149, .end = 0x80B2D2, .source = "src/shop.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_shop_panel_frame},
  {.name = "text_centred_line_at", .entry = 0x828911, .end = 0x828923, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_text_centred_line_at, .fuzz = fuzz_text_centred_line_at},
  {.name = "money_skip_bonus", .entry = 0x8AC355, .end = 0x8AC379, .source = "src/misc.c", .needM16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_money_skip_bonus, .compareA = true, .fuzz = fuzz_money_skip_bonus},
  {.name = "text_centred_line", .entry = 0x82B18B, .end = 0x82B234, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x7E, .call = call_text_centred_line, .fuzz = fuzz_text_centred_line},
  {.name = "vs_centred_line", .entry = 0x82A134, .end = 0x82A1D3, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x7E, .call = call_vs_centred_line, .fuzz = fuzz_text_centred_line},
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
  {.name = "race_message_update", .entry = 0x82B11F, .end = 0x82B18B, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_race_message_update, .fuzz = fuzz_race_message_update},
  {.name = "vs_message_update", .entry = 0x82A0C6, .end = 0x82A134, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_vs_message_update, .fuzz = fuzz_race_message_update},
  {.name = "func_82B769", .entry = 0x82B769, .end = 0x82B78D, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_func_82B769, ALL_OUTPUTS, .fuzz = fuzz_func_82B769},
  {.name = "empty_80B94F", .entry = 0x80B94F, .end = 0x80B950, .source = "src/misc.c", .call = call_empty, ALL_OUTPUTS},
  {.name = "sprite_number_point", .entry = 0x80855A, .end = 0x8085CA, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_sprite_number_point, ALL_OUTPUTS,
   .fuzz = fuzz_sprite_number_point},
  {.name = "sprite_bar8", .entry = 0x80C475, .end = 0x80C4F9, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_sprite_bar8, ALL_OUTPUTS, .fuzz = fuzz_sprite_bar8},
  {.name = "clc_82DA46", .entry = 0x82DA46, .end = 0x82DA48, .source = "src/misc.c", .call = call_clc, ALL_OUTPUTS},
  {.name = "clc_83FA32", .entry = 0x83FA32, .end = 0x83FA34, .source = "src/misc.c", .call = call_clc, ALL_OUTPUTS},
  {.name = "mode7_zoom_step", .entry = 0x80F226, .end = 0x80F247, .source = "src/scale.c", .needM16 = true,
   .needX16 = true, .call = call_mode7_zoom_step, ALL_OUTPUTS, .fuzz = fuzz_mode7_zoom_step},
  {.name = "title_zoom_pan", .entry = 0x80F2A0, .end = 0x80F2EE, .source = "src/scale.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_title_zoom_pan, ALL_OUTPUTS, .fuzz = fuzz_title_zoom},
  {.name = "title_zoom_cycle", .entry = 0x80F2EE, .end = 0x80F303, .source = "src/scale.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_title_zoom_cycle, ALL_OUTPUTS, .fuzz = fuzz_title_zoom},
  {.name = "title_zoom_drop", .entry = 0x80F595, .end = 0x80F5BF, .source = "src/scale.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_title_zoom_drop, ALL_OUTPUTS, .fuzz = fuzz_title_zoom},
  {.name = "far_7fff_4", .entry = 0x81E7CE, .end = 0x81E7DA, .source = "src/misc.c", .needM16 = true, .fixedD = true,
   .call = call_far_7fff_4, ALL_OUTPUTS},
  {.name = "empty_80F225", .entry = 0x80F225, .end = 0x80F226, .source = "src/misc.c", .call = call_empty, ALL_OUTPUTS},
  {.name = "window_shape_a", .entry = 0x80DB7D, .end = 0x80DBCB, .alsoIn = {{0x80DC19, 0x80DC5A}}, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .call = call_window_shape_a, ALL_OUTPUTS,
   .fuzz = fuzz_window_shape},
  {.name = "window_shape_b", .entry = 0x80DBCB, .end = 0x80DC5A, .source = "src/misc.c",  // (verify-cover-1: its tail)
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .call = call_window_shape_b, ALL_OUTPUTS,
   .fuzz = fuzz_window_shape},
  {.name = "sprite_number_point_right", .entry = 0x808667, .end = 0x808698, .source = "src/text.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_sprite_number_point_right, ALL_OUTPUTS,
   .fuzz = fuzz_sprite_number_point},
  {.name = "car_message_request", .entry = 0x83FEBD, .end = 0x83FED7, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .call = call_car_message_request, ALL_OUTPUTS, .fuzz = fuzz_car_message_request},
  {.name = "track_event_clear", .entry = 0x82EADA, .end = 0x82EAFE, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x7E, .call = call_track_event_clear, ALL_OUTPUTS,
   .fuzz = fuzz_track_event_clear},
  {.name = "track_event_tail", .entry = 0x82EB1A, .end = 0x82EB45, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x7E, .call = call_track_event_tail, ALL_OUTPUTS, .fuzz = fuzz_track_event_tail},
  {.name = "track_event_2007", .entry = 0x82EA81, .end = 0x82EA87, .alsoIn = {{0x82EB1A, 0x82EB45}}, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x7E, .call = call_track_event_2007, ALL_OUTPUTS, .fuzz = fuzz_track_event_set},
  {.name = "track_event_2047", .entry = 0x82EA87, .end = 0x82EA8D, .alsoIn = {{0x82EB1A, 0x82EB45}}, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x7E, .call = call_track_event_2047, ALL_OUTPUTS, .fuzz = fuzz_track_event_set},
  {.name = "track_event_2087", .entry = 0x82EA8D, .end = 0x82EA93, .alsoIn = {{0x82EB1A, 0x82EB45}}, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x7E, .call = call_track_event_2087, ALL_OUTPUTS, .fuzz = fuzz_track_event_set},
  {.name = "track_event_2107", .entry = 0x82EACE, .end = 0x82EAD4, .alsoIn = {{0x82EB1A, 0x82EB45}}, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x7E, .call = call_track_event_2107, ALL_OUTPUTS, .fuzz = fuzz_track_event_set},
  {.name = "track_event_c2e7", .entry = 0x82EAD4, .end = 0x82EADA, .alsoIn = {{0x82EB1A, 0x82EB45}}, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x7E, .call = call_track_event_c2e7, ALL_OUTPUTS, .fuzz = fuzz_track_event_set},
  {.name = "title_sprites_place", .entry = 0x8A824B, .end = 0x8A8290, .source = "src/misc.c", .needM16 = true,
   .lowWramDb = true, .fixedD = true, .call = call_title_sprites_place, ALL_OUTPUTS, .fuzz = fuzz_title_sprites_place},
  {.name = "rotation_from_angle", .entry = 0x8A867C, .end = 0x8A869A, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_rotation_from_angle, ALL_OUTPUTS, .fuzz = fuzz_rotation_from_angle},
  {.name = "race_message_request_0233", .entry = 0x829BC9, .end = 0x829BE5, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_race_message_request_0233, ALL_OUTPUTS, .fuzz = fuzz_race_message_request},
  {.name = "race_message_request_02e6", .entry = 0x829BE5, .end = 0x829C01, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_race_message_request_02e6, ALL_OUTPUTS, .fuzz = fuzz_race_message_request},
  {.name = "race_message_request_0295", .entry = 0x829C01, .end = 0x829C1D, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_race_message_request_0295, ALL_OUTPUTS, .fuzz = fuzz_race_message_request_34},
  {.name = "race_message_request_0348", .entry = 0x829C1D, .end = 0x829C39, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_race_message_request_0348, ALL_OUTPUTS, .fuzz = fuzz_race_message_request_34},
  {.name = "track_entry_bounds", .entry = 0x81E7DA, .end = 0x81E812, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_track_entry_bounds, ALL_OUTPUTS, .fuzz = fuzz_track_entry_bounds},
  {.name = "spark_step", .entry = 0x80EA77, .end = 0x80EB66, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x81, .call = call_spark_step, .fuzz = fuzz_spark_step},
  {.name = "car_clear_53", .entry = 0x83FD3C, .end = 0x83FD45, .source = "src/misc.c", .needM16 = true,
   .call = call_car_clear_53, ALL_OUTPUTS},
  {.name = "block_copy_7f2000", .entry = 0x80E8B5, .end = 0x80E8CE, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x81, .call = call_block_copy_7f2000, ALL_OUTPUTS, .fuzz = fuzz_block_copy_7f2000},
  {.name = "text_line_push", .entry = 0x8A99E2, .end = 0x8A99FC, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_text_line_push, .compareX = true, .fuzz = fuzz_text_line_push},
  {.name = "track_object_test", .entry = 0x83FA34, .end = 0x83FA6A,  // (verify-cover-1: its tail)
   .source = "src/misc.c", .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_track_object_test,
   .fuzz = fuzz_track_object},
  {.name = "track_object_bump", .entry = 0x83FA6A, .end = 0x83FAD7, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_track_object_bump, .fuzz = fuzz_track_object},
  {.name = "track_object_money", .entry = 0x83FAD7, .end = 0x83FB15, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_track_object_money, .fuzz = fuzz_track_object},
  {.name = "car_set_53_tail", .entry = 0x83FD45, .end = 0x83FD5D, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_car_set_53_tail, ALL_OUTPUTS, .fuzz = fuzz_car_set_53},
  {.name = "car_set_53_1", .entry = 0x83FD26, .end = 0x83FD31, .alsoIn = {{0x83FD45, 0x83FD5D}},
   .source = "src/misc.c", .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_car_set_53_1,
   ALL_OUTPUTS, .fuzz = fuzz_car_set_53},
  {.name = "car_set_53_2", .entry = 0x83FD31, .end = 0x83FD3C, .alsoIn = {{0x83FD45, 0x83FD5D}},
   .source = "src/misc.c", .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_car_set_53_2,
   ALL_OUTPUTS, .fuzz = fuzz_car_set_53},
  {.name = "wait_frames_8a8bb6", .entry = 0x8A8BB6, .end = 0x8A8BC2, .rtl = true, .source = "src/frame.c",
   .needM16 = true, .waitsForInterrupts = true, .call = call_wait_frames, ALL_OUTPUTS},
  {.name = "wait_frames_8a9880", .entry = 0x8A9880, .end = 0x8A988C, .rtl = true, .source = "src/frame.c",
   .needM16 = true, .waitsForInterrupts = true, .call = call_wait_frames, ALL_OUTPUTS},
  {.name = "variant_lines", .entry = 0x8A9AB1, .end = 0x8A9AC9, .source = "src/misc.c", .needM16 = true,
   .fixedD = true, .call = call_variant_lines, ALL_OUTPUTS, .fuzz = fuzz_variant},
  {.name = "variant_5a", .entry = 0x8A98F0, .end = 0x8A9908, .source = "src/misc.c", .needM16 = true,
   .fixedD = true, .call = call_variant_5a, ALL_OUTPUTS, .fuzz = fuzz_variant},
  {.name = "variant_66", .entry = 0x8A99BE, .end = 0x8A99D0, .alsoIn = {{0x8A99D0, 0x8A99D6}}, .source = "src/misc.c",
   .needM16 = true, .fixedD = true, .call = call_variant_66, ALL_OUTPUTS, .fuzz = fuzz_variant},
  {.name = "text_lines_copy", .entry = 0x8A9AC9, .end = 0x8A9AF1, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_text_lines_copy, ALL_OUTPUTS, .fuzz = fuzz_text_lines_copy},
  {.name = "track_object_count", .entry = 0x83FB15, .end = 0x83FB4A, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_track_object_count, .fuzz = fuzz_track_object_count},
  {.name = "race_section_seek_1", .entry = 0x859D39, .end = 0x859D5A, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_race_section_seek_1, ALL_OUTPUTS,
   .fuzz = fuzz_race_section_seek},
  {.name = "race_section_seek_2", .entry = 0x859D5A, .end = 0x859D7B, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_race_section_seek_2, ALL_OUTPUTS,
   .fuzz = fuzz_race_section_seek},
  {.name = "row_copy_6a33", .entry = 0x8582D8, .end = 0x8582FA, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x83, .call = call_row_copy_6a33, ALL_OUTPUTS, .fuzz = fuzz_row_copy},
  {.name = "row_copy_6b33", .entry = 0x8582FA, .end = 0x85831C, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x83, .call = call_row_copy_6b33, ALL_OUTPUTS, .fuzz = fuzz_row_copy},
  {.name = "tile_block_5x5", .entry = 0x82AD99, .end = 0x82ADC2, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_tile_block_5x5, ALL_OUTPUTS, .fuzz = fuzz_tile_block_5x5},
  {.name = "track_settings_load", .entry = 0x85831C, .end = 0x858377, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x83, .call = call_track_settings_load, ALL_OUTPUTS,
   .fuzz = fuzz_track_settings_load},
  {.name = "rows_7f_copy8", .entry = 0x86CE4A, .end = 0x86CEA7, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .call = call_rows_7f_copy8, ALL_OUTPUTS, .fuzz = fuzz_rows_7f_copy8},
  {.name = "block_c4_build", .entry = 0x80C31A, .end = 0x80C35E, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .call = call_block_c4_build, ALL_OUTPUTS, .fuzz = fuzz_block_c4_build},
  {.name = "queue_7f_init", .entry = 0x80EBCE, .end = 0x80EBF7, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .call = call_queue_7f_init, ALL_OUTPUTS, .fuzz = fuzz_queue_7f_init},
  {.name = "text_map_clear", .entry = 0x8A9BF6, .end = 0x8A9C19, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .call = call_text_map_clear, ALL_OUTPUTS},
  {.name = "variant_offsets_add", .entry = 0x8A98B4, .end = 0x8A98F0, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_variant_offsets_add, ALL_OUTPUTS,
   .fuzz = fuzz_variant_offsets_add},
  {.name = "palettes_load_8a", .entry = 0x8A81BA, .end = 0x8A8205, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x8A, .call = call_palettes_load_8a, ALL_OUTPUTS, .fuzz = fuzz_palettes_load_8a},
  {.name = "track_map_load", .entry = 0x859D7B, .end = 0x859DAD, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .db = 0x83, .call = call_track_map_load, ALL_OUTPUTS, .fuzz = fuzz_track_map_load},
  {.name = "track_segments_load", .entry = 0x82A954, .end = 0x82A9AF, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x83, .call = call_track_segments_load, ALL_OUTPUTS, .fuzz = fuzz_track_segments_load},
  {.name = "wait_03e0", .entry = 0x84A4CD, .end = 0x84A4D4, .rtl = true, .source = "src/frame.c", .needM16 = true,
   .lowWramDb = true, .waitsForInterrupts = true, .call = call_wait_03e0, ALL_OUTPUTS},
  {.name = "variant_flags_apply", .entry = 0x8AB0E1, .end = 0x8AB11A, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_variant_flags_apply, ALL_OUTPUTS,
   .fuzz = fuzz_variant_flags_apply},
  {.name = "sprite_slots_six", .entry = 0x8AC19C, .end = 0x8AC1C6, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_sprite_slots_six, ALL_OUTPUTS,
   .fuzz = fuzz_sprite_slots_six},
  {.name = "tiles_planar_upload", .entry = 0x8A8CB7, .end = 0x8A8D81, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_tiles_planar_upload, ALL_OUTPUTS,
   .fuzz = fuzz_tiles_planar_upload},
  {.name = "track_backdrop_load", .entry = 0x82A1D3, .end = 0x82A28B, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x83, .call = call_track_backdrop_load, ALL_OUTPUTS, .fuzz = fuzz_track_backdrop_load},
  {.name = "race_positions_update", .entry = 0x82AD2C, .end = 0x82AD99, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x83, .call = call_race_positions_update, ALL_OUTPUTS, .fuzz = fuzz_race_positions_update},
  {.name = "race_bands_merge", .entry = 0x85B1FF, .end = 0x85B2B7, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .lowWramDb = true, .call = call_race_bands_merge, ALL_OUTPUTS,
   .fuzz = fuzz_race_bands_merge},
  {.name = "race_bands_merge2", .entry = 0x85B2B7, .end = 0x85B36F, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .lowWramDb = true, .call = call_race_bands_merge2, ALL_OUTPUTS,
   .fuzz = fuzz_race_bands_merge2},
  {.name = "vs_bands_merge", .entry = 0x85B1F9, .end = 0x85B1FF, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .lowWramDb = true, .call = call_vs_bands_merge, ALL_OUTPUTS,
   .fuzz = fuzz_vs_bands_merge, .alsoIn = {{0x85B2B7, 0x85B36F}}},  // its JMP to race_bands_merge2
  {.name = "tiles_planar_upload_5fe0", .entry = 0x85817B, .end = 0x858245, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x83, .call = call_tiles_planar_upload_5fe0, ALL_OUTPUTS,
   .fuzz = fuzz_tiles_planar_upload},
  {.name = "hdma_gradients_init", .entry = 0x8A8F18, .end = 0x8A8FA4, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .lowWramDb = true, .fixedD = true, .call = call_hdma_gradients_init,
   .fuzz = fuzz_hdma_gradients_init},
  {.name = "player_frame_setup", .entry = 0x83E2D7, .end = 0x83E431, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_player_frame_setup, .fuzz = fuzz_player_frame_setup},
  {.name = "scene_objects_place", .entry = 0x8A86AB, .end = 0x8A8768, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_scene_objects_place, ALL_OUTPUTS,
   .fuzz = fuzz_scene_objects_place},
  {.name = "result_slot_step", .entry = 0x84A31B, .end = 0x84A34B, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_result_slot_step, ALL_OUTPUTS, .fuzz = fuzz_result},
  {.name = "result_tally", .entry = 0x84A34B, .end = 0x84A388, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_result_tally, ALL_OUTPUTS, .fuzz = fuzz_result},
  {.name = "result_slot_step_at", .entry = 0x84A31E, .end = 0x84A34B, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_result_slot_step_at, ALL_OUTPUTS,
   .fuzz = fuzz_result_at},
  {.name = "result_tally_at", .entry = 0x84A34E, .end = 0x84A388, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_result_tally_at, ALL_OUTPUTS,
   .fuzz = fuzz_result_at},
  {.name = "clear_041c", .entry = 0x8A9D1E, .end = 0x8A9D2D, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .call = call_clear_041c, ALL_OUTPUTS},
  {.name = "name_copy_7f2000", .entry = 0x80E8CE, .end = 0x80E8F4, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x81, .call = call_name_copy_7f2000, ALL_OUTPUTS, .fuzz = fuzz_name_copy_7f2000},
  {.name = "car_blocks_7f2000", .entry = 0x80E8F4, .end = 0x80E911, .alsoIn = {{0x80E8B5, 0x80E8CE}},
   .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_car_blocks_7f2000, ALL_OUTPUTS, .fuzz = fuzz_car_blocks},
  {.name = "car_block_7f2000", .entry = 0x80E911, .end = 0x80E925, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x81, .call = call_car_block_7f2000, ALL_OUTPUTS, .fuzz = fuzz_car_blocks},
  {.name = "track_next", .entry = 0x84A41B, .end = 0x84A487, .rtl = true, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_track_next, ALL_OUTPUTS, .fuzz = fuzz_track_next},
  {.name = "results_init", .entry = 0x84A4ED, .end = 0x84A553, .rtl = true, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_results_init, ALL_OUTPUTS, .fuzz = fuzz_results_init},
  {.name = "prize_money_award", .entry = 0x84A388, .end = 0x84A41B, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_prize_money_award, ALL_OUTPUTS,
   .fuzz = fuzz_prize_money_award},
  {.name = "title_zoom_count", .entry = 0x80F5BF, .end = 0x80F5CD, .source = "src/scale.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_title_zoom_count, ALL_OUTPUTS, .fuzz = fuzz_title_zoom_count},
  {.name = "slot_order_fill", .entry = 0x8A9CF8, .end = 0x8A9D1E, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_slot_order_fill, ALL_OUTPUTS, .fuzz = fuzz_slot_order_fill},
  {.name = "player_names_put", .entry = 0x8A9AF1, .end = 0x8A9B7D, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_player_names_put, ALL_OUTPUTS,
   .fuzz = fuzz_player_names_put},
  {.name = "shake_step", .entry = 0x828F8F, .end = 0x829064, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_shake_step, ALL_OUTPUTS, .fuzz = fuzz_shake_step},
  {.name = "track_objects_stream", .entry = 0x82AF33, .end = 0x82B026, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_track_objects_stream, ALL_OUTPUTS,
   .fuzz = fuzz_track_objects_stream},
  {.name = "results_text_build", .entry = 0x8A9AA5, .end = 0x8A9AB1, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_results_text_build, ALL_OUTPUTS,
   .fuzz = fuzz_results_text_build},
  {.name = "player_in_top5", .entry = 0x8A9CB0, .end = 0x8A9CF8, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .call = call_player_in_top5,
   .compareA = true, .compareX = true, .fuzz = fuzz_player_in_top5},
  {.name = "title_scroll_start", .entry = 0x80F6FD, .end = 0x80F709, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .lowWramDb = true, .call = call_title_scroll_start, ALL_OUTPUTS,
   .fuzz = fuzz_title_scroll_start},
  {.name = "save_data_seal", .entry = 0x808199, .end = 0x8081C8, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_save_data_seal, ALL_OUTPUTS,
   .fuzz = fuzz_save_data_seal},
  {.name = "track_snap", .entry = 0x8AC2DD, .end = 0x8AC355, .rtl = true, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_track_snap, ALL_OUTPUTS, .fuzz = fuzz_track_snap},
  {.name = "effect_request_p1", .entry = 0x81F48B, .end = 0x81F4AD, .alsoIn = {{0x81F4AD, 0x81F4AF}},
   .source = "src/misc.c", .needM16 = true, .needX16 = true, .lowWramDb = true, .call = call_effect_request_p1,
   ALL_OUTPUTS, .fuzz = fuzz_effect_request_p1},
  {.name = "tile_anim_step", .entry = 0x8A85AF, .end = 0x8A867C, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_tile_anim_step, ALL_OUTPUTS, .fuzz = fuzz_tile_anim_step},
  {.name = "track_rows_build", .entry = 0x81E631, .end = 0x81E7CE, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x83, .call = call_track_rows_build, ALL_OUTPUTS,
   .fuzz = fuzz_track_rows_build},
  {.name = "track_block_triple", .entry = 0x82ADC2, .end = 0x82AE15, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x83, .call = call_track_block_triple, ALL_OUTPUTS, .fuzz = fuzz_track_block_triple},
  {.name = "track_palettes_load", .entry = 0x82A672, .end = 0x82A7F9, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x83, .call = call_track_palettes_load, ALL_OUTPUTS, .fuzz = fuzz_track_palettes_load},
  {.name = "title_zoom_settle", .entry = 0x80F303, .end = 0x80F38D, .source = "src/scale.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_title_zoom_settle, ALL_OUTPUTS, .fuzz = fuzz_title_zoom_settle},
  {.name = "title_stars_spread", .entry = 0x80F247, .end = 0x80F2A0, .source = "src/scale.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_title_stars_spread, ALL_OUTPUTS, .fuzz = fuzz_title_stars_spread},
  {.name = "stars_draw", .entry = 0x80F950, .end = 0x80FA3F, .source = "src/stars.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_stars_draw, ALL_OUTPUTS, .fuzz = fuzz_stars_draw},
  {.name = "stars_draw_8a", .entry = 0x8A9773, .end = 0x8A9863, .source = "src/stars.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_stars_draw_8a, ALL_OUTPUTS, .fuzz = fuzz_stars_draw},
  {.name = "window_curve_build", .entry = 0x80FA5B, .end = 0x80FAEE, .source = "src/line_tables.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_window_curve_build, ALL_OUTPUTS, .fuzz = fuzz_window_curve_build},
  {.name = "palette_fade_60", .entry = 0x80FAEE, .end = 0x80FB5C, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_palette_fade_60, ALL_OUTPUTS, .fuzz = fuzz_palette_fade_60},
  {.name = "stars_scatter_8a", .entry = 0x8A971B, .end = 0x8A9773, .source = "src/stars.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .lowWramDb = true, .call = call_stars_scatter_8a, ALL_OUTPUTS,
   .fuzz = fuzz_stars_scatter_8a},
  {.name = "hdma_coldata_setup", .entry = 0x8A9C69, .end = 0x8A9C86, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_hdma_coldata_setup, ALL_OUTPUTS,
   .fuzz = fuzz_hdma_table_0d00_init},
  {.name = "hdma_title_setup", .entry = 0x8A8787, .end = 0x8A8828, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_hdma_title_setup, ALL_OUTPUTS, .fuzz = fuzz_hdma_title_setup},
  {.name = "palette_load_70", .entry = 0x8A8DBF, .end = 0x8A8DEC, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_palette_load_70, ALL_OUTPUTS, .fuzz = fuzz_palette_load_70},
  {.name = "text_screen_init", .entry = 0x8AC1C6, .end = 0x8AC224, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_text_screen_init, ALL_OUTPUTS, .fuzz = fuzz_text_screen_init},
  {.name = "stars_scatter", .entry = 0x80F8F8, .end = 0x80F950, .source = "src/stars.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .lowWramDb = true, .call = call_stars_scatter_8a, ALL_OUTPUTS,
   .fuzz = fuzz_stars_scatter_8a},
  {.name = "title_mode7_setup", .entry = 0x80F424, .end = 0x80F4AD, .source = "src/scale.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_title_mode7_setup, ALL_OUTPUTS, .fuzz = fuzz_title_zoom},
  {.name = "hdma_window_setup", .entry = 0x8AA06D, .end = 0x8AA09F, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_hdma_window_setup, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "text_tiles_put_0", .entry = 0x8ABACB, .end = 0x8ABAE3, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x8A, .call = call_text_tiles_put_0, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "text_palettes_load", .entry = 0x8AC24B, .end = 0x8AC278, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_text_palettes_load, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "car_pair_contact", .entry = 0x81F252, .end = 0x81F48B, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x7E, .call = call_car_pair_contact, ALL_OUTPUTS, .fuzz = fuzz_car_pair_contact},
  {.name = "track_object_test_sides", .entry = 0x83FC65, .end = 0x83FD26, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .call = call_track_object_test_sides, .fuzz = fuzz_track_object_test_sides,
   .holes = {{0x83FC90, 0x83FC98}}},  // the jump table
  {.name = "ticker_draw", .entry = 0x80EBF7, .end = 0x80EE56, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .call = call_ticker_draw, .fuzz = fuzz_ticker_draw,
   .holes = {{0x80EDDB, 0x80EDF6}}},  // the control-code table and the marks
  {.name = "race_hud_sprites", .entry = 0x80D823, .end = 0x80DA5F, .source = "src/hud.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_race_hud_sprites, .fuzz = fuzz_race_hud_sprites},
  {.name = "text_buffer_screen", .entry = 0x8AC15D, .end = 0x8AC19C, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_text_buffer_screen, ALL_OUTPUTS, .fuzz = fuzz_text_buffer_screen},
  {.name = "scenery_build", .entry = 0x80DCBB, .end = 0x80E0C8, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_scenery_build, .fuzz = fuzz_scenery_build,
   .holes = {{0x80DD65, 0x80DD71}}},  // the kinds' handler table
  {.name = "race_cars_frame", .entry = 0x81E812, .end = 0x81F1FC, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x83, .call = call_race_cars_frame, ALL_OUTPUTS,
   .fuzz = fuzz_race_cars_frame, .fuzzStack = 0x1FF1,  // the game's stack: the cars' handlers go deep
   .holes = {{0x81EEB6, 0x81EEE3},    // the forward wrap's 15th-17th cars: a BRA jumps over them
             {0x81F0E9, 0x81F0F0}}},  // the backward wrap's 20th car: the leader behind the line wraps
                                      // forward first (an unsigned check), unless a lap is $8000 or more
  {.name = "player1_frame", .entry = 0x83E241, .end = 0x83E25F, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x83, .call = call_player1_frame,
   .fuzz = fuzz_player1_frame},
  {.name = "player_car_frame", .entry = 0x83EAC6, .end = 0x83F6E1, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x83, .call = call_player_car_frame, .fuzz = fuzz_player_car_frame,
   .holes = {{0x83F3B9, 0x83F3C5},    // the jump table by D+$60
             {0x83F54D, 0x83F555}}},  // the jump table by D+$53 (it starts in its JMP's operand)
  {.name = "car_contacts_chain", .entry = 0x81F21E, .end = 0x81F251, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x7E, .call = call_car_contacts_chain, ALL_OUTPUTS, .fuzz = fuzz_car_contacts_chain},
  {.name = "ticker_scene_step", .entry = 0x80E925, .end = 0x80EA77, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x81, .call = call_ticker_scene_step, .fuzz = fuzz_ticker_scene_step},
  {.name = "title_frame_step", .entry = 0x80F5DE, .end = 0x80F6AD, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_title_frame_step, ALL_OUTPUTS, .fuzz = fuzz_title_frame_step},
  {.name = "title_fade_step", .entry = 0x80F6AD, .end = 0x80F6FD, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_title_fade_step, ALL_OUTPUTS, .fuzz = fuzz_title_fade_step},
  {.name = "title_frame_begin", .entry = 0x80F5CD, .end = 0x80F5DE, .alsoIn = {{0x80F670, 0x80F6AD}},
   .source = "src/misc.c", .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .call = call_title_frame_begin,
   ALL_OUTPUTS, .fuzz = fuzz_title_frame_step},
  {.name = "menu_bar_draw", .entry = 0x80C35E, .end = 0x80C475, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x81, .call = call_menu_bar_draw, .fuzz = fuzz_menu_bar_draw},
  {.name = "track_tiles_load", .entry = 0x858000, .end = 0x85817B, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x83, .call = call_track_tiles_load, ALL_OUTPUTS,
   .fuzz = fuzz_track_tiles_load, .holes = {{0x858025, 0x858093}}},  // the layouts' table
  {.name = "track_preview_maybe", .entry = 0x8A8768, .end = 0x8A8787, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_track_preview_maybe, ALL_OUTPUTS,
   .fuzz = fuzz_track_preview_maybe},
  {.name = "track_tiles_preview", .entry = 0x8A8BC2, .end = 0x8A8CB7, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_track_tiles_preview, ALL_OUTPUTS,
   .fuzz = fuzz_track_tiles_preview, .holes = {{0x8A8BED, 0x8A8C5B}}},  // the layouts' table
  {.name = "title_card_show", .entry = 0x8AC056, .end = 0x8AC0F2, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .waitsForInterrupts = true, .maxInsns = 40000000,
   .call = call_title_card_show, ALL_OUTPUTS},
  {.name = "vram_planes_build", .entry = 0x8A8828, .end = 0x8A88DD, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x8A, .call = call_vram_planes_build, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "road_shift_apply", .entry = 0x82F1C1, .end = 0x82F2B3, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x30, .call = call_road_shift_apply, ALL_OUTPUTS, .fuzz = fuzz_road_shift_apply},
  {.name = "road_shift_settle", .entry = 0x82EFDA, .end = 0x82F1C1, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x30, .call = call_road_shift_settle, ALL_OUTPUTS, .fuzz = fuzz_road_shift_settle,
   .holes = {{0x82F0D6, 0x82F0E7}}},  // dead bytes after a JMP (a 32-bit halving nothing reaches)
  {.name = "follow_car_step", .entry = 0x83F73E, .end = 0x83F7F9, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x30, .call = call_follow_car_step, ALL_OUTPUTS, .fuzz = fuzz_follow_car_step},
  {.name = "car_sync_player", .entry = 0x81FA8F, .end = 0x81FB20, .source = "src/track.c", .needM16 = true,
   .needX16 = true, .db = 0x7E, .call = call_car_sync_player, ALL_OUTPUTS, .fuzz = fuzz_car_sync_player},
  {.name = "hud_markers_write", .entry = 0x828961, .end = 0x8289B2, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_hud_markers_write, ALL_OUTPUTS,
   .fuzz = fuzz_hud_markers_write, .endAt = 0x8289B2,  // its tail jump to race_hud_digits
   .vramReadAt = 0x828976},  // a set marker's step over: LDA $2139, the value dead (harness-9)
  {.name = "race_hud_digits", .entry = 0x82B4ED, .end = 0x82B769, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_race_hud_digits, .fuzz = fuzz_race_hud_digits},
  {.name = "car_track_step", .entry = 0x83F7F9, .end = 0x83F9D1, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x83, .call = call_car_track_step, .fuzz = fuzz_car_track_step},
  {.name = "minimap_car", .entry = 0x82D8C6, .end = 0x82D934, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x30, .call = call_minimap_car, .fuzz = fuzz_minimap_car},
  {.name = "player_segment_step", .entry = 0x82F2B3, .end = 0x82F374, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_player_segment_step, .fuzz = fuzz_player_segment_step},
  {.name = "player2_segment_step", .entry = 0x82FA4F, .end = 0x82FB10, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_player2_segment_step, .fuzz = fuzz_player2_segment_step},
  {.name = "road_shift_by_place", .entry = 0x82EEB7, .end = 0x82EEF0, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x30, .call = call_road_shift_by_place, .fuzz = fuzz_road_shift_by_place},
  {.name = "road_shift_frame", .entry = 0x82EC36, .end = 0x82ECEA, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_road_shift_frame, .fuzz = fuzz_road_shift_frame,
   .fuzzStack = 0x1FF1},
  {.name = "road_shift_frame_call", .entry = 0x82EC32, .end = 0x82EC36, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_road_shift_frame_call, .fuzz = fuzz_road_shift_frame,
   .fuzzStack = 0x1FF3},
  {.name = "track_event_none", .entry = 0x82EA80, .end = 0x82EA81, .source = "src/misc.c",
   .call = call_track_event_none, ALL_OUTPUTS, .fuzz = fuzz_nothing},
  {.name = "player_segment_step_long", .entry = 0x82A9AF, .end = 0x82A9B3, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x83, .call = call_player_segment_step_long,
   .fuzz = fuzz_player_segment_step},
  {.name = "player2_segment_step_long", .entry = 0x82A9B3, .end = 0x82A9B7, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x83, .call = call_player2_segment_step_long,
   .fuzz = fuzz_player2_segment_step},
  {.name = "track_setup_load", .entry = 0x82A7F9, .end = 0x82A954, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_track_setup_load, .fuzz = fuzz_track_setup_load},
  {.name = "results_text", .entry = 0x8A9908, .end = 0x8A99B4, .rtl = true, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .call = call_results_text, .fuzz = fuzz_results_text},
  {.name = "save_data_check", .entry = 0x808119, .end = 0x808199, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x00, .call = call_save_data_check, ALL_OUTPUTS, .fuzz = fuzz_save_data_check},
  {.name = "oam_high_load2", .entry = 0x82AEA4, .end = 0x82AF33, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_oam_high_load2, ALL_OUTPUTS, .fuzz = fuzz_oam_high_load2,
   .dsp4Tape = true, .replays = 32},
  {.name = "oam_high_load", .entry = 0x82AE15, .end = 0x82AEA4, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_oam_high_load, ALL_OUTPUTS, .fuzz = fuzz_oam_high_load,
   .dsp4Tape = true},
  {.name = "sprite_put_rows_a", .entry = 0x82D934, .end = 0x82D988, .alsoIn = {{0x82DDBF, 0x82DEA8}},
   .source = "src/misc.c", .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x30,
   .call = call_sprite_put_rows_a, .fuzz = fuzz_sprite_put_rows, .dsp4Tape = true},
  {.name = "sprite_put_rows_b", .entry = 0x82D988, .end = 0x82D9DD, .alsoIn = {{0x82DDBF, 0x82DEA8}},
   .source = "src/misc.c", .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x30,
   .call = call_sprite_put_rows_b, .fuzz = fuzz_sprite_put_rows, .dsp4Tape = true},
  {.name = "sprite_parts_stream", .entry = 0x82DDBF, .end = 0x82DEA8, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x30, .call = call_sprite_parts_stream, .fuzz = fuzz_sprite_parts_stream,
   .dsp4Tape = true},
  {.name = "sprite_parts_stream_rom", .entry = 0x82DEA8, .end = 0x82DF91, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x30, .call = call_sprite_parts_stream_rom,
   .fuzz = fuzz_sprite_parts_stream_rom, .dsp4Tape = true},
  {.name = "race_frame_draw", .entry = 0x828923, .end = 0x828944, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_race_frame_draw, .fuzz = fuzz_race_frame_draw,
   .dsp4Tape = true, .endAt = 0x828944,  // its tail jump to oam_high_load
   .fuzzStack = 0x1FF1},                // the race's stack, as its road callees' rows
  {.name = "vs_sprites_view0", .entry = 0x82BE18, .end = 0x82C170, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_sprites_view0, .fuzz = fuzz_vs_sprites_view0,
   .dsp4Tape = true, .replays = 32, .fuzzStack = 0x1FF3,  // the VS raster IRQs land in its live samples
   .holes = {{0x82BF5D, 0x82BF60}, {0x82BFB1, 0x82BFC2}, {0x82C123, 0x82C168}}},  // a dead JMP; two never-run paths
  {.name = "vs_sprites_view1", .entry = 0x82C170, .end = 0x82C489, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_sprites_view1, .fuzz = fuzz_vs_sprites_view1,
   .dsp4Tape = true, .replays = 32, .fuzzStack = 0x1FF3, .holes = {{0x82C2B7, 0x82C2BA}, {0x82C30B, 0x82C31C}}},
  {.name = "vs_sprites_view2", .entry = 0x82C489, .end = 0x82C7BA, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_sprites_view2, .fuzz = fuzz_vs_sprites_view2,
   .dsp4Tape = true, .replays = 32, .fuzzStack = 0x1FF3, .holes = {{0x82C5E3, 0x82C5E6}, {0x82C637, 0x82C648}}},
  {.name = "vs_sprites_view3", .entry = 0x82C7BA, .end = 0x82CAE5, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_sprites_view3, .fuzz = fuzz_vs_sprites_view3,
   .dsp4Tape = true, .replays = 32, .fuzzStack = 0x1FF3, .holes = {{0x82C90E, 0x82C911}, {0x82C962, 0x82C973}}},
  {.name = "object_sprites_stream", .entry = 0x82CBD4, .end = 0x82D267, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_object_sprites_stream, .fuzz = fuzz_object_sprites_stream,
   .dsp4Tape = true, .dsp4Native = {{0x82CBD4, 0x82CC8F}},  // command $03's prologue (the mini-map's dots)
   .holes = {{0x82CCC7, 0x82CDB7},    // a second entry (never run) that joins at $82:CDB7
             {0x82CED7, 0x82CEDA}}},  // a JMP $D1E7 after a BRA (dead)
  {.name = "car_sprite_head", .entry = 0x82D267, .end = 0x82D2CC, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x30, .call = call_car_sprite_head, ALL_OUTPUTS, .fuzz = fuzz_car_sprite_head,
   .dsp4Tape = true},
  {.name = "object_sprite_raised", .entry = 0x82DAFD, .end = 0x82DB4D, .alsoIn = {{0x82D934, 0x82D9DD}, {0x82DDBF, 0x82DEA8}},
   .source = "src/misc.c", .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x30,
   .call = call_object_sprite_raised, .fuzz = fuzz_object_sprite, .dsp4Tape = true},
  {.name = "sprite_put_rows_c", .entry = 0x82CAE5, .end = 0x82CB39, .alsoIn = {{0x82DDBF, 0x82DEA8}},
   .source = "src/misc.c", .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x30,
   .call = call_sprite_put_rows_c, .fuzz = fuzz_sprite_put_rows, .dsp4Tape = true},
  {.name = "object_sprite_kind0", .entry = 0x82DCEC, .end = 0x82DD3B, .alsoIn = {{0x82CAE5, 0x82CB39}, {0x82DDBF, 0x82DEA8}},
   .source = "src/misc.c", .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x30,
   .call = call_object_sprite_kind0, .fuzz = fuzz_object_sprite, .dsp4Tape = true},
  {.name = "vs_object_flat", .entry = 0x82DD3B, .end = 0x82DD69, .alsoIn = {{0x82CAE5, 0x82CB39}, {0x82DDBF, 0x82DEA8}},
   .source = "src/misc.c", .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x30,
   .call = call_vs_object_flat, .fuzz = fuzz_object_sprite, .dsp4Tape = true},
  {.name = "vs_object_group", .entry = 0x82DD69, .end = 0x82DDBF, .alsoIn = {{0x82CAE5, 0x82CB39}, {0x82DDBF, 0x82DEA8}},
   .source = "src/misc.c", .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x30,
   .call = call_vs_object_group, .fuzz = fuzz_object_sprite, .dsp4Tape = true},
  {.name = "object_sprite_flat", .entry = 0x82DB4D, .end = 0x82DB87, .alsoIn = {{0x82D934, 0x82D9DD}, {0x82DDBF, 0x82DEA8}},
   .source = "src/misc.c", .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x30,
   .call = call_object_sprite_flat, .fuzz = fuzz_object_sprite, .dsp4Tape = true},
  {.name = "object_sprite_group", .entry = 0x82DB87, .end = 0x82DC69, .alsoIn = {{0x82D934, 0x82D9DD}, {0x82DDBF, 0x82DEA8}},
   .source = "src/misc.c", .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x30,
   .call = call_object_sprite_group, .fuzz = fuzz_object_sprite, .dsp4Tape = true,
   .holes = {{0x82DBAA, 0x82DBB2}}},  // the handlers' jump table
  {.name = "object_sprite_shadowed", .entry = 0x82DA48, .end = 0x82DAAF, .alsoIn = {{0x82D934, 0x82D9DD}, {0x82DDBF, 0x82DEA8}},
   .source = "src/misc.c", .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x30,
   .call = call_object_sprite_shadowed, .fuzz = fuzz_object_sprite_shadowed, .dsp4Tape = true},
  {.name = "race_road_frame", .entry = 0x85B3A9, .end = 0x85B3D7, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x83, .call = call_race_road_frame, ALL_OUTPUTS,
   .fuzz = fuzz_race_road_frame, .dsp4Tape = true, .fuzzStack = 0x1FF1},
  {.name = "race_road_frame_alt", .entry = 0x85B36F, .end = 0x85B3A9, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x83, .call = call_race_road_frame_alt, ALL_OUTPUTS,
   .fuzz = fuzz_race_road_frame, .dsp4Tape = true, .fuzzStack = 0x1FF1},
  {.name = "road_rows_project2", .entry = 0x85B828, .end = 0x85BC20, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x83, .call = call_road_rows_project2, ALL_OUTPUTS,
   .fuzz = fuzz_road_rows_project2, .dsp4Tape = true, .fuzzStack = 0x1FF1},
  {.name = "vs_road_project", .entry = 0x85A471, .end = 0x85B1F9, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x83, .call = call_vs_road_project, ALL_OUTPUTS,
   .fuzz = fuzz_vs_road_project, .dsp4Tape = true, .fuzzStack = 0x1FF1,
   .replays = 32,  // the VS raster IRQs land in every live sample (harness-34)
   .holes = {{0x85A94C, 0x85A94F}, {0x85A964, 0x85A967}, {0x85AAB5, 0x85AAB8}, {0x85AABF, 0x85AAD9},
             {0x85AB04, 0x85AB13}}},  // player 1's never-run paths (traps in the C)
  {.name = "road_rows_project", .entry = 0x85B3D7, .end = 0x85B828, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x83, .call = call_road_rows_project, ALL_OUTPUTS,
   .fuzz = fuzz_road_rows_project, .dsp4Tape = true, .fuzzStack = 0x1FF1},
  {.name = "road_rows_stream", .entry = 0x818564, .end = 0x8189F5, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x83, .call = call_road_rows_stream, ALL_OUTPUTS,
   .fuzz = fuzz_road_rows_stream, .dsp4Tape = true, .fuzzStack = 0x1FF1},
  {.name = "dsp4_answers_store", .entry = 0x8189F5, .end = 0x818A4A, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x30, .call = call_dsp4_answers_store, .compareX = true, .compareY = true,
   .fuzz = fuzz_dsp4_answers_store, .dsp4Tape = true},
  {.name = "sprite_part_put", .entry = 0x818527, .end = 0x818564, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x30, .call = call_sprite_part_put, .compareY = true,
   .fuzz = fuzz_sprite_part_put, .dsp4Tape = true},
  {.name = "sprite_bar_put", .entry = 0x818487, .end = 0x818527, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x30, .call = call_sprite_bar_put, ALL_OUTPUTS,
   .fuzz = fuzz_sprite_bar_put, .dsp4Tape = true, .alsoIn = {{0x818527, 0x818564}},
   .holes = {{0x818508, 0x818527}}},  // the full buffer's path: it would return with two words left on the stack
  {.name = "race_grid_setup", .entry = 0x859ADB, .end = 0x859D39, .rtl = true, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x83, .call = call_race_grid_setup,
   .fuzz = fuzz_race_grid_setup},
  {.name = "race_state_reset", .entry = 0x82A28B, .end = 0x82A672, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_race_state_reset, ALL_OUTPUTS,
   .fuzz = fuzz_race_state_reset},
  {.name = "road_rows_build_call", .entry = 0x82DFA9, .end = 0x82DFAD, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_road_rows_build_call, .fuzz = fuzz_road_rows_build,
   .fuzzStack = 0x1FF3},  // the game's stack, one call above road_rows_build's
  {.name = "road_rows_build", .entry = 0x82DFAD, .end = 0x82E503, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_road_rows_build, .fuzz = fuzz_road_rows_build,
   .fuzzStack = 0x1FF1,  // the game's stack (entry SP $1FEF, as road_shift_frame): it writes $1EE6-$1EFA
   .holes = {{0x82E185, 0x82E19A}, {0x82E261, 0x82E279}}},  // never reachable with the ROM's data: a row
                       // list's second pair; the shape table and its flat shape (no segment uses it)
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
  {.name = "func_82EA49_85", .entry = 0x85CD78, .end = 0x85CDAF, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .call = call_func_82EA49, ALL_OUTPUTS, .fuzz = fuzz_func_82EA49},  // (vs-race-32)
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
  // harness-17: a cut whose carry the code after it does not overwrite before its first
  // branch (split_screen_setup's), given wrong: it must still fail on its few samples
  {.name = "ticker_screen_wrong_carry", .entry = 0x80E4E6, .end = 0x80E5F7, .endAt = 0x80E5F7, .source = "src/misc.c",
   .needM16 = true, .needX16 = true, .fixedD = true, .db = 0x81, .compareA = true, .compareX = true, .compareY = true,
   .call = call_ticker_screen_wrong_carry},
  {.name = "hud_markers_write_wrong", .entry = 0x828961, .end = 0x8289B2, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_hud_markers_write_wrong, ALL_OUTPUTS,
   .fuzz = fuzz_hud_markers_write, .endAt = 0x8289B2, .vramReadAt = 0x828976},
  {.name = "oam_high_load_wrong_command", .entry = 0x82AE15, .end = 0x82AEA4, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x83, .call = call_oam_high_load_wrong_command, ALL_OUTPUTS,
   .fuzz = fuzz_oam_high_load, .dsp4Tape = true},
  {.name = "minimap_car_wrong", .entry = 0x82D8C6, .end = 0x82D934, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x30, .call = call_minimap_car_wrong, .fuzz = fuzz_minimap_car},
  {.name = "vram_planes_build_wrong", .entry = 0x8A8828, .end = 0x8A88DD, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .db = 0x8A, .call = call_vram_planes_build_wrong, ALL_OUTPUTS, .fuzz = fuzz_nothing},
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

// ---- glue-1: composed routines (bench --difftest-composed, one row per process) ----
// The player's password shown whole ($8A:BE2E): returns after the NMI handler is put back
// (A = $0008's value; C set by the 8-frame wait's last CMP, which nothing after it changes).
static void call_password_show(const Regs* in, Regs* out) {
  (void)in;
  password_show();
  out->a = rd16(0x0008);
  setNZ16(out, out->a);
  out->c = true;
}
// The password entry whole ($8A:B232): the caret's slide, the typing, the check (the codes'
// handlers, the menus' submenu after a star name, a wrong one's message: never run)
// (exit: a valid password returns after its last STA of $03DC, A = $03DC, V clear and C
// func_8ABF76's; after a code, at the found test's RTS, A = D+$52 and C set by the search)
static void call_password_entry(const Regs* in, Regs* out) {
  (void)in;
  bool carry;
  if(password_entry(&carry)) {
    out->a = rd16(0x03DC);
    out->v = false;
  } else {
    out->a = rd16(DP + 0x52);
  }
  out->c = carry;
  setNZ16(out, out->a);
}
// The VS screen's frames ($8A:A67B, the loop's head, to the RTS; its setup is cut rows:
// whole, an NMI landing in the graphics' unpacking lags it): returns after HDMA is put back
// (A = $0082)
static void call_vs_screen_loop(const Regs* in, Regs* out) {
  (void)in;
  vs_screen_loop();
  out->a = 0x0082;
  setNZ16(out, out->a);
}
// Each composed row names the movies it runs on (verify_all.sh reads the "movies:" comment
// before it).
const Routine composedRoutines[] = {
  // movies: autopilot_60k
  {.name = "password_show", .entry = 0x8ABE2E, .end = 0x8ABF0D, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .waitsForInterrupts = true, .maxInsns = 40000000,
   .manyEffects = true, .call = call_password_show},
  // movies: explore_pw code_infodata
  {.name = "password_entry", .entry = 0x8AB232, .end = 0x8AB339, .source = "src/password.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .waitsForInterrupts = true, .maxInsns = 40000000,
   .manyEffects = true, .call = call_password_entry, .holes = {{0x8AB2ED, 0x8AB2FD}, {0x8AB302, 0x8AB31B}}},
  // movies: vs_race
  {.name = "vs_screen_loop", .entry = 0x8AA67B, .end = 0x8AA6F6, .source = "src/vs.c", .needM16 = true,
   .needX16 = true, .fixedD = true, .db = 0x8A, .compareA = true, .waitsForInterrupts = true, .maxInsns = 400000000,
   .manyEffects = true, .call = call_vs_screen_loop,
   .holes = {{0x8AA692, 0x8AA698}, {0x8AA69D, 0x8AA6A3}, {0x8AA6A8, 0x8AA6AE}}},  // (the other players' pads)
};
const int composedRoutineCount = sizeof composedRoutines / sizeof composedRoutines[0];
