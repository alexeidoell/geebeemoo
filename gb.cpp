#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include "core/timer.h"
#include "lib/types.h"
#include "core/mmu.h"
#include "core/core.h"
#include "core/ppu.h"
#include "gb.h"
#include <cassert>
#include <exception>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <memory>

void setPixel(SDL_Surface* surface, u8 w, u8 h, u8 pixel) {
    u32* pixelAddress = (u32*)surface->pixels;
    pixelAddress += surface->w * h + w;

    u32 colors[4] = { 0xFFFFFF, 0x555555, 0xAAAAAA, 0x000000 };

    *pixelAddress = colors[pixel];

}

void GB::runEmu(char* filename) {
    const double FPS = 59.7275;
    const u32 frameDelay = 1000 / FPS; // gameboy framerate to 4 decimal places lol
    const u32 maxTicks = 70224; // number of instuctions per frame
    double current_ticks = maxTicks;
    u32 frameStart;
    u32 frameTime;
    s32 div_ticks;
    u32 operation_ticks;
    bool tima_flag = false;
    
    std::shared_ptr<MMU> mem = std::make_shared<MMU>();
    mem->load_cart(filename);

    SDL_Window* window = SDL_CreateWindow("test window", SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED, 160, 144, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cout << "error creating window " << SDL_GetError() << "\n"; 
        exit(-1);
    }
    SDL_Surface* surface = SDL_GetWindowSurface(window);
    SDL_Event event;

    Core& core = *new Core(mem);
    Timer& timer = *new Timer(mem);
    PPU& ppu = *new PPU(mem);
    core.bootup();


    std::ofstream log("log.txt", std::ofstream::trunc);
    
    const static u16 tima_freq[] = { 9, 3, 5, 7 };
    while(SDL_WaitEvent(&event)) { // idk how to make an actual sdl main loop
        if (event.type == SDL_QUIT) {
            std::cout << "closing gbemu\n";
            SDL_Quit();
            exit(0);
        }
        frameStart = SDL_GetTicks();
        mem->ppuState = mode2;
        mem->write(0xFF44, (u8)0);
        current_ticks = current_ticks - maxTicks;
        div_ticks = 0;
        while (current_ticks < maxTicks) {
            u16 div = (mem->read(0xFF04) << 8) + mem->read(0xFF03);
            u8 tima_bit = (div >> tima_freq[mem->read(0xFF07) & 0b11]) & 0b1;
            //doctor_log(log, core, *mem);
            operation_ticks = core.op_tree();
            if ((mem->ppu_read(0xFF40) & 0x80) == 0x80) {
                ppu.ppuLoop(operation_ticks);
            }
            current_ticks += operation_ticks;
            div_ticks += operation_ticks;
            while (div_ticks >= 4) {
                timer.div_inc();
                div = (mem->read(0xFF04) << 8) + mem->read(0xFF03);
                u8 after_tima_bit = (div >> tima_freq[mem->read(0xFF07) & 0b11]) & 0b1; 
                if ((mem->read(0xFF07) > 3 && tima_flag) || ((tima_bit == 1) && (after_tima_bit == 0) && mem->read(0xFF07) > 3)) { // falling edge
                    tima_flag = (timer.tima_inc() == -1);
                }
                div_ticks -= 4;
                div = (mem->read(0xFF04) << 8) + mem->read(0xFF03);
                tima_bit = (div >> tima_freq[mem->read(0xFF07) & 0b11]) & 0b1;
            }
        }
        std::array<u8, 23040>& buffer = ppu.getBuffer();
        for (auto i = 0; i < 144; ++i) {
            for (auto j = 0; j < 160; ++j) {
                u8 pixel = buffer[j + i * 160];
                setPixel(surface, j, i, pixel);
            }
        }
        surface = SDL_GetWindowSurface(window);
        SDL_UpdateWindowSurface(window);
        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) SDL_Delay(frameDelay - frameTime);
        std::cout << (int)SDL_GetTicks() - frameStart << " ms per frame\n";
        //assert(mem->read(0xFF44) >= 153);

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

