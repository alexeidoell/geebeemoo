#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include "lib/types.h"
#include "core/mmu.h"
#include <iostream>
#include <string>
#include <memory>


class GB {
public:
    void runEmu();
};

void GB::runEmu() {
    const int FPS = 60;
    const int frameDelay = 1000 / 60;
    u32 frameStart;
    s32 frameTime;
    std::string filename = "../testrom.gb";
    
    std::shared_ptr<MMU> mem = std::make_shared<MMU>();
    std::cout << mem->load_cart(filename);


    while(true) { // idk how to make an actual sdl main loop
        frameStart = SDL_GetTicks();

        // handle input
        // perform computations and get amount of extra delay needed
        // draw screen

        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) SDL_Delay(frameDelay - frameTime);
                  
    }
}
