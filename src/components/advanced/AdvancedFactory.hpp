#pragma once
#include "nts/IComponent.hpp"
#include <memory>
#include <string>

namespace nts {
std::unique_ptr<IComponent> createAdvancedComponent(const std::string &type);
}
