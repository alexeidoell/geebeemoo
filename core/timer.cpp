#include "timer.h"
#include "mmu.h"


u8 Timer::div_inc() {
    u16 div = mem->read(0xFF03);
    div = (mem->read(0xFF04) << 8) + div;
    div += 4;
    mem->write(0xFF03, div);
    return 0;
}
s8 Timer::tima_inc() {
    if (mem->read(0xFF05) == 255) {
        tima_flag = true;
        mem->write(0xFF05, (u8)0x00);
        tima_val = mem->read(0xFF06);
        return -1;
    } else if (tima_flag) {
        tima_flag = false;
        mem->write(0xFF05, (u8)(tima_val));
        mem->write(0xFF0F, (u8)(mem->read(0xFF0F) | 0b100));
    } else {
        mem->write(0xFF05, (u8)(mem->read(0xFF05) + 1));
    }
    return 0;
}

