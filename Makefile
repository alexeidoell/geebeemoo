CC = g++
CPPFLAGS = -I.
CFLAGS = -std=gnu++23 -Wall -pedantic -g -fPIC -Og
INCLUDES = -I. -Icore/ -Ilib/

EXECUTABLES = main

CORE_FILES = mmu.cpp core.cpp timer.cpp ppu.cpp joypad.cpp mbc.cpp apu.cpp
CORE_HEADERS = $(CORE_FILES:%.cpp=core/%.h)
CORE = $(CORE_FILES:%.cpp=build/%.o)
BUILD_DIR = build/

.PHONY: all clean

all: $(EXECUTABLES)

clean:
	rm -rf $(EXECUTABLES) log.txt build/

BUILD_DIR:
	mkdir -p build/

CORE : $(CORE_HEADERS)

build/mmu.o : core/mmu.cpp | BUILD_DIR
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -lSDL2main -lSDL2 -o $@

build/core.o : core/core.cpp | BUILD_DIR
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -lSDL2main -lSDL2 -o $@

build/timer.o : core/timer.cpp | BUILD_DIR
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -lSDL2main -lSDL2 -o $@

build/ppu.o : core/ppu.cpp | BUILD_DIR
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -lSDL2main -lSDL2 -o $@

build/joypad.o : core/joypad.cpp | BUILD_DIR
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -lSDL2main -lSDL2 -o $@

build/mbc.o : core/mbc.cpp | BUILD_DIR
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -lSDL2main -lSDL2 -o $@

build/apu.o : core/apu.cpp | BUILD_DIR
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -lSDL2main -lSDL2 -o $@

build/main.o : main.cpp | BUILD_DIR
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -lSDL2main -lSDL2 -o $@

build/gb.o : gb.cpp | BUILD_DIR
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -lSDL2main -lSDL2 -o $@

$(EXECUTABLES) : % : $(CORE) build/main.o build/gb.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ -lSDL2main -lSDL2 -o $@


