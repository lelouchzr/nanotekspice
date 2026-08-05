#include "nts/ComponentFactory.hpp"
#include "../advanced/AdvancedFactory.hpp"
#include "../basic/BasicFactory.hpp"
#include "nts/Error.hpp"
#ifdef NTS_BONUS
#include "BonusFactory.hpp"
#endif

namespace nts {

std::unique_ptr<IComponent> ComponentFactory::createComponent(const std::string &type) {
    if (auto result = createBasicComponent(type))
        return result;
    if (auto result = createAdvancedComponent(type))
        return result;
#ifdef NTS_BONUS
    if (auto result = createBonusComponent(type))
        return result;
#endif
    std::string message = "unknown component type: ";
    message.append(type);
    throw ComponentError(message);
}

std::unique_ptr<IComponent> createComponent(const std::string &type) {
    return ComponentFactory{}.createComponent(type);
}

} // namespace nts
