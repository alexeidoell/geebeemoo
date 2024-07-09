#include "ppu.h"
#include "mmu.h"
#include <cassert>
#include <exception>
#include <iomanip>
#include <iostream>

u16 PPU::combineTile(u8 tileHigh, u8 tileLow, tileType tiletype) { // 0 dots
    u16 line;
    u8 mask;
    line = 0;
    for (auto j = 0; j < 8; ++j) {
        mask = 0b1 << j;
        line += (((u16)tileHigh & mask) << (j + 1)) + (((u16)tileLow & mask) << j);
    }

    //std::cout << std::setw(4) << std::setfill('0') << std::hex << (int)line << '\n';

    for (auto i = 0; i < 8; ++i) {
        u16 mask = 0b11 << 14;
        mask >>= (i * 2);
        u16 color = line & mask;
        color >>= (14 - (i * 2));
        Pixel& pixel = *new Pixel((u8)color, 0, 0);
        if (bgQueue.size() < 8) bgQueue.push(pixel);
        assert(bgQueue.size() <= 8);
    }
    return 0;
}

u8 PPU::getTileByte(u16 index) { // 2 dots
    return mem->ppu_read(index); 
}

u16 PPU::pixelFetcher() { //  2 dots
    u16 tileMap = 0x9800; 
    if ((mem->ppu_read(0xFF40) & 0b1000) > 0) { // assumes background not window
        tileMap = 0x9C00;
    }
    u16 offset = 0xFF & ((mem->ppu_read(0xFF44) + mem->ppu_read(0xFF42)) / 8);
    offset <<= 5;
    offset += 0x1F & ((xCoord + mem->ppu_read(0xFF43)) / 8);
    if (!firstTile) {
        offset += 1;
    }
    u8 tileID = mem->ppu_read(tileMap + offset);
    u16 tileAddress = ((u16)tileID) << 4;
    tileAddress += ((mem->ppu_read(0xFF44) + mem->read(0xFF42)) % 8) << 1;
    tileAddress += 0b1 << 15;
    u8 addressing_method = mem->ppu_read(0xFF40) & 0b10000;
    if (!addressing_method && ((tileID & 0x80) > 0)) {
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
        if (currentLine == mem->ppu_read(0xFF45)) { // ly = lyc
            mem->ppu_write(0xFF41, (u8)(mem->ppu_read(0xFF41) | 0b100));
            if ((mem->ppu_read(0xFF41) & 0b1000000) > 0) {
                mem->ppu_write(0xFFFF, (u8)(mem->ppu_read(0xFFFF) | 0b10));
            }
            
        } else mem->ppu_write(0xFF41, (u8)(mem->ppu_read(0xFF41) & 0b11111011));
        if (ppuState == mode1) {
            return 0;
        }
        if (finishedLineDots < 80 && finishedLineDots < currentLineDots) {
            while (finishedLineDots < 80 && finishedLineDots < currentLineDots) {
                u16 address = 0xFEA0 - 2 * (80 - finishedLineDots);
                oamScan(address);
                finishedLineDots += 2;
            }
        } 
        if (finishedLineDots >= 80 && finishedLineDots < 172 + 80 && finishedLineDots < currentLineDots) {
            ppuState = mode3;
            if (finishedLineDots == 80) { // setting up mode3
                while(!bgQueue.empty()) bgQueue.pop();
                while(!objQueue.empty()) objQueue.pop();
            }
            while (finishedLineDots < 86 && finishedLineDots < currentLineDots) { // do nothing to simulate discarded tile
                finishedLineDots += 2;
            }
            while (finishedLineDots >= 86 && finishedLineDots < 92 && finishedLineDots < currentLineDots) { // initial fetches
                if (!fifoFlags.fetchTileID) {
                    fifoFlags.tileAddress = pixelFetcher();
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
                    combineTile(fifoFlags.highByte, fifoFlags.lowByte, background);
                    fifoFlags.fetchLowByte = false;
                    fifoFlags.fetchHighByte = false;
                    fifoFlags.fetchTileID = false;
                    while (!bgQueue.empty()) {
                        bgQueue.pop();
                    }
                }
            }
            while (finishedLineDots >= 92 && finishedLineDots < 172 + 80 && finishedLineDots < currentLineDots) { // normal mode3 cycle
                if (!fifoFlags.awaitingPush) { // get next push ppu_ready
                    fifoFlags.tileAddress = pixelFetcher(); // can i be this lazy ???
                                                            // PROBABLY NOT !!!!!
                    fifoFlags.highByte = getTileByte(fifoFlags.tileAddress);
                    fifoFlags.lowByte = getTileByte(fifoFlags.tileAddress + 1);
                    fifoFlags.awaitingPush = true;
                }
                if (bgQueue.empty() && fifoFlags.awaitingPush) {
                    // push new tile row
                    combineTile(fifoFlags.highByte, fifoFlags.lowByte, background);
                    fifoFlags.awaitingPush = false;
                    fifoFlags.fetchLowByte = false;
                    fifoFlags.fetchHighByte = false;
                    fifoFlags.fetchTileID = false;
                }
                assert((u8)bgQueue.front().color <= 3 && "pixel color is greater than 3 in dgb mode");
                if (firstTile) {
                    for (auto i = 0; i < (mem->ppu_read(0xff43) % 8); ++i) {
                        //bgQueue.pop();
                    }
                }
                for (auto i = 0; i < objArr.size(); ++i) {
                    if (xCoord == objArr[i].xPos) {
                        fifoFlags.tileAddress = 0x8000;                       
                        fifoFlags.tileAddress += ((u16)objArr[i].tileIndex) << 4;
                        fifoFlags.tileAddress += (currentLine - objArr[i].yPos % 8) << 1;
                        std::cout << std::hex << (int)fifoFlags.tileAddress << " " << (int) objArr[i].tileIndex << " " << (int)(currentLine - objArr[i].yPos % 8) << '\n';

                        
                    }
                }
                if (xCoord < 160) frameBuffer[xCoord++ + currentLine * 160] = (u8)bgQueue.front().color; // placeholder
                if (firstTile) firstTile = false;
                bgQueue.pop();
                finishedLineDots += 1;
            }
        }
        if (finishedLineDots >= 172 + 80 && finishedLineDots < 456 && finishedLineDots < currentLineDots) { // hblank
            ppuState = mode0;
            firstTile = true;
            xCoord = 0;
            while (finishedLineDots < currentLineDots) {
                finishedLineDots += 2;
            }
        }
    }
        if (currentLineDots >= 456) {
            // implement moving down to next scan line
            mem->ppu_write(0xFF44, (u8)(currentLine + 1));
            currentLine = mem->ppu_read(0xFF44);
            currentLineDots -= 456;
            finishedLineDots -= 456; // idk tbh?
            if (ppuState != mode1) ppuState = mode2;
            if (currentLine >= 154) {
                // either need it to chill out until the last frame is done rendering
                // or somehow start the next frame early
                // former option is probably way better
                return 0; // ??
            } else if (currentLine == 144) { // vblank
                //std::cout << "vblank\n";
                ppuState = mode1;
                mem->ppu_write(0xFF0F, (u8)(mem->ppu_read(0xFF0F) | 0b1));
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

u8 PPU::modeSwitch() {

    return 0;
}

u8 PPU::oamScan(u16 address) { // 2 dots
    u8 currentLine = mem->ppu_read(0xFF44); // ly register    
    u8 objY_pos = mem->ppu_read(address);
    Object obj = Object(objY_pos, mem->ppu_read(address + 1), mem->ppu_read(address + 2), mem->ppu_read(address + 3));
    if ((mem->ppu_read(0xFF40) & 0b100) > 0) { // 8x16 tiles
        if (((objY_pos) - currentLine) > 0) {
            if (objFetchIdx < 10) {
                objArr[objFetchIdx] = obj;
                objFetchIdx += 1;
            }
        }
    } else { // 8x8 tiles
        if (((objY_pos) - currentLine) > 0) {
            if (objFetchIdx < 10) {
                objArr[objFetchIdx] = obj;
                objFetchIdx += 1;
            }
        }
    }
    return 0;
}
