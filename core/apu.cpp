#include <apu.h>
#include <mmu.h>
#include <types.h>


u8 APU::period_clock() {
    u8 ch2_wave_duty = mem->ppu_read(0xFF16) >> 6;
    ch2.buffer.push(duty_cycle[ch2_wave_duty][ch2.duty_step]);
    if (ch2.period_timer == 0x7FF) {
        ch2.period_timer = mem->ppu_read(0xFF18) + ((mem->ppu_read(0xFF19) & 0b111) << 8);
        if (ch2.duty_step == 7) ch2.duty_step = 0;
    } else {
        ch2.period_timer += 1;
        ch2.duty_step += 1;
    }
    return 0;
}

float APU::getSample() {
    for (auto i = 0; i < 22; ++i) {
        if (ch2.buffer.size() > 1) ch2.buffer.pop();
    }
    float sample = 0;
    if (ch2.buffer.size() > 0) {
        sample = ch2.buffer.front();
        ch2.buffer.pop();
    }
    return sample;
}
