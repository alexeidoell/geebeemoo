#include <timer.h>
#include <mmu.h>

void Timer::div_inc() {
    u16 div = mem.hw_read(0xFF03);
    div = (mem.hw_read(0xFF04) << 8) + div;
    div += 4;
    mem.hw_dwrite(0xFF03, div);
}

s8 Timer::tima_inc() {
    /*
    u16 div = mem.hw_read(0xFF03);
    div = (mem.hw_read(0xFF04) << 8) + div;
    */
    if (mem.hw_read(0xFF05) == 0xFF) {
        tima_flag = true;
        mem.hw_write(0xFF05, 0x00);
        tima_val = mem.hw_read(0xFF06);
        return -1;
    } else if ((mem.hw_read(0xFF05) == 0) && tima_flag) {
        tima_flag = false;
        mem.hw_write(0xFF05, (tima_val));
        mem.hw_write(0xFF0F, (mem.hw_read(0xFF0F) | 0b100));
    } else {
        mem.hw_write(0xFF05, (mem.hw_read(0xFF05) + 1));
    }
    return 0;
}

