// SPDX-License-Identifier: GPL-3.0-or-later
// harness-27: the DSP-4 run from its firmware on the bench (bench --dsp4-firmware FILE), the
// owner's decision of 2026-11-24: the firmware file he supplied (ext/firmware/dsp4.rom, never
// committed, never read or disassembled by the project) is executed by bsnes v087's uPD7725
// core, as on the reference runner, for verification only; the port's C never uses it.
// The core is compiled from ext/ (tools/refbench/fetch.sh) with tools/bench/necdsp_shim.
//
// Timing: the chip runs, on each data access, until it next waits for the CPU: until it is
// about to test RQM (JRQM/JNRQM) while RQM is set, its loop for the CPU to take or give a
// word. (RQM alone is set as soon as it has read the CPU's word, before its answer exists.)
// So it always looks ready; the game accepts that (harness-26's tape replay).
#include <snes/snes.hpp>

namespace SNES {
Scheduler scheduler;
CPUShim cpu;
CartridgeShim cartridge;
}

#include <snes/chip/necdsp/necdsp.cpp>

#include <cstdio>
#include <cstring>
#include <vector>

using SNES::necdsp;

// at most this many instructions before the chip waits again (a stuck chip is a fault)
static const unsigned STEP_LIMIT = 1u << 22;

static void runUntilWaiting(void) {
  for(unsigned n = 0; n < STEP_LIMIT; n++) {
    const uint24 next = necdsp.programROM[necdsp.regs.pc];
    const unsigned brch = (next >> 13) & 0x1FF;
    if((next >> 22) == 2 && (brch == 0x0bc || brch == 0x0be) && necdsp.regs.sr.rqm) return;
    const uint24 opcode = necdsp.programROM[necdsp.regs.pc++];
    switch(opcode >> 22) {
      case 0: necdsp.exec_op(opcode); break;
      case 1: necdsp.exec_rt(opcode); break;
      case 2: necdsp.exec_jp(opcode); break;
      case 3: necdsp.exec_ld(opcode); break;
    }
    const int32_t result = (int32_t)necdsp.regs.k * necdsp.regs.l;
    necdsp.regs.m = result >> 15;
    necdsp.regs.n = result << 1;
  }
  std::fprintf(stderr, "dsp4 firmware: the chip ran %u instructions without waiting\n", STEP_LIMIT);
}

extern "C" bool dsp4_lle_load(const char* path) {
  FILE* f = std::fopen(path, "rb");
  if(f == nullptr) return false;
  std::vector<uint8_t> data(2048 * 3 + 1024 * 2);
  const bool ok = std::fread(data.data(), 1, data.size(), f) == data.size() && std::fgetc(f) == EOF;
  std::fclose(f);
  if(!ok) return false;
  // (as bsnes's cartridge loader: little-endian 24-bit program words, then 16-bit data words)
  for(unsigned n = 0; n < 16384; n++) necdsp.programROM[n] = 0;
  for(unsigned n = 0; n < 2048; n++) necdsp.dataROM[n] = 0;
  for(unsigned n = 0; n < 2048; n++) necdsp.programROM[n] = data[3 * n] | data[3 * n + 1] << 8 | data[3 * n + 2] << 16;
  for(unsigned n = 0; n < 1024; n++) necdsp.dataROM[n] = data[6144 + 2 * n] | data[6144 + 2 * n + 1] << 8;
  necdsp.revision = SNES::NECDSP::Revision::uPD7725;
  necdsp.power();
  necdsp.reset();
  std::memset(necdsp.dataRAM, 0, sizeof necdsp.dataRAM);
  runUntilWaiting();
  return true;
}

extern "C" void dsp4_lle_reset(void) {
  necdsp.reset();
  runUntilWaiting();
}

extern "C" uint8_t dsp4_lle_read(bool data) {
  if(!data) return 0x80;  // ready (the chip has run to its next wait)
  const uint8_t v = necdsp.dr_read(0);
  runUntilWaiting();
  return v;
}

extern "C" void dsp4_lle_write(bool data, uint8_t v) {
  if(!data) return;
  necdsp.dr_write(0, v);
  runUntilWaiting();
}

extern "C" int dsp4_lle_state_size(void) {
  serializer s(1 << 16);
  necdsp.serialize(s);
  return int(s.size());
}
extern "C" void dsp4_lle_save_state(uint8_t* out, int size) {
  serializer s(1 << 16);
  necdsp.serialize(s);
  std::memcpy(out, s.data(), size_t(size));
}
extern "C" void dsp4_lle_load_state(const uint8_t* in, int size) {
  serializer s(in, unsigned(size));
  necdsp.serialize(s);
}

// dsp4-11a: one command on an idle chip (reset first): its word, its input words, its output
// words, as the game sends and reads them (low byte first). For tools/bench/dsp4_probe.cpp.
extern "C" void dsp4_lle_command(uint16_t cmd, const uint16_t* in, int nin, uint16_t* out, int nout) {
  dsp4_lle_reset();
  dsp4_lle_write(true, uint8_t(cmd));
  dsp4_lle_write(true, uint8_t(cmd >> 8));
  for(int i = 0; i < nin; i++) {
    dsp4_lle_write(true, uint8_t(in[i]));
    dsp4_lle_write(true, uint8_t(in[i] >> 8));
  }
  for(int i = 0; i < nout; i++) {
    const uint8_t lo = dsp4_lle_read(true);
    const uint8_t hi = dsp4_lle_read(true);
    out[i] = uint16_t(lo | hi << 8);
  }
}
