// SPDX-License-Identifier: GPL-3.0-or-later
// Bench-only bridge between LakeSnes's cart mapper and StackBlender's clean-room DSP-4.
// Every access is logged so the bench's ground truth can be re-validated later, and so the
// native DSP-4 C can be checked command by command.
#include "dsp4_bridge.h"
#include "bench_log.h"

#include "dsp4/Dsp4.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

static dsp4::Dsp4 chip;

extern "C" void dsp4_bridge_reset(void) { chip.reset(); }

extern "C" uint8_t dsp4_bridge_read(uint8_t bank, uint16_t adr) {
	const bool data = adr < 0xC000;
	const uint8_t val = data ? chip.readData() : chip.readStatus();
	bench_log_dsp4(false, data, bank, adr, val);
	return val;
}

extern "C" void dsp4_bridge_write(uint8_t bank, uint16_t adr, uint8_t val) {
	const bool data = adr < 0xC000;
	if(data) chip.writeData(val);  // writes to the status register are ignored, as on the chip
	bench_log_dsp4(true, data, bank, adr, val);
}

extern "C" int dsp4_bridge_state_size(void) { return int(chip.saveState().size()); }

extern "C" void dsp4_bridge_save_state(uint8_t* out) {
	const std::vector<uint8_t> blob = chip.saveState();
	std::memcpy(out, blob.data(), blob.size());
}

extern "C" void dsp4_bridge_load_state(const uint8_t* data, int size) {
	if(!chip.loadState(data, size_t(size))) {
		std::fprintf(stderr, "dsp4: save state rejected\n");
		std::abort();  // a silently wrong DSP-4 would poison every comparison
	}
}
