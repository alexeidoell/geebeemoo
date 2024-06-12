#include <memory>
#include <iostream>
#include "../lib/types.h"
#include "core.h"

class Core {
private:
    gbRegisters registers;
    std::array<u8, 16384> memory = { 0 };
    void op_tree();
public:
    Core() {
        
    }
    ~Core();
};

void Core::op_tree() {
    u8 byte1 = memory[registers.pc]; 

    if (byte1 == 0b01110110) { // halt op
        // gotta implement halt
    }
    else if (byte1 >= 0x40 && byte1 < 0x80) { // load op
        u8 src = byte1 & 0b111;
        u8 dst = (byte1 >> 3) & 0b111;

        // HL in src or dst
        if (src == 6) {
            u16 hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
            registers.gpr.r[dst] = memory[hl];
        } else if (dst == 6) {
            u16 hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
            memory[hl] = registers.gpr.r[src];
        } else {
            registers.gpr.r[dst] = registers.gpr.r[src];
        }
    }
}
