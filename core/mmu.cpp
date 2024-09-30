#include <bit>
#include <bitset>
#include <cmath>
#include <cstdio>
#include <filesystem>
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
    std::ifstream cart_file(filename.data(), std::ios::binary);
    cart_file.seekg(0x100, std::ios_base::beg);
    cart_file.read(std::bit_cast<char*>(&cartridge.header[0]), 0x50); // lol ????
    if (cart_file.fail()) {
        std::cout << "failed to read cartridge header\n";
        return 0;
    }
    cartridge.rom_size = 0x8000 * (1 << cartridge.header[0x48]);
    if (cartridge.header[0x47] == 0x0) {
        mbc = std::make_unique<MBC0>();
    } else if (cartridge.header[0x47] < 0x04) {
        mbc = std::make_unique<MBC1>();
    }
    const static std::array<u8,6> ram_sizes = {0, 0, 8, 32, 128, 64};
    cartridge.ram_size = ram_sizes[cartridge.header[0x49]] * 0x400;
    cartridge.ram.resize(cartridge.ram_size);
    save_file  = std::string_view(filename).substr(0, filename.length() - 2);
    temp_file = save_file + "tmp";
    save_file += "sav";
    std::cout << save_file << "\n";
    std::ifstream ram_file(save_file, std::ios::binary);
    ram_file.seekg(0, std::ios_base::beg);
    ram_file.read(std::bit_cast<char*>(&cartridge.ram[0]), cartridge.ram_size);
    if (!ram_file.fail()) {
        std::cout << "save loaded\n";
    } else {
        std::cout << "save not found\n";
    }
    cartridge.rom.resize(cartridge.rom_size);
    cart_file.seekg(0, std::ios_base::beg);
    cart_file.read(std::bit_cast<char*>(&cartridge.rom[0]), cartridge.rom_size);
    if (cart_file.fail()) {
        std::cout << "failed to read cartridge\n";
        return 0;
    } else {
        std::cout << "cartridge loaded\n";
        std::copy(cartridge.rom.begin(), cartridge.rom.begin() + 0x8000, &mem[0]);
        return cart_file.gcount();
    }
}
u8 MMU::read(u16 address) {
    if (address < 0x8000) {
        return cartridge.rom[mbc->mapper(address) % cartridge.rom_size];
    }
    if (address < 0xC000 && address >= 0xA000) {
        if (!mbc->ram_enable) {
            return 0xFF;
        } else return cartridge.ram[mbc->mapper(address)];
    }
    if (address < 0xFF80 && oam_state) {
        return 0xFF;
    }
    if (address >= 0x8000 && address < 0xA000 && ppuState == mode3) {
        return 0xFF;
    }
    if (address >= 0xFE00 && address < 0xFEA0 && ppuState == mode2) {
        return 0xFF;
    }
    if (address == 0xFF00) {
        u8 inputReg = mem[address];
        if ((inputReg & 0x30) == 0x10) { // buttons
            inputReg &= 0xF0;
            inputReg += joypad->getButton();
        } else { // dpad
            inputReg &= 0xF0;
            inputReg += joypad->getDpad();
            if ((inputReg & 0b11) == 0) {
                inputReg += 0b11;
            }
            if ((inputReg & 0b1100) == 0) {
                inputReg += 0b1100;
            }
        }
        return inputReg | 0xC0;
    }
    if (address == 0xFF01) {
        return 0xFF;
    }
    if (address == 0xFF23) {
        return ppu_read(0xFF23) | 0xBF;
    }
    if (address == 0xFF4D) {
        return 0xFF;
    }
    if (address == 0xFF02) {
    }
    return mem[address];

}
u8 MMU::write(u16 address, u8 word) {
    if (address < 0x8000) { // mbc read
        if (1 == mbc->mbc_write(address, word)) { // ram disabled
            std::ofstream temp_save(temp_file, std::ios::binary | std::ios::trunc);
            temp_save.write(std::bit_cast<char*>(&cartridge.ram[0]), cartridge.ram_size);
            std::filesystem::rename(temp_file, save_file);
        }
        return 0;
    }
    if (address < 0xC000 && address >= 0xA000) {
        if (!mbc->ram_enable) {
            return 0;
        } else { 
            u16 mapped_address = mbc->mapper(address);
            cartridge.ram[mapped_address] = word;
            return 0;
        }
    }
    if (address < 0xFF80 && oam_state) {
        return 0;
    }
    if (address >= 0x8000 && address < 0xA000 && ppuState == mode3) {
        return 0;
    } else if (address >= 0xFE00 && address < 0xFEA0 && (ppuState == mode2 || ppuState == mode3)) { 
        return 0;
    }
    if (address == 0xFF19) {
        if (word > 0x70) {
            channel_trigger = 2;
        }
    }
    if (address == 0xFF14) {
        if (word > 0x70) {
            channel_trigger = 1;
        }
    }
    if (address == 0xFF23) {
        if (word > 0x70) {
            channel_trigger = 4;
        }
    }
    if (address == 0xFF1E) {
        if (word > 0x70) {
            channel_trigger = 3;
        }
    }
    if (address == 0xFF02) {
           std::cout << (char) mem[0xFF01];
    }
    else if (address == 0xFF04) { // div register
                                  // this prevents normal writes
                                  // but the timer uses double writes 
                                  // so still has permission
        write(0xFF03, (u16)0x00);
    }
    else if (address == 0xFF46) {
        if (!oam_state) {
            oam_state = true;
            oam_address = word << 8;
        }

    }
    else if (address == 0x2000) { // mbc 1 register

    }
    else mem[address] = word;
    return 0;
}

u8 MMU::write(u16 address, u16 dword) {
    if (address < 0x8000) { // mbc read
        return 0;
    }
    if (address < 0xC000 && address >= 0xA000) {
        if (!mbc->ram_enable) {
            return 0;
        } else { 
            u16 mapped_address = mbc->mapper(address);
            cartridge.ram[mapped_address] = (u8) dword & 0xFF;
            cartridge.ram[mapped_address + 1] = (u8) (dword >> 8);
            return 0;
        }
    }
    if (address < 0xFF80 && oam_state) {
        return -1;
    }
    if (address >= 0x8000 && address < 0xA000 && ppuState == mode3) {
        return 0;
    }
    if (address >= 0xFE00 && address < 0xFEA0 && ppuState == mode2) {
        return 0;
    }
    if (address == 0x2000) { 
        return 0;
    }
    if (address == 0xFF46) {
        // start oam transfer process

    } else {
        mem[address] = (u8) (dword & 0xFF);
        mem[address + 1] = (u8) (dword >> 8);
    }
    return 0;
}

u8 MMU::ppu_read(u16 address) {
    if (address < 0x8000) {
        return cartridge.rom[mbc->mapper(address) % cartridge.rom_size];
    }
    if (address < 0xC000 && address >= 0xA000) {
        if (!mbc->ram_enable) {
            return 0xFF;
        } else return cartridge.ram[mbc->mapper(address)];
    }
    return mem[address];
}
u8 MMU::ppu_write(u16 address, u8 word) {
    mem[address] = word;
    return 0;
}

u8 MMU::ppu_write(u16 address, u16 dword) {
    mem[address] = (u8) (dword & 0xFF);
    mem[address + 1] = (u8) (dword >> 8);
    return 0;
}

bool MMU::get_oam() {
    return oam_state;
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
