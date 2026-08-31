#include "nts/IComponent.hpp"
#include "nts/Error.hpp"

namespace nts {

void IComponent::beginTick(std::size_t) {}

void IComponent::prepare(std::size_t) {}

void IComponent::endTick(std::size_t) {}

bool IComponent::hasAsynchronousState() const noexcept {
    return false;
}

bool IComponent::settle() {
    return false;
}

Drive IComponent::drive(std::size_t pin) {
    return toDrive(compute(pin));
}

bool IComponent::isInputSource() const noexcept {
    return false;
}

void IComponent::setInput(Tristate) {
    throw ComponentError("component does not accept input assignment");
}

Tristate IComponent::currentInput() const {
    throw ComponentError("component is not an input source");
}

#ifdef NTS_BONUS
std::optional<bonus::DisplayFrame> IComponent::visualization() const {
    return std::nullopt;
}

void IComponent::loadProgram(const std::filesystem::path &) {
    throw ComponentError("component does not accept a program image");
}
#endif

} // namespace nts
