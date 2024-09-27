#include <apu.h>
#include <mmu.h>
#include <types.h>


u8 APU::period_clock() {
    // need to check if any channels have been triggered in the last m cycle
    
    if (div_raised == true) {
        if ((mem->ppu_read(0xFF04) & 0b10000) == 0) {
            div_raised = false; // falling edge increment apu div
            apu_div += 1;
            if (apu_div % 8 == 0) { // envelope
               envelopeAdjust();
            } 
            if (apu_div % 4 == 0) { // sound length

            }
            if (apu_div % 2 == 0) { // ch1 sweep

            }
        }
    } else if ((mem->ppu_read(0xFF04) & 0b10000) > 1) {
        div_raised = true;
    }

    if ((mem->ppu_read(0xFF19) & 0b10000000) > 1) { // ch2 triggered
        triggerCH2();
        mem->ppu_write(0xFF19, (u8)(mem->ppu_read(0xFF19) & 0b01111111));
    }
    
    if ((mem->ppu_read(0xFF14) & 0b10000000) > 1) { // ch1 triggered
        triggerCH1();
        mem->ppu_write(0xFF14, (u8)(mem->ppu_read(0xFF14) & 0b01111111));
    }

    u8 ch2_wave_duty = mem->ppu_read(0xFF16) >> 6;
    u8 ch1_wave_duty = mem->ppu_read(0xFF11) >> 6;
    sample_counter += 48000;
    if (sample_counter >= 1048576) {
        if (ch2.buffer.size() >= 8192) {
            ch2.buffer.pop();
        }
        if (ch1.buffer.size() >= 8192) {
            ch1.buffer.pop();
        }
        if ((mem->ppu_read(0xFF26) & 0b10000000) == 0) {
            ch2.buffer.push(0);
            ch1.buffer.push(0);
        } else {
            ch2.buffer.push(0.1 * ch2.internal_volume * duty_cycle[ch2_wave_duty][ch2.duty_step]);
            ch1.buffer.push(0.1 * ch1.internal_volume * duty_cycle[ch1_wave_duty][ch1.duty_step]);
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
    if (ch1.period_timer == 0x7FF) {
        ch1.period_timer = mem->ppu_read(0xFF13) + ((mem->ppu_read(0xFF14) & 0b111) << 8);
        if (ch1.duty_step == 7) ch1.duty_step = 0;
        else ch1.duty_step += 1;
    } else {
        ch1.period_timer += 1;
    }
    return 0;
}

float APU::getSample() {
    float sample = 0;
    if (ch2.buffer.size() > 0) {
        sample += ch2.buffer.front();
        ch2.buffer.pop();
    }
    if (ch1.buffer.size() > 0) {
        sample += ch1.buffer.front();
        ch1.buffer.pop();
    }
    return sample;
}

u8 APU::initAPU() {
    ch2.internal_volume = mem->ppu_read(0xFF17) >> 4;
    ch1.internal_volume = mem->ppu_read(0xFF12) >> 4;
    return 0;
}

u8 APU::triggerCH2() {
    ch2.internal_volume = mem->ppu_read(0xFF17) >> 4;
    ch2.duty_step = 0;
    return 0;
}

u8 APU::triggerCH1() {
    ch1.internal_volume = mem->ppu_read(0xFF12) >> 4;
    ch1.duty_step = 0;
    return 0;
}

u8 APU::envelopeAdjust() {
    apu_div = 0;
    ch2.env_sweep_tick += 1;
    ch1.env_sweep_tick += 1;
    if (ch2.env_sweep_tick != 0 && ch2.env_sweep_tick == (mem->ppu_read(0xFF17) & 0b111)) {
        ch2.env_sweep_tick = 0;
        if ((mem->ppu_read(0xFF17) & 0b1000) > 0) {
            if (ch2.internal_volume < 0xF) ch2.internal_volume += 1;
        } else {
            if (ch2.internal_volume > 0) ch2.internal_volume -= 1;
        }
    }
    if (ch1.env_sweep_tick != 0 && ch1.env_sweep_tick == (mem->ppu_read(0xFF12) & 0b111)) {
        ch1.env_sweep_tick = 0;
        if ((mem->ppu_read(0xFF12) & 0b1000) > 0) {
            if (ch1.internal_volume < 0xF) ch1.internal_volume += 1;
        } else {
            if (ch1.internal_volume > 0) ch1.internal_volume -= 1;
        }
    }

    return 0;
}