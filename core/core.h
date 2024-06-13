#pragma once

#include "../lib/types.h"
#include "mmu.h"
#include <memory>

union GPRRegs {
    struct {
        u8 b, c, d, e, h, l, hl_val, a;
    } n;
    u8 r[8];
};

struct gbRegisters {
    GPRRegs gpr = { 0 };
    u8 flags = 0;
    u16 sp;
    u16 pc;
};

class Core {
    public:
    gbRegisters registers; // need to figure out a good way
                           // of letting future video and audio
                           // implementations access memory
                           // maybe a shared pointer ?
    std::unique_ptr<MMU> mem;
    u8 bootup();
    u8 op_tree();
    Core(std::unique_ptr<MMU> memPtr);
    ~Core();
};

