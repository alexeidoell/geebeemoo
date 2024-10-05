#include <joypad.h>
#include <SDL2/SDL_events.h>

void Joypad::pollPresses(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_DOWN:
             dpadState &= 0b0111;
             break;
        case SDLK_UP:
             dpadState &= 0b1011;
             break;
        case SDLK_LEFT:
             dpadState &= 0b1101;
             break;
        case SDLK_RIGHT:
             dpadState &= 0b1110;
             break;
        case SDLK_x:
             buttonState &= 0b1110;
             break;
        case SDLK_z:
             buttonState &= 0b1101;
             break;
        case SDLK_m:
             buttonState &= 0b1011;
             break;
        case SDLK_n:
             buttonState &= 0b0111;
             break;
        default:
             break;
        }
    } else if (event.type == SDL_KEYUP) {
        switch (event.key.keysym.sym) {
        case SDLK_DOWN:
            dpadState |= 0b1000;
            break;
        case SDLK_UP:
            dpadState |= 0b0100;
            break;
        case SDLK_LEFT:
            dpadState |= 0b0010;
            break;
        case SDLK_RIGHT:
            dpadState |= 0b0001;
            break;
        case SDLK_x:
            buttonState |= 0b0001;
            break;
        case SDLK_z:
            buttonState |= 0b0010;
            break;
        case SDLK_m:
            buttonState |= 0b0100;
            break;
        case SDLK_n:
            buttonState |= 0b1000;
            break;
        default:
            break;
        }
    }
}

u8 Joypad::getButton() {
    return buttonState;
}

u8 Joypad::getDpad() {
    return dpadState;
}
