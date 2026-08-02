#pragma once
#include "nts/IComponent.hpp"
#include <memory>
#include <string>

namespace nts {

[[nodiscard]] std::unique_ptr<IComponent> createBasicComponent(const std::string &type);

} // namespace nts
