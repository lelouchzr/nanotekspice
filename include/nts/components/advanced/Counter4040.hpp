#pragma once
#include "nts/Component.hpp"
#include <bitset>

namespace nts {
class Counter4040 final : public Component {
  public:
    Counter4040();
    [[nodiscard]] bool hasAsynchronousState() const noexcept override;
    [[nodiscard]] bool settle() override;

  protected:
    Drive evaluate(std::size_t pin) override;
    void onPrepare(std::size_t tick) override;
    void onSimulate(std::size_t tick) override;

  private:
    std::bitset<4096> states_;
    std::bitset<4096> pendingStates_;
    Tristate clock_ = Undefined;
    Tristate pendingClock_ = Undefined;
};
} // namespace nts
