#include "Support.hpp"
#include <algorithm>
#include <array>

namespace advanced_test {
namespace {
void cascade(bool reversed) {
    Rig first("4013"), second("4013");
    Source clock;
    clock.set(nts::Drive::Low);
    clock.setLink(1, first.chip(), 3);
    clock.setLink(1, second.chip(), 3);
    first.chip().setLink(1, second.chip(), 5);
    first.set(4, nts::True);
    first.set(6, nts::False);
    second.set(4, nts::True);
    second.set(6, nts::False);
    // Reset with Rig helpers, then sample the shared clock through the global phases.
    first.tick();
    second.tick();
    first.set(4, nts::False);
    second.set(4, nts::False);
    first.set(5, nts::True);
    std::array<nts::IComponent *, 3> components{&first.chip(), &second.chip(), &clock};
    if (reversed)
        std::reverse(components.begin(), components.end());
    advanced_test::advance(components, 100);
    clock.set(nts::Drive::High);
    advanced_test::advance(components, 101);
    require(first.out(1) == nts::True && second.out(1) == nts::False,
            "4013 simultaneous cascade sampled newly committed upstream state");
}
} // namespace

void flipflops() {
    Rig rig("4013");
    for (const auto pin : {3U, 11U, 6U, 8U})
        rig.set(pin, nts::False);
    for (const auto pin : {4U, 10U})
        rig.set(pin, nts::True);
    rig.set(5, nts::True);
    rig.set(9, nts::False);
    rig.tick();
    require(rig.out(1) == nts::False && rig.out(2) == nts::True && rig.out(13) == nts::False,
            "4013 asynchronous resets must affect both halves");
    rig.set(4, nts::False);
    rig.set(10, nts::False);
    rig.tick();
    rig.set(3, nts::True);
    rig.set(11, nts::True);
    rig.tick();
    require(rig.out(1) == nts::True && rig.out(2) == nts::False && rig.out(13) == nts::False &&
                rig.out(12) == nts::True,
            "4013 positive edge must sample independent D inputs");
    rig.set(5, nts::False);
    rig.tick();
    require(rig.out(1) == nts::True, "4013 high level must not retrigger");
    rig.set(4, nts::True);
    rig.set(6, nts::True);
    rig.tick();
    require(rig.out(1) == nts::True && rig.out(2) == nts::True,
            "4013 simultaneous SET/RESET must drive both outputs high");
    rig.set(4, nts::False);
    rig.set(6, nts::False);
    rig.set(3, nts::False);
    rig.tick();
    rig.set(5, nts::Undefined);
    rig.set(3, nts::True);
    rig.tick();
    require(rig.out(1) == nts::Undefined && rig.out(2) == nts::Undefined,
            "4013 captured unknown data must remain unknown");
    cascade(false);
    cascade(true);
}
} // namespace advanced_test
