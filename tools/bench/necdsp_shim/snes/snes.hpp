// SPDX-License-Identifier: GPL-3.0-or-later
// harness-27: the minimal surroundings bsnes v087's NEC uPD7725 core (snes/chip/necdsp, from
// the BizHawk 1.9.4 tree tools/refbench/fetch.sh puts in ext/; GPLv3) needs to compile on
// its own for the bench: nall's types, and stubs for the scheduler, the CPU's
// synchronisation and the coprocessor base. The bench drives the core itself
// (tools/bench/dsp4_lle.cpp): it runs it until the chip waits for data.
#ifndef TG3000_NECDSP_SHIM_HPP
#define TG3000_NECDSP_SHIM_HPP

#include <base/base.hpp>

namespace SNES {

struct Scheduler {
  enum class SynchronizeMode : unsigned { None, CPU, All } sync = SynchronizeMode::None;
  enum class ExitReason : unsigned { UnknownEvent, FrameEvent, SynchronizeEvent, DebuggerEvent };
  void exit(ExitReason) {}
};
extern Scheduler scheduler;

struct Processor {
  void serialize(serializer&) {}
};

struct Coprocessor : Processor {
  void create(void (*)(), unsigned) {}
  void step(unsigned) {}
  void synchronize_cpu() {}
};

struct CPUShim {
  void synchronize_coprocessors() {}
};
extern CPUShim cpu;

struct NvramEntry {
  NvramEntry(const char*, uint8_t*, unsigned) {}
};
struct NvramShim {
  void append(const NvramEntry&) {}  // (the uPD96050's RAM; the DSP-4 is a uPD7725)
};
struct CartridgeShim {
  NvramShim nvram;
};
extern CartridgeShim cartridge;

#include <snes/chip/necdsp/necdsp.hpp>

}

#endif
