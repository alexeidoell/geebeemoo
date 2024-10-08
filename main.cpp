#include <gb.h>
#include <iostream>

int main(int argc, char* argv[]) {
//    std::unique_ptr<GB> testGB = std::make_unique<GB>();
    if (argc != 2) {
        std::cout << "intended usage: ./geebeemoo /path/to/game\n";
        return -1;
    }
    GB testGB;
    testGB.runEmu(argv[1]);


}
