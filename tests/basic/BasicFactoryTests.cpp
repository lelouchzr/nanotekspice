#include "BasicSupport.hpp"
#include "nts/ComponentFactory.hpp"
#include <array>
#include <type_traits>

namespace nts::test {

void factoryTests() {
    using Creator = std::unique_ptr<IComponent> (ComponentFactory::*)(const std::string &);
    static_assert(std::is_same_v<decltype(&ComponentFactory::createComponent), Creator>);
    ComponentFactory factory;
    const std::array names{"input", "output", "true", "false", "clock", "and",  "or",  "xor",
                           "not",   "4001",   "4011", "4030",  "4069",  "4071", "4081"};
    for (const auto *name : names) {
        auto component = factory.createComponent(name);
        require(component && component->type() == name && component->pinCount() > 0,
                "factory returned incorrect component metadata");
    }
    rejected([&] { static_cast<void>(factory.createComponent("unknown")); },
             "unknown factory type accepted");
    rejected([&] { static_cast<void>(createComponent("")); }, "empty factory type accepted");
    require(toString(Undefined) == "U" && toString(False) == "0" && toString(True) == "1",
            "display tristate formatting");
}

} // namespace nts::test
