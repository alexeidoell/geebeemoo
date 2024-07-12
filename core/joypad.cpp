#include "joypad.h"
#include <SDL2/SDL_events.h>


u8 Joypad::pollPresses(SDL_Event& event) {
    inputState inputstate;
    if ((mem->read(0xFF00) & 0x30) == 0x20) inputstate = dpad;
    else if ((mem->read(0xFF00) & 0x30) == 0x10) inputstate = buttons;

    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_DOWN && inputstate == dpad) {
            mem->write(0xFF00, (u8)(mem->read(0xFF00) & 0b110111));
        } else if (event.key.keysym.sym == SDLK_UP && inputstate == dpad) {
            mem->write(0xFF00, (u8)(mem->read(0xFF00) & 0b111011));
        } else if (event.key.keysym.sym == SDLK_LEFT && inputstate == dpad) {
            mem->write(0xFF00, (u8)(mem->read(0xFF00) & 0b111101));
        } else if (event.key.keysym.sym == SDLK_RIGHT && inputstate == dpad) {
            mem->write(0xFF00, (u8)(mem->read(0xFF00) & 0b111110));
        } else if (event.key.keysym.sym == SDLK_x && inputstate == buttons) {
            mem->write(0xFF00, (u8)(mem->read(0xFF00) & 0b111110));
        } else if (event.key.keysym.sym == SDLK_z && inputstate == buttons) {
            mem->write(0xFF00, (u8)(mem->read(0xFF00) & 0b111101));
        } else if (event.key.keysym.sym == SDLK_m && inputstate == buttons) {
            mem->write(0xFF00, (u8)(mem->read(0xFF00) & 0b111011));
        } else if (event.key.keysym.sym == SDLK_n && inputstate == buttons) {
            mem->write(0xFF00, (u8)(mem->read(0xFF00) & 0b110111));
        }
    } else if (event.type == SDL_KEYUP) {
        if (event.key.keysym.sym == SDLK_DOWN && inputstate == dpad) {
            mem->write(0xFF00, (u8)(mem->read(0xFF00) | 0b1000));
        } else if (event.key.keysym.sym == SDLK_UP && inputstate == dpad) {
            mem->write(0xFF00, (u8)(mem->read(0xFF00) | 0b0100));
        } else if (event.key.keysym.sym == SDLK_LEFT && inputstate == dpad) {
            mem->write(0xFF00, (u8)(mem->read(0xFF00) | 0b0010));
        } else if (event.key.keysym.sym == SDLK_RIGHT && inputstate == dpad) {
            mem->write(0xFF00, (u8)(mem->read(0xFF00) | 0b0001));
        } else if (event.key.keysym.sym == SDLK_x && inputstate == buttons) {
            mem->write(0xFF00, (u8)(mem->read(0xFF00) | 0b0001));
        } else if (event.key.keysym.sym == SDLK_z && inputstate == buttons) {
            mem->write(0xFF00, (u8)(mem->read(0xFF00) | 0b0010));
        } else if (event.key.keysym.sym == SDLK_m && inputstate == buttons) {
            mem->write(0xFF00, (u8)(mem->read(0xFF00) | 0b0100));
        } else if (event.key.keysym.sym == SDLK_n && inputstate == buttons) {
            mem->write(0xFF00, (u8)(mem->read(0xFF00) | 0b1000));
        }
    }
    return 0;
}
