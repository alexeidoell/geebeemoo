#include "ppu.h"
#include "mmu.h"

u16 PPU::getTile(u8 index) {
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
        for (int j = 0; j < 8; ++j) {
            mask = 0b1 << j;
            line += (((u16)highByte & mask) << (j + 1)) + (((u16)lowByte & mask) << j);
        }
    return line;
}

void PPU::pixelFetcher() {


}
