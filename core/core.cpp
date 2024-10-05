#include <bit>
#include <lib/types.h>
#include <mmu.h>
#include <core.h>


u8 Core::bootup() {
    // set registers and memory to 0x100 state
    registers.gpr.n.a = 0x01;
    registers.flags = 0xB0;
    registers.gpr.n.b = 0x00;
    registers.gpr.n.c = 0x13;
    registers.gpr.n.d = 0x00;
    registers.gpr.n.e = 0xD8;
    registers.gpr.n.h = 0x01;
    registers.gpr.n.l = 0x4D;
    registers.pc  = 0x0100;
    registers.sp  = 0xFFFE;
    mem.write(0xFF00, (u8)0xCF);
    mem.write(0xFF03, (u16)0xABCC);
    mem.write(0xFF0F, (u8)0xE1);
    mem.write(0xFF40, (u8)0x91);
    mem.write(0xFF41, (u8)0x81);
    mem.write(0xFF07, (u8)0xF8);
    mem.write(0xFF47, (u8)0xFC);
    mem.write(0xFF48, (u16)0x0000);

    mem.write(0xFF10, (u8)0x80);
    mem.write(0xFF11, (u8)0xBF);
    mem.write(0xFF12, (u8)0xF3);
    mem.write(0xFF13, (u8)0xFF);
    mem.write(0xFF14, (u8)0xBF);
    mem.write(0xFF16, (u8)0x3F);
    mem.write(0xFF17, (u8)0x00);
    mem.write(0xFF18, (u8)0xFF);
    mem.write(0xFF19, (u8)0xBF);
    mem.write(0xFF1A, (u8)0x7F);
    mem.write(0xFF1B, (u8)0xFF);
    mem.write(0xFF1C, (u8)0x9F);
    mem.write(0xFF1D, (u8)0xFF);
    mem.write(0xFF1E, (u8)0xBF);
    mem.write(0xFF20, (u8)0xFF);
    mem.write(0xFF21, (u8)0x00);
    mem.write(0xFF22, (u8)0x00);
    mem.write(0xFF23, (u8)0xBF);
    mem.write(0xFF24, (u8)0x77);
    mem.write(0xFF25, (u8)0xF3);
    mem.write(0xFF26, (u8)0xF1);

    return 0;
}

