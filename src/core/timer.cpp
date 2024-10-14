#include <timer.h>
#include <mmu.h>

void Timer::div_inc() {
    DIV += 4;
}

s8 Timer::tima_inc() {
    /*
    u16 div = mem.hw_read(0xFF03);
    div = (mem.hw_read(0xFF04) << 8) + div;
    */
    if (TIMA == 0xFF) {
        tima_flag = true;
        TIMA = 0;
        tima_val = TMA;
        return -1;
    } else if ((TIMA == 0) && tima_flag) {
        tima_flag = false;
        TIMA = tima_val | 0b100;
    } else {
        TIMA += 1;
    }
    return 0;
}

