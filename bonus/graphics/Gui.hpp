#pragma once

#include "../GraphModel.hpp"
#include <optional>
#include <string>

struct SDL_Window;
struct SDL_Renderer;

namespace nts::bonus {

class Gui {
  public:
    explicit Gui(GraphCallbacks callbacks);
    ~Gui();
    Gui(const Gui &) = delete;
    Gui &operator=(const Gui &) = delete;
    void run();

  private:
    GraphCallbacks callbacks_;
    GraphModel graph_;
    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;
    bool running_ = true;
    bool playing_ = false;
    bool dragging_ = false;
    bool captured_ = false;
    int lastX_ = 0, lastY_ = 0;
    int width_ = 1280, height_ = 820;
    double zoom_ = 1;
    double panX_ = 0, panY_ = 0;
    unsigned smokeFrames_ = 0;
    std::optional<std::string> selected_;
    void poll();
    void render();
    void click(int x, int y);
    void assign(Tristate value);
    void node(const GraphNode &node);
    void pixels(const DisplayFrame &display, int x, int y);
    void text(int x, int y, std::string_view value, std::uint32_t color, int scale = 1);
    void rectangle(int x, int y, int width, int height, std::uint32_t color);
    void worldRectangle(double x, double y, double width, double height, std::uint32_t color);
    [[nodiscard]] int screenX(double x) const;
    [[nodiscard]] int screenY(double y) const;
};

} // namespace nts::bonus
