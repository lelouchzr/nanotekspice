#include "nts/components/advanced/Shift4094.hpp"
#include "nts/components/advanced/Logic.hpp"

namespace nts {
Shift4094::Shift4094() : Component("4094", 16) {
    shift_.fill(Undefined);
    latch_.fill(Undefined);
    pendingShift_ = shift_;
    pendingLatch_ = latch_;
    for (const auto pin : {1U, 2U, 3U, 15U})
        definePin(pin, PinDirection::Input);
    for (const auto pin : {4U, 5U, 6U, 7U, 9U, 10U, 11U, 12U, 13U, 14U})
        definePin(pin, PinDirection::Output);
    definePin(8, PinDirection::Power);
    definePin(16, PinDirection::Power);
}

Drive Shift4094::evaluate(std::size_t pin) {
    if (pin == 9)
        return toDrive(shift_.back());
    if (pin == 10)
        return toDrive(delayed_);
    const auto enable = read(15);
    if (enable == False)
        return Drive::HighImpedance;
    if (enable == Undefined)
        return Drive::Unknown;
    constexpr std::array<std::size_t, 8> outputs{4, 5, 6, 7, 14, 13, 12, 11};
    const auto strobe = read(1);
    for (std::size_t bit = 0; bit < outputs.size(); ++bit)
        if (pin == outputs[bit])
            return toDrive(strobe == True    ? shift_[bit]
                           : strobe == False ? latch_[bit]
                                             : advanced::merge(shift_[bit], latch_[bit]));
    return Drive::HighImpedance;
}

void Shift4094::onPrepare(std::size_t) {
    pendingClock_ = read(3);
    const auto rising = advanced::edge(clock_, pendingClock_, true);
    const auto falling = advanced::edge(clock_, pendingClock_, false);
    const auto input = read(2), strobe = read(1);
    for (std::size_t bit = 0; bit < shift_.size(); ++bit) {
        const auto shifted = bit == 0 ? input : shift_[bit - 1];
        pendingShift_[bit] = rising == True    ? shifted
                             : rising == False ? shift_[bit]
                                               : advanced::merge(shift_[bit], shifted);
        pendingLatch_[bit] = strobe == True    ? pendingShift_[bit]
                             : strobe == False ? latch_[bit]
                                               : advanced::merge(latch_[bit], pendingShift_[bit]);
    }
    pendingDelayed_ = falling == True    ? shift_.back()
                      : falling == False ? delayed_
                                         : advanced::merge(delayed_, shift_.back());
}

void Shift4094::onSimulate(std::size_t) {
    shift_ = pendingShift_;
    latch_ = pendingLatch_;
    delayed_ = pendingDelayed_;
    clock_ = pendingClock_;
}
} // namespace nts