u8 Core::op_tree() {
    u8 ticks = 4; // it seems that instructions are pre fetched
                  // so there is no extra 4 ticks for fetching
    
    std::array<u8,0x100> tick_chart = {
    1,3,2,2,1,1,2,1,5,2,2,2,1,1,2,1,
	0,3,2,2,1,1,2,1,3,2,2,2,1,1,2,1,
	2,3,2,2,1,1,2,1,2,2,2,2,1,1,2,1,
	2,3,2,2,3,3,3,1,2,2,2,2,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	2,2,2,2,2,2,1,2,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	2,3,3,4,3,4,2,4,2,4,3,0,3,6,2,4,
	2,3,3,0,3,4,2,4,2,4,3,0,3,0,2,4,
	3,3,2,0,0,4,2,4,4,1,4,0,0,0,2,4,
	3,3,2,1,0,4,2,4,3,2,4,1,0,0,2,4 };


    if (halt_bug) {

    }

    if (halt_flag) {
        if (((mem.read(0xFFFF) & mem.read(0xFF0F)) != 0)) { // interrupt to be handled
            halt_flag = false;
        } else return ticks;
    }

    if (ime && (mem.read(0xFFFF) & mem.read(0xFF0F)) != 0) { // interrupt handling
        ticks += 16;
        if (((mem.read(0xFF0F) & 0b1) & (mem.read(0xFFFF) & 0b1)) != 0) { // vblank interrupt
            mem.write(0xFF0F, (u8)(mem.read(0xFF0F) & 0b11111110));
            registers.sp -= 2;
            mem.write(registers.sp, registers.pc);
            registers.pc = 0x40;
        } else if (((mem.read(0xFF0F) & 0b10) & (mem.read(0xFFFF) & 0b10)) != 0) { // lcd interrupt
            mem.write(0xFF0F, (u8)(mem.read(0xFF0F) & 0b11111101));
            registers.sp -= 2;
            mem.write(registers.sp, registers.pc);
            registers.pc = 0x48;
        } else if (((mem.read(0xFF0F) & 0b100) & (mem.read(0xFFFF) & 0b100)) != 0) { // timer interrupt
            mem.write(0xFF0F, (u8)(mem.read(0xFF0F) & 0b11111011));
            registers.sp -= 2;
            mem.write(registers.sp, registers.pc);
            registers.pc = 0x50;
        } else if (((mem.read(0xFF0F) & 0b1000) & (mem.read(0xFFFF) & 0b1000)) != 0) { // serial interrupt
            mem.write(0xFF0F, (u8)(mem.read(0xFF0F) & 0b11110111));
            registers.sp -= 2;
            mem.write(registers.sp, registers.pc);
            registers.pc = 0x58;
        } else if (((mem.read(0xFF0F) & 0b10000) & (mem.read(0xFFFF) & 0b10000)) != 0) { // joypad interrupt
            mem.write(0xFF0F, (u8)(mem.read(0xFF0F) & 0b11101111));
            registers.sp -= 2;
            mem.write(registers.sp, registers.pc);
            registers.pc = 0x60;
        }
        ime = false;
        return ticks;
    }
    u8 byte1 = mem.read(registers.pc++); 
    ticks = tick_chart[byte1] * 4;
    bool ei_op = false;


    if (byte1 == 0) { // nop
    }
    else if (byte1 == 0x10) { // STOP instruction
        registers.pc++;
    } else if (byte1 == 0xD3 || byte1 == 0xDB || byte1 == 0xDD ||
            byte1 == 0xE3 || byte1 == 0xE4 || byte1 == 0xEB ||
            byte1 == 0xEC || byte1 == 0xED || byte1 == 0xF4 ||
            byte1 == 0xFC || byte1 == 0xFD) {
        // lock up system
    } else if (byte1 < 0x40) { // operations 0x01 to 0x3F
        if (byte1 == 0x08) { // store sp at imm
            u16 address = mem.read(registers.pc++);
            address = address + (mem.read(registers.pc++) << 8);
            mem.write(address, registers.sp);
        } else if ((byte1 & 0b111) == 0) { // relative jumps
            s8 offset = std::bit_cast<s8>(mem.read(registers.pc++));
            if ((byte1 >> 3) == 0b11) { // unconditional jump
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
            registers.flags &= 0b10111111; // set subtraction flag
            u8 operand = (byte1 >> 4) & 0b111;
            u16 operandValue = 0;
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
            u8 imm_byte1 = mem.read(registers.pc++);
            u8 imm_byte2 = mem.read(registers.pc++);
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
                u8 dst = (byte1 >> 3) & 0b111;
                if (dst == 6) { // load into HL
                    u16 hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
                    mem.write(hl, mem.read(registers.pc++));
                } else {
                    registers.gpr.r[dst] = mem.read(registers.pc++);
                }
            } else if ((byte1 & 0b111) == 0b010) { // loading memory to/from A
                u8 dst = (byte1) >> 4;
                u16 address = 0;
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
                    registers.gpr.n.a = mem.read(address);
                } else {
                    mem.write(address, registers.gpr.n.a);
                }

            } 
        } else if ((byte1 & 0b111) == 0b11) { // 2 byte increment/decrement
            s8 operand = 0;
            u16 reg = 0;
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
            u8 operand = 0;
            u16 hl = 0;
            if (dst != 6) {
                operand = registers.gpr.r[dst];
            } else {
                hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
                operand = mem.read(hl);
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
                mem.write(hl, (u8) result);
            }
        } else if ((byte1 & 0b111) == 0b101) { // one byte decrement
            u8 dst = (byte1 >> 3) & 0b111;
            registers.flags |= 0b01000000; // set subtraction flag
            u8 operand = 0;
            u16 hl = 0;
            if (dst != 6) {
                operand = registers.gpr.r[dst];
            } else {
                hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
                operand = mem.read(hl);
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
                mem.write(hl, (u8) result);
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
                if (registers.gpr.n.a == 0) registers.flags |= 0b10000000;
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
            if (!ime && (mem.read(0xFFFF) & mem.read(0xFF0F)) != 0) { 
                halt_bug = true;
            } else halt_flag = true;           
        } else if (src == 6) {
            u16 hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
            registers.gpr.r[dst] = mem.read(hl);
        } else if (dst == 6) {
            u16 hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
            mem.write(hl, registers.gpr.r[src]);
        } else {
            registers.gpr.r[dst] = registers.gpr.r[src];
        }
    } else if (byte1 >= 0x80 && byte1 < 0xC0) { // arithmetic
        u8 operand = 0;
        u8 operandValue = 0;
        operand = byte1 & 0b111;
        if (operand != 6) operandValue = registers.gpr.r[operand];
        else {
            u16 hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
            operandValue = mem.read(hl);
        }
        u16 result = 0;

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
        registers.sp = (registers.gpr.n.h << 8) + registers.gpr.n.l;
    } else if (byte1 == 0xCB) { //  cb instructions
        ticks += cb_op();
    } else if (byte1 == 0xF3) { // di
        ime = false;
    } else if (byte1 == 0xFB) { // ei
        ei_op = true;
        ei_set = true;
    } else if ((byte1 & 0b111) == 0b110) { // arithmetic on A register with immediate
        u8 operandValue = mem.read(registers.pc++);
        u8 operation = (byte1 >> 4) & 0b11;
        u16 result = 0;
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
                if ((operandValue + 1) > registers.gpr.n.a) registers.flags |= 0b00010000; // carry bit
                else registers.flags &= 0b11101111;
            } else {
                if ((((registers.gpr.n.a & 0xF) < (operandValue & 0xF)))) registers.flags |= 0b00100000;
                else registers.flags &= 0b11011111;
                if (operandValue > registers.gpr.n.a) registers.flags |= 0b00010000; // carry bit
                else registers.flags &= 0b11101111;
            }

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
        u8 dst = (byte1 >> 4) & 0b11;
        if ((byte1 & 0b111) == 0b1) { // pop
            u8 lowReg = mem.read(registers.sp++);
            u8 highReg = mem.read(registers.sp++);
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
            u16 regValue = 0;
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
            mem.write(registers.sp, regValue);
        }
    } else if ((byte1 & 0b111) == 0b111 ) { // vector calls
        u16 address = (byte1 & 0b00111000);
        registers.sp -= 2;
        mem.write(registers.sp, registers.pc);
        registers.pc = address;
    } else if ((byte1 >> 5) == 0b110) { // jumps and returns
        if ((byte1 & 0b111) < 0b10) { // returns
            u16 address = mem.read(registers.sp++);
            address = address + (mem.read(registers.sp++) << 8);
            if (byte1 == 0xC9) { // unconditional return
                registers.pc = address;
            } else if (byte1 == 0xD9) { // reti
                ime = true;
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
            u16 address = mem.read(registers.pc++);
            address = address + (mem.read(registers.pc++) << 8);
            if (byte1 == 0xC3) { // unconditional jump
                registers.pc = address;
            } else if ((byte1 & 0b111) == 2) { // conditional jump
                u8 condition = (byte1 >> 3) & 0b11;
                bool carry_flag = ((registers.flags >> 4) & 0b1) == 1;
                bool zero_flag = ((registers.flags >> 7) & 0b1) == 1;
                if ((condition == 0 && !zero_flag) || (condition == 1 && zero_flag) ||
                    (condition == 2 && !carry_flag) || (condition == 3 && carry_flag)) {
                    registers.pc = address;
                    ticks += 4;
                }
            }
 

        } else if ((byte1 & 0b110) == 0b100) { // calls
            u16 address = mem.read(registers.pc++);
            address = address + (mem.read(registers.pc++) << 8);
            if ((byte1 & 0b1) == 0b1) { // unconditional call
                registers.sp -= 2;
                mem.write(registers.sp, registers.pc);
                registers.pc = address;
            } else { // conditional calls
                u8 condition = (byte1 >> 3) & 0b11;
                bool carry_flag = ((registers.flags >> 4) & 0b1) == 1;
                bool zero_flag = ((registers.flags >> 7) & 0b1) == 1;
                if ((condition == 0 && !zero_flag) || (condition == 1 && zero_flag) ||
                    (condition == 2 && !carry_flag) || (condition == 3 && carry_flag)) {
                    registers.sp -= 2;
                    mem.write(registers.sp, registers.pc);
                    registers.pc = address;
                    ticks += 12;
                }
            }
        }
    } else if (byte1 == 0xE9) { // jp hl
        registers.pc = ((u16)registers.gpr.n.h << 8) + registers.gpr.n.l;
    } else if ((byte1 >> 5) == 0b111) { // stack and heap operations
        if (((byte1 & 0b1111) == 0b1000)) { // both instructions add an immediate to sp
            registers.flags &= 0b00111111; // set zero and subtraction flags
            s8 operandValue = std::bit_cast<s8>(mem.read(registers.pc++));
            u16 result = registers.sp + operandValue;
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
                registers.gpr.n.l = result & 0xFF;
                registers.gpr.n.h = result >> 8;
            } else registers.sp = result;
        } else {
            u16 address = 0;
            if ((byte1 & 0b1111) == 0b0000) { // load to and from imm8
                address = 0xFF00 + (mem.read(registers.pc++));
            } else if ((byte1 & 0b1111) == 0b0010) { // c register
                address = 0xFF00 + (registers.gpr.n.c);
            } else if ((byte1 & 0b1111) == 0b1010) {
                address = mem.read(registers.pc++);
                address = address + (mem.read(registers.pc++) << 8);
            }
            if ((byte1 >> 4) == 0xE) {
                mem.write(address, registers.gpr.n.a);
            } else registers.gpr.n.a = mem.read(address);
        }
    }
    registers.flags &= 0xF0;
    if (ei_set && !ei_op) {
        ime = true;
        ei_set = false;
    }
    //std::cout << (int)ticks << "\n";
    return ticks;
}

