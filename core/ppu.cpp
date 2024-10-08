#include <ppu.h>
#include <mmu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <cassert>

void PPU::combineTile(u8 tileHigh, u8 tileLow, tileType tiletype, Object * object) {
    u16 line = 0;
    u16 mask = 0;
    line = 0;

    bool flipCond = false;
    if (tiletype == obj) flipCond = (object->flags & 0b100000) > 0;

    for (auto j = 0; j < 8; ++j) {
        mask = 0b1 << j;
        line += (((u16)tileHigh & mask) << (j + 1)) + (((u16)tileLow & mask) << j);
    }
    
    if (flipCond) {
        u16 tempLine = line;
        line = 0;
        for (auto j = 7; j >= 0; --j) {
            line = (line << 2) | (tempLine & 0b11);
            tempLine >>= 2;
        }
    }

    //std::cout << std::setw(4) << std::setfill('0') << std::hex << (int)line << '\n';
    u8 objQueueSize = objQueue.size();

    for (auto i = 0; i < 8; ++i) {
        u16 mask = 0b11 << 14;
        mask >>= (i * 2);
        u16 color = line & mask;
        color >>= (14 - (i * 2));
        u8 palette = 0;
        u8 bgPriority = 0;
        u8 xCoord = 0;
        u8 objIndex = 0;
        if (tiletype == obj)  {
            xCoord = object->xPos;
            palette = (object->flags & 0b10000) >> 4;
            bgPriority = (object->flags & 0b10000000) >> 7;
            objIndex = object->objIndex;
        }
        Pixel pixel(color, palette, 0, bgPriority, xCoord, objIndex);
        if (tiletype == bg) {
            if (bgQueue.size() < 8) bgQueue.push(pixel);
            assert(bgQueue.size() <= 8);
        } else if (tiletype == obj) {
            if (i < objQueueSize) {
                Pixel oldPixel = objQueue.front();
                objQueue.pop();
                if (oldPixel.color == 0 || oldPixel.xCoord > pixel.xCoord) objQueue.push(pixel);
                else {
                    if (oldPixel.xCoord == pixel.xCoord && pixel.xCoord < oldPixel.xCoord) {
                        objQueue.push(pixel);
                    } else {
                        objQueue.push(oldPixel);
                    }
                }
            } else objQueue.push(pixel);

        }
    }
}

u8 PPU::getTileByte(u16 index) { // 2 dots
    return mem.hw_read(index); 
}

u16 PPU::bgPixelFetcher() { //  2 dots
    u16 tileMap = 0x9800; 
    if ((mem.hw_read(LCDC) & 0b1000) > 0) { 
        tileMap = 0x9C00;
    }
    u16 offset = 0xFF & ((u8)(mem.hw_read(LY) + mem.hw_read(SCY)) / 8);
    offset <<= 5;
    offset |= 0x1F & ((u8)(xCoord + mem.hw_read(SCX)) / 8);
    offset &= 0x3FF;
    u16 tileID = mem.hw_read(tileMap + offset);
    u16 tileAddress = (tileID) << 4;
    tileAddress += (((u16)mem.hw_read(LY) + (u16)mem.read(SCY)) % 8) << 1;
    tileAddress += 0b1 << 15;
    bool addressing_method = (mem.hw_read(LCDC) & 0b10000) == 0b10000;
    if (!addressing_method && (tileID & 0x80) == 0) {
        tileAddress += (0b1 << 12);
    }
    return tileAddress;
}

u16 PPU::winPixelFetcher() { 
    u16 tileMap = 0x9800; 
    if ((mem.hw_read(LCDC) & 0b1000000) > 0) { 
        tileMap = 0x9C00;
    }
    u16 offset = 0xFF & (window.yCoord / 8);
    offset <<= 5;
    offset += 0x1F & (window.xCoord / 8);
    u8 tileID = mem.hw_read(tileMap + offset);
    u16 tileAddress = ((u16)tileID) << 4;
    tileAddress += (window.yCoord % 8) << 1;
    tileAddress += 0b1 << 15;
    bool addressing_method = (mem.hw_read(LCDC) & 0b10000) == 0b10000;
    if (!addressing_method && (tileID & 0x80) == 0) {
        tileAddress += (0b1 << 12);
    }
    return tileAddress;
}

