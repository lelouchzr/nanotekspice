#pragma once
#include "nts/Component.hpp"

namespace nts {
class Adder4008 final : public Component {
  public:
    Adder4008();

  protected:
    Drive evaluate(std::size_t pin) override;
};
} // namespace nts
