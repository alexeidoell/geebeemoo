#include <lib/types.h>

class MBC {
protected:
    u8 rom_bank = 0;
    u8 ram_bank = 0;
    u8 banking_mode = 0;
public:
    bool ram_enable = false;
    virtual u8 mbc_write(u16 address, u8 word) = 0;
    virtual u32 mapper(u16 base_address) = 0;
    virtual ~MBC() = default;
};

class MBC0 : public MBC { // no mbc
    u8 mbc_write(u16 address, u8 word) final;
    u32 mapper(u16 base_address) final;
};

class MBC1 : public MBC {
    u8 mbc_write(u16 address, u8 word) final;
    u32 mapper(u16 base_address) final;
};
