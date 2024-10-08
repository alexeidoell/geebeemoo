#pragma once
#include <array>
#include <memory>
#include <string_view>
#include <vector>
#include <lib/types.h>
#include <joypad.h>
#include <mbc.h>

enum MEM_BLOCKS {
    ROM_BANK_0 = 0x0,
    ROM_BANK_N = 0x4,
    VRAM = 0x8,
    EXTERN_RAM = 0xA,
    WRAM_BANK_0 = 0xC,
    WRAM_BANK_N = 0xD
};

enum UPPER_MEM_BLOCKS {
    ECHO_RAM = 0xE000,
    OAM = 0xFE00,
    UNUSABLE = 0xFEA0,
    IO_REGS = 0xFF00,
    HRAM = 0xFF80
};

enum DMG_HW_REGISTERS {
    JOYP = 0xFF00,
    SB = 0xFF01,
    SC = 0xFF02,
    DIV_HIDDEN = 0xFF03,
    DIV = 0xFF04,
    TIMA = 0xFF05,
    TMA = 0xFF06,
    TAC = 0xFF07,
    IF = 0xFF0F,
    NR10 = 0xFF10,
    NR11 = 0xFF11,
    NR12 = 0xFF12,
    NR13 = 0xFF13,
    NR14 = 0xFF14,
    NR21 = 0xFF16,
    NR22 = 0xFF17,
    NR23 = 0xFF18,
    NR24 = 0xFF19,
    NR30 = 0xFF1A,
    NR31 = 0xFF1B,
    NR32 = 0xFF1C,
    NR33 = 0xFF1D,
    NR34 = 0xFF1E,
    NR41 = 0xFF20,
    NR42 = 0xFF21,
    NR43 = 0xFF22,
    NR44 = 0xFF23,
    NR50 = 0xFF24,
    NR51 = 0xFF25,
    NR52 = 0xFF26,
    WAVE_RAM_START = 0xFF30,
    LCDC = 0xFF40,
    STAT = 0xFF41,
    SCY = 0xFF42,
    SCX = 0xFF43,
    LY = 0xFF44,
    LYC = 0xFF45,
    DMA_TRIGGER = 0xFF46,
    BGP = 0xFF47,
    OBP0 = 0xFF48,
    OBP1 = 0xFF49,
    WY = 0xFF4A,
    WX = 0xFF4B,
    IE = 0xFFFF
};

enum PPUState { mode0 = 0, mode1, mode2, mode3 };

struct Cartridge {
    std::array<u8, 0x50> header = {0};
    u32 rom_size = 0; 
    u32 ram_size = 0;
    bool battery = false;
    std::vector<u8> rom;
    std::vector<u8> ram;
};

class MMU {
private:
    Cartridge cartridge;
    std::array<u8, 0x10000> mem = {0};
    bool oam_state = false;
    u8 oam_offset = 0;
    u16 oam_address = 0;
    Joypad& joypad;
    std::unique_ptr<MBC> mbc;
    std::string save_file;
    std::string temp_file;
public:
    u8 channel_trigger = 0;
    MMU(Joypad& joypad) : joypad(joypad) {};
    PPUState ppu_state = mode2;
    u32 load_cart(std::string_view filename);
    u8 read(u16 address);
    void write(u16 address, u8 word);
    void dwrite(u16 address, u16 dword);
    bool tima_tick = false;
    u8 oam_transfer(u8 ticks);

    // inlines
    u8 hw_read(u16 address) { // honestly the linker is probably gonna call me stupid for this one
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
    void hw_write(u16 address, u8 word) {
        mem[address] = word;
    }
    void hw_dwrite(u16 address, u16 dword) {
        mem[address] = (u8) (dword & 0xFF);
        mem[address + 1] = (u8) (dword >> 8);
    }
    bool get_oam() {
        return oam_state;
    }
};

