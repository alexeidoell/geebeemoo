#pragma once
#include <array>
#include <fstream>
#include <memory>
#include <string_view>
#include <vector>
#include <lib/types.h>
#include <joypad.h>
#include <mbc.h>

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
    std::shared_ptr<Joypad> joypad;
    std::unique_ptr<MBC> mbc;
    std::string save_file;
    std::string temp_file;
public:
    u8 channel_trigger = 0;

    MMU(std::shared_ptr<Joypad> joypad) : joypad(std::move(joypad)) {};

    PPUState ppuState = mode2;
    u32 load_cart(std::string_view filename);
    u8 read(u16 address);
    u8 write(u16 address, u8 word);
    u8 write(u16 address, u16 dword);

    u8 ppu_read(u16 address);
    u8 ppu_write(u16 address, u8 word);
    u8 ppu_write(u16 address, u16 dword);


    u8 div_inc();
    u8 tima_inc();
    bool tima_tick = false;

    bool get_oam();
    u8 oam_transfer(u8 ticks);
};

