CC = g++
CPPFLAGS = -I.
CFLAGS = -Wall -pedantic -g -fPIC

EXECUTABLES = gb

.PHONY: all clean

all: $(EXECUTABLES)

clean:
	rm -rf $(EXECUTABLES)

$(EXECUTABLES) : % : %.cpp core/core.cpp core/mmu.cpp
	$(CC) $(CFLAGS) $^ -lSDL2main -lSDL2 -o $@
