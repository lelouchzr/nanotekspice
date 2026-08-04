#include "nts/components/advanced/Adder4008.hpp"
#include <array>

namespace nts {
Adder4008::Adder4008() : Component("4008", 16) {
    for (const auto pin : {1U, 2U, 3U, 4U, 5U, 6U, 7U, 9U, 15U})
        definePin(pin, PinDirection::Input);
    for (std::size_t pin = 10; pin <= 14; ++pin)
        definePin(pin, PinDirection::Output);
    definePin(8, PinDirection::Power);
    definePin(16, PinDirection::Power);
}

Drive Adder4008::evaluate(std::size_t pin) {
    constexpr std::array<std::size_t, 4> a{7, 5, 3, 1}, b{6, 4, 2, 15};
    Tristate carry = read(9);
    for (std::size_t bit = 0; bit < a.size(); ++bit) {
        const auto left = read(a[bit]), right = read(b[bit]);
        const auto sum = logicXor(logicXor(left, right), carry);
        if (pin == 10 + bit)
            return toDrive(sum);
        carry =
            logicOr(logicOr(logicAnd(left, right), logicAnd(left, carry)), logicAnd(right, carry));
    }
    return toDrive(carry);
}
} // namespace nts
