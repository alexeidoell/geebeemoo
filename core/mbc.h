#include <vector>
#include <iostream>
#include <optional>
#include <lib/types.h>

class Battery {
private:
    std::string save_file;
    std::string temp_file;
    std::vector<u8>& ram;
public:
    Battery(std::string save_file, std::vector<u8>& ram) : ram(ram) {
        temp_file = save_file + ".tmp";
        std::cout << save_file << "\n";
    }
    void writeSave();
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
