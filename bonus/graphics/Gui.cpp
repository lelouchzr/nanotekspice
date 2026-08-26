#ifdef NTS_WITH_GUI
#include "Gui.hpp"
#include "../GraphGui.hpp"
#include "Export.hpp"
#include "Glyphs.hpp"
#include <SDL.h>
#include <algorithm>
#include <charconv>
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

int Gui::screenX(double x) const {
    return static_cast<int>((x - panX_) * zoom_ + 12);
}

int Gui::screenY(double y) const {
    return static_cast<int>((y - panY_) * zoom_ + 72);
}

void Gui::rectangle(int x, int y, int width, int height, std::uint32_t color) {
    SDL_SetRenderDrawColor(renderer_, static_cast<Uint8>((color >> 16U) & 255U),
                           static_cast<Uint8>((color >> 8U) & 255U),
                           static_cast<Uint8>(color & 255U), 255);
    const SDL_Rect rectangle{x, y, width, height};
    SDL_RenderFillRect(renderer_, &rectangle);
}

void Gui::worldRectangle(double x, double y, double width, double height, std::uint32_t color) {
    rectangle(screenX(x), screenY(y), std::max(1, static_cast<int>(width * zoom_)),
              std::max(1, static_cast<int>(height * zoom_)), color);
}

void Gui::text(int x, int y, std::string_view value, std::uint32_t color, int scale) {
    for (char character : value) {
        const auto glyph = Glyphs::rows(character);
        for (std::size_t row = 0; row < glyph.size(); ++row)
            for (unsigned column = 0; column < 5; ++column)
                if (glyph[row] & (1U << (4U - column)))
                    rectangle(x + static_cast<int>(column) * scale,
                              y + static_cast<int>(row) * scale, scale, scale, color);
        x += 6 * scale;
    }
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
    const int font = zoom_ >= 1.6 ? 2 : 1;
    text(screenX(value.x + 12), screenY(value.y + 12), value.name.substr(0, 28), 0xdce8ef, font);
    text(screenX(value.x + 12), screenY(value.y + 35), value.type, 0x78d5ad, font);
    std::string bits;
    for (const auto &pin : value.pins)
        bits += std::string(toString(pin.value));
    text(screenX(value.x + 12), screenY(value.y + 58), bits.substr(0, 32), 0x86a3be, font);
    if (value.display) {
        pixels(*value.display, value.x + 14, value.y + 90);
        if (value.display->pixels.empty()) {
            const auto &content = value.display->text;
            text(screenX(value.x + 12), screenY(value.y + 82), content.substr(0, 28), 0xf3a85c);
            if (content.size() > 28)
                text(screenX(value.x + 12), screenY(value.y + 98), content.substr(28, 28),
                     0x86a3be);
        }
    }
}

void Gui::assign(Tristate value) {
    if (!selected_)
        return;
    const auto node = std::find_if(graph_.nodes.begin(), graph_.nodes.end(),
                                   [&](const auto &entry) { return entry.name == *selected_; });
    if (node != graph_.nodes.end() && (node->type == "input" || node->type == "clock"))
        callbacks_.setInput(node->name, value);
}

void Gui::click(int x, int y) {
    if (y < 60) {
        if (x < 120)
            callbacks_.step();
        else if (x < 250)
            playing_ = !playing_;
        else if (x < 390) {
            panX_ = panY_ = 0;
            zoom_ = 1;
        }
        return;
    }
    if (x >= width_ - 300)
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
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            running_ = false;
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            width_ = std::max(640, event.window.data1);
            height_ = std::max(480, event.window.data2);
        }
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
            click(event.button.x, event.button.y);
        if (event.type == SDL_MOUSEBUTTONUP)
            dragging_ = false;
        if (event.type == SDL_MOUSEMOTION && dragging_) {
            panX_ = std::clamp(panX_ - (event.motion.x - lastX_) / zoom_, -1000000.0, 1000000.0);
            panY_ = std::clamp(panY_ - (event.motion.y - lastY_) / zoom_, -1000000.0, 1000000.0);
            lastX_ = event.motion.x;
            lastY_ = event.motion.y;
        }
        if (event.type == SDL_MOUSEWHEEL)
            zoom_ = std::clamp(zoom_ * (event.wheel.y > 0 ? 1.15 : 1 / 1.15), .1, 4.0);
        if (event.type == SDL_KEYDOWN) {
            const auto key = event.key.keysym.sym;
            if (key == SDLK_ESCAPE)
                running_ = false;
            else if (key == SDLK_SPACE)
                playing_ = !playing_;
            else if (key == SDLK_F10 || key == SDLK_RETURN)
                callbacks_.step();
            else if (key == SDLK_0)
                assign(False);
            else if (key == SDLK_1)
                assign(True);
            else if (key == SDLK_u)
                assign(Undefined);
        }
    }
}

void Gui::render() {
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
    text(18, 40, "WHEEL ZOOM / DRAG PAN / CLICK INPUT / 0 1 U EDIT / ESC EXIT", 0x86a3be);
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
