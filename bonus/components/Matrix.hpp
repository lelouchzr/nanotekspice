#pragma once

#include "nts/Component.hpp"
#include <array>
#include <optional>

namespace nts::bonus {

struct MatrixWrite {
    std::size_t address = 0;
    std::uint32_t color = 0;
    bool known = false;
};

class Matrix final : public Component {
  public:
    Matrix();
    [[nodiscard]] std::optional<DisplayFrame> visualization() const override;

  protected:
    [[nodiscard]] Drive evaluate(std::size_t pin) override;
    void onPrepare(std::size_t tick) override;
    void onSimulate(std::size_t tick) override;

  private:
    std::array<std::uint32_t, 4096> pixels_{};
    std::array<bool, 4096> known_{};
    std::optional<MatrixWrite> pending_;
};

} // namespace nts::bonus
