#pragma once

#include "Circuit.hpp"
#include <filesystem>
#include <istream>
#include <string>

namespace nts {

class Parser {
  public:
    [[nodiscard]] static Circuit load(const std::filesystem::path &path);
    [[nodiscard]] static Circuit parse(std::istream &stream,
                                       const std::string &source = "<stream>");
};

} // namespace nts
