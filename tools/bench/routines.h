// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef BENCH_ROUTINES_H
#define BENCH_ROUTINES_H

#include "difftest.h"

extern const Routine routines[];
extern const int routineCount;
extern const Routine negativeRoutines[];
extern const int negativeRoutineCount;
// glue-1: composed routines (a screen whole, its parts' rows inside it), run apart from the
// main table (bench --difftest-composed): each encloses waiting rows
extern const Routine composedRoutines[];
extern const int composedRoutineCount;

#endif