u8 Core::cb_op() {
    u8 byte2 = mem.read(registers.pc++);
    u8 dst = byte2 & 0b111;
    u16 hl = 0;
    if (dst == 6) {
        hl = ((u16)registers.gpr.n.h << 8) + registers.gpr.n.l;
    }

    if ((byte2 >> 6) > 0) { // individual bit operations
        if ((byte2 >> 6) == 1) { // bit test
            registers.flags |= 0b00100000;
            registers.flags &= 0b10111111;
            u8 bit = (byte2 >> 3) & 0b111;
            u8 check = 0;
            if (dst != 6) {
                check = (registers.gpr.r[dst] >> bit) & 0b1;
            } else {
                check = (mem.read(hl) >> bit) & 0b1;
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
                u8 operand = mem.read(hl);
                operand &= mask;
                mem.write(hl, operand);
            }
        } else { // set bit
            u8 bit = (byte2 >> 3) & 0b111;
            u8 mask = 0b00000001 << bit;
            if (dst != 6) {
                registers.gpr.r[dst] |= mask;
            } else {
                u8 operand = mem.read(hl);
                operand |= mask;
                mem.write(hl, operand);
            }
        }
    } else if ((byte2 >> 5) == 0) { // rotates
            registers.flags &= 0b10011111; // set subtraction and half carry
            u8 operand = 0;
            if (dst == 6) {
                operand = mem.read(hl);
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
                mem.write(hl, (u8)((operand << 1) + msb));
            } else {
                registers.gpr.r[dst] = (operand << 1) + msb;
            }
            if ((((operand << 1) + msb) & 0xFF) == 0) registers.flags |= 0b10000000;
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
                mem.write(hl, (u8)((operand >> 1) + (lsb << 7)));
            } else {
                registers.gpr.r[dst] = (operand >> 1) + (lsb << 7);
            }
            if ((((operand >> 1) + (lsb << 7)) & 0xFF) == 0) registers.flags |= 0b10000000;
            else registers.flags &= 0b01111111;
        }
    } else { // shifts and swap
        if ((byte2 >> 3) == 0b00110) { // swap
            registers.flags &= 0b10001111; // reset all flags except zero
            if (dst == 6) {
                u8 operand = mem.read(hl);
                u8 lower = operand & 0xF;
                u8 higher = operand >> 4;
                mem.write(hl, (u8)((lower << 4) + higher));
                if (operand == 0) registers.flags |= 0b10000000;
                else registers.flags &= 0b01111111;

            } else {
                u8 lower = registers.gpr.r[dst] & 0xF;
                u8 higher = registers.gpr.r[dst] >> 4;
                registers.gpr.r[dst] = (lower << 4) + higher;
                if (registers.gpr.r[dst] == 0) registers.flags |= 0b10000000;
                else registers.flags &= 0b01111111;

            }
        } else if ((byte2 >> 3) == 0b00100) { // left shift
            registers.flags &= 0b10011111; // set subtraction and half carry
            u8 operand = 0;
            if (dst == 6) {
                operand = mem.read(hl);
            } else operand = registers.gpr.r[dst];
            u8 msb = (operand >> 7) & 0b1;
            if (msb == 1) registers.flags |= 0b00010000;
            else registers.flags &= 0b11101111;
            if (dst == 6) {
                mem.write(hl, (u8)((operand << 1)));
            } else {
                registers.gpr.r[dst] = (operand << 1);
            }
            if (((operand << 1) & 0xFF) == 0) registers.flags |= 0b10000000;
            else registers.flags &= 0b01111111;

        } else { // right shifts
            registers.flags &= 0b10011111; // set subtraction and half carry
            u8 operand = 0;
            if (dst == 6) {
                operand = mem.read(hl);
            } else operand = registers.gpr.r[dst];
            u8 lsb = operand & 0b1;
            if (lsb == 1) registers.flags |= 0b00010000;
            else registers.flags &= 0b11101111;
            
            operand = (s8)operand >> 1;
            if (((byte2 >> 4) & 0b1) == 1) { // logical right shift
                operand &= 0b01111111;
            }
            if (dst == 6) {
                mem.write(hl, (u8)((operand)));
            } else {
                registers.gpr.r[dst] = (operand);
            }
            if (operand == 0) registers.flags |= 0b10000000;
            else registers.flags &= 0b01111111;
        }
            

    }
    std::array<u8,0x100> tick_chart = {
    2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
	2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
	2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
	2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2 };

    return tick_chart[byte2] * 4;
}
