#pragma once
#include "Tristate.hpp"
#include <cstddef>
#include <string_view>
#ifdef NTS_BONUS
#include "DisplayFrame.hpp"
#include <filesystem>
#include <optional>
#endif

namespace nts {

enum class PinDirection { Unused, Power, Input, Output, Bidirectional };

class IComponent {
  public:
    virtual ~IComponent() = default;
    virtual void simulate(std::size_t tick) = 0;
    [[nodiscard]] virtual Tristate compute(std::size_t pin) = 0;
    virtual void setLink(std::size_t pin, IComponent &other, std::size_t otherPin) = 0;
    virtual void beginTick(std::size_t tick);
    virtual void prepare(std::size_t tick);
    virtual void endTick(std::size_t tick);
    [[nodiscard]] virtual Drive drive(std::size_t pin);
    [[nodiscard]] virtual std::size_t pinCount() const noexcept = 0;
    [[nodiscard]] virtual PinDirection pinDirection(std::size_t pin) const = 0;
    [[nodiscard]] virtual std::string_view type() const noexcept = 0;
    [[nodiscard]] virtual bool isInputSource() const noexcept;
    virtual void setInput(Tristate value);
    [[nodiscard]] virtual Tristate currentInput() const;
#ifdef NTS_BONUS
    [[nodiscard]] virtual std::optional<bonus::DisplayFrame> visualization() const;
    virtual void loadProgram(const std::filesystem::path &path);
#endif
};

} // namespace nts
