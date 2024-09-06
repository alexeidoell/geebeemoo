#include "../lib/types.h"
#include "mmu.h"
#include <array>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <iterator>

u8 MMU::load_cart(char* filename) {
    FILE* f;
    u32 read_chars;
    f = fopen(filename, "rb"); // maybe i should replace this with more c++ type file handling
    if (f) {
        read_chars = fread(mem.data(), sizeof(u8), 0x8000, f);
        fclose(f);
        return read_chars;
    } else {
        std::cout << "opening cartridge failed\n"; 
        return 0;
    }
}
u8 MMU::read(u16 address) {
    if (address < 0xFF80 && oam_state) {
        return 0xFF;
    }
    if (address >= 0x8000 && address < 0xA000 && ppuState == mode3) {
        return 0xFF;
    }
    if (address >= 0xFE00 && address < 0xFEA0 && ppuState == mode2) {
        return 0xFF;
    }
    if (address == 0xFF00) {
        u8 inputReg = mem[address];
        if ((inputReg & 0x30) == 0x10) { // buttons
            inputReg &= 0xF0;
            inputReg += joypad->getButton();
        } else { // dpad
            inputReg &= 0xF0;
            inputReg += joypad->getDpad();
        }
        return inputReg;
    }
    if (address == 0xFF01) {
        return 0xFF;
    }
    if (address == 0xFF4D) {
        return 0xFF;
    }
    if (address == 0xFF02) {
    }
    return mem[address];

}
u8 MMU::write(u16 address, u8 word) {
    if (address < 0xFF80 && oam_state) {
        return -1;
    }
    if (address >= 0x8000 && address < 0xA000 && ppuState == mode3) {
        return 0;
    } else if (address >= 0xFE00 && address < 0xFEA0 && (ppuState == mode2 || ppuState == mode3)) { 
        return 0;
    }
    if (address == 0xFF02) {
        if (word == 0x81) {
           std::cout << (char) mem[0xFF01];
        }
    }
    else if (address == 0xFF04) { // div register
                                  // this prevents normal writes
                                  // but the timer uses double writes 
                                  // so still has permission
        write(0xFF03, (u16)0x00);
    }
    else if (address == 0xFF46) {
        if (!oam_state) {
            oam_state = true;
            oam_address = word << 8;
        }

    }
    else if (address == 0x2000) { // mbc 1 register

    }
    else mem[address] = word;
    return 0;
}

u8 MMU::write(u16 address, u16 dword) {
    if (address < 0xFF80 && oam_state) {
        return -1;
    }
    if (address >= 0x8000 && address < 0xA000 && ppuState == mode3) {
        return 0;
    }
    if (address >= 0xFE00 && address < 0xFEA0 && ppuState == mode2) {
        return 0;
    }
    if (address == 0x2000) { 
        return 0;
    }
    if (address == 0xFF46) {
        // start oam transfer process

    } else {
        mem[address] = (u8) (dword & 0xFF);
        mem[address + 1] = (u8) (dword >> 8);
    }
    return 0;
}

u8 MMU::ppu_read(u16 address) {
    return mem[address];
}
u8 MMU::ppu_write(u16 address, u8 word) {
    mem[address] = word;
    return 0;
}

u8 MMU::ppu_write(u16 address, u16 dword) {
    mem[address] = (u8) (dword & 0xFF);
    mem[address + 1] = (u8) (dword >> 8);
    return 0;
}

bool MMU::get_oam() {
    return oam_state;
}

u8 MMU::oam_transfer(u8 ticks) {
    for (auto i = 0; i < ticks; i += 4) {
        if (oam_offset == 160){
            oam_state = false;
            oam_offset = 0;
            break;
        }
        mem[0xFE00 + oam_offset] = mem[oam_address + oam_offset];
        oam_offset += 1;
    }
    return 0;
}
