CC = clang
INCLUDES = -Isrc/ -Isrc/core/ -Isrc/lib/
CFLAGS = -std=gnu++20 -Wall -pedantic -O3 -fno-exceptions
CFLAGS += $(shell pkg-config sdl3 --cflags)
# CFLAGS += -DDEBUG
LDFLAGS = -flto=auto
LDFLAGS += $(shell pkg-config sdl3 --libs)
LDFLAGS += -lstdc++

# Windows flags 
# INCLUDES += -I"C:\Program Files (x86)\SDL3\include"
# LDFLAGS += -L"C:\Program Files (x86)\SDL3\lib"

EXECUTABLES = geebeemoo

CORE_FILES = mmu.cpp timer.cpp ppu.cpp joypad.cpp mbc.cpp apu.cpp
CORE = $(CORE_FILES:%.cpp=build/%.o)
BUILD_DIR = build/

.PHONY: all clean

all: $(EXECUTABLES)

clean:
	rm -rf $(EXECUTABLES) newlog.txt build/ oldlog.txt

$(BUILD_DIR) :
	mkdir -p $@


build/mmu.o : src/core/mmu.cpp src/core/mmu.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/core.o : src/core/core.cpp src/core/core.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/oldcore.o : src/core/oldcore.cpp src/core/core.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/timer.o : src/core/timer.cpp src/core/timer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/ppu.o : src/core/ppu.cpp src/core/ppu.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/joypad.o : src/core/joypad.cpp src/core/joypad.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/mbc.o : src/core/mbc.cpp src/core/mbc.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/apu.o : src/core/apu.cpp src/core/apu.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/main.o : src/main.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

build/oldgb.o : src/gb.cpp src/gb.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DOLD $(INCLUDES) -c $< -o $@

build/gb.o : src/gb.cpp src/gb.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

oldmain : $(CORE) build/oldcore.o build/main.o build/oldgb.o
	$(CC) $(LDFLAGS) $(INCLUDES) $^ -o $@

geebeemoo : $(CORE) build/core.o build/main.o build/gb.o
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

