#include <memory>
#include "mmu.h"
#include "../lib/types.h"
#include "core.h"

Core::Core(std::shared_ptr<MMU> memPtr) {
    mem = memPtr;
}

u8 Core::bootup() {
    // set registers and memory to 0x100 state
    return 0;
}

u8 Core::op_tree() {
    u8 byte1 = mem->read(registers.pc); 
    registers.pc++;
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
            registers.flags &=  0b10111111; // subtraction bit
            result = registers.gpr.n.a + operandValue;
            if (byte1 >= 0x88 && (registers.flags & 0b10000000) > 0) { // carry add
                result += 1;
                if ((((registers.gpr.n.a & 0xF) + (operandValue & 0xF) + 1) > 0xF)) registers.flags |= 0b00100000; // half carry bit
                else registers.flags &= 0b11011111;
            } else {
                if ((((registers.gpr.n.a & 0xF) + (operandValue & 0xF)) > 0xF)) registers.flags |= 0b00100000;
                else registers.flags &= 0b11011111;
            }

            if (result > 0xFF) registers.flags |= 0b10000000; // carry bit
            else registers.flags &= 0b01111111;
            if (result == 0) registers.flags |= 0b00010000; // zero bit
            else registers.flags &= 0b11101111;

            registers.gpr.n.a = result & 0xFF;
        } else if (byte1 < 0xA0) { // subtraction (there's no way its this simple)
            registers.flags |= 0b01000000; // subtraction bit
            result = registers.gpr.n.a - operandValue;
            if (byte1 >= 0x98 && (registers.flags & 0b10000000) > 0) { // carry subtraction
                result -= 1;
                if ((((registers.gpr.n.a & 0xF) - (operandValue & 0xF) - 1) > 0xF)) registers.flags |= 0b00100000; // half carry bit
                else registers.flags &= 0b11011111;
            } else {
                if ((((registers.gpr.n.a & 0xF) - (operandValue & 0xF)) > 0xF)) registers.flags |= 0b00100000;
                else registers.flags &= 0b11011111;
            }

            if (result > 0xFF) registers.flags |= 0b10000000; // carry bit
            else registers.flags &= 0b01111111;
            if (result == 0) registers.flags |= 0b00010000; // zero bit
            else registers.flags &= 0b11101111;

            registers.gpr.n.a = result & 0xFF;


        } else if (byte1 < 0xA8) { // logical AND
            registers.flags &= 0b01011111; // set subtraction and carry flag
            registers.flags |= 0b01000000; // set half carry flag
            result = registers.gpr.n.a & operandValue;
            if (result == 0) registers.flags |= 0b00010000; // zero flag
            else registers.flags &= 0b11101111;
            registers.gpr.n.a = result & 0xFF;
        } else if (byte1 < 0xB0) { // exclusive or
            registers.flags &= 0b00011111; // set subtraction, hc and carry flag
            result = registers.gpr.n.a ^ operandValue;
            if (result == 0) registers.flags |= 0b00010000; // zero flag
            else registers.flags &= 0b11101111;
            registers.gpr.n.a = result & 0xFF;
        }
    } 

    return ticks;
}
