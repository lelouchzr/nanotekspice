#pragma once

namespace nts::bonus {

struct ViewPoint {
    double x = 0;
    double y = 0;
};

class Viewport {
  public:
    void resize(double width, double height) noexcept;
    void reset() noexcept;
    void pan(double dx, double dy) noexcept;
    void zoom(double factor, ViewPoint anchor) noexcept;
    void fit(ViewPoint minimum, ViewPoint maximum) noexcept;
    [[nodiscard]] ViewPoint screen(ViewPoint world) const noexcept;
    [[nodiscard]] ViewPoint world(ViewPoint screen) const noexcept;
    [[nodiscard]] ViewPoint center() const noexcept;
    [[nodiscard]] double scale() const noexcept;

  private:
    double width_ = 0;
    double height_ = 0;
    double scale_ = 1;
    ViewPoint offset_;
    void clampOffset() noexcept;
};

} // namespace nts::bonus
