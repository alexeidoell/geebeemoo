#pragma once
#include <lib/types.h>

class Timer {
    private:
        bool tima_flag = false;
        u8 tima_val = 0;
    public:
        bool timer_interrupt = false;
        u16 DIV = 0;
        u8 TIMA = 0;
        u8 TMA = 0;
        u8 TAC = 0;
        void div_inc();
        s8 tima_inc();
};
