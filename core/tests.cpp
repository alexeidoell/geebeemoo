#include <fstream>
#include "json.hpp"
#include "mmu.h"
#include "core.h"
#include <iostream>
#include <filesystem>

using json = nlohmann::json;

int main(int argc, char** argv) {
    json initial;
    json final;

    Core* testcore = new Core(std::make_unique<MMU>());

    std::string path = "./tests/";
    if (argc == 1) {
        for (const auto& file : std::filesystem::directory_iterator(path)) {
            std::ifstream f(file.path());
            json test_data = json::parse(f);
            for (const auto& element : test_data) {
                std::string name = element.at("name");
                initial = element.at("initial");
                testcore->registers.pc = initial.at("pc");
                testcore->registers.sp = initial.at("sp");
                testcore->registers.gpr.n.a = initial.at("a");
                testcore->registers.gpr.n.b = initial.at("b");
                testcore->registers.gpr.n.c = initial.at("c");
                testcore->registers.gpr.n.d = initial.at("d");
                testcore->registers.gpr.n.e = initial.at("e");
                testcore->registers.flags = initial.at("f");
                testcore->registers.flags &= 0xF0;
                testcore->registers.gpr.n.h = initial.at("h");
                testcore->registers.gpr.n.l = initial.at("l");
                testcore->ei_set = false;
                testcore->ime = (initial.at("ime") == 1);

                for (const auto address_pair : initial.at("ram")) {
                    testcore->mem->write(address_pair.at(0), (u8) address_pair.at(1));
                }
                testcore->op_tree();

                final = element.at("final");
                if (testcore->registers.pc != final.at("pc")) std::cout << "test: " << name << "mismatch in pc got:" << (int) testcore->registers.pc << " expected: " << final.at("pc") << "\n";
                if (testcore->registers.sp != final.at("sp")) std::cout << "test: " << name << "mismatch in sp got:" << (int) testcore->registers.sp << " expected: " << final.at("sp") << "\n";
                if (testcore->registers.gpr.n.a != final.at("a")) std::cout << "test: " << name << "mismatch in register a got:" << (int) testcore->registers.gpr.n.a << " expected: " << final.at("a") << "\n";
                if (testcore->registers.gpr.n.b != final.at("b")) std::cout << "test: " << name << "mismatch in register b got:" << (int) testcore->registers.gpr.n.b << " expected: " << final.at("b") << "\n";
                if (testcore->registers.gpr.n.c != final.at("c")) std::cout << "test: " << name << "mismatch in register c got:" << (int) testcore->registers.gpr.n.c << " expected: " << final.at("c") << "\n";
                if (testcore->registers.gpr.n.d != final.at("d")) std::cout << "test: " << name << "mismatch in register d got:" << (int) testcore->registers.gpr.n.d << " expected: " << final.at("d") << "\n";
                if (testcore->registers.gpr.n.e != final.at("e")) std::cout << "test: " << name << "mismatch in register e got:" << (int) testcore->registers.gpr.n.e << " expected: " << final.at("e") << "\n";
                if ((0xF0 & testcore->registers.flags) != final.at("f")) std::cout << "test: " << name << "mismatch in flags got:" << (int) testcore->registers.flags << " expected: " << final.at("f") << "\n";
                if (testcore->registers.gpr.n.h != final.at("h")) std::cout << "test: " << name << "mismatch in register h got:" << (int) testcore->registers.gpr.n.h << " expected: " << final.at("h") << "\n";
                if (testcore->registers.gpr.n.l != final.at("l")) std::cout << "test: " << name << "mismatch in register l got:" << (int) testcore->registers.gpr.n.l << " expected: " << final.at("l") << "\n";
                try {
                    if (testcore->ei_set != (final.at("ei") == 1)) std::cout << "test: " << name << "mismatch in ei_set got:" << std::boolalpha << testcore->ei_set << " expected: " << (final.at("ei") == 1) << "\n";
                }
                catch (json::exception e) {

                }
                if (testcore->ime != (final.at("ime") == 1)) std::cout << "test: " << name <<  " mismatch in ime got:" << std::boolalpha << testcore->ime << " expected: " << (final.at("ime") == 1) << "\n";
                for (const auto address_pair : final.at("ram")) {
                    if (testcore->mem->read(address_pair.at(0)) != address_pair.at(1)) std::cout << "test: " << name << "mismatch in memory address: " << address_pair.at(0) << " got: " << (int) testcore->mem->read(address_pair.at(0)) << " expected: " << address_pair.at(1) << "\n";
                }

            }
        }
    } else {
            std::ifstream f(argv[1]);
            json test_data = json::parse(f);
            for (const auto& element : test_data) {
                std::string name = element.at("name");
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
                testcore->ei_set = false;
                testcore->ime = (initial.at("ime") == 1);

                for (const auto address_pair : initial.at("ram")) {
                    testcore->mem->write(address_pair.at(0), (u8) address_pair.at(1));
                }
                testcore->op_tree();

                final = element.at("final");
                if (testcore->registers.pc != final.at("pc")) std::cout << "test: " << name << "mismatch in pc got:" << (int) testcore->registers.pc << " expected: " << final.at("pc") << "\n";
                if (testcore->registers.sp != final.at("sp")) std::cout << "test: " << name << "mismatch in sp got:" << (int) testcore->registers.sp << " expected: " << final.at("sp") << "\n";
                if (testcore->registers.gpr.n.a != final.at("a")) std::cout << "test: " << name << "mismatch in register a got:" << (int) testcore->registers.gpr.n.a << " expected: " << final.at("a") << "\n";
                if (testcore->registers.gpr.n.b != final.at("b")) std::cout << "test: " << name << "mismatch in register b got:" << (int) testcore->registers.gpr.n.b << " expected: " << final.at("b") << "\n";
                if (testcore->registers.gpr.n.c != final.at("c")) std::cout << "test: " << name << "mismatch in register c got:" << (int) testcore->registers.gpr.n.c << " expected: " << final.at("c") << "\n";
                if (testcore->registers.gpr.n.d != final.at("d")) std::cout << "test: " << name << "mismatch in register d got:" << (int) testcore->registers.gpr.n.d << " expected: " << final.at("d") << "\n";
                if (testcore->registers.gpr.n.e != final.at("e")) std::cout << "test: " << name << "mismatch in register e got:" << (int) testcore->registers.gpr.n.e << " expected: " << final.at("e") << "\n";
                if ((0xF0 & testcore->registers.flags) != final.at("f")) std::cout << "test: " << name << "mismatch in flags got:" << (int) testcore->registers.flags << " expected: " << final.at("f") << "\n";
                if (testcore->registers.gpr.n.h != final.at("h")) std::cout << "test: " << name << "mismatch in register h got:" << (int) testcore->registers.gpr.n.h << " expected: " << final.at("h") << "\n";
                if (testcore->registers.gpr.n.l != final.at("l")) std::cout << "test: " << name << "mismatch in register l got:" << (int) testcore->registers.gpr.n.l << " expected: " << final.at("l") << "\n";
                try {
                    if (testcore->ei_set != (final.at("ei") == 1)) std::cout << "test: " << name << "mismatch in ei_set got:" << std::boolalpha << testcore->ei_set << " expected: " << (final.at("ei") == 1) << "\n";
                }
                catch (json::exception e) {

                }
                if (testcore->ime != (final.at("ime") == 1)) std::cout << "test: " << name <<  " mismatch in ime got:" << std::boolalpha << testcore->ime << " expected: " << (final.at("ime") == 1) << "\n";
                for (const auto address_pair : final.at("ram")) {
                    if (testcore->mem->read(address_pair.at(0)) != address_pair.at(1)) std::cout << "test: " << name << "mismatch in memory address: " << address_pair.at(0) << " got: " << (int) testcore->mem->read(address_pair.at(0)) << " expected: " << address_pair.at(1) << "\n";
                }

            }
    }
    std::cout << "tests finished\n";
}
