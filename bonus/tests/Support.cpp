#include "Support.hpp"
#include <stdexcept>

namespace nts::bonus::test {

std::size_t checks = 0;

void require(bool condition, const std::string &message) {
    ++checks;
    if (!condition)
        throw std::runtime_error(message);
}

void rejects(const std::function<void()> &operation, const std::string &message) {
    bool caught = false;
    try {
        operation();
    } catch (const std::exception &) {
        caught = true;
    }
    require(caught, message);
}

void addInputs(Circuit &circuit, const std::string &target, std::size_t count,
               std::size_t firstPin) {
    for (std::size_t bit = 0; bit < count; ++bit) {
        const auto name = target + "_bit" + std::to_string(firstPin + bit);
        circuit.add("input", name);
        circuit.connect({name, 1, target, firstPin + bit});
    }
}

void setBits(Circuit &circuit, const std::string &target, std::size_t count, std::uint32_t value,
             std::size_t firstPin) {
    for (std::size_t bit = 0; bit < count; ++bit)
        circuit.setInput(target + "_bit" + std::to_string(firstPin + bit),
                         value & (1U << bit) ? True : False);
}

} // namespace nts::bonus::test
