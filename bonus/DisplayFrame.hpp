#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace nts::bonus {

enum class DisplayKind { SevenSegment, PixelMatrix, Processor };

struct DisplayFrame {
    DisplayKind kind = DisplayKind::PixelMatrix;
    std::size_t width = 0;
    std::size_t height = 0;
    std::vector<std::uint32_t> pixels;
    std::vector<bool> known;
    std::string text;
};

} // namespace nts::bonus
