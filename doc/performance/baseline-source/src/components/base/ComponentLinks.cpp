#include "nts/Component.hpp"
#include "nts/Error.hpp"
#include <algorithm>

namespace nts {

void Component::merge(Component &other, std::size_t pin, std::size_t otherPin) {
    auto &left = checked(pin).net;
    auto &right = other.checked(otherPin).net;
    if (!left) {
        left = std::make_shared<ComponentNet>();
        left->endpoints.push_back({this, pin});
    }
    if (!right) {
        right = std::make_shared<ComponentNet>();
        right->endpoints.push_back({&other, otherPin});
    }
    if (left == right)
        return;
    // Keep one shared node for the entire electrical net, including input-to-input links.
    const auto destination = left;
    const auto source = right;
    destination->endpoints.reserve(destination->endpoints.size() + source->endpoints.size());
    for (const auto &endpoint : source->endpoints) {
        destination->endpoints.push_back(endpoint);
        if (auto *component = dynamic_cast<Component *>(endpoint.component))
            component->checked(endpoint.pin).net = destination;
    }
    source->endpoints.clear();
}

void Component::setLink(std::size_t pin, IComponent &other, std::size_t otherPin) {
    const auto leftDirection = pinDirection(pin);
    const auto rightDirection = other.pinDirection(otherPin);
    if (leftDirection == PinDirection::Power || leftDirection == PinDirection::Unused ||
        rightDirection == PinDirection::Power || rightDirection == PinDirection::Unused)
        throw LinkError("power and unused pins cannot be linked");
    if (&other == this && pin == otherPin)
        return;
    if (auto *component = dynamic_cast<Component *>(&other)) {
        merge(*component, pin, otherPin);
        invalidate();
        return;
    }
    auto &net = checked(pin).net;
    if (!net) {
        net = std::make_shared<ComponentNet>();
        net->endpoints.push_back({this, pin});
    }
    const auto duplicate =
        std::find_if(net->endpoints.begin(), net->endpoints.end(), [&](const auto &endpoint) {
            return endpoint.component == &other && endpoint.pin == otherPin;
        });
    if (duplicate == net->endpoints.end()) {
        net->endpoints.push_back({&other, otherPin});
        other.setLink(otherPin, *this, pin);
    }
    invalidate();
}

} // namespace nts
