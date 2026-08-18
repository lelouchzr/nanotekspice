#pragma once

#include <array>

namespace nts::bonus {

class Glyphs {
  public:
    [[nodiscard]] static std::array<unsigned char, 7> rows(char character);
};

} // namespace nts::bonus
