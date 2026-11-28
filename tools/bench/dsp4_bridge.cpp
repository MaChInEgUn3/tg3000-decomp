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
#include "dsp4_chip.h"
}

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

static dsp4::Dsp4 chip;

// Commands proved on the firmware (refbench) are answered by the native C (src/dsp4.c),
// through its chip model (tools/bench/dsp4_chip.c, `nat`), instead of the clean-room chip.
// Both get every data byte, so both stay in step. When a routed command's input is complete
// (a whole command, or a stream's header or row), the bridge reads the clean-room chip's
// whole answer away itself, by its own count words (its line counts can differ from ours),
// and the game reads ours. An answer the native C does not model (dsp4_chip_unmodelled) is
// served from the clean-room chip's instead.
static Dsp4Chip nat;
// nat starts at power-on (rounding $8000, $8000) even if the bench never calls a reset
static const struct NatPowerOn { NatPowerOn() { dsp4_chip_reset(&nat); } } natPowerOn;
static bool isRouted(int cmd) {
	return cmd == 0x0A || cmd == 0x11 || cmd == 0x03 || cmd == 0x0B || cmd == 0x01 || cmd == 0x07 || cmd == 0x08;
}
static bool isStream(int cmd) { return cmd == 0x01 || cmd == 0x07 || cmd == 0x08; }

// The routing state, saved after the clean-room chip's (fixed size, no pointers).
struct Route {
	int32_t cmd;        // the routed command (low 5 bits), or -1
	uint8_t pass;       // the rest of it goes to the clean-room chip (nat lost it: an $8001)
	uint8_t last;       // the answer being served is the command's last
	uint8_t lastWrite;  // the data byte before (a command word's low byte)
	uint8_t pad;
	uint32_t answers;   // answers given in this command ($08's first is its header's)
	uint16_t len, pos;  // the answer being served, and the game's place in it
	uint8_t buf[DSP4_CHIP_OUT_MAX];
};
static Route route = {-1, 0, 0, 0, 0, 0, 0, 0, {0}};
static unsigned long unmodelledTotal, desyncTotal;
// both answers as read away, before one is chosen (not state: filled and used in one call)
static uint8_t natAnswer[DSP4_CHIP_OUT_MAX], standAnswer[DSP4_CHIP_OUT_MAX];

static void routeClear(void) {
	route.cmd = -1;
	route.pass = route.last = 0;
	route.answers = 0;
	route.len = route.pos = 0;
}

// nat swaps its own rounding in and out of src/dsp4.c's global; the bridge also puts the global
// back, so the native sites that use it never see the bridge's commands
static void natWrite(uint8_t v) {
	int16_t e2, e3;
	dsp4_round_get(&e2, &e3);
	dsp4_chip_write(&nat, v);
	dsp4_round_set(e2, e3);
}
static uint8_t natRead(void) {
	int16_t e2, e3;
	dsp4_round_get(&e2, &e3);
	const uint8_t v = dsp4_chip_read(&nat);
	dsp4_round_set(e2, e3);
	return v;
}

// The clean-room chip's answer, read away into standAnswer by its own framing. A count over
// DSP4_CHIP_LINES is read whole but kept as DSP4_CHIP_LINES lines, its count word saying so.
static uint16_t standWord(int* len, bool keep) {
	const uint8_t lo = chip.readData(), hi = chip.readData();
	if(keep && *len + 2 <= DSP4_CHIP_OUT_MAX) {
		standAnswer[*len] = lo;
		standAnswer[*len + 1] = hi;
		*len += 2;
	}
	return uint16_t(lo | hi << 8);
}
static void standRun(int* len, int words) {  // a count word, then that many entries of `words`
	const unsigned n = standWord(len, false);
	const unsigned kept = n < DSP4_CHIP_LINES ? n : DSP4_CHIP_LINES;
	if(*len + 2 <= DSP4_CHIP_OUT_MAX) {
		standAnswer[*len] = uint8_t(kept);
		standAnswer[*len + 1] = uint8_t(kept >> 8);
		*len += 2;
	}
	for(unsigned i = 0; i < n; i++)
		for(int j = 0; j < words; j++) standWord(len, i < kept);
}
// single: $08's header or last answer, one word
static int standRead(int cmd, bool single) {
	int len = 0;
	int words = 0;
	switch(cmd) {
	case 0x0A: words = 4; break;
	case 0x11: words = 1; break;
	case 0x03:
	case 0x0B: words = 3; break;  // (dsp4-03c) the clean-room chip shows every sprite: 3 words always, as before
	case 0x01: words = 4; break;
	case 0x07: words = 2; break;
	default: words = single ? 1 : 0; break;  // $08
	}
	for(int i = 0; i < words; i++) standWord(&len, true);
	if(cmd == 0x01 || cmd == 0x07) standRun(&len, 3);  // lines: index, $0798 word, $0796 word
	if(cmd == 0x08 && !single) {  // the two roads' windows: (address, word) pairs
		standRun(&len, 2);
		standRun(&len, 2);
	}
	return len;
}

// nat has an answer due (the input is complete): read both answers away, serve nat's, or the
// clean-room chip's if the native C does not model it. `before`: nat's unmodelled count before
// the write.
static void routeAnswer(unsigned long before) {
	int natLen = 0;
	while(dsp4_chip_pending(&nat) != 0 && natLen < DSP4_CHIP_OUT_MAX) natAnswer[natLen++] = natRead();
	const bool last = dsp4_chip_idle(&nat);  // a whole command, or $08's word after $8000
	const bool single = route.cmd == 0x08 && (route.answers == 0 || last);
	const int standLen = standRead(route.cmd, single);
	route.answers++;
	route.last = last;
	const bool own = dsp4_chip_unmodelled(&nat) == before;
	if(!own) unmodelledTotal++;
	route.len = uint16_t(own ? natLen : standLen);
	std::memcpy(route.buf, own ? natAnswer : standAnswer, route.len);
	route.pos = 0;
	if(last && isStream(route.cmd) && chip.m.busy) desyncTotal++;  // $08 ended for us, not for it
}

