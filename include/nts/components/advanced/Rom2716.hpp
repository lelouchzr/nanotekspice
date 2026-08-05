#pragma once
#include "nts/Component.hpp"
#include <array>

namespace nts {
class Rom2716 final : public Component {
  public:
    Rom2716();

  protected:
    Drive evaluate(std::size_t pin) override;

  private:
    std::array<std::array<Tristate, 8>, 2048> memory_;
    void load();
};
} // namespace nts
