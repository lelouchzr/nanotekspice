#include "nts/Circuit.hpp"
#include "nts/ComponentFactory.hpp"
#include "nts/Error.hpp"
#include <limits>

namespace nts {

void Circuit::add(const std::string &type, const std::string &name) {
    if (nodes_.contains(name))
        throw Error("duplicate component name '" + name + "'");
    if (nodes_.size() >= 100000)
        throw Error("circuit exceeds 100000 components");
    auto value = createComponent(type);
    if (!value)
        throw Error("unknown component type '" + type + "'");
    const auto position = nodes_.emplace(name, Node{type, std::move(value)}).first;
    try {
        if (position->second.component->hasAsynchronousState())
            asynchronous_.push_back(position->second.component.get());
    } catch (...) {
        nodes_.erase(position);
        throw;
    }
}

IComponent &Circuit::component(const std::string &name) const {
    const auto found = nodes_.find(name);
    if (found == nodes_.end())
        throw Error("unknown component name '" + name + "'");
    return *found->second.component;
}

void Circuit::connect(const Link &link) {
    if (links_.size() >= 500000)
        throw Error("circuit exceeds 500000 links");
    component(link.first).setLink(link.firstPin, component(link.second), link.secondPin);
    links_.push_back(link);
}

void Circuit::setLayout(const std::string &name, LayoutPosition position) {
    (void)component(name);
    if (!layout_.emplace(name, position).second)
        throw Error("duplicate graphic position for '" + name + "'");
}

void Circuit::setInput(const std::string &name, Tristate value) {
    auto &source = component(name);
    if (!source.isInputSource())
        throw Error("component '" + name + "' is not an input or clock");
    if (value != Undefined && value != False && value != True)
        throw Error("input value must be 0, 1 or U");
    source.setInput(value);
}

void Circuit::simulate() {
    if (tick_ == std::numeric_limits<std::size_t>::max())
        throw Error("simulation tick counter exhausted");
    const auto next = tick_ + 1;
    // Sample every sequential component before any state is committed.
    for (const auto &[name, node] : nodes_) {
        (void)name;
        node.component->beginTick(next);
    }
    settleAsynchronous();
    for (const auto &[name, node] : nodes_) {
        (void)name;
        node.component->prepare(next);
    }
    for (const auto &[name, node] : nodes_) {
        (void)name;
        node.component->simulate(next);
    }
    for (const auto &[name, node] : nodes_) {
        (void)name;
        node.component->endTick(next);
    }
    settleAsynchronous();
    tick_ = next;
}

void Circuit::settleAsynchronous() {
    // A register output may assert its own reset through combinational feedback.
    // Settle that state before sampling and after committing, without clocking again.
    for (std::size_t pass = 0; pass < 1024; ++pass) {
        bool changed = false;
        for (auto *component : asynchronous_) {
            if (component->settle())
                changed = true;
        }
        if (!changed)
            return;
    }
    throw Error("asynchronous circuit did not settle within 1024 passes");
}

std::size_t Circuit::tick() const noexcept {
    return tick_;
}

const std::map<std::string, Node, std::less<>> &Circuit::nodes() const noexcept {
    return nodes_;
}

const std::vector<Link> &Circuit::links() const noexcept {
    return links_;
}

const std::map<std::string, LayoutPosition, std::less<>> &Circuit::layout() const noexcept {
    return layout_;
}

} // namespace nts
