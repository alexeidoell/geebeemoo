#include "ppu.h"
#include "mmu.h"
#include <bitset>
#include <cassert>
#include <cstring>
#include <exception>
#include <iomanip>
#include <ios>
#include <iostream>

u16 PPU::combineTile(u8 tileHigh, u8 tileLow, tileType tiletype, const Object * object) {
    u16 line;
    u16 mask;
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
        u8 palette;
        u8 bgPriority;
        if (tiletype == obj)  {
            palette = (object->flags & 0b10000) >> 4;
            bgPriority = (object->flags & 0b10000000) >> 7;
        }
        else {
            palette = 0;
            bgPriority = 0;
        }
        Pixel& pixel = *new Pixel(color, palette, 0, bgPriority);
        if (tiletype == bg) {
            if (bgQueue.size() < 8) bgQueue.push(pixel);
            assert(bgQueue.size() <= 8);
        } else if (tiletype == obj) {
            if (i < objQueueSize) {
                Pixel oldPixel = objQueue.front();
                objQueue.pop();
                if (oldPixel.color == 0) objQueue.push(pixel);
                else objQueue.push(oldPixel);
            } else objQueue.push(pixel);

        }
    }
    return 0;
}

u8 PPU::getTileByte(u16 index) { // 2 dots
    return mem->ppu_read(index); 
}

u16 PPU::bgPixelFetcher() { //  2 dots
    u16 tileMap = 0x9800; 
    if ((mem->ppu_read(0xFF40) & 0b1000) > 0) { 
        tileMap = 0x9C00;
    }
    u16 offset = 0xFF & ((mem->ppu_read(0xFF44) + mem->ppu_read(0xFF42)) / 8);
    offset <<= 5;
    offset += 0x1F & ((xCoord + mem->ppu_read(0xFF43)) / 8);
    u8 tileID = mem->ppu_read(tileMap + offset);
    u16 tileAddress = ((u16)tileID) << 4;
    tileAddress += ((mem->ppu_read(0xFF44) + mem->read(0xFF42)) % 8) << 1;
    tileAddress += 0b1 << 15;
    bool addressing_method = (mem->ppu_read(0xFF40) & 0b10000) == 0b10000;
    if (!addressing_method && (tileID & 0x80) == 0) {
        tileAddress += (0b1 << 12);
    }
    return tileAddress;
}

u16 PPU::winPixelFetcher() { 
    u16 tileMap = 0x9800; 
    if ((mem->ppu_read(0xFF40) & 0b1000000) > 0) { 
        tileMap = 0x9C00;
    }
    u16 offset = 0xFF & (window.yCoord / 8);
    offset <<= 5;
    offset += 0x1F & (window.xCoord / 8);
    u8 tileID = mem->ppu_read(tileMap + offset);
    u16 tileAddress = ((u16)tileID) << 4;
    tileAddress += (window.yCoord % 8) << 1;
    tileAddress += 0b1 << 15;
    bool addressing_method = (mem->ppu_read(0xFF40) & 0b10000) == 0b10000;
    if (!addressing_method && (tileID & 0x80) == 0) {
        tileAddress += (0b1 << 12);
    }
    return tileAddress;
}

