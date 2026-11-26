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
#include <vector>

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
// flagEnds: a first word of 0 is the whole answer (dsp4-03c: a hidden sprite)
static const struct { int cmd, in, out; void (*answer)(const uint8_t*, uint8_t*); bool flagEnds; } routes[] = {
	{0x0A, 6, 8, answerEdges, false},
	{0x11, 8, 2, answerRow, false},
	{0x03, 10, 6, answerSpriteList, true},
	{0x0B, 6, 6, answerSprite, true},
};
static int routed = -1;  // index into routes, or -1
static uint8_t routedIn[16], routedOut[16];
static int routedNIn, routedNOut = -1;

// harness-27: the firmware DSP-4 (tools/bench/dsp4_lle.cpp) in place of the clean-room
// chip: data and status go to it, and save states carry its state. Scripted cases (the
// difftest's fuzz) do not touch it.
#ifdef BENCH_LLE
extern "C" bool dsp4_lle_load(const char* path);
extern "C" void dsp4_lle_reset(void);
extern "C" uint8_t dsp4_lle_read(bool data);
extern "C" void dsp4_lle_write(bool data, uint8_t v);
extern "C" int dsp4_lle_state_size(void);
extern "C" void dsp4_lle_save_state(uint8_t* out, int size);
extern "C" void dsp4_lle_load_state(const uint8_t* in, int size);
#endif
static bool lleOn;
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
#ifdef BENCH_LLE
	if(lleOn) dsp4_lle_reset();  // (so does the firmware chip, which answers them under it)
#endif
	routed = -1;
	routedNOut = -1;
}

// harness-26: a firmware tape (refbench --dsp4-tape, 5-byte records: PC low first, value, 1
// for a read) replayed in place of the clean-room chip's answers. Each data read returns the
// tape's next byte, which must be a read; each data write must equal the tape's next byte,
// which must be a write. The tape is consumed in order, never by frame. The status register
// reads ready ($80). The first mismatch stops the replay (dsp4_bridge_tape_failed); the
// chip is not fed while it runs. Scripted cases (the difftest's fuzz) do not touch it.
static std::vector<uint8_t> tapeData;
static size_t tapePos;  // in records
static bool tapeOn, tapeBad;
static uint32_t tapeBadPc;
static uint8_t tapeBadVal;
static bool tapeBadWrite;

// whether the firmware chip answers (harness-29: replays keep its state)
extern "C" bool dsp4_bridge_firmware_on(void) { return lleOn; }
// 1: loaded; 0: the bench was built without bsnes; -1: the file could not be loaded
extern "C" int dsp4_bridge_firmware(const char* path) {
#ifdef BENCH_LLE
	if(!dsp4_lle_load(path)) return -1;
	lleOn = true;
	return 1;
#else
	(void)path;
	return 0;
#endif
}

extern "C" long dsp4_bridge_tape(const char* path) {
	FILE* f = std::fopen(path, "rb");
	if(f == nullptr) return -1;
	std::fseek(f, 0, SEEK_END);
	const long size = std::ftell(f);
	std::rewind(f);
	tapeData.resize(size_t(size));
	const bool ok = std::fread(tapeData.data(), 1, size_t(size), f) == size_t(size);
	std::fclose(f);
	if(!ok) return -1;
	tapePos = 0;
	tapeOn = true;
	tapeBad = false;
	return size / 5;
}
static size_t tapeRecords(void) { return tapeData.size() / 5; }
// the record at the replay's place: its PC, value and direction; false past the end
static bool tapeRecord(size_t i, uint32_t* pc, uint8_t* val, bool* read) {
	if(i >= tapeRecords()) return false;
	const uint8_t* r = &tapeData[5 * i];
	*pc = uint32_t(r[0] | r[1] << 8 | r[2] << 16);
	*val = r[3];
	*read = (r[4] & 1) != 0;
	return true;
}
static void tapeMismatch(uint8_t val, bool write) {
	tapeBad = true;
	tapeOn = false;
	tapeBadPc = bench_cpu_pc();
	tapeBadVal = val;
	tapeBadWrite = write;
}
// After a mismatch: the record it stopped at (index; its PC, value, direction) and the
// bench's access (PC, value, write). False while none.
extern "C" bool dsp4_bridge_tape_failed(long* index, uint32_t* tapePc, uint8_t* tapeVal, int* tapeRead,
                                        uint32_t* pc, uint8_t* val, int* write) {
	if(!tapeBad) return false;
	*index = long(tapePos);
	uint32_t p = 0;
	uint8_t v = 0;
	bool r = false;
	const bool have = tapeRecord(tapePos, &p, &v, &r);
	*tapePc = have ? p : 0xFFFFFFFF;
	*tapeVal = v;
	*tapeRead = have ? r : -1;
	*pc = tapeBadPc;
	*val = tapeBadVal;
	*write = tapeBadWrite;
	return true;
}
extern "C" long dsp4_bridge_tape_used(long* total) {
	*total = long(tapeRecords());
	return long(tapePos);
}

