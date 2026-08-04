#include "nts/components/advanced/Decoder4514.hpp"
#include "nts/components/advanced/Logic.hpp"

namespace nts {
Decoder4514::Decoder4514() : Component("4514", 24) {
    latched_.fill(Undefined);
    pending_ = latched_;
    for (const auto pin : {1U, 2U, 3U, 21U, 22U, 23U})
        definePin(pin, PinDirection::Input);
    for (const auto pin :
         {4U, 5U, 6U, 7U, 8U, 9U, 10U, 11U, 13U, 14U, 15U, 16U, 17U, 18U, 19U, 20U})
        definePin(pin, PinDirection::Output);
    definePin(12, PinDirection::Power);
    definePin(24, PinDirection::Power);
}

std::array<Tristate, 4> Decoder4514::visibleAddress() {
    auto result = latched_;
    const auto strobe = read(1);
    constexpr std::array<std::size_t, 4> pins{2, 3, 21, 22};
    if (strobe != False)
        for (std::size_t bit = 0; bit < pins.size(); ++bit) {
            const auto input = read(pins[bit]);
            result[bit] = strobe == True ? input : advanced::merge(latched_[bit], input);
        }
    return result;
}

Drive Decoder4514::evaluate(std::size_t pin) {
    const auto inhibit = read(23);
    if (inhibit == True)
        return Drive::Low;
    constexpr std::array<std::size_t, 16> outputs{11, 9,  10, 8,  7,  6,  5,  4,
                                                  18, 17, 20, 19, 14, 13, 16, 15};
    const auto address = visibleAddress();
    for (std::size_t index = 0; index < outputs.size(); ++index)
        if (pin == outputs[index]) {
            const auto value = advanced::decodeAddress(address, index);
            return toDrive(inhibit == Undefined ? advanced::merge(False, value) : value);
        }
    return Drive::HighImpedance;
}

void Decoder4514::onPrepare(std::size_t) {
    pending_ = visibleAddress();
}

void Decoder4514::onSimulate(std::size_t) {
    latched_ = pending_;
}
} // namespace nts
