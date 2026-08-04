#pragma once
#include "nts/Component.hpp"

namespace nts {
class Selector4512 final : public Component {
  public:
    Selector4512();

  protected:
    Drive evaluate(std::size_t pin) override;
};
} // namespace nts
