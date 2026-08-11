#include "Support.hpp"
#include <array>

namespace advanced_test {
void shiftAndLatch() {
    Rig rig("4094");
    constexpr std::array<std::size_t, 8> pins{4, 5, 6, 7, 14, 13, 12, 11};
    rig.set(15, nts::True);
    rig.set(1, nts::True);
    rig.set(3, nts::False);
    rig.tick();
    require(rig.out(9) == nts::Undefined, "4094 power-up shift state must be unknown");
    rig.set(2, nts::False);
    for (int clear = 0; clear < 8; ++clear) {
        rig.set(3, nts::False);
        rig.tick();
        rig.set(3, nts::True);
        rig.tick();
    }
    for (std::size_t index = 8; index-- > 0;) {
        rig.set(3, nts::False);
        rig.set(2, ((0xa5U >> index) & 1U) != 0 ? nts::True : nts::False);
        rig.tick();
        rig.set(3, nts::True);
        rig.tick();
    }
    for (std::size_t bit = 0; bit < pins.size(); ++bit)
        require(rig.out(pins[bit]) == (((0xa5U >> bit) & 1U) != 0 ? nts::True : nts::False),
                "4094 serial-to-parallel bit order incorrect");
    require(rig.out(9) == nts::True, "4094 QS exposes shifted eighth stage");
    require(rig.out(10) == nts::False, "4094 QS-prime must retain previous falling-edge sample");
    rig.set(3, nts::False);
    rig.tick();
    require(rig.out(10) == nts::True, "4094 QS-prime must sample on negative edge");
    rig.set(1, nts::False);
    rig.set(2, nts::False);
    for (int i = 0; i < 8; ++i) {
        rig.set(3, nts::False);
        rig.tick();
        rig.set(3, nts::True);
        rig.tick();
    }
    require(rig.out(9) == nts::False && rig.out(4) == nts::True && rig.out(11) == nts::True,
            "4094 low strobe must hold latch while shift register continues");
    rig.set(1, nts::True);
    rig.tick();
    for (const auto pin : pins)
        require(rig.out(pin) == nts::False,
                "4094 high strobe must transparently expose shift register");
    rig.set(15, nts::False);
    rig.tick();
    for (const auto pin : pins)
        require(rig.chip().drive(pin) == nts::Drive::HighImpedance,
                "4094 disabled outputs must float");
    require(rig.out(9) == nts::False, "4094 OE must not disable serial outputs");
}
} // namespace advanced_test
