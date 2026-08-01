#pragma once
#include <string_view>

namespace nts {

enum Tristate { Undefined = -1, False = 0, True = 1 };
enum class Drive { HighImpedance, Low, High, Unknown };

[[nodiscard]] Tristate logicNot(Tristate value) noexcept;
[[nodiscard]] Tristate logicAnd(Tristate left, Tristate right) noexcept;
[[nodiscard]] Tristate logicOr(Tristate left, Tristate right) noexcept;
[[nodiscard]] Tristate logicXor(Tristate left, Tristate right) noexcept;
[[nodiscard]] Drive toDrive(Tristate value) noexcept;
[[nodiscard]] Tristate toTristate(Drive value) noexcept;
[[nodiscard]] std::string_view toString(Tristate value) noexcept;

} // namespace nts
