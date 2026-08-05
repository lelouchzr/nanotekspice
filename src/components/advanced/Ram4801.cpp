#include "nts/components/advanced/Ram4801.hpp"
#include "nts/components/advanced/Logic.hpp"
#include <algorithm>

namespace nts {
namespace {
constexpr std::array<std::size_t, 10> addresses{8, 7, 6, 5, 4, 3, 2, 1, 23, 22};
constexpr std::array<std::size_t, 8> dataPins{9, 10, 11, 13, 14, 15, 16, 17};
} // namespace

Ram4801::Ram4801() : Component("4801", 24) {
    for (auto &byte : memory_)
        byte.fill(Undefined);
    pendingAddress_.fill(Undefined);
    pendingData_.fill(Undefined);
    for (const auto pin : addresses)
        definePin(pin, PinDirection::Input);
    for (const auto pin : {18U, 20U, 21U})
        definePin(pin, PinDirection::Input);
    for (const auto pin : dataPins)
        definePin(pin, PinDirection::Bidirectional);
    definePin(12, PinDirection::Power);
    definePin(19, PinDirection::Unused);
    definePin(24, PinDirection::Power);
}

std::array<Tristate, 10> Ram4801::address() {
    std::array<Tristate, 10> result{};
    for (std::size_t bit = 0; bit < addresses.size(); ++bit)
        result[bit] = read(addresses[bit]);
    return result;
}

Drive Ram4801::evaluate(std::size_t pin) {
    const auto selected = logicNot(read(18)), output = logicNot(read(20)),
               write = logicNot(read(21));
    if (selected == False || output == False || write == True)
        return Drive::HighImpedance;
    if (selected == Undefined || output == Undefined || write == Undefined)
        return Drive::Unknown;
    const auto location = address();
    const auto bit = static_cast<std::size_t>(std::find(dataPins.begin(), dataPins.end(), pin) -
                                              dataPins.begin());
    bool first = true;
    Tristate result = Undefined;
    for (std::size_t index = 0; index < memory_.size(); ++index)
        if (advanced::matchesAddress(location, index)) {
            result = first ? memory_[index][bit] : advanced::merge(result, memory_[index][bit]);
            first = false;
        }
    return toDrive(result);
}

void Ram4801::onPrepare(std::size_t) {
    pendingWrite_ = logicAnd(logicNot(read(18)), logicNot(read(21)));
    if (pendingWrite_ == False)
        return;
    pendingAddress_ = address();
    for (std::size_t bit = 0; bit < dataPins.size(); ++bit)
        pendingData_[bit] = read(dataPins[bit]);
}

void Ram4801::onSimulate(std::size_t) {
    if (pendingWrite_ == False)
        return;
    const bool certainAddress = std::none_of(pendingAddress_.begin(), pendingAddress_.end(),
                                             [](Tristate value) { return value == Undefined; });
    for (std::size_t index = 0; index < memory_.size(); ++index)
        if (advanced::matchesAddress(pendingAddress_, index))
            for (std::size_t bit = 0; bit < pendingData_.size(); ++bit)
                memory_[index][bit] = pendingWrite_ == True && certainAddress
                                          ? pendingData_[bit]
                                          : advanced::merge(memory_[index][bit], pendingData_[bit]);
}
} // namespace nts
