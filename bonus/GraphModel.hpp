#pragma once

#include "DisplayFrame.hpp"
#include "nts/Tristate.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace nts::bonus {

struct PinView {
    std::size_t number = 0;
    std::string direction;
    Tristate value = Undefined;
};

struct GraphNode {
    std::string name;
    std::string type;
    int x = 0;
    int y = 0;
    std::vector<PinView> pins;
    std::optional<DisplayFrame> display;
};

struct GraphEdge {
    std::string fromNode;
    std::size_t fromPin = 0;
    std::string toNode;
    std::size_t toPin = 0;
};

struct GraphModel {
    std::size_t tick = 0;
    std::vector<GraphNode> nodes;
    std::vector<GraphEdge> edges;
};

struct GraphCallbacks {
    std::function<GraphModel()> captureGraph;
    std::function<void()> step;
    std::function<void(const std::string &, Tristate)> setInput;
};

} // namespace nts::bonus