void PPU::ppuLoop(u8 ticks) {
    currentLineDots += ticks;
    statInterruptHandler();
    u8 currentLine = mem.hw_read(LY); // ly register 
    while (mem.hw_read(LY) < 144 && finishedLineDots < currentLineDots) {
        if (finishedLineDots >= 456) {
            break;
        }
        if (finishedLineDots < 80 && finishedLineDots < currentLineDots) {
            if (mem.read(WY) <= currentLine) {
                window.WY_cond = true;
            }
            while (finishedLineDots < 80 && finishedLineDots < currentLineDots) {
                u16 address = 0xFEA0 - 2 * (80 - finishedLineDots);
                oamScan(address);
                finishedLineDots += 2;
            }
        } 
        if (finishedLineDots >= 80 && finishedLineDots < 172 + 80 + mode3_delay && finishedLineDots < currentLineDots) {
            ppu_state = mode3;
            if (finishedLineDots == 80) { // setting up mode3
                while(!bgQueue.empty()) bgQueue.pop();
                while(!objQueue.empty()) objQueue.pop();
            }
            while (finishedLineDots < 86 && finishedLineDots < currentLineDots) { // do nothing to simulate discarded tile
                finishedLineDots += 2;
            }
            while (finishedLineDots >= 86 && finishedLineDots < 92 && finishedLineDots < currentLineDots) { // initial fetches
                if (!fifoFlags.fetchTileID) {
                    fifoFlags.tileAddress = bgPixelFetcher();
                    fifoFlags.fetchTileID = true;
                    finishedLineDots += 2;
                } else if (!fifoFlags.fetchHighByte) {
                    fifoFlags.highByte = getTileByte(fifoFlags.tileAddress + 1);
                    fifoFlags.fetchHighByte = true;
                    finishedLineDots += 2;
                } else if (!fifoFlags.fetchLowByte) {
                    fifoFlags.lowByte = getTileByte(fifoFlags.tileAddress);
                    fifoFlags.fetchLowByte = true;
                    fifoFlags.awaitingPush = true;
                    finishedLineDots += 2;
                }
            }
            if (finishedLineDots == 92) { // first pixel push
                if (fifoFlags.awaitingPush) {
                    combineTile(fifoFlags.highByte, fifoFlags.lowByte, bg, nullptr);
                    newTile = true;
                    fifoFlags.fetchLowByte = false;
                    fifoFlags.fetchHighByte = false;
                    fifoFlags.fetchTileID = false;
                }
            }
            while (finishedLineDots >= 92 && finishedLineDots < 172 + 80 + mode3_delay && finishedLineDots < currentLineDots) { // normal mode3 cycle
                if (!fifoFlags.awaitingPush) { // get next push hw_ready
                    fifoFlags.tileAddress = bgPixelFetcher();
                    fifoFlags.highByte = getTileByte(fifoFlags.tileAddress + 1);
                    fifoFlags.lowByte = getTileByte(fifoFlags.tileAddress);
                    fifoFlags.awaitingPush = true;
                }
                if (!window.WX_cond && bgQueue.empty() && fifoFlags.awaitingPush) {
                    // push new tile row
                    combineTile(fifoFlags.highByte, fifoFlags.lowByte, bg, nullptr);
                    newTile = true;
                    fifoFlags.awaitingPush = false;
                    fifoFlags.fetchLowByte = false;
                    fifoFlags.fetchHighByte = false;
                    fifoFlags.fetchTileID = false;
                }
                if (firstTile) {
                    for (auto i = 0; i < (mem.hw_read(SCX) % 8); ++i) {
                        bgQueue.pop();
                        //xCoord += 1;
                        mode3_delay += 1;
                    }
                }
                for (auto i = 0; i < objFetchIdx; ++i) {
                    if (xCoord == objArr[i].xPos) {
                        mode3_delay += 6;
                        if (newTile) {
                            newTile = false;
                            s8 objPenalty = bgQueue.size();
                            objPenalty -= 2;
                            objPenalty = (objPenalty < 0) ? 0 : objPenalty;
                            mode3_delay += objPenalty;
                        }
                        fifoFlags.objTileAddress = 0x8000;                       
                        fifoFlags.objTileAddress += ((u16)objArr[i].tileIndex) << 4; // assumes tile is not flipped
                        bool flipCond = (objArr[i].flags & 0b1000000) > 0;
                        if (flipCond) {
                            fifoFlags.objTileAddress += (~(((currentLine - (objArr[i].yPos - 16)) % 8) << 1)) & 0b1110;
                        } else fifoFlags.objTileAddress += ((currentLine - (objArr[i].yPos - 16)) % 8) << 1;
                        if ((mem.hw_read(LCDC) & 0b100) > 0) { // 8x16 tiles
                            if ((objArr[i].yPos - currentLine) > 8) {
                                if (!flipCond) {
                                    fifoFlags.objTileAddress &= ~((u16)0b10000);
                                } else fifoFlags.objTileAddress |= ((u16)0b10000);
                            } else {
                                if (flipCond) {
                                    fifoFlags.objTileAddress &= ~((u16)0b10000);
                                } else fifoFlags.objTileAddress |= ((u16)0b10000);
                            }
                        }
                        fifoFlags.objHighByte = getTileByte(fifoFlags.objTileAddress + 1);
                        fifoFlags.objLowByte = getTileByte(fifoFlags.objTileAddress);
                        combineTile(fifoFlags.objHighByte, fifoFlags.objLowByte, obj, &objArr[i]);
                    }
                }
                if (xCoord < 168 && ((mem.read(LCDC) & 0b100000) > 0) && window.WY_cond && (xCoord - 1 == mem.read(WX) || window.WX_cond)) { // window time
                    if (bgQueue.empty() || window.WX_cond == false) {
                        while (!bgQueue.empty()) bgQueue.pop();
                        fifoFlags.tileAddress = winPixelFetcher();
                        window.xCoord += 8;
                        fifoFlags.highByte = getTileByte(fifoFlags.tileAddress + 1);
                        fifoFlags.lowByte = getTileByte(fifoFlags.tileAddress);
                        combineTile(fifoFlags.highByte, fifoFlags.lowByte, bg, nullptr);
                        fifoFlags.awaitingPush = true;
                    }
                    if (!window.WX_cond) {
                        mode3_delay += 6;
                    }
                    window.WX_cond = true;
                }
                if (xCoord > 7 && xCoord < 168) {
                    setPixel(xCoord - 8, currentLine, pixelPicker());
                    finishedLineDots += 1;
                }
                if (xCoord < 168) xCoord += 1;
                else finishedLineDots += 1;
                if (firstTile) firstTile = false;
                if (!objQueue.empty()) objQueue.pop();
                if (!bgQueue.empty()) bgQueue.pop();
            }
        }
        if (finishedLineDots >= 172 + 80 + mode3_delay && finishedLineDots < 456 && finishedLineDots < currentLineDots) { // hblank
            ppu_state = mode0;
            firstTile = true;
            while (finishedLineDots < currentLineDots) {
                finishedLineDots += 2;
            }
        }
    }
    if (currentLineDots >= 456) {
        window.WY_cond = false;
        if (window.WX_cond) {
            window.yCoord += 1;
        }
        window.WX_cond = false;
        if (currentLine == 153) {
            currentLine = 0;
            ppu_state = mode2;
        }
        else currentLine += 1;
        mem.hw_write(LY, (u8)(currentLine));
        if (currentLine == mem.hw_read(LYC)) { // ly = lyc
            mem.hw_write(STAT, (u8)(mem.hw_read(STAT) | 0b100));
        } else mem.hw_write(STAT, (u8)(mem.hw_read(STAT) & 0b11111011));
        window.xCoord = 0;
        xCoord = 0;
        mode3_delay = 0;
        objArr = {};
        objFetchIdx = 0;
        currentLineDots -= 456;
        finishedLineDots = 0; // idk tbh?
        if (currentLine == 144) { // vblank
            ppu_state = mode1;
            window.yCoord = 0;
            mem.hw_write(IF, (u8)(mem.hw_read(IF) | 0b1));
        }
        if (ppu_state != mode1) { 
            ppu_state = mode2;
        } 
        statInterruptHandler();
    }
    //std::cout << (int)finishedLineDots << " " << (int)currentLineDots << " " << (int)ticks << " " << (int)mem.hw_read(LY) << "\n";
    //std::cout << (int)currentLineDots << " " << (int)mem.hw_read(LY) << " " << ppu_state << '\n';
    mem.hw_write(STAT, (u8)((mem.hw_read(STAT) & (u8)0b11111100) | (u8)ppu_state));
    //assert(finishedLineDots == currentLineDots);
}

