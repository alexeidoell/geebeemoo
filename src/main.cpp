#include <gb.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "intended usage: ./geebeemoo /path/to/game\n";
        exit(-1);
    }
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
    GB gb_core;
    gb_core.runEmu(argv[1]);
    SDL_Quit();

    exit(0);
}
