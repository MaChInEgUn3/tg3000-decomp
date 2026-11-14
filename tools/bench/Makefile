# TG3000 bench. Run tools/bench/fetch.sh first.
ROOT := ../..
EXT := $(ROOT)/ext
OUT := $(ROOT)/build/bench
LS := $(EXT)/lakesnes-dinkc64/snes
D4 := $(EXT)/snes-dsp4/src

CC ?= gcc
CXX ?= g++
SRC := $(ROOT)/src
CFLAGS := -O2 -Wall -I$(LS) -I. -I$(SRC)
CXXFLAGS := -O2 -Wall -std=c++17 -I$(LS) -I$(D4) -I.

LS_SRC := $(filter-out $(LS)/statehandler.c,$(wildcard $(LS)/*.c)) $(LS)/statehandler.c
OBJ := $(patsubst $(LS)/%.c,$(OUT)/ls/%.o,$(LS_SRC)) $(OUT)/Dsp4.o $(OUT)/dsp4_bridge.o $(OUT)/bench.o $(OUT)/png.o $(OUT)/coverage.o $(OUT)/difftest.o $(OUT)/routines.o $(OUT)/platform_bench.o $(OUT)/census.o $(patsubst $(SRC)/%.c,$(OUT)/src_%.o,$(wildcard $(SRC)/*.c))

.PHONY: all clean
all: $(OUT)/bench

$(OUT)/bench: $(OBJ)
	$(CXX) -o $@ $^

$(OUT)/ls/%.o: $(LS)/%.c | $(OUT)/ls
	$(CC) $(CFLAGS) -w -c $< -o $@

$(OUT)/Dsp4.o: $(D4)/dsp4/Dsp4.cpp | $(OUT)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUT)/dsp4_bridge.o: dsp4_bridge.cpp bench_log.h | $(OUT)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUT)/png.o: png.c png.h | $(OUT)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT)/coverage.o: coverage.c coverage.h | $(OUT)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT)/difftest.o: difftest.c difftest.h routines.h coverage.h $(SRC)/tg3000.h | $(OUT)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT)/census.o: census.c census.h | $(OUT)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT)/platform_bench.o: platform_bench.c difftest.h $(SRC)/platform.h | $(OUT)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT)/routines.o: routines.c routines.h difftest.h $(SRC)/*.h | $(OUT)
	$(CC) $(CFLAGS) -c $< -o $@

# the game's C, built exactly as a port would build it (no bench headers)
$(OUT)/src_%.o: $(SRC)/%.c $(SRC)/*.h | $(OUT)
	$(CC) -O2 -Wall -Wextra -std=c99 -pedantic -I$(SRC) -c $< -o $@

$(OUT)/bench.o: bench.c difftest.h bench_log.h png.h coverage.h | $(OUT)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT) $(OUT)/ls:
	mkdir -p $@

clean:
	rm -rf $(OUT)
