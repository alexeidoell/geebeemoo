#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include "lib/types.h"
#include "core/mmu.h"
#include "core/core.h"
#include <iostream>
#include <string>
#include <memory>


class GB {
public:
    void runEmu(char* filename);
    GB() {
    }
    ~GB() {}

};

void GB::runEmu(char* filename) {
    const int FPS = 60;
    const int frameDelay = 1000 / 60;
    u32 frameStart;
    s32 frameTime;
    
    std::shared_ptr<MMU> mem = std::make_shared<MMU>();
    std::cout << (int) mem->load_cart(filename);

    std::unique_ptr<Core> core = std::make_unique<Core>(mem);
    core->bootup();
    while (true) {
        core->op_tree();
    }

    //while(true) { // idk how to make an actual sdl main loop
                  //        frameStart = SDL_GetTicks();

        // handle input
        // perform computations and get amount of extra delay needed
        // draw screen

 //       frameTime = SDL_GetTicks() - frameStart;
  //      if (frameDelay > frameTime) SDL_Delay(frameDelay - frameTime);
                  
   // }
}


int main(int argc, char* argv[]) {
    std::unique_ptr<GB> testGB = std::make_unique<GB>();
    testGB->runEmu(argv[1]);


}
