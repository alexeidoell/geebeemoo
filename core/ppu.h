#pragma once
#include <lib/types.h>
#include <mmu.h>
#include <SDL2/SDL_surface.h>
#include <array>
#include <memory>
#include <queue>

struct Pixel {
    u8 color = 0;
    u8 palette = 0;
    u8 spritePriority = 0; // cgb only
    u8 bgPriority = 0;
    u8 xCoord = 0;
    u8 objIndex = 0;
    Pixel(u8 color, u8 palette, u8 spritePriority, u8 bgPriority, u8 xCoord, u8 objIndex) : color(color), palette(palette), spritePriority(spritePriority), bgPriority(bgPriority), xCoord(xCoord), objIndex(objIndex) {}
};

struct Window {
    bool WY_cond = false;
    bool WX_cond = false;
    u8 yCoord = 0;
    u8 xCoord = 0;
};

struct Object {
    u8 yPos = 0;
    u8 xPos = 0;
    u8 tileIndex = 0;
    u8 flags = 0;
    Object() = default;
    u8 objIndex = 0;
    Object(u8 yPos, u8 xPos, u8 tileIndex, u8 flags, u8 objIndex) : yPos(yPos), xPos(xPos), tileIndex(tileIndex), flags(flags), objIndex(objIndex) {}
};

struct FIFO {
    bool fetchTileID = false;
    bool fetchLowByte = false;
    bool fetchHighByte = false;
    bool awaitingPush = false;
    u16 tileAddress = 0;
    u8 highByte = 0;
    u8 lowByte = 0;
    u16 objTileAddress = 0;
    u8 objHighByte = 0;
    u8 objLowByte = 0;
};

enum tileType { bg, win, obj };

class PPU {
    private:
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
        u8 objFetchIdx = 0;
        u8 xCoord = 0;
        u8 mode3_delay = 0;
        FIFO fifoFlags;
        Window window;
        std::array<u8, 23040> frameBuffer = { 0 };
        u8 oamScan(u16 address);
        bool firstTile = true;
        void setPixel(u8 w, u8 h, u8 pixel);
        SDL_Surface *surface;
        bool newTile = true;
        s16 finishedLineDots = 0;
        u8 statInterruptHandler();
        bool statIRQ = false;
    public:
        s16 currentLineDots = 0; // need to keep track of state between
                             // calls so that the ppu can tell the
                             // memory or cpu what not to do before
                             // the next call of the ppu loop that will
                             // resume the state it was at in the current
                             // line
        PPU(std::shared_ptr<MMU> memPtr, SDL_Surface* surface) 
        :mem(std::move(memPtr)), ppuState(mem->ppuState), surface(surface){
        } // this feels gross
        u8 ppuLoop(u8 ticks);
        std::array<u8, 23040>& getBuffer(); // u8 array is a placeholder
};