u8 PPU::ppuLoop(u8 ticks) {
    if (mem->ppu_read(0xFF44) == 154) return 0;
    s16 finishedLineDots = (s16)currentLineDots;
    currentLineDots += ticks;
    if (mem->ppu_read(0xFF44) >= 144) {
        finishedLineDots = currentLineDots;
    }
    u8 currentLine = mem->ppu_read(0xFF44); // ly register    
    while (mem->ppu_read(0xFF44) < 144 && finishedLineDots < currentLineDots) {
        if (ppuState == mode1) {
            return 0;
        }
        if (finishedLineDots < 80 && finishedLineDots < currentLineDots) {
            if (mem->read(0xFF4A) <= currentLine) {
                window.WY_cond = true;
            }
            while (finishedLineDots < 80 && finishedLineDots < currentLineDots) {
                u16 address = 0xFEA0 - 2 * (80 - finishedLineDots);
                oamScan(address);
                finishedLineDots += 2;
            }
        } 
        if (finishedLineDots >= 80 && finishedLineDots < 172 + 80 && finishedLineDots < currentLineDots) {
            ppuState = mode3;
            mem->ppu_write(0xFF41, (u8)(mem->ppu_read(0xFF41) | ppuState));
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
                    fifoFlags.highByte = getTileByte(fifoFlags.tileAddress);
                    fifoFlags.fetchHighByte = true;
                    finishedLineDots += 2;
                } else if (!fifoFlags.fetchLowByte) {
                    fifoFlags.lowByte = getTileByte(fifoFlags.tileAddress + 1);
                    fifoFlags.fetchLowByte = true;
                    fifoFlags.awaitingPush = true;
                    finishedLineDots += 2;
                }
            }
            if (finishedLineDots == 92) { // first pixel push
                if (fifoFlags.awaitingPush) {
                    combineTile(fifoFlags.highByte, fifoFlags.lowByte, bg, nullptr);
                    fifoFlags.fetchLowByte = false;
                    fifoFlags.fetchHighByte = false;
                    fifoFlags.fetchTileID = false;
                }
            }
            while (finishedLineDots >= 92 && finishedLineDots < 172 + 80 && finishedLineDots < currentLineDots) { // normal mode3 cycle
                if (!fifoFlags.awaitingPush) { // get next push ppu_ready
                    fifoFlags.tileAddress = bgPixelFetcher();
                    fifoFlags.highByte = getTileByte(fifoFlags.tileAddress + 1);
                    fifoFlags.lowByte = getTileByte(fifoFlags.tileAddress);
                    fifoFlags.awaitingPush = true;
                }
                if (!window.WX_cond && bgQueue.empty() && fifoFlags.awaitingPush) {
                    // push new tile row
                    combineTile(fifoFlags.highByte, fifoFlags.lowByte, bg, nullptr);
                    fifoFlags.awaitingPush = false;
                    fifoFlags.fetchLowByte = false;
                    fifoFlags.fetchHighByte = false;
                    fifoFlags.fetchTileID = false;
                }
                if (firstTile) {
                    for (auto i = 0; i < (mem->ppu_read(0xff43) % 8); ++i) {
                        bgQueue.pop();
                        xCoord += 1;
                    }
                }
                for (auto i = 0; i < objFetchIdx; ++i) {
                    if (xCoord == objArr[i].xPos) {
                        fifoFlags.objTileAddress = 0x8000;                       
                        fifoFlags.objTileAddress += ((u16)objArr[i].tileIndex) << 4; // assumes tile is not flipped
                        bool flipCond = (objArr[i].flags & 0b1000000) > 0;
                        if (flipCond) {
                            fifoFlags.objTileAddress += (~(((currentLine - (objArr[i].yPos - 16)) % 8) << 1)) & 0b1110;
                        } else fifoFlags.objTileAddress += ((currentLine - (objArr[i].yPos - 16)) % 8) << 1;
                        if ((mem->ppu_read(0xFF40) & 0b100) > 0) { // 8x16 tiles
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
                if (xCoord < 168 && ((mem->read(0xFF40) & 0b100000) > 0) && window.WY_cond && (xCoord - 1 == mem->read(0xFF4B) || window.WX_cond)) { // window time
                    if (bgQueue.empty() || window.WX_cond == false) {
                        while (!bgQueue.empty()) bgQueue.pop();
                        fifoFlags.tileAddress = winPixelFetcher();
                        window.xCoord += 8;
                        fifoFlags.highByte = getTileByte(fifoFlags.tileAddress + 1);
                        fifoFlags.lowByte = getTileByte(fifoFlags.tileAddress);
                        combineTile(fifoFlags.highByte, fifoFlags.lowByte, bg, nullptr);
                        fifoFlags.awaitingPush = true;
                    }
                    window.WX_cond = true;
                }
                if (xCoord > 7 && xCoord < 168) {
                    frameBuffer[(xCoord - 8) + currentLine * 160] = pixelPicker();
                    finishedLineDots += 1;
                }
                if (xCoord < 168) xCoord += 1;
                if (firstTile) firstTile = false;
                if (!objQueue.empty()) objQueue.pop();
                bgQueue.pop();
            }
        }
        if (finishedLineDots >= 172 + 80 && finishedLineDots < 456 && finishedLineDots < currentLineDots) { // hblank
            ppuState = mode0;
            if ((mem->ppu_read(0xFF41) & 0b001000) > 0) {
                mem->ppu_write(0xFF0F, (u8)(mem->ppu_read(0xFF0F) | 0b10));
            }
            mem->ppu_write(0xFF41, (u8)(mem->ppu_read(0xFF41) | ppuState));
            firstTile = true;
            xCoord = 0;
            while (finishedLineDots < currentLineDots) {
                finishedLineDots += 2;
            }
        }
    }
        if (currentLineDots >= 456) {
            // implement moving down to next scan line
            window.WY_cond = false;
            if (window.WX_cond) {
                window.yCoord += 1;
            }
            window.WX_cond = false;
            currentLine += 1;
            mem->ppu_write(0xFF44, (u8)(currentLine));
            if (currentLine == mem->ppu_read(0xFF45)) { // ly = lyc
                mem->ppu_write(0xFF41, (u8)(mem->ppu_read(0xFF41) | 0b100));
                if ((mem->ppu_read(0xFF41) & 0b1000000) > 0) {
                    mem->ppu_write(0xFF0F, (u8)(mem->ppu_read(0xFF0F) | 0b10));
                }

            } else mem->ppu_write(0xFF41, (u8)(mem->ppu_read(0xFF41) & 0b11111011));
            window.xCoord = 0;
            memset(objArr.data(), 0, objArr.size());
            objFetchIdx = 0;
            currentLineDots -= 456;
            finishedLineDots -= 456; // idk tbh?
            if (ppuState != mode1) { 
                ppuState = mode2;
                if ((mem->ppu_read(0xFF41) & 0b100000) > 0) {
                    mem->ppu_write(0xFF0F, (u8)(mem->ppu_read(0xFF0F) | 0b10));
                }
                mem->ppu_write(0xFF41, (u8)(mem->ppu_read(0xFF41) | ppuState));
            }
            if (currentLine >= 154) {
                // either need it to chill out until the last frame is done rendering
                // or somehow start the next frame early
                // former option is probably way better
                return 0; // ??
            } else if (currentLine == 144) { // vblank
                //std::cout << "vblank\n";
                ppuState = mode1;
                window.yCoord = 0;
                mem->ppu_write(0xFF41, (u8)(mem->ppu_read(0xFF41) | ppuState));
                mem->ppu_write(0xFF0F, (u8)(mem->ppu_read(0xFF0F) | 0b1));
            } else if (currentLine > 144) {
                mem->ppu_write(0xFF0F, (u8)(mem->ppu_read(0xFF0F) | 0b1));
                if ((mem->ppu_read(0xFF41) & 0b010000) > 0) {
                    mem->ppu_write(0xFF0F, (u8)(mem->ppu_read(0xFF0F) | 0b10));
                }
            }
        }
    //std::cout << (int)finishedLineDots << " " << (int)currentLineDots << " " << (int)ticks << " " << (int)mem->ppu_read(0xFF44) << "\n";
    //std::cout << (int)currentLineDots << " " << (int)mem->ppu_read(0xFF44) << " " << ppuState << '\n';
        mem->ppu_write(0xFF41, (u8)(mem->ppu_read(0xFF41) | (u8)ppuState));
        assert(finishedLineDots == currentLineDots);
        return 0;
}

std::array<u8, 23040>& PPU::getBuffer() {
    return frameBuffer;
}

u8 PPU::pixelPicker() {
    if ((objQueue.front().bgPriority == 1 && bgQueue.front().color != 0) || (mem->read(0xFF40) & 0b10) == 0 || objQueue.empty() || objQueue.front().color == 0) {
        if ((mem->read(0xFF40) & 0b1) == 0) return 0;
        else return (mem->ppu_read(0xFF47) >> (2 * bgQueue.front().color)) & 0b11;
    } else {
        if (objQueue.front().palette == 0) {
            return (mem->ppu_read(0xFF48) >> (2 * objQueue.front().color)) & 0b11;
        } else {
            return (mem->ppu_read(0xFF49) >> (2 * objQueue.front().color)) & 0b11;
        }
    }
}

u8 PPU::modeSwitch() {
    return 0;
}

u8 PPU::oamScan(u16 address) { // 2 dots
    u8 currentLine = mem->ppu_read(0xFF44); // ly register    
    u8 objY_pos = mem->ppu_read(address);
    Object obj = Object(objY_pos, mem->ppu_read(address + 1), mem->ppu_read(address + 2), mem->ppu_read(address + 3));
    if ((mem->ppu_read(0xFF40) & 0b100) > 0) { // 8x16 tiles
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
    return 0;
}
