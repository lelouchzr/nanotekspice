#include "nts/Component.hpp"
#include "nts/Error.hpp"

namespace nts {

Drive Component::drive(std::size_t pin) {
    auto &target = checked(pin);
    if (target.direction != PinDirection::Output && target.direction != PinDirection::Bidirectional)
        return Drive::HighImpedance;
    refresh();
    if (target.cached)
        return *target.cached;
    if (target.evaluating) {
        ++cycleVersion_;
        return Drive::Unknown;
    }
    if (depth_ >= 512)
        throw ComponentError("evaluation exceeds the safety limit of 512 recursive components");
    const auto cycleBefore = cycleVersion_;
    target.evaluating = true;
    ++depth_;
    try {
        const auto value = evaluate(pin);
        // A cycle break is provisional. Another branch can still establish a known
        // controlling value in this phase, so never memoize its propagated unknown.
        if (value != Drive::Unknown || cycleBefore == cycleVersion_)
            target.cached = value;
        target.evaluating = false;
        --depth_;
        return value;
    } catch (...) {
        target.evaluating = false;
        --depth_;
        throw;
    }
}

Drive Component::resolve(std::size_t pin, bool includeSelf) {
    const auto &target = checked(pin);
    if (!target.net)
        return includeSelf ? drive(pin) : Drive::HighImpedance;
    Drive result = Drive::HighImpedance;
    for (const auto &endpoint : target.net->drivers) {
        if (!includeSelf && endpoint.component == this && endpoint.pin == pin)
            continue;
        const auto value = endpoint.component->drive(endpoint.pin);
        if (value == Drive::HighImpedance)
            continue;
        if (value == Drive::Unknown || (result != Drive::HighImpedance && result != value))
            return Drive::Unknown;
        result = value;
    }
    return result;
}

Tristate Component::read(std::size_t pin) {
    return toTristate(resolve(pin, false));
}

Tristate Component::compute(std::size_t pin) {
    const auto direction = checked(pin).direction;
    if (direction == PinDirection::Power || direction == PinDirection::Unused)
        return Undefined;
    return toTristate(resolve(pin, true));
}

} // namespace nts
