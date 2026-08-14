#include "nts/Component.hpp"
#include "nts/Error.hpp"
#include <algorithm>
#include <utility>

namespace nts {
namespace {

std::size_t checkedCount(std::size_t pins) {
    if (pins == 0 || pins > 65536)
        throw ComponentError("component pin count must be between 1 and 65536");
    return pins + 1;
}

} // namespace

std::atomic<std::uint64_t> Component::epoch_{1};
thread_local std::size_t Component::depth_ = 0;

Component::Component(std::string type, std::size_t pins)
    : type_(std::move(type)), pins_(checkedCount(pins)) {}

Component::~Component() {
    for (std::size_t pin = 1; pin < pins_.size(); ++pin)
        if (pins_[pin].net) {
            auto &endpoints = pins_[pin].net->endpoints;
            std::erase_if(endpoints,
                          [this](const auto &endpoint) { return endpoint.component == this; });
        }
    invalidate();
}

ComponentPin &Component::checked(std::size_t pin) {
    if (pin == 0 || pin >= pins_.size())
        throw PinError(type_ + ": pin out of range: " + std::to_string(pin));
    return pins_[pin];
}

const ComponentPin &Component::checked(std::size_t pin) const {
    if (pin == 0 || pin >= pins_.size())
        throw PinError(type_ + ": pin out of range: " + std::to_string(pin));
    return pins_[pin];
}

std::size_t Component::pinCount() const noexcept {
    return pins_.size() - 1;
}

PinDirection Component::pinDirection(std::size_t pin) const {
    return checked(pin).direction;
}

std::string_view Component::type() const noexcept {
    return type_;
}

void Component::definePin(std::size_t pin, PinDirection direction) {
    auto &target = checked(pin);
    if (target.net)
        throw LinkError("cannot redefine an already linked pin");
    target.direction = direction;
}

void Component::invalidate() noexcept {
    epoch_.fetch_add(1, std::memory_order_relaxed);
}

void Component::refresh() noexcept {
    const auto current = epoch_.load(std::memory_order_relaxed);
    if (epochSeen_ == current)
        return;
    for (auto &pin : pins_)
        pin.cached.reset();
    epochSeen_ = current;
}

void Component::onBeginTick(std::size_t) {}

void Component::onPrepare(std::size_t) {}

void Component::onSimulate(std::size_t) {}

void Component::onEndTick(std::size_t) {}

void Component::beginTick(std::size_t tick) {
    if (begun_ == tick)
        return;
    begun_ = tick;
    prepared_.reset();
    invalidate();
    onBeginTick(tick);
}

void Component::prepare(std::size_t tick) {
    if (prepared_ == tick)
        return;
    beginTick(tick);
    onPrepare(tick);
    prepared_ = tick;
}

void Component::simulate(std::size_t tick) {
    if (committed_ == tick)
        return;
    const bool standalone = prepared_ != tick;
    prepare(tick);
    onSimulate(tick);
    committed_ = tick;
    if (standalone)
        endTick(tick);
}

void Component::endTick(std::size_t tick) {
    onEndTick(tick);
    invalidate();
}

} // namespace nts
