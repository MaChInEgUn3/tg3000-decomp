// SPDX-License-Identifier: GPL-3.0-or-later
// dsp4-09l: src/dsp4.c's sprite list driven by a script, one list a line (tools/dsp4_09_check.py):
//   O  $0003          Z  $0005          B x y attr  a $000B entry (prints its flag, and 2 words when shown)
//   S w0..w6  $0009 and its words       L line  an empty slot     N w0..w6  a scenery object
//   C w0..w8  a car (prints its answer)  T height attr  the car's last two words
//   P w a b   a part (prints its entries' count, then theirs)     E  a list's end   X  the stream's end
//   H  $0006 (prints the 16 words)
// All numbers hex. Prints, per line, every word the chip would answer, in order.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dsp4.h"

int main(void) {
  static char line[1 << 20];
  while(fgets(line, sizeof line, stdin)) {
    char* save = NULL;
    for(char* t = strtok_r(line, " \t\r\n", &save); t; t = strtok_r(NULL, " \t\r\n", &save)) {
      uint16_t v[9];
      int n = 0;
      const char op = t[0];
      const int need = op == 'B' ? 3 : op == 'S' || op == 'N' ? 7 : op == 'L' ? 1 : op == 'C' ? 9 : op == 'T' ? 2 : op == 'P' ? 3 : 0;
      while(n < need) {
        char* w = strtok_r(NULL, " \t\r\n", &save);
        if(!w) break;
        v[n++] = (uint16_t)strtoul(w, NULL, 16);
      }
      if(n < need) break;
      switch(op) {
      case 'O': dsp4_list_open(); break;
      case 'Z': dsp4_list_sizes(); break;
      case 'B': {
        uint16_t out[2];
        const uint16_t f = dsp4_list_sprite(v[0], v[1], v[2], out);
        printf("%04X ", f);
        if(f) printf("%04X %04X ", out[0], out[1]);
        break;
      }
      case 'S': dsp4_objects_begin(v); break;
      case 'L': dsp4_objects_slot(v[0]); break;
      case 'N': dsp4_objects_scenery(v); break;
      case 'C': printf("%04X ", dsp4_objects_car(v)); break;
      case 'T': dsp4_objects_car_tail(v[0], v[1]); break;
      case 'P': {
        uint16_t out[2][2];
        if(getenv("DSP4_SPRITE_DEBUG")) {  // the state before the part: its place, the crest, the lines
          Dsp4Sprites st;
          dsp4_sprites_get(&st);
          const int y = st.y0 + (int16_t)v[1];
          fprintf(stderr, "part %04X: kind %u list %u x %d y %d crest %d(%d) lines", v[0], st.kind, st.list,
                  st.x0 + (int16_t)v[2], y, st.crest, st.crestSet);
          for(int i = 0; i < 16; i++) fprintf(stderr, " %u", st.lines[(uint8_t)(y + i)]);
          fprintf(stderr, "\n");
        }
        const unsigned k = dsp4_objects_part(v[0], v[1], v[2], out);
        printf("%u ", k);
        for(unsigned i = 0; i < k; i++) printf("%04X %04X ", out[i][0], out[i][1]);
        break;
      }
      case 'E': dsp4_objects_list_end(); break;
      case 'X':
        dsp4_objects_end();
        if(getenv("DSP4_SPRITE_LINES")) {  // dsp4-01v: the slivers counted per screen line, at the end
          Dsp4Sprites sp;
          dsp4_sprites_get(&sp);
          printf("lines");
          for(int i = 0; i < 256; i++) printf(" %u", sp.lines[i]);
          printf(" ");
        }
        break;
      case 'H': {
        uint16_t h[16];
        dsp4_high_table(h);
        for(int i = 0; i < 16; i++) printf("%04X ", h[i]);
        break;
      }
      default: break;
      }
    }
    printf("\n");
  }
  return 0;
}
