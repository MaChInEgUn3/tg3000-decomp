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
// the game reads ours.
static void answerEdges(const uint8_t* in, uint8_t* out) {
	int16_t edge[4];
	dsp4_edges(uint16_t(in[0] | in[1] << 8), uint16_t(in[2] | in[3] << 8), uint16_t(in[4] | in[5] << 8), edge);
	for(int j = 0; j < 4; j++) {
		out[2 * j] = uint8_t(edge[j]);
		out[2 * j + 1] = uint8_t(uint16_t(edge[j]) >> 8);
	}
}
static void answerRow(const uint8_t* in, uint8_t* out) {
	int16_t edge[4];
	for(int j = 0; j < 4; j++) edge[j] = int16_t(in[2 * j] | in[2 * j + 1] << 8);
	const uint16_t row = dsp4_row(edge);
	out[0] = uint8_t(row);
	out[1] = uint8_t(row >> 8);
}
// A sprite entry: the last six bytes in (x, y, attribute); its three words out.
static void answerSpriteAt(const uint8_t* e, uint8_t* out) {
	uint16_t mine[2];
	const uint16_t flag = dsp4_sprite(uint16_t(e[0] | e[1] << 8), uint16_t(e[2] | e[3] << 8), uint16_t(e[4] | e[5] << 8), mine);
	const uint16_t words[3] = {flag, mine[0], mine[1]};
	for(int j = 0; j < 3; j++) {
		out[2 * j] = uint8_t(words[j]);
		out[2 * j + 1] = uint8_t(words[j] >> 8);
	}
}
static void answerSprite(const uint8_t* in, uint8_t* out) { answerSpriteAt(in, out); }
static void answerSpriteList(const uint8_t* in, uint8_t* out) { answerSpriteAt(in + 4, out); }
static const struct { int cmd, in, out; void (*answer)(const uint8_t*, uint8_t*); } routes[] = {
	{0x0A, 6, 8, answerEdges},
	{0x11, 8, 2, answerRow},
	{0x03, 10, 6, answerSpriteList},
	{0x0B, 6, 6, answerSprite},
};
static int routed = -1;  // index into routes, or -1
static uint8_t routedIn[16], routedOut[16];
static int routedNIn, routedNOut = -1;

// A taped row's fuzz case scripts the chip (dsp4_bridge_script): its status always reads
// ready, data reads come from the script (low byte first, 0 past its end) and writes are
// not fed to the chip. A reset clears it.
static bool scripted;
static uint8_t scriptBytes[8192];
// harness-5: a native site's bytes (src/dsp4.c's whole commands) go to the real stand-in
extern "C" bool difftest_dsp4_native_now(void);
static int scriptLen, scriptPos;

extern "C" void dsp4_bridge_script(const uint16_t* words, int n) {
	if(n > int(sizeof scriptBytes / 2)) n = int(sizeof scriptBytes / 2);
	for(int i = 0; i < n; i++) {
		scriptBytes[2 * i] = uint8_t(words[i]);
		scriptBytes[2 * i + 1] = uint8_t(words[i] >> 8);
	}
	scriptLen = 2 * n;
	scriptPos = 0;
	scripted = true;
	chip.reset();  // the native sites' commands (harness-5) start on an idle chip
	routed = -1;
	routedNOut = -1;
}

extern "C" void dsp4_bridge_reset(void) {
	scripted = false;
	chip.reset();
	routed = -1;
	routedNOut = -1;
}

extern "C" uint8_t dsp4_bridge_read(uint8_t bank, uint16_t adr) {
	const bool data = adr < 0xC000;
	if(scripted && !difftest_dsp4_native_now()) {
		const uint8_t val = !data ? 0x80 : scriptPos < scriptLen ? scriptBytes[scriptPos++] : 0;
		bench_log_dsp4(false, data, bank, adr, val);
		return val;
	}
	uint8_t val = data ? chip.readData() : chip.readStatus();
	if(data && routed >= 0 && routedNOut >= 0) {
		val = routedOut[routedNOut++];
		if(routedNOut == routes[routed].out) routed = -1;
	}
	bench_log_dsp4(false, data, bank, adr, val);
	return val;
}

extern "C" void dsp4_bridge_write(uint8_t bank, uint16_t adr, uint8_t val) {
	const bool data = adr < 0xC000;
	if(scripted && !difftest_dsp4_native_now()) {
		bench_log_dsp4(true, data, bank, adr, val);
		return;
	}
	if(data) {  // writes to the status register are ignored, as on the chip
		const bool wasBusy = chip.m.busy != 0;
		chip.writeData(val);
		if(!wasBusy && chip.m.busy) {
			routed = -1;
			for(int r = 0; r < int(sizeof routes / sizeof routes[0]); r++)
				if((chip.m.command & 31) == routes[r].cmd) routed = r;
			routedNIn = 0;
			routedNOut = -1;
		} else if(routed >= 0 && routedNOut < 0) {
			routedIn[routedNIn++] = val;
			if(routedNIn == routes[routed].in) {
				routes[routed].answer(routedIn, routedOut);
				routedNOut = 0;
			}
		} else if(routed >= 0) {
			routed = -1;  // a write where results were due: not this command's use any more
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

// For the bench's report: whether the chip is running a command, and which (low 5 bits).
extern "C" int dsp4_bridge_busy(void) { return chip.m.busy != 0; }
extern "C" int dsp4_bridge_command(void) { return chip.m.command & 31; }
