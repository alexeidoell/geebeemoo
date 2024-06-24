#pragma once
#include "../lib/types.h"
#include "mmu.h"
#include <array>
#include <iostream>
#include <memory>
#include <queue>

struct Pixel {
    u8 color : 2;
    u8 palette : 3;
    u8 spritePriority : 1; // cgb only
    u8 bgPriority : 1;
    Pixel(u8 color, u8 palette, u8 bgPriority) : color(color), palette(palette), spritePriority(0), bgPriority(bgPriority) {}
};

struct Window {
    bool WY_cond = false;
    bool WX_cond = false;
    u8 yCoord;
    u8 xCoord;
};

struct Object {
    u8 yPos;
    u8 xPos;
    u8 tileIndex;
    u8 flags;
    Object() = default;
    Object(u8 yPos, u8 xPos, u8 tileIndex, u8 flags) : yPos(yPos), xPos(xPos), tileIndex(tileIndex), flags(flags) {}
};

class PPU {
    private:
        std::shared_ptr<MMU> mem;
       void pixelFetcher();
        u16 getTile(u16 index); // only gets tile data for a single line
        PPUState& ppuState;
        std::array<Object, 10> objArr;
        std::queue<Pixel> objQueue;
        std::queue<Pixel> bgQueue;
        u8 objFetchIdx;
        u8 xCoord = 0;
        u8 yCoord = 0;
        Window window;
        std::array<u8, 23040> frameBuffer;
        u8 printedPixels = 0;
        u8 oamScan(u16 address);
    public:
        u16 currentLineDots; // need to keep track of state between
                             // calls so that the ppu can tell the
                             // memory or cpu what not to do before
                             // the next call of the ppu loop that will
                             // resume the state it was at in the current
                             // line
        PPU(std::shared_ptr<MMU> memPtr) 
        :mem(memPtr), ppuState(mem->ppuState) {
        } // this feels gross
        ~PPU();
        u8 ppuLoop(u8 ticks);
        std::array<u8, 23040>& getBuffer(); // u8 array is a placeholder
};
