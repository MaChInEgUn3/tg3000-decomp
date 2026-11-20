// SPDX-License-Identifier: GPL-3.0-or-later
// Reference bench: replays a BizHawk SNES movie on the bsnes core BizHawk 1.9.4 used
// (libsnes, compatibility profile), headless. It answers one question the main bench
// cannot: does a community TAS stay in sync when the emulator is the one it was recorded
// on? With --firmware the DSP-4 runs its real program on the uPD7725 core (a file
// supplied by the owner; never downloaded, never committed). Without it, the DSP-4 is StackBlender's
// clean-room replacement, the same one the main bench uses, so a desync then points at the
// DSP-4 rather than at the emulator's timing. Measuring instrument only: nothing here goes
// into src/ or into a port.
//
//   refbench ROM "Input Log.txt" [--firmware dsp4.rom] [--frames N] [--hashes FILE]
//            [--dump DIR --every N] [--wram-at N FILE] [--offset N] [--p2-pad]
//
// "Input Log.txt" is the file inside the .bk2 (a zip). One log line is one frame.
// --hashes writes "<frame> <FNV-1a 64 of the 128 KB WRAM> <DSP-4 data writes that frame>"
// after every frame; DSP-4 traffic means a race is being drawn, which is how the summary
// line measures how much of a movie is spent racing (a synced TAS races most of the time).
// --offset N plays N idle frames before the log (negative: skips the log's first -N
// lines). --p2-pad connects a joypad in port 2 (BizHawk's default) instead of nothing.
// --bizhawk replays what BizHawk 1.9.4's LibsnesCore does while a movie is recorded
// (deterministic mode): power once more after loading, take a save state then and before
// every frame (bsnes's snes_serialize runs the system to a save point, which moves
// thread synchronisation, so it is part of the timing the movie was recorded with), and
// leave both ports with libsnes's default joypads.
// --coverage FILE OR-merges execution coverage into FILE (and FILE.ram), in the main
// bench's format (tools/bench/coverage.h), for tools/map.py.
// --dsp4-mul FILE checks every DSP-4 command $0000 the game sends against src/dsp4.c's
// dsp4_multiply (the first mismatches go to FILE; totals to stderr).
#include "libsnes.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>
// The catalogue reads the clean-room chip's busy flag and command (a measuring tool only;
// the chip's code is not modified)
#define private public
#include "dsp4/Dsp4.h"
#undef private

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

extern "C" {
#include "png.h"
#include "dsp4.h"
}

namespace SNES {
struct Interface;
Interface* interface();  // creates libsnes's interface object, which the setters fill in
extern unsigned char (*necdsp_hle_read)(bool data);
extern void (*necdsp_hle_write)(unsigned char data);
extern unsigned long necdsp_writes;
extern void (*refbench_exec_hook)(unsigned pc, bool e, bool m, bool x);
extern void (*refbench_read_hook)(unsigned addr);
extern void (*refbench_rw_hook)(unsigned addr, unsigned char value, bool write);
}

// libsnes.cpp calls BizHawk's process wrapper (a Windows IPC layer, not built here).
void pwrap_init() {}

static dsp4::Dsp4 chip;
static unsigned char hleRead(bool data) { return data ? chip.readData() : chip.readStatus(); }
static void hleWrite(unsigned char v) { chip.writeData(v); }

static uint32_t lastPc;   // the instruction being executed (from the exec hook)
static bool coverageOn;

// ---- coverage, same file format and flags as tools/bench/coverage.c ----
enum { COV_START = 0x01, COV_CODE = 0x02, COV_READ = 0x04, COV_M8X8 = 0x08, COV_M8X16 = 0x10,
       COV_M16X8 = 0x20, COV_M16X16 = 0x40, COV_EMU = 0x80 };
static const uint32_t kRomSize = 0x100000;
static std::vector<uint8_t> covFlags(kRomSize), covRamExec(1u << 21), romImage;
static unsigned long long covInsns, covRamInsns;

