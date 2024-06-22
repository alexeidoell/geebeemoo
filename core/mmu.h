#pragma once
#include <array>
#include <string>
#include "../lib/types.h"
#include "joypad.h"

enum PPUState { mode2, mode3, mode0, mode1 };

class MMU {
private:
    std::array<u8, 0xFFFF> mem{{0}};
    Joypad joypad;
public:
    PPUState ppuState = mode2;
    u8 load_cart(char* filename);
    u8 read(u16 address);
    u8 write(u16 address, u8 word);
    u8 write(u16 address, u16 dword);
    u8 div_inc();
    u8 tima_inc();
    bool tima_tick = false;
};

