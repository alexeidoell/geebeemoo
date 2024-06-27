CC = g++
CPPFLAGS = -I.
CFLAGS = -Wall -pedantic -g -fPIC

EXECUTABLES = main

CORE_FILES = core.cpp mmu.cpp timer.cpp ppu.cpp
CORE = $(CORE_FILES:%.cpp=core/%.cpp)

.PHONY: all clean

all: $(EXECUTABLES)

clean:
	rm -rf $(EXECUTABLES) log.txt

$(EXECUTABLES) : % : main.cpp gb.cpp $(CORE)
	$(CC) $(CFLAGS) $^ -lSDL2main -lSDL2 -o $@
