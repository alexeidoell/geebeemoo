#pragma once
#include "../lib/types.h"
#include "mmu.h"
#include <array>
#include <memory>

class PPU {
    private:
        std::shared_ptr<MMU> mem;
        u8 xCoord;
        u8 yCoord;
    public:
        PPU(std::shared_ptr<MMU> memPtr) 
            :mem(memPtr) {}
        ~PPU();
        void pixelFetcher();
        u16 getTile(u8 index);
        std::array<u8, 23040> getBuffer(); // u8 array is a placeholder
};
