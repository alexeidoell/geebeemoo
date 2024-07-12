#pragma once
#include "../lib/types.h"
#include "SDL2/SDL.h"
#include "mmu.h"
#include <memory>

enum inputState { buttons, dpad };

class Joypad {
    std::shared_ptr<MMU> mem;
    u8 buttonState : 4;
    u8 dpadState : 4;
public:
    Joypad(std::shared_ptr<MMU> mem) 
    : mem(mem) {
    }
    u8 pollPresses(SDL_Event& event);
};
