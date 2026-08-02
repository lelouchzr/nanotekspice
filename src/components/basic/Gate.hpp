#pragma once
#include "nts/Component.hpp"

namespace nts {

enum class GateOperation { And, Or, Xor, Not, Nand, Nor };

struct GateFunction {
    std::size_t first = 0;
    std::size_t second = 0;
    std::size_t output = 0;
    GateOperation operation = GateOperation::And;
};

class Gate final : public Component {
  public:
    Gate(std::string type, std::size_t pins, std::vector<GateFunction> functions,
         bool powered = false);

  protected:
    [[nodiscard]] Drive evaluate(std::size_t pin) override;

  private:
    std::vector<GateFunction> functions_;
    [[nodiscard]] Tristate apply(const GateFunction &function);
};

} // namespace nts
