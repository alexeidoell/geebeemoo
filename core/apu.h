#include <mmu.h>
#include <memory>
#include <types.h>
class APU {
private:
    std::shared_ptr<MMU> mem;
public:
    APU(std::shared_ptr<MMU> mem) : mem(mem) {};
    u8 apu_loop(u8 ticks);
};
