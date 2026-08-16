#pragma once

#include "Program.hpp"
#include <cstdint>
#include <deque>
#include <optional>
#include <vector>

namespace nts::bonus {

enum class CpuStatus { Ready, Waiting, Halted };

struct CpuOutput {
    std::uint8_t value = 0;
    std::uint16_t address = 0;
    bool serial = true;
};

class Subleq {
  public:
    explicit Subleq(std::size_t cellLimit = 1000000);
    void load(ProgramData program);
    void reset();
    bool step();
    std::size_t run(std::size_t budget);
    void input(std::uint8_t value);
    [[nodiscard]] Integer read(const Integer &address) const;
    [[nodiscard]] const Integer &pc() const;
    [[nodiscard]] CpuStatus status() const;
    [[nodiscard]] std::uint64_t instructions() const;
    [[nodiscard]] const std::vector<CpuOutput> &output() const;
    [[nodiscard]] std::optional<CpuOutput> lastOutput() const;

  private:
    ProgramData program_;
    std::map<Integer, Integer> memory_;
    Integer pc_;
    CpuStatus status_ = CpuStatus::Halted;
    std::size_t cellLimit_;
    std::uint64_t instructions_ = 0;
    std::deque<std::uint8_t> input_;
    std::vector<CpuOutput> output_;
    std::optional<CpuOutput> lastOutput_;
    void write(const Integer &address, Integer value);
    void emit(std::uint8_t value, std::uint16_t address, bool serial);
};

} // namespace nts::bonus
