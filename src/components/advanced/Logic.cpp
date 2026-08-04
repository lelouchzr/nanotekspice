#include "nts/components/advanced/Logic.hpp"

namespace nts::advanced {
Tristate merge(Tristate left, Tristate right) noexcept {
    return left == right ? left : Undefined;
}

bool permits(Tristate value, bool candidate) noexcept {
    return value == Undefined || (value == True) == candidate;
}

Tristate edge(Tristate previous, Tristate current, bool rising) noexcept {
    const Tristate before = rising ? False : True;
    const Tristate after = rising ? True : False;
    if (previous == before && current == after)
        return True;
    if ((previous != Undefined && previous != before) || (current != Undefined && current != after))
        return False;
    return Undefined;
}

bool matchesAddress(std::span<const Tristate> bits, std::size_t address) noexcept {
    for (std::size_t bit = 0; bit < bits.size(); ++bit)
        if (!permits(bits[bit], ((address >> bit) & 1U) != 0))
            return false;
    return true;
}

Tristate decodeAddress(std::span<const Tristate> bits, std::size_t address) noexcept {
    if (!matchesAddress(bits, address))
        return False;
    for (const auto bit : bits)
        if (bit == Undefined)
            return Undefined;
    return True;
}
} // namespace nts::advanced
