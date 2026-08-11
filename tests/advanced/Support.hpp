#pragma once
#include "nts/Component.hpp"
#include <filesystem>
#include <map>
#include <memory>
#include <span>
#include <string>

namespace advanced_test {
void require(bool condition, const std::string &message);
std::size_t checks();
void advance(std::span<nts::IComponent *> components, std::size_t tick);

class Source final : public nts::Component {
  public:
    Source();
    void set(nts::Drive value);

  protected:
    nts::Drive evaluate(std::size_t pin) override;

  private:
    nts::Drive value_ = nts::Drive::Unknown;
};

class Rig {
  public:
    explicit Rig(const std::string &type);
    void set(std::size_t pin, nts::Tristate value);
    void release(std::size_t pin);
    void tick();
    nts::IComponent &chip();
    nts::Tristate out(std::size_t pin);
    void bus(std::span<const std::size_t> pins, std::size_t value);

  private:
    std::unique_ptr<nts::IComponent> chip_;
    std::map<std::size_t, std::unique_ptr<Source>> sources_;
    std::size_t tick_ = 0;
    Source &source(std::size_t pin);
};

class Directory {
  public:
    Directory();
    ~Directory();
    Directory(const Directory &) = delete;
    Directory &operator=(const Directory &) = delete;

  private:
    std::filesystem::path previous_, temporary_;
};

void combinational();
void flipflops();
void counters();
void shiftAndLatch();
void memories();
void logger();
} // namespace advanced_test
