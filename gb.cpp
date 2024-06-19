#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include "lib/types.h"
#include "core/mmu.h"
#include "core/core.h"
#include <iostream>
#include <iomanip>
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
    mem->load_cart(filename);

    std::unique_ptr<Core> core = std::make_unique<Core>(mem);
    core->bootup();
    while (true) {
        std::cout << std::hex << std::setfill('0') << "A:" << std::setw(2) << (int) core->registers.gpr.n.a;
        std::cout << " F:" << std::setw(2) << (int) core->registers.flags;
        std::cout << " B:" <<  std::setw(2) << (int) core->registers.gpr.n.b;
        std::cout << " C:" <<  std::setw(2) << (int) core->registers.gpr.n.c;
        std::cout << " D:" <<  std::setw(2) << (int) core->registers.gpr.n.d;
        std::cout << " E:" <<  std::setw(2) << (int) core->registers.gpr.n.e;
        std::cout << " H:" <<  std::setw(2) << (int) core->registers.gpr.n.h;
        std::cout << " L:" <<  std::setw(2) << (int) core->registers.gpr.n.l;
        std::cout << " SP:" <<  std::setw(4) << (int) core->registers.sp;
        std::cout << " PC:" <<  std::setw(4) << (int) core->registers.pc;
        std::cout << " PCMEM:" <<  std::setw(2) << (int) mem->read(core->registers.pc) << ",";
        std::cout <<  std::setw(2) << (int) mem->read(core->registers.pc + 1) << ",";
        std::cout <<  std::setw(2) << (int) mem->read(core->registers.pc + 2) << ",";
        std::cout <<  std::setw(2) << (int) mem->read(core->registers.pc + 3);
        std::cout << "\n";
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
