#pragma once
#include "nts/Component.hpp"
#include <array>

namespace nts {
class Ram4801 final : public Component {
  public:
    Ram4801();

  protected:
    Drive evaluate(std::size_t pin) override;
    void onPrepare(std::size_t tick) override;
    void onSimulate(std::size_t tick) override;

  private:
    using Byte = std::array<Tristate, 8>;
    std::array<Byte, 1024> memory_;
    std::array<Tristate, 10> pendingAddress_;
    Byte pendingData_;
    Tristate pendingWrite_ = False;
    std::array<Tristate, 10> address();
};
} // namespace nts
