#include "Support.hpp"
#include "cpu/Integer.hpp"
#include <cstdint>
#include <limits>

namespace nts::bonus::test {

void integerTests() {
    require(Integer("-000").string() == "0", "integer negative zero normalization");
    require(Integer("+00017").string() == "17", "integer positive normalization");
    for (std::int64_t a = -80; a <= 80; ++a)
        for (std::int64_t b = -80; b <= 80; ++b) {
            const Integer first(a), second(b);
            require((first + second).string() == std::to_string(a + b), "integer signed addition");
            require((first - second).string() == std::to_string(a - b),
                    "integer signed subtraction");
            require((first < second) == (a < b), "integer ordering");
            require((first <= second) == (a <= b), "integer inclusive ordering");
        }
    const Integer huge("999999999999999999999999999999999999999999999999999999999999");
    require((huge + Integer(1)).string() ==
                "1000000000000000000000000000000000000000000000000000000000000",
            "integer carry beyond fixed machine words");
    require((huge - huge).zero(), "integer cancellation");
    auto alias = huge;
    alias += alias;
    require(alias == huge + huge, "integer self-aliasing addition");
    alias -= alias;
    require(alias.zero(), "integer self-aliasing subtraction");
    require(Integer(std::numeric_limits<std::int64_t>::min()).string() == "-9223372036854775808",
            "integer minimum native value");
    require(Integer(-1).lowBits(8) == 255 && Integer("4294967297").lowBits(12) == 1,
            "integer modular output ports");
    for (const std::string value : {"", "+", "-", "1.2", " 1", "1x"})
        rejects([&] { (void)Integer(value); }, "malformed integer accepted");
    rejects([&] { (void)huge.lowBits(32); }, "unsafe port bit count accepted");
}

} // namespace nts::bonus::test
