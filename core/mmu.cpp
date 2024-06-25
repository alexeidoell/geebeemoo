#include "../lib/types.h"
#include "mmu.h"
#include <array>
#include <cstdio>
#include <iostream>

u8 MMU::load_cart(char* filename) {
    FILE* f;
    u32 read_chars;
    f = fopen(filename, "rb");
    if (f) {
        read_chars = fread(mem.data(), sizeof(u8), mem.size(), f);
        return read_chars;
    } else {
        std::cout << "opening cartridge failed\n";
        return -1;
    }
}
u8 MMU::read(u16 address) {
    if (address == 0xFF4D) {
        return 0xFF;
    }
    if (address == 0xFF02) {
    }
    if (address == 0xFF00) {
        return 0x0F;
    }
    return mem[address];

}
u8 MMU::write(u16 address, u8 word) {
    if (address == 0xFF02) {
        if (word == 0x81) {
           std::cout << std::hex << read(0xFF01);
        }
    }
    else if (address == 0xFF04) { // div register
                                  // this prevents normal writes
                                  // but the timer uses double writes 
                                  // so still has permission
        write(0xFF03, (u16)0x00);
    }
    else mem[address] = word;
    return 0;
}

u8 MMU::write(u16 address, u16 dword) {
    if (address == 0xFF46) {
        // start oam transfer process

    } else {
        mem[address] = (u8) (dword & 0xFF);
        mem[address + 1] = (u8) (dword >> 8);
    }
    return 0;
}

