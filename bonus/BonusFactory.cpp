#include "BonusFactory.hpp"
#include "components/Cpu.hpp"
#include "components/Matrix.hpp"
#include "components/SevenSegment.hpp"

namespace nts {

std::unique_ptr<IComponent> createBonusComponent(const std::string &type) {
    if (type == "7seg")
        return std::make_unique<bonus::SevenSegment>();
    if (type == "matrix")
        return std::make_unique<bonus::Matrix>();
    if (type == "subleq" || type == "cpu")
        return std::make_unique<bonus::Cpu>();
    return nullptr;
}

} // namespace nts
