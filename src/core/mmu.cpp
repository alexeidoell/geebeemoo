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
    }
    if (address >= OAM && address < UNUSABLE && (ppu_state == mode2 || ppu_state == mode3)) {
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
            if (ppu_state == mode3) {
                return 0xFF;
            } else {
                return mem[address];
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
        case 0xFF4D: // CGB speed switch
            return 0xFF;
        case NR44:
            return mem[NR44] | 0xBF;
        default:
            return mem[address];
    }
}
void MMU::write(u16 address, u8 word) {
    if (address < HRAM && oam_state) {
        return;
    } else if (address >= OAM && address < UNUSABLE && (ppu_state == mode2 || ppu_state == mode3)) { 
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
            if (ppu_state == mode3) {
                return;
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
            return;
        case DIV:
            dwrite(DIV_HIDDEN, 0x00);
            return;
        case DMA_TRIGGER:
            if (!oam_state) {
                oam_state = true;
                oam_address = word << 8;
            }
            return;
        default:
            mem[address] = word;
            return;
    }
}

void MMU::dwrite(u16 address, u16 dword) {
    if (address < HRAM && oam_state) {
        return;
    } else if (address >= OAM && address < UNUSABLE && (ppu_state == mode2 || ppu_state == mode3)) { 
        return;
    }
    if (address >= 0x8000 && address < 0xA000 && ppu_state == mode3) {
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
        mem[0xFE00 + oam_offset] = mem[oam_address + oam_offset];
        oam_offset += 1;
    }
    return 0;
}
