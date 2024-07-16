#pragma once
#include <array>
#include <string>
#include "../lib/types.h"

enum PPUState { mode0, mode1, mode2, mode3 };

class MMU {
private:
    std::array<u8, 0x10000> mem = {0};
    bool oam_state = false;
    u8 oam_offset = 0;
    u16 oam_address;
public:
    PPUState ppuState = mode2;
    u8 load_cart(char* filename);
    u8 read(u16 address);
    u8 write(u16 address, u8 word);
    u8 write(u16 address, u16 dword);

    u8 ppu_read(u16 address);
    u8 ppu_write(u16 address, u8 word);
    u8 ppu_write(u16 address, u16 dword);


    u8 div_inc();
    u8 tima_inc();
    bool tima_tick = false;

    bool get_oam();
    u8 oam_transfer(u8 ticks);
};

