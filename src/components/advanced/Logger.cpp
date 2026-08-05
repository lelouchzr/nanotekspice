#include "nts/components/advanced/Logger.hpp"
#include "nts/Error.hpp"
#include "nts/components/advanced/Logic.hpp"
#include <fstream>

namespace nts {
Logger::Logger() : Component("logger", 10) {
    for (std::size_t pin = 1; pin <= 10; ++pin)
        definePin(pin, PinDirection::Input);
}

Drive Logger::evaluate(std::size_t) {
    return Drive::HighImpedance;
}

void Logger::onPrepare(std::size_t) {
    pendingByte_.reset();
    pendingClock_ = read(9);
    if (advanced::edge(clock_, pendingClock_, true) != True || read(10) != False)
        return;
    unsigned int value = 0;
    for (std::size_t bit = 0; bit < 8; ++bit) {
        const auto input = read(bit + 1);
        if (input == Undefined)
            return;
        if (input == True)
            value |= 1U << bit;
    }
    pendingByte_ = static_cast<unsigned char>(value);
}

void Logger::onSimulate(std::size_t) {
    clock_ = pendingClock_;
    if (!pendingByte_)
        return;
    std::ofstream file("log.bin", std::ios::binary | std::ios::app);
    if (!file)
        throw ComponentError("cannot append to log.bin");
    const char byte = static_cast<char>(*pendingByte_);
    file.write(&byte, 1);
    file.flush();
    if (!file)
        throw ComponentError("failed writing log.bin");
}
} // namespace nts
