CC = g++
CPPFLAGS = -I.
CFLAGS = -Wall -pedantic -g -fPIC

EXECUTABLES = main

.PHONY: all clean

all: $(EXECUTABLES)

clean:
	rm -rf $(EXECUTABLES) log.txt

$(EXECUTABLES) : % : main.cpp gb.cpp core/core.cpp core/mmu.cpp core/timer.cpp core/ppu.cpp
	$(CC) $(CFLAGS) $^ -lSDL2main -lSDL2 -o $@
