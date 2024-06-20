#include "../lib/types.h"
#include <array>
#include <iomanip>
#include <iostream>

int main (void) {
    u16 address = 0;
    u8 mem[] = { 0xFF, 0x00, 0x7e, 0xff, 0x85, 0x81, 0x89, 0x83, 0x93, 0x85, 0xa5, 0x8b, 0xc9, 0x97, 0x7e, 0xff };
    
    std::array<u16, 8> tileArray;
    u8 lowByte;
    u8 highByte;
    u16 line;
    u8 mask;
    for (int i = 0; i < 8; i += 1) {
        line = 0;
        lowByte = mem[address];
        highByte = mem[address + 1];
        for (int j = 0; j < 8; ++j) {
            mask = 0b1 << j;
            line += (((u16)highByte & mask) << (j + 1)) + (((u16)lowByte & mask) << j);
        }
        tileArray[i] = line;
        address += 2;
    }
    for (int i = 0; i < 8; ++i) {
        std::cout << std::setfill('0') << std::hex << std::setw(4) << (int)tileArray[i] << "\n";
    }

}
