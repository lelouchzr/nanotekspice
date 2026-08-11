#include "Support.hpp"
#include "components/advanced/AdvancedFactory.hpp"
#include <chrono>
#include <stdexcept>
#include <vector>

namespace advanced_test {
namespace {
std::size_t count = 0;
}

void require(bool condition, const std::string &message) {
    ++count;
    if (!condition)
        throw std::runtime_error(message);
}

std::size_t checks() {
    return count;
}

void advance(std::span<nts::IComponent *> components, std::size_t tick) {
    for (auto *component : components)
        component->beginTick(tick);
    for (auto *component : components)
        component->prepare(tick);
    for (auto *component : components)
        component->simulate(tick);
    for (auto *component : components)
        component->endTick(tick);
}

Source::Source() : Component("test-source", 1) {
    definePin(1, nts::PinDirection::Output);
}

void Source::set(nts::Drive value) {
    value_ = value;
    invalidate();
}

nts::Drive Source::evaluate(std::size_t) {
    return value_;
}

Rig::Rig(const std::string &type) : chip_(nts::createAdvancedComponent(type)) {
    require(chip_ != nullptr, "factory missing " + type);
}

Source &Rig::source(std::size_t pin) {
    auto &result = sources_[pin];
    if (!result) {
        result = std::make_unique<Source>();
        try {
            result->setLink(1, *chip_, pin);
        } catch (const std::exception &error) {
            throw std::runtime_error(std::string(chip_->type()) + " pin " + std::to_string(pin) +
                                     ": " + error.what());
        }
    }
    return *result;
}

void Rig::set(std::size_t pin, nts::Tristate value) {
    source(pin).set(nts::toDrive(value));
}

void Rig::release(std::size_t pin) {
    source(pin).set(nts::Drive::HighImpedance);
}

void Rig::tick() {
    std::vector<nts::IComponent *> components;
    for (auto &[pin, input] : sources_) {
        (void)pin;
        components.push_back(input.get());
    }
    components.push_back(chip_.get());
    advanced_test::advance(components, ++tick_);
}

nts::IComponent &Rig::chip() {
    return *chip_;
}

nts::Tristate Rig::out(std::size_t pin) {
    return chip_->compute(pin);
}

void Rig::bus(std::span<const std::size_t> pins, std::size_t value) {
    for (std::size_t bit = 0; bit < pins.size(); ++bit)
        set(pins[bit], ((value >> bit) & 1U) != 0 ? nts::True : nts::False);
}

Directory::Directory() : previous_(std::filesystem::current_path()) {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    temporary_ = std::filesystem::temp_directory_path() / ("nts-advanced-" + std::to_string(stamp));
    if (!std::filesystem::create_directory(temporary_))
        throw std::runtime_error("cannot create advanced test directory");
    std::filesystem::current_path(temporary_);
}

Directory::~Directory() {
    std::error_code error;
    std::filesystem::current_path(previous_, error);
    std::filesystem::remove_all(temporary_, error);
}
} // namespace advanced_test
