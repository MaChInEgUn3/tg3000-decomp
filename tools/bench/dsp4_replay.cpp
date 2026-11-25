// SPDX-License-Identifier: GPL-3.0-or-later
// dsp4-09b: replay the game's $09 sprite streams from a refbench --dsp4-tape capture into
// the firmware DSP-4 (harness-27's chip), alone, without the commands the game sent before
// them in its frame (the road commands), and compare every byte the game read.
//   build/bench/dsp4_replay ext/firmware/dsp4.rom tape.bin
// Outputs only: the tape holds what the game wrote and read; the chip is driven as the game
// drives it. A stream runs from its $0009 word (written at $82:CC8F) to the $0006 word
// (written at $82:AE1C) and the 16 words the chip answers it with.
//   fresh: each stream on a chip just powered on (its memory zero);
//   chained: the streams back to back on one chip, with nothing between them;
//   frame: each stream on a chip just powered on, from its frame's road command word
//   ($000F at $85:B536, or $0001 at $85:B53B) on, everything the game sent in between
//   included (dsp4-01a's run start);
//   frames N: the same from the road command N frames back (N = 2, 3, 5);
//   frame + $0A (dsp4-01b): the frame from its road command, after the last $000A command the
//   game sent before it (its words and reads), and again with that $0A's near row's bit 7
//   flipped (the one input bit found to matter).
// And the bit's effect (dsp4-01c): over the first 400 frames with a $0A, which reads change
// when the bit flips, and by how much; and the near rows the $0A at $81:F90E sends.
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>
#include <map>
extern "C" {
bool dsp4_lle_load(const char* path);
uint8_t dsp4_lle_read(bool data);
void dsp4_lle_write(bool data, uint8_t v);
int dsp4_lle_state_size(void);
void dsp4_lle_save_state(uint8_t* out, int size);
void dsp4_lle_load_state(const uint8_t* in, int size);
}
#include <cstdlib>

struct Rec { uint32_t pc; uint8_t v; bool read; };
static const uint32_t OPEN = 0x82CC8F, CLOSE = 0x82AE1C;

