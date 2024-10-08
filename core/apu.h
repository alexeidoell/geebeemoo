#include <SDL2/SDL_mutex.h>
#include <algorithm>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <types.h>
#include <mmu.h>

#define SDL_BUFFER_SIZE 1024

constexpr float duty_cycle[4][8] = { // NOLINT
    {1,1,1,1,1,1,1,-1},
    {-1,1,1,1,1,1,1,-1},
    {-1,1,1,1,1,-1,-1,-1},
    {1,-1,-1,-1,-1,-1,-1,1}
};

class APU {
private:
    MMU& mem;
    // THERE HAS GOT TO BE A BETTER WAY TO DO THIS
    struct {
        bool enabled = true;
        bool dac = true;
        u16 period_timer = 0x7FF;
        u8 duty_step = 0;
        u8 internal_volume = 0;
        u8 env_sweep_tick = 0;
        u8 length_timer = 0;
        u8 env_dir = 0;
        u8 pulse_timer = 0;
        u8 pulse_pace = 0;
        std::queue<float> buffer;
    } ch1;
    struct {
        bool enabled = true;
        bool dac = true;
        u16 period_timer = 0x7FF;
        u8 duty_step = 0;
        u8 internal_volume = 0;
        u8 length_timer = 0;
        u8 env_dir = 0;
        u8 env_sweep_tick = 0;
        std::queue<float> buffer;
    } ch2;
    struct {
        bool enabled = true;
        bool dac = true;
        u16 period_timer = 0x7FF;
        u8 duty_step = 0;
        u8 internal_volume = 0;
        u16 length_timer = 0;
        std::queue<float> buffer;
    } ch3;
    struct {
        bool enabled = true;
        bool dac = true;
        u16 period_timer = 0x7FF;
        u8 duty_step = 0;
        u8 internal_volume = 0;
        u8 length_timer = 0;
        u8 env_dir = 0;
        u8 env_sweep_tick = 0;
        u16 lfsr = 0;
        u8 output = 0;
        u16 clock_pace = 0;
        std::queue<float> buffer;
    } ch4;
    SDL_mutex* buffer_lock;
    u32 sample_counter = 0;
    bool div_raised = false;
    u8 apu_div = 0;
    bool ch3_tick = false;
    u8 ch4_tick = 0;
    u32 buffer_size = 0;
public:
    APU(MMU& mem) : mem(mem) {
        buffer_lock = SDL_CreateMutex();
    };
    ~APU() {
        // signal awaiting_buffer
        SDL_DestroyMutex(buffer_lock);
    }
    void period_clock();
    void initAPU();
    void triggerCH2();
    void triggerCH1();
    void triggerCH3();
    void triggerCH4();
    float getSample();
    u8 getNibble();
    void lfsrClock();
    void envelopeAdjust();
    void lengthAdjust();
    void periodSweep();
    void disableChannel(u8 channel);
    SDL_mutex* getMutex() {
        return buffer_lock;
    }
};
