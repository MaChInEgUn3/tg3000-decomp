# TG3000 reference bench (bsnes from BizHawk 1.9.4). Run tools/refbench/fetch.sh first.
ROOT := ../..
EXT := $(ROOT)/ext
OUT := $(ROOT)/build/refbench
BSNES := $(EXT)/bizhawk-1.9.4/libsnes/bsnes
D4 := $(EXT)/snes-dsp4/src

CC ?= gcc
CXX ?= g++

# bsnes v087 predates current compilers: these pre-includes stand in for headers it used
# to get transitively.
BSNES_CFLAGS := -I. -O2 -fPIC -include stdint.h -include stddef.h -include string.h -include limits.h -w
BSNES_CPP := $(CXX) -std=gnu++11 -include vector -include algorithm -include type_traits -include utility

.PHONY: all clean libsnes
all: $(OUT)/refbench

$(BSNES)/out/libsnes.a:
	mkdir -p $(BSNES)/obj $(BSNES)/out
	$(MAKE) -C $(BSNES) platform=x target=libsnes profile=compatibility compiler=$(CC) \
	  cpp="$(BSNES_CPP)" BIZWINCFLAGS="$(BSNES_CFLAGS)" build

$(OUT)/refbench: refbench.cpp $(OUT)/Dsp4.o $(OUT)/png.o $(BSNES)/out/libsnes.a | $(OUT)
	$(CXX) -O2 -Wall -std=c++17 -I$(BSNES)/target-libsnes -I$(D4) -I../bench -o $@ \
	  refbench.cpp $(OUT)/Dsp4.o $(OUT)/png.o $(BSNES)/out/libsnes.a -ldl

$(OUT)/Dsp4.o: $(D4)/dsp4/Dsp4.cpp | $(OUT)
	$(CXX) -O2 -std=c++17 -I$(D4) -c $< -o $@

$(OUT)/png.o: ../bench/png.c ../bench/png.h | $(OUT)
	$(CC) -O2 -c $< -o $@

$(OUT):
	mkdir -p $@

clean:
	rm -rf $(OUT)
