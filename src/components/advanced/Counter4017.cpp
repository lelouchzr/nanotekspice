#include "nts/components/advanced/Counter4017.hpp"
#include "nts/components/advanced/Logic.hpp"
#include <array>

namespace nts {
Counter4017::Counter4017() : Component("4017", 16) {
    for (const auto pin : {13U, 14U, 15U})
        definePin(pin, PinDirection::Input);
    for (const auto pin : {1U, 2U, 3U, 4U, 5U, 6U, 7U, 9U, 10U, 11U, 12U})
        definePin(pin, PinDirection::Output);
    definePin(8, PinDirection::Power);
    definePin(16, PinDirection::Power);
}

Drive Counter4017::evaluate(std::size_t pin) {
    constexpr std::array<std::size_t, 10> outputs{3, 2, 4, 7, 10, 1, 5, 6, 9, 11};
    const auto reset = read(15);
    const auto states = reset == True
                            ? std::uint16_t{1}
                            : static_cast<std::uint16_t>(states_ | (reset == Undefined ? 1 : 0));
    bool first = true;
    Tristate result = Undefined;
    for (std::size_t state = 0; state < outputs.size(); ++state)
        if ((states & (1U << state)) != 0) {
            const auto value = (pin == 12 ? state < 5 : outputs[state] == pin) ? True : False;
            result = first ? value : advanced::merge(result, value);
            first = false;
        }
    return toDrive(result);
}

void Counter4017::onPrepare(std::size_t) {
    // CP0 rising while CP1 is low, or CP1 falling while CP0 is high.
    pendingClock_ = logicAnd(read(14), logicNot(read(13)));
    const auto edge = advanced::edge(clock_, pendingClock_, true);
    const auto advancedStates = static_cast<std::uint16_t>(
        ((static_cast<unsigned int>(states_) << 1U) & 0x3ffU) | (states_ >> 9U));
    pendingStates_ = edge == True    ? advancedStates
                     : edge == False ? states_
                                     : static_cast<std::uint16_t>(states_ | advancedStates);
    const auto reset = read(15);
    if (reset == True)
        pendingStates_ = 1;
    else if (reset == Undefined)
        pendingStates_ = static_cast<std::uint16_t>(pendingStates_ | 1U);
}

void Counter4017::onSimulate(std::size_t) {
    states_ = pendingStates_;
    clock_ = pendingClock_;
}
} // namespace nts