static const int8_t kOpLength[256] = {
  2, 2, 2, 2, 2, 2, 2, 2, 1,-1, 1, 1, 3, 3, 3, 4,  2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
  3, 2, 4, 2, 2, 2, 2, 2, 1,-1, 1, 1, 3, 3, 3, 4,  2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
  1, 2, 2, 2, 3, 2, 2, 2, 1,-1, 1, 1, 3, 3, 3, 4,  2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4,
  1, 2, 3, 2, 2, 2, 2, 2, 1,-1, 1, 1, 3, 3, 3, 4,  2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
  2, 2, 3, 2, 2, 2, 2, 2, 1,-1, 1, 1, 3, 3, 3, 4,  2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
 -2, 2,-2, 2, 2, 2, 2, 2, 1,-1, 1, 1, 3, 3, 3, 4,  2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
 -2, 2, 2, 2, 2, 2, 2, 2, 1,-1, 1, 1, 3, 3, 3, 4,  2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
 -2, 2, 2, 2, 2, 2, 2, 2, 1,-1, 1, 1, 3, 3, 3, 4,  2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
};

// LoROM ROM offset of a bus address, or -1 (WRAM, I/O, the DSP-4 at $30-$3F:8000-FFFF).
static int32_t romOffset(uint32_t a) {
  uint8_t b = (a >> 16) & 0x7F;
  uint16_t adr = uint16_t(a);
  if(b >= 0x7E) return -1;
  if(adr < 0x8000 && b < 0x40) return -1;
  if(b >= 0x30 && b < 0x40) return -1;
  return int32_t(((uint32_t(b) << 15) | (adr & 0x7FFF)) & (kRomSize - 1));
}

static void covExec(unsigned pc, bool e, bool m, bool x) {
  covInsns++;
  int32_t off = romOffset(pc);
  if(off < 0) {
    covRamInsns++;
    uint32_t a = pc & 0xFFFFFF;
    covRamExec[a >> 3] |= uint8_t(1 << (a & 7));
    return;
  }
  bool m8 = e || m, x8 = e || x;
  int len = kOpLength[romImage[off]];
  if(len == -1) len = m8 ? 2 : 3;
  if(len == -2) len = x8 ? 2 : 3;
  uint8_t state = m8 ? (x8 ? COV_M8X8 : COV_M8X16) : (x8 ? COV_M16X8 : COV_M16X16);
  covFlags[off] |= COV_START | COV_CODE | state | (e ? COV_EMU : 0);
  for(int i = 1; i < len; i++) {  // operands stay in the bank
    int32_t o = romOffset((pc & 0xFF0000) | uint16_t(pc + i));
    if(o >= 0) covFlags[o] |= COV_CODE;
  }
}

static void execHook(unsigned pc, bool e, bool m, bool x) {
  lastPc = pc & 0xFFFFFF;
  if(coverageOn) covExec(pc, e, m, x);
}

static void covRead(unsigned addr) {
  int32_t off = romOffset(addr);
  if(off >= 0) covFlags[off] |= COV_READ;
}

static bool covMergeSave(const std::string& path) {
  std::vector<uint8_t> old(kRomSize);
  if(FILE* f = std::fopen(path.c_str(), "rb")) {
    if(std::fread(old.data(), 1, kRomSize, f) == kRomSize)
      for(uint32_t i = 0; i < kRomSize; i++) covFlags[i] |= old[i];
    std::fclose(f);
  }
  FILE* f = std::fopen(path.c_str(), "wb");
  if(f == nullptr) { std::perror(path.c_str()); return false; }
  bool ok = std::fwrite(covFlags.data(), 1, kRomSize, f) == kRomSize;
  ok = std::fclose(f) == 0 && ok;
  std::string ramPath = path + ".ram";
  if(FILE* r = std::fopen(ramPath.c_str(), "r")) {
    unsigned b, a;
    while(std::fscanf(r, "%x:%x", &b, &a) == 2) {
      uint32_t x = (b << 16) | a;
      covRamExec[x >> 3] |= uint8_t(1 << (x & 7));
    }
    std::fclose(r);
  }
  FILE* r = std::fopen(ramPath.c_str(), "w");
  if(r == nullptr) { std::perror(ramPath.c_str()); return false; }
  for(uint32_t x = 0; x < (1u << 24); x++)
    if(covRamExec[x >> 3] & (1 << (x & 7))) std::fprintf(r, "%02X:%04X\n", x >> 16, x & 0xFFFF);
  std::fclose(r);
  return ok;
}

