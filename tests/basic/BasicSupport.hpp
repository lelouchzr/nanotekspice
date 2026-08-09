#pragma once
#include "nts/Component.hpp"
#include <functional>
#include <string>
#include <vector>

namespace nts::test {

extern int checks;
void require(bool condition, const std::string &message);
void rejected(const std::function<void()> &operation, const std::string &message);
void tick(const std::vector<IComponent *> &components, std::size_t number);

class DriveProbe final : public Component {
  public:
    explicit DriveProbe(Drive value);
    void value(Drive value);

  protected:
    Drive evaluate(std::size_t pin) override;

  private:
    Drive value_;
};

class BidirectionalProbe final : public Component {
  public:
    explicit BidirectionalProbe(Drive value);

  protected:
    Drive evaluate(std::size_t pin) override;

  private:
    Drive value_;
};

class CountedBuffer final : public Component {
  public:
    CountedBuffer();
    [[nodiscard]] int evaluations() const noexcept;

  protected:
    Drive evaluate(std::size_t pin) override;

  private:
    int evaluations_ = 0;
};

class SampledLatch final : public Component {
  public:
    SampledLatch();

  protected:
    Drive evaluate(std::size_t pin) override;
    void onPrepare(std::size_t tick) override;
    void onSimulate(std::size_t tick) override;

  private:
    Tristate value_ = False;
    Tristate pending_ = False;
};

void basicTruthTests();
void sourceTests();
void connectionTests();
void phaseAndGuardTests();
void factoryTests();

} // namespace nts::test
