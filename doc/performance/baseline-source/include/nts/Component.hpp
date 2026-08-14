#pragma once
#include "IComponent.hpp"
#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace nts {

struct ComponentEndpoint {
    IComponent *component = nullptr;
    std::size_t pin = 0;
};

struct ComponentNet {
    std::vector<ComponentEndpoint> endpoints;
};

struct ComponentPin {
    PinDirection direction = PinDirection::Unused;
    std::shared_ptr<ComponentNet> net;
    std::optional<Drive> cached;
    bool evaluating = false;
};

class Component : public IComponent {
  public:
    Component(std::string type, std::size_t pins);
    ~Component() override;
    Component(const Component &) = delete;
    Component &operator=(const Component &) = delete;
    void simulate(std::size_t tick) override;
    [[nodiscard]] Tristate compute(std::size_t pin) override;
    [[nodiscard]] Drive drive(std::size_t pin) override;
    void setLink(std::size_t pin, IComponent &other, std::size_t otherPin) override;
    void beginTick(std::size_t tick) override;
    void prepare(std::size_t tick) override;
    void endTick(std::size_t tick) override;
    [[nodiscard]] std::size_t pinCount() const noexcept override;
    [[nodiscard]] PinDirection pinDirection(std::size_t pin) const override;
    [[nodiscard]] std::string_view type() const noexcept override;

  protected:
    void definePin(std::size_t pin, PinDirection direction);
    [[nodiscard]] Tristate read(std::size_t pin);
    [[nodiscard]] virtual Drive evaluate(std::size_t pin) = 0;
    virtual void onBeginTick(std::size_t tick);
    virtual void onPrepare(std::size_t tick);
    virtual void onSimulate(std::size_t tick);
    virtual void onEndTick(std::size_t tick);
    void invalidate() noexcept;

  private:
    std::string type_;
    std::vector<ComponentPin> pins_;
    std::optional<std::size_t> begun_, prepared_, committed_;
    std::uint64_t epochSeen_ = 0;
    static std::atomic<std::uint64_t> epoch_;
    static thread_local std::size_t depth_;
    void refresh() noexcept;
    [[nodiscard]] Drive resolve(std::size_t pin, bool includeSelf);
    ComponentPin &checked(std::size_t pin);
    [[nodiscard]] const ComponentPin &checked(std::size_t pin) const;
    void merge(Component &other, std::size_t pin, std::size_t otherPin);
};

} // namespace nts
