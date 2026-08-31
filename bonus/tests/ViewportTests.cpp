#include "Support.hpp"
#include "graphics/Viewport.hpp"
#include <cmath>
#include <limits>

namespace nts::bonus::test {

void viewportTests() {
    Viewport view;
    view.resize(956, 736);
    const ViewPoint anchor{713.25, 412.5};
    const auto fixed = view.world(anchor);
    view.zoom(2, anchor);
    auto projected = view.screen(fixed);
    require(std::abs(projected.x - anchor.x) < 1e-9 && std::abs(projected.y - anchor.y) < 1e-9,
            "zoom must preserve the world point under its anchor");
    for (int i = 0; i < 100; ++i) {
        view.zoom(1.15, anchor);
        view.zoom(1 / 1.15, anchor);
    }
    projected = view.screen(fixed);
    require(std::abs(projected.x - anchor.x) < 1e-8 && std::abs(projected.y - anchor.y) < 1e-8,
            "repeated fractional zoom introduced anchor drift");
    view.pan(45, -32);
    projected = view.screen(fixed);
    require(std::abs(projected.x - anchor.x - 45) < 1e-8 &&
                std::abs(projected.y - anchor.y + 32) < 1e-8,
            "panning must use screen pixels at every zoom");
    const auto center = view.world(view.center());
    view.resize(400, 300);
    const auto resized = view.world(view.center());
    require(std::abs(center.x - resized.x) < 1e-9 && std::abs(center.y - resized.y) < 1e-9,
            "resize must preserve the world point at the canvas center");
    view.fit({-200, -100}, {500, 300});
    const auto minimum = view.screen({-200, -100});
    const auto maximum = view.screen({500, 300});
    require(minimum.x >= 15 && minimum.y >= 15 && maximum.x <= 385 && maximum.y <= 285,
            "fit must contain all graph bounds with padding");
    view.zoom(1e100, view.center());
    require(view.scale() == 4, "maximum zoom not clamped");
    view.zoom(1e-100, view.center());
    require(view.scale() == .1, "minimum zoom not clamped");
    view.zoom(0, anchor);
    view.zoom(std::numeric_limits<double>::infinity(), anchor);
    view.zoom(std::numeric_limits<double>::quiet_NaN(), anchor);
    require(view.scale() == .1, "non-finite or nonpositive zoom must be ignored");
    view.reset();
    const auto reset = view.screen({30, 50});
    require(view.scale() == 1 && reset.x == 30 && reset.y == 50,
            "reset must restore a predictable one-to-one view");
}

} // namespace nts::bonus::test
