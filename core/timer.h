#pragma once
#include "mmu.h"
#include <memory>

class Timer {
    private:
        std::shared_ptr<MMU> mem;
        bool tima_flag = false;
        u8 tima_val;
    public:
        Timer(std::shared_ptr<MMU> memPtr) 
            :mem(memPtr) {}
        u8 div_inc();
        s8 tima_inc();
};
