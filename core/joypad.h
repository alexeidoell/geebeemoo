#pragma once
#include <lib/types.h>
#include <SDL2/SDL.h>

class Joypad {
private:
    u8 buttonState = 0xF;
    u8 dpadState = 0xF;
public:
    u8 getButton();
    u8 getDpad();
    u8 pollPresses(SDL_Event& event);
};
