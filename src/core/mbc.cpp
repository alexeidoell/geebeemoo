#include <SDL3/SDL_time.h>
#include <iostream>
#include <fstream>
#include <optional>
#include <bit>
#include <SDL3/SDL.h>
#include <lib/types.h>
#include <mbc.h>

u8 MBC1::mbc_write(u16 address, u8 word) {
    if (address < 0x2000) {
        if ((word & 0b1111) == 0xA) ram_enable = true;
        else {
            ram_enable = false;
            if (battery.has_value()) {
                battery.value().writeSave();
            }
            return 1;
        }
    } else if (address < 0x4000) {
        rom_bank = word & 0b11111;       
        if (rom_bank == 0x0) rom_bank = 0x1;
    } else if (address < 0x6000) {
        ram_bank = word & 0b11;
    } else if (address < 0x8000) {
        banking_mode = word & 0b1;
    }
    return 0;
}

u32 MBC1::mapper(u16 base_address)  {
    u32 mapped_address = 0;
    if (base_address < 0x4000) {
        base_address = base_address & ~(((u16)0b11) << 14);
        if (banking_mode == 1) {
            mapped_address += ((u32)ram_bank << 19);
        }
        mapped_address += base_address;
    } else if (base_address < 0x8000) {
        base_address = base_address & ~(((u32)0b11) << 14); 
        mapped_address += ((u32)ram_bank << 19);
        mapped_address += base_address;
        mapped_address += ((u32)rom_bank << 14);
    } else if (base_address < 0xC000) {
        if (banking_mode == 1) {
            mapped_address += ((u32)ram_bank << 13);
        }
        base_address = base_address & ~(((u32)0b111) << 13); 
        mapped_address += base_address;
    }
    return mapped_address;
}

u8 MBC0::mbc_write(u16 address, u8 word) {
    return 0;
}

u32 MBC0::mapper(u16 base_address) {
    return base_address;
}

/*
u8 MBC3::mbc_write(u16 address, u8 word) {
    switch (address >> 12) {
    case 0x0: // ram and timer enable
    case 0x1:
        if ((word & 0b1111) == 0xA) ram_enable = true;
        else {
            ram_enable = false;
            if (battery.has_value()) {
                battery.value().writeSave();
            }
            return 1;
        }
        break;
    case 0x2: // rom bank number
    case 0x3:
        rom_bank = word & 0b1111111;       
        if (rom_bank == 0x0) rom_bank = 0x1;
        break;
    case 0x4: // ram bank/rtc register select
    case 0x5:
        ram_bank = word & 0b1111;
        break;
    case 0x6: // latch clock data
    case 0x7:
        if (latch_status && word == 0x0) {
            latch_status = false;
        } else if (!latch_status && word == 0x1) { // put date into rtc registers
            // this is actually insane lmao
            // need to implement halting the clock too
            u64 old_time = curr_time / 1000;           
            SDL_GetCurrentTime(&curr_time);
            u64 new_time = curr_time / 1000;
            u64 time_diff = new_time - old_time;
            u64 days = clock_registers.flags & 0b1;
            days <<= 8;
            days += clock_registers.day_counter;
            while (time_diff >= 86400) {
                days += 1;
                time_diff -= 86400;
            }
            u64 hours = clock_registers.hours;
            while (time_diff >= 3600) {
                hours += 1;
                time_diff -= 3600;
            }
            u64 minutes = clock_registers.minutes;
            while (time_diff >= 60) {
                minutes += 1;
                time_diff -= 60;
            }
            u64 seconds = clock_registers.seconds;
            while (time_diff >= 60) {
                seconds += 1;
                time_diff -= 60;
            }
            while (seconds >= 60) {
                minutes += 1;
                seconds -= 60;
            }
            clock_registers.seconds = seconds;
            while (minutes >= 60) {
                hours += 1;
                minutes -= 60;
            }
            clock_registers.minutes = minutes;
            while (hours >= 24) {
                days += 1;
                hours -= 24;
            }
            clock_registers.hours = hours;
            if (days > 0x1FF) {
                clock_registers.flags |= 0b10000000;
            }
            days &= 0x1FF;
            clock_registers.day_counter = days & 0xFF;
            days >>= 8;
            clock_registers.flags = (clock_registers.flags & 0b11111110) | (days & 0b1);
        }
        break;
    default:
        break;
    }

    return 0;
}
*/

u32 MBC3::mapper(u16 address) { // stub

    return 0;
}

void Battery::writeSave() const {
    std::ofstream temp_save(temp_file, std::ios::binary | std::ios::trunc);
    temp_save.write(std::bit_cast<char*>(&ram[0]), ram.capacity());
    temp_save.close();
    SDL_RenamePath(temp_file.c_str(), save_file.c_str());
}
