#include "../lib/types.h"

class MBC {
protected:
    bool ram_enable;
    u8 rom_bank;
    u8 ram_bank;
    u8 banking_mode;
public:
    virtual u8 mbc_write(u16 address, u8 word);
    virtual u32 mapper(u16 base_address);
};

class MBC1 : MBC {
    u8 mbc_write(u16 address, u8 word);
    u32 mapper(u16 base_address);
};
