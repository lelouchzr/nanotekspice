#include "BasicSupport.hpp"
#include "nts/ComponentFactory.hpp"
#include <memory>
#include <vector>

namespace nts::test {
namespace {

void cacheTests() {
    auto source = createComponent("input");
    CountedBuffer gate;
    source->setLink(1, gate, 1);
    source->setInput(True);
    tick({source.get(), &gate}, 1);
    for (int i = 0; i < 100; ++i)
        require(gate.compute(2) == True, "cached gate value changed within one phase");
    require(gate.evaluations() == 1, "output evaluation was not memoized");
    source->setInput(False);
    require(gate.compute(2) == True && gate.evaluations() == 1,
            "pending assignment invalidated committed output");
    tick({&gate, source.get()}, 2);
    require(gate.compute(2) == False && gate.evaluations() == 2,
            "new tick did not invalidate output cache");
}

void simultaneousTests() {
    for (const bool reverse : {false, true}) {
        auto input = createComponent("input");
        SampledLatch first, second;
        input->setLink(1, first, 1);
        first.setLink(2, second, 1);
        const std::vector<IComponent *> components =
            reverse ? std::vector<IComponent *>{&second, &first, input.get()}
                    : std::vector<IComponent *>{input.get(), &first, &second};
        input->setInput(True);
        tick(components, 1);
        require(first.compute(2) == True && second.compute(2) == False,
                "sequential cascade observed another component's unprepared next state");
        tick(components, 2);
        require(second.compute(2) == True, "sequential cascade did not advance next tick");
        input->setInput(False);
        tick(components, 3);
        require(first.compute(2) == False && second.compute(2) == True,
                "sequential capture depended on component iteration order");
    }
}

void cycleTests() {
    auto inverter = createComponent("not");
    inverter->setLink(1, *inverter, 2);
    require(inverter->compute(2) == Undefined, "inverter cycle did not resolve as U");
    auto gate = createComponent("and");
    auto zero = createComponent("false");
    gate->setLink(1, *gate, 3);
    zero->setLink(1, *gate, 2);
    std::vector<std::unique_ptr<IComponent>> consumers;
    for (int i = 0; i < 3; ++i) {
        consumers.push_back(createComponent("output"));
        consumers.back()->setLink(1, *gate, 3);
    }
    require(gate->compute(3) == False, "controlling zero in a feedback AND was lost");
}

void depthTests() {
    auto source = createComponent("true");
    std::vector<std::unique_ptr<IComponent>> gates;
    for (int i = 0; i < 600; ++i) {
        auto gate = createComponent("not");
        if (gates.empty())
            gate->setLink(1, *source, 1);
        else
            gate->setLink(1, *gates.back(), 2);
        gates.push_back(std::move(gate));
    }
    std::vector<std::unique_ptr<IComponent>> consumers;
    for (int i = 0; i < 4; ++i) {
        consumers.push_back(createComponent("output"));
        consumers.back()->setLink(1, *gates.back(), 2);
    }
    rejected([&] { static_cast<void>(gates.back()->compute(2)); },
             "deep acyclic chain silently returned an incorrect U instead of a diagnostic");
    // An iterative graph scheduler can prewarm a valid DAG without recursive evaluation.
    for (std::size_t i = 0; i < gates.size(); ++i)
        require(gates[i]->compute(2) == (i % 2 == 0 ? False : True),
                "topological prewarming did not recover a deep valid chain");
}

} // namespace

void phaseAndGuardTests() {
    cacheTests();
    simultaneousTests();
    cycleTests();
    depthTests();
}

} // namespace nts::test
