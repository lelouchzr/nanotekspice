#include "Viewport.hpp"
#include <algorithm>
#include <cmath>

namespace nts::bonus {

void Viewport::resize(double width, double height) noexcept {
    if (!std::isfinite(width) || !std::isfinite(height) || width <= 0 || height <= 0)
        return;
    const auto previous = world(center());
    const bool initialized = width_ > 0 && height_ > 0;
    width_ = width;
    height_ = height;
    if (initialized) {
        offset_ = {previous.x - width_ / (2 * scale_), previous.y - height_ / (2 * scale_)};
        clampOffset();
    }
}

void Viewport::reset() noexcept {
    offset_ = {};
    scale_ = 1;
}

void Viewport::clampOffset() noexcept {
    offset_.x = std::clamp(offset_.x, -10000000.0, 10000000.0);
    offset_.y = std::clamp(offset_.y, -10000000.0, 10000000.0);
}

void Viewport::pan(double dx, double dy) noexcept {
    if (!std::isfinite(dx) || !std::isfinite(dy))
        return;
    offset_.x -= dx / scale_;
    offset_.y -= dy / scale_;
    clampOffset();
}

void Viewport::zoom(double factor, ViewPoint anchor) noexcept {
    if (!std::isfinite(factor) || factor <= 0 || !std::isfinite(anchor.x) ||
        !std::isfinite(anchor.y))
        return;
    const auto fixed = world(anchor);
    scale_ = std::clamp(scale_ * factor, .1, 4.0);
    offset_ = {fixed.x - anchor.x / scale_, fixed.y - anchor.y / scale_};
    clampOffset();
}

void Viewport::fit(ViewPoint minimum, ViewPoint maximum) noexcept {
    const auto width = maximum.x - minimum.x, height = maximum.y - minimum.y;
    if (!std::isfinite(minimum.x) || !std::isfinite(minimum.y) || !std::isfinite(width) ||
        !std::isfinite(height) || width <= 0 || height <= 0 || width_ <= 0 || height_ <= 0)
        return;
    scale_ = std::clamp(
        std::min(std::max(1.0, width_ - 32) / width, std::max(1.0, height_ - 32) / height), .1,
        4.0);
    offset_ = {minimum.x + width / 2 - width_ / (2 * scale_),
               minimum.y + height / 2 - height_ / (2 * scale_)};
    clampOffset();
}

ViewPoint Viewport::screen(ViewPoint point) const noexcept {
    return {(point.x - offset_.x) * scale_, (point.y - offset_.y) * scale_};
}

ViewPoint Viewport::world(ViewPoint point) const noexcept {
    return {point.x / scale_ + offset_.x, point.y / scale_ + offset_.y};
}

ViewPoint Viewport::center() const noexcept {
    return {width_ / 2, height_ / 2};
}

double Viewport::scale() const noexcept {
    return scale_;
}

} // namespace nts::bonus
