#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include "lib/types.h"
#include "core/mmu.h"
#include "core/core.h"
#include <thread>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <memory>


class GB {
public:
    void runEmu(char* filename);
    void doctor_log(std::ofstream& log, Core& core, MMU& mem);
    GB() {
    }
    ~GB() {}

};

void GB::runEmu(char* filename) {
    const double FPS = 59.7275;
    const double frameDelay = 1000 / FPS; // gameboy framerate to 4 decimal places lol
    const double maxTicks = 4194304 / 59.7275; // number of instuctions per frame
    double current_ticks = maxTicks;
    u32 frameStart;
    s32 frameTime;
    u32 tima_ticks;
    u32 div_ticks;
    u32 operation_ticks;
    
    std::shared_ptr<MMU> mem = std::make_shared<MMU>();
    mem->load_cart(filename);

    std::unique_ptr<Core> core = std::make_unique<Core>(mem);
    core->bootup();

    std::ofstream log("log.txt", std::ofstream::trunc);
    
    u16 tima_freq[] = { 1024, 16, 64, 256 };
    while(true) { // idk how to make an actual sdl main loop
        frameStart = SDL_GetTicks();
        
        current_ticks = std::trunc(current_ticks - maxTicks);
        while (current_ticks < maxTicks) {
            doctor_log(log, *core, *mem);
            operation_ticks = core->op_tree();
            current_ticks += operation_ticks;
            div_ticks += operation_ticks;
            tima_ticks += operation_ticks;
            if (div_ticks > 256) {
                mem->div_inc();
                div_ticks -= 256;
            }
            if (mem->read(0xFF07) > 3 && tima_ticks > tima_freq[mem->read(0xFF07) % 0b11]) {
                mem->tima_inc();
                tima_ticks -= tima_freq[mem->read(0xFF07) & 0b11];
            }
        }

        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) SDL_Delay(frameDelay - frameTime);


    } 
}

void GB::doctor_log(std::ofstream& log, Core& core, MMU& mem) {

    log << std::hex << std::setfill('0') << "A:" << std::setw(2) << (int) core.registers.gpr.n.a;
    log << " F:" << std::setw(2) << (int) core.registers.flags;
    log << " B:" <<  std::setw(2) << (int) core.registers.gpr.n.b;
    log << " C:" <<  std::setw(2) << (int) core.registers.gpr.n.c;
    log << " D:" <<  std::setw(2) << (int) core.registers.gpr.n.d;
    log << " E:" <<  std::setw(2) << (int) core.registers.gpr.n.e;
    log << " H:" <<  std::setw(2) << (int) core.registers.gpr.n.h;
    log << " L:" <<  std::setw(2) << (int) core.registers.gpr.n.l;
    log << " SP:" <<  std::setw(4) << (int) core.registers.sp;
    log << " PC:" <<  std::setw(4) << (int) core.registers.pc;
    log << " PCMEM:" <<  std::setw(2) << (int) mem.read(core.registers.pc) << ",";
    log <<  std::setw(2) << (int) mem.read(core.registers.pc + 1) << ",";
    log <<  std::setw(2) << (int) mem.read(core.registers.pc + 2) << ",";
    log <<  std::setw(2) << (int) mem.read(core.registers.pc + 3);
    log << "\n";
}


int main(int argc, char* argv[]) {
    std::unique_ptr<GB> testGB = std::make_unique<GB>();
    testGB->runEmu(argv[1]);


}
