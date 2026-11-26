// SPDX-License-Identifier: GPL-3.0-or-later
// The DSP-4 coprocessor's commands as native functions. The game talks to the chip
// through $30:8000 (command word, parameters, results); a port calls these instead.
// Each one is checked against the chip's real program (the reference bench with the
// firmware, tools/refbench) and, inside ported routines, against the bench's
// differential test.
#ifndef TG3000_DSP4_H
#define TG3000_DSP4_H

#include <stdbool.h>
#include <stdint.h>

// Command $0000: the signed 16 x 16 -> 32-bit product (results read low word first).
int32_t dsp4_multiply(int16_t a, int16_t b);

// Command $000A: four edges between two rows. Each row word holds four signed 4-bit
// widths (bits 15-12, 11-8, 7-4, 3-0), each worth 48; edge j goes from row `near`'s width
// j (t = 0) to row `far`'s (t = $8000), at t = `t` (the floor of the linear step).
// Measured on the firmware over the TAS (tools/refbench --dsp4-cmdlog 0A): t is a
// multiple of $80 below $8000, widths 0 and 2 are -7..-1 and widths 1 and 3 are 0..7 in
// every run the game makes; outside that the formula is the natural extension, not a
// measured one (docs/DECISIONS.md, "DSP-4 command $0A").
void dsp4_edges(uint16_t t, uint16_t near, uint16_t far, int16_t out[4]);

// Command $0011, the inverse of $000A: a row word from four edges, each edge to a signed
// 4-bit width, floor(edge * 1364 / 65536) (about edge / 48), edge 0 in bits 15-12 ... edge 3
// in bits 3-0. First measured on the TAS (90 distinct inputs), which could not tell the
// rounding apart; settled on chosen inputs to the firmware chip (tools/bench/dsp4_probe,
// docs/DECISIONS.md dsp4-11a): exact over the whole 16-bit range.
uint16_t dsp4_row(const int16_t edge[4]);

// Command $000B, a sprite entry (and command $0003, which takes one word, $0005 in every
// run, then the first entry's $000B and the entry): three words in (x, y, an attribute
// word). Out: a flag; when it is 1 (the sprite is shown), the low bytes of x and y as one
// word (x in bits 7-0, y in bits 15-8) and the attribute unchanged; when it is 0 (hidden),
// nothing more (the game reads only the flag). `out` gets the two words; the function
// returns the flag.
// Measured on the firmware: over the whole TAS (refbench --dsp4-cmdlog 03 and 0B: 117,650
// and 117,664 runs, the mini-map's two cars, y 32-67, all shown, 0 differ), and on chosen
// inputs (tools/bench/dsp4_probe, docs/DECISIONS.md dsp4-03b/03c). Hidden, by y alone:
// below -7, and rows 233 and 235-511 of every 512. Not modelled, the chip's state: it hides
// the 34th entry of a list on one row and after (the game sends two a list), and rows 233,
// 235, 236 and 238 depend on its history (the C gives the fresh chip's answer).
uint16_t dsp4_sprite(uint16_t x, uint16_t y, uint16_t attr, uint16_t out[2]);

