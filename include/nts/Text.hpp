#pragma once

#include <cstddef>
#include <istream>
#include <string>

namespace nts {

// Bounds memory before reading an entire untrusted line; preserves embedded NULs for validation.
[[nodiscard]] bool readBoundedLine(std::istream &input, std::string &line, std::size_t limit,
                                   const std::string &source);

} // namespace nts
