#include "Subleq.hpp"
#include <algorithm>
#include <stdexcept>
#include <utility>

namespace nts::bonus {

Subleq::Subleq(std::size_t cellLimit) : cellLimit_(cellLimit) {
    if (cellLimit == 0)
        throw std::invalid_argument("CPU cell budget must be positive");
}

void Subleq::load(ProgramData program) {
    if (program.memory.size() > cellLimit_ || program.entry.negative() ||
        std::any_of(program.memory.begin(), program.memory.end(),
                    [](const auto &cell) { return cell.first.negative(); }))
        throw std::invalid_argument("invalid or oversized CPU image");
    program_ = std::move(program);
    reset();
}

void Subleq::reset() {
    memory_ = program_.memory;
    pc_ = program_.entry;
    status_ = memory_.empty() ? CpuStatus::Halted : CpuStatus::Ready;
    instructions_ = 0;
    input_.clear();
    output_.clear();
    lastOutput_.reset();
}

Integer Subleq::read(const Integer &address) const {
    if (address.negative())
        throw std::invalid_argument("negative CPU memory address");
    const auto found = memory_.find(address);
    return found == memory_.end() ? Integer{} : found->second;
}

void Subleq::write(const Integer &address, Integer value) {
    if (address.negative())
        throw std::invalid_argument("negative CPU memory destination");
    const auto found = memory_.find(address);
    if (found == memory_.end() && memory_.size() >= cellLimit_)
        throw std::runtime_error("CPU sparse memory cell budget exceeded");
    memory_[address] = std::move(value);
}

void Subleq::emit(std::uint8_t value, std::uint16_t address, bool serial) {
    if (output_.size() >= 1048576)
        throw std::runtime_error("CPU output buffer budget exceeded");
    lastOutput_ = CpuOutput{value, address, serial};
    output_.push_back(*lastOutput_);
}

bool Subleq::step() {
    if (status_ == CpuStatus::Halted)
        return false;
    const auto a = read(pc_);
    const auto b = read(pc_ + Integer(1));
    const auto c = read(pc_ + Integer(2));
    if (a == Integer(-1)) {
        if (b.negative())
            throw std::runtime_error("CPU input destination must be nonnegative");
        if (input_.empty()) {
            status_ = CpuStatus::Waiting;
            return false;
        }
        write(b, Integer(input_.front()));
        input_.pop_front();
        pc_ += Integer(3);
    } else if (b == Integer(-1) || b == Integer(-2)) {
        const auto value = static_cast<std::uint8_t>(read(a).lowBits(8));
        const auto address = b == Integer(-2) ? static_cast<std::uint16_t>(read(c).lowBits(12))
                                              : static_cast<std::uint16_t>(output_.size() % 4096);
        emit(value, address, b == Integer(-1));
        pc_ += Integer(3);
    } else {
        const auto result = read(b) - read(a);
        write(b, result);
        pc_ = result <= Integer{} ? c : pc_ + Integer(3);
    }
    ++instructions_;
    status_ = pc_.negative() ? CpuStatus::Halted : CpuStatus::Ready;
    return true;
}

std::size_t Subleq::run(std::size_t budget) {
    std::size_t executed = 0;
    while (executed < budget && step())
        ++executed;
    return executed;
}

void Subleq::input(std::uint8_t value) {
    if (input_.size() >= 65536)
        throw std::runtime_error("CPU input queue budget exceeded");
    input_.push_back(value);
    if (status_ == CpuStatus::Waiting)
        status_ = CpuStatus::Ready;
}

const Integer &Subleq::pc() const {
    return pc_;
}

CpuStatus Subleq::status() const {
    return status_;
}

std::uint64_t Subleq::instructions() const {
    return instructions_;
}

const std::vector<CpuOutput> &Subleq::output() const {
    return output_;
}

std::optional<CpuOutput> Subleq::lastOutput() const {
    return lastOutput_;
}

} // namespace nts::bonus
