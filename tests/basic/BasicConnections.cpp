#include "BasicSupport.hpp"
#include "nts/ComponentFactory.hpp"
#include <memory>
#include <vector>

namespace nts::test {
namespace {

void busTests() {
    DriveProbe high(Drive::High), low(Drive::HighImpedance), unknown(Drive::HighImpedance);
    auto first = createComponent("output");
    auto second = createComponent("output");
    first->setLink(1, high, 1);
    low.setLink(1, *second, 1);
    require(first->compute(1) == True && second->compute(1) == Undefined,
            "separate cached nets fixture");
    first->setLink(1, *second, 1); // Input-to-input linking merges existing electrical nets.
    require(first->compute(1) == True && second->compute(1) == True,
            "high impedance masked an active driver");
    low.value(Drive::Low);
    require(first->compute(1) == Undefined && high.compute(1) == Undefined,
            "conflicting drivers were not resolved as U");
    low.value(Drive::High);
    require(first->compute(1) == True, "matching drivers incorrectly conflicted");
    unknown.setLink(1, *second, 1);
    unknown.value(Drive::Unknown);
    require(second->compute(1) == Undefined, "unknown active driver was ignored");
    unknown.value(Drive::HighImpedance);
    high.value(Drive::HighImpedance);
    low.value(Drive::HighImpedance);
    require(first->compute(1) == Undefined, "floating bus guessed a value");
}

void excludedDriverTests() {
    BidirectionalProbe component(Drive::High);
    DriveProbe external(Drive::Low);
    component.setLink(1, external, 1);
    std::vector<std::unique_ptr<IComponent>> consumers;
    for (int i = 0; i < 3; ++i) {
        consumers.push_back(createComponent("output"));
        consumers.back()->setLink(1, component, 1);
    }
    require(consumers.front()->compute(1) == Undefined, "wide bus contention fixture");
    require(component.compute(1) == Undefined, "conflicting bidirectional bus fixture");
    require(component.compute(2) == False,
            "excluding a bidirectional driver returned full-bus contention");
    external.value(Drive::HighImpedance);
    require(component.compute(1) == True, "own bidirectional output was not visible");
    require(component.compute(2) == Undefined,
            "excluded self driver leaked into the external bus read");
    external.value(Drive::High);
    require(component.compute(1) == True && component.compute(2) == True,
            "bidirectional peer value did not invalidate cached net resolution");
}

void driverIndexMergeTests() {
    auto high = std::make_unique<DriveProbe>(Drive::High);
    DriveProbe peer(Drive::HighImpedance);
    std::vector<std::unique_ptr<IComponent>> left, right;
    for (int i = 0; i < 4; ++i) {
        left.push_back(createComponent("output"));
        right.push_back(createComponent("output"));
        left.back()->setLink(1, *high, 1);
        right.back()->setLink(1, peer, 1);
    }
    require(left.front()->compute(1) == True && right.front()->compute(1) == Undefined,
            "two independently observed wide networks fixture");
    left.front()->setLink(1, *right.front(), 1);
    for (const auto &output : right)
        require(output->compute(1) == True, "merging wide nets lost the active driver");
    peer.value(Drive::Low);
    require(left.back()->compute(1) == Undefined,
            "wide net contention retained the old high value");
    high.reset();
    require(right.back()->compute(1) == False,
            "destroying a driver left a conflicting entry in the net index");
}

void fanoutTests() {
    auto source = createComponent("input");
    source->setInput(True);
    source->simulate(1);
    std::vector<std::unique_ptr<IComponent>> outputs;
    for (int i = 0; i < 512; ++i) {
        outputs.push_back(createComponent("output"));
        outputs.back()->setLink(1, *source, 1);
    }
    for (const auto &output : outputs)
        require(output->compute(1) == True, "fanout lost a consumer");
    source->setInput(False);
    source->simulate(2);
    for (const auto &output : outputs)
        require(output->compute(1) == False, "fanout retained stale source value");
    source->setLink(1, *outputs.front(), 1); // Repeating either orientation is a no-op.
    outputs.front()->setLink(1, *source, 1);
    source->setLink(1, *source, 1);
    require(outputs.back()->compute(1) == False, "duplicate links changed a net");
}

void destructionTests() {
    auto source = createComponent("input");
    CountedBuffer gate;
    source->setLink(1, gate, 1);
    source->setInput(True);
    source->simulate(1);
    require(gate.compute(2) == True, "destruction fixture did not warm output cache");
    source.reset();
    require(gate.compute(2) == Undefined, "destroyed source left a dangling or cached driver");
    auto survivor = createComponent("true");
    {
        auto temporary = createComponent("output");
        temporary->setLink(1, *survivor, 1);
        require(temporary->compute(1) == True, "temporary output link failed");
    }
    require(survivor->compute(1) == True, "destroyed consumer damaged its source net");
}

} // namespace

void connectionTests() {
    busTests();
    excludedDriverTests();
    driverIndexMergeTests();
    fanoutTests();
    destructionTests();
    auto gate = createComponent("4081");
    auto source = createComponent("true");
    rejected([&] { gate->setLink(7, *source, 1); }, "power pin accepted a signal link");
    rejected([&] { gate->setLink(1, *source, 0); }, "invalid remote pin accepted");
    rejected([&] { gate->setLink(15, *source, 1); }, "invalid local pin accepted");
}

} // namespace nts::test
