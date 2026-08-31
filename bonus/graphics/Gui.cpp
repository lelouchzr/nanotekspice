#ifdef NTS_WITH_GUI
#include "Gui.hpp"
#include "../GraphGui.hpp"
#include "Export.hpp"
#include <SDL.h>
#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <utility>

namespace nts::bonus {

Gui::Gui(GraphCallbacks callbacks) : callbacks_(std::move(callbacks)) {
    if (!callbacks_.captureGraph || !callbacks_.step || !callbacks_.setInput)
        throw std::invalid_argument("GUI requires capture, simulation and input callbacks");
    if (const char *value = std::getenv("NTS_GUI_SMOKE_FRAMES")) {
        const std::string_view input(value);
        const auto parsed =
            std::from_chars(input.data(), input.data() + input.size(), smokeFrames_);
        if (parsed.ec != std::errc{} || parsed.ptr != input.data() + input.size() ||
            smokeFrames_ == 0 || smokeFrames_ > 1000)
            throw std::invalid_argument("NTS_GUI_SMOKE_FRAMES expects 1..1000");
    }
    ownsSdl_ = SDL_WasInit(0) == 0;
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        const std::string error = SDL_GetError();
        shutdown();
        throw std::runtime_error("SDL video initialization: " + error);
    }
    videoReady_ = true;
    viewport_.resize(width_ - 324, height_ - 84);
    window_ = SDL_CreateWindow("NanoTekSpice / live circuit", SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, width_, height_, SDL_WINDOW_RESIZABLE);
    if (window_)
        SDL_SetWindowMinimumSize(window_, 640, 480);
    if (window_)
        renderer_ =
            SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (window_ && !renderer_)
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    if (!window_ || !renderer_) {
        const std::string error = SDL_GetError();
        shutdown();
        throw std::runtime_error("SDL window/renderer: " + error);
    }
}

Gui::~Gui() {
    shutdown();
}

const Viewport &Gui::view() const noexcept {
    return viewport_;
}

void Gui::shutdown() noexcept {
    if (renderer_)
        SDL_DestroyRenderer(renderer_);
    if (window_)
        SDL_DestroyWindow(window_);
    renderer_ = nullptr;
    window_ = nullptr;
    if (videoReady_)
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    videoReady_ = false;
    // Subsystem shutdown leaves SDL's global caches alive; release them when we own SDL.
    if (ownsSdl_)
        SDL_Quit();
    ownsSdl_ = false;
}

void Gui::pixels(const DisplayFrame &display, int x, int y) {
    if (display.width == 0 || display.height == 0 || display.width > 1024 ||
        display.height > 1024 || display.pixels.size() != display.width * display.height)
        return;
    const double scale = std::min(200.0 / static_cast<double>(display.width),
                                  120.0 / static_cast<double>(display.height));
    for (std::size_t row = 0; row < display.height; ++row)
        for (std::size_t column = 0; column < display.width; ++column) {
            const auto index = row * display.width + column;
            const bool known =
                display.known.empty() || (index < display.known.size() && display.known[index]);
            const auto color = known                ? display.pixels[index]
                               : (row + column) % 2 ? 0x303c4cU
                                                    : 0x1d2735U;
            worldRectangle(x + static_cast<double>(column) * scale,
                           y + static_cast<double>(row) * scale, scale, scale, color);
        }
}

void Gui::node(const GraphNode &value) {
    worldRectangle(value.x, value.y, 240, nodeHeight(value), 0x142131);
    worldRectangle(value.x, value.y, 240, 3, selected_ == value.name ? 0xf3a85c : 0x345169);
    worldText(value.x + 12, value.y + 12, value.name.substr(0, 28), 0xdce8ef);
    worldText(value.x + 12, value.y + 35, value.type, 0x78d5ad);
    std::string bits;
    for (const auto &pin : value.pins)
        bits += std::string(toString(pin.value));
    worldText(value.x + 12, value.y + 58, bits.substr(0, 32), 0x86a3be);
    if (value.display) {
        pixels(*value.display, value.x + 14, value.y + 90);
        if (value.display->pixels.empty()) {
            const auto &content = value.display->text;
            worldText(value.x + 12, value.y + 82, content.substr(0, 28), 0xf3a85c);
            if (content.size() > 28)
                worldText(value.x + 12, value.y + 98, content.substr(28, 28), 0x86a3be);
        }
    }
}

