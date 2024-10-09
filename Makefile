CC = g++
INCLUDES = -I. -Icore/ -Ilib/
CFLAGS = -std=gnu++23 -Wall -pedantic -g -O3 -fno-exceptions
CFLAGS += $(shell pkg-config sdl3 --cflags)
# CFLAGS += -DDEBUG
LDFLAGS = -flto=auto
LDFLAGS += $(shell pkg-config sdl3 --libs)

EXECUTABLES = main

CORE_FILES = mmu.cpp timer.cpp ppu.cpp joypad.cpp mbc.cpp apu.cpp
CORE = $(CORE_FILES:%.cpp=build/%.o)
BUILD_DIR = build/

.PHONY: all clean

all: $(EXECUTABLES)

clean:
	rm -rf $(EXECUTABLES) newlog.txt build/ oldlog.txt

$(BUILD_DIR) :
	mkdir -p $@


build/mmu.o : core/mmu.cpp core/mmu.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/core.o : core/core.cpp core/core.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/oldcore.o : core/oldcore.cpp core/core.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/timer.o : core/timer.cpp core/timer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/ppu.o : core/ppu.cpp core/ppu.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/joypad.o : core/joypad.cpp core/joypad.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/mbc.o : core/mbc.cpp core/mbc.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/apu.o : core/apu.cpp core/apu.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/main.o : main.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/oldgb.o : gb.cpp gb.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DOLD $(INCLUDES) -c $< -o $@

build/gb.o : gb.cpp gb.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

oldmain : $(CORE) build/oldcore.o build/main.o build/oldgb.o
	$(CC) $(LDFLAGS) $(INCLUDES) $^ -o $@

main : $(CORE) build/core.o build/main.o build/gb.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

