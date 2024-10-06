CC = g++
CPPFLAGS = -I.
CFLAGS = -std=gnu++23 -Wall -pedantic -g -fPIC -Og
INCLUDES = -I. -Icore/ -Ilib/

EXECUTABLES = main

CORE_FILES = mmu.cpp core.cpp timer.cpp ppu.cpp joypad.cpp mbc.cpp apu.cpp
CORE = $(CORE_FILES:%.cpp=build/%.o)
BUILD_DIR = build/

.PHONY: all clean

all: $(EXECUTABLES)

clean:
	rm -rf $(EXECUTABLES) build/

$(BUILD_DIR) :
	mkdir -p $@


build/mmu.o : core/mmu.cpp core/mmu.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/core.o : core/core.cpp core/core.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/timer.o : core/timer.cpp core/timer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/ppu.o : core/ppu.cpp core/ppu.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/joypad.o : core/joypad.cpp core/joypad.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -lSDL2main -lSDL2 -o $@

build/mbc.o : core/mbc.cpp core/mbc.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/apu.o : core/apu.cpp core/apu.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/main.o : main.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/gb.o : gb.cpp gb.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -lSDL2main -lSDL2 -o $@

$(EXECUTABLES) : % : $(CORE) build/main.o build/gb.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ -lSDL2main -lSDL2 -o $@


