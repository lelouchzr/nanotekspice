#pragma once

#include "GraphModel.hpp"
#include <filesystem>
#include <string>

namespace nts::bonus {

void runGui(GraphCallbacks callbacks);
void exportSvg(const GraphModel &graph, const std::filesystem::path &path);
void exportHtml(const GraphModel &graph, const std::filesystem::path &path);
[[nodiscard]] std::string graphSvg(const GraphModel &graph);

} // namespace nts::bonus