int main(int argc, char** argv) {
  if(argc < 3 || !dsp4_lle_load(argv[1])) { std::fprintf(stderr, "usage: %s dsp4.rom tape.bin\n", argv[0]); return 2; }
  FILE* f = std::fopen(argv[2], "rb");
  if(f == nullptr) return 2;
  std::vector<Rec> tape;
  uint8_t b[5];
  while(std::fread(b, 1, 5, f) == 5) tape.push_back({uint32_t(b[0] | b[1] << 8 | b[2] << 16), b[3], (b[4] & 1) != 0});
  std::fclose(f);
  // the streams: [start, end) in the tape
  std::vector<std::pair<size_t, size_t>> streams;
  for(size_t i = 0; i + 1 < tape.size(); i++) {
    if(tape[i].read || tape[i].pc != OPEN || tape[i].v != 0x09 || tape[i + 1].v != 0x00) continue;
    size_t j = i + 2;
    while(j + 1 < tape.size() && !(!tape[j].read && tape[j].pc == CLOSE)) j++;
    if(j + 2 + 32 > tape.size()) break;
    streams.push_back({i, j + 2 + 32});  // the $0006 word, then its 16 words
    i = j;
  }
  const int size = dsp4_lle_state_size();
  std::vector<uint8_t> power(size);
  dsp4_lle_save_state(power.data(), size);
  {  // whole: the tape from its first byte, on a chip just powered on
    dsp4_lle_load_state(power.data(), size);
    long bad = 0, n = 0, firstBad = -1;
    std::map<uint32_t, long> byPc;
    for(size_t k = 0; k < tape.size(); k++) {
      if(!tape[k].read) { dsp4_lle_write(true, tape[k].v); continue; }
      n++;
      if(dsp4_lle_read(true) != tape[k].v) { bad++; byPc[tape[k].pc]++; if(firstBad < 0) firstBad = long(k); }
    }
    std::printf("whole: %ld of %ld bytes read differ (first at tape record %ld)\n", bad, n, firstBad);
    int shown = 0;
    for(const auto& p : byPc) if(shown++ < 12) std::printf("    differing reads at $%06X: %ld\n", p.first, p.second);
  }
  // the history a frame needs: for a few frames that differ alone, the latest start X (a
  // write after a read, so a command's first word) from which the frame matches; then what
  // the tape holds there
  {
    auto isRoad = [&](size_t k) { return !tape[k].read && ((tape[k].pc == 0x85B536 && tape[k].v == 0x0F) || (tape[k].pc == 0x85B53B && tape[k].v == 0x01)) && k + 1 < tape.size() && tape[k + 1].v == 0x00; };
    auto run = [&](size_t from, size_t frame, size_t end) {
      dsp4_lle_load_state(power.data(), size);
      bool ok = true;
      for(size_t k = from; k < end; k++) {
        if(!tape[k].read) dsp4_lle_write(true, tape[k].v);
        else if(dsp4_lle_read(true) != tape[k].v && k >= frame) ok = false;
      }
      return ok;
    };
    int done = 0;
    for(size_t si = 0; si < streams.size() && done < 6; si += streams.size() / 6) {
      size_t f = streams[si].first, end = streams[si].second;
      while(f > 0 && !isRoad(f)) f--;
      if(run(f, f, end)) continue;
      std::vector<size_t> starts;
      for(size_t k = 1; k < f; k++) if(!tape[k].read && tape[k - 1].read) starts.push_back(k);
      starts.insert(starts.begin(), 0);
      size_t lo = 0, hi = starts.size() - 1;  // run(starts[lo]) matches (the whole tape does), run(starts[hi]) may not
      while(lo + 1 < hi) { size_t mid = (lo + hi) / 2; if(run(starts[mid], f, end)) lo = mid; else hi = mid; }
      const size_t x = starts[lo];
      std::printf("frame at tape record %zu: needs the tape from record %zu (%zu records before); there:", f, x, f - x);
      for(size_t k = x; k < x + 24 && k < tape.size(); k++) std::printf(" %s$%06X:%02X", tape[k].read ? "r" : "w", tape[k].pc, tape[k].v);
      std::printf("\n");
      done++;
    }
  }
  {  // dsp4-01c: the bit's effect, and the near rows sent
    auto isRoad = [&](size_t k) { return !tape[k].read && ((tape[k].pc == 0x85B536 && tape[k].v == 0x0F) || (tape[k].pc == 0x85B53B && tape[k].v == 0x01)) && k + 1 < tape.size() && tape[k + 1].v == 0x00; };
    auto isEdges = [&](size_t k) { return !tape[k].read && tape[k].pc == 0x81F90E && tape[k].v == 0x0A && k + 1 < tape.size() && tape[k + 1].v == 0x00; };
    auto replay = [&](size_t a, size_t f, size_t e, bool flip, std::vector<uint8_t>& out) {
      dsp4_lle_load_state(power.data(), size);
      for(size_t q = a; q < a + 16; q++) {
        if(tape[q].read) dsp4_lle_read(true);
        else dsp4_lle_write(true, uint8_t(q == a + 4 && flip ? tape[q].v ^ 0x80 : tape[q].v));
      }
      out.clear();
      for(size_t k = f; k < e; k++) {
        if(tape[k].read) out.push_back(dsp4_lle_read(true));
        else dsp4_lle_write(true, tape[k].v);
      }
    };
    std::map<uint32_t, long> byPc;
    std::map<int, long> byDelta;
    std::map<uint16_t, long> nearRows;
    long frames = 0, changed = 0;
    size_t lastRow = 0, maxRow = 0;
    for(size_t k = 0; k + 8 < tape.size(); k++)
      if(isEdges(k)) nearRows[uint16_t(tape[k + 4].v | tape[k + 5].v << 8)]++;
    std::vector<uint8_t> t, u;
    for(size_t f = 0; f < tape.size() && frames < 400; f++) {
      if(!isRoad(f)) continue;
      size_t e = f + 2;
      while(e < tape.size() && !isRoad(e)) e++;
      size_t a = f;
      while(a > 0 && !isEdges(a)) a--;
      if(a == 0) continue;
      replay(a, f, e, false, t);
      replay(a, f, e, true, u);
      frames++;
      size_t r = 0;
      for(size_t k = f; k < e; k++) {
        if(!tape[k].read) continue;
        if(t[r] != u[r]) {
          changed++;
          byPc[tape[k].pc]++;
          byDelta[int(t[r]) - int(u[r])]++;
          lastRow = k - f;
          if(lastRow > maxRow) maxRow = lastRow;
        }
        r++;
      }
    }
    std::printf("bit 7 flipped, %ld frames: %ld bytes read change (the last within %zu records of the road command)\n", frames,
                changed, maxRow);
    for(const auto& p : byPc) std::printf("    at $%06X: %ld\n", p.first, p.second);
    for(const auto& p : byDelta) std::printf("    as sent minus flipped: %+d: %ld\n", p.first, p.second);
    std::printf("near rows sent with the $0A at $81:F90E:");
    for(const auto& p : nearRows) std::printf(" $%04X x %ld", p.first, p.second);
    std::printf("\n");
  }
  static const char* names[8] = {"fresh", "chained", "frame", "frames 2", "frames 3", "frames 5", "frame + $0A", "frame + $0A, bit 7 flipped"};
  static const int back[8] = {0, 0, 1, 2, 3, 5, 1, 1};
  for(int mode = 0; mode < 8; mode++) {
    dsp4_lle_load_state(power.data(), size);
    long same = 0, differ = 0, bytes = 0, badBytes = 0;
    int shown = 0;
    long noRoad = 0, stale = 0;
    uint16_t prevWord = 0;
    uint8_t curLo = 0;
    std::map<uint32_t, long> byPc, examples;
    for(auto s : streams) {
      if(mode != 1) dsp4_lle_load_state(power.data(), size);
      size_t counted = s.first;  // reads before this are replayed but not compared
      if(mode >= 2) {  // back to the frame's road command word (or N frames')
        size_t k = s.first;
        bool found = true;
        for(int n = 0; n < back[mode] && found; n++) {
          if(n == 1) counted = k;  // (frames N: only the stream's own frame is compared)
          if(n > 0) k--;
          while(k > 0 && !(!tape[k].read && ((tape[k].pc == 0x85B536 && tape[k].v == 0x0F) || (tape[k].pc == 0x85B53B && tape[k].v == 0x01)) &&
                           k + 1 < tape.size() && tape[k + 1].v == 0x00 && !tape[k + 1].read))
            k--;
          if(k == 0) found = false;
        }
        if(!found || s.first - k > 4000000) { noRoad++; continue; }
        if(back[mode] == 1) counted = k;
        s.first = k;
        if(mode >= 6) {  // the last $000A command before it (written at $81:F90E), replayed first
          size_t a = k;
          while(a > 0 && !(!tape[a].read && tape[a].pc == 0x81F90E && tape[a].v == 0x0A && tape[a + 1].v == 0x00)) a--;
          if(a == 0) { noRoad++; continue; }
          for(size_t q = a; q < a + 16; q++) {
            if(tape[q].read) dsp4_lle_read(true);
            else dsp4_lle_write(true, uint8_t(q == a + 4 && mode == 7 ? tape[q].v ^ 0x80 : tape[q].v));  // a+4: the near row's low byte
          }
        }
      }
      bool ok = true;
      size_t firstBad = 0;
      for(size_t k = s.first; k < s.second; k++) {
        if(tape[k].read) {
          const uint8_t v = dsp4_lle_read(true);
          if(k < counted) continue;
          bytes++;
          // (the same byte of the chip's word before: a read that came before the chip's answer)
          const bool hi = k > 0 && tape[k - 1].read && tape[k - 1].pc == tape[k].pc;
          const uint8_t before = hi ? uint8_t(prevWord >> 8) : uint8_t(prevWord);
          if(v != tape[k].v && tape[k].v == before) stale++;
          if(hi) prevWord = uint16_t(curLo | v << 8);
          else curLo = v;
          if(v != tape[k].v) {
            badBytes++;
            if(ok) firstBad = k;
            ok = false;
            if(mode == 2) { byPc[tape[k].pc]++; if(examples[tape[k].pc]++ < 3) std::printf("    $%06X: tape %02X, chip %02X (+%zu)\n", tape[k].pc, tape[k].v, v, k - s.first); }
          }
        } else dsp4_lle_write(true, tape[k].v);
      }
      if(ok) same++;
      else {
        differ++;
        if(shown++ < 5)
          std::printf("  %s: stream at tape byte %zu (%zu bytes): first difference at +%zu (PC $%06X)\n", names[mode],
                      s.first, s.second - s.first, firstBad - s.first, tape[firstBad].pc);
      }
    }
    std::printf("%s: %zu streams, %ld the same, %ld differ; %ld of %ld bytes read differ", names[mode], streams.size(), same,
                differ, badBytes, bytes);
    if(mode >= 2) std::printf(" (%ld with no road command before them)", noRoad);
    std::printf("; %ld of the differing bytes are the chip's word before", stale);
    std::printf("\n");
    for(const auto& p : byPc) std::printf("    differing reads at $%06X: %ld\n", p.first, p.second);
  }
  return 0;
}
