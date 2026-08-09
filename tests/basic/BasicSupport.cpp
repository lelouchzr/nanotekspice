#include "BasicSupport.hpp"
#include "nts/Error.hpp"
#include <stdexcept>

namespace nts::test {

int checks = 0;

void require(bool condition, const std::string &message) {
    ++checks;
    if (!condition)
        throw std::runtime_error(message);
}

void rejected(const std::function<void()> &operation, const std::string &message) {
    bool caught = false;
    try {
        operation();
    } catch (const Error &) {
        caught = true;
    }
    require(caught, message);
}

void tick(const std::vector<IComponent *> &components, std::size_t number) {
    for (auto *component : components)
        component->beginTick(number);
    for (auto *component : components)
        component->prepare(number);
    for (auto *component : components)
        component->simulate(number);
    for (auto *component : components)
        component->endTick(number);
}

DriveProbe::DriveProbe(Drive value) : Component("test-driver", 1), value_(value) {
    definePin(1, PinDirection::Output);
}

void DriveProbe::value(Drive value) {
    value_ = value;
    invalidate();
}

Drive DriveProbe::evaluate(std::size_t) {
    return value_;
}

BidirectionalProbe::BidirectionalProbe(Drive value)
    : Component("test-bidirectional", 2), value_(value) {
    definePin(1, PinDirection::Bidirectional);
    definePin(2, PinDirection::Output);
}

Drive BidirectionalProbe::evaluate(std::size_t pin) {
    return pin == 1 ? value_ : toDrive(read(1));
}

CountedBuffer::CountedBuffer() : Component("test-buffer", 2) {
    definePin(1, PinDirection::Input);
    definePin(2, PinDirection::Output);
}

int CountedBuffer::evaluations() const noexcept {
    return evaluations_;
}

Drive CountedBuffer::evaluate(std::size_t) {
    ++evaluations_;
    return toDrive(read(1));
}

SampledLatch::SampledLatch() : Component("test-latch", 2) {
    definePin(1, PinDirection::Input);
    definePin(2, PinDirection::Output);
}

Drive SampledLatch::evaluate(std::size_t) {
    return toDrive(value_);
}

void SampledLatch::onPrepare(std::size_t) {
    pending_ = read(1);
}

void SampledLatch::onSimulate(std::size_t) {
    value_ = pending_;
}

} // namespace nts::test
