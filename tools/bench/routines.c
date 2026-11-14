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
#include "stars.h"
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

static void fuzz_upload_tiles(Regs* r, uint8_t* ram, uint32_t (*rnd)(void)) {
  (void)ram;
  r->x = (uint16_t)(2 * (rnd() % 64));
  r->a = (uint16_t)(rnd() & 0x1FFF);
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
};
const int routineCount = sizeof routines / sizeof routines[0];

const Routine negativeRoutines[] = {
  {.name = "rng_next_wrong", .entry = 0x8087E9, .end = 0x808807, .source = "src/rng.c",
   .needM16 = true, .call = call_rng_next_wrong, ALL_OUTPUTS, .fuzz = fuzz_rng_next},
  {.name = "upload_vram_now_wrong", .entry = 0x8083AD, .end = 0x8083CB, .rtl = true, .source = "src/video_dma.c",
   .needM16 = true, .needX16 = true, .call = call_upload_vram_now_wrong, ALL_OUTPUTS, .fuzz = fuzz_upload_vram_now},
};
const int negativeRoutineCount = sizeof negativeRoutines / sizeof negativeRoutines[0];
