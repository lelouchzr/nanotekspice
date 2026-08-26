#include "cpu/Subleq.hpp"
#include <iostream>
#include <stdexcept>

int main() {
    try {
        std::size_t cases = 0;
        if (!(std::cin >> cases) || cases > 1000)
            throw std::runtime_error("invalid oracle case count");
        for (std::size_t test = 0; test < cases; ++test) {
            std::size_t cells = 0, budget = 0, inputs = 0, watches = 0;
            std::string entry;
            if (!(std::cin >> cells >> entry >> budget >> inputs >> watches) || cells > 10000 ||
                budget > 10000 || inputs > 10000 || watches > 10000)
                throw std::runtime_error("invalid oracle case");
            nts::bonus::ProgramData program;
            program.entry = nts::bonus::Integer(entry);
            for (std::size_t i = 0; i < cells; ++i) {
                std::string address, value;
                std::cin >> address >> value;
                program.memory.emplace(nts::bonus::Integer(address), nts::bonus::Integer(value));
            }
            nts::bonus::Subleq cpu;
            cpu.load(std::move(program));
            for (std::size_t i = 0; i < inputs; ++i) {
                unsigned value = 0;
                std::cin >> value;
                if (value > 255)
                    throw std::runtime_error("invalid input byte");
                cpu.input(static_cast<std::uint8_t>(value));
            }
            std::vector<nts::bonus::Integer> watch;
            for (std::size_t i = 0; i < watches; ++i) {
                std::string address;
                std::cin >> address;
                watch.emplace_back(address);
            }
            if (!std::cin)
                throw std::runtime_error("truncated oracle input");
            cpu.run(budget);
            const char *state = cpu.status() == nts::bonus::CpuStatus::Halted    ? "halted"
                                : cpu.status() == nts::bonus::CpuStatus::Waiting ? "waiting"
                                                                                 : "ready";
            std::cout << cpu.pc().string() << ' ' << state << ' ' << cpu.instructions() << ' '
                      << cpu.output().size() << '\n';
            for (const auto &event : cpu.output())
                std::cout << static_cast<unsigned>(event.value) << ' ' << event.address << ' '
                          << event.serial << '\n';
            for (const auto &address : watch)
                std::cout << cpu.read(address).string() << '\n';
        }
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
