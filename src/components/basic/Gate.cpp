#include "Gate.hpp"
#include "nts/Error.hpp"
#include <algorithm>
#include <utility>

namespace nts {

Gate::Gate(std::string type, std::size_t pins, std::vector<GateFunction> functions, bool powered)
    : Component(std::move(type), pins), functions_(std::move(functions)) {
    if (powered) {
        definePin(7, PinDirection::Power);
        definePin(14, PinDirection::Power);
    }
    for (const auto &function : functions_) {
        definePin(function.first, PinDirection::Input);
        if (function.operation != GateOperation::Not)
            definePin(function.second, PinDirection::Input);
        definePin(function.output, PinDirection::Output);
    }
}

Tristate Gate::apply(const GateFunction &function) {
    const auto left = read(function.first);
    if (function.operation == GateOperation::Not)
        return logicNot(left);
    const auto right = read(function.second);
    switch (function.operation) {
    case GateOperation::And:
        return logicAnd(left, right);
    case GateOperation::Or:
        return logicOr(left, right);
    case GateOperation::Xor:
        return logicXor(left, right);
    case GateOperation::Nand:
        return logicNot(logicAnd(left, right));
    case GateOperation::Nor:
        return logicNot(logicOr(left, right));
    case GateOperation::Not:
        break;
    }
    return Undefined;
}

Drive Gate::evaluate(std::size_t pin) {
    const auto found = std::find_if(functions_.begin(), functions_.end(),
                                    [pin](const auto &function) { return function.output == pin; });
    return found == functions_.end() ? Drive::Unknown : toDrive(apply(*found));
}

} // namespace nts
