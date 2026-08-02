#include "BasicFactory.hpp"
#include "Gate.hpp"
#include "Source.hpp"
#include <array>

namespace nts {
namespace {

std::vector<GateFunction> quad(GateOperation operation) {
    return {
        {1, 2, 3, operation}, {5, 6, 4, operation}, {8, 9, 10, operation}, {12, 13, 11, operation}};
}

std::unique_ptr<IComponent> packaged(const std::string &type) {
    const std::array<std::pair<const char *, GateOperation>, 5> kinds{
        {{"4001", GateOperation::Nor},
         {"4011", GateOperation::Nand},
         {"4030", GateOperation::Xor},
         {"4071", GateOperation::Or},
         {"4081", GateOperation::And}}};
    for (const auto &[name, operation] : kinds)
        if (type == name)
            return std::make_unique<Gate>(type, 14, quad(operation), true);
    if (type == "4069")
        return std::make_unique<Gate>(type, 14,
                                      std::vector<GateFunction>{{1, 0, 2, GateOperation::Not},
                                                                {3, 0, 4, GateOperation::Not},
                                                                {5, 0, 6, GateOperation::Not},
                                                                {9, 0, 8, GateOperation::Not},
                                                                {11, 0, 10, GateOperation::Not},
                                                                {13, 0, 12, GateOperation::Not}},
                                      true);
    return nullptr;
}

} // namespace

std::unique_ptr<IComponent> createBasicComponent(const std::string &type) {
    if (type == "input" || type == "clock")
        return std::make_unique<Source>(type,
                                        type == "clock" ? SourceMode::Clock : SourceMode::Input);
    if (type == "true" || type == "false")
        return std::make_unique<Source>(type, SourceMode::Constant, type == "true" ? True : False);
    if (type == "output")
        return std::make_unique<Source>(type, SourceMode::Output);
    const std::array kinds{std::pair{"and", GateOperation::And}, std::pair{"or", GateOperation::Or},
                           std::pair{"xor", GateOperation::Xor}};
    for (const auto &[name, operation] : kinds)
        if (type == name)
            return std::make_unique<Gate>(type, 3, std::vector<GateFunction>{{1, 2, 3, operation}});
    if (type == "not")
        return std::make_unique<Gate>(type, 2,
                                      std::vector<GateFunction>{{1, 0, 2, GateOperation::Not}});
    return packaged(type);
}

} // namespace nts
