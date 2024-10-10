#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <queue>
#include <types.h>
#include <mmu.h>

constexpr u16 SDL_BUFFER_SIZE = 1024;

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
    u32 sample_counter = 0;
    bool div_raised = false;
    u8 apu_div = 0;
    u8 ch4_tick = 0;
    SDL_Mutex* buffer_lock = nullptr;
    SDL_AudioStream* audio_stream = nullptr;
public:
    APU(MMU& mem, SDL_Mutex* mutex) : mem(mem), buffer_lock(mutex) {};
    void period_clock();
    void initAPU();
    void triggerCH2();
    void triggerCH1();
    void triggerCH3();
    void triggerCH4();
    void putSample();
    u8 getNibble();
    void lfsrClock();
    void envelopeAdjust();
    void lengthAdjust();
    void periodSweep();
    void disableChannel(u8 channel);
    SDL_Mutex* getMutex() {
        return buffer_lock;
    }
    void setAudioStream(SDL_AudioStream* new_stream);
};
