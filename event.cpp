#include <SDL2/SDL_timer.h>
#include "lib/types.h"
#include <iostream>

int main(void) {
    const int FPS = 60;
    const int frameDelay = 1000 / 60;
    u32 frameStart;
    s32 frameTime;

    while(true) { // idk how to make an actual sdl main loop
        frameStart = SDL_GetTicks();

        // handle input
        // perform computations and get amount of extra delay needed
        // draw screen

        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) SDL_Delay(frameDelay - frameTime);
                  
    }

    return 0;
}
