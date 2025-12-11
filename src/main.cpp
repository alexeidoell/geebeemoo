#include <gb.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "intended usage: ./geebeemoo /path/to/game\n";
        exit(-1);
    }
    GB gb_core;
    gb_core.runEmu(argv[1]);

    exit(0);
}
