#pragma once
#include "nts/Tristate.hpp"
#include <cstddef>
#include <span>

namespace nts::advanced {
Tristate merge(Tristate left, Tristate right) noexcept;
bool permits(Tristate value, bool candidate) noexcept;
Tristate edge(Tristate previous, Tristate current, bool rising) noexcept;
bool matchesAddress(std::span<const Tristate> bits, std::size_t address) noexcept;
Tristate decodeAddress(std::span<const Tristate> bits, std::size_t address) noexcept;
} // namespace nts::advanced
