// SPDX-License-Identifier: GPL-3.0-or-later
// Bench-only bridge between LakeSnes's cart mapper and StackBlender's clean-room DSP-4.
// Every access is logged so the bench's ground truth can be re-validated later, and so the
// native DSP-4 C can be checked command by command.
#include "dsp4_bridge.h"
#include "bench_log.h"

// The bridge reads the clean-room chip's busy flag and command to route commands (the
// chip's code is not modified)
#define private public
#include "dsp4/Dsp4.h"
#undef private

extern "C" {
#include "dsp4.h"
}

#include <cstdio>
#include <cstdlib>
#include <cstring>

static dsp4::Dsp4 chip;

// Commands proved on the firmware (refbench) are answered by the native C (src/dsp4.c)
// instead of the clean-room chip: the chip still gets every byte, so it stays in step, but
// the game reads ours. So far: $0A (dsp4_edges).
static bool routed;
static uint8_t routedIn[6], routedOut[8];
static int routedNIn, routedNOut = -1;

extern "C" void dsp4_bridge_reset(void) {
	chip.reset();
	routed = false;
	routedNOut = -1;
}

extern "C" uint8_t dsp4_bridge_read(uint8_t bank, uint16_t adr) {
	const bool data = adr < 0xC000;
	uint8_t val = data ? chip.readData() : chip.readStatus();
	if(data && routed && routedNOut >= 0) {
		val = routedOut[routedNOut++];
		if(routedNOut == 8) routed = false;
	}
	bench_log_dsp4(false, data, bank, adr, val);
	return val;
}

extern "C" void dsp4_bridge_write(uint8_t bank, uint16_t adr, uint8_t val) {
	const bool data = adr < 0xC000;
	if(data) {  // writes to the status register are ignored, as on the chip
		const bool wasBusy = chip.m.busy != 0;
		chip.writeData(val);
		if(!wasBusy && chip.m.busy && (chip.m.command & 31) == 0x0A) {
			routed = true;
			routedNIn = 0;
			routedNOut = -1;
		} else if(routed && routedNOut < 0) {
			routedIn[routedNIn++] = val;
			if(routedNIn == 6) {
				int16_t edge[4];
				dsp4_edges(uint16_t(routedIn[0] | routedIn[1] << 8), uint16_t(routedIn[2] | routedIn[3] << 8),
				           uint16_t(routedIn[4] | routedIn[5] << 8), edge);
				for(int j = 0; j < 4; j++) {
					routedOut[2 * j] = uint8_t(edge[j]);
					routedOut[2 * j + 1] = uint8_t(uint16_t(edge[j]) >> 8);
				}
				routedNOut = 0;
			}
		} else if(routed) {
			routed = false;  // a write where results were due: not this command's use any more
		}
	}
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
