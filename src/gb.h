#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <core/mmu.h>
#include <core/apu.h>
#include <core/ppu.h>
#include <core/timer.h>
#include <core/core.h>


class GB {
    Joypad joypad;
    PPU ppu{};
    MMU mem;
    Core core;
    Timer timer;
    APU apu;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    SDL_AudioStream* audio_stream;
    SDL_AudioDeviceID dev;
public:
    void runEmu(char* filename);
    void doctor_log(u32 frame, u32 ticks, std::ofstream& log, Core& core, MMU& mem);
    GB();
    ~GB();
};