// Command $0001, the road: a header, then row after row, each answered with four words and the
// screen lines it draws. A row's lines are (an index, the word for $0798, the word for $0796).
// Measured on the firmware from outputs only (tools/bench/dsp4_road_probe; docs/DECISIONS.md
// dsp4-01e and dsp4-01k to dsp4-01q): exact on recorded and random runs, for segments of fewer
// than 88 lines (the game's are under 52) and header word 1 (the camera's line) at or above
// word 2 (the line limit), as road_rows_project sends.
// The first row's rounding comes from the chip's last $0A: `edge2` and `edge3` are that
// command's last two edges (the multiplies in between leave them), $8000 for none since
// power-on. Measured for $0A runs whose two rows are equal, which is every one the game
// sends before the road. Without a $0A since the last road run (the race's first frames), the
// chip's rounding is what the commands since left: not modelled (a line word may be 1 off).
// (dsp4-01r: the rounding is used once: the first row that draws leaves $2000, edges $8000, for
// the next run, and a run that draws nothing leaves it unused. So pass the last $0A's edges if
// it came after the last run that drew, else $8000; the sprite commands, $07/$08/$09 or $03,
// change it too.)
typedef struct {
  int32_t x, g, e, s;       // the road's sideways place, its turn rate, the height, the slope (16.16)
  uint16_t head[15];
  int16_t m;                // the lowest line drawn so far
  int16_t yPrev;            // the previous row's line
  int32_t endA, endB;       // the previous row's ends for the two words
  uint16_t round[2];        // the first row's rounding (from the last $0A), then $2000
  uint16_t index;           // the next line's index
  int row;
  bool fromState;           // begun by dsp4_road_start, its rounding not used yet
} Dsp4Road;
void dsp4_road_begin(Dsp4Road* r, const uint16_t head[15], int16_t edge2, int16_t edge3);
// One row: its ROM height `h`; `first` (the first row only: three words, else NULL); its three
// track words. `out` gets the four answers; up to `max` lines go to `lines`; returns the count.
unsigned dsp4_road_row(Dsp4Road* r, uint16_t h, const uint16_t first[3], const uint16_t track[3], uint16_t out[4],
                       uint16_t (*lines)[3], unsigned max);

// The chip's road rounding as state (dsp4-01r/01t): dsp4_edges sets it from its edges 2 and 3;
// dsp4_road_start begins a road with it, and the run's first row that draws uses it up
// (back to $8000, $8000, the power-on value). The sprite commands change it too and are not
// done: a port gets the race's first frames (before any $0A) up to 1 off in a line word.
void dsp4_road_start(Dsp4Road* r, const uint16_t head[15]);
void dsp4_round_get(int16_t* edge2, int16_t* edge3);

// Command $0007, the second road (dsp4-07a): $01's lines from rows the caller places itself.
// Its header is $01's first 11 words (word 6 and 7: the start's fine and whole x). Each row is
// 6 words: its height h, a screen line and an offset, a screen x and an offset, a word added to
// its $0798 words; its answers are x + offset * h >> 15, then line + offset * h >> 15, and its
// lines are $01's from those. The rounding is the chip's state, as $01's.
void dsp4_road2_start(Dsp4Road* r, const uint16_t head[11]);

unsigned dsp4_road2_row(Dsp4Road* r, const uint16_t words[6], uint16_t out[2], uint16_t (*lines)[3], unsigned max);
void dsp4_round_set(int16_t edge2, int16_t edge3);

// Command $0008, the road's windows (dsp4-08a to dsp4-08f): per screen line of each road, a
// window's left and right edges as one word (left << 8 | right, each 0..$FF), written by the
// caller at the address the chip gives (each road's table address, less 4 a line). Its header
// is 45 words; then per row 9 words (h, road 1's x and line, road 2's x and line, the four edge
// words: road 1's right and left, road 2's right and left). The windows come from road 2's x
// alone (word 3) and header word 39; road 1's x is unused. Measured on the firmware from
// outputs only: exact when both roads' lines are equal, which is every run the game makes
// (road 2's line is road 1's + a zero offset); with different lines, road 2's table differs.
typedef struct {
  int32_t x, base;
  int32_t prevL[2], prevR[2];
  int16_t yPrev[2], m[2];
  uint16_t addr[2];
} Dsp4Window;
// Returns the first answer (road 1's camera window, which the game uses only as a fill value).
uint16_t dsp4_window_begin(Dsp4Window* w, const uint16_t head[45]);
// One row: its 9 words; each road's (address, word) pairs into `runs[k]` (up to `max`), their
// counts into `n`.
void dsp4_window_row(Dsp4Window* w, const uint16_t words[9], uint16_t (*runs[2])[2], unsigned n[2], unsigned max);
// The last answer, after $8000: 0 in every run measured (the game's "far" windows never run).
uint16_t dsp4_window_end(const Dsp4Window* w);
#endif
