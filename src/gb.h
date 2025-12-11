#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <core/mmu.h>
#include <core/apu.h>
#include <core/ppu.h>
#include <core/timer.h>
#include <core/core.h>

constexpr static std::array<u8,4> tima_freq = { 9, 3, 5, 7 };
constexpr double FPS = 59.7275;
constexpr u64 frameDelay = 1000000000 / FPS;
constexpr u32 maxTicks = 70224; // number of instuctions per frame

class GB {
    Joypad joypad;
    MMU mem;
    Core core;
    Timer timer;
    PPU ppu;
    APU apu;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    SDL_AudioStream* audio_stream;
    SDL_AudioDeviceID dev;
    u32 current_ticks = maxTicks;
    bool tima_flag = false;
    bool white = false;
    bool running = true;

    u8 tima_loop(u16 div, u8 tima_bit);
    u8 run_instr();
    void poll_SDL_event(SDL_Event& event); 
    void update_non_core_hw(u8 operation_ticks);
public:
    void runEmu(char* filename);
    void doctor_log(u32 frame, u32 ticks, std::ofstream& log, Core& core, MMU& mem);
    GB();
    ~GB();
};

