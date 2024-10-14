#include <bits/fs_fwd.h>
#include <lib/types.h>
#include <mmu.h>
#include <array>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <string_view>
#include <vector>

u32 MMU::load_cart(std::string_view filename) {
    std::string save_file;
    std::ifstream cart_file(filename.data(), std::ios::binary);
    cart_file.seekg(0x100, std::ios_base::beg);
    cart_file.read(std::bit_cast<char*>(&cartridge.header[0]), 0x50); // lol ????
    if (cart_file.fail()) {
        std::cout << "failed to read cartridge header\n";
        return 0;
    }
    cartridge.rom_size = 0x8000 * (1 << cartridge.header[0x48]);
    const static std::array<u8,6> ram_sizes = {0, 0, 8, 32, 128, 64};
    cartridge.ram_size = ram_sizes[cartridge.header[0x49]] * 0x400;
    cartridge.ram.resize(cartridge.ram_size);
    save_file  = std::string_view(filename).substr(0, filename.find_last_of(".") + 1);
    save_file += "sav";
    std::ifstream ram_file(save_file, std::ios::binary);
    ram_file.seekg(0, std::ios_base::beg);
    ram_file.read(std::bit_cast<char*>(&cartridge.ram[0]), cartridge.ram_size);
    if (!ram_file.fail()) {
        std::cout << save_file << "\n";
        std::cout << "save loaded\n";
    } else {
        std::cout << "save not found\n";
    }
    ram_file.close();
    cartridge.rom.resize(cartridge.rom_size);
    cart_file.seekg(0, std::ios_base::beg);
    cart_file.read(std::bit_cast<char*>(&cartridge.rom[0]), cartridge.rom_size);
    if (cartridge.header[0x47] == 0x0) {
        mbc = std::make_unique<MBC0, std::vector<u8>&>(cartridge.ram);
    }
    else if (cartridge.header[0x47] < 0x04) {
        mbc = std::make_unique<MBC1, std::vector<u8>&>(cartridge.ram);
        if (cartridge.header[0x47] == 0x3) {
            mbc->battery.emplace(save_file, cartridge.ram);
        }
    }
    /*
    else if (cartridge.header[0x47] >= 0x0F && cartridge.header[0x47] < 0x14) {
        mbc = std::make_unique<MBC3, std::vector<u8>&>(cartridge.ram);
        if (cartridge.header[0x47] == 0x0F || cartridge.header[0x47] == 0x10 ||
                cartridge.header[0x47] == 0x13) { // gross
            mbc->battery.emplace(save_file, cartridge.ram);
        }

    }
    */
    if (cart_file.fail()) {
        std::cout << "failed to read cartridge\n";
        return 0;
    } else {
        std::cout << "cartridge loaded\n";
        std::copy(cartridge.rom.begin(), cartridge.rom.begin() + 0x8000, &mem[0]);
        return cart_file.gcount();
    }
}
u8 MMU::read(u16 address) { // TODO: clean up all read and write functions
    u8 word = 0;
    if (address < HRAM && oam_state) {
        return 0xFF;
    } else if (address >= OAM && address < UNUSABLE) {
        if (!(ppu.ppu_state == mode2 || ppu.ppu_state == mode3)) {
            return ppu.oam_mem[address - 0xFE00];
        }
        return 0xFF;
    }
    switch (address >> 12) {
        case ROM_BANK_0:
        case ROM_BANK_0 + 1:
        case ROM_BANK_0 + 2:
        case ROM_BANK_0 + 3:
        case ROM_BANK_N:
        case ROM_BANK_N + 1:
        case ROM_BANK_N + 2:
        case ROM_BANK_N + 3:
            return cartridge.rom[mbc->mapper(address) % cartridge.rom_size];
        case EXTERN_RAM:
            if (!mbc->ram_enable) {
                return 0xFF;
            } else return cartridge.ram[mbc->mapper(address)];
        case VRAM:
        case VRAM + 1:
            if (ppu.ppu_state == mode3) {
                return 0xFF;
            } else {
                return ppu.getVram()[address - 0x8000];
            }
        case WRAM_BANK_0:
        case WRAM_BANK_N:
            return mem[address];
        default:
            break;
    }
    switch (address) { // special registers
        case JOYP:
            word = mem[address];
            if ((word & 0x30) == 0x10) { // buttons
                word &= 0xF0;
                word += joypad.getButton();
            } else { // dpad
                word &= 0xF0;
                word += joypad.getDpad();
                if ((word & 0b11) == 0) {
                    word += 0b11;
                }
                if ((word & 0b1100) == 0) {
                    word += 0b1100;
                }
            }
            return word | 0xC0;
        case SB:
        case TIMA:
            return timer.TIMA;
        case TMA:
            return timer.TMA;
        case DIV:
            return timer.DIV >> 8;
        case TAC:
            return timer.TAC;
        case 0xFF4D: // CGB speed switch
            return 0xFF;
        case NR44:
            return mem[NR44] | 0xBF;
        case LCDC:
        case STAT:
        case SCY:
        case SCX:
        case LY:
        case LYC:
        case DMA_TRIGGER:
        case BGP:
        case OBP0:
        case OBP1:
        case WY:
        case WX:
            return *((&ppu.hw_registers.LCDC) + (address & 0b1111));
        default:
            return mem[address];
    }
}
void MMU::write(u16 address, u8 word) {
    if (address < HRAM && oam_state) {
        return;
    } else if (address >= OAM && address < UNUSABLE) {
        if (!(ppu.ppu_state == mode2 || ppu.ppu_state == mode3)) {
            ppu.oam_mem[address - 0xFE00] = word;
        }
        return;
    }
    switch (address >> 12) {
        case ROM_BANK_0:
        case ROM_BANK_0 + 1:
        case ROM_BANK_0 + 2:
        case ROM_BANK_0 + 3:
        case ROM_BANK_N:
        case ROM_BANK_N + 1:
        case ROM_BANK_N + 2:
        case ROM_BANK_N + 3:
            mbc->mbc_write(address, word);
            return;
        case EXTERN_RAM:
            if (!mbc->ram_enable) {
                return;
            } else { 
                u16 mapped_address = mbc->mapper(address);
                cartridge.ram[mapped_address] = word;
                return;
            }
        case VRAM:
        case VRAM + 1:
            if (ppu.ppu_state == mode3) {
                return;
            } else {
                ppu.getVram()[address - 0x8000] = word;
            }
        case WRAM_BANK_0:
        case WRAM_BANK_N:
            mem[address] = word;
            return;
        default:
            break;
    }
    switch (address) { // special cases
        case NR14:
            if (word > 0x7F) {
                channel_trigger = 1;
            }
            mem[address] = word | 0x80;
            return;
        case NR24:
            if (word > 0x7F) {
                channel_trigger = 2;
            }
            mem[address] = word | 0x80;
            return;
        case NR34:
            if (word > 0x7F) {
                channel_trigger = 3;
            }
            mem[address] = word | 0x80;
            return;
        case NR44:
            if (word > 0x7F) {
                channel_trigger = 4;
            }
            mem[address] = word | 0x80;
            return;
        case SC:
            //std::cout << (char) mem[0xFF01];
        case TIMA:
            timer.TIMA = word;
            return;
        case TMA:
            timer.TMA = word;
            return;
        case DIV:
            timer.DIV = 0;
            return;
        case TAC:
            timer.TAC = word;
            return;
        case DMA_TRIGGER:
            if (!oam_state) {
                oam_state = true;
                oam_address = word << 8;
            }
            return;
        case LCDC:
        case STAT:
        case SCY:
        case SCX:
        case LY:
        case LYC:
        case BGP:
        case OBP0:
        case OBP1:
        case WY:
        case WX:
            *((&ppu.hw_registers.LCDC) + (address & 0b1111)) = word;
            return;
        default:
            mem[address] = word;
            return;
    }
}

