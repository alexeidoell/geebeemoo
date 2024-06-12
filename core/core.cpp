#include <memory>
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

    if (byte1 == 0) { // nop
        // gotta implement nop
    }
    else if (byte1 >= 0x40 && byte1 < 0x80) { // load op
        u8 src = byte1 & 0b111;
        u8 dst = (byte1 >> 3) & 0b111;

        // HL in src or dst
        if (src == 6 && dst == 6) {
            // halt op
        }
        else if (src == 6) {
            u16 hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
            registers.gpr.r[dst] = memory[hl];
        } else if (dst == 6) {
            u16 hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
            memory[hl] = registers.gpr.r[src];
        } else {
            registers.gpr.r[dst] = registers.gpr.r[src];
        }
    } else if (byte1 >= 0x80 && byte1 < 0xC0) { // arithmetic
        if (byte1 < 0x88) { // addition
            registers.flags &=  0b11111101; // subtraction bit
            u8 operand = byte1 & 0b111;

            u16 result = registers.gpr.n.a + registers.gpr.r[operand];

            if (result > 0xFF) registers.flags |= 0b00001000; // carry bit
            else registers.flags &= 0b11110111;
            if (result == 0) registers.flags |= 0b00000001; // zero bit
            else registers.flags &= 0b11111110;
            if ((((registers.gpr.n.a & 0xF) + (registers.gpr.r[operand] & 0xF)) & 0x10) == 0x10) registers.flags |= 0b00000100; // half carry bit
            else registers.flags &= 0b11111011;

            registers.gpr.n.a = result & 0xFF;
        }
    }
}
