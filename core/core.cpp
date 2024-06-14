#include <memory>
#include <iostream>
#include "mmu.h"
#include "../lib/types.h"
#include "core.h"

Core::Core(std::unique_ptr<MMU> memPtr) {
    mem = std::move(memPtr);
}

u8 Core::bootup() {
    // set registers and memory to 0x100 state
    return 0;
}

u8 Core::op_tree() {
    u8 byte1 = mem->read(registers.pc++); 
    u8 ticks = 8; // 4 to fetch instruction + initial 4
                  // will probably change how timing works later

    if (byte1 == 0) { // nop
        // gotta implement nop
    }
    else if (byte1 == 0x10) { // STOP instruction

    }
    else if (byte1 < 0x40) { // operations 0x01 to 0x3F
        if ((byte1 & 0b11) == 0b01) { // two byte load ops
            ticks += 8;
            u8 imm_byte1 = mem->read(registers.pc++);
            u8 imm_byte2 = mem->read(registers.pc++);
            u8 dst = byte1 >> 4;
            if (dst == 0x00) {
                registers.gpr.n.c = imm_byte1;
                registers.gpr.n.b = imm_byte2;
            } else if (dst == 0x01) {
                registers.gpr.n.e = imm_byte1;
                registers.gpr.n.d = imm_byte2;
            } else if (dst == 0x02) {
                registers.gpr.n.l = imm_byte1;
                registers.gpr.n.h = imm_byte2;
            } else if (dst == 0x03) {
                u16 imm = (imm_byte2 << 8) + imm_byte1;
                registers.sp = imm;
            }
        }
        else if ((byte1 & 0b11) == 0b10) { // one byte load ops
            if ((byte1 & 0b111) == 0b110) { // load immediate
                ticks += 4;
                u8 dst = (byte1 >> 3) & 0b111;
                if (dst == 6) { // load into HL
                    ticks += 4;
                    u16 hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
                    mem->write(hl, mem->read(registers.pc++));
                } else {
                    registers.gpr.r[dst] = mem->read(registers.pc++);
                }
            } else if ((byte1 & 0b111) == 0b010) { // loading memory to/from A
                u8 dst = (byte1) >> 4;
                u16 address;
                if (dst == 0) address = (registers.gpr.n.b << 8) + registers.gpr.n.c;
                else if (dst == 1) address = (registers.gpr.n.d << 8) + registers.gpr.n.e;
                else if (dst == 2) {
                    u16 hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
                    address = hl;
                    hl += 1;
                    registers.gpr.n.h = hl >> 8;
                    registers.gpr.n.l = hl & 0xFF;
                } else if (dst == 3) {
                    u16 hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
                    address = hl;
                    hl -= 1;
                    registers.gpr.n.h = hl >> 8;
                    registers.gpr.n.l = hl & 0xFF;
                }
                if ((byte1 & 0b1000) == 0b1000) {
                    registers.gpr.n.a = mem->read(address);
                } else {
                    mem->write(address, registers.gpr.n.a);
                }

            }
        }
    }
    else if (byte1 < 0x80) { // load op
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
            if (byte1 >= 0x88 && (registers.flags & 0b00010000) > 0) { // carry add
                result += 1;
                if ((((registers.gpr.n.a & 0xF) + (operandValue & 0xF) + 1) > 0xF)) registers.flags |= 0b00100000; // half carry bit
                else registers.flags &= 0b11011111;
            } else {
                if ((((registers.gpr.n.a & 0xF) + (operandValue & 0xF)) > 0xF)) registers.flags |= 0b00100000;
                else registers.flags &= 0b11011111;
            }

            if (result > 0xFF) registers.flags |= 0b00010000; // carry bit
            else registers.flags &= 0b11101111;
            if ((result & 0xFF) == 0) registers.flags |= 0b10000000; // zero bit
            else registers.flags &= 0b01111111;

            registers.gpr.n.a = result & 0xFF;
        } else if (byte1 < 0xA0) { // subtraction (there's no way its this simple)
            registers.flags |= 0b01000000; // subtraction bit
            result = registers.gpr.n.a - operandValue;
            if (byte1 >= 0x98 && (registers.flags & 0b00010000) > 0) { // carry subtraction
                result -= 1;
                if ((((registers.gpr.n.a & 0xF) < (operandValue & 0xF) + 1))) registers.flags |= 0b00100000; // half carry bit
                else registers.flags &= 0b11011111;
            } else {
                if ((((registers.gpr.n.a & 0xF) < (operandValue & 0xF)))) registers.flags |= 0b00100000;
                else registers.flags &= 0b11011111;
            }

            if (operandValue > registers.gpr.n.a) registers.flags |= 0b00010000; // carry bit
            else registers.flags &= 0b11101111;
            if ((result & 0xFF) == 0) registers.flags |= 0b10000000; // zero bit
            else registers.flags &= 0b01111111;

            registers.gpr.n.a = result & 0xFF;


        } else if (byte1 < 0xA8) { // logical AND
            registers.flags &= 0b10101111; // set subtraction and carry flag
            registers.flags |= 0b00100000; // set half carry flag
            result = registers.gpr.n.a & operandValue;
            if (result == 0) registers.flags |= 0b10000000; // zero flag
            else registers.flags &= 0b01111111;
            registers.gpr.n.a = result & 0xFF;
        } else if (byte1 < 0xB0) { // exclusive or
            registers.flags &= 0b10001111; // set subtraction, hc and carry flag
            result = registers.gpr.n.a ^ operandValue;
            if (result == 0) registers.flags |= 0b10000000; // zero flag
            else registers.flags &= 0b01111111;
            registers.gpr.n.a = result & 0xFF;
        } else if (byte1 < 0xB8) { // or
            registers.flags &= 0b10001111; // set subtraction, hc and carry flag
            result = registers.gpr.n.a | operandValue;
            if (result == 0) registers.flags |= 0b10000000; // zero flag
            else registers.flags &= 0b01111111;
            registers.gpr.n.a = result & 0xFF;
        } else if (byte1 < 0xC0) { // compare operation
            registers.flags |= 0b01000000; // subtraction bit
            result = registers.gpr.n.a - operandValue;
                if ((((registers.gpr.n.a & 0xF) < (operandValue & 0xF)))) registers.flags |= 0b00100000;
                else registers.flags &= 0b11011111;

            if (operandValue > registers.gpr.n.a) registers.flags |= 0b00010000; // carry bit
            else registers.flags &= 0b11101111;
            if ((result & 0xFF) == 0) registers.flags |= 0b10000000; // zero bit
            else registers.flags &= 0b01111111;

 
        }
    } 

    return ticks;
}