// ---- DSP-4 oracle: every command $0000 the game sends (an "LDA #$0000 / STA $30:8000"
// at the instruction) is captured with its two operands and the result bytes the chip
// returns, and checked against the native dsp4_multiply (src/dsp4.c) ----
// After the command word and two operands (6 bytes written), the product's 4 bytes go out
// one per data access: a read takes the byte, a write skips it (the chip's documented
// behaviour, checked here). slot = how many of the 4 are done; read[i] says which were read.
struct MulCapture { bool active; uint32_t pc; uint8_t w[6]; int nw; uint8_t r[4]; bool read[4]; int slot; };
static MulCapture cap;
static unsigned long mulCases, mulMatch, mulMismatch, mulSkipped;
static FILE* mulLog;

static bool isDsp4Data(unsigned a) {
  uint8_t b = (a >> 16) & 0x7F;
  uint16_t adr = uint16_t(a);
  return b >= 0x30 && b < 0x40 && adr >= 0x8000 && adr < 0xC000;
}

static bool isCommand0Site(uint32_t pc) {
  int32_t off = romOffset(pc);
  if(off < 3) return false;
  const uint8_t* p = &romImage[off];
  return p[0] == 0x8F && p[1] == 0x00 && p[2] == 0x80 && p[3] == 0x30 && p[-3] == 0xA9 && p[-2] == 0 && p[-1] == 0;
}

static void finishCapture() {
  int reads = 0;
  for(int i = 0; i < cap.slot; i++) reads += cap.read[i];
  if(cap.active && cap.nw == 6 && reads > 0) {
    int16_t a = int16_t(cap.w[2] | cap.w[3] << 8), b = int16_t(cap.w[4] | cap.w[5] << 8);
    uint32_t p = uint32_t(dsp4_multiply(a, b));
    bool ok = true;
    for(int i = 0; i < cap.slot; i++) if(cap.read[i] && cap.r[i] != uint8_t(p >> (8 * i))) ok = false;
    mulCases++;
    if(ok) mulMatch++;
    else {
      mulMismatch++;
      if(mulLog && mulMismatch <= 50) {
        std::fprintf(mulLog, "mismatch at $%06X: %d * %d: chip", cap.pc, a, b);
        for(int i = 0; i < cap.slot; i++) {
          if(cap.read[i]) std::fprintf(mulLog, " %02X", cap.r[i]);
          else std::fprintf(mulLog, " --");
        }
        std::fprintf(mulLog, ", C %08X\n", p);
      }
    }
    for(int i = 0; i < cap.slot; i++) if(!cap.read[i]) { mulSkipped++; break; }
  }
  cap.active = false;
}

static bool catalogueOn;
static void catalogueAccess(unsigned addr, unsigned char v, bool write);

static void rwHook(unsigned addr, unsigned char v, bool write) {
  if(!isDsp4Data(addr)) return;
  if(catalogueOn) catalogueAccess(addr, v, write);
  if(write && isCommand0Site(lastPc) && !(cap.active && cap.nw == 1)) {  // a new command $0000
    finishCapture();
    cap = MulCapture{};
    cap.active = true;
    cap.pc = lastPc;
  }
  if(!cap.active) return;
  if(cap.nw < 6) {  // command word and operands
    if(write) cap.w[cap.nw++] = v;
    else finishCapture();  // not a multiply as the game writes it
    return;
  }
  if(cap.slot < 4) {  // the product: read, or skipped by a write
    cap.read[cap.slot] = !write;
    cap.r[cap.slot] = v;
    if(++cap.slot == 4) finishCapture();
    return;
  }
  finishCapture();
}

