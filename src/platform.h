// SPDX-License-Identifier: GPL-3.0-or-later
// Services the game logic needs from the platform. Each port implements them natively
// (no emulated hardware); the bench implements them by recording their effect so the
// differential test can compare it with what the original did through the hardware.
//
// Addresses of data in the game's memory are 24-bit SNES bus addresses (bank:address):
// LoROM ROM or WRAM ($7E/$7F, or $0000-$1FFF of banks $00-$3F/$80-$BF).
#ifndef TG3000_PLATFORM_H
#define TG3000_PLATFORM_H

#include <stdbool.h>
#include <stdint.h>

// Copy `size` bytes (1-65536) from `src` into video RAM starting at word address
// `vram_word`, low byte then high byte of each word, one word after another.
// (The original does this with DMA in mode 1 to $2118 with VMAIN = $80.)
void video_upload_vram(uint16_t vram_word, uint32_t src, uint32_t size);

// Fill `size` bytes (1-65536) of video RAM from word address `vram_word` with `value`
// (both bytes of every word). (The original does this with a fixed-source DMA to
// $2118/$2119, VMAIN = $80.)
void video_fill_vram(uint16_t vram_word, uint8_t value, uint32_t size);

// Write one word to video RAM at word address `vram_word`. (The original writes $2118 then
// $2119 with VMAIN = $80; the console takes it only in vblank or under forced blank, which
// is when the game does it.)
void video_write_vram_word(uint16_t vram_word, uint16_t value);
// Read one word of video RAM at word address `vram_word` (the original sets VMADD and reads
// VMDATA, $2139/$213A, after a dead read; harness-30).
// (Port request H: the game reads back only words it wrote itself, once a frame per VS player:
// the race's progress strip, VRAM words $57C0-$585F, holding tile $0006 with the players' bits
// ORed in: $0006 or $6006 on the movies. A port answers with the last word it was given at that
// address, through video_write_vram_word, video_upload_vram or video_fill_vram: a copy of what
// the game wrote, not a model of the video chip.)
uint16_t video_read_vram_word(uint16_t vram_word);
// The Mode 7 vertical offset alone (the original's byte writes to $210E leave it apart from
// BG1's: vs-race-20).
void video_set_m7_vofs(uint16_t value);

// The VRAM address step ($2115 VMAIN, raw byte): $80 = one word, $81 = 32 words (a
// tilemap column), after the high byte. Uploads and word writes name their addresses;
// this is compared as the last value written.
void video_set_vram_increment(uint8_t value);

// DMA channels, as slots 0-7 (the platform keeps each slot's setup; a later routine may
// enable or kick what an earlier one set up). The raw mode byte ($43x0) and B-bus target
// ($43x1, $21xx) are passed as the original writes them.
// - HDMA: a channel reading `table` line by line into $21`target`; the bank of its
//   indirect data when the mode is indirect; a new table address in the same bank;
//   which slots run this frame ($420C, a mask).
void dma_hdma_setup(int slot, uint8_t mode, uint8_t target, uint32_t table);
void dma_hdma_set_indirect_bank(int slot, uint8_t bank);
void dma_hdma_set_table(int slot, uint16_t table);
// (the parts of a setup on their own: mode and target; the table's bank)
void dma_hdma_set_mode(int slot, uint8_t mode, uint8_t target);
void dma_hdma_set_bank(int slot, uint8_t bank);
// A slot's table address as last set (its $43x2/$43x3, read back; harness-31).
// (Port request H: the HDMA never moves this word, only its own pointer; the game reads it once
// a VS frame for slot 4, always $A411, the table the race's setup gave it, and stores it at
// $7F:FFFE, which nothing reads. A port answers with the low word of the table address it was
// last given for the slot by dma_hdma_setup or dma_hdma_set_table.)
uint16_t dma_hdma_get_table(int slot);
void video_hdma_enable(uint8_t slots);
// - Streams: a source read in chunks over the frame (the race sends a palette band per
//   raster line this way). Setup; each slot's next chunk size; send the next chunk of
//   every slot in `slots`, in slot order, from where the last send stopped. A chunk to
//   CGRAM data lands at the CGRAM address (set by video_set_cgram_address, advanced by
//   each colour; a slot targeting $2121 sets it from its source byte).
void dma_stream_setup(int slot, uint8_t mode, uint8_t target, uint32_t src);
// A stream slot's source address alone, in the bank it has (vs-race-22).
void dma_stream_set_source(int slot, uint16_t src);
void dma_stream_size(int slot, uint32_t size);
void dma_stream_kick(uint8_t slots);
void video_set_cgram_address(uint8_t index);

