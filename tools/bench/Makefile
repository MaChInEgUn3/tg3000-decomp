# TG3000 bench. Run tools/bench/fetch.sh first.
ROOT := ../..
EXT := $(ROOT)/ext
OUT := $(ROOT)/build/bench
LS := $(EXT)/lakesnes-dinkc64/snes
D4 := $(EXT)/snes-dsp4/src

CC ?= gcc
CXX ?= g++
CFLAGS := -O2 -Wall -I$(LS) -I.
CXXFLAGS := -O2 -Wall -std=c++17 -I$(LS) -I$(D4) -I.

LS_SRC := $(filter-out $(LS)/statehandler.c,$(wildcard $(LS)/*.c)) $(LS)/statehandler.c
OBJ := $(patsubst $(LS)/%.c,$(OUT)/ls/%.o,$(LS_SRC)) $(OUT)/Dsp4.o $(OUT)/dsp4_bridge.o $(OUT)/bench.o $(OUT)/png.o

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

$(OUT)/bench.o: bench.c bench_log.h png.h | $(OUT)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT) $(OUT)/ls:
	mkdir -p $@

clean:
	rm -rf $(OUT)
