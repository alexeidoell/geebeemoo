#include <memory>
#include <queue>
#include <types.h>
#include <mmu.h>

constexpr float duty_cycle[4][8] = { // NOLINT
    {1,1,1,1,1,1,1,-1},
    {-1,1,1,1,1,1,1,-1},
    {-1,1,1,1,1,-1,-1,-1},
    {1,-1,-1,-1,-1,-1,-1,1}
};

struct channel2 {
    bool enabled = true;
    bool dac = true;
    u16 period_timer = 0x7FF;
    u8 duty_step = 0;
    u8 internal_volume = 0;
    u8 length_timer = 0;
    u8 env_dir = 0;
    u8 env_sweep_tick = 0;
    std::queue<float> buffer;
};

struct channel1 {
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
};

struct channel3 {
    bool enabled = true;
    bool dac = true;
    u16 period_timer = 0x7FF;
    u8 duty_step = 0;
    u8 internal_volume = 0;
    u8 length_timer = 0;
    std::queue<float> buffer;
};



class APU {
private:
    u32 sample_counter = 0;
    channel1 ch1;
    channel2 ch2;
    channel3 ch3;
    std::shared_ptr<MMU> mem;
    bool div_raised = false;
    u8 apu_div = 0;
    bool ch3_tick = false;
public:
    APU(std::shared_ptr<MMU> mem) : mem(std::move(mem)) {};
    u8 period_clock();
    u8 initAPU();
    u8 triggerCH2();
    u8 triggerCH1();
    u8 triggerCH3();
    float getSample();
    u8 getNibble();
    u8 envelopeAdjust();
    u8 lengthAdjust();
    u8 periodSweep();
    u8 disableChannel(u8 channel);
};
