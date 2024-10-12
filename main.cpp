#include <gb.h>
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

int main(int argc, char* argv[]) {
//    std::unique_ptr<GB> testGB = std::make_unique<GB>();
    if (argc != 2) {
        std::cout << "intended usage: ./geebeemoo /path/to/game\n";
        exit(-1);
    }
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
    GB testGB;
    testGB.runEmu(argv[1]);
    SDL_Quit();

    exit(0);
}