extern "C" void dsp4_bridge_reset(void) {
#ifdef BENCH_LLE
	if(lleOn) dsp4_lle_reset();
#endif
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
#ifdef BENCH_LLE
	if(lleOn) {
		const uint8_t val = dsp4_lle_read(data);
		bench_log_dsp4(false, data, bank, adr, val);
		return val;
	}
#endif
	if(tapeOn) {
		uint8_t val = 0x80;
		if(data) {
			uint32_t p;
			bool read;
			if(!tapeRecord(tapePos, &p, &val, &read) || !read) tapeMismatch(0, false);
			else tapePos++;
		}
		bench_log_dsp4(false, data, bank, adr, val);
		return val;
	}
	uint8_t val = data ? chip.readData() : chip.readStatus();
	if(data && routed >= 0 && routedNOut >= 0) {
		val = routedOut[routedNOut++];
		if(routedNOut == 2 && routes[routed].flagEnds && routedOut[0] == 0 && routedOut[1] == 0) {
			// (dsp4-03c) the clean-room chip shows every sprite: read its other words away
			for(int i = 2; i < routes[routed].out; i++) chip.readData();
			routed = -1;
		} else if(routedNOut == routes[routed].out) routed = -1;
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
#ifdef BENCH_LLE
	if(lleOn) {
		dsp4_lle_write(data, val);
		bench_log_dsp4(true, data, bank, adr, val);
		return;
	}
#endif
	if(tapeOn) {
		if(data) {
			uint32_t p;
			uint8_t v;
			bool read;
			if(!tapeRecord(tapePos, &p, &v, &read) || read || v != val) tapeMismatch(val, true);
			else tapePos++;
		}
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

// dsp4-01t: bytes sent to the chip itself, a scripted case or not (the difftest's priming of
// the road's rounding: a script resets the chip, and its writes would be dropped)
extern "C" void dsp4_bridge_direct(const uint8_t* bytes, int nWrite, int nRead) {
	const bool was = scripted;
	scripted = false;
	for(int i = 0; i < nWrite; i++) dsp4_bridge_write(0x30, 0x8000, bytes[i]);
	for(int i = 0; i < nRead; i++) (void)dsp4_bridge_read(0x30, 0x8000);
	scripted = was;
}

extern "C" int dsp4_bridge_state_size(void) {
#ifdef BENCH_LLE
	if(lleOn) return dsp4_lle_state_size();
#endif
	return int(chip.saveState().size());
}

extern "C" void dsp4_bridge_save_state(uint8_t* out) {
#ifdef BENCH_LLE
	if(lleOn) {
		dsp4_lle_save_state(out, dsp4_lle_state_size());
		return;
	}
#endif
	const std::vector<uint8_t> blob = chip.saveState();
	std::memcpy(out, blob.data(), blob.size());
}

extern "C" void dsp4_bridge_load_state(const uint8_t* data, int size) {
	// harness-36: a replay's chip answers as the chip, not from a script an earlier fuzz case left
	scripted = false;
	routed = -1;
	routedNOut = -1;
#ifdef BENCH_LLE
	if(lleOn) {
		dsp4_lle_load_state(data, size);
		return;
	}
#endif
	if(!chip.loadState(data, size_t(size))) {
		std::fprintf(stderr, "dsp4: save state rejected\n");
		std::abort();  // a silently wrong DSP-4 would poison every comparison
	}
}

// For the bench's report: whether the chip is running a command, and which (low 5 bits).
extern "C" int dsp4_bridge_busy(void) { return chip.m.busy != 0; }
extern "C" int dsp4_bridge_command(void) { return chip.m.command & 31; }
