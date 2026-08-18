#pragma once

#include "../GraphModel.hpp"
#include <string>
#include <string_view>

namespace nts::bonus {

[[nodiscard]] std::string escapeXml(std::string_view value);
[[nodiscard]] std::string colorHex(std::uint32_t color);
[[nodiscard]] int nodeHeight(const GraphNode &node);
[[nodiscard]] GraphModel arrangeGraph(GraphModel graph);

} // namespace nts::bonus
