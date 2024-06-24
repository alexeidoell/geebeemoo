#include "ppu.h"
#include "mmu.h"
#include <iostream>

u16 PPU::getTile(u16 index) {
    u8 addressing_method = mem->read(0xFF40) & 0b10000;
    u16 address;
    if (addressing_method == 0) {
        address = 0x8800 + ((s8)index * 16);
    } else address = 0x8000 + (index * 16);
    
    u8 lowByte;
    u8 highByte;
    u16 line;
    u8 mask;
    line = 0;
    lowByte = mem->read(address);
    highByte = mem->read(address + 1);
    for (auto j = 0; j < 8; ++j) {
        mask = 0b1 << j;
        line += (((u16)highByte & mask) << (j + 1)) + (((u16)lowByte & mask) << j);
    }
    return line;
}

void PPU::pixelFetcher() {
    u16 tileMap = 0x9800; // need to also implement the 9c00 map
    u8 offset = ((u8)(mem->read(0xFF44) + mem->read(0xFF42))) / 8;
    offset <<= 5;
    offset += ((u8)(xCoord + mem->read(0xFF43))) / 8;
    u16 tileRow = getTile(mem->read(tileMap + offset));
    for (auto i = 0; i < 8; ++i) {
        u16 mask = 0b11 << 14;
        mask >>= (i * 2);
        u8 color = tileRow & mask;
        color >>= (14 - (i * 2));
        Pixel& pixel = *new Pixel(color, 0, 0);
        bgQueue.push(pixel);
    }
}

u8 PPU::ppuLoop(u8 ticks) {
    u8 currentLine = mem->read(0xFF44); // ly register    
    u16 finishedLineDots = currentLineDots;
    currentLineDots += ticks;
    if (finishedLineDots < 80 && finishedLineDots < currentLineDots) {
        while (finishedLineDots < 80 && finishedLineDots < currentLineDots) {
            u16 address = 0xFEA0 - 2 * (80 - finishedLineDots);
            oamScan(address);
            finishedLineDots += 2;
        }
    }
    if (ppuState == mode2 && currentLineDots > 80) {
        ppuState = mode3;
    }
    mem->write(0xFF41, (u8)(mem->read(0xFF41) | (u8)ppuState)); // set ppu mode bits
    return 0;
}

std::array<u8, 23040>& PPU::getBuffer() {
    return frameBuffer;
}

u8 PPU::oamScan(u8 address) {
    u8 currentLine = mem->read(0xFF44); // ly register    
    u8 objY_pos = mem->read(address);
    Object obj = *new Object(objY_pos, mem->read(address + 1), mem->read(address + 2), mem->read(address + 3));
    if ((mem->read(0xFF40) & 0b100) > 0) { // 8x16 tiles
        if (((objY_pos + 16) - currentLine) > 16) {
            if (objList.size() < 10) objList.insert(objList.end(), obj);
        }
    } else { // 8x8 tiles
        if (((objY_pos + 8) - currentLine) > 16) {
            if (objList.size() < 10) objList.insert(objList.end(), obj);
        }
    }
    return 0;
}
