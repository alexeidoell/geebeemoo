#pragma once
#include <mmu.h>
#include <memory>

class Timer {
    private:
        MMU& mem;
        bool tima_flag = false;
        u8 tima_val = 0;
    public:
        Timer(MMU& mem) 
            : mem(mem) {}
        void div_inc();
        s8 tima_inc();
};
