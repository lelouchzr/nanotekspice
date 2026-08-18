#include "Cpu.hpp"
#include <sstream>

namespace nts::bonus {

Cpu::Cpu() : Component("subleq", 34) {
    for (std::size_t pin = 1; pin <= 3; ++pin)
        definePin(pin, PinDirection::Input);
    for (std::size_t pin = 4; pin <= 25; ++pin)
        definePin(pin, PinDirection::Output);
    for (std::size_t pin = 26; pin <= 34; ++pin)
        definePin(pin, PinDirection::Input);
}

void Cpu::loadProgram(const std::filesystem::path &path) {
    auto program = Program::load(path);
    machine_.load(std::move(program));
    outputKnown_ = false;
    previousClock_ = previousInputClock_ = Undefined;
    invalidate();
}

Drive Cpu::evaluate(std::size_t pin) {
    if (pin <= 3 || pin >= 26)
        return Drive::HighImpedance;
    if (pin == 24)
        return toDrive(machine_.status() == CpuStatus::Halted ? True : False);
    if (pin == 25)
        return toDrive(machine_.status() == CpuStatus::Waiting ? True : False);
    if (!outputKnown_)
        return Drive::Unknown;
    const unsigned bit = static_cast<unsigned>(pin <= 11 ? pin - 4 : pin - 12);
    const unsigned value = pin <= 11 ? value_ : address_;
    return (value & (1U << bit)) ? Drive::High : Drive::Low;
}

void Cpu::onPrepare(std::size_t) {
    pendingClock_ = read(1);
    reset_ = read(2);
    enable_ = read(3);
    pendingInputClock_ = read(34);
    pendingInput_.reset();
    if (previousInputClock_ == False && pendingInputClock_ == True) {
        unsigned value = 0;
        bool known = true;
        for (std::size_t bit = 0; bit < 8; ++bit) {
            const auto signal = read(26 + bit);
            known = known && signal != Undefined;
            if (signal == True)
                value |= 1U << bit;
        }
        if (known)
            pendingInput_ = static_cast<std::uint8_t>(value);
    }
}

void Cpu::onSimulate(std::size_t) {
    if (reset_ == True) {
        machine_.reset();
        outputKnown_ = false;
    } else {
        if (reset_ == False && pendingInput_)
            machine_.input(*pendingInput_);
        if (reset_ == False && enable_ == True && previousClock_ == False &&
            pendingClock_ == True) {
            machine_.step();
            if (const auto output = machine_.lastOutput()) {
                outputKnown_ = true;
                value_ = output->value;
                address_ = output->address;
            }
        }
    }
    previousInputClock_ = pendingInputClock_;
    previousClock_ = pendingClock_;
}

std::optional<DisplayFrame> Cpu::visualization() const {
    DisplayFrame frame;
    frame.kind = DisplayKind::Processor;
    std::ostringstream text;
    text << "SUBLEQ PC=" << machine_.pc().string() << " STEPS=" << machine_.instructions()
         << " STATE="
         << (machine_.status() == CpuStatus::Halted    ? "HALTED"
             : machine_.status() == CpuStatus::Waiting ? "WAITING"
                                                       : "READY")
         << " OUTPUT=";
    for (const auto &event : machine_.output())
        if (event.serial) {
            if (event.value >= 32 && event.value < 127)
                text << static_cast<char>(event.value);
            else
                text << '[' << static_cast<unsigned>(event.value) << ']';
        }
    frame.text = text.str();
    return frame;
}

} // namespace nts::bonus