// Copy `size` bytes (1-65536) from `src` into the palette starting at colour `index`;
// each colour is two bytes, low byte first (BGR555).
void video_upload_cgram(uint8_t index, uint32_t src, uint32_t size);

// PPU registers, set as the original writes them (both bytes, low first); the console
// latches them and they take effect on the next lines it draws (see docs/DECISIONS.md,
// "PPU registers").
//
// Mode 7 matrix parameters A-D ($211B-$211E): signed 8.8 fixed point.
void video_set_m7a(int16_t value);
void video_set_m7b(int16_t value);
void video_set_m7c(int16_t value);
void video_set_m7d(int16_t value);

// Mode 7 centre X / Y ($211F / $2120): 13 bits, signed.
void video_set_m7x(int16_t value);
void video_set_m7y(int16_t value);

// BG1 horizontal / vertical scroll ($210D / $210E). The console keeps 10 bits as the BG1
// scroll and, from the same write, 13 bits (signed) as the Mode 7 offset.
void video_set_bg1_hofs(uint16_t value);
void video_set_bg1_vofs(uint16_t value);

// BG2 horizontal / vertical scroll ($210F / $2110): 10 bits.
void video_set_bg2_hofs(uint16_t value);
void video_set_bg2_vofs(uint16_t value);

// BG3 horizontal / vertical scroll ($2111 / $2112): 10 bits.
void video_set_bg3_hofs(uint16_t value);
void video_set_bg3_vofs(uint16_t value);
// BG4 horizontal / vertical scroll ($2113 / $2114): 10 bits.
void video_set_bg4_hofs(uint16_t value);
void video_set_bg4_vofs(uint16_t value);

// More screen registers, raw bytes compared as the last value written: mosaic ($2106),
// mode 7 settings ($211A), the two windows' edges ($2126 + edge: 0/1 window 1 left/right,
// 2/3 window 2), the window mask logic for BGs and sprites ($212A/$212B), the window
// masks of the main and sub screens ($212E/$212F), the screen settings ($2133: interlace,
// overscan, pseudo-hires), and the ROM access speed ($420D).
void video_set_mosaic(uint8_t value);
void video_set_m7_select(uint8_t value);
void video_set_window_position(int edge, uint8_t value);
void video_set_window_logic_bg(uint8_t value);
void video_set_window_logic_obj(uint8_t value);
void video_set_main_window_mask(uint8_t value);
void video_set_sub_window_mask(uint8_t value);
void video_set_screen_init(uint8_t value);
void platform_set_rom_speed(uint8_t value);

// The VRAM address ($2116/$2117) when it is set and left for a later routine's writes;
// uploads and word writes name their own addresses.
void video_set_vram_address(uint16_t word);

// Screen layout (raw bytes): the BG mode ($2105), each BG's tilemap address and size
// ($2107-$210A, bg 1-4), the BG tile addresses ($210B for BG1/BG2, $210C for BG3/BG4),
// and the sprites' size and tile address ($2101).
void video_set_bg_mode(uint8_t value);
void video_set_bg_map(int bg, uint8_t value);
void video_set_bg_tiles(int pair, uint8_t value);  // pair 0: BG1/BG2, 1: BG3/BG4
void video_set_obj_select(uint8_t value);

// The display ($2100, raw byte): bit 7 forced blank, bits 0-3 brightness.
void video_set_display(uint8_t value);

