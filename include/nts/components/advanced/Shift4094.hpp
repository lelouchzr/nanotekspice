#pragma once
#include "nts/Component.hpp"
#include <array>

namespace nts {
class Shift4094 final : public Component {
  public:
    Shift4094();

  protected:
    Drive evaluate(std::size_t pin) override;
    void onPrepare(std::size_t tick) override;
    void onSimulate(std::size_t tick) override;

  private:
    std::array<Tristate, 8> shift_;
    std::array<Tristate, 8> latch_;
    std::array<Tristate, 8> pendingShift_;
    std::array<Tristate, 8> pendingLatch_;
    Tristate delayed_ = Undefined;
    Tristate pendingDelayed_ = Undefined;
    Tristate clock_ = Undefined;
    Tristate pendingClock_ = Undefined;
};
} // namespace nts
