#include <fstream>
#include "json.hpp"
#include "mmu.h"
#include "core.h"
#include <iostream>
#include <filesystem>

using json = nlohmann::json;

int main(void) {
json initial;
json final;

Core* testcore = new Core(std::make_unique<MMU>());

std::string path = "./tests/";
for (const auto& file : std::filesystem::directory_iterator(path)) {
std::ifstream f(file.path());
json test_data = json::parse(f);
std::cout << "running tests for opcode: " << file.path().string().substr(8,2) << "\n";
for (const auto& element : test_data) {
    initial = element.at("initial");
    testcore->registers.pc = initial.at("pc");
    testcore->registers.sp = initial.at("sp");
    testcore->registers.gpr.n.a = initial.at("a");
    testcore->registers.gpr.n.b = initial.at("b");
    testcore->registers.gpr.n.c = initial.at("c");
    testcore->registers.gpr.n.d = initial.at("d");
    testcore->registers.gpr.n.e = initial.at("e");
    testcore->registers.flags = initial.at("f");
    testcore->registers.gpr.n.h = initial.at("h");
    testcore->registers.gpr.n.l = initial.at("l");
    
    for (const auto address_pair : initial.at("ram")) {
        testcore->mem->write(address_pair.at(0), (u8) address_pair.at(1));
    }
    testcore->op_tree();

    final = element.at("final");
    if (testcore->registers.pc != final.at("pc")) std::cout << "mismatch in pc\n";
    if (testcore->registers.sp != final.at("sp")) std::cout << "mismatch in sp\n";
    if (testcore->registers.gpr.n.a != final.at("a")) std::cout << "mismatch in a register\n";
    if (testcore->registers.gpr.n.b != final.at("b")) std::cout << "mismatch in b register\n";
    if (testcore->registers.gpr.n.c != final.at("c")) std::cout << "mismatch in c register\n";
    if (testcore->registers.gpr.n.d != final.at("d")) std::cout << "mismatch in d register\n";
    if (testcore->registers.gpr.n.e != final.at("e")) std::cout << "mismatch in e register\n";
    if (testcore->registers.flags != final.at("f")) std::cout << "mismatch in flags got:" << (int) testcore->registers.flags << " expected: " << final.at("f") << "\n";
    if (testcore->registers.gpr.n.h != final.at("h")) std::cout << "mismatch in h register\n";
    if (testcore->registers.gpr.n.l != final.at("l")) std::cout << "mismatch in l register\n";
}

    std::cout << "tests finished\n";
    }
}
