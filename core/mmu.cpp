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
    if (address == 0xFF44) {
        return 0xff;
    }
    if (address == 0xFF02) {
    }
    return mem[address];

}
u8 MMU::write(u16 address, u8 word) {
    if (address == 0xFF02) {
        if (word == 0x81) {
           std::cout << std::hex << read(0xFF01);
        }
    }
    if (address == 0xFF04) { // div register
        u16 div = mem[0xFF03];
        div = (mem[0xFF04] << 8) + div;
        div += 4;
        write(0xFF03, div);
    }
    if (address == 0xFF05) { // tima register

    }
    if (address == 0xFF07) { // check for extra ticks

    }
    if (address == 0xFF46) {
        
    }
    mem[address] = word;
    return 0;
}
u8 MMU::write(u16 address, u16 dword) {
    mem[address] = (u8) (dword & 0xFF);
    mem[address + 1] = (u8) (dword >> 8);
    return 0;
}

