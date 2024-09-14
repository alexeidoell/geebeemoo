CC = g++
CPPFLAGS = -I.
CFLAGS = -Wall -pedantic -g -fPIC -Og
INCLUDES = -I. -Icore/ -Ilib/

EXECUTABLES = main

CORE_FILES = core.cpp mmu.cpp timer.cpp ppu.cpp joypad.cpp mbc.cpp
CORE = $(CORE_FILES:%.cpp=core/%.cpp)

.PHONY: all clean

all: $(EXECUTABLES)

clean:
	rm -rf $(EXECUTABLES) log.txt

$(EXECUTABLES) : % : main.cpp gb.cpp $(CORE)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -lSDL2main -lSDL2 -o $@
