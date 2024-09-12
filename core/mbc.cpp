#include "../lib/types.h"
#include "mbc.h"

u8 MBC1::mbc_write(u16 address, u8 word) {
    if (address < 0x2000) {
        if ((word & 0b1111) == 0xA) ram_enable = true;
        else ram_enable = false;
    } else if (address < 0x4000) {
        rom_bank = word & 0b11111;       
        if (rom_bank == 0x0) rom_bank = 0x1;
    } else if (address < 0x6000) {
        ram_bank = word & 0b11;
    } else if (address < 0x8000) {
        banking_mode = word & 0b1;
    }
    return 0;
}

u32 MBC1::mapper(u16 base_address) {
    u32 mapped_address = 0;
    if (base_address < 0x4000) {
        base_address = base_address & ~(((u32)0b11) << 14); // idk if this should be 13
        if (banking_mode == 1) {
            mapped_address += ((u32)ram_bank << 19);
            mapped_address += base_address;
        }
    } else if (base_address < 0x8000) {
        base_address = base_address & ~(((u32)0b11) << 14); // idk if this should be 13
        if (banking_mode == 1) {
            mapped_address += ((u32)ram_bank << 19);
            mapped_address += base_address;
        }
        mapped_address += ((u32)rom_bank << 14);
    }
    return mapped_address;
}
