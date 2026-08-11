#include "Support.hpp"
#include "components/advanced/AdvancedFactory.hpp"
#include <array>
#include <fstream>
#include <iterator>
#include <string>

namespace advanced_test {
namespace {
constexpr std::array<std::size_t, 10> ramAddress{8, 7, 6, 5, 4, 3, 2, 1, 23, 22};
constexpr std::array<std::size_t, 11> romAddress{8, 7, 6, 5, 4, 3, 2, 1, 23, 22, 19};
constexpr std::array<std::size_t, 8> dataPins{9, 10, 11, 13, 14, 15, 16, 17};

void ram() {
    Rig rig("4801");
    rig.set(18, nts::False);
    rig.set(20, nts::False);
    rig.set(21, nts::True);
    rig.bus(ramAddress, 0);
    rig.tick();
    for (const auto pin : dataPins)
        require(rig.out(pin) == nts::Undefined, "4801 uninitialized memory must be unknown");
    for (std::size_t location = 0; location < 1024; ++location) {
        rig.bus(ramAddress, location);
        rig.bus(dataPins, (location * 37 + (location >> 8) * 53 + 19) & 255U);
        rig.set(21, nts::False);
        rig.tick();
    }
    for (const auto pin : dataPins)
        rig.release(pin);
    rig.set(21, nts::True);
    for (std::size_t location = 0; location < 1024; ++location) {
        rig.bus(ramAddress, location);
        rig.tick();
        const auto value = (location * 37 + (location >> 8) * 53 + 19) & 255U;
        for (std::size_t bit = 0; bit < dataPins.size(); ++bit)
            require(rig.out(dataPins[bit]) == (((value >> bit) & 1U) != 0 ? nts::True : nts::False),
                    "4801 full address/data roundtrip mismatch");
    }
    rig.bus(ramAddress, 0);
    rig.set(22, nts::Undefined);
    rig.tick();
    require(
        rig.out(9) == nts::True && rig.out(10) == nts::Undefined,
        "4801 unknown address must resolve agreeing bits and preserve disagreeing bits unknown");
    rig.set(22, nts::False);
    rig.set(20, nts::True);
    rig.tick();
    require(rig.chip().drive(9) == nts::Drive::HighImpedance, "4801 OE must disable read driver");
    rig.set(18, nts::True);
    rig.set(21, nts::False);
    rig.bus(ramAddress, 0);
    rig.bus(dataPins, 0);
    rig.tick();
    for (const auto pin : dataPins)
        rig.release(pin);
    rig.set(18, nts::False);
    rig.set(20, nts::False);
    rig.set(21, nts::True);
    rig.tick();
    require(rig.out(9) == nts::True, "4801 chip disable must prevent writes");
    rig.set(21, nts::False);
    rig.bus(dataPins, 0);
    rig.set(8, nts::Undefined);
    rig.tick();
    for (const auto pin : dataPins)
        rig.release(pin);
    rig.set(21, nts::True);
    rig.bus(ramAddress, 0);
    rig.tick();
    require(rig.out(9) == nts::Undefined,
            "4801 unknown write address must conservatively merge old/new cells");
}

void writeRom(const std::string &bytes) {
    std::ofstream file("rom.bin", std::ios::binary | std::ios::trunc);
    file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    require(static_cast<bool>(file), "cannot create ROM test fixture");
}

void rom() {
    Rig missing("2716");
    missing.set(18, nts::False);
    missing.set(20, nts::False);
    missing.bus(romAddress, 0);
    missing.tick();
    require(missing.out(9) == nts::Undefined, "2716 absent rom.bin must leave unknown memory");
    std::string bytes(2048, '\0');
    for (std::size_t location = 0; location < bytes.size(); ++location)
        bytes[location] = static_cast<char>((location * 37 + (location >> 8) * 53 + 17) & 255U);
    writeRom(bytes);
    Rig rig("2716");
    rig.set(18, nts::False);
    rig.set(20, nts::False);
    for (std::size_t location = 0; location < 2048; ++location) {
        rig.bus(romAddress, location);
        rig.tick();
        const auto value = static_cast<unsigned char>(bytes[location]);
        for (std::size_t bit = 0; bit < dataPins.size(); ++bit)
            require(rig.out(dataPins[bit]) == (((static_cast<unsigned int>(value) >> bit) & 1U) != 0
                                                   ? nts::True
                                                   : nts::False),
                    "2716 address/data mismatch including A10 pin19");
    }
    rig.bus(romAddress, 0);
    rig.set(19, nts::Undefined);
    rig.tick();
    require(rig.out(9) == nts::True && rig.out(11) == nts::Undefined,
            "2716 unknown A10 must merge the two actual ROM bytes");
    rig.set(18, nts::True);
    rig.tick();
    require(rig.chip().drive(9) == nts::Drive::HighImpedance, "2716 CE must disable output driver");
    writeRom(std::string(1, static_cast<char>(0xa5)));
    Rig shortRom("2716");
    shortRom.set(18, nts::False);
    shortRom.set(20, nts::False);
    shortRom.bus(romAddress, 1);
    shortRom.tick();
    require(shortRom.out(9) == nts::Undefined,
            "2716 short ROM must not invent trailing zero bytes");
    writeRom(std::string(2049, 'x'));
    bool rejected = false;
    try {
        auto oversized = nts::createAdvancedComponent("2716");
        (void)oversized;
    } catch (const std::exception &) {
        rejected = true;
    }
    require(rejected, "2716 oversized image must be rejected");
    std::filesystem::remove("rom.bin");
    std::filesystem::create_directory("rom.bin");
    rejected = false;
    try {
        auto directory = nts::createAdvancedComponent("2716");
        (void)directory;
    } catch (const std::exception &) {
        rejected = true;
    }
    require(rejected, "2716 non-regular ROM image must be rejected");
    std::filesystem::remove("rom.bin");
}
} // namespace

void memories() {
    ram();
    rom();
}
} // namespace advanced_test