std::array<u8, 23040>& PPU::getBuffer() {
    return frameBuffer;
}

u8 PPU::pixelPicker() {
    if (objQueue.empty() || (objQueue.front().bgPriority == 1 && bgQueue.front().color != 0) || (mem.read(LCDC) & 0b10) == 0 || objQueue.front().color == 0) {
        if ((mem.read(LCDC) & 0b1) == 0) return 0;
        else return (mem.hw_read(BGP) >> (2 * bgQueue.front().color)) & 0b11;
    } else {
        if (objQueue.front().palette == 0) {
            return (mem.hw_read(OBP0) >> (2 * objQueue.front().color)) & 0b11;
        } else {
            return (mem.hw_read(OBP1) >> (2 * objQueue.front().color)) & 0b11;
        }
    }
}

void PPU::oamScan(u16 address) { // 2 dots
    u8 currentLine = mem.hw_read(LY); // ly register    
    u8 objY_pos = mem.hw_read(address);
    Object obj(objY_pos, mem.hw_read(address + 1), mem.hw_read(address + 2), mem.hw_read(address + 3), address - 0xFE00);
    if ((mem.hw_read(LCDC) & 0b100) > 0) { // 8x16 tiles
        if ((objY_pos - currentLine) > 0 && (objY_pos - currentLine) < 17) {
            if (objFetchIdx < 10) {
                objArr[objFetchIdx] = obj;
                objFetchIdx += 1;
            }
        }
    } else { // 8x8 tiles
        if (((objY_pos - 8) - currentLine) > 0 && ((objY_pos - 8) - currentLine) < 9) {
            if (objFetchIdx < 10) {
                //std::cout << std::hex << (int)address << " " << std::dec << (int)objFetchIdx << " " << (int)objY_pos << " " << (int)currentLine << "\n";
                objArr[objFetchIdx] = obj;
                objFetchIdx += 1;
            }
        }
    }
}

