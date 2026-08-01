#pragma once
#include "IComponent.hpp"
#include <memory>
#include <string>

namespace nts {

class ComponentFactory {
  public:
    [[nodiscard]] std::unique_ptr<IComponent> createComponent(const std::string &type);
};

[[nodiscard]] std::unique_ptr<IComponent> createComponent(const std::string &type);

} // namespace nts
