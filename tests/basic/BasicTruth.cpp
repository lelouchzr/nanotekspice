#include "BasicSupport.hpp"
#include "nts/ComponentFactory.hpp"
#include <array>
#include <memory>

namespace nts::test {
namespace {

constexpr std::array<Tristate, 3> States{Undefined, False, True};
constexpr std::array<Tristate, 9> And{Undefined, False,     Undefined, False, False,
                                      False,     Undefined, False,     True};
constexpr std::array<Tristate, 9> Or{Undefined, Undefined, True, Undefined, False,
                                     True,      True,      True, True};
constexpr std::array<Tristate, 9> Xor{Undefined, Undefined, Undefined, Undefined, False,
                                      True,      Undefined, True,      False};
constexpr std::array<Tristate, 9> Nand{Undefined, True,      Undefined, True, True,
                                       True,      Undefined, True,      False};
constexpr std::array<Tristate, 9> Nor{Undefined, Undefined, False, Undefined, True,
                                      False,     False,     False, False};
constexpr std::array<Tristate, 3> Not{Undefined, True, False};

void elementary(const std::string &name, const std::array<Tristate, 9> &expected) {
    auto first = createComponent("input");
    auto second = createComponent("input");
    auto gate = createComponent(name);
    auto sink = createComponent("output");
    first->setLink(1, *gate, 1);
    second->setLink(1, *gate, 2);
    sink->setLink(1, *gate, 3);
    for (std::size_t left = 0; left < States.size(); ++left)
        for (std::size_t right = 0; right < States.size(); ++right) {
            first->setInput(States[left]);
            second->setInput(States[right]);
            tick({sink.get(), gate.get(), second.get(), first.get()}, left * 3 + right + 1);
            require(sink->compute(1) == expected[left * 3 + right], name + " public truth table");
        }
}

void packaged(const std::string &name, const std::array<Tristate, 9> &expected) {
    auto gate = createComponent(name);
    std::array<std::unique_ptr<IComponent>, 8> inputs;
    std::vector<IComponent *> components{gate.get()};
    constexpr std::array<std::size_t, 8> pins{1, 2, 5, 6, 8, 9, 12, 13};
    constexpr std::array<std::size_t, 4> outputs{3, 4, 10, 11};
    for (std::size_t i = 0; i < inputs.size(); ++i) {
        inputs[i] = createComponent("input");
        inputs[i]->setLink(1, *gate, pins[i]);
        components.push_back(inputs[i].get());
    }
    for (std::size_t round = 0; round < 9; ++round) {
        for (std::size_t group = 0; group < outputs.size(); ++group) {
            inputs[group * 2]->setInput(States[(round + group) % 3]);
            inputs[group * 2 + 1]->setInput(States[(round / 3 + group) % 3]);
        }
        tick(components, round + 1);
        for (std::size_t group = 0; group < outputs.size(); ++group) {
            const auto index = ((round + group) % 3) * 3 + (round / 3 + group) % 3;
            require(gate->compute(outputs[group]) == expected[index], name + " independent gates");
        }
    }
    require(gate->compute(7) == Undefined && gate->compute(14) == Undefined,
            "power pins must be ignored");
}

void inverter(const std::string &name, const std::vector<std::size_t> &inputs,
              const std::vector<std::size_t> &outputs) {
    auto gate = createComponent(name);
    std::vector<std::unique_ptr<IComponent>> sources;
    std::vector<IComponent *> components{gate.get()};
    for (const auto pin : inputs) {
        sources.push_back(createComponent("input"));
        sources.back()->setLink(1, *gate, pin);
        components.push_back(sources.back().get());
    }
    for (std::size_t round = 0; round < 3; ++round) {
        for (std::size_t i = 0; i < inputs.size(); ++i)
            sources[i]->setInput(States[(round + i) % 3]);
        tick(components, round + 1);
        for (std::size_t i = 0; i < outputs.size(); ++i)
            require(gate->compute(outputs[i]) == Not[(round + i) % 3], name + " inverter pins");
    }
}

} // namespace

void basicTruthTests() {
    elementary("and", And);
    elementary("or", Or);
    elementary("xor", Xor);
    packaged("4081", And);
    packaged("4071", Or);
    packaged("4030", Xor);
    packaged("4011", Nand);
    packaged("4001", Nor);
    inverter("not", {1}, {2});
    inverter("4069", {1, 3, 5, 9, 11, 13}, {2, 4, 6, 8, 10, 12});
}

} // namespace nts::test