// ---- DSP-4 catalogue: the clean-room chip runs as a shadow of the firmware, fed the same
// data writes; its busy flag gives command boundaries, and each byte the firmware returns
// is compared with the shadow's ----
struct CmdStats {
  unsigned long count, mismatched, readBytes, wrongBytes;
  int minIn, maxIn, minOut, maxOut;
  uint32_t writers[8], readers[8];
  int nWriters, nReaders;
  long firstBadFrame;
};
static dsp4::Dsp4 shadow;
static CmdStats cmdStats[32];
static long curFrame;
static int curCmd = -1, curIn, curOut;
static bool curBad;
static long firstMismatchFrame = -1;
static int firstMismatchCmd = -1;
static uint32_t firstMismatchPc;
static unsigned long idleReads;

static void notePc(uint32_t* list, int* n, uint32_t pc) {
  for(int i = 0; i < *n; i++) if(list[i] == pc) return;
  if(*n < 8) list[(*n)++] = pc;
}

static void endCommand() {
  if(curCmd < 0) return;
  CmdStats& c = cmdStats[curCmd];
  c.count++;
  if(curBad) c.mismatched++;
  if(c.count == 1 || curIn < c.minIn) c.minIn = curIn;
  if(curIn > c.maxIn) c.maxIn = curIn;
  if(c.count == 1 || curOut < c.minOut) c.minOut = curOut;
  if(curOut > c.maxOut) c.maxOut = curOut;
  curCmd = -1;
}

// ---- one command's runs, as the game sees them: from the command word (boundary from the
// shadow, valid while the shadow agrees for that command), every byte the game writes, then
// every byte it reads from the firmware, until it writes again (outputs only: the chip's
// memory and program are never read) ----
static int cmdlogCmd = -1;
static FILE* cmdlog;
static bool cmdlogOn;
static uint8_t cmdlogIn[64], cmdlogOut[64];
static int cmdlogNIn, cmdlogNOut;
static unsigned long cmdlogRuns, cmdlogWrong, cmdlogOther;
static uint32_t cmdlogPc;  // where the command word was written
static void cmdlogFlush() {
  if(!cmdlogOn) return;
  // commands with a native function: check it on this run (src/dsp4.c)
  if(cmdlogCmd == 0x0A && cmdlogNIn == 6 && cmdlogNOut >= 8) {
    int16_t mine[4];
    dsp4_edges(uint16_t(cmdlogIn[0] | cmdlogIn[1] << 8), uint16_t(cmdlogIn[2] | cmdlogIn[3] << 8),
               uint16_t(cmdlogIn[4] | cmdlogIn[5] << 8), mine);
    cmdlogRuns++;
    for(int j = 0; j < 4; j++)
      if(uint16_t(mine[j]) != uint16_t(cmdlogOut[2 * j] | cmdlogOut[2 * j + 1] << 8)) { cmdlogWrong++; break; }
  }
  if(cmdlogCmd == 0x11 && cmdlogNIn == 8 && cmdlogNOut >= 2) {
    int16_t edge[4];
    for(int j = 0; j < 4; j++) edge[j] = int16_t(cmdlogIn[2 * j] | cmdlogIn[2 * j + 1] << 8);
    cmdlogRuns++;
    if(dsp4_row(edge) != uint16_t(cmdlogOut[0] | cmdlogOut[1] << 8)) cmdlogWrong++;
  }
  // $03 (its word, then a $0B entry) and $0B: the entry is the last six bytes in; the flag
  // is always read, the two words only when it is not 0. Only the game's sprite entries
  // (their command words written at $82:CBE1 and $82:D8FA) are checked: elsewhere the
  // shadow sometimes takes another command's data word $0003 or $000B for a command.
  const bool sprite = (cmdlogCmd == 0x03 && cmdlogPc == 0x82CBE1) || (cmdlogCmd == 0x0B && cmdlogPc == 0x82D8FA);
  if((cmdlogCmd == 0x03 || cmdlogCmd == 0x0B) && !sprite) cmdlogOther++;
  if(sprite && ((cmdlogCmd == 0x03 && cmdlogNIn == 10) || (cmdlogCmd == 0x0B && cmdlogNIn == 6))) {
    const uint8_t* e = cmdlogIn + cmdlogNIn - 6;
    uint16_t mine[2];
    const uint16_t flag = dsp4_sprite(uint16_t(e[0] | e[1] << 8), uint16_t(e[2] | e[3] << 8), uint16_t(e[4] | e[5] << 8), mine);
    cmdlogRuns++;
    bool same = cmdlogNOut >= 2 && flag == uint16_t(cmdlogOut[0] | cmdlogOut[1] << 8);
    for(int j = 0; same && j < 2 && cmdlogNOut >= 4 + 2 * j; j++)
      same = mine[j] == uint16_t(cmdlogOut[2 + 2 * j] | cmdlogOut[3 + 2 * j] << 8);
    if(!same) cmdlogWrong++;
  }
  for(int i = 0; i < cmdlogNIn; i++) std::fprintf(cmdlog, "%02X", cmdlogIn[i]);
  std::fprintf(cmdlog, " ");
  for(int i = 0; i < cmdlogNOut; i++) std::fprintf(cmdlog, "%02X", cmdlogOut[i]);
  std::fprintf(cmdlog, "\n");
  cmdlogOn = false;
}

