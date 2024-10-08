#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <core/mmu.h>
#include <core/apu.h>
#include <core/ppu.h>
#include <core/timer.h>
#include <core/core.h>

class GB {
    Joypad joypad;
    MMU mem;
    Core core;
    Timer timer;
    PPU ppu;
    APU apu;
    SDL_Window* window;
    SDL_Surface* surface;
    SDL_Event event;
    SDL_AudioDeviceID dev;
public:
    void runEmu(char* filename);
    void doctor_log(u32 frame, u32 ticks, std::ofstream& log, Core& core, MMU& mem);
    GB();
    ~GB();
};

