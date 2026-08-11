#include "Support.hpp"
#include <array>

namespace advanced_test {
void counters() {
    Rig decade("4017");
    constexpr std::array<std::size_t, 10> outputs{3, 2, 4, 7, 10, 1, 5, 6, 9, 11};
    decade.set(14, nts::False);
    decade.set(13, nts::False);
    decade.set(15, nts::True);
    decade.tick();
    require(decade.out(3) == nts::True && decade.out(12) == nts::True,
            "4017 reset selects zero and carry");
    decade.set(15, nts::False);
    for (std::size_t count = 1; count <= 20; ++count) {
        decade.set(14, nts::False);
        decade.tick();
        decade.set(14, nts::True);
        decade.tick();
        for (std::size_t bit = 0; bit < outputs.size(); ++bit)
            require(decade.out(outputs[bit]) == (bit == count % 10 ? nts::True : nts::False),
                    "4017 count or pin mapping incorrect");
        require(decade.out(12) == (count % 10 < 5 ? nts::True : nts::False),
                "4017 divide-ten carry mismatch");
    }
    decade.set(13, nts::True);
    decade.tick();
    require(decade.out(3) == nts::True, "4017 CP1 rising must inhibit without counting");
    decade.set(13, nts::False);
    decade.tick();
    require(decade.out(2) == nts::True, "4017 CP1 falling while CP0 high must count");
    decade.set(15, nts::Undefined);
    decade.tick();
    require(decade.out(3) == nts::Undefined && decade.out(2) == nts::Undefined &&
                decade.out(4) == nts::False,
            "4017 unknown reset must merge possible states");

    Rig binary("4040");
    constexpr std::array<std::size_t, 12> bits{9, 7, 6, 5, 3, 2, 4, 13, 12, 14, 15, 1};
    binary.set(10, nts::True);
    binary.set(11, nts::True);
    binary.tick();
    binary.set(11, nts::False);
    binary.tick();
    for (std::size_t count = 1; count <= 4096; ++count) {
        binary.set(10, nts::False);
        binary.tick();
        for (std::size_t bit = 0; bit < bits.size(); ++bit)
            require(binary.out(bits[bit]) == (((count >> bit) & 1U) != 0 ? nts::True : nts::False),
                    "4040 exhaustive count or pin mapping incorrect");
        binary.set(10, nts::True);
        binary.tick();
    }
    binary.set(10, nts::Undefined);
    binary.tick();
    require(binary.out(9) == nts::Undefined && binary.out(7) == nts::False,
            "4040 unknown edge must preserve known higher zero bits");
    binary.set(11, nts::True);
    binary.tick();
    for (const auto pin : bits)
        require(binary.out(pin) == nts::False, "4040 reset must recover from unknown clock");
}
} // namespace advanced_test
