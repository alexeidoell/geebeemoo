CC = g++
CPPFLAGS = -I.
CFLAGS = -Wall -pedantic -g -fPIC

EXECUTABLES = window

.PHONY: all clean

all: $(EXECUTABLES)

clean:
	rm -rf $(EXECUTABLES)

$(EXECUTABLES) : % : %.cpp
	$(CC) $(CFLAGS) $< -lSDL2main -lSDL2 -o $@
