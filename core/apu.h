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
    u16 period_timer = 0x7FF;
    u8 duty_step = 0;
    u8 internal_volume = 0;
    std::queue<float> buffer;
};

class APU {
private:
    u32 sample_counter = 0;
    channel2 ch2;
    std::shared_ptr<MMU> mem;
public:
    APU(std::shared_ptr<MMU> mem) : mem(std::move(mem)) {};
    u8 period_clock();
    u8 initAPU();
    u8 triggerCH2();
    float getSample();
};
