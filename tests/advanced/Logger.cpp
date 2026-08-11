#include "Support.hpp"
#include <array>
#include <fstream>
#include <iterator>
#include <string>

namespace advanced_test {
void logger() {
    {
        std::ofstream file("log.bin", std::ios::binary);
        file << "prefix";
    }
    Rig rig("logger");
    constexpr std::array<std::size_t, 8> data{1, 2, 3, 4, 5, 6, 7, 8};
    rig.bus(data, 0);
    rig.set(9, nts::False);
    rig.set(10, nts::False);
    rig.tick();
    rig.set(9, nts::True);
    rig.tick();
    rig.tick();
    rig.set(9, nts::False);
    rig.set(10, nts::True);
    rig.tick();
    rig.set(9, nts::True);
    rig.tick();
    rig.set(9, nts::False);
    rig.set(10, nts::False);
    rig.set(1, nts::Undefined);
    rig.tick();
    rig.set(9, nts::True);
    rig.tick();
    rig.bus(data, 255);
    rig.set(9, nts::Undefined);
    rig.tick();
    rig.set(9, nts::True);
    rig.tick();
    rig.set(9, nts::False);
    rig.set(10, nts::Undefined);
    rig.tick();
    rig.set(9, nts::True);
    rig.tick();
    rig.set(9, nts::False);
    rig.set(10, nts::False);
    rig.tick();
    rig.set(9, nts::True);
    rig.tick();
    std::ifstream file("log.bin", std::ios::binary);
    const std::string bytes{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    require(
        bytes == std::string("prefix\0\xff", 8),
        "logger must append exact bytes only on certain rising edge, known data and inhibit false");
    std::filesystem::remove("log.bin");
    std::filesystem::create_directory("log.bin");
    rig.set(9, nts::False);
    rig.tick();
    rig.set(9, nts::True);
    bool rejected = false;
    try {
        rig.tick();
    } catch (const std::exception &) {
        rejected = true;
    }
    require(rejected, "logger write failure must raise an exception");
    std::filesystem::remove("log.bin");
}
} // namespace advanced_test
