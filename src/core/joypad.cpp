#include <SDL3/SDL.h>
#include <joypad.h>

void Joypad::pollPresses(SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN) {
        switch (event.key.key) {
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
        case SDLK_X :
             buttonState &= 0b1110;
             break;
        case SDLK_Z :
             buttonState &= 0b1101;
             break;
        case SDLK_M :
             buttonState &= 0b1011;
             break;
        case SDLK_N :
             buttonState &= 0b0111;
             break;
        default:
             break;
        }
    } else if (event.type == SDL_EVENT_KEY_UP) {
        switch (event.key.key) {
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
        case SDLK_X :
            buttonState |= 0b0001;
            break;
        case SDLK_Z :
            buttonState |= 0b0010;
            break;
        case SDLK_M :
            buttonState |= 0b0100;
            break;
        case SDLK_N :
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