void PPU::setPixel(u8 w, u8 h, u8 pixel) {
    u32* pixelAddress = std::bit_cast<u32*>(surface->pixels);
    const static std::array<u32,4> colors = { 0xFFFFFF, 0xAAAAAA, 0x555555, 0x000000 };
    pixelAddress += surface->w * h + w;

    *pixelAddress = colors[pixel];
}

void PPU::statInterruptHandler() {
    bool prevIRQ = statIRQ;
    if (mem.hw_read(LYC) == 0 && mem.hw_read(LY) == 153 && currentLineDots > 4 && (mem.hw_read(STAT) & 0b1000000) > 0) {
            statIRQ = true;
            mem.hw_write(STAT, (u8)(mem.hw_read(STAT) | 0b100));
    } else if ((mem.hw_read(LY) == mem.hw_read(LYC) && (mem.hw_read(STAT) & 0b1000000) > 0)) {
            statIRQ = true;
            mem.hw_write(STAT, (u8)(mem.hw_read(STAT) | 0b100));
    } else if (ppu_state == mode2 && (mem.hw_read(STAT) & 0b100000) > 0) {
            statIRQ = true;
            mem.hw_write(STAT, (u8)((mem.hw_read(STAT) & 0b11111100) | mode2));
    } else if (ppu_state == mode1 && (mem.hw_read(STAT) & 0b10000) > 0) {
            statIRQ = true;
            mem.hw_write(STAT, (u8)((mem.hw_read(STAT) & 0b11111100) | mode1));
    } else if (ppu_state == mode0 && (mem.hw_read(STAT) & 0b1000) > 0) {
            statIRQ = true;
            mem.hw_write(STAT, (u8)((mem.hw_read(STAT) & 0b11111100) | mode0));
    } else {
        statIRQ = false;
        if (mem.hw_read(LY) != mem.hw_read(LYC)) {
            mem.hw_write(STAT, (u8)(mem.hw_read(STAT) & 0b11111011));
        }
    }
    if (!prevIRQ && statIRQ) {
        mem.hw_write(IF, (u8)(mem.hw_read(IF) | 0b10));
    }
}

void PPU::setSurface(SDL_Surface* new_surface) {
    surface = new_surface;
}
