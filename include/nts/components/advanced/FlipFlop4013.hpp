#pragma once
#include "nts/Component.hpp"
#include <array>

namespace nts {
class FlipFlop4013 final : public Component {
  public:
    FlipFlop4013();

  protected:
    Drive evaluate(std::size_t pin) override;
    void onPrepare(std::size_t tick) override;
    void onSimulate(std::size_t tick) override;

  private:
    struct Half {
        Tristate q = Undefined;
        Tristate complement = Undefined;
        Tristate clock = Undefined;
    };

    std::array<Half, 2> state_{};
    std::array<Half, 2> pending_{};
    Half next(std::size_t half, bool sampleClock);
};
} // namespace nts
