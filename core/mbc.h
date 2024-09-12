#include "../lib/types.h"

class MBC1 {
    bool ram_enable;
    u8 rom_bank;
    u8 ram_bank;
    u8 banking_mode;

    u8 mbc_write(u16 address, u8 word);

    u32 mapper(u16 base_address);
};
