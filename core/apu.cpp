#include <apu.h>
#include <bitset>
#include <iostream>
#include <mmu.h>
#include <types.h>


// digusting code
u8 APU::period_clock() {
    float sample = 0;
    float volume = mem->ppu_read(0xFF24) & 0b111; // currently only taking right ear volume into account
    volume = (volume + 1) / 8;
    if ((mem->ppu_read(0xFF10) & 0b1110000) == 0) { // pulse pace disabled
        ch1.pulse_pace = 0;
    } else if (ch1.pulse_pace == 0) { // internal pulse pace is 0, and new pace is written 
        ch1.pulse_pace = mem->ppu_read(0xFF10) >> 4;
    }

    if (div_raised == true) {
        if ((mem->ppu_read(0xFF04) & 0b10000) == 0) {
            div_raised = false; // falling edge increment apu div
            apu_div += 1;
            if (apu_div % 8 == 0) { // envelope
               envelopeAdjust();
            } 
            if (apu_div % 4 == 0) { // sound length
               lengthAdjust();
            }
            if (apu_div % 2 == 0) { // ch1 sweep
                periodSweep();
            }
        }
    } else if ((mem->ppu_read(0xFF04) & 0b10000) > 1) {
        div_raised = true;
    }

    // need to check if any channels have been triggered in the last m cycle
    if ((mem->ppu_read(0xFF19) & 0b10000000) > 1) { // ch2 triggered
        triggerCH2();
        mem->ppu_write(0xFF19, (u8)(mem->ppu_read(0xFF19) & 0b01111111));
    }
    
    if ((mem->ppu_read(0xFF14) & 0b10000000) > 1) { // ch1 triggered
        triggerCH1();
        mem->ppu_write(0xFF14, (u8)(mem->ppu_read(0xFF14) & 0b01111111));
    }
    if ((mem->ppu_read(0xFF1E) & 0b10000000) > 1) { // ch3 triggered
        triggerCH3();
        mem->ppu_write(0xFF1E, (u8)(mem->ppu_read(0xFF1E) & 0b01111111));
    }
    if ((mem->ppu_read(0xFF23) & 0b10000000) > 1) { // ch4 triggered
        triggerCH4();
        mem->ppu_write(0xFF23, (u8)(mem->ppu_read(0xFF23) & 0b01111111));
    }

    // dac disable
    // need to research how dac works more
    if (ch1.enabled && (mem->ppu_read(0xFF12) >> 3) == 0) {
        disableChannel(1);
        ch2.dac = false;
    } else {
        ch1.dac = true;
    }

    if (ch2.enabled && (mem->ppu_read(0xFF17) >> 3) == 0) {
        disableChannel(2);
        ch2.dac = false;
    } else {
        ch2.dac = true;
    }
    if (ch4.enabled && (mem->ppu_read(0xFF21) >> 3) == 0) {
        disableChannel(4);
        ch4.dac = false;
    } else {
        ch4.dac = true;
    }
    if (ch3.enabled && (mem->ppu_read(0xFF1A) >> 7) == 0) {
        disableChannel(3);
        ch3.dac = false;
    } else {
        ch3.dac = true;
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
        if (ch3.buffer.size() >= 8192) {
            ch3.buffer.pop();
        }
        if (ch4.buffer.size() >= 8192) {
            ch4.buffer.pop();
        }
        if ((mem->ppu_read(0xFF26) & 0b10000000) == 0) {
            ch1.duty_step = 0;
            ch2.duty_step = 0;
            ch3.duty_step = 0;
            ch4.duty_step = 0;
            ch4.lfsr = 0;
            ch2.buffer.push(0);
            ch1.buffer.push(0);
            ch3.buffer.push(0);
            ch4.buffer.push(0);
        } else {
            if (ch2.enabled && ch2.dac) {
                sample = duty_cycle[ch2_wave_duty][ch2.duty_step] * ch2.internal_volume;
                sample = 0 - (sample * (1.0/0xF));
                ch2.buffer.push(volume * sample);
            } else if (ch2.dac) {
                ch2.buffer.push(volume * 1.0);
            } else {
                ch2.buffer.push(0);
            }
            if (ch1.enabled && ch1.dac) {
                sample = duty_cycle[ch1_wave_duty][ch1.duty_step] * ch1.internal_volume;
                sample = 0 - (sample * (1.0/0xF));
                ch1.buffer.push(volume * sample);
            } else if (ch1.dac) {
                ch1.buffer.push(volume * 1.0);
            } else {
                ch1.buffer.push(0);
            }
            if (ch3.internal_volume == 0) {
                ch3.buffer.push(0);
            } else if (ch3.enabled && ch3.dac) {
                sample = getNibble();
                sample -= 7.5;
                sample /= 7.5;
                sample /= 1 << (ch3.internal_volume - 1);
                ch3.buffer.push(volume * sample);
            } else if (ch3.dac) {
                ch3.buffer.push(volume * 1.0);
            } else {
                ch3.buffer.push(0);
            }
            if (ch4.enabled && ch4.dac) {
                sample = ch4.output;
                sample = 0 - (ch4.output * (1.0/0xF));
                ch4.buffer.push(volume * sample);
            } else if (ch1.dac) {
                ch4.buffer.push(volume * 1.0);
            } else {
                ch4.buffer.push(0);
            }
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
    for (int i = 0; i < 2; ++i) {
        if (ch3.period_timer == 0x7FF) {
            ch3.period_timer = mem->ppu_read(0xFF1D) + ((mem->ppu_read(0xFF1E) & 0b111) << 8);
            if (ch3.duty_step == 31) ch3.duty_step = 0;
            else ch3.duty_step += 1;
        } else {
            ch3.period_timer += 1;
        }
    }
    if (ch4_tick == 3) {
        if (ch4.period_timer == ch4.clock_pace) {
            auto divider = (float)((int)(mem->ppu_read(0xFF22) & 0b111));
            divider = divider != 0 ? divider : 0.5;
            u16 shift = 2 << (mem->ppu_read(0xFF22) >> 4);
            ch4.clock_pace = shift * divider;
            ch4.period_timer = 0;
            // clock lfsr
            lfsrClock();
        } else {
            ch4.period_timer += 1;
        }
        ch4_tick = 0;
    } else {
        ch4_tick += 1;
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
    if (ch3.buffer.size() > 0) {
        sample += ch3.buffer.front();
        ch3.buffer.pop();
    }
    if (ch4.buffer.size() > 0) {
        sample += ch4.buffer.front();
        ch4.buffer.pop();
    }
    sample /= 4;
    return sample;
}

u8 APU::initAPU() {
    triggerCH1();
    triggerCH2();
    triggerCH3();
    triggerCH4();
    return 0;
}

u8 APU::triggerCH2() {
    ch2.enabled = true;
    mem->ppu_write(0xFF26, (u8)(mem->ppu_read(0xFF26) | 0b10));
    ch2.internal_volume = mem->ppu_read(0xFF17) >> 4;
    ch2.length_timer = mem->ppu_read(0xFF16) & 0b111111;
    ch2.env_dir = 1 & (mem->ppu_read(0xFF17) >> 3);
    ch2.duty_step = 0;
    return 0;
}

u8 APU::triggerCH1() {
    ch1.enabled = true;
    mem->ppu_write(0xFF26, (u8)(mem->ppu_read(0xFF26) | 0b1));
    ch1.internal_volume = mem->ppu_read(0xFF12) >> 4;
    ch1.length_timer = mem->ppu_read(0xFF11) & 0b111111;
    ch1.pulse_pace = mem->ppu_read(0xFF10) >> 4;
    ch1.env_dir = 1 & (mem->ppu_read(0xFF12) >> 3);
    ch1.duty_step = 0;
    return 0;
}

u8 APU::triggerCH3() {
    ch3.enabled = true;
    mem->ppu_write(0xFF26, (u8)(mem->ppu_read(0xFF26) | 0b100));
    ch3.internal_volume = mem->ppu_read(0xFF1C) >> 5;
    ch3.length_timer = mem->ppu_read(0xFF1B);
    ch3.duty_step = 1;
    return 0;
}

u8 APU::triggerCH4() {
    ch4.enabled = true;
    mem->ppu_write(0xFF26, (u8)(mem->ppu_read(0xFF26) | 0b1000));
    ch4.internal_volume = mem->ppu_read(0xFF21) >> 4;
    ch4.length_timer = mem->ppu_read(0xFF20) & 0b111111;
    ch4.env_dir = 1 & (mem->ppu_read(0xFF21) >> 3);
    auto divider = (float)((int)(mem->ppu_read(0xFF22) & 0b111));
    divider = divider != 0 ? divider : 0.5;
    u16 shift = 2 << (mem->ppu_read(0xFF22) >> 4);
    ch4.clock_pace = shift * divider;
    ch4.period_timer = 0;
    ch4.lfsr = 0;
    ch4.duty_step = 0;
    ch4.output = 0;
    return 0;
}

u8 APU::envelopeAdjust() {
    apu_div = 0;
    ch2.env_sweep_tick += 1;
    ch1.env_sweep_tick += 1;
    if (ch2.env_sweep_tick != 0 && ch2.env_sweep_tick == (mem->ppu_read(0xFF17) & 0b111)) {
        ch2.env_sweep_tick = 0;
        if (ch2.env_dir == 1) {
            if (ch2.internal_volume < 0xF) ch2.internal_volume += 1;
        } else {
            if (ch2.internal_volume > 0) ch2.internal_volume -= 1;
        }
    }
    if (ch1.env_sweep_tick != 0 && ch1.env_sweep_tick == (mem->ppu_read(0xFF12) & 0b111)) {
        ch1.env_sweep_tick = 0;
        if (ch1.env_dir == 1) {
            if (ch1.internal_volume < 0xF) ch1.internal_volume += 1;
        } else {
            if (ch1.internal_volume > 0) ch1.internal_volume -= 1;
        }
    }
    if (ch4.env_sweep_tick != 0 && ch4.env_sweep_tick == (mem->ppu_read(0xFF21) & 0b111)) {
        ch4.env_sweep_tick = 0;
        if (ch4.env_dir == 1) {
            if (ch4.internal_volume < 0xF) ch4.internal_volume += 1;
        } else {
            if (ch4.internal_volume > 0) ch4.internal_volume -= 1;
        }
    }

    return 0;
}

u8 APU::lengthAdjust() {
    if ((mem->ppu_read(0xFF14) & 0b1000000) > 0) {
        ch1.length_timer += 1;
        if (ch1.length_timer == 64) {
            disableChannel(1);
        }
    }
    if ((mem->ppu_read(0xFF19) & 0b1000000) > 0) {
        ch2.length_timer += 1;
        if (ch2.length_timer == 64) {
            disableChannel(2);
        }
    }
    if ((mem->ppu_read(0xFF1E) & 0b1000000) > 0) {
        ch3.length_timer += 1;
        if (ch3.length_timer == 64) {
            disableChannel(3);
        }
    }
    if ((mem->ppu_read(0xFF23) & 0b1000000) > 0) {
        ch4.length_timer += 1;
        if (ch4.length_timer == 64) {
            disableChannel(4);
        }
    }

    return 0;
}

u8 APU::periodSweep() {
    ch1.pulse_timer += 1;
    if (ch1.pulse_timer != 0 && ch1.pulse_timer == ch1.pulse_pace) {
        u16 current_period =  mem->ppu_read(0xFF13) + ((mem->ppu_read(0xFF14) & 0b111) << 8);
        u16 period_diff = current_period / (1 << (mem->ppu_read(0xFF14) & 0b111));
        if ((mem->ppu_read(0xFF14) & 0b1000) > 0) { // subtraction
            current_period -= period_diff;
            // if this is less than 0 this should completely break the sweep functionality
            // need to do more research
        } else {
            current_period += period_diff;
            if (current_period > 0x7FFF) {
                disableChannel(1);
            }
        }
        mem->ppu_write(0xFF13, (u8)(current_period & 0xFF));
        mem->ppu_write(0xFF14, (u8)((mem->ppu_read(0xFF14) & 0b11000000) + (current_period >> 8)));
        ch1.pulse_pace = mem->ppu_read(0xFF10) >> 4;
        ch1.pulse_timer = 0;
    }
    return 0;
}

u8 APU::disableChannel(u8 channel) {
    switch (channel) {
        case 1:
            ch1.enabled = false;
            mem->ppu_write(0xFF26, (u8)(mem->ppu_read(0xFF26) & 0b11111110));
            break;
        case 2:
            ch2.enabled = false;
            mem->ppu_write(0xFF26, (u8)(mem->ppu_read(0xFF26) & 0b11111101));
            break;
        case 3:
            ch3.enabled = false;
            mem->ppu_write(0xFF26, (u8)(mem->ppu_read(0xFF26) & 0b11111011));
            break;
        case 4:
            ch4.enabled = false;
            mem->ppu_write(0xFF26, (u8)(mem->ppu_read(0xFF26) & 0b11110111));
            break;
    }
    return 0;
}

u8 APU::getNibble() {
    u8 byte = mem->ppu_read(0xFF30 + ch3.duty_step / 2);
    if (ch3.duty_step % 2 == 1) {
        byte &= 0xF;
    } else {
        byte >>= 4;
    }
    return byte;
}

u8 APU::lfsrClock() {
    u8 bit0 = 0, bit1 = 0, new_bit = 0;
    bit0 = ch4.lfsr & 0b1;
    bit1 = (ch4.lfsr >> 1) & 0b1;
    new_bit = bit0 == bit1 ? 1 : 0;
    ch4.lfsr &= ~(0b1 << 15);
    ch4.lfsr |= (new_bit << 15);
    if ((mem->ppu_read(0xFF22) & 0b1000) > 0) { // short lfsr
        ch4.lfsr &= ~(0b1 << 7);
        ch4.lfsr |= (new_bit << 7);
    }

    if (bit0 == 1) {
        ch4.output = 0;
    } else {
        ch4.output = ch4.internal_volume;
    }

    ch4.lfsr >>= 1;
    return 0;
}
