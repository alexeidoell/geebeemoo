#include <lib/types.h>

class MBC {
protected:
    bool ram_enable;
    u8 rom_bank;
    u8 ram_bank;
    u8 banking_mode;
public:
    virtual u8 mbc_write(u16 address, u8 word) = 0;
    virtual u32 mapper(u16 base_address) = 0;
    virtual ~MBC() = default;
};

class MBC0 : public MBC { // no mbc
    u8 mbc_write(u16 address, u8 word);
    u32 mapper(u16 base_address);
};

class MBC1 : public MBC {
    u8 mbc_write(u16 address, u8 word);
    u32 mapper(u16 base_address);
};