void Gui::render() {
    int outputWidth = 0, outputHeight = 0;
    if (SDL_GetRendererOutputSize(renderer_, &outputWidth, &outputHeight) != 0)
        throw std::runtime_error(std::string("SDL renderer size: ") + SDL_GetError());
    if (outputWidth <= 0 || outputHeight <= 0)
        return;
    if (SDL_RenderSetScale(renderer_, static_cast<float>(outputWidth) / static_cast<float>(width_),
                           static_cast<float>(outputHeight) / static_cast<float>(height_)) != 0)
        throw std::runtime_error(std::string("SDL renderer scale: ") + SDL_GetError());
    SDL_SetRenderDrawColor(renderer_, 10, 17, 28, 255);
    SDL_RenderClear(renderer_);
    const SDL_Rect canvas{0, 64, width_ - 300, height_ - 64};
    SDL_RenderSetClipRect(renderer_, &canvas);
    SDL_SetRenderDrawColor(renderer_, 67, 99, 127, 255);
    for (const auto &edge : graph_.edges) {
        const auto first =
            std::find_if(graph_.nodes.begin(), graph_.nodes.end(),
                         [&](const auto &node) { return node.name == edge.fromNode; });
        const auto second =
            std::find_if(graph_.nodes.begin(), graph_.nodes.end(),
                         [&](const auto &node) { return node.name == edge.toNode; });
        if (first == graph_.nodes.end() || second == graph_.nodes.end())
            continue;
        SDL_RenderDrawLine(renderer_, screenX(first->x + 240), screenY(first->y + 49),
                           screenX(second->x), screenY(second->y + 49));
    }
    for (const auto &entry : graph_.nodes)
        node(entry);
    SDL_RenderSetClipRect(renderer_, nullptr);
    rectangle(0, 0, width_, 62, 0x172638);
    text(18, 18, "ENTER / F10 STEP", 0xdce8ef);
    text(135, 18, playing_ ? "SPACE PAUSE" : "SPACE RUN X32", 0x78d5ad);
    text(268, 18, "RESET VIEW", 0x86a3be);
    text(420, 18, "TICK " + std::to_string(graph_.tick), 0xf3a85c);
    text(width_ - 80, 18,
         std::to_string(static_cast<int>(std::lround(viewport_.scale() * 100))) + "%", 0xf3a85c);
    text(18, 40, "WHEEL +/- ZOOM / CTRL/CMD+0 RESET / HOME FIT / DRAG PAN / 0 1 U EDIT / ESC EXIT",
         0x86a3be);
    rectangle(width_ - 300, 62, 300, height_ - 62, 0x101c2a);
    int row = 82;
    text(width_ - 280, row, "COMPONENT INSPECTOR", 0x78d5ad);
    row += 25;
    if (selected_)
        for (const auto &entry : graph_.nodes)
            if (entry.name == *selected_) {
                text(width_ - 280, row, entry.name.substr(0, 40), 0xdce8ef);
                row += 20;
                for (const auto &pin : entry.pins) {
                    text(width_ - 280, row,
                         "PIN " + std::to_string(pin.number) + " " + pin.direction + " = " +
                             std::string(toString(pin.value)),
                         0x86a3be);
                    row += 13;
                    if (row > height_ - 100)
                        break;
                }
                if (entry.display) {
                    const auto &content = entry.display->text;
                    for (std::size_t offset = 0; offset < content.size() && row < height_ - 20;
                         offset += 40) {
                        text(width_ - 280, row, content.substr(offset, 40), 0xf3a85c);
                        row += 14;
                    }
                }
            }
    if (!captured_) {
        if (const char *path = std::getenv("NTS_GUI_CAPTURE"); path && *path) {
            int width = 0, height = 0;
            if (SDL_GetRendererOutputSize(renderer_, &width, &height) != 0)
                throw std::runtime_error(std::string("SDL capture dimensions: ") + SDL_GetError());
            std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> surface(
                SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32),
                SDL_FreeSurface);
            if (!surface ||
                SDL_RenderReadPixels(renderer_, nullptr, SDL_PIXELFORMAT_RGBA32, surface->pixels,
                                     surface->pitch) != 0 ||
                SDL_SaveBMP(surface.get(), path) != 0)
                throw std::runtime_error(std::string("SDL capture: ") + SDL_GetError());
            captured_ = true;
        }
    }
    SDL_RenderPresent(renderer_);
}

void Gui::run() {
    unsigned frames = 0;
    while (running_) {
        poll();
        if (playing_ || smokeFrames_)
            for (unsigned step = 0; step < (smokeFrames_ ? 1U : 32U); ++step)
                callbacks_.step();
        graph_ = arrangeGraph(callbacks_.captureGraph());
        render();
        if (smokeFrames_ && ++frames >= smokeFrames_)
            break;
        SDL_Delay(16);
    }
}

void runGui(GraphCallbacks callbacks) {
    Gui gui(std::move(callbacks));
    gui.run();
}

} // namespace nts::bonus

#else
#include "../GraphGui.hpp"
#include <stdexcept>

namespace nts::bonus {
void runGui(GraphCallbacks) {
    throw std::runtime_error("GUI support is disabled; rebuild with NTS_BUILD_GRAPHICS=ON");
}
} // namespace nts::bonus
#endif
