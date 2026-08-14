#pragma once
#include "nts/Circuit.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace nts::benchmark {

class Workload {
  public:
    Workload(const std::string &name, std::size_t size);
    [[nodiscard]] std::uint64_t run(std::size_t iterations);

  private:
    Circuit circuit_;
    std::vector<std::string> outputs_;
    std::string source_;
    std::size_t iteration_ = 0;
    void output(const std::string &name, std::size_t pin);
    void fanout(std::size_t size);
    void cascade(std::size_t size);
    void counters(std::size_t size);
};

} // namespace nts::benchmark
