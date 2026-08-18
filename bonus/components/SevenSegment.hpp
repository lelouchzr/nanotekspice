#pragma once

#include "nts/Component.hpp"
#include <array>

namespace nts::bonus {

class SevenSegment final : public Component {
  public:
    SevenSegment();
    [[nodiscard]] std::optional<DisplayFrame> visualization() const override;

  protected:
    [[nodiscard]] Drive evaluate(std::size_t pin) override;
    void onPrepare(std::size_t tick) override;
    void onSimulate(std::size_t tick) override;

  private:
    std::array<Tristate, 4> bits_{{Undefined, Undefined, Undefined, Undefined}};
    std::array<Tristate, 4> pending_ = bits_;
};

} // namespace nts::bonus
