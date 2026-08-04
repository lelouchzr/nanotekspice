#pragma once
#include "nts/Component.hpp"
#include <array>

namespace nts {
class Decoder4514 final : public Component {
  public:
    Decoder4514();

  protected:
    Drive evaluate(std::size_t pin) override;
    void onPrepare(std::size_t tick) override;
    void onSimulate(std::size_t tick) override;

  private:
    std::array<Tristate, 4> latched_;
    std::array<Tristate, 4> pending_;
    std::array<Tristate, 4> visibleAddress();
};
} // namespace nts
