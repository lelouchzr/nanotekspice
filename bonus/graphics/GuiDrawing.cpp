#ifdef NTS_WITH_GUI
#include "Glyphs.hpp"
#include "Gui.hpp"
#include <SDL.h>
#include <cmath>

namespace nts::bonus {

int Gui::screenX(double x) const {
    return static_cast<int>(std::lround(viewport_.screen({x, 0}).x + 12));
}

int Gui::screenY(double y) const {
    return static_cast<int>(std::lround(viewport_.screen({0, y}).y + 72));
}

void Gui::rectangle(int x, int y, int width, int height, std::uint32_t color) {
    SDL_SetRenderDrawColor(renderer_, static_cast<Uint8>((color >> 16U) & 255U),
                           static_cast<Uint8>((color >> 8U) & 255U),
                           static_cast<Uint8>(color & 255U), 255);
    const SDL_Rect rectangle{x, y, width, height};
    SDL_RenderFillRect(renderer_, &rectangle);
}

void Gui::worldRectangle(double x, double y, double width, double height, std::uint32_t color) {
    const auto point = viewport_.screen({x, y});
    const auto scale = viewport_.scale();
    const SDL_FRect rectangle{static_cast<float>(point.x + 12), static_cast<float>(point.y + 72),
                              static_cast<float>(width * scale),
                              static_cast<float>(height * scale)};
    SDL_SetRenderDrawColor(renderer_, static_cast<Uint8>((color >> 16U) & 255U),
                           static_cast<Uint8>((color >> 8U) & 255U),
                           static_cast<Uint8>(color & 255U), 255);
    SDL_RenderFillRectF(renderer_, &rectangle);
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

void Gui::worldText(double x, double y, std::string_view value, std::uint32_t color) {
    if (viewport_.scale() < .5)
        return;
    for (char character : value) {
        const auto glyph = Glyphs::rows(character);
        for (std::size_t row = 0; row < glyph.size(); ++row)
            for (unsigned column = 0; column < 5; ++column)
                if (glyph[row] & (1U << (4U - column)))
                    worldRectangle(x + column, y + static_cast<double>(row), 1, 1, color);
        x += 6;
    }
}

} // namespace nts::bonus
#endif
