#include "ppu.h"
#include "mmu.h"
#include <cassert>
#include <exception>
#include <iostream>

u16 PPU::combineTile(u8 tileHigh, u8 tileLow) { // 0 dots
    u16 line;
    u8 mask;
    line = 0;
    for (auto j = 0; j < 8; ++j) {
        mask = 0b1 << j;
        line += (((u16)tileHigh & mask) << (j + 1)) + (((u16)tileLow & mask) << j);
    }
    for (auto i = 0; i < 8; ++i) {
        u16 mask = 0b11 << 14;
        mask >>= (i * 2);
        u8 color = line & mask;
        color >>= (14 - (i * 2));
        Pixel& pixel = *new Pixel(color, 0, 0);
        if (bgQueue.size() < 8) bgQueue.push(pixel);
        assert(bgQueue.size() <= 8);
    }
    return 0;
}

u8 PPU::getTileByte(u16 index) { // 2 dots
   return mem->read(index); 
}

u16 PPU::pixelFetcher() { //  2 dots
    u16 tileMap = 0x9800; // need to also implement the 9c00 map
    if ((mem->read(0xFF40) & 0b1000) > 0) { // assumes background not window
        tileMap = 0x9C00;
    }
    u16 offset = (0x1F & (mem->read(0xFF44) + mem->read(0xFF42))) / 8;
    offset <<= 5;
    offset += (0x1F & (xCoord + mem->read(0xFF43))) / 8;
    u8 tileID = mem->read(tileMap + offset);
    u16 tileAddress = ((u16)tileID) << 4;
    tileAddress += ((mem->read(0xFF44) + mem->read(0xFF42)) % 8) << 1;
    tileAddress += 0b1 << 15;
    u8 addressing_method = mem->read(0xFF40) & 0b10000;
    if (addressing_method || ((tileID & 0x80) > 0)) {
        tileAddress += (0b1 << 12);
    }

    return tileAddress;
}

u8 PPU::ppuLoop(u8 ticks) {
    u8 currentLine = mem->read(0xFF44); // ly register    
    u16 finishedLineDots = currentLineDots;
    currentLineDots += ticks;
    if (ppuState == mode2 && currentLineDots >= 80) {
        ppuState = mode3;
    } else if (ppuState == mode3 && currentLineDots >= 172 + mode3_delay) {
        // not yet implemented
        // ppuState = mode0;
    } else if (ppuState == mode0 && currentLineDots == 456) {
        // not yet implemented
        // ppuState = mode2;
    } 
    currentLine = mem->read(0xFF44);
    if (currentLine >= 143 && currentLineDots >= 456) {
        // not yet implemented
    }
    if (finishedLineDots < 80 && finishedLineDots < currentLineDots) {
        while (finishedLineDots < 80 && finishedLineDots < currentLineDots) {
            u16 address = 0xFEA0 - 2 * (80 - finishedLineDots);
            oamScan(address);
            finishedLineDots += 2;
        }
    } 
    if (finishedLineDots >= 80 && finishedLineDots < 172 + mode3_delay && finishedLineDots < currentLineDots) {
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
                combineTile(fifoFlags.highByte, fifoFlags.lowByte);
                fifoFlags.awaitingPush = false;
                fifoFlags.fetchLowByte = false;
                fifoFlags.fetchHighByte = false;
                fifoFlags.fetchTileID = false;
            }
            for (auto i = 0; i < (mem->read(0xFF43) & 0b111); ++i) {
                bgQueue.pop();
            }
        }
        while (finishedLineDots >= 92 && finishedLineDots < 172 + mode3_delay && finishedLineDots < currentLineDots) { // normal mode3 cycle
            if (bgQueue.empty() && fifoFlags.awaitingPush) {
                // push new tile row
                combineTile(fifoFlags.highByte, fifoFlags.lowByte);
                fifoFlags.awaitingPush = false;
                fifoFlags.fetchLowByte = false;
                fifoFlags.fetchHighByte = false;
                fifoFlags.fetchTileID = false;
            } else if (!fifoFlags.awaitingPush) { // get next push ready
                fifoFlags.tileAddress = pixelFetcher(); // can i be this lazy ???
                                                        // PROBABLY NOT !!!!!
                fifoFlags.highByte = getTileByte(fifoFlags.tileAddress);
                fifoFlags.lowByte = getTileByte(fifoFlags.tileAddress + 1);
                fifoFlags.awaitingPush = true;
            }
            assert((u8)bgQueue.front().color <= 3);
            if (xCoord < 160) frameBuffer[xCoord++ + currentLine * 160] = (u8)bgQueue.front().color; // placeholder
            bgQueue.pop();
            finishedLineDots += 1;
        }
    }
    if (finishedLineDots > 172 + mode3_delay && finishedLineDots < currentLineDots) { // hblank
        while (finishedLineDots < currentLineDots) {
            finishedLineDots += 2;
        }
    }
    if (currentLineDots >= 456) {
        // implement moving down to next scan line
    }
    mem->write(0xFF41, (u8)(mem->read(0xFF41) | (u8)ppuState)); // set ppu mode bits
    /*if (finishedLineDots != currentLineDots) {
        std::cout << "finishedLineDots != currentLineDots at end of ppuLoop call\n";
        throw new std::exception();
    }*/
    return 0;
}

std::array<u8, 23040>& PPU::getBuffer() {
    return frameBuffer;
}

u8 PPU::oamScan(u16 address) { // 2 dots
    u8 currentLine = mem->read(0xFF44); // ly register    
    u8 objY_pos = mem->read(address);
    Object obj = Object(objY_pos, mem->read(address + 1), mem->read(address + 2), mem->read(address + 3));
    if ((mem->read(0xFF40) & 0b100) > 0) { // 8x16 tiles
        if (((objY_pos + 16) - currentLine) > 16) {
            if (objFetchIdx < 10) {
                objArr[objFetchIdx] = obj;
                objFetchIdx += 1;
            }
        }
    } else { // 8x8 tiles
        if (((objY_pos + 8) - currentLine) > 16) {
            if (objFetchIdx < 10) {
                objArr[objFetchIdx] = obj;
                objFetchIdx += 1;
            }
        }
    }
    return 0;
}
