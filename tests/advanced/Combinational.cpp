#include "Support.hpp"
#include "components/advanced/AdvancedFactory.hpp"
#include <array>

namespace advanced_test {
namespace {
void adder() {
    Rig rig("4008");
    constexpr std::array<std::size_t, 4> a{7, 5, 3, 1}, b{6, 4, 2, 15};
    for (std::size_t left = 0; left < 16; ++left)
        for (std::size_t right = 0; right < 16; ++right)
            for (std::size_t carry = 0; carry < 2; ++carry) {
                rig.bus(a, left);
                rig.bus(b, right);
                rig.set(9, carry != 0 ? nts::True : nts::False);
                rig.tick();
                const auto expected = left + right + carry;
                for (std::size_t bit = 0; bit < 5; ++bit)
                    require(rig.out(10 + bit) ==
                                (((expected >> bit) & 1U) != 0 ? nts::True : nts::False),
                            "4008 exhaustive addition mismatch");
            }
    rig.bus(a, 0);
    rig.bus(b, 0);
    rig.set(9, nts::Undefined);
    rig.tick();
    require(rig.out(10) == nts::Undefined && rig.out(11) == nts::False && rig.out(14) == nts::False,
            "4008 unknown carry must not contaminate known higher carry");
    rig.set(7, nts::Undefined);
    rig.set(6, nts::True);
    rig.set(9, nts::True);
    rig.tick();
    require(rig.out(10) == nts::Undefined && rig.out(11) == nts::True,
            "4008 majority carry must preserve two known ones");
}

void selector() {
    Rig rig("4512");
    constexpr std::array<std::size_t, 8> pins{1, 2, 3, 4, 5, 6, 7, 9};
    constexpr std::array<std::size_t, 3> address{11, 12, 13};
    rig.set(15, nts::False);
    rig.set(10, nts::False);
    for (std::size_t data = 0; data < 256; ++data)
        for (std::size_t index = 0; index < 8; ++index) {
            rig.bus(pins, data);
            rig.bus(address, index);
            rig.tick();
            require(rig.out(14) == (((data >> index) & 1U) != 0 ? nts::True : nts::False),
                    "4512 exhaustive selection mismatch");
        }
    rig.bus(pins, 0xff);
    rig.set(11, nts::Undefined);
    rig.tick();
    require(rig.out(14) == nts::True, "4512 equal alternatives resolve despite unknown select");
    rig.set(10, nts::True);
    rig.tick();
    require(rig.out(14) == nts::False, "4512 inhibit must force zero");
    rig.set(15, nts::True);
    rig.tick();
    require(rig.chip().drive(14) == nts::Drive::HighImpedance, "4512 OE is active low");
}

void decoder() {
    Rig rig("4514");
    constexpr std::array<std::size_t, 4> address{2, 3, 21, 22};
    constexpr std::array<std::size_t, 16> outputs{11, 9,  10, 8,  7,  6,  5,  4,
                                                  18, 17, 20, 19, 14, 13, 16, 15};
    rig.set(1, nts::True);
    rig.set(23, nts::False);
    for (std::size_t value = 0; value < 16; ++value) {
        rig.bus(address, value);
        rig.tick();
        for (std::size_t bit = 0; bit < 16; ++bit)
            require(rig.out(outputs[bit]) == (bit == value ? nts::True : nts::False),
                    "4514 one-hot decode mismatch");
    }
    rig.set(1, nts::False);
    rig.bus(address, 0);
    rig.tick();
    require(rig.out(15) == nts::True && rig.out(11) == nts::False,
            "4514 falling strobe holds address");
    rig.set(1, nts::True);
    rig.set(2, nts::Undefined);
    rig.tick();
    require(rig.out(11) == nts::Undefined && rig.out(9) == nts::Undefined &&
                rig.out(10) == nts::False,
            "4514 unknown address preserves impossible outputs low");
    rig.set(23, nts::True);
    rig.tick();
    for (const auto pin : outputs)
        require(rig.out(pin) == nts::False, "4514 inhibit dominates unknown address");
}
} // namespace

void combinational() {
    require(nts::createAdvancedComponent("missing") == nullptr,
            "unknown advanced factory type must return nullptr");
    adder();
    selector();
    decoder();
}
} // namespace advanced_test
