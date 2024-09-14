#pragma once

#include <../lib/types.h>
#include <mmu.h>
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
public: // need to change a lot of these to private when I start using rom tests
    bool ei_set = false;
    bool ime = false;
    bool halt_flag = false;
    bool halt_bug = false;
    gbRegisters registers; 
    std::shared_ptr<MMU> mem;
    u8 bootup();
    u8 op_tree();
    u8 cb_op();
    Core(std::shared_ptr<MMU> memPtr);
};

