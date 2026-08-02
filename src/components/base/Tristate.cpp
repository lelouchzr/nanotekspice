#include "nts/Tristate.hpp"

namespace nts {

Tristate logicNot(Tristate value) noexcept {
    return value == Undefined ? Undefined : value == True ? False : True;
}

Tristate logicAnd(Tristate left, Tristate right) noexcept {
    if (left == False || right == False)
        return False;
    return left == True && right == True ? True : Undefined;
}

Tristate logicOr(Tristate left, Tristate right) noexcept {
    if (left == True || right == True)
        return True;
    return left == False && right == False ? False : Undefined;
}

Tristate logicXor(Tristate left, Tristate right) noexcept {
    return left == Undefined || right == Undefined ? Undefined : left == right ? False : True;
}

Drive toDrive(Tristate value) noexcept {
    return value == False ? Drive::Low : value == True ? Drive::High : Drive::Unknown;
}

Tristate toTristate(Drive value) noexcept {
    return value == Drive::Low ? False : value == Drive::High ? True : Undefined;
}

std::string_view toString(Tristate value) noexcept {
    return value == False ? "0" : value == True ? "1" : "U";
}

} // namespace nts
