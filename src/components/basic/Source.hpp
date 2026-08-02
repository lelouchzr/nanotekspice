#pragma once
#include "nts/Component.hpp"

namespace nts {

enum class SourceMode { Input, Clock, Constant, Output };

class Source final : public Component {
  public:
    Source(std::string type, SourceMode mode, Tristate value = Undefined);
    [[nodiscard]] bool isInputSource() const noexcept override;
    void setInput(Tristate value) override;
    [[nodiscard]] Tristate currentInput() const override;

  protected:
    [[nodiscard]] Drive evaluate(std::size_t pin) override;
    void onBeginTick(std::size_t tick) override;

  private:
    SourceMode mode_;
    Tristate value_;
    std::optional<Tristate> pending_;
};

} // namespace nts
