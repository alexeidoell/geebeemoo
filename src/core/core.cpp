#include <bit>
#include <lib/types.h>
#include <mmu.h>
#include <core.h>

void Core::bootup() {
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
    mem.write(0xFF00, 0xCF);
    mem.dwrite(0xFF03, 0xABCC);
    mem.write(0xFF0F, 0xE1);
    mem.write(0xFF40, 0x91);
    mem.write(0xFF41, 0x81);
    mem.write(0xFF07, 0xF8);
    mem.write(0xFF47, 0xFC);
    mem.write(0xFF48, 0x00);
    mem.write(0xFF49, 0x00);

    // need to add the rest of the boot up process
    // maybe memmove a static const array based on
    // dmg or cgb?

    mem.write(0xFF10, 0x80);
    mem.write(0xFF11, 0xBF);
    mem.write(0xFF12, 0xF3);
    mem.write(0xFF13, 0xFF);
    mem.write(0xFF14, 0xBF);
    mem.write(0xFF16, 0x3F);
    mem.write(0xFF17, 0x00);
    mem.write(0xFF18, 0xFF);
    mem.write(0xFF19, 0xBF);
    mem.write(0xFF1A, 0x7F);
    mem.write(0xFF1B, 0xFF);
    mem.write(0xFF1C, 0x9F);
    mem.write(0xFF1D, 0xFF);
    mem.write(0xFF1E, 0xBF);
    mem.write(0xFF20, 0xFF);
    mem.write(0xFF21, 0x00);
    mem.write(0xFF22, 0x00);
    mem.write(0xFF23, 0xBF);
    mem.write(0xFF24, 0x77);
    mem.write(0xFF25, 0xF3);
    mem.write(0xFF26, 0xF1);
}

