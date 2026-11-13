// SPDX-License-Identifier: GPL-3.0-or-later
// Minimal RGB PNG writer (uncompressed deflate blocks), so the bench needs no libraries.
#include "png.h"

#include <stdio.h>
#include <stdlib.h>

static uint32_t crcTable[256];

static uint32_t crc32(uint32_t crc, const uint8_t* data, size_t len) {
  if(crcTable[1] == 0) {
    for(uint32_t n = 0; n < 256; n++) {
      uint32_t c = n;
      for(int k = 0; k < 8; k++) c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
      crcTable[n] = c;
    }
  }
  crc ^= 0xFFFFFFFFu;
  for(size_t i = 0; i < len; i++) crc = crcTable[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
  return crc ^ 0xFFFFFFFFu;
}

static void be32(uint8_t* p, uint32_t v) { p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = v; }

static void chunk(FILE* f, const char* type, const uint8_t* data, uint32_t len) {
  uint8_t hdr[8];
  be32(hdr, len);
  for(int i = 0; i < 4; i++) hdr[4 + i] = (uint8_t)type[i];
  fwrite(hdr, 1, 8, f);
  if(len) fwrite(data, 1, len, f);
  uint32_t crc = crc32(0, hdr + 4, 4);
  crc = crc32(crc, data, len);
  uint8_t c[4];
  be32(c, crc);
  fwrite(c, 1, 4, f);
}

bool png_write_rgb(const char* path, const uint8_t* rgb, int width, int height) {
  FILE* f = fopen(path, "wb");
  if(f == NULL) return false;
  static const uint8_t sig[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
  fwrite(sig, 1, 8, f);
  uint8_t ihdr[13];
  be32(ihdr, width);
  be32(ihdr + 4, height);
  ihdr[8] = 8; ihdr[9] = 2; ihdr[10] = 0; ihdr[11] = 0; ihdr[12] = 0;  // 8-bit RGB
  chunk(f, "IHDR", ihdr, 13);

  // raw scanlines (filter byte 0 + RGB), wrapped in stored deflate blocks of <= 65535 bytes
  size_t rowLen = (size_t)width * 3 + 1, rawLen = rowLen * height;
  uint8_t* raw = malloc(rawLen);
  for(int y = 0; y < height; y++) {
    raw[y * rowLen] = 0;
    for(size_t i = 0; i < (size_t)width * 3; i++) raw[y * rowLen + 1 + i] = rgb[(size_t)y * width * 3 + i];
  }
  size_t blocks = (rawLen + 65534) / 65535;
  size_t zLen = 2 + rawLen + blocks * 5 + 4;
  uint8_t* z = malloc(zLen);
  size_t p = 0;
  z[p++] = 0x78; z[p++] = 0x01;
  uint32_t a = 1, b = 0;
  for(size_t off = 0; off < rawLen; off += 65535) {
    size_t n = rawLen - off < 65535 ? rawLen - off : 65535;
    z[p++] = (off + n == rawLen) ? 1 : 0;
    z[p++] = n & 0xFF; z[p++] = n >> 8;
    z[p++] = ~n & 0xFF; z[p++] = (~n >> 8) & 0xFF;
    for(size_t i = 0; i < n; i++) {
      z[p++] = raw[off + i];
      a = (a + raw[off + i]) % 65521;
      b = (b + a) % 65521;
    }
  }
  be32(z + p, (b << 16) | a);
  p += 4;
  chunk(f, "IDAT", z, (uint32_t)p);
  chunk(f, "IEND", NULL, 0);
  free(raw);
  free(z);
  return fclose(f) == 0;
}
