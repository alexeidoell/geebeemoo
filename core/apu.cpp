#include <apu.h>
#include <mmu.h>
#include <types.h>


u8 APU::period_clock() {
    u8 ch2_wave_duty = mem->ppu_read(0xFF16) >> 6;
    sample_counter += 48000;
    if (sample_counter >= 1048576) {
        if (ch2.buffer.size() >= 8192) {
            ch2.buffer.pop();
        }
        if ((mem->ppu_read(0xFF26) & 0b10000000) == 0) {
            ch2.buffer.push(0);
        } else {
            ch2.buffer.push(0.1 * (mem->ppu_read(0xFF17) >> 4) * duty_cycle[ch2_wave_duty][ch2.duty_step]);
        }
        sample_counter -= 1048576;
    }
    if (ch2.period_timer == 0x7FF) {
        ch2.period_timer = mem->ppu_read(0xFF18) + ((mem->ppu_read(0xFF19) & 0b111) << 8);
        if (ch2.duty_step == 7) ch2.duty_step = 0;
        else ch2.duty_step += 1;
    } else {
        ch2.period_timer += 1;
    }
    return 0;
}

float APU::getSample() {
    float sample = 0;
    if (ch2.buffer.size() > 0) {
        sample = ch2.buffer.front();
        ch2.buffer.pop();
    }
    return sample;
}