void MMU::dwrite(u16 address, u16 dword) {
    if (address < HRAM && oam_state) {
        return;
    } else if (address >= OAM && address < UNUSABLE && (ppu.ppu_state == mode2 || ppu.ppu_state == mode3)) { 
        return;
    }
    if (address >= 0x8000 && address < 0xA000 && ppu.ppu_state == mode3) {
        return;
    }
    switch (address >> 12) {
        case ROM_BANK_0:
        case ROM_BANK_0 + 1:
        case ROM_BANK_0 + 2:
        case ROM_BANK_0 + 3:
        case ROM_BANK_N:
        case ROM_BANK_N + 1:
        case ROM_BANK_N + 2:
        case ROM_BANK_N + 3:
            return;
        case VRAM:
        case VRAM + 1:
            ppu.getVram()[address - 0x8000] = (u8) (dword & 0xFF);
            ppu.getVram()[address + 1 - 0x8000] = (u8) (dword >> 8);
            return;
        case EXTERN_RAM:
            if (!mbc->ram_enable) {
                return;
            } else { 
                u16 mapped_address = mbc->mapper(address);
                cartridge.ram[mapped_address] = (u8) dword & 0xFF;
                cartridge.ram[mapped_address + 1] = (u8) (dword >> 8);
                return;
            }
        default:
            mem[address] = (u8) (dword & 0xFF);
            mem[address + 1] = (u8) (dword >> 8);
            return;
    }
}
u8 MMU::oam_transfer(u8 ticks) {
    for (auto i = 0; i < ticks; i += 4) {
        if (oam_offset == 160){
            oam_state = false;
            oam_offset = 0;
            break;
        }
        ppu.oam_mem[oam_offset] = mem[oam_address + oam_offset];
        oam_offset += 1;
    }
    return 0;
}

void MMU::statInterruptHandler() {
    bool prevIRQ = ppu.statIRQ;
    if (ppu.hw_registers.LYC == 0 && ppu.hw_registers.LY == 153 && ppu.currentLineDots > 4 && (ppu.hw_registers.STAT & 0b1000000) > 0) {
            ppu.statIRQ = true;
            ppu.hw_registers.STAT |= 0b100;
    } else if ((ppu.hw_registers.LY == ppu.hw_registers.LYC) && (ppu.hw_registers.STAT & 0b1000000) > 0) {
            ppu.statIRQ = true;
            ppu.hw_registers.STAT |= 0b100;
    } else if (ppu_state == mode2 && (ppu.hw_registers.STAT & 0b100000) > 0) {
            ppu.statIRQ = true;
            ppu.hw_registers.STAT = (ppu.hw_registers.STAT & 0b11111100) | mode2;
    } else if (ppu_state == mode1 && (ppu.hw_registers.STAT & 0b10000) > 0) {
            ppu.statIRQ = true;
            ppu.hw_registers.STAT = (ppu.hw_registers.STAT & 0b11111100) | mode1;
    } else if (ppu_state == mode0 && (ppu.hw_registers.STAT & 0b1000) > 0) {
            ppu.statIRQ = true;
            ppu.hw_registers.STAT = (ppu.hw_registers.STAT & 0b11111100) | mode0;
    } else {
        ppu.statIRQ = false;;
        if (ppu.hw_registers.LY != ppu.hw_registers.LYC) {
            ppu.hw_registers.STAT &= 0b11111011;
        }
    }
    if (!prevIRQ && ppu.statIRQ) {
        hw_write(IF, (u8)(hw_read(IF) | 0b10));
    }
}

