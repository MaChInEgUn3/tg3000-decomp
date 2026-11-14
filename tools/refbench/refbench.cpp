// SPDX-License-Identifier: GPL-3.0-or-later
// Reference bench: replays a BizHawk SNES movie on the bsnes core BizHawk 1.9.4 used
// (libsnes, compatibility profile), headless. It answers one question the main bench
// cannot: does a community TAS stay in sync when the emulator is the one it was recorded
// on? With --firmware the DSP-4 runs its real program on the uPD7725 core (the owner's
// own dump; never downloaded, never committed). Without it, the DSP-4 is StackBlender's
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
#include "libsnes.hpp"

#include "dsp4/Dsp4.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

extern "C" {
#include "png.h"
}

namespace SNES {
struct Interface;
Interface* interface();  // creates libsnes's interface object, which the setters fill in
extern unsigned char (*necdsp_hle_read)(bool data);
extern void (*necdsp_hle_write)(unsigned char data);
extern unsigned long necdsp_writes;
}

// libsnes.cpp calls BizHawk's process wrapper (a Windows IPC layer, not built here).
void pwrap_init() {}

static dsp4::Dsp4 chip;
static unsigned char hleRead(bool data) { return data ? chip.readData() : chip.readStatus(); }
static void hleWrite(unsigned char v) { chip.writeData(v); }

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
  bool p2pad = false;
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
    else if(!std::strcmp(argv[i], "--wram-at") && i + 2 < argc) { wramAt = std::atol(argv[++i]); wramFile = argv[++i]; }
    else { std::fprintf(stderr, "unknown option %s\n", argv[i]); return 2; }
  }

  FILE* f = std::fopen(argv[1], "rb");
  if(f == nullptr) { std::perror(argv[1]); return 1; }
  std::vector<uint8_t> rom;
  for(int c; (c = std::fgetc(f)) != EOF;) rom.push_back(uint8_t(c));
  std::fclose(f);
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
  snes_set_controller_port_device(false, SNES_DEVICE_JOYPAD);
  snes_set_controller_port_device(true, p2pad ? SNES_DEVICE_JOYPAD : SNES_DEVICE_NONE);
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
    if(power) snes_power();
    else if(reset) snes_reset();
    snes_run();
    frame++;
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
  std::fprintf(stderr, "frames=%ld racing=%ld (%.1f%%)\n", frame, racing, frame ? 100.0 * racing / frame : 0.0);
  return 0;
}
