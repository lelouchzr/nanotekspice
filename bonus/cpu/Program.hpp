#pragma once

#include "Integer.hpp"
#include <filesystem>
#include <istream>
#include <map>
#include <string>
#include <vector>

namespace nts::bonus {

struct ProgramData {
    std::map<Integer, Integer> memory;
    Integer entry;
};

struct ProgramWord {
    Integer address;
    std::string token;
    std::size_t line = 0;
};

class Program {
  public:
    [[nodiscard]] static ProgramData load(const std::filesystem::path &path);
    [[nodiscard]] static ProgramData parse(std::istream &input, const std::string &source);

  private:
    [[nodiscard]] static Integer resolve(const std::string &token,
                                         const std::map<std::string, Integer> &labels);
};

} // namespace nts::bonus
