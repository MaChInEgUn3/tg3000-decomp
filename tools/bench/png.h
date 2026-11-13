// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef BENCH_PNG_H
#define BENCH_PNG_H

#include <stdbool.h>
#include <stdint.h>

bool png_write_rgb(const char* path, const uint8_t* rgb, int width, int height);

#endif
