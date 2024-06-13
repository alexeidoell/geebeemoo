#include <memory>
#include "mmu.h"
#include "../lib/types.h"
#include "core.h"

class Core {
private:
    gbRegisters registers; // need to figure out a good way
                           // of letting future video and audio
                           // implementations access memory
                           // maybe a shared pointer ?
    std::shared_ptr<MMU> mem;
public:
    u8 bootup();
    u8 op_tree();
    Core(std::shared_ptr<MMU> memPtr) {
        mem = memPtr;
    }
    ~Core();
};

u8 Core::bootup() {
    // set registers and memory to 0x100 state
    return 0;
}

u8 Core::op_tree() {
    u8 byte1 = mem->read(registers.pc); 
    u8 ticks = 4;

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
            registers.gpr.r[dst] = mem->read(hl);
        } else if (dst == 6) {
            ticks = 8;
            u16 hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
            mem->write(hl, registers.gpr.r[src]);
        } else {
            ticks = 8;
            registers.gpr.r[dst] = registers.gpr.r[src];
        }
    } else if (byte1 >= 0x80 && byte1 < 0xC0) { // arithmetic
        u8 operand;
        u8 operandValue;
        operand = byte1 & 0b111;
        if (operand != 6) operandValue = registers.gpr.r[operand];
        else {
            ticks = 8;
            u16 hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
            operandValue = mem->read(hl);
        }
        u16 result;

        if (byte1 < 0x90) { // addition
            registers.flags &=  0b11111101; // subtraction bit
            result = registers.gpr.n.a + operandValue;
            if (byte1 >= 0x88 && (registers.flags & 0b00000001) > 0) { // carry add
                result += 1;
                if ((((registers.gpr.n.a & 0xF) + (operandValue & 0xF) + 1) > 0xF)) registers.flags |= 0b00000100; // half carry bit
                else registers.flags &= 0b11111011;
            } else {
                if ((((registers.gpr.n.a & 0xF) + (operandValue & 0xF)) > 0xF)) registers.flags |= 0b00000100;
                else registers.flags &= 0b11111011;
            }

            if (result > 0xFF) registers.flags |= 0b00000001; // carry bit
            else registers.flags &= 0b11111110;
            if (result == 0) registers.flags |= 0b00001000; // zero bit
            else registers.flags &= 0b11110111;

            registers.gpr.n.a = result & 0xFF;
        } else if (byte1 < 0xA0) { // subtraction (there's no way its this simple)
            registers.flags |= 0b00000010; // subtraction bit
            result = registers.gpr.n.a - operandValue;
            if (byte1 >= 0x98 && (registers.flags & 0b00000001) > 0) { // carry subtraction
                result -= 1;
                if ((((registers.gpr.n.a & 0xF) - (operandValue & 0xF) - 1) > 0xF)) registers.flags |= 0b00000100; // half carry bit
                else registers.flags &= 0b11111011;
            } else {
                if ((((registers.gpr.n.a & 0xF) - (operandValue & 0xF)) > 0xF)) registers.flags |= 0b00000100;
                else registers.flags &= 0b11111011;
            }

            if (result > 0xFF) registers.flags |= 0b00000001; // carry bit
            else registers.flags &= 0b11111110;
            if (result == 0) registers.flags |= 0b00001000; // zero bit
            else registers.flags &= 0b11110111;

            registers.gpr.n.a = result & 0xFF;


        } else if (byte1 < 0xA8) { // logical AND
            registers.flags &= 0b11111010; // set subtraction and carry flag
            registers.flags |= 0b00000010; // set half carry flag
            result = registers.gpr.n.a & operandValue;
            if (result == 0) registers.flags |= 0b00001000; // zero flag
            else registers.flags &= 0b11110111;
            registers.gpr.n.a = result & 0xFF;
        } else if (byte1 < 0xB0) { // exclusive or
            registers.flags &= 0b11111000; // set subtraction, hc and carry flag
            result = registers.gpr.n.a ^ operandValue;
            if (result == 0) registers.flags |= 0b00001000; // zero flag
            else registers.flags &= 0b11110111;
            registers.gpr.n.a = result & 0xFF;
        }
    } 

    return ticks;
}
