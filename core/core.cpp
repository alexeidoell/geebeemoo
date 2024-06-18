#include <memory>
#include <iostream>
#include <bit>
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
    u8 ticks = 4; // it seems that instructions are pre fetched
                  // so there is no extra 4 ticks for fetching

    if (ei_set) {
        ime = true;
        ei_set = false;
    }

    if (byte1 == 0) { // nop
        // gotta implement nop
    }
    else if (byte1 == 0x10) { // STOP instruction

    } else if (byte1 == 0xD3 || byte1 == 0xDB || byte1 == 0xDD ||
            byte1 == 0xE3 || byte1 == 0xE4 || byte1 == 0xEB ||
            byte1 == 0xEC || byte1 == 0xED || byte1 == 0xF4 ||
            byte1 == 0xFC || byte1 == 0xFD) {
        // lock up system
    } else if (byte1 < 0x40) { // operations 0x01 to 0x3F
        if (byte1 == 0x08) { // store sp at imm
            ticks += 16;
            u16 address = mem->read(registers.pc++);
            address = address + (mem->read(registers.pc++) << 8);
            mem->write(address, registers.sp);
        } else if ((byte1 & 0b111) == 0) { // relative jumps
            ticks += 4;
            s8 offset = mem->read(registers.pc++);
            if ((byte1 >> 3) == 0b11) { // unconditional jump
                ticks += 4;
                registers.pc += offset;
            } else if ((byte1 >> 5) == 1) { // conditional jumps
                u8 condition = (byte1 >> 3) & 0b11;
                bool carry_flag = ((registers.flags >> 4) & 0b1) == 1;
                bool zero_flag = ((registers.flags >> 7) & 0b1) == 1;
                if ((condition == 0 && !zero_flag) || (condition == 1 && zero_flag) ||
                    (condition == 2 && !carry_flag) || (condition == 3 && carry_flag)) {
                    registers.pc += offset;
                    ticks += 4;
                }
            }
        } else if ((byte1 & 0b1111) == 0b1001) { // two byte addition to hl
            ticks += 4;
            registers.flags &= 0b10111111; // set subtraction flag
            u8 operand = (byte1 >> 4) & 0b111;
            u16 operandValue;
            u16 hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
            switch (operand) {
            case 0:
                operandValue = (registers.gpr.n.b << 8) + registers.gpr.n.c;
                break;
            case 1:
                operandValue = (registers.gpr.n.d << 8) + registers.gpr.n.e;
                break;
            case 2:
                operandValue = hl;
                break;
            case 3:
                operandValue = registers.sp;
                break;
            }
            u16 result = hl + operandValue;
            if ((((hl & 0x0FFF) + (operandValue & 0x0FFF)) > 0xFFF)) registers.flags |= 0b00100000;
            else registers.flags &= 0b11011111;
            if ((hl + operandValue) > 0xFFFF) registers.flags |= 0b00010000;
            else registers.flags &= 0b11101111;
            registers.gpr.n.l = result & 0xFF;
            registers.gpr.n.h = result >> 8;


        } else if ((byte1 & 0b111) == 0b001) { // two byte imm load ops
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
            ticks += 4;
            if ((byte1 & 0b111) == 0b110) { // load immediate
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
        } else if ((byte1 & 0b111) == 0b11) { // 2 byte increment/decrement
            ticks += 4;
            s8 operand;
            u16 reg;
            if ((byte1 & 0b1000) == 0b1000) { //decrement
                operand = -1;
            } else operand = 1; // increment
            u8 dst = byte1 >> 4;
            if (dst == 0) { 
                reg = (registers.gpr.n.b << 8) + registers.gpr.n.c;
                reg += operand;
                registers.gpr.n.b = reg >> 8;
                registers.gpr.n.c = reg & 0xFF;
            } else if (dst == 1) {
                reg = (registers.gpr.n.d << 8) + registers.gpr.n.e;
                reg += operand;
                registers.gpr.n.d = reg >> 8;
                registers.gpr.n.e = reg & 0xFF;
            } else if (dst == 2) {
                reg = (registers.gpr.n.h << 8) + registers.gpr.n.l;
                reg += operand;
                registers.gpr.n.h = reg >> 8;
                registers.gpr.n.l = reg & 0xFF;
            } else if (dst == 3) {
                registers.sp += operand;
            }
        } else if ((byte1 & 0b111) == 0b100) { // one byte increments
            u8 dst = (byte1 >> 3) & 0b111;
            registers.flags &= 0b10111111; // set subtraction flag
            u8 operand;
            u16 hl;
            if (dst != 6) {
                operand = registers.gpr.r[dst];
            } else {
                ticks += 4;
                hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
                operand = mem->read(hl);
            }
            if ((operand & 0xF) == 0xF) registers.flags |= 0b00100000;
            else registers.flags &= 0b11011111;
            u16 result = operand + 1;
            if ((result & 0xFF) == 0) registers.flags |= 0b10000000;
            else registers.flags &= 0b01111111;
            result = result & 0xFF;
            if (dst != 6) {
                registers.gpr.r[dst] = result;
            } else {
                mem->write(hl, (u8) result);
            }
        } else if ((byte1 & 0b111) == 0b101) { // one byte decrement
            u8 dst = (byte1 >> 3) & 0b111;
            registers.flags |= 0b01000000; // set subtraction flag
            u8 operand;
            u16 hl;
            if (dst != 6) {
                operand = registers.gpr.r[dst];
            } else {
                ticks += 4;
                hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
                operand = mem->read(hl);
            }
            if ((operand & 0xF) == 0) registers.flags |= 0b00100000;
            else registers.flags &= 0b11011111;
            u16 result = operand - 1;
            if ((result & 0xFF) == 0) registers.flags |= 0b10000000;
            else registers.flags &= 0b01111111;
            result = result & 0xFF;
            if (dst != 6) {
                registers.gpr.r[dst] = result;
            } else {
                mem->write(hl, (u8) result);
            }

        } else if ((byte1 & 0b111) == 0b111) { // rotates and weird transformations
            if (byte1 < 0x20) { // rotates
                registers.flags &= 0b00011111; // set all except carry to 0
                if (((byte1 >> 3) & 0b1) == 0) { // left rotate
                    u8 msb = (registers.gpr.n.a >> 7) & 0b1;
                    u8 carry_flag = (registers.flags >> 4) & 0b1;
                        if (msb == 1) registers.flags |= 0b00010000;
                        else registers.flags &= 0b11101111;
                    if ((byte1 >> 4) == 1) { // non carry rotate
                        msb = carry_flag;
                    }
                    registers.gpr.n.a = (registers.gpr.n.a << 1) + msb;
                } else { // right rotate
                    u8 lsb = registers.gpr.n.a & 0b1;
                    u8 carry_flag = (registers.flags >> 4) & 0b1;
                        if (lsb == 1) registers.flags |= 0b00010000;
                        else registers.flags &= 0b11101111;
                    if ((byte1 >> 4) == 1) { // non carry rotate
                        lsb = carry_flag;
                    }
                    registers.gpr.n.a = (registers.gpr.n.a >> 1) + (lsb << 7);
                }
            } else if (byte1 == 0x27) {
                bool carry_flag = ((registers.flags >> 4) & 0b1) == 1;
                bool half_carry_flag = ((registers.flags >> 5) & 0b1) == 1;
                bool subtraction_flag = ((registers.flags >> 6) & 0b1) == 1;
                if (!subtraction_flag) { // addition adjust
                    if (carry_flag || registers.gpr.n.a > 0x99) { 
                        registers.flags |= 0b00010000;
                        registers.gpr.n.a += 0x60;
                    } 
                    if (half_carry_flag || (registers.gpr.n.a & 0x0F) > 0x9) registers.gpr.n.a += 0x6;
                } else { // subtraction adjust
                    if (carry_flag) registers.gpr.n.a -= 0x60;
                    if (half_carry_flag) registers.gpr.n.a -= 0x6;

                }
                if (registers.gpr.n.a == 0) registers.flags |= 0b1000000;
                else registers.flags &= 0b01111111;
                registers.flags &= 0b11011111; 

            } else if (byte1 == 0x37) { // set carry flag
                registers.flags &= 0b10011111;
                registers.flags |= 0b00010000;
            } else if (byte1 == 0x2f) { // compliment accumulator
                registers.gpr.n.a = ~registers.gpr.n.a; 
                registers.flags |= 0b01100000;
            } else if (byte1 == 0x3f) { // compliment carry flag
                bool carry_flag = ((registers.flags >> 4) & 0b1) == 1;
                registers.flags &= 0b10011111;
                if (carry_flag) registers.flags &= 0b11101111;
                else registers.flags |= 0b00010000;
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
            ticks += 4;
            u16 hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
            mem->write(hl, registers.gpr.r[src]);
        } else {
            ticks += 4;
            registers.gpr.r[dst] = registers.gpr.r[src];
        }
    } else if (byte1 >= 0x80 && byte1 < 0xC0) { // arithmetic
        u8 operand;
        u8 operandValue;
        operand = byte1 & 0b111;
        if (operand != 6) operandValue = registers.gpr.r[operand];
        else {
            ticks += 4;
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

            if (result > 0xFF) registers.flags |= 0b00010000; // carry bit
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
    } else if (byte1 == 0xF9) { // ld sp, hl
                                // terrible organization of my if statements
                                // made me move this here
        ticks += 4;
        registers.sp = (registers.gpr.n.h << 8) + registers.gpr.n.l;
    } else if (byte1 == 0xCB) { //  cb instructions
        cb_op();
    } else if (byte1 == 0xF3) { // di
        ime = false;
    } else if (byte1 == 0xFB) { // ei
        ei_set = true;
    } else if ((byte1 & 0b111) == 0b110) { // arithmetic on A register with immediate
        ticks += 4;
        u8 operandValue = mem->read(registers.pc++);
        u8 operation = (byte1 >> 4) & 0b11;
        u16 result;
        switch (operation) {
        case 0:
            registers.flags &=  0b10111111; // subtraction bit
            result = registers.gpr.n.a + operandValue;
            if (byte1 >= 0xC8 && (registers.flags & 0b00010000) > 0) { // carry add
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
            break;
        case 1:
            registers.flags |= 0b01000000; // subtraction bit
            result = registers.gpr.n.a - operandValue;
            if (byte1 >= 0xD8 && (registers.flags & 0b00010000) > 0) { // carry subtraction
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
            break;
        case 2:
            if (byte1 >= 0xE8) { // xor
                registers.flags &= 0b10001111; // set subtraction, hc and carry flag
                result = registers.gpr.n.a ^ operandValue;
                if (result == 0) registers.flags |= 0b10000000; // zero flag
                else registers.flags &= 0b01111111;
                registers.gpr.n.a = result & 0xFF;
            } else { // and
                registers.flags &= 0b10101111; // set subtraction and carry flag
                registers.flags |= 0b00100000; // set half carry flag
                result = registers.gpr.n.a & operandValue;
                if (result == 0) registers.flags |= 0b10000000; // zero flag
                else registers.flags &= 0b01111111;
                registers.gpr.n.a = result & 0xFF;
            }
            break;
        case 3:
            if (byte1 >= 0xF8) { // cp
                registers.flags |= 0b01000000; // subtraction bit
                result = registers.gpr.n.a - operandValue;
                if ((((registers.gpr.n.a & 0xF) < (operandValue & 0xF)))) registers.flags |= 0b00100000;
                else registers.flags &= 0b11011111;
                if (operandValue > registers.gpr.n.a) registers.flags |= 0b00010000; // carry bit
                else registers.flags &= 0b11101111;
                if ((result & 0xFF) == 0) registers.flags |= 0b10000000; // zero bit
                else registers.flags &= 0b01111111;
            } else { // or
                registers.flags &= 0b10001111; // set subtraction, hc and carry flag
                result = registers.gpr.n.a | operandValue;
                if (result == 0) registers.flags |= 0b10000000; // zero flag
                else registers.flags &= 0b01111111;
                registers.gpr.n.a = result & 0xFF;
            }
            break;
        }
    } else if ((byte1 & 0b11001011) == 0b11000001) { // pop and push
        ticks += 8;
        u8 dst = (byte1 >> 4) & 0b11;
        if ((byte1 & 0b111) == 0b1) { // pop
            u8 lowReg = mem->read(registers.sp++);
            u8 highReg = mem->read(registers.sp++);
            switch (dst) {
                case 0:
                    registers.gpr.n.c = lowReg;
                    registers.gpr.n.b = highReg;
                    break;
                case 1:
                    registers.gpr.n.e = lowReg;
                    registers.gpr.n.d = highReg;
                    break;
                case 2:
                    registers.gpr.n.l = lowReg;
                    registers.gpr.n.h = highReg;
                    break;
                case 3:
                    registers.flags = lowReg;
                    registers.gpr.n.a = highReg;
                    break;
            }
 
        } else { // push
            u16 regValue;
            ticks += 4;
            switch (dst) {
                case 0:
                    regValue = (registers.gpr.n.b << 8) + registers.gpr.n.c;
                    break;
                case 1:
                    regValue = (registers.gpr.n.d << 8) + registers.gpr.n.e;
                    break;
                case 2:
                    regValue = (registers.gpr.n.h << 8) + registers.gpr.n.l;
                    break;
                case 3:
                    regValue = (registers.gpr.n.a << 8) + registers.flags;
                    break;
            }
            registers.sp -= 2;
            mem->write(registers.sp, regValue);
        }
    } else if ((byte1 & 0b111) == 0b111 ) { // vector calls
        u16 address = (byte1 & 0b00111000);
        registers.sp -= 2;
        mem->write(registers.sp, registers.pc);
        registers.pc = address;

 
    } else if ((byte1 >> 5) == 0b110) { // jumps and returns
        if ((byte1 & 0b111) < 0b10) { // returns
            ticks += 4;
            u16 address = mem->read(registers.sp++);
            address = address + (mem->read(registers.sp++) << 8);
            if (byte1 == 0xC9) { // unconditional return
                ticks += 8;
                registers.pc = address;
            } else if (byte1 == 0xD9) { // reti
                ime = true;
                ticks += 8;
                registers.pc = address;
            } else if ((byte1 & 0b111) == 0) { // conditional return
                u8 condition = (byte1 >> 3) & 0b11;
                bool carry_flag = ((registers.flags >> 4) & 0b1) == 1;
                bool zero_flag = ((registers.flags >> 7) & 0b1) == 1;
                if ((condition == 0 && !zero_flag) || (condition == 1 && zero_flag) ||
                    (condition == 2 && !carry_flag) || (condition == 3 && carry_flag)) {
                    ticks += 12;
                    registers.pc = address;
                } else {
                    registers.sp -= 2;
                }
            }
        } else if ((byte1 & 0b111) < 4) { // jp instructions
            ticks += 4;
            u16 address = mem->read(registers.pc++);
            address = address + (mem->read(registers.pc++) << 8);
            if (byte1 == 0xC3) { // unconditional jump
                ticks += 4;
                registers.pc = address;
            } else if ((byte1 & 0b111) == 2) { // conditional jump
                u8 condition = (byte1 >> 3) & 0b11;
                bool carry_flag = ((registers.flags >> 4) & 0b1) == 1;
                bool zero_flag = ((registers.flags >> 7) & 0b1) == 1;
                if ((condition == 0 && !zero_flag) || (condition == 1 && zero_flag) ||
                    (condition == 2 && !carry_flag) || (condition == 3 && carry_flag)) {
                    ticks += 4;
                    registers.pc = address;
                }
            }
 

        } else if ((byte1 & 0b110) == 0b100) { // calls
            ticks += 8;
            u16 address = mem->read(registers.pc++);
            address = address + (mem->read(registers.pc++) << 8);
            if ((byte1 & 0b1) == 0b1) { // unconditional call
                ticks += 12;
                registers.sp -= 2;
                mem->write(registers.sp, registers.pc);
                registers.pc = address;
            } else { // conditional calls
                u8 condition = (byte1 >> 3) & 0b11;
                bool carry_flag = ((registers.flags >> 4) & 0b1) == 1;
                bool zero_flag = ((registers.flags >> 7) & 0b1) == 1;
                if ((condition == 0 && !zero_flag) || (condition == 1 && zero_flag) ||
                    (condition == 2 && !carry_flag) || (condition == 3 && carry_flag)) {
                    registers.sp -= 2;
                    mem->write(registers.sp, registers.pc);
                    registers.pc = address;
                    ticks += 12;
                }


            }

        }
    } else if (byte1 == 0xE9) { // jp hl
        registers.pc = ((u16)registers.gpr.n.h << 8) + registers.gpr.n.l;
    } else if ((byte1 >> 5) == 0b111) { // stack and heap operations
        if (((byte1 & 0b1111) == 0b1000)) { // both instructions add an immediate to sp
            ticks += 8;
            registers.flags &= 0b00111111; // set zero and subtraction flags
            s8 operandValue = mem->read(registers.pc++);
            s16 result = registers.sp + operandValue;
            if ((((registers.sp & 0xF) + (operandValue & 0xF)) > 0xF)) registers.flags |= 0b00100000;
            else registers.flags &= 0b11011111;

            if (operandValue >= 0) {
                if (((registers.sp & 0xFF) + operandValue) > 0xFF) registers.flags |= 0b00010000;
                else registers.flags &= 0b11101111;
            } else { // negative operand I HATE THIS
                if (((registers.sp & 0xFF) + (u8)operandValue) > 0xFF) registers.flags |= 0b00010000;
                else registers.flags &= 0b11101111;
            }


            if (byte1 == 0xF8) { // load into hl 
                ticks -= 4;
                registers.gpr.n.l = result & 0xFF;
                registers.gpr.n.h = result >> 8;
            } else registers.sp = result;
        } else {
            u16 address;
            if ((byte1 & 0b1111) == 0b0000) { // load to and from imm8
                ticks += 8;
                address = 0xFF00 + (mem->read(registers.pc++));
            } else if ((byte1 & 0b1111) == 0b0010) { // c register
                address = 0xFF00 + (registers.gpr.n.c);
                ticks += 4;
            } else if ((byte1 & 0b1111) == 0b1010) {
                address = mem->read(registers.pc++);
                address = address + (mem->read(registers.pc++) << 8);
            }
            if ((byte1 >> 4) == 0xE) {
                mem->write(address, registers.gpr.n.a);
            } else registers.gpr.n.a = mem->read(address);
        }
    }
    return ticks;
}

u8 Core::cb_op() {
    u8 ticks = 4;
    u8 byte2 = mem->read(registers.pc++);
    u8 dst = byte2 & 0b111;
    u16 hl;
    if (dst == 6) {
        hl = ((u16)registers.gpr.n.h << 8) + registers.gpr.n.l;
    }

    if ((byte2 >> 6) > 0) { // individual bit operations
        if ((byte2 >> 6) == 1) { // bit test
            registers.flags |= 0b00100000;
            registers.flags &= 0b10111111;
            u8 bit = (byte2 >> 3) & 0b111;
            u8 check;
            if (dst != 6) {
                check = (registers.gpr.r[dst] >> bit) & 0b1;
            } else {
                check = (mem->read(hl) >> bit) & 0b1;
            }
            if (check == 0) {
                registers.flags |= 0b10000000;
            } else registers.flags &= 0b01111111;
        } else if ((byte2 >> 6) == 2) { // reset bit
            u8 bit = (byte2 >> 3) & 0b111;
            u8 mask = 0b11111110;
            for (int i = 0; i < bit; ++i) {
                mask <<= 1;
                mask += 1;
            }
            if (dst != 6) {
                registers.gpr.r[dst] &= mask;
            } else {
                u8 operand = mem->read(hl);
                operand &= mask;
                mem->write(hl, operand);
            }
        } else { // set bit
            u8 bit = (byte2 >> 3) & 0b111;
            u8 mask = 0b00000001 << bit;
            if (dst != 6) {
                registers.gpr.r[dst] |= mask;
            } else {
                u8 operand = mem->read(hl);
                operand |= mask;
                mem->write(hl, operand);
            }
        }
    } else if ((byte2 >> 5) == 0) { // rotates
            registers.flags &= 0b10011111; // set subtraction and half carry
            u8 operand;
            if (dst == 6) {
                operand = mem->read(hl);
            } else operand = registers.gpr.r[dst];
        if (((byte2 >> 3) & 0b1) == 0) { // left rotate
            u8 msb = (operand >> 7) & 0b1;
            u8 carry_flag = (registers.flags >> 4) & 0b1;
            if (msb == 1) registers.flags |= 0b00010000;
            else registers.flags &= 0b11101111;
            if ((byte2 >> 4) == 1) { // non carry rotate
                msb = carry_flag;
            }
            if (dst == 6) {
                mem->write(hl, (u8)((operand << 1) + msb));
            } else {
                registers.gpr.r[dst] = (operand << 1) + msb;
            }
            if ((operand << 1) + msb == 0) registers.flags |= 0b10000000;
            else registers.flags &= 0b01111111;
        } else { // right rotate
            u8 lsb = operand & 0b1;
            u8 carry_flag = (registers.flags >> 4) & 0b1;
            if (lsb == 1) registers.flags |= 0b00010000;
            else registers.flags &= 0b11101111;
            if ((byte2 >> 4) == 1) { // non carry rotate
                lsb = carry_flag;
            }
            if (dst == 6) {
                mem->write(hl, (u8)((operand >> 1) + (lsb << 7)));
            } else {
                registers.gpr.r[dst] = (operand >> 1) + (lsb << 7);
            }
            if ((operand >> 1) + (lsb << 7) == 0) registers.flags |= 0b10000000;
            else registers.flags &= 0b01111111;
        }



    }

    return ticks;
}