static void catalogueAccess(unsigned addr, unsigned char v, bool write) {
  bool wasBusy = shadow.m.busy != 0;
  if(cmdlog != nullptr) {
    if(write && cmdlogOn && cmdlogNOut > 0) cmdlogFlush();
    else if(write && cmdlogOn && cmdlogNIn < 64) cmdlogIn[cmdlogNIn++] = v;
    else if(!write && cmdlogOn && cmdlogNOut < 64) cmdlogOut[cmdlogNOut++] = v;
  }
  if(write) {
    shadow.writeData(v);
    if(!wasBusy && shadow.m.busy) {  // a command word just completed
      endCommand();
      curCmd = shadow.m.command & 31;
      curIn = curOut = 0;
      if(cmdlog != nullptr) {
        cmdlogFlush();
        if(curCmd == cmdlogCmd) { cmdlogOn = true; cmdlogNIn = cmdlogNOut = 0; cmdlogPc = lastPc; }
      }
      curBad = false;
      notePc(cmdStats[curCmd].writers, &cmdStats[curCmd].nWriters, lastPc);
    } else if(curCmd >= 0) {
      curIn++;
      notePc(cmdStats[curCmd].writers, &cmdStats[curCmd].nWriters, lastPc);
    }
  } else {
    uint8_t mine = shadow.readData();
    if(curCmd < 0) idleReads++;
    else {
      CmdStats& c = cmdStats[curCmd];
      curOut++;
      c.readBytes++;
      notePc(c.readers, &c.nReaders, lastPc);
      if(mine != v) {
        c.wrongBytes++;
        if(!curBad && c.firstBadFrame == 0) c.firstBadFrame = curFrame;
        curBad = true;
        if(firstMismatchFrame < 0) { firstMismatchFrame = curFrame; firstMismatchCmd = curCmd; firstMismatchPc = lastPc; }
      }
    }
  }
  if(wasBusy && !shadow.m.busy) endCommand();
  (void)addr;
}

static void writeCatalogue(const std::string& path) {
  endCommand();
  FILE* f = std::fopen(path.c_str(), "w");
  if(f == nullptr) { std::perror(path.c_str()); return; }
  std::fprintf(f, "# cmd  count  mismatched  bytes-read  wrong-bytes  in-bytes  out-bytes  first-bad-frame  writers / readers (instruction PCs)\n");
  for(int k = 0; k < 32; k++) {
    CmdStats& c = cmdStats[k];
    if(c.count == 0) continue;
    std::fprintf(f, "$%02X %9lu %9lu %11lu %11lu  %d-%d  %d-%d  %ld  w:", k, c.count, c.mismatched, c.readBytes, c.wrongBytes,
                 c.minIn, c.maxIn, c.minOut, c.maxOut, c.firstBadFrame);
    for(int i = 0; i < c.nWriters; i++) std::fprintf(f, " %06X", c.writers[i]);
    std::fprintf(f, " r:");
    for(int i = 0; i < c.nReaders; i++) std::fprintf(f, " %06X", c.readers[i]);
    std::fprintf(f, "\n");
  }
  std::fprintf(f, "# reads with no command running: %lu\n", idleReads);
  if(firstMismatchFrame >= 0)
    std::fprintf(f, "# first mismatch: frame %ld, command $%02X, at $%06X\n", firstMismatchFrame, firstMismatchCmd, firstMismatchPc);
  std::fclose(f);
}

