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
        return 0x90;
    }
    return mem[address];

}
u8 MMU::write(u16 address, u8 word) {
    if (address == 0xFF02) {
        if (word == 0x81) {
           std::cout << std::hex << read(0xFF01);
        }
    }
    if (address == 0xFF04) {
        mem[address] = 0x00;
    }
    mem[address] = word;
    return 0;
}
u8 MMU::write(u16 address, u16 dword) {
    mem[address] = (u8) (dword & 0xFF);
    mem[address + 1] = (u8) (dword >> 8);
    return 0;
}
u8 MMU::div_inc() {
    mem[0xFF04] += 1;
    return 0;
}
u8 MMU::tima_inc() {
    if (mem[0xFF05] == 255) {
        mem[0xFF05] = mem[0xFF06];
        write(0xFF0F, (u8)(read(0xFF0F) | 0b00000100));
    } else mem[0xFF05] += 1;
    return 0;
}
