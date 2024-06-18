#pragma once
#include <array>
#include <string>
#include "../lib/types.h"
#include "joypad.h"

class MMU {
private:
    std::array<u8, 0xFFFF> mem{{0}};
    Joypad joypad;
public:
    u8 load_cart(char* filename);
    u8 read(u16 address);
    u8 write(u16 address, u8 word);
    u8 write(u16 address, u16 dword);
};

