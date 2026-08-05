#include "nts/components/advanced/Rom2716.hpp"
#include "nts/Error.hpp"
#include "nts/components/advanced/Logic.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>

namespace nts {
Rom2716::Rom2716() : Component("2716", 24) {
    for (auto &byte : memory_)
        byte.fill(Undefined);
    for (const auto pin : {1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 18U, 19U, 20U, 22U, 23U})
        definePin(pin, PinDirection::Input);
    for (const auto pin : {9U, 10U, 11U, 13U, 14U, 15U, 16U, 17U})
        definePin(pin, PinDirection::Output);
    for (const auto pin : {12U, 21U, 24U})
        definePin(pin, PinDirection::Power);
    load();
}

void Rom2716::load() {
    std::error_code error;
    const bool exists = std::filesystem::exists("rom.bin", error);
    if (error)
        throw ComponentError("cannot inspect rom.bin: " + error.message());
    if (!exists)
        return;
    if (!std::filesystem::is_regular_file("rom.bin", error) || error)
        throw ComponentError("rom.bin must be a readable regular file");
    std::ifstream file("rom.bin", std::ios::binary);
    if (!file)
        throw ComponentError("cannot read rom.bin");
    std::array<char, 2049> bytes{};
    file.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    if (file.bad() || (file.fail() && !file.eof()))
        throw ComponentError("failed reading rom.bin");
    const auto size = static_cast<std::size_t>(file.gcount());
    if (size > memory_.size())
        throw ComponentError("rom.bin exceeds the 2048-byte ROM capacity");
    for (std::size_t index = 0; index < size; ++index) {
        const auto byte = static_cast<unsigned char>(bytes[index]);
        for (std::size_t bit = 0; bit < 8; ++bit)
            memory_[index][bit] =
                ((static_cast<unsigned int>(byte) >> bit) & 1U) != 0 ? True : False;
    }
}

Drive Rom2716::evaluate(std::size_t pin) {
    const auto enabled = logicAnd(logicNot(read(18)), logicNot(read(20)));
    if (enabled == False)
        return Drive::HighImpedance;
    if (enabled == Undefined)
        return Drive::Unknown;
    constexpr std::array<std::size_t, 11> pins{8, 7, 6, 5, 4, 3, 2, 1, 23, 22, 19};
    constexpr std::array<std::size_t, 8> outputs{9, 10, 11, 13, 14, 15, 16, 17};
    std::array<Tristate, 11> address{};
    for (std::size_t bit = 0; bit < pins.size(); ++bit)
        address[bit] = read(pins[bit]);
    const auto bit =
        static_cast<std::size_t>(std::find(outputs.begin(), outputs.end(), pin) - outputs.begin());
    bool first = true;
    Tristate result = Undefined;
    for (std::size_t index = 0; index < memory_.size(); ++index)
        if (advanced::matchesAddress(address, index)) {
            result = first ? memory_[index][bit] : advanced::merge(result, memory_[index][bit]);
            first = false;
        }
    return toDrive(result);
}
} // namespace nts
