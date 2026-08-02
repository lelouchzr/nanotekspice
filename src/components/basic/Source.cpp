#include "Source.hpp"
#include "nts/Error.hpp"
#include <utility>

namespace nts {

Source::Source(std::string type, SourceMode mode, Tristate value)
    : Component(std::move(type), 1), mode_(mode), value_(value) {
    definePin(1, mode == SourceMode::Output ? PinDirection::Input : PinDirection::Output);
}

bool Source::isInputSource() const noexcept {
    return mode_ == SourceMode::Input || mode_ == SourceMode::Clock;
}

void Source::setInput(Tristate value) {
    if (!isInputSource())
        throw ComponentError("only input and clock components accept assignment");
    if (value != Undefined && value != False && value != True)
        throw ComponentError("input must be 0, 1 or U");
    pending_ = value;
}

Tristate Source::currentInput() const {
    if (!isInputSource())
        throw ComponentError("component is not an input or clock");
    return value_;
}

Drive Source::evaluate(std::size_t) {
    return toDrive(value_);
}

void Source::onBeginTick(std::size_t) {
    if (pending_) {
        value_ = *pending_;
        pending_.reset();
    } else if (mode_ == SourceMode::Clock)
        value_ = logicNot(value_);
}

} // namespace nts
