#include <gb.h>
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

int main(int argc, char* argv[]) {
//    std::unique_ptr<GB> testGB = std::make_unique<GB>();
    if (argc != 2) {
        std::cout << "intended usage: ./geebeemoo /path/to/game\n";
        return -1;
    }
    GB testGB;
    testGB.runEmu(argv[1]);


}
