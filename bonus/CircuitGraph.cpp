#include "CircuitGraph.hpp"
#include <array>

namespace nts::bonus {

GraphModel captureCircuit(const Circuit &circuit) {
    GraphModel graph;
    graph.tick = circuit.tick();
    std::size_t index = 0;
    constexpr std::array<const char *, 5> directions{"unused", "power", "input", "output",
                                                     "bidirectional"};
    for (const auto &[name, entry] : circuit.nodes()) {
        GraphNode node;
        node.name = name;
        node.type = entry.type;
        const auto location = circuit.layout().find(name);
        if (location == circuit.layout().end()) {
            node.x = 80 + static_cast<int>(index % 5) * 260;
            node.y = 100 + static_cast<int>(index / 5) * 240;
        } else {
            node.x = location->second.x;
            node.y = location->second.y;
        }
        ++index;
        auto &component = *entry.component;
        for (std::size_t pin = 1; pin <= component.pinCount(); ++pin) {
            const auto direction = component.pinDirection(pin);
            if (direction == PinDirection::Unused || direction == PinDirection::Power)
                continue;
            node.pins.push_back(
                {pin, directions.at(static_cast<std::size_t>(direction)), component.compute(pin)});
        }
        node.display = component.visualization();
        graph.nodes.push_back(std::move(node));
    }
    for (const auto &link : circuit.links())
        graph.edges.push_back({link.first, link.firstPin, link.second, link.secondPin});
    return graph;
}

GraphCallbacks circuitCallbacks(Circuit &circuit) {
    return {[&circuit] { return captureCircuit(circuit); }, [&circuit] { circuit.simulate(); },
            [&circuit](const std::string &name, Tristate value) { circuit.setInput(name, value); }};
}

} // namespace nts::bonus
