#pragma once

#include "GraphModel.hpp"
#include "nts/Circuit.hpp"

namespace nts::bonus {

[[nodiscard]] GraphModel captureCircuit(const Circuit &circuit);
[[nodiscard]] GraphCallbacks circuitCallbacks(Circuit &circuit);

} // namespace nts::bonus
