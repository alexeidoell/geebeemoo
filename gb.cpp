#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_video.h>
#include <chrono>
#include <lib/types.h>
#include <core/mmu.h>
#include <core/timer.h>
#include <core/core.h>
#include <core/ppu.h>
#include <core/apu.h>
#include <gb.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <bit>
#include <thread>

void callback(void* apu_ptr, u8* stream, int len) {

    auto* float_stream{std::bit_cast<float*>(stream)};
    float sample = 0;
    APU& apu = *(std::bit_cast<APU*>(apu_ptr)); // lol???? ????? ???
    len /= sizeof(float); // LOL!!
    for (auto i = 0; i < len; ++i) {
        sample = apu.getSample();
        float_stream[i] = 0.1f * sample;
    }

}

void GB::runEmu(char* filename) {
    // putting 60 instead of the actual value makes it slightly more accurate lol
    const double FPS = 60;
    std::chrono::duration<double, std::micro> frameDelay(1000000 / FPS);
    const u32 maxTicks = 70224; // number of instuctions per frame
    u32 current_ticks = maxTicks;
    std::chrono::time_point<std::chrono::high_resolution_clock> frameStart;
    std::chrono::duration<double, std::micro> frameTime{};
    u32 div_ticks = 0;
    u32 operation_ticks = 0;
    bool tima_flag = false;
    
    Joypad joypad{};
    MMU mem(joypad);
    if (0 == mem.load_cart(filename)) {
        std::cout << "emu quitting due to rom not existing\n";
        return;
    }

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);

    SDL_Window* window = SDL_CreateWindow("Geebeemoo", SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED, 160, 144, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if (!window) {
        std::cout << "error creating window " << SDL_GetError() << "\n"; 
        exit(-1);
    }
    //SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_Surface* surface = SDL_GetWindowSurface(window);
    SDL_Event event;

    Core core(mem);
    Timer timer(mem);
    PPU ppu(mem, surface);
    APU apu(mem);
    core.bootup();
    apu.initAPU();

    bool running = true;
    bool first_frame = true;
    bool white = false;

#ifdef OLD
    std::ofstream log("oldlog.txt", std::ofstream::trunc);
#else
    std::ofstream log("newlog.txt", std::ofstream::trunc);
#endif
    u32 frame = 1;
    std::chrono::duration<double, std::micro> frameavg{};

    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = 48000;
    want.format = AUDIO_F32;
    want.channels = 1;
    want.samples = 1024;
    want.callback = &callback;
    want.userdata = &apu;

    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(nullptr, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
        SDL_PauseAudioDevice(dev, 0);
    
    frameStart = std::chrono::high_resolution_clock::now();
    constexpr static std::array<u8,4> tima_freq = { 9, 3, 5, 7 };
    while(running) {
        current_ticks = current_ticks - maxTicks;
        div_ticks = 0;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else joypad.pollPresses(event);
        }
        white = false;
        while (current_ticks < maxTicks) {
            u16 div = (mem.read(0xFF04) << 8) + mem.read(0xFF03);
            u8 tima_bit = (div >> tima_freq[mem.read(0xFF07) & 0b11]) & 0b1;
#ifdef DEBUG
            doctor_log(frame, current_ticks, log, core, mem);
#endif
            operation_ticks = core.op_tree();
            current_ticks += operation_ticks;
            if (mem.get_oam()) {
                mem.oam_transfer(current_ticks);
            }
            if ((mem.hw_read(0xFF40) & 0x80) == 0x80) {
                ppu.ppuLoop(operation_ticks);
            } else { // lcd disable
                mem.hw_write(0xFF44, (u8)0);
                mem.hw_write(0xFF41, (u8)((mem.hw_read(0xFF41) & (u8)0b11111100) | (u8)mode0));
                ppu.currentLineDots = 0;
                white = true;
            }
            div_ticks += operation_ticks;
            while (div_ticks >= 4) {
                timer.div_inc();
                apu.period_clock();
                div = (mem.read(0xFF04) << 8) + mem.read(0xFF03);
                u8 after_tima_bit = (div >> tima_freq[mem.read(0xFF07) & 0b11]) & 0b1; 
                if ((mem.read(0xFF07) > 3 && tima_flag) || ((tima_bit == 1) && (after_tima_bit == 0) && mem.read(0xFF07) > 3)) { // falling edge
                    tima_flag = (timer.tima_inc() == -1);
                }
                div_ticks -= 4;
                div = (mem.read(0xFF04) << 8) + mem.read(0xFF03);
                tima_bit = (div >> tima_freq[mem.read(0xFF07) & 0b11]) & 0b1;
            }
        }
        surface = SDL_GetWindowSurface(window);
        if (first_frame) {
            first_frame = false;
            SDL_FillRect(surface, nullptr, 0xFFFFFFFF);
        }
        if (white) {
            SDL_FillRect(surface, nullptr, 0xFFFFFFFF);
        }
        SDL_UpdateWindowSurface(window);
        frameTime = std::chrono::high_resolution_clock::now().time_since_epoch() - frameStart.time_since_epoch();
        if (frameDelay > frameTime) std::this_thread::sleep_for(std::chrono::duration(frameDelay - frameTime));
        frame += 1;
        frameavg += std::chrono::high_resolution_clock::now().time_since_epoch() - frameStart.time_since_epoch();;
        //std::cout << std::dec << (double)(std::chrono::high_resolution_clock::now().time_since_epoch() - frameStart.time_since_epoch()).count() / 1000000 << " ms for frame " << (int) frame << "\n";
        //assert(mem.read(0xFF44) >= 153);
        frameStart = std::chrono::high_resolution_clock::now();

    } 
    std::cout << "\n" << frameavg.count() / 1000 / frame << " avg ms per frame\n";
    std::cout << 1000000 / frameavg.count() * frame << " avg fps\n";
    std::cout << "closing gbemu\n";
    SDL_Quit();
}

void GB::doctor_log(u32 frame, u32 ticks, std::ofstream& log, Core& core, MMU& mem) {
    log << "Frame: " << std::dec << (int)frame;
    log << " Ticks: " << std::dec << (int)ticks;
    log << std::hex << std::setfill('0') << " A:" << std::setw(2) << (int) core.registers.gpr.n.a;
    log << " F:" << std::setw(2) << (int) core.registers.flags;
    log << " B:" <<  std::setw(2) << (int) core.registers.gpr.n.b;
    log << " C:" <<  std::setw(2) << (int) core.registers.gpr.n.c;
    log << " D:" <<  std::setw(2) << (int) core.registers.gpr.n.d;
    log << " E:" <<  std::setw(2) << (int) core.registers.gpr.n.e;
    log << " H:" <<  std::setw(2) << (int) core.registers.gpr.n.h;
    log << " L:" <<  std::setw(2) << (int) core.registers.gpr.n.l;
    log << " SP:" <<  std::setw(4) << (int) core.registers.sp;
    log << " SPMEM:" <<  std::setw(4) << (int) ((mem.hw_read(core.registers.sp + 1) << 8) + mem.hw_read(core.registers.sp));
    log << " PC:" <<  std::setw(4) << (int) core.registers.pc;
    log << " PCMEM:" <<  std::setw(2) << (int) mem.read(core.registers.pc) << ",";
    log <<  std::setw(2) << (int) mem.read(core.registers.pc + 1) << ",";
    log <<  std::setw(2) << (int) mem.read(core.registers.pc + 2) << ",";
    log <<  std::setw(2) << (int) mem.read(core.registers.pc + 3);
    log << "\n";
}

