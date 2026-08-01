#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace nts {

struct Options {
    std::filesystem::path circuit;
    bool help = false;
    bool gui = false;
    std::optional<std::size_t> ticks;
    std::optional<std::filesystem::path> svg;
    std::optional<std::filesystem::path> html;
    std::vector<std::string> assignments;
    std::vector<std::pair<std::string, std::filesystem::path>> programs;
};

[[nodiscard]] Options parseOptions(int argc, char **argv);

} // namespace nts