static std::string firmwarePath;
static uint16_t pad;  // libsnes button ids as bits
static std::vector<uint32_t> frameRgb;
static unsigned frameW, frameH;

static void* allocShared(const char*, size_t amt) { return calloc(1, amt); }
static void freeShared(void* p) { free(p); }
static void videoRefresh(const uint32_t* data, unsigned w, unsigned h) {
  frameW = w;
  frameH = h;
  // libsnes hands 1024-pixel rows (interlace layout); keep what is shown
  frameRgb.assign(data, data + 1024 * h);
}
static void audioSample(uint16_t, uint16_t) {}
static void inputPoll(void) {}
static int16_t inputState(unsigned port, unsigned device, unsigned, unsigned id) {
  if(port != 0 || device != SNES_DEVICE_JOYPAD) return 0;
  return (pad >> id) & 1;
}
static const char* pathRequest(int, const char* hint) {
  static std::string out;
  out = (!firmwarePath.empty() && std::strstr(hint, "dsp4") != nullptr) ? firmwarePath : std::string(hint);
  return out.c_str();
}

// BizHawk's SNES log key after the "|..|" console field: Up Down Left Right Select Start
// Y B X A L R. libsnes ids: B=0 Y=1 Select=2 Start=3 Up=4 Down=5 Left=6 Right=7 A=8 X=9 L=10 R=11.
static const int kLogToId[12] = {4, 5, 6, 7, 2, 3, 1, 0, 9, 8, 10, 11};

static bool parseLine(const char* line, uint16_t* buttons, bool* reset, bool* power) {
  if(line[0] != '|') return false;
  const char* p1 = std::strchr(line + 1, '|');
  if(p1 == nullptr) return false;
  *reset = line[1] != '.';
  *power = line[2] != '.';
  uint16_t b = 0;
  for(int i = 0; i < 12 && p1[1 + i] && p1[1 + i] != '|'; i++)
    if(p1[1 + i] != '.') b |= uint16_t(1u << kLogToId[i]);
  *buttons = b;
  return true;
}

static uint64_t wramHash(const uint8_t* ram, unsigned size) {
  uint64_t h = 1469598103934665603ull;
  for(unsigned i = 0; i < size; i++) { h ^= ram[i]; h *= 1099511628211ull; }
  return h;
}

static void dumpFrame(const char* dir, long frame) {
  if(frameRgb.empty()) return;
  unsigned w = frameW, h = frameH;
  std::vector<uint8_t> rgb(size_t(w) * h * 3);
  for(unsigned y = 0; y < h; y++)
    for(unsigned x = 0; x < w; x++) {
      uint32_t c = frameRgb[size_t(y) * 1024 + x];
      uint8_t* o = &rgb[(size_t(y) * w + x) * 3];
      o[0] = uint8_t(c >> 16); o[1] = uint8_t(c >> 8); o[2] = uint8_t(c);
    }
  char path[1024];
  std::snprintf(path, sizeof path, "%s/ref_%06ld.png", dir, frame);
  png_write_rgb(path, rgb.data(), int(w), int(h));
}

