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
#include "scale.h"
#include "line_tables.h"
#include "decompress.h"
#include "dsp4.h"
#include "frame.h"
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

static void fuzz_byte_buffer_push(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)r;
  wr16ram(ram, DP + 0x58, (uint16_t)(rnd() % 0x200));
  ram[DP + 0x60] = (uint8_t)rnd();
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
static void fuzz_decompress(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  for(int i = 0; i < 0x2100; i++) ram[0x10000 + i] = (uint8_t)rnd();
  uint32_t p = 0x4000, y = 0x2100;
  int commands = 4 + (int)(rnd() % 24);
  for(int c = 0; c < commands && y < 0x3800; c++) {
    switch(rnd() % 8) {
      case 0: {
        uint8_t n = (uint8_t)(1 + rnd() % 0x3F);
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
  r->a = 0x7F7E;
  r->x = 0x4000;
  r->y = 0x2100;
}

// ---- decompress_near ($80:FBA0) ----  JSL decompress / RTS: same registers and flags.
static void call_decompress_near(const Regs* in, Regs* out) { call_decompress(in, out); }

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

// ---- sprite_group ($80:B49E) ----  A low byte = table bank, X = table address; D = $0100
// or $0200. Exits with A = 0 (the last terminator), X = the new slot, Y = the second
// terminator's address, N/Z from the PLB (the caller's DB), C/V of the last item's
// ADC #6 (or as on entry if both lists are empty).
static void call_sprite_group(const Regs* in, Regs* out) {
  uint32_t bank = (uint32_t)(in->a & 0xFF) << 16, at = bank | in->x;
  bool any = false;
  uint16_t lastY = 0;
  for(int list = 0; list < 2; list++) {
    while(bus_rd16(at) != 0) { any = true; lastY = (uint16_t)at; at = bank | (uint16_t)(at + 6); }
    if(list == 0) at = bank | (uint16_t)(at + 2);
  }
  uint32_t end = sprite_group(in->d, bank | in->x);
  out->a = 0;
  out->x = rd16((uint16_t)(in->d + 0x74));
  out->y = (uint16_t)end;
  out->n = (in->db & 0x80) != 0;
  out->z = in->db == 0;
  if(any) setAdc16(out, lastY, 6);
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
static void call_oam_finish_0b00(const Regs* in, Regs* out) {
  uint16_t end = rd16((uint16_t)(in->d + 0x74)), last = rd16((uint16_t)(in->d + 0x72));
  oam_finish_0b00(in->d);
  oam_high_table_outputs_from(out, 0x0900, end, last);
  out->a = end;
  setNZ16(out, end);
}

static void fuzz_oam_finish_0b00(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  fuzz_oam_tables(r, ram, rnd, 0x0B00);
}

// ---- oam_finish ($81:80DA, long) ----  exits with X = the old D+$6C, A = the old D+$6E
// (N/Z from its LDA), C/V of its SBC #$0900, Y as the packer left it.
static void call_oam_finish(const Regs* in, Regs* out) {
  uint16_t built = rd16((uint16_t)(in->d + 0x6C)), other = rd16((uint16_t)(in->d + 0x6E));
  uint16_t end = rd16((uint16_t)(in->d + 0x74)), last = rd16((uint16_t)(in->d + 0x72));
  oam_finish(in->d);
  oam_high_table_outputs_from(out, built, end, last);
  out->x = built;
  out->a = other;
  setNZ16(out, other);
  setSbc16(out, other, 0x0900);
}

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

static void fuzz_big_number(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  for(int i = 0; i < 32; i++) ram[0x0DAC + i] = (uint8_t)rnd();
  r->y = (uint16_t)(1 + rnd() % 24);
  wr16ram(ram, DP + 0x46, (uint16_t)rnd());
  wr16ram(ram, DP + 0x84, (uint16_t)rnd());
  wr16ram(ram, DP + 0x92, (uint16_t)rnd());
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
  {.name = "func_8ABBF4", .entry = 0x8ABBF4, .end = 0x8ABC32, .source = "src/misc.c", .needM16 = true,
   .needX16 = true, .lowWramDb = true, .fixedD = true, .call = call_func_8ABBF4, ALL_OUTPUTS, .fuzz = fuzz_big_number},
  {.name = "func_81FBB1", .entry = 0x81FBB1, .end = 0x81FC72, .source = "src/track.c", .needM16 = true,
   .needX16 = true, .db = 0x7E, .fixedD = true, .call = call_func_81FBB1, ALL_OUTPUTS, .fuzz = fuzz_func_81FBB1},
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
};
const int routineCount = sizeof routines / sizeof routines[0];

const Routine negativeRoutines[] = {
  {.name = "rng_next_wrong", .entry = 0x8087E9, .end = 0x808807, .source = "src/rng.c",
   .needM16 = true, .call = call_rng_next_wrong, ALL_OUTPUTS, .fuzz = fuzz_rng_next},
  {.name = "upload_vram_now_wrong", .entry = 0x8083AD, .end = 0x8083CB, .rtl = true, .source = "src/video_dma.c",
   .needM16 = true, .needX16 = true, .call = call_upload_vram_now_wrong, ALL_OUTPUTS, .fuzz = fuzz_upload_vram_now},
  {.name = "wait_frames_wrong", .entry = 0x808373, .end = 0x80837D, .source = "src/frame.c", .needM16 = true,
   .lowWramDb = true, .waitsForInterrupts = true, .call = call_wait_frames_wrong, ALL_OUTPUTS},
};
const int negativeRoutineCount = sizeof negativeRoutines / sizeof negativeRoutines[0];
