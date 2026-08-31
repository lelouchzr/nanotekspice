#ifdef NTS_WITH_GUI
#include "Export.hpp"
#include "Gui.hpp"
#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <limits>

namespace nts::bonus {

bool Gui::canvasContains(int x, int y) const {
    return x >= 0 && x < width_ - 300 && y >= 64 && y < height_;
}

void Gui::assign(Tristate value) {
    if (!selected_)
        return;
    const auto node = std::find_if(graph_.nodes.begin(), graph_.nodes.end(),
                                   [&](const auto &entry) { return entry.name == *selected_; });
    if (node != graph_.nodes.end() && (node->type == "input" || node->type == "clock"))
        callbacks_.setInput(node->name, value);
}

void Gui::fit() {
    if (graph_.nodes.empty()) {
        viewport_.reset();
        return;
    }
    ViewPoint minimum{std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
    ViewPoint maximum{std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest()};
    for (const auto &node : graph_.nodes) {
        minimum.x = std::min(minimum.x, static_cast<double>(node.x));
        minimum.y = std::min(minimum.y, static_cast<double>(node.y));
        maximum.x = std::max(maximum.x, static_cast<double>(node.x) + 240);
        maximum.y = std::max(maximum.y, static_cast<double>(node.y) + nodeHeight(node));
    }
    viewport_.fit(minimum, maximum);
}

void Gui::wheel(double steps, int x, int y) {
    if (!canvasContains(x, y) || !std::isfinite(steps) || steps == 0)
        return;
    viewport_.zoom(std::pow(1.15, std::clamp(steps, -8.0, 8.0)),
                   {static_cast<double>(x) - 12, static_cast<double>(y) - 72});
}

void Gui::key(int code, unsigned modifiers) {
    const bool shortcut = (modifiers & (KMOD_CTRL | KMOD_GUI)) != 0;
    if (code == SDLK_ESCAPE)
        running_ = false;
    else if (code == SDLK_PLUS || code == SDLK_EQUALS || code == SDLK_KP_PLUS)
        viewport_.zoom(1.15, viewport_.center());
    else if (code == SDLK_MINUS || code == SDLK_KP_MINUS)
        viewport_.zoom(1 / 1.15, viewport_.center());
    else if (shortcut && (code == SDLK_0 || code == SDLK_KP_0))
        viewport_.reset();
    else if (code == SDLK_HOME)
        fit();
    else if (code == SDLK_SPACE)
        playing_ = !playing_;
    else if (code == SDLK_F10 || code == SDLK_RETURN)
        callbacks_.step();
    else if (code == SDLK_0 || code == SDLK_KP_0)
        assign(False);
    else if (code == SDLK_1 || code == SDLK_KP_1)
        assign(True);
    else if (code == SDLK_u)
        assign(Undefined);
}

void Gui::click(int x, int y) {
    if (y < 64) {
        if (y >= 32)
            return;
        if (x < 120)
            callbacks_.step();
        else if (x < 250)
            playing_ = !playing_;
        else if (x < 390)
            viewport_.reset();
        return;
    }
    if (!canvasContains(x, y))
        return;
    for (auto node = graph_.nodes.rbegin(); node != graph_.nodes.rend(); ++node)
        if (x >= screenX(node->x) && x <= screenX(node->x + 240) && y >= screenY(node->y) &&
            y <= screenY(node->y + nodeHeight(*node))) {
            selected_ = node->name;
            if (node->type == "input" || node->type == "clock") {
                const auto value = node->pins.empty() ? Undefined : node->pins.front().value;
                assign(value == Undefined ? False : value == False ? True : Undefined);
            }
            return;
        }
    selected_.reset();
    dragging_ = true;
    lastX_ = x;
    lastY_ = y;
}

void Gui::poll() {
    SDL_Event event;
    while (SDL_PollEvent(&event))
        handleEvent(event);
}

void Gui::handleEvent(const SDL_Event &event) {
    if (event.type == SDL_QUIT)
        running_ = false;
    if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            SDL_GetWindowSize(window_, &width_, &height_);
            width_ = std::max(640, width_);
            height_ = std::max(480, height_);
            viewport_.resize(width_ - 324, height_ - 84);
        } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
            dragging_ = false;
    }
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
        click(event.button.x, event.button.y);
    if (event.type == SDL_MOUSEBUTTONUP)
        dragging_ = false;
    if (event.type == SDL_MOUSEMOTION && dragging_) {
        viewport_.pan(event.motion.x - lastX_, event.motion.y - lastY_);
        lastX_ = event.motion.x;
        lastY_ = event.motion.y;
    }
    if (event.type == SDL_MOUSEWHEEL) {
        int x = 0, y = 0;
        SDL_GetMouseState(&x, &y);
#if SDL_VERSION_ATLEAST(2, 26, 0)
        x = event.wheel.mouseX;
        y = event.wheel.mouseY;
#endif
        double steps = event.wheel.y;
#if SDL_VERSION_ATLEAST(2, 0, 18)
        // A zero precise delta is meaningful for horizontal-only gestures.
        steps = event.wheel.preciseY;
#endif
        if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
            steps = -steps;
        wheel(steps, x, y);
    }
    if (event.type == SDL_KEYDOWN)
        key(event.key.keysym.sym, event.key.keysym.mod);
}

} // namespace nts::bonus
#endif