u8 Core::op_tree() {

    constexpr static std::array<u8,0x100> tick_chart = {
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

    u8 byte1 = 0;
    u8 ticks = 4;
    u8 operand = 0;
    u8 result = 0;
    u8 carry_value = 0;
    u8 imm_byte1 = 0;
    u8 imm_byte2 = 0;
    u8 msb = 0;
    u8 lsb = 0;
    u16 hl = (registers.gpr.n.h << 8) + registers.gpr.n.l;
    u16 dword_operand = 0;
    u16 dword_result = 0;
    u16 address = 0;
    s8 offset = 0;
    bool zero_flag = false;
    bool carry_flag = false;
    bool half_carry_flag = false;;
    bool subtraction_flag = false;
    bool ei_op = false;

    // halt handling

    if (halt_bug) {

    }

    if (halt_flag) {
        if (((mem.read(IE) & mem.read(IF)) != 0)) { // interrupt to be handled
            halt_flag = false;
        } else return ticks;
    }

    if (ime && (mem.read(IE) & mem.read(IF)) != 0) { // interrupt handling
        ticks += 16;
        if (((mem.read(IF) & 0b1) & (mem.read(IE) & 0b1)) != 0) { // vblank interrupt
            mem.write(IF, (u8)(mem.read(IF) & 0b11111110));
            registers.sp -= 2;
            mem.dwrite(registers.sp, registers.pc);
            registers.pc = 0x40;
        } else if (((mem.read(IF) & 0b10) & (mem.read(IE) & 0b10)) != 0) { // lcd interrupt
            mem.write(IF, (u8)(mem.read(IF) & 0b11111101));
            registers.sp -= 2;
            mem.dwrite(registers.sp, registers.pc);
            registers.pc = 0x48;
        } else if (((mem.read(IF) & 0b100) & (mem.read(IE) & 0b100)) != 0) { // timer interrupt
            mem.write(IF, (u8)(mem.read(IF) & 0b11111011));
            registers.sp -= 2;
            mem.dwrite(registers.sp, registers.pc);
            registers.pc = 0x50;
        } else if (((mem.read(IF) & 0b1000) & (mem.read(IE) & 0b1000)) != 0) { // serial interrupt
            mem.write(IF, (u8)(mem.read(IF) & 0b11110111));
            registers.sp -= 2;
            mem.dwrite(registers.sp, registers.pc);
            registers.pc = 0x58;
        } else if (((mem.read(IF) & 0b10000) & (mem.read(IE) & 0b10000)) != 0) { // joypad interrupt
            mem.write(IF, (u8)(mem.read(IF) & 0b11101111));
            registers.sp -= 2;
            mem.dwrite(registers.sp, registers.pc);
            registers.pc = 0x60;
        }
        ime = false;
        return ticks;
    }

    // interrupt handling
    byte1 = mem.read(registers.pc++);
    ticks = tick_chart[byte1] * 4;

    switch (byte1) { // is this even worth it...
    case 0x00: // NOP
        break;
    case 0x01: // LD BC, n16
        imm_byte1 = mem.read(registers.pc++);
        imm_byte2 = mem.read(registers.pc++);
        registers.gpr.n.c = imm_byte1;
        registers.gpr.n.b = imm_byte2;
        break;
    case 0x02: // LD [BC], A
        address = (registers.gpr.n.b << 8) + registers.gpr.n.c;
        mem.write(address, registers.gpr.n.a);
        break;
    case 0x03: // INC BC
        dword_result = (registers.gpr.n.b << 8) + registers.gpr.n.c;
        dword_result += 1;
        registers.gpr.n.b = dword_result >> 8;
        registers.gpr.n.c = dword_result & 0xFF;
        break;
    case 0x04: // INC B
        registers.flags &= 0b10111111; // set subtraction flag
        if ((registers.gpr.n.b & 0xF) == 0xF) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        registers.gpr.n.b += 1;
        if ((registers.gpr.n.b & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.b &= 0xFF;
        break;
    case 0x05: // DEC B
        registers.flags |= 0b01000000; // set subtraction flag
        if ((registers.gpr.n.b & 0xF) == 0x0) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        registers.gpr.n.b -= 1;
        if ((registers.gpr.n.b & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.b &= 0xFF;
        break;
    case 0x06: // LD B, n8
        registers.gpr.n.b = mem.read(registers.pc++);
        break;
    case 0x07: // RLCA
        registers.flags &= 0b00011111;
        msb = (registers.gpr.n.a >> 7) & 0b1;
        if (msb == 1) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        registers.gpr.n.a = (registers.gpr.n.a << 1) + msb;
        break;
    case 0x08: // LD [a16], SP
        address = mem.read(registers.pc++);
        address = address + (mem.read(registers.pc++) << 8);
        mem.dwrite(address, registers.sp);
        break;
    case 0x09: // ADD HL, BC 
        registers.flags &= 0b10111111; // set subtraction flag
        dword_operand = (registers.gpr.n.b << 8) + registers.gpr.n.c;
        dword_result = hl + dword_operand;
        if ((((hl & 0x0FFF) + (dword_operand & 0x0FFF)) > 0xFFF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if ((hl + dword_operand) > 0xFFFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        registers.gpr.n.l = dword_result & 0xFF;
        registers.gpr.n.h = dword_result >> 8;
        break;
    case 0x0A: // LD A, [BC]
        address = (registers.gpr.n.b << 8) + registers.gpr.n.c;
        registers.gpr.n.a = mem.read(address);
        break;
    case 0x0B: // DEC BC
        dword_result = (registers.gpr.n.b << 8) + registers.gpr.n.c;
        dword_result -= 1;
        registers.gpr.n.b = dword_result >> 8;
        registers.gpr.n.c = dword_result & 0xFF;
        break;
    case 0x0C: // INC C
        registers.flags &= 0b10111111; // set subtraction flag
        if ((registers.gpr.n.c & 0xF) == 0xF) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        registers.gpr.n.c += 1;
        if ((registers.gpr.n.c & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.c &= 0xFF;
        break;
    case 0x0D: // DEC C
        registers.flags |= 0b01000000; // set subtraction flag
        if ((registers.gpr.n.c & 0xF) == 0x0) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        registers.gpr.n.c -= 1;
        if ((registers.gpr.n.c & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.c &= 0xFF;
        break;
    case 0x0E: // LD C, n8
        registers.gpr.n.c = mem.read(registers.pc++);
        break;
    case 0x0F: // RRCA
        registers.flags &= 0b00011111;
        lsb = registers.gpr.n.a & 0b1;
        if (lsb == 1) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        registers.gpr.n.a = (registers.gpr.n.a >> 1) + (lsb << 7);
        break;
    case 0x10: // STOP
        registers.pc++;
        break;
    case 0x11: // LD DE, n16
        imm_byte1 = mem.read(registers.pc++);
        imm_byte2 = mem.read(registers.pc++);
        registers.gpr.n.e = imm_byte1;
        registers.gpr.n.d = imm_byte2;
        break;
    case 0x12: // LD [DE], A
        address = (registers.gpr.n.d << 8) + registers.gpr.n.e;
        mem.write(address, registers.gpr.n.a);
        break;
    case 0x13: // INC DE
        dword_result = (registers.gpr.n.d << 8) + registers.gpr.n.e;
        dword_result += 1;
        registers.gpr.n.d = dword_result >> 8;
        registers.gpr.n.e = dword_result & 0xFF;
        break;
    case 0x14: // INC D
        registers.flags &= 0b10111111; // set subtraction flag
        if ((registers.gpr.n.d & 0xF) == 0xF) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        registers.gpr.n.d += 1;
        if ((registers.gpr.n.d & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.d &= 0xFF;
        break;
    case 0x15: // DEC D
        registers.flags |= 0b01000000; // set subtraction flag
        if ((registers.gpr.n.d & 0xF) == 0x0) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        registers.gpr.n.d -= 1;
        if ((registers.gpr.n.d & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.d &= 0xFF;
        break;
    case 0x16: // LD D, n8
        registers.gpr.n.d = mem.read(registers.pc++);
        break;
    case 0x17: // RLA
        registers.flags &= 0b00011111;
        msb = (registers.gpr.n.a >> 7) & 0b1;
        carry_flag = (registers.flags >> 4) & 0b1;
        if (msb == 1) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        registers.gpr.n.a = (registers.gpr.n.a << 1) + carry_flag;
        break;
    case 0x18: // JR e8
        offset = std::bit_cast<s8>(mem.read(registers.pc++));
        registers.pc += offset;
        break;
    case 0x19: // ADD HL, DE 
        registers.flags &= 0b10111111; // set subtraction flag
        dword_operand = (registers.gpr.n.d << 8) + registers.gpr.n.e;
        dword_result = hl + dword_operand;
        if ((((hl & 0x0FFF) + (dword_operand & 0x0FFF)) > 0xFFF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if ((hl + dword_operand) > 0xFFFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        registers.gpr.n.l = dword_result & 0xFF;
        registers.gpr.n.h = dword_result >> 8;
        break;
    case 0x1A: // LD A, [DE]
        address = (registers.gpr.n.d << 8) + registers.gpr.n.e;
        registers.gpr.n.a = mem.read(address);
        break;
    case 0x1B: // DEC DE
        dword_result = (registers.gpr.n.d << 8) + registers.gpr.n.e;
        dword_result -= 1;
        registers.gpr.n.d = dword_result >> 8;
        registers.gpr.n.e = dword_result & 0xFF;
        break;
    case 0x1C: // INC E
        registers.flags &= 0b10111111; // set subtraction flag
        if ((registers.gpr.n.e & 0xF) == 0xF) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        registers.gpr.n.e += 1;
        if ((registers.gpr.n.e & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.e &= 0xFF;
        break;
    case 0x1D: // DEC E
        registers.flags |= 0b01000000; // set subtraction flag
        if ((registers.gpr.n.e & 0xF) == 0x0) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        registers.gpr.n.e -= 1;
        if ((registers.gpr.n.e & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.e &= 0xFF;
        break;
    case 0x1E: // LD E, n8
        registers.gpr.n.e = mem.read(registers.pc++);
        break;
    case 0x1F: // RRA
        registers.flags &= 0b00011111; // set all except carry to 0
        lsb = registers.gpr.n.a & 0b1;
        carry_flag = (registers.flags >> 4) & 0b1;
        if (lsb == 1) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        registers.gpr.n.a = (registers.gpr.n.a >> 1) + (carry_flag << 7);
        break;
    case 0x20: // JR NZ, e8
        offset = std::bit_cast<s8>(mem.read(registers.pc++));
        zero_flag = ((registers.flags >> 7) & 0b1) == 1;
        if (!zero_flag) {
            ticks += 4;
            registers.pc += offset;
        }
        break;
    case 0x21: // LD HL, n16
        imm_byte1 = mem.read(registers.pc++);
        imm_byte2 = mem.read(registers.pc++);
        registers.gpr.n.l = imm_byte1;
        registers.gpr.n.h = imm_byte2;
        break;
    case 0x22: // LD [HL+], A
        address = hl;
        hl += 1;
        registers.gpr.n.h = hl >> 8;
        registers.gpr.n.l = hl & 0xFF;
        mem.write(address, registers.gpr.n.a);
        break;
    case 0x23: // INC HL
        dword_result = (registers.gpr.n.h << 8) + registers.gpr.n.l;
        dword_result += 1;
        registers.gpr.n.h = dword_result >> 8;
        registers.gpr.n.l = dword_result & 0xFF;
        break;
    case 0x24: // INC H
        registers.flags &= 0b10111111;
        if ((registers.gpr.n.h & 0xF) == 0xF) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        registers.gpr.n.h += 1;
        if ((registers.gpr.n.h & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.h &= 0xFF;
        break;
    case 0x25: // DEC H
        registers.flags |= 0b01000000;
        if ((registers.gpr.n.h & 0xF) == 0x0) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        registers.gpr.n.h -= 1;
        if ((registers.gpr.n.h & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.h &= 0xFF;
        break;
    case 0x26: // LD H, n8
        registers.gpr.n.h = mem.read(registers.pc++);
        break;
    case 0x27: // DAA
        carry_flag = ((registers.flags >> 4) & 0b1) == 1;
        half_carry_flag = ((registers.flags >> 5) & 0b1) == 1;
        subtraction_flag = ((registers.flags >> 6) & 0b1) == 1;
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
        break;
    case 0x28: // JR Z, e8
        offset = std::bit_cast<s8>(mem.read(registers.pc++));
        zero_flag = ((registers.flags >> 7) & 0b1) == 1;
        if (zero_flag) {
            ticks += 4;
            registers.pc += offset;
        }
        break;
    case 0x29: // ADD HL, HL 
        registers.flags &= 0b10111111; // set subtraction flag
        dword_operand = hl;
        dword_result = hl + dword_operand;
        if ((((hl & 0x0FFF) + (dword_operand & 0x0FFF)) > 0xFFF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if ((hl + dword_operand) > 0xFFFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        registers.gpr.n.l = dword_result & 0xFF;
        registers.gpr.n.h = dword_result >> 8;
        break;
    case 0x2A: // LD A, [HL+]
        address = hl;
        hl += 1;
        registers.gpr.n.h = hl >> 8;
        registers.gpr.n.l = hl & 0xFF;
        registers.gpr.n.a = mem.read(address);
        break;
    case 0x2B: // DEC HL
        dword_result = (registers.gpr.n.h << 8) + registers.gpr.n.l;
        dword_result -= 1;
        registers.gpr.n.h = dword_result >> 8;
        registers.gpr.n.l = dword_result & 0xFF;
        break;
    case 0x2C: // INC L
        registers.flags &= 0b10111111;
        if ((registers.gpr.n.l & 0xF) == 0xF) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        registers.gpr.n.l += 1;
        if ((registers.gpr.n.l & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.l &= 0xFF;
        break;
    case 0x2D: // DEC L
        registers.flags |= 0b01000000;
        if ((registers.gpr.n.l & 0xF) == 0x0) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        registers.gpr.n.l -= 1;
        if ((registers.gpr.n.l & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.l &= 0xFF;
        break;
    case 0x2E: // LD L, n8
        registers.gpr.n.l = mem.read(registers.pc++);
        break;
    case 0x2F: // CPL
        registers.gpr.n.a = ~registers.gpr.n.a; 
        registers.flags |= 0b01100000;
        break;
    case 0x30: // JR NC, e8
        offset = std::bit_cast<s8>(mem.read(registers.pc++));
        carry_flag = ((registers.flags >> 4) & 0b1) == 1;
        if (!carry_flag) {
            ticks += 4;
            registers.pc += offset;
        }
        break;
    case 0x31: // LD SP, n16
        imm_byte1 = mem.read(registers.pc++);
        imm_byte2 = mem.read(registers.pc++);
        registers.sp = (imm_byte2 << 8) + imm_byte1;
        break;
    case 0x32: // LD [HL-], A
        address = hl;
        hl -= 1;
        registers.gpr.n.h = hl >> 8;
        registers.gpr.n.l = hl & 0xFF;
        mem.write(address, registers.gpr.n.a);
        break;
    case 0x33: // INC SP
        registers.sp += 1;
        break;
    case 0x34: // INC [HL]
        registers.flags &= 0b10111111;
        operand = mem.read(hl);
        if ((operand & 0xF) == 0xF) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        operand += 1;
        if ((operand & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        operand &= 0xFF;
        mem.write(hl, operand);
        break;
    case 0x35: // DEC [HL]
        registers.flags |= 0b01000000;
        operand = mem.read(hl);
        if ((operand & 0xF) == 0) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        operand -= 1;
        if ((operand & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        operand &= 0xFF;
        mem.write(hl, operand);
        break;
    case 0x36: // LD [HL], n8
        mem.write(hl, mem.read(registers.pc++));
        break;
    case 0x37: // SCF
        registers.flags &= 0b10011111;
        registers.flags |= 0b00010000;
        break;
    case 0x38: // JR C, e8
        offset = std::bit_cast<s8>(mem.read(registers.pc++));
        carry_flag = ((registers.flags >> 4) & 0b1) == 1;
        if (carry_flag) {
            ticks += 4;
            registers.pc += offset;
        }
        break;
    case 0x39: // ADD HL, SP 
        registers.flags &= 0b10111111; // set subtraction flag
        dword_operand = registers.sp;
        dword_result = hl + dword_operand;
        if ((((hl & 0x0FFF) + (dword_operand & 0x0FFF)) > 0xFFF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if ((hl + dword_operand) > 0xFFFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        registers.gpr.n.l = dword_result & 0xFF;
        registers.gpr.n.h = dword_result >> 8;
        break;
    case 0x3A: // LD A, [HL-]
        address = hl;
        hl -= 1;
        registers.gpr.n.h = hl >> 8;
        registers.gpr.n.l = hl & 0xFF;
        registers.gpr.n.a = mem.read(address);
        break;
    case 0x3B: // DEC SP
        registers.sp -= 1;
        break;
    case 0x3C: // INC A
        registers.flags &= 0b10111111;
        if ((registers.gpr.n.a & 0xF) == 0xF) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        registers.gpr.n.a += 1;
        if ((registers.gpr.n.a & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a &= 0xFF;
        break;
    case 0x3D: // DEC A
        registers.flags |= 0b01000000;
        if ((registers.gpr.n.a & 0xF) == 0x0) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        registers.gpr.n.a -= 1;
        if ((registers.gpr.n.a & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a &= 0xFF;
        break;
    case 0x3E: // LD A, n8
        registers.gpr.n.a = mem.read(registers.pc++);
        break;
    case 0x3F: // CCF
        carry_flag = ((registers.flags >> 4) & 0b1) == 1;
        registers.flags &= 0b10011111;
        if (carry_flag) registers.flags &= 0b11101111;
        else registers.flags |= 0b00010000;
        break;
    case 0x40: // LD B, B
        registers.gpr.n.b = registers.gpr.n.b; // idk if this is necessary
        break;
    case 0x41: // LD B, C
        registers.gpr.n.b = registers.gpr.n.c;
        break;
    case 0x42: // LD B, D
        registers.gpr.n.b = registers.gpr.n.d;
        break;
    case 0x43: // LD B, E
        registers.gpr.n.b = registers.gpr.n.e;
        break;
    case 0x44: // LD B, H
        registers.gpr.n.b = registers.gpr.n.h;
        break;
    case 0x45: // LD B, L
        registers.gpr.n.b = registers.gpr.n.l;
        break;
    case 0x46: // LD B, [HL]
        registers.gpr.n.b = mem.read(hl);
        break;
    case 0x47: // LD B, A
        registers.gpr.n.b = registers.gpr.n.a;
        break;
    case 0x48: // LD C, B
        registers.gpr.n.c = registers.gpr.n.b;
        break;
    case 0x49: // LD C, C
        registers.gpr.n.c = registers.gpr.n.c;
        break;
    case 0x4A: // LD C, D
        registers.gpr.n.c = registers.gpr.n.d;
        break;
    case 0x4B: // LD C, E
        registers.gpr.n.c = registers.gpr.n.e;
        break;
    case 0x4C: // LD C, H
        registers.gpr.n.c = registers.gpr.n.h;
        break;
    case 0x4D: // LD C, L
        registers.gpr.n.c = registers.gpr.n.l;
        break;
    case 0x4E: // LD C, [HL]
        registers.gpr.n.c = mem.read(hl);
        break;
    case 0x4F: // LD C, A
        registers.gpr.n.c = registers.gpr.n.a;
        break;
    case 0x50: // LD D, B
        registers.gpr.n.d = registers.gpr.n.b;
        break;
    case 0x51: // LD D, C
        registers.gpr.n.d = registers.gpr.n.c;
        break;
    case 0x52: // LD D, D
        registers.gpr.n.d = registers.gpr.n.d;
        break;
    case 0x53: // LD D, E
        registers.gpr.n.d = registers.gpr.n.e;
        break;
    case 0x54: // LD D, H
        registers.gpr.n.d = registers.gpr.n.h;
        break;
    case 0x55: // LD D, L
        registers.gpr.n.d = registers.gpr.n.l;
        break;
    case 0x56: // LD D, [HL]
        registers.gpr.n.d = mem.read(hl);
        break;
    case 0x57: // LD D, A
        registers.gpr.n.d = registers.gpr.n.a;
        break;
    case 0x58: // LD E, B
        registers.gpr.n.e = registers.gpr.n.b;
        break;
    case 0x59: // LD E, C
        registers.gpr.n.e = registers.gpr.n.c;
        break;
    case 0x5A: // LD E, D
        registers.gpr.n.e = registers.gpr.n.d;
        break;
    case 0x5B: // LD E, E
        registers.gpr.n.e = registers.gpr.n.e;
        break;
    case 0x5C: // LD E, H
        registers.gpr.n.e = registers.gpr.n.h;
        break;
    case 0x5D: // LD E, L
        registers.gpr.n.e = registers.gpr.n.l;
        break;
    case 0x5E: // LD E, [HL]
        registers.gpr.n.e = mem.read(hl);
        break;
    case 0x5F: // LD E, A
        registers.gpr.n.e = registers.gpr.n.a;
        break;
    case 0x60: // LD H, B
        registers.gpr.n.h = registers.gpr.n.b;
        break;
    case 0x61: // LD H, C
        registers.gpr.n.h = registers.gpr.n.c;
        break;
    case 0x62: // LD H, D
        registers.gpr.n.h = registers.gpr.n.d;
        break;
    case 0x63: // LD H, E
        registers.gpr.n.h = registers.gpr.n.e;
        break;
    case 0x64: // LD H, H
        registers.gpr.n.h = registers.gpr.n.h;
        break;
    case 0x65: // LD H, L
        registers.gpr.n.h = registers.gpr.n.l;
        break;
    case 0x66: // LD H, [HL]
        registers.gpr.n.h = mem.read(hl);
        break;
    case 0x67: // LD H, A
        registers.gpr.n.h = registers.gpr.n.a;
        break;
    case 0x68: // LD L, B
        registers.gpr.n.l = registers.gpr.n.b;
        break;
    case 0x69: // LD L, C
        registers.gpr.n.l = registers.gpr.n.c;
        break;
    case 0x6A: // LD L, D
        registers.gpr.n.l = registers.gpr.n.d;
        break;
    case 0x6B: // LD L, E
        registers.gpr.n.l = registers.gpr.n.e;
        break;
    case 0x6C: // LD L, H
        registers.gpr.n.l = registers.gpr.n.h;
        break;
    case 0x6D: // LD L, L
        registers.gpr.n.l = registers.gpr.n.l;
        break;
    case 0x6E: // LD L, [HL]
        registers.gpr.n.l = mem.read(hl);
        break;
    case 0x6F: // LD L, A
        registers.gpr.n.l = registers.gpr.n.a;
        break;
    case 0x70: // LD [HL], B
        mem.write(hl, registers.gpr.n.b);
        break;
    case 0x71: // LD [HL], C
        mem.write(hl, registers.gpr.n.c);
        break;
    case 0x72: // LD [HL], D
        mem.write(hl, registers.gpr.n.d);
        break;
    case 0x73: // LD [HL], E
        mem.write(hl, registers.gpr.n.e);
        break;
    case 0x74: // LD [HL], H
        mem.write(hl, registers.gpr.n.h);
        break;
    case 0x75: // LD [HL], L
        mem.write(hl, registers.gpr.n.l);
        break;
    case 0x76: // HALT
        // idk if this halt implementation is correct at all tbh
        if (!ime && (mem.read(IE) & mem.read(IF)) != 0) { 
            halt_bug = true;
        } else halt_flag = true;           
        break;
    case 0x77: // LD [HL], A
        mem.write(hl, registers.gpr.n.a);
        break;
    case 0x78: // LD A, B
        registers.gpr.n.a = registers.gpr.n.b;
        break;
    case 0x79: // LD A, C
        registers.gpr.n.a = registers.gpr.n.c;
        break;
    case 0x7A: // LD A, D
        registers.gpr.n.a = registers.gpr.n.d;
        break;
    case 0x7B: // LD A, E
        registers.gpr.n.a = registers.gpr.n.e;
        break;
    case 0x7C: // LD A, H
        registers.gpr.n.a = registers.gpr.n.h;
        break;
    case 0x7D: // LD A, L
        registers.gpr.n.a = registers.gpr.n.l;
        break;
    case 0x7E: // LD A, [HL]
        registers.gpr.n.a = mem.read(hl);
        break;
    case 0x7F: // LD A, A
        registers.gpr.n.a = registers.gpr.n.a;
        break;
    case 0x80: // ADD A, B
        registers.flags &= 0b10111111; // subtraction bit
        operand = registers.gpr.n.b;
        dword_result = registers.gpr.n.a + operand;
        if ((((registers.gpr.n.a & 0xF) + (operand & 0xF)) > 0xF)) registers.flags |= 0b00100000; // half carry bit
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000; // carry bit
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000; // zero bit
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x81: // ADD A, C
        registers.flags &= 0b10111111;
        operand = registers.gpr.n.c;
        dword_result = registers.gpr.n.a + operand;
        if ((((registers.gpr.n.a & 0xF) + (operand & 0xF)) > 0xF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x82: // ADD A, D
        registers.flags &= 0b10111111;
        operand = registers.gpr.n.d;
        dword_result = registers.gpr.n.a + operand;
        if ((((registers.gpr.n.a & 0xF) + (operand & 0xF)) > 0xF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x83: // ADD A, E
        registers.flags &= 0b10111111;
        operand = registers.gpr.n.e;
        dword_result = registers.gpr.n.a + operand;
        if ((((registers.gpr.n.a & 0xF) + (operand & 0xF)) > 0xF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x84: // ADD A, H
        registers.flags &= 0b10111111;
        operand = registers.gpr.n.h;
        dword_result = registers.gpr.n.a + operand;
        if ((((registers.gpr.n.a & 0xF) + (operand & 0xF)) > 0xF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x85: // ADD A, L
        registers.flags &= 0b10111111;
        operand = registers.gpr.n.l;
        dword_result = registers.gpr.n.a + operand;
        if ((((registers.gpr.n.a & 0xF) + (operand & 0xF)) > 0xF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x86: // ADD A, [HL]
        registers.flags &= 0b10111111;
        operand = mem.read(hl);
        dword_result = registers.gpr.n.a + operand;
        if ((((registers.gpr.n.a & 0xF) + (operand & 0xF)) > 0xF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x87: // ADD A, A
        registers.flags &= 0b10111111;
        operand = registers.gpr.n.a;
        dword_result = registers.gpr.n.a + operand;
        if ((((registers.gpr.n.a & 0xF) + (operand & 0xF)) > 0xF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x88: // ADC A, B
        registers.flags &= 0b10111111;
        operand = registers.gpr.n.b;
        carry_value = (registers.flags >> 4) & 0b1;
        dword_result = registers.gpr.n.a + operand + carry_value;
        if ((((registers.gpr.n.a & 0xF) + (operand & 0xF) + carry_value) > 0xF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x89: // ADC A, C
        registers.flags &= 0b10111111;
        operand = registers.gpr.n.c;
        carry_value = (registers.flags >> 4) & 0b1;
        dword_result = registers.gpr.n.a + operand + carry_value;
        if ((((registers.gpr.n.a & 0xF) + (operand & 0xF) + carry_value) > 0xF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x8A: // ADC A, D
        registers.flags &= 0b10111111;
        operand = registers.gpr.n.d;
        carry_value = (registers.flags >> 4) & 0b1;
        dword_result = registers.gpr.n.a + operand + carry_value;
        if ((((registers.gpr.n.a & 0xF) + (operand & 0xF) + carry_value) > 0xF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x8B: // ADC A, E
        registers.flags &= 0b10111111;
        operand = registers.gpr.n.e;
        carry_value = (registers.flags >> 4) & 0b1;
        dword_result = registers.gpr.n.a + operand + carry_value;
        if ((((registers.gpr.n.a & 0xF) + (operand & 0xF) + carry_value) > 0xF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x8C: // ADC A, H
        registers.flags &= 0b10111111;
        operand = registers.gpr.n.h;
        carry_value = (registers.flags >> 4) & 0b1;
        dword_result = registers.gpr.n.a + operand + carry_value;
        if ((((registers.gpr.n.a & 0xF) + (operand & 0xF) + carry_value) > 0xF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x8D: // ADC A, L
        registers.flags &= 0b10111111;
        operand = registers.gpr.n.l;
        carry_value = (registers.flags >> 4) & 0b1;
        dword_result = registers.gpr.n.a + operand + carry_value;
        if ((((registers.gpr.n.a & 0xF) + (operand & 0xF) + carry_value) > 0xF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x8E: // ADC A, [HL]
        registers.flags &= 0b10111111;
        operand = mem.read(hl);
        carry_value = (registers.flags >> 4) & 0b1;
        dword_result = registers.gpr.n.a + operand + carry_value;
        if ((((registers.gpr.n.a & 0xF) + (operand & 0xF) + carry_value) > 0xF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x8F: // ADC A, A
        registers.flags &= 0b10111111;
        operand = registers.gpr.n.a;
        carry_value = (registers.flags >> 4) & 0b1;
        dword_result = registers.gpr.n.a + operand + carry_value;
        if ((((registers.gpr.n.a & 0xF) + (operand & 0xF) + carry_value) > 0xF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x90: // SUB A, B
        registers.flags |= 0b01000000; // subtraction bit
        operand = registers.gpr.n.b;
        dword_result = registers.gpr.n.a - operand;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF)))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x91: // SUB A, C
        registers.flags |= 0b01000000;
        operand = registers.gpr.n.c;
        dword_result = registers.gpr.n.a - operand;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF)))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x92: // SUB A, D
        registers.flags |= 0b01000000;
        operand = registers.gpr.n.d;
        dword_result = registers.gpr.n.a - operand;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF)))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x93: // SUB A, E
        registers.flags |= 0b01000000;
        operand = registers.gpr.n.e;
        dword_result = registers.gpr.n.a - operand;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF)))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x94: // SUB A, H
        registers.flags |= 0b01000000;
        operand = registers.gpr.n.h;
        dword_result = registers.gpr.n.a - operand;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF)))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x95: // SUB A, L
        registers.flags |= 0b01000000;
        operand = registers.gpr.n.l;
        dword_result = registers.gpr.n.a - operand;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF)))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x96: // SUB A, [HL]
        registers.flags |= 0b01000000;
        operand = mem.read(hl);
        dword_result = registers.gpr.n.a - operand;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF)))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x97: // SUB A, A
        registers.flags |= 0b01000000;
        operand = registers.gpr.n.a;
        dword_result = registers.gpr.n.a - operand;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF)))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x98: // SBC A, B
        registers.flags |= 0b01000000;
        operand = registers.gpr.n.b;
        carry_value = (registers.flags >> 4) & 0b1;
        dword_result = registers.gpr.n.a - operand - carry_value;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF) + carry_value))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x99: // SBC A, C
        registers.flags |= 0b01000000;
        operand = registers.gpr.n.c;
        carry_value = (registers.flags >> 4) & 0b1;
        dword_result = registers.gpr.n.a - operand - carry_value;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF) + carry_value))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x9A: // SBC A, D
        registers.flags |= 0b01000000;
        operand = registers.gpr.n.d;
        carry_value = (registers.flags >> 4) & 0b1;
        dword_result = registers.gpr.n.a - operand - carry_value;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF) + carry_value))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x9B: // SBC A, E
        registers.flags |= 0b01000000;
        operand = registers.gpr.n.e;
        carry_value = (registers.flags >> 4) & 0b1;
        dword_result = registers.gpr.n.a - operand - carry_value;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF) + carry_value))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x9C: // SBC A, H
        registers.flags |= 0b01000000;
        operand = registers.gpr.n.h;
        carry_value = (registers.flags >> 4) & 0b1;
        dword_result = registers.gpr.n.a - operand - carry_value;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF) + carry_value))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x9D: // SBC A, L
        registers.flags |= 0b01000000;
        operand = registers.gpr.n.l;
        carry_value = (registers.flags >> 4) & 0b1;
        dword_result = registers.gpr.n.a - operand - carry_value;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF) + carry_value))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x9E: // SBC A, [HL]
        registers.flags |= 0b01000000;
        operand = mem.read(hl);
        carry_value = (registers.flags >> 4) & 0b1;
        dword_result = registers.gpr.n.a - operand - carry_value;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF) + carry_value))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0x9F: // SBC A, A
        registers.flags |= 0b01000000;
        operand = registers.gpr.n.a;
        carry_value = (registers.flags >> 4) & 0b1;
        dword_result = registers.gpr.n.a - operand - carry_value;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF) + carry_value))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0xA0: // AND A, B
        registers.flags &= 0b10101111; // set subtraction and carry flag
        registers.flags |= 0b00100000; // set half carry flag
        operand = registers.gpr.n.b;
        result = registers.gpr.n.a & operand;
        if (result == 0) registers.flags |= 0b10000000; // zero flag
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result;
        break;
    case 0xA1: // AND A, C
        registers.flags &= 0b10101111;
        registers.flags |= 0b00100000;
        operand = registers.gpr.n.c;
        result = registers.gpr.n.a & operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result;
        break;
    case 0xA2: // AND A, D
        registers.flags &= 0b10101111;
        registers.flags |= 0b00100000;
        operand = registers.gpr.n.d;
        result = registers.gpr.n.a & operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result;
        break;
    case 0xA3: // AND A, E
        registers.flags &= 0b10101111;
        registers.flags |= 0b00100000;
        operand = registers.gpr.n.e;
        result = registers.gpr.n.a & operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result;
        break;
    case 0xA4: // AND A, H
        registers.flags &= 0b10101111;
        registers.flags |= 0b00100000;
        operand = registers.gpr.n.h;
        result = registers.gpr.n.a & operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result;
        break;
    case 0xA5: // AND A, L
        registers.flags &= 0b10101111;
        registers.flags |= 0b00100000;
        operand = registers.gpr.n.l;
        result = registers.gpr.n.a & operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result;
        break;
    case 0xA6: // AND A, [HL]
        registers.flags &= 0b10101111;
        registers.flags |= 0b00100000;
        operand = mem.read(hl);
        result = registers.gpr.n.a & operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result;
        break;
    case 0xA7: // AND A, A
        registers.flags &= 0b10101111;
        registers.flags |= 0b00100000;
        operand = registers.gpr.n.a;
        result = registers.gpr.n.a & operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result;
        break;
    case 0xA8: // XOR A, B
        registers.flags &= 0b10001111; // set subtraction, hc and carry flag
        operand = registers.gpr.n.b;
        result = registers.gpr.n.a ^ operand;
        if (result == 0) registers.flags |= 0b10000000; // zero flag
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result & 0xFF;
        break;
    case 0xA9: // XOR A, C
        registers.flags &= 0b10001111;
        operand = registers.gpr.n.c;
        result = registers.gpr.n.a ^ operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result & 0xFF;
        break;
    case 0xAA: // XOR A, D
        registers.flags &= 0b10001111;
        operand = registers.gpr.n.d;
        result = registers.gpr.n.a ^ operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result & 0xFF;
        break;
    case 0xAB: // XOR A, E
        registers.flags &= 0b10001111;
        operand = registers.gpr.n.e;
        result = registers.gpr.n.a ^ operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result & 0xFF;
        break;
    case 0xAC: // XOR A, H
        registers.flags &= 0b10001111;
        operand = registers.gpr.n.h;
        result = registers.gpr.n.a ^ operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result & 0xFF;
        break;
    case 0xAD: // XOR A, L
        registers.flags &= 0b10001111;
        operand = registers.gpr.n.l;
        result = registers.gpr.n.a ^ operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result & 0xFF;
        break;
    case 0xAE: // XOR A, [HL]
        registers.flags &= 0b10001111;
        operand = mem.read(hl);
        result = registers.gpr.n.a ^ operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result & 0xFF;
        break;
    case 0xAF: // XOR A, A
        registers.flags &= 0b10001111;
        operand = registers.gpr.n.a;
        result = registers.gpr.n.a ^ operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result & 0xFF;
        break;
    case 0xB0: // OR A, B
        registers.flags &= 0b10001111;
        operand = registers.gpr.n.b;
        result = registers.gpr.n.a | operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result & 0xFF;
        break;
    case 0xB1: // OR A, C
        registers.flags &= 0b10001111;
        operand = registers.gpr.n.c;
        result = registers.gpr.n.a | operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result & 0xFF;
        break;
    case 0xB2: // OR A, D
        registers.flags &= 0b10001111;
        operand = registers.gpr.n.d;
        result = registers.gpr.n.a | operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result & 0xFF;
        break;
    case 0xB3: // OR A, E
        registers.flags &= 0b10001111;
        operand = registers.gpr.n.e;
        result = registers.gpr.n.a | operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result & 0xFF;
        break;
    case 0xB4: // OR A, H
        registers.flags &= 0b10001111;
        operand = registers.gpr.n.h;
        result = registers.gpr.n.a | operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result & 0xFF;
        break;
    case 0xB5: // OR A, L
        registers.flags &= 0b10001111;
        operand = registers.gpr.n.l;
        result = registers.gpr.n.a | operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result & 0xFF;
        break;
    case 0xB6: // OR A, [HL]
        registers.flags &= 0b10001111;
        operand = mem.read(hl);
        result = registers.gpr.n.a | operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result & 0xFF;
        break;
    case 0xB7: // OR A, A
        registers.flags &= 0b10001111;
        operand = registers.gpr.n.a;
        result = registers.gpr.n.a | operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result & 0xFF;
        break;
    case 0xB8: // CP A, B
        registers.flags |= 0b01000000; // subtraction bit
        operand = registers.gpr.n.b;
        result = registers.gpr.n.a - operand;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF)))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (operand > registers.gpr.n.a) registers.flags |= 0b00010000; // carry bit
        else registers.flags &= 0b11101111;
        if ((result & 0xFF) == 0) registers.flags |= 0b10000000; // zero bit
        else registers.flags &= 0b01111111;
        break;
    case 0xB9: // CP A, C
        registers.flags |= 0b01000000;
        operand = registers.gpr.n.c;
        result = registers.gpr.n.a - operand;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF)))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (operand > registers.gpr.n.a) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        break;
    case 0xBA: // CP A, D
        registers.flags |= 0b01000000;
        operand = registers.gpr.n.d;
        result = registers.gpr.n.a - operand;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF)))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (operand > registers.gpr.n.a) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        break;
    case 0xBB: // CP A, E
        registers.flags |= 0b01000000;
        operand = registers.gpr.n.e;
        result = registers.gpr.n.a - operand;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF)))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (operand > registers.gpr.n.a) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        break;
    case 0xBC: // CP A, H
        registers.flags |= 0b01000000;
        operand = registers.gpr.n.h;
        result = registers.gpr.n.a - operand;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF)))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (operand > registers.gpr.n.a) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        break;
    case 0xBD: // CP A, L
        registers.flags |= 0b01000000;
        operand = registers.gpr.n.l;
        result = registers.gpr.n.a - operand;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF)))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (operand > registers.gpr.n.a) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        break;
    case 0xBE: // CP A, [HL]
        registers.flags |= 0b01000000;
        operand = mem.read(hl);
        result = registers.gpr.n.a - operand;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF)))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (operand > registers.gpr.n.a) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        break;
    case 0xBF: // CP A, A
        registers.flags |= 0b01000000;
        operand = registers.gpr.n.a;
        result = registers.gpr.n.a - operand;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF)))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (operand > registers.gpr.n.a) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        break;
    case 0xC0: // RET NZ
        zero_flag = ((registers.flags >> 7) & 0b1) == 1;
        if (!zero_flag) {
            address = mem.read(registers.sp++);
            address = address + (mem.read(registers.sp++) << 8);
            ticks += 12;
            registers.pc = address;
        }
        break;
    case 0xC1: // POP BC
        registers.gpr.n.c = mem.read(registers.sp++);
        registers.gpr.n.b = mem.read(registers.sp++);
        break;
    case 0xC2: // JP NZ, a16
        zero_flag = ((registers.flags >> 7) & 0b1) == 1;
        address = mem.read(registers.pc++);
        address = address + (mem.read(registers.pc++) << 8);
        if (!zero_flag) {
            ticks += 4;
            registers.pc = address;
        }
        break;
    case 0xC3: // JP a16
        address = mem.read(registers.pc++);
        address = address + (mem.read(registers.pc++) << 8);
        registers.pc = address;
        break;
    case 0xC4: // CALL NZ, a16
        zero_flag = ((registers.flags >> 7) & 0b1) == 1;
        address = mem.read(registers.pc++);
        address = address + (mem.read(registers.pc++) << 8);
        if (!zero_flag) {
            registers.sp -= 2;
            mem.dwrite(registers.sp, registers.pc);
            ticks += 12;
            registers.pc = address;
        }
        break;
    case 0xC5: // PUSH BC
        dword_result = (registers.gpr.n.b << 8) + registers.gpr.n.c;
        registers.sp -= 2;
        mem.dwrite(registers.sp, dword_result);
        break;
    case 0xC6: // ADD A, n8
        registers.flags &= 0b10111111;
        operand = mem.read(registers.pc++);
        dword_result = registers.gpr.n.a + operand;
        if ((((registers.gpr.n.a & 0xF) + (operand & 0xF)) > 0xF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0xC7: // RST $00
        registers.sp -= 2;
        mem.dwrite(registers.sp, registers.pc);
        registers.pc = 0x00;
        break;
    case 0xC8: // RET Z
        zero_flag = ((registers.flags >> 7) & 0b1) == 1;
        if (zero_flag) {
            address = mem.read(registers.sp++);
            address = address + (mem.read(registers.sp++) << 8);
            ticks += 12;
            registers.pc = address;
        }
        break;
    case 0xC9: // RET
        address = mem.read(registers.sp++);
        address = address + (mem.read(registers.sp++) << 8);
        registers.pc = address;
        break;
    case 0xCA: // JP Z, a16
        zero_flag = ((registers.flags >> 7) & 0b1) == 1;
        address = mem.read(registers.pc++);
        address = address + (mem.read(registers.pc++) << 8);
        if (zero_flag) {
            ticks += 4;
            registers.pc = address;
        }
        break;
    case 0xCB: // CB PREFIX
        ticks += cb_op();
        break;
    case 0xCC: // CALL Z, a16
        zero_flag = ((registers.flags >> 7) & 0b1) == 1;
        address = mem.read(registers.pc++);
        address = address + (mem.read(registers.pc++) << 8);
        if (zero_flag) {
            registers.sp -= 2;
            mem.dwrite(registers.sp, registers.pc);
            ticks += 12;
            registers.pc = address;
        }
        break;
    case 0xCD: // CALL a16
        address = mem.read(registers.pc++);
        address = address + (mem.read(registers.pc++) << 8);
        registers.sp -= 2;
        mem.dwrite(registers.sp, registers.pc);
        registers.pc = address;
        break;
    case 0xCE: // ADC A, n8
        registers.flags &= 0b10111111;
        operand = mem.read(registers.pc++);
        carry_value = (registers.flags >> 4) & 0b1;
        dword_result = registers.gpr.n.a + operand + carry_value;
        if ((((registers.gpr.n.a & 0xF) + (operand & 0xF) + carry_value) > 0xF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0xCF: // RST $08
        registers.sp -= 2;
        mem.dwrite(registers.sp, registers.pc);
        registers.pc = 0x08;
        break;
    case 0xD0: // RET NC
        carry_flag = ((registers.flags >> 4) & 0b1) == 1;
        if (!carry_flag) {
            address = mem.read(registers.sp++);
            address = address + (mem.read(registers.sp++) << 8);
            ticks += 12;
            registers.pc = address;
        }
        break;
    case 0xD1: // POP DE
        registers.gpr.n.e = mem.read(registers.sp++);
        registers.gpr.n.d = mem.read(registers.sp++);
        break;
    case 0xD2: // JP NC, a16
        carry_flag = ((registers.flags >> 4) & 0b1) == 1;
        address = mem.read(registers.pc++);
        address = address + (mem.read(registers.pc++) << 8);
        if (!carry_flag) {
            ticks += 4;
            registers.pc = address;
        }
        break;
    case 0xD3: // invalid
        break;
    case 0xD4: // CALL NC, a16
        carry_flag = ((registers.flags >> 4) & 0b1) == 1;
        address = mem.read(registers.pc++);
        address = address + (mem.read(registers.pc++) << 8);
        if (!carry_flag) {
            registers.sp -= 2;
            mem.dwrite(registers.sp, registers.pc);
            ticks += 12;
            registers.pc = address;
        }
        break;
    case 0xD5: // PUSH DE
        dword_result = (registers.gpr.n.d << 8) + registers.gpr.n.e;
        registers.sp -= 2;
        mem.dwrite(registers.sp, dword_result);
        break;
    case 0xD6: // SUB A, n8
        registers.flags |= 0b01000000;
        operand = mem.read(registers.pc++);
        dword_result = registers.gpr.n.a - operand;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF)))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0xD7: // RST $10
        registers.sp -= 2;
        mem.dwrite(registers.sp, registers.pc);
        registers.pc = 0x10;
        break;
    case 0xD8: // RET C
        carry_flag = ((registers.flags >> 4) & 0b1) == 1;
        if (carry_flag) {
            address = mem.read(registers.sp++);
            address = address + (mem.read(registers.sp++) << 8);
            ticks += 12;
            registers.pc = address;
        }
        break;
    case 0xD9: // RETI
        address = mem.read(registers.sp++);
        address = address + (mem.read(registers.sp++) << 8);
        ime = true;
        registers.pc = address;
        break;
    case 0xDA: // JP C, a16
        carry_flag = ((registers.flags >> 4) & 0b1) == 1;
        address = mem.read(registers.pc++);
        address = address + (mem.read(registers.pc++) << 8);
        if (carry_flag) {
            ticks += 4;
            registers.pc = address;
        }
        break;
    case 0xDB: // invalid
        break;
    case 0xDC: // CALL C, a16
        carry_flag = ((registers.flags >> 4) & 0b1) == 1;
        address = mem.read(registers.pc++);
        address = address + (mem.read(registers.pc++) << 8);
        if (carry_flag) {
            registers.sp -= 2;
            mem.dwrite(registers.sp, registers.pc);
            ticks += 12;
            registers.pc = address;
        }
        break;
    case 0xDD: // invalid
        break;
    case 0xDE: // SBC A, n8
        registers.flags |= 0b01000000;
        operand = mem.read(registers.pc++);
        carry_value = (registers.flags >> 4) & 0b1;
        dword_result = registers.gpr.n.a - operand - carry_value;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF) + carry_value))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (dword_result > 0xFF) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((dword_result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = dword_result & 0xFF;
        break;
    case 0xDF: // RST $18
        registers.sp -= 2;
        mem.dwrite(registers.sp, registers.pc);
        registers.pc = 0x18;
        break;
    case 0xE0: // LDH [a8], A
        address = 0xFF00 + mem.read(registers.pc++);
        mem.write(address, registers.gpr.n.a);
        break;
    case 0xE1: // POP HL
        registers.gpr.n.l = mem.read(registers.sp++);
        registers.gpr.n.h = mem.read(registers.sp++);
        break;
    case 0xE2: // LD [C], A
        address = 0xFF00 + registers.gpr.n.c;
        mem.write(address, registers.gpr.n.a);
        break;
    case 0xE3: // invalid
        break;
    case 0xE4: // invalid
        break;
    case 0xE5: // PUSH HL
        registers.sp -= 2;
        mem.dwrite(registers.sp, hl);
        break;
    case 0xE6: // AND A, n8
        registers.flags &= 0b10101111;
        registers.flags |= 0b00100000;
        operand = mem.read(registers.pc++);
        result = registers.gpr.n.a & operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result;
        break;
    case 0xE7: // RST $20
        registers.sp -= 2;
        mem.dwrite(registers.sp, registers.pc);
        registers.pc = 0x20;
        break;
    case 0xE8: // ADD SP, e8
        registers.flags &= 0b00111111;
        offset = std::bit_cast<s8>(mem.read(registers.pc++));
        dword_result = registers.sp + offset;
        if ((((registers.sp & 0xF) + (offset & 0xF)) > 0xF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (offset >= 0) {
            if (((registers.sp & 0xFF) + offset) > 0xFF) registers.flags |= 0b00010000;
            else registers.flags &= 0b11101111;
        } else { // negative operand I HATE THIS
            if (((registers.sp & 0xFF) + (u8)offset) > 0xFF) registers.flags |= 0b00010000;
            else registers.flags &= 0b11101111;
        }
        registers.sp = dword_result;
        break;
    case 0xE9: // JP HL
        registers.pc = hl;
        break;
    case 0xEA: // LD [a16], A
        address = mem.read(registers.pc++);
        address = address + (mem.read(registers.pc++) << 8);
        mem.write(address, registers.gpr.n.a);
        break;
    case 0xEB: // invalid
        break;
    case 0xEC: // invalid
        break;
    case 0xED: // invalid
        break;
    case 0xEE: // XOR A, n8
        registers.flags &= 0b10001111;
        operand = mem.read(registers.pc++);
        result = registers.gpr.n.a ^ operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result & 0xFF;
        break;
    case 0xEF: // RST $28
        registers.sp -= 2;
        mem.dwrite(registers.sp, registers.pc);
        registers.pc = 0x28;
        break;
    case 0xF0: // LDH A, [a8]
        address = 0xFF00 + mem.read(registers.pc++);
        registers.gpr.n.a = mem.read(address);
        break;
    case 0xF1: // POP AF
        registers.flags = mem.read(registers.sp++);
        registers.gpr.n.a = mem.read(registers.sp++);
        break;
    case 0xF2: // LD A, [C]
        address = 0xFF00 + registers.gpr.n.c;
        registers.gpr.n.a = mem.read(address);
        break;
    case 0xF3: // DI
        ime = false;
        break;
    case 0xF4: // invalid
        break;
    case 0xF5: // PUSH AF
        dword_result = (registers.gpr.n.a << 8) + registers.flags;
        registers.sp -= 2;
        mem.dwrite(registers.sp, dword_result);
        break;
    case 0xF6: // OR A, n8
        registers.flags &= 0b10001111;
        operand = mem.read(registers.pc++);
        result = registers.gpr.n.a | operand;
        if (result == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        registers.gpr.n.a = result & 0xFF;
        break;
    case 0xF7: // RST $30
        registers.sp -= 2;
        mem.dwrite(registers.sp, registers.pc);
        registers.pc = 0x30;
        break;
    case 0xF8: // LD HL, SP + e8
        registers.flags &= 0b00111111; // set zero and subtraction flags
        offset = std::bit_cast<s8>(mem.read(registers.pc++));
        dword_result = registers.sp + offset;
        if ((((registers.sp & 0xF) + (offset & 0xF)) > 0xF)) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (offset >= 0) {
            if (((registers.sp & 0xFF) + offset) > 0xFF) registers.flags |= 0b00010000;
            else registers.flags &= 0b11101111;
        } else { // negative operand I HATE THIS
            if (((registers.sp & 0xFF) + (u8)offset) > 0xFF) registers.flags |= 0b00010000;
            else registers.flags &= 0b11101111;
        }
        registers.gpr.n.l = dword_result & 0xFF;
        registers.gpr.n.h = dword_result >> 8;
        break;
    case 0xF9: // LD SP, HL
        registers.sp = hl;
        break;
    case 0xFA: // LD A, [a16]
        address = mem.read(registers.pc++);
        address = address + (mem.read(registers.pc++) << 8);
        registers.gpr.n.a = mem.read(address);
        break;
    case 0xFB: // EI
        ei_op = true;
        ei_set = true;
        break;
    case 0xFC: // invalid
        break;
    case 0xFD: // invalid
        break;
    case 0xFE: // CP A, n8
        registers.flags |= 0b01000000;
        operand = mem.read(registers.pc++);
        result = registers.gpr.n.a - operand;
        if ((((registers.gpr.n.a & 0xF) < (operand & 0xF)))) registers.flags |= 0b00100000;
        else registers.flags &= 0b11011111;
        if (operand > registers.gpr.n.a) registers.flags |= 0b00010000;
        else registers.flags &= 0b11101111;
        if ((result & 0xFF) == 0) registers.flags |= 0b10000000;
        else registers.flags &= 0b01111111;
        break;
    case 0xFF: // RST $38
        registers.sp -= 2;
        mem.dwrite(registers.sp, registers.pc);
        registers.pc = 0x38;
        break;
    default:
            __builtin_unreachable();
    }

    registers.flags &= 0xF0;
    if (ei_set && !ei_op) {
        ime = true;
        ei_set = false;
    }
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
