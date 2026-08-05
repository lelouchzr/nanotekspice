#pragma once
#include "nts/Component.hpp"
#include <cstdint>

namespace nts {
class Counter4017 final : public Component {
  public:
    Counter4017();

  protected:
    Drive evaluate(std::size_t pin) override;
    void onPrepare(std::size_t tick) override;
    void onSimulate(std::size_t tick) override;

  private:
    std::uint16_t states_ = 0x3ff;
    std::uint16_t pendingStates_ = 0x3ff;
    Tristate clock_ = Undefined;
    Tristate pendingClock_ = Undefined;
};
} // namespace nts