// The sprite table (OAM). Its address ($2102/$2103, raw word: bits 0-8 the word address,
// bit 8 set = the 32-byte high table, bit 15 = priority rotation), compared as the last
// value written; and an upload of `size` bytes (1-65536) from `src` starting at OAM word
// `oam_word` (the original: DMA to $2104 after setting the address).
void video_set_oam_address(uint16_t value);
void video_upload_oam(uint16_t oam_word, uint32_t src, uint32_t size);

// The layers on the main and sub screens ($212C / $212D): bits 0-3 BG1-BG4, bit 4 sprites.
void video_set_main_screen(uint8_t layers);
void video_set_sub_screen(uint8_t layers);

// Colour math: which layers it applies to and how ($2131 CGADSUB, raw byte), and its
// window and source control ($2130 CGWSEL, raw byte).
void video_set_color_math(uint8_t value);
void video_set_color_math_control(uint8_t value);

// The fixed colour for colour math ($2132, raw byte): bits 7/6/5 select blue/green/red,
// and those channels take the low 5 bits.
void video_set_fixed_color(uint8_t value);

// Window masks ($2123 BG1/BG2, $2124 BG3/BG4, $2125 sprites/colour; raw bytes).
void video_set_window_bg12(uint8_t value);
void video_set_window_bg34(uint8_t value);
void video_set_window_obj(uint8_t value);

// One palette colour (BGR555), as the original writes it through $2121/$2122.
void video_set_color(uint8_t index, uint16_t bgr);

// The interrupt enables ($4200: bit 7 NMI, bits 5-4 the line interrupt's H/V mode, bit 0
// the automatic joypad read; raw byte), and the line of the line interrupt ($4209/$420A,
// 9 bits).
void platform_set_interrupts(uint8_t value);
void platform_set_irq_v(uint16_t line);

// The horizontal position of the line interrupt ($4207/$4208, 9 bits). A handler may
// move the interrupt position; the next interrupt of the same frame follows the new one.
void platform_set_irq_h(uint16_t dot);
// The low bytes alone ($4207 / $4209: the original writes 8 bits, the high bit stays).
void platform_set_irq_h_low(uint8_t dot);
void platform_set_irq_v_low(uint8_t line);
// The high byte of the line alone ($420A).
void platform_set_irq_v_high(uint8_t line);

// The WRAM port's address ($2181-$2183, 17 bits). The game reads WRAM through the port
// ($2180) only as a stream of bytes from there, which a port reads directly; the address is
// set as the original sets it.
void platform_set_wram_address(uint32_t address);

// Sound (docs/DECISIONS.md, "The APU ports, measured"). The game talks to its sound
// driver through a 16-byte message per frame; a port gives it to its own driver.
//
// Send the frame's message: four groups of four bytes, each group in the order the
// original writes them ($2141, $2142, $2143, then $2140, bit 7 of the last byte
// alternating group to group). Returns once the driver has taken it.
void sound_send_frame(const uint8_t msg[16]);

// Whether the sound driver is ready for a message (the original: $77 on $2141).
bool sound_is_ready(void);

// The sound driver's upload at start-up (port request 4), through the console's standard
// loader: wait for the loader, send each block of the driver (`size` bytes at the WRAM
// address `src`, to `spc_dest` in the sound chip's memory), then start it at `spc_entry`.
// The original's handshakes and the loader's counters are the platform's. A port that plays
// sound its own way has no driver to run and may ignore all three.
void sound_boot_begin(void);
void sound_boot_block(uint16_t spc_dest, uint32_t src, uint16_t size);
void sound_boot_start(uint16_t spc_entry);

// Clear the four sound ports, once the driver started (its first message is all zero).
void sound_clear_ports(void);

