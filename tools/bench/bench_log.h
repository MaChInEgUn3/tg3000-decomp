// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef BENCH_LOG_H
#define BENCH_LOG_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// One DSP-4 register access. `data` is false for the status register.
void bench_log_dsp4(bool write, bool data, uint8_t bank, uint16_t adr, uint8_t val);

#ifdef __cplusplus
}
#endif

#endif
