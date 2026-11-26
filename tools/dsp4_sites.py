#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# dsp4-chip-1: the places in the listing where the game writes a constant command word to the
# DSP-4 (`LDA #$00cc` then `STA $30:8000`, or `STA $8000` with the data bank there), for
# tools/bench/dsp4_compare. One line per store: the PC after it (a tape's records carry the
# PC after the accessing instruction) and the word's low byte.
#   python3 -I tools/dsp4_sites.py > build/cache/dsp4_sites.txt
import glob
import os
import re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ADDR = re.compile(r'; \$([0-9A-F]{6})')
for fn in sorted(glob.glob(os.path.join(ROOT, 'asm', 'bank_*.asm'))):
    lines = [line for line in open(fn) if ADDR.search(line)]
    for i in range(1, len(lines)):
        store = re.match(r'\s*(sta\.[lw]) \$(308000|8000)\s', lines[i])
        load = re.match(r'\s*lda\.w #\$00([0-9A-F]{2})\s', lines[i - 1])
        if not store or not load:
            continue
        pc = int(ADDR.search(lines[i]).group(1), 16)
        print('%06X %s' % (pc + (4 if store.group(1) == 'sta.l' else 3), load.group(1)))
