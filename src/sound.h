// SPDX-License-Identifier: GPL-3.0-or-later
// The game's side of the sound driver protocol (docs/DECISIONS.md, "The APU ports,
// measured"): the per-frame message built from the sound state in WRAM.
#ifndef TG3000_SOUND_H
#define TG3000_SOUND_H

#include <stdbool.h>
#include <stdint.h>

#define SOUND_FLAGS 0x045B    // byte: low bits of the first message byte
#define SOUND_VOLUME 0x045C   // byte: master volume, faded by SOUND_FADE
#define SOUND_FADE 0x03E0     // word: volume step per frame (signed, doubled); 0 when done
#define SOUND_ENABLE 0x046D   // word: bit 15 set = send the message this frame
#define SOUND_REQUEST_A 0x0463  // byte: a sound request (cleared once sent), then $0464-$0467
#define SOUND_REQUEST_B 0x0468  // byte: a second one, then $0469-$046C

// $86:DFAE: SOUND_VOLUME += 2 * SOUND_FADE (8 bits), clamped to $7F going up and to 0
// going down, where the fade also stops (SOUND_FADE = 0). Returns the new volume.
uint8_t sound_fade_step(void);

// $86:DFFB: build and send the frame's message (sound_send_frame), then clear both
// request bytes and set $045F to $FF.
void sound_send_message(void);

// $86:DFE1 (long): sound_fade_step, then the message if SOUND_ENABLE's bit 15 is set and
// the driver is ready. Returns whether it was sent.
bool sound_frame(void);

#endif