// A sound set's uploads to the running driver (port request 4), each between
// sound_upload_begin and sound_upload_end:
// - sound_upload_block: `size` bytes at the WRAM address `src`, to `spc_dest` in the sound
//   chip's memory. The game sends, per set, its songs and its instruments and settings this
//   way, and last the sample directory (4 bytes a slot: the sample's address, its loop point).
// - sound_upload_sample: sample `slot` (`size` bytes of the chip's compressed audio at the
//   bus address `src`, to `spc_dest`), whose loop point is `spc_loop`. The directory sent
//   after holds the same slot, address and loop point (the bench checks it).
// The driver's protocol (handshakes, three bytes at a time) is the platform's.
void sound_upload_begin(void);
void sound_upload_block(uint16_t spc_dest, uint32_t src, uint16_t size);
void sound_upload_sample(uint8_t slot, uint16_t spc_dest, uint32_t src, uint16_t size, uint16_t spc_loop);
void sound_upload_end(void);

// Whether the console runs at 50 Hz (the original: bit 4 of $213F).
bool video_is_pal(void);

// Controllers. The game reads up to five: controller 1 through the console's automatic
// read, and four behind a multitap in port 2, read serially two at a time.
//
// Controller 1's 16 bits as the automatic read gives them ($4218/$4219): buttons B Y
// Select Start Up Down Left Right A X L R from the top, then a 4-bit signature (0 for a
// standard controller).
uint16_t input_pad1(void);

// Port 1's first data line after the automatic read (1 with a standard controller
// plugged in, 0 with none).
bool input_port1_data(void);

// Two multitap controllers through port 2, 16 bits each, first bit highest: `pair` 0
// with the I/O line high (the original's $4201 = $80), 1 with it low. d0 comes from the
// first data line, d1 from the second; *tail gets the two lines on one more read (bit 0
// = line 1, bit 1 = line 2), where a plugged-in standard controller answers 1.
void input_multitap_pair(int pair, uint16_t* d0, uint16_t* d1, uint8_t* tail);

// At start-up, with the interrupts off: return once the next vertical blank has begun (the
// original polls $4212). A port may return at once.
void platform_wait_vblank_start(void);

// At start-up: wait until the DSP-4 has started (the original reads $FFFF from its data port
// 257 times in a row, $80:8085). A port, whose DSP-4 commands are native functions, returns
// at once.
void platform_wait_dsp4_start(void);

// Two-task screens (port request D; docs/DECISIONS.md, "Two-task screens"). The shop, the
// championship screen and the ticker run their code twice, as two tasks: task 1 with D = $0100
// (player 1's half), task 2 with D = $0200 (player 2's), sharing no WRAM. Each is ordinary
// blocking C that waits with platform_wait_vblank.
// - platform_task_split(task): called by task 1 (the screen's code) right after
//   split_screen_setup. Task 1 keeps running; task 2 is task(0x0200) and gets its first turn at
//   task 1's next platform_wait_vblank.
// - From then on, when task 1 calls platform_wait_vblank, the platform: runs irq_menu_frame
//   then irq_menu_frame_tail; runs task 2 until its next platform_wait_vblank (or until its
//   function returns: then task 2 ends, with no more turns); presents the frame; runs
//   nmi_menu_frame then nmi_menu_frame_tail then nmi_tail; and resumes task 1. (On the console:
//   the IRQ at line 108 switches to task 2, the NMI at vblank back to task 1.)
// - platform_task_join(): called by task 1 only, at the screen's end; task 2 ends if it still
//   runs. One split at a time: a split inside a split is a trap.
// The bench runs the tasks' pieces as rows of their own, either D: there both calls do nothing.
void platform_task_split(void (*task)(uint16_t dp));
void platform_task_join(void);

// Return after the next interrupt has run. On the console the NMI and IRQ handlers do
// the frame's work; they are game code too (ported to C like the rest), so a port
// presents the frame here and then runs the game's NMI code, and runs its IRQ code at
// the line the game asked for (see docs/DECISIONS.md, "HDMA, measured"). Game code that
// waits for "the next frame" calls this in a loop until its own frame counter has moved on.
void platform_wait_vblank(void);

// The DSP-4's data port ($30:8000), for the commands src/dsp4.c does not have as native
// functions yet (the streamed sprite and road commands): one word written or read, low
// byte first, in the order the original does it. A port answers from its own chip; the
// status register ($30:C000, "ready") is the platform's to wait on.
void dsp4_port_write16(uint16_t value);
uint16_t dsp4_port_read16(void);

#endif
