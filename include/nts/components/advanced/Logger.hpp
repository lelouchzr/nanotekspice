#pragma once
#include "nts/Component.hpp"
#include <optional>

namespace nts {
class Logger final : public Component {
  public:
    Logger();

  protected:
    Drive evaluate(std::size_t pin) override;
    void onPrepare(std::size_t tick) override;
    void onSimulate(std::size_t tick) override;

  private:
    Tristate clock_ = Undefined;
    Tristate pendingClock_ = Undefined;
    std::optional<unsigned char> pendingByte_;
};
} // namespace nts
