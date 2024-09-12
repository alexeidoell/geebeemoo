#include "../lib/types.h"
#include "mbc.h"

u8 MBC1::mbc_write(u16 address, u8 word) {
    if (address < 0x2000) {
        if (word == 0xA) ram_enable = true;
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

u16 MBC1::mapper(u16 base_address) {

    return 0;
}
