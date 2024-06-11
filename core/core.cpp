#include <memory>
#include <iostream>
#include "lib/types.h"

class Core {
private:
    u8 C = 0, B = 0, E = 0, D = 0, L = 0, H = 0, F = 0, A = 0;
    u16 SP; u16 PC;
    std::array<u8, 16384> memory = { 0 };
    void op_tree();
public:
    Core() {
        
    }
    ~Core();
};

void Core::op_tree() {
    u8 byte1 = memory[PC]; 

    if (byte1 == 0b01110110) { // halt op
    }
    else if (byte1 >= 0x40 && byte1 < 0x80) { // load op
        u8 src = byte1 & 0b111;
        u8 dst = (byte1 >> 3) & 0b111;

    }



}