// A command starts on the clean-room chip (busy from its command word's high byte, `val`):
// nat starts it too, from idle, whatever the command before left it in (it is lost in one it
// does not have, or a multiply skipped)
static void routeStart(uint8_t val) {
	routeClear();
	int16_t e2, e3;  // (as natWrite: the global rounding put back)
	dsp4_round_get(&e2, &e3);
	dsp4_chip_start(&nat, route.lastWrite, val);  // (dsp4-01w: the low byte was written already)
	dsp4_round_set(e2, e3);
	const int cmd = chip.m.command & 31;
	if(isRouted(cmd) && !dsp4_chip_lost(&nat)) route.cmd = cmd;
}

static void routeWrite(uint8_t val) {
	const unsigned long before = dsp4_chip_unmodelled(&nat);
	natWrite(val);
	if(route.cmd < 0 || route.pass) return;
	if(route.pos < route.len) {  // a write where answers were due: not this command's use any more
		if(isStream(route.cmd)) desyncTotal++;
		routeClear();
		return;
	}
	if(dsp4_chip_pending(&nat) != 0) routeAnswer(before);
	else if(dsp4_chip_lost(&nat)) {
		// a road's $8001 (an edge): its framing is not known here, so the clean-room chip
		// answers the rest of the command directly, up to its next command
		if(dsp4_chip_unmodelled(&nat) != before) unmodelledTotal++;
		route.pass = 1;
	} else if(dsp4_chip_idle(&nat)) {  // $01/$07's $8000: the end, no answer
		if(chip.m.busy) desyncTotal++;
		routeClear();
	}
}

// Answers served from the clean-room chip because the native C does not model them, and
// routed stream commands that ended (or broke off) with the clean-room chip still busy.
extern "C" unsigned long dsp4_bridge_unmodelled(void) { return unmodelledTotal; }
extern "C" unsigned long dsp4_bridge_desyncs(void) { return desyncTotal; }

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
	dsp4_chip_reset(&nat);  // (nat mirrors the clean-room chip: power-on rounding too)
	routeClear();
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
	dsp4_chip_reset(&nat);
	routeClear();
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
	uint8_t val;
	if(!data) val = chip.readStatus();
	else if(route.cmd >= 0 && !route.pass && route.pos < route.len) {
		// a routed answer: the clean-room chip's was read away when the input was complete
		val = route.buf[route.pos++];
		if(route.pos == route.len && route.last) routeClear();
	} else val = chip.readData();
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
		// inside a routed stream, nat's framing decides where it ends (a busy flag that drops
		// between rows is not a new command); its end checks the clean-room chip's busy
		const bool inStream = route.cmd >= 0 && isStream(route.cmd) && !route.pass;
		if(!wasBusy && chip.m.busy && !inStream) routeStart(val);
		else routeWrite(val);
		route.lastWrite = val;
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

// The clean-room chip's state, then the routing's (nat, the route with its answer, a tag): a
// save taken mid-row restores the answer being served. The firmware path saves its own only.
static const uint8_t routeTag[4] = {'D', '4', 'R', '1'};
static size_t routeStateSize(void) { return sizeof nat + sizeof route + sizeof routeTag; }

extern "C" int dsp4_bridge_state_size(void) {
#ifdef BENCH_LLE
	if(lleOn) return dsp4_lle_state_size();
#endif
	return int(chip.saveState().size() + routeStateSize());
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
	out += blob.size();
	std::memcpy(out, &nat, sizeof nat);
	std::memcpy(out + sizeof nat, &route, sizeof route);
	std::memcpy(out + sizeof nat + sizeof route, routeTag, sizeof routeTag);
}

extern "C" void dsp4_bridge_load_state(const uint8_t* data, int size) {
	// harness-36: a replay's chip answers as the chip, not from a script an earlier fuzz case left
	scripted = false;
#ifdef BENCH_LLE
	if(lleOn) {
		routeClear();
		dsp4_lle_load_state(data, size);
		return;
	}
#endif
	// the routing's part, if the state has one (a state from before it: the chip's alone)
	size_t own = size_t(size);
	const bool routing = own >= routeStateSize() && std::memcmp(data + own - sizeof routeTag, routeTag, sizeof routeTag) == 0;
	if(routing) own -= routeStateSize();
	if(!chip.loadState(data, own)) {
		std::fprintf(stderr, "dsp4: save state rejected\n");
		std::abort();  // a silently wrong DSP-4 would poison every comparison
	}
	if(routing) {
		std::memcpy(&nat, data + own, sizeof nat);
		std::memcpy(&route, data + own + sizeof nat, sizeof route);
		if(route.len > DSP4_CHIP_OUT_MAX || route.pos > route.len) {
			std::fprintf(stderr, "dsp4: save state's routing rejected\n");
			std::abort();
		}
	} else {
		// no routing saved: nat restarts at power-on (its rounding may then differ until a $0A)
		dsp4_chip_reset(&nat);
		routeClear();
	}
}

// For the bench's report: whether the chip is running a command, and which (low 5 bits).
extern "C" int dsp4_bridge_busy(void) { return chip.m.busy != 0; }
extern "C" int dsp4_bridge_command(void) { return chip.m.command & 31; }
