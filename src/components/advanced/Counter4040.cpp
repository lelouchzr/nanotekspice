#include "nts/components/advanced/Counter4040.hpp"
#include "nts/components/advanced/Logic.hpp"
#include <array>

namespace nts {
namespace {
const std::array<std::bitset<4096>, 12> &bitMasks() {
    static const auto masks = [] {
        std::array<std::bitset<4096>, 12> result;
        for (std::size_t bit = 0; bit < result.size(); ++bit)
            for (std::size_t value = 0; value < 4096; ++value)
                if ((value & (std::size_t{1} << bit)) != 0)
                    result[bit].set(value);
        return result;
    }();
    return masks;
}
} // namespace

Counter4040::Counter4040() : Component("4040", 16) {
    states_.set();
    pendingStates_.set();
    definePin(10, PinDirection::Input);
    definePin(11, PinDirection::Input);
    for (const auto pin : {1U, 2U, 3U, 4U, 5U, 6U, 7U, 9U, 12U, 13U, 14U, 15U})
        definePin(pin, PinDirection::Output);
    definePin(8, PinDirection::Power);
    definePin(16, PinDirection::Power);
}

Drive Counter4040::evaluate(std::size_t pin) {
    const auto reset = read(11);
    if (reset == True)
        return Drive::Low;
    constexpr std::array<std::size_t, 12> outputs{9, 7, 6, 5, 3, 2, 4, 13, 12, 14, 15, 1};
    for (std::size_t bit = 0; bit < outputs.size(); ++bit)
        if (outputs[bit] == pin) {
            const auto &mask = bitMasks()[bit];
            const bool high = (states_ & mask).any();
            const bool low = (states_ & ~mask).any() || reset == Undefined;
            return high && low ? Drive::Unknown : high ? Drive::High : Drive::Low;
        }
    return Drive::HighImpedance;
}

void Counter4040::onPrepare(std::size_t) {
    pendingClock_ = read(10);
    const auto edge = advanced::edge(clock_, pendingClock_, false);
    auto next = states_ << 1;
    next.set(0, states_.test(4095));
    pendingStates_ = edge == True ? next : edge == False ? states_ : states_ | next;
    const auto reset = read(11);
    if (reset == True) {
        pendingStates_.reset();
        pendingStates_.set(0);
    } else if (reset == Undefined)
        pendingStates_.set(0);
}

void Counter4040::onSimulate(std::size_t) {
    states_ = pendingStates_;
    clock_ = pendingClock_;
}
} // namespace nts