int main(int argc, char** argv) {
  if(argc < 3) {
    std::fprintf(stderr, "usage: %s ROM \"Input Log.txt\" [--firmware FILE] [--frames N] [--hashes FILE] [--dump DIR --every N] [--wram-at N FILE]\n", argv[0]);
    return 2;
  }
  long maxFrames = -1, every = 0, wramAt = -1, offset = 0;
  bool p2pad = false, bizhawk = false;
  std::string coveragePath, mulPath, cataloguePath, cmdlogPath;
  const char* dumpDir = nullptr;
  const char* wramFile = nullptr;
  FILE* hashes = nullptr;
  for(int i = 3; i < argc; i++) {
    if(!std::strcmp(argv[i], "--firmware") && i + 1 < argc) firmwarePath = argv[++i];
    else if(!std::strcmp(argv[i], "--frames") && i + 1 < argc) maxFrames = std::atol(argv[++i]);
    else if(!std::strcmp(argv[i], "--hashes") && i + 1 < argc) hashes = std::fopen(argv[++i], "w");
    else if(!std::strcmp(argv[i], "--dump") && i + 1 < argc) dumpDir = argv[++i];
    else if(!std::strcmp(argv[i], "--every") && i + 1 < argc) every = std::atol(argv[++i]);
    else if(!std::strcmp(argv[i], "--offset") && i + 1 < argc) offset = std::atol(argv[++i]);
    else if(!std::strcmp(argv[i], "--p2-pad")) p2pad = true;
    else if(!std::strcmp(argv[i], "--bizhawk")) bizhawk = true;
    else if(!std::strcmp(argv[i], "--coverage") && i + 1 < argc) coveragePath = argv[++i];
    else if(!std::strcmp(argv[i], "--dsp4-mul") && i + 1 < argc) mulPath = argv[++i];
    else if(!std::strcmp(argv[i], "--dsp4-catalogue") && i + 1 < argc) cataloguePath = argv[++i];
    else if(!std::strcmp(argv[i], "--dsp4-cmdlog") && i + 2 < argc) {
      cmdlogCmd = (int)std::strtol(argv[++i], nullptr, 16);
      cmdlogPath = argv[++i];
    }
    else if(!std::strcmp(argv[i], "--wram-at") && i + 2 < argc) { wramAt = std::atol(argv[++i]); wramFile = argv[++i]; }
    else { std::fprintf(stderr, "unknown option %s\n", argv[i]); return 2; }
  }

  FILE* f = std::fopen(argv[1], "rb");
  if(f == nullptr) { std::perror(argv[1]); return 1; }
  std::vector<uint8_t> rom;
  for(int c; (c = std::fgetc(f)) != EOF;) rom.push_back(uint8_t(c));
  std::fclose(f);
  romImage = rom;
  romImage.resize(kRomSize);
  coverageOn = !coveragePath.empty();
  SNES::refbench_exec_hook = execHook;
  if(coverageOn) SNES::refbench_read_hook = covRead;
  if(!mulPath.empty()) mulLog = std::fopen(mulPath.c_str(), "w");
  if(!cmdlogPath.empty()) {  // needs the shadow for the boundaries
    cmdlog = std::fopen(cmdlogPath.c_str(), "w");
    if(cataloguePath.empty()) cataloguePath = cmdlogPath + ".catalogue";
  }
  catalogueOn = !cataloguePath.empty();
  if(!mulPath.empty() || catalogueOn) SNES::refbench_rw_hook = rwHook;
  FILE* log = std::fopen(argv[2], "r");
  if(log == nullptr) { std::perror(argv[2]); return 1; }

  if(firmwarePath.empty()) {
    SNES::necdsp_hle_read = hleRead;
    SNES::necdsp_hle_write = hleWrite;
    chip.reset();
  }
  SNES::interface();
  snes_set_allocSharedMemory(allocShared);
  snes_set_freeSharedMemory(freeShared);
  snes_set_video_refresh(videoRefresh);
  snes_set_audio_sample(audioSample);
  snes_set_input_poll(inputPoll);
  snes_set_input_state(inputState);
  snes_set_path_request(pathRequest);
  snes_init();
  {  // libsnes looks colours up by (brightness << 15 | BGR555); BizHawk's frontend fills this
    std::vector<uint32_t> lut(16 * 32768);
    for(uint32_t l = 0; l < 16; l++)
      for(uint32_t c = 0; c < 32768; c++) {
        uint32_t r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
        auto scale = [l](uint32_t v) { return (v * 255 / 31) * l / 15; };
        lut[l << 15 | c] = scale(r) << 16 | scale(g) << 8 | scale(b);
      }
    snes_set_color_lut(lut.data());
  }
  if(!bizhawk) {
    snes_set_controller_port_device(false, SNES_DEVICE_JOYPAD);
    snes_set_controller_port_device(true, p2pad ? SNES_DEVICE_JOYPAD : SNES_DEVICE_NONE);
  }
  if(!snes_load_cartridge_normal(nullptr, rom.data(), unsigned(rom.size()))) {
    std::fprintf(stderr, "cartridge load failed\n");
    return 1;
  }
  while(snes_poll_message() >= 0) {  // e.g. "NEC DSP firmware ... is missing"
    char msg[1024] = "";
    snes_dequeue_message(msg);
    std::fprintf(stderr, "bsnes: %s\n", msg);
  }
  std::fprintf(stderr, "DSP-4: %s\n", firmwarePath.empty() ? "clean-room replacement (no firmware)" : firmwarePath.c_str());

  std::vector<uint8_t> state;
  auto saveState = [&state]() {
    state.resize(snes_serialize_size());
    snes_serialize(state.data(), unsigned(state.size()));
  };
  if(bizhawk) {
    snes_power();
    saveState();  // BizHawk's frame-0 state
  }
  uint8_t* wram = snes_get_memory_data(SNES_MEMORY_WRAM);
  unsigned wramSize = snes_get_memory_size(SNES_MEMORY_WRAM);
  char line[256];
  long frame = 0, racing = 0, idle = offset > 0 ? offset : 0, skip = offset < 0 ? -offset : 0;
  unsigned long writesBefore = SNES::necdsp_writes;
  while(maxFrames < 0 || frame < maxFrames) {
    bool reset = false, power = false;
    if(idle > 0) {
      idle--;
      pad = 0;
    } else {
      if(!std::fgets(line, sizeof line, log)) break;
      if(!parseLine(line, &pad, &reset, &power)) continue;  // LogKey and blank lines
      if(skip > 0) { skip--; continue; }
    }
    if(bizhawk) saveState();
    if(bizhawk) {  // BizHawk's order: reset, then power
      if(reset) snes_reset();
      if(power) snes_power();
    } else if(power) snes_power();
    else if(reset) snes_reset();
    snes_run();
    frame++;
    curFrame = frame;
    unsigned long writes = SNES::necdsp_writes - writesBefore;
    writesBefore = SNES::necdsp_writes;
    if(writes > 0) racing++;
    if(hashes) std::fprintf(hashes, "%ld %016llx %lu\n", frame, (unsigned long long)wramHash(wram, wramSize), writes);
    if(dumpDir && every > 0 && frame % every == 0) dumpFrame(dumpDir, frame);
    if(frame == wramAt && wramFile) {
      FILE* w = std::fopen(wramFile, "wb");
      if(w) { std::fwrite(wram, 1, wramSize, w); std::fclose(w); }
    }
  }
  std::fclose(log);
  if(hashes) std::fclose(hashes);
  if(catalogueOn) writeCatalogue(cataloguePath);
  if(cmdlog != nullptr) {
    cmdlogFlush();
    std::fclose(cmdlog);
    if(cmdlogRuns) std::fprintf(stderr, "dsp4 command $%02X: %lu runs checked against src/dsp4.c, %lu differ\n", cmdlogCmd,
                                cmdlogRuns, cmdlogWrong);
    if(cmdlogOther) std::fprintf(stderr, "dsp4 command $%02X: %lu runs from other writers (not checked)\n", cmdlogCmd, cmdlogOther);
  }
  if(!mulPath.empty()) {
    finishCapture();
    std::fprintf(stderr, "DSP-4 command $0000: %lu cases, %lu match dsp4_multiply, %lu differ (%lu with skipped bytes)\n",
                 mulCases, mulMatch, mulMismatch, mulSkipped);
    if(mulLog) std::fclose(mulLog);
  }
  if(!coveragePath.empty()) {
    if(!covMergeSave(coveragePath)) return 1;
    unsigned code = 0, starts = 0, read = 0;
    for(uint32_t i = 0; i < kRomSize; i++) {
      if(covFlags[i] & COV_CODE) code++;
      if(covFlags[i] & COV_START) starts++;
      if((covFlags[i] & COV_READ) && !(covFlags[i] & COV_CODE)) read++;
    }
    std::fprintf(stderr, "coverage (merged): %llu instructions (%llu outside ROM); %u code bytes, %u instruction starts, %u data bytes read\n",
                 covInsns, covRamInsns, code, starts, read);
  }
  std::fprintf(stderr, "frames=%ld racing=%ld (%.1f%%)\n", frame, racing, frame ? 100.0 * racing / frame : 0.0);
  return 0;
}
