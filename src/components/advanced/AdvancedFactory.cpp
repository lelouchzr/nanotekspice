#include "AdvancedFactory.hpp"
#include "nts/components/advanced/Adder4008.hpp"
#include "nts/components/advanced/Counter4017.hpp"
#include "nts/components/advanced/Counter4040.hpp"
#include "nts/components/advanced/Decoder4514.hpp"
#include "nts/components/advanced/FlipFlop4013.hpp"
#include "nts/components/advanced/Logger.hpp"
#include "nts/components/advanced/Ram4801.hpp"
#include "nts/components/advanced/Rom2716.hpp"
#include "nts/components/advanced/Selector4512.hpp"
#include "nts/components/advanced/Shift4094.hpp"

namespace nts {
std::unique_ptr<IComponent> createAdvancedComponent(const std::string &type) {
    if (type == "4008")
        return std::make_unique<Adder4008>();
    if (type == "4013")
        return std::make_unique<FlipFlop4013>();
    if (type == "4017")
        return std::make_unique<Counter4017>();
    if (type == "4040")
        return std::make_unique<Counter4040>();
    if (type == "4094")
        return std::make_unique<Shift4094>();
    if (type == "4512")
        return std::make_unique<Selector4512>();
    if (type == "4514")
        return std::make_unique<Decoder4514>();
    if (type == "4801")
        return std::make_unique<Ram4801>();
    if (type == "2716")
        return std::make_unique<Rom2716>();
    if (type == "logger")
        return std::make_unique<Logger>();
    return nullptr;
}
} // namespace nts
