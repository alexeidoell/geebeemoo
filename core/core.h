#pragma once

#include "../lib/types.h"

union GPRRegs {
    struct {
        u8 b, c, d, e, h, l, hl_val, a;
    } n;
    u8 r[8];
};

struct gbRegisters {
    GPRRegs gpr;
    u8 flags;
    u16 sp;
    u16 pc;
};
