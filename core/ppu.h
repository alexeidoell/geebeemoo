#pragma once
#include "../lib/types.h"
#include "mmu.h"
#include <array>
#include <memory>

class PPU {
    private:
        std::shared_ptr<MMU> mem;
        u16 currentLineDots; // need to keep track of state between
                             // calls so that the ppu can tell the
                             // memory or cpu what not to do before
                             // the next call of the ppu loop that will
                             // resume the state it was at in the current
                             // line
    public:
        PPU(std::shared_ptr<MMU> memPtr) 
            :mem(memPtr) {}
        ~PPU();
        void pixelFetcher();
        u16 getTile(u8 index);
        std::array<u8, 23040> getBuffer(); // u8 array is a placeholder
};
