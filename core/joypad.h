#pragma once
#include "../lib/types.h"

class Joypad {
public:
    u8 joypadFlags;
    u8 pollPresses();
};
