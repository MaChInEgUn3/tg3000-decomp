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
#include "frame.h"
#include "misc.h"
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
