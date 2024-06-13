#include "../lib/types.h"
#include "mmu.h"
#include <array>
#include <string>
#include <cstdio>
#include <iostream>

u8 MMU::load_cart(std::string filename) {
    FILE* f;
    u32 read_chars;
    f = fopen(filename.c_str(), "rb");
    if (f) {
        read_chars = fread(mem.data(), sizeof(u8), mem.size(), f);
        return 0;
    } else {
        std::cout << "opening cartridge failed";
        return -1;
    }
}
u8 MMU::read(u16 address) {
    if (address < 0x8000) return mem[address];

}
u8 MMU::write(u16 address, u8 word) {
}
u8 MMU::write(u16 address, u16 dword) {
}

