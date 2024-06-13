#include <memory>
#include "../lib/types.h"
#include "core.h"

class Core {
private:
    gbRegisters registers;
    std::array<u8, 16384> memory = { 0 };
    u8 op_tree();
public:
    Core() {
        
    }
    ~Core();
};

u8 Core::op_tree() {
    u8 byte1 = memory[registers.pc]; 
    u8 cycles = 4;

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
            cycles = 8;
            u16 hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
            memory[hl] = registers.gpr.r[src];
        } else {
            cycles = 8;
            registers.gpr.r[dst] = registers.gpr.r[src];
        }
    } else if (byte1 >= 0x80 && byte1 < 0xC0) { // arithmetic
        u8 operand;
        u8 operandValue;
        operand = byte1 & 0b111;
        if (operand != 6) operandValue = registers.gpr.r[operand];
        else {
            cycles = 8;
            u16 hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
            operandValue = memory[hl];
        }
        u16 result;

        if (byte1 < 0x90) { // addition
            registers.flags &=  0b11111101; // subtraction bit
            result = registers.gpr.n.a + operandValue;
            if (byte1 > 0x88) { // carry add
                result += 1;
                if ((((registers.gpr.n.a & 0xF) + (operandValue & 0xF) + 1) > 0xF)) registers.flags |= 0b00000100; // half carry bit
                else registers.flags &= 0b11111011;
            } else {
                if ((((registers.gpr.n.a & 0xF) + (operandValue & 0xF)) > 0xF)) registers.flags |= 0b00000100; // half carry bit
                else registers.flags &= 0b11111011;
            }

            if (result > 0xFF) registers.flags |= 0b00001000; // carry bit
            else registers.flags &= 0b11110111;
            if (result == 0) registers.flags |= 0b00000001; // zero bit
            else registers.flags &= 0b11111110;

            registers.gpr.n.a = result & 0xFF;
        }
    }
    return cycles;
}
