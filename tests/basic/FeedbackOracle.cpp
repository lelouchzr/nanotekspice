#include "BasicSupport.hpp"
#include "nts/ComponentFactory.hpp"
#include <array>
#include <memory>

namespace nts::test {
namespace {
std::array<Tristate, 2> reference(Tristate set, Tristate reset) {
    // Enumerate Boolean fixed points independently of the simulator and its gate helpers.
    unsigned possibleA = 0, possibleB = 0;
    for (int s = 0; s < 2; ++s)
        for (int r = 0; r < 2; ++r) {
            if ((set != Undefined && static_cast<int>(set) != s) ||
                (reset != Undefined && static_cast<int>(reset) != r))
                continue;
            for (int a = 0; a < 2; ++a)
                for (int b = 0; b < 2; ++b)
                    if (a == !(b || s) && b == !(a || r)) {
                        possibleA |= 1U << static_cast<unsigned>(a);
                        possibleB |= 1U << static_cast<unsigned>(b);
                    }
        }
    auto merge = [](unsigned mask) { return mask == 1 ? False : mask == 2 ? True : Undefined; };
    return {merge(possibleA), merge(possibleB)};
}
} // namespace

void feedbackOracleTests() {
    for (auto s : {False, True, Undefined})
        for (auto r : {False, True, Undefined})
            for (bool reverse : {false, true})
                for (bool swapA : {false, true})
                    for (bool swapB : {false, true}) {
                        auto set = createComponent("input"), reset = createComponent("input"),
                             gate = createComponent("4001");
                        set->setLink(1, *gate, swapA ? 1 : 2);
                        reset->setLink(1, *gate, swapB ? 5 : 6);
                        gate->setLink(4, *gate, swapA ? 2 : 1);
                        gate->setLink(3, *gate, swapB ? 6 : 5);
                        set->setInput(s);
                        reset->setInput(r);
                        tick({set.get(), reset.get(), gate.get()}, 1);
                        const auto expected = reference(s, r);
                        for (int repetition = 0; repetition < 3; ++repetition) {
                            const auto first = gate->compute(reverse ? 4 : 3);
                            const auto second = gate->compute(reverse ? 3 : 4);
                            require(first == expected[reverse ? 1U : 0U] &&
                                        second == expected[reverse ? 0U : 1U],
                                    "cross-coupled NOR outputs differ from Boolean fixed-point "
                                    "oracle or depend on read order");
                        }
                    }
}
} // namespace nts::test
