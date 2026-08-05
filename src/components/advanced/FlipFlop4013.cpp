#include "nts/components/advanced/FlipFlop4013.hpp"
#include "nts/components/advanced/Logic.hpp"

namespace nts {
FlipFlop4013::FlipFlop4013() : Component("4013", 14) {
    for (const auto pin : {3U, 4U, 5U, 6U, 8U, 9U, 10U, 11U})
        definePin(pin, PinDirection::Input);
    for (const auto pin : {1U, 2U, 12U, 13U})
        definePin(pin, PinDirection::Output);
    definePin(7, PinDirection::Power);
    definePin(14, PinDirection::Power);
}

FlipFlop4013::Half FlipFlop4013::next(std::size_t half, bool sampleClock) {
    constexpr std::array<std::size_t, 2> clocks{3, 11}, resets{4, 10}, sets{6, 8}, data{5, 9};
    const auto reset = read(resets[half]), set = read(sets[half]);
    const auto clock = sampleClock ? read(clocks[half]) : state_[half].clock;
    const auto capture = sampleClock ? advanced::edge(state_[half].clock, clock, true) : False;
    const auto input = sampleClock ? read(data[half]) : Undefined;
    Half result;
    result.clock = clock;
    bool first = true;
    for (const bool r : {false, true})
        for (const bool s : {false, true})
            for (const bool edge : {false, true}) {
                if (!advanced::permits(reset, r) || !advanced::permits(set, s) ||
                    !advanced::permits(capture, edge))
                    continue;
                Tristate q = state_[half].q, complement = state_[half].complement;
                if (r || s) {
                    q = s ? True : False;
                    complement = r ? True : False;
                } else if (edge) {
                    q = input;
                    complement = logicNot(input);
                }
                result.q = first ? q : advanced::merge(result.q, q);
                result.complement =
                    first ? complement : advanced::merge(result.complement, complement);
                first = false;
            }
    return result;
}

Drive FlipFlop4013::evaluate(std::size_t pin) {
    const std::size_t half = pin >= 12 ? 1 : 0;
    const auto visible = next(half, false);
    return toDrive(pin == 1 || pin == 13 ? visible.q : visible.complement);
}

void FlipFlop4013::onPrepare(std::size_t) {
    for (std::size_t half = 0; half < state_.size(); ++half)
        pending_[half] = next(half, true);
}

void FlipFlop4013::onSimulate(std::size_t) {
    state_ = pending_;
}
} // namespace nts
