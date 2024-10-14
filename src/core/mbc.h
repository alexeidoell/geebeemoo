#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include <optional>
#include <lib/types.h>

class Battery {
private:
    std::string save_file;
    std::string temp_file;
    std::vector<u8>& ram;
public:
    Battery(std::string save_file, std::vector<u8>& ram) : save_file(save_file), ram(ram) {
        temp_file = save_file + ".tmp";
    }
    void writeSave() const;
};

class MBC {
protected:
    u8 rom_bank = 0;
    u8 ram_bank = 0;
    u8 banking_mode = 0;
    std::vector<u8>& ram;
public:
    std::optional<Battery> battery;
    bool ram_enable = false;
    virtual u8 mbc_write(u16 address, u8 word) = 0;
    virtual u32 mapper(u16 base_address) = 0;
    MBC(std::vector<u8>& ram) : ram(ram) {};
    virtual ~MBC() = default;
};

class MBC0 final : public MBC { // no mbc
public:
    u8 mbc_write(u16 address, u8 word) final;
    u32 mapper(u16 base_address) final;
    MBC0(std::vector<u8>& ram) : MBC(ram) {};
};

class MBC1 final : public MBC {
public:
    u8 mbc_write(u16 address, u8 word) final;
    u32 mapper(u16 base_address) final;
    MBC1(std::vector<u8>& ram) : MBC(ram) {}; // WTF IS THIS??
};

/*
enum RTC_SELECT {
    SECONDS = 0x08,
    MINUTES,
    HOURS,
    DAYS,
    FLAGS
};

class MBC3 final : public MBC {
private:
    struct {
        u8 seconds;
        u8 minutes;
        u8 hours;
        u8 day_counter;
        u8 flags;
    } clock_registers{};
    SDL_Time curr_time = 0;
    bool latch_status = true;
public:
    u8 mbc_write(u16 address, u8 word) final;
    u8 rtc_write(u8 word); // might move ram writes into the mbc
                           // to avoid making this function
    u32 mapper(u16 base_address) final;
    MBC3(std::vector<u8>& ram) : MBC(ram) {
        SDL_GetCurrentTime(&curr_time);
    };
};
*/
