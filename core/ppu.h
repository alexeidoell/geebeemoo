#pragma once
#include "../lib/types.h"
#include "mmu.h"
#include <SDL2/SDL_surface.h>
#include <array>
#include <iostream>
#include <memory>
#include <queue>

struct Pixel {
    u8 color;
    u8 palette;
    u8 spritePriority; // cgb only
    u8 bgPriority;
    Pixel(u8 color, u8 palette, u8 spritePriority, u8 bgPriority) : color(color), palette(palette), spritePriority(spritePriority), bgPriority(bgPriority) {}
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

struct FIFO {
    bool fetchTileID = false;
    bool fetchLowByte = false;
    bool fetchHighByte = false;
    bool awaitingPush = false;
    u16 tileAddress;
    u8 highByte;
    u8 lowByte;
    u16 objTileAddress;
    u8 objHighByte;
    u8 objLowByte;
};

enum tileType { bg, win, obj };

class PPU {
    private:
        const u32 colors[4] = { 0xFFFFFF, 0xAAAAAA, 0x555555, 0x000000 };
        std::shared_ptr<MMU> mem;
        u16 bgPixelFetcher();
        u16 winPixelFetcher();
        u8 getTileByte(u16 index);
        u16 combineTile(u8 tileHigh, u8 tileLow, tileType tiletype, Object * object);
        u8 pixelPicker();
        u8 modeSwitch();
        PPUState& ppuState;
        std::array<Object, 10> objArr;
        std::queue<Pixel> objQueue;
        std::queue<Pixel> bgQueue;
        u8 objFetchIdx;
        u8 xCoord;
        u8 mode3_delay = 0;
        FIFO fifoFlags;
        Window window;
        std::array<u8, 23040> frameBuffer = { 0 };
        u8 oamScan(u16 address);
        bool firstTile = true;
        void setPixel(u8 w, u8 h, u8 pixel);
        SDL_Surface *surface;
        bool newTile = true;
        s16 finishedLineDots;
    public:
        s16 currentLineDots; // need to keep track of state between
                             // calls so that the ppu can tell the
                             // memory or cpu what not to do before
                             // the next call of the ppu loop that will
                             // resume the state it was at in the current
                             // line
        PPU(std::shared_ptr<MMU> memPtr, SDL_Surface* surface) 
        :mem(memPtr), ppuState(mem->ppuState), surface(surface){
        } // this feels gross
        u8 ppuLoop(u8 ticks);
        std::array<u8, 23040>& getBuffer(); // u8 array is a placeholder
};
