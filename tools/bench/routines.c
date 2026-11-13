// SPDX-License-Identifier: GPL-3.0-or-later
// Registry of ported routines for the differential test: the adapters map the original's
// registers to the C function's arguments. This mapping lives here, never in src/.
#include "routines.h"

#include <stddef.h>

#include "rng.h"
#include "tg3000.h"

static void setNZ16(Regs* r, uint16_t v) { r->n = (v >> 15) & 1; r->z = v == 0; }

// rng_next: state at D+$10, 16-bit A; returns A = new low word, C = bit shifted out.
// N/Z follow A (the routine ends with LDA); V, X, Y, D, DB are untouched.
static void call_rng_next(const Regs* in, Regs* out) {
  bool carry;
  out->a = rng_next((uint16_t)(in->d + 0x10), &carry);
  out->c = carry;
  setNZ16(out, out->a);
}

// Negative test: the same adapter around a deliberately wrong C (one XOR constant off).
// --difftest-negative must report FAIL, proving the comparison can fail.
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

const Routine routines[] = {
  {"rng_next", 0x8087E9, 0x808807, false, "src/rng.c", true, false, call_rng_next, true, true, true, true, 0x10, 4},
  {"rng_next_8a", 0x8A988C, 0x8A98AA, false, "src/rng.c", true, false, call_rng_next, true, true, true, true, 0x10, 4},
};
const int routineCount = sizeof routines / sizeof routines[0];

const Routine negativeRoutines[] = {
  {"rng_next_wrong", 0x8087E9, 0x808807, false, "src/rng.c", true, false, call_rng_next_wrong, true, true, true, true, 0x10, 4},
};
const int negativeRoutineCount = sizeof negativeRoutines / sizeof negativeRoutines[0];
