#include "joypad.h"
#include <SDL2/SDL_events.h>

u8 Joypad::pollPresses(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_DOWN) {
            dpadState &= 0b0111;
        } else if (event.key.keysym.sym == SDLK_UP) {
            dpadState &= 0b1011;
        } else if (event.key.keysym.sym == SDLK_LEFT) {
            dpadState &= 0b1101;
        } else if (event.key.keysym.sym == SDLK_RIGHT) {
            dpadState &= 0b1110;
        } else if (event.key.keysym.sym == SDLK_x) {
            buttonState &= 0b1110;
        } else if (event.key.keysym.sym == SDLK_z) {
            buttonState &= 0b1101;
        } else if (event.key.keysym.sym == SDLK_m) {
            buttonState &= 0b1011;
        } else if (event.key.keysym.sym == SDLK_n) {
            buttonState &= 0b0111;
        }
    } else if (event.type == SDL_KEYUP) {
         if (event.key.keysym.sym == SDLK_DOWN) {
            dpadState |= 0b1000;
        } else if (event.key.keysym.sym == SDLK_UP) {
            dpadState |= 0b0100;
        } else if (event.key.keysym.sym == SDLK_LEFT) {
            dpadState |= 0b0010;
        } else if (event.key.keysym.sym == SDLK_RIGHT) {
            dpadState |= 0b0001;
        } else if (event.key.keysym.sym == SDLK_x) {
            buttonState |= 0b0001;
        } else if (event.key.keysym.sym == SDLK_z) {
            buttonState |= 0b0010;
        } else if (event.key.keysym.sym == SDLK_m) {
            buttonState |= 0b0100;
        } else if (event.key.keysym.sym == SDLK_n) {
            buttonState |= 0b1000;
        }
    }
    return 0;
}

u8 Joypad::getButton() {
    return buttonState;
}

u8 Joypad::getDpad() {
    return dpadState;
}
