#pragma once
#include <array>
#include <string>
#include "../lib/types.h"
#include "joypad.h"

class MMU {
private:
    std::array<u8, 65536> mem{{0}}; // placeholder length for testing purposes
    Joypad joypad;
public:
    u8 load_cart(std::string filename);
    u8 read(u16 address);
    u8 write(u16 address, u8 word);
    u8 write(u16 address, u16 dword);
};

