#include "Workload.hpp"
#include <stdexcept>

namespace nts::benchmark {

Workload::Workload(const std::string &name, std::size_t size) {
    if (size == 0 || size > 10000)
        throw std::invalid_argument("workload size must be 1..10000");
    if (name == "fanout")
        fanout(size);
    else if (name == "cascade")
        cascade(size);
    else if (name == "counters")
        counters(size);
    else
        throw std::invalid_argument("unknown workload");
}

void Workload::output(const std::string &name, std::size_t pin) {
    const auto port = "out_" + std::to_string(outputs_.size());
    circuit_.add("output", port);
    circuit_.connect({name, pin, port, 1});
    outputs_.push_back(port);
}

void Workload::fanout(std::size_t size) {
    source_ = "source";
    circuit_.add("input", source_);
    circuit_.add("true", "one");
    for (std::size_t index = 0; index < size; ++index) {
        const auto name = "gate_" + std::to_string(index);
        circuit_.add("and", name);
        circuit_.connect({source_, 1, name, 1});
        circuit_.connect({"one", 1, name, 2});
        output(name, 3);
    }
}

void Workload::cascade(std::size_t size) {
    if (size > 400)
        throw std::invalid_argument("cascade safety depth is 400");
    source_ = "source";
    circuit_.add("input", source_);
    std::string previous = source_;
    std::size_t pin = 1;
    for (std::size_t index = 0; index < size; ++index) {
        const auto name = "gate_" + std::to_string(index);
        circuit_.add("not", name);
        circuit_.connect({previous, pin, name, 1});
        previous = name;
        pin = 2;
    }
    output(previous, pin);
}

void Workload::counters(std::size_t size) {
    source_ = "clock";
    circuit_.add("clock", source_);
    circuit_.add("input", "reset");
    circuit_.setInput("reset", True);
    for (std::size_t index = 0; index < size; ++index) {
        const auto name = "counter_" + std::to_string(index);
        circuit_.add("4040", name);
        circuit_.connect({source_, 1, name, 10});
        circuit_.connect({"reset", 1, name, 11});
        output(name, 9);
        output(name, 1);
    }
    circuit_.simulate();
    circuit_.setInput("reset", False);
}

std::uint64_t Workload::run(std::size_t iterations) {
    std::uint64_t digest = 1469598103934665603ULL;
    for (std::size_t step = 0; step < iterations; ++step) {
        circuit_.setInput(source_, (iteration_++ & 1U) == 0 ? False : True);
        circuit_.simulate();
        for (const auto &name : outputs_) {
            const auto value = circuit_.component(name).compute(1);
            digest ^= static_cast<std::uint64_t>(static_cast<int>(value) + 1);
            digest *= 1099511628211ULL;
        }
    }
    return digest;
}

} // namespace nts::benchmark
