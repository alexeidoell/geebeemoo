#include <gb.h>

int main(int argc, char* argv[]) {
//    std::unique_ptr<GB> testGB = std::make_unique<GB>();
    GB testGB;
    testGB.runEmu(argv[1]);


}
