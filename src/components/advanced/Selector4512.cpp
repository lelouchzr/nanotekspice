#include "nts/components/advanced/Selector4512.hpp"
#include "nts/components/advanced/Logic.hpp"
#include <array>

namespace nts {
Selector4512::Selector4512() : Component("4512", 16) {
    for (const auto pin : {1U, 2U, 3U, 4U, 5U, 6U, 7U, 9U, 10U, 11U, 12U, 13U, 15U})
        definePin(pin, PinDirection::Input);
    definePin(14, PinDirection::Output);
    definePin(8, PinDirection::Power);
    definePin(16, PinDirection::Power);
}

Drive Selector4512::evaluate(std::size_t) {
    const auto disabled = read(15);
    if (disabled == True)
        return Drive::HighImpedance;
    if (disabled == Undefined)
        return Drive::Unknown;
    const auto inhibit = read(10);
    if (inhibit == True)
        return Drive::Low;
    const std::array<Tristate, 3> address{read(11), read(12), read(13)};
    constexpr std::array<std::size_t, 8> data{1, 2, 3, 4, 5, 6, 7, 9};
    Tristate value = Undefined;
    bool first = true;
    for (std::size_t index = 0; index < data.size(); ++index)
        if (advanced::matchesAddress(address, index)) {
            const auto candidate = read(data[index]);
            value = first ? candidate : advanced::merge(value, candidate);
            first = false;
        }
    return toDrive(inhibit == Undefined ? advanced::merge(False, value) : value);
}
} // namespace nts
