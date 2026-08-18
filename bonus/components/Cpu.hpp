#pragma once

#include "../cpu/Subleq.hpp"
#include "nts/Component.hpp"
#include <array>

namespace nts::bonus {

class Cpu final : public Component {
  public:
    Cpu();
    void loadProgram(const std::filesystem::path &path) override;
    [[nodiscard]] std::optional<DisplayFrame> visualization() const override;

  protected:
    [[nodiscard]] Drive evaluate(std::size_t pin) override;
    void onPrepare(std::size_t tick) override;
    void onSimulate(std::size_t tick) override;

  private:
    Subleq machine_;
    Tristate previousClock_ = Undefined;
    Tristate pendingClock_ = Undefined;
    Tristate reset_ = Undefined;
    Tristate enable_ = Undefined;
    bool outputKnown_ = false;
    std::uint8_t value_ = 0;
    std::uint16_t address_ = 0;
    Tristate previousInputClock_ = Undefined;
    Tristate pendingInputClock_ = Undefined;
    std::optional<std::uint8_t> pendingInput_;
};

} // namespace nts::bonus
