#include "BasicSupport.hpp"
#include "nts/ComponentFactory.hpp"

namespace nts::test {

void sourceTests() {
    auto input = createComponent("input");
    auto clock = createComponent("clock");
    auto high = createComponent("true");
    auto low = createComponent("false");
    auto output = createComponent("output");
    output->setLink(1, *clock, 1);
    require(input->currentInput() == Undefined && output->compute(1) == Undefined,
            "sources must start undefined");
    clock->setInput(False);
    require(clock->currentInput() == Undefined && output->compute(1) == Undefined,
            "pending clock assignment became visible before simulate");
    clock->simulate(1);
    require(clock->currentInput() == False && output->compute(1) == False,
            "first clock assignment was inverted too early");
    clock->simulate(1);
    require(output->compute(1) == False, "simulate must be idempotent within one tick");
    clock->simulate(2);
    require(output->compute(1) == True, "clock did not toggle on its next tick");
    clock->setInput(Undefined);
    require(output->compute(1) == True, "pending U changed a committed clock value");
    clock->simulate(3);
    clock->simulate(4);
    require(output->compute(1) == Undefined, "undefined clock guessed a value");
    input->setInput(False);
    input->setInput(True);
    input->simulate(1);
    require(input->compute(1) == True && input->currentInput() == True,
            "last pending input assignment must win");
    input->simulate(2);
    require(input->compute(1) == True, "input incorrectly toggled like a clock");
    require(high->compute(1) == True && low->compute(1) == False, "constant source values");
    require(!high->isInputSource() && !output->isInputSource() && clock->isInputSource(),
            "source introspection");
    rejected([&] { high->setInput(False); }, "constant accepted assignment");
    rejected([&] { output->setInput(True); }, "output accepted assignment");
    rejected([&] { input->setInput(static_cast<Tristate>(-2)); }, "invalid tristate accepted");
    rejected([&] { static_cast<void>(input->compute(0)); }, "pin zero accepted");
    rejected([&] { static_cast<void>(input->compute(2)); }, "past-end pin accepted");
    rejected([&] { static_cast<void>(high->currentInput()); }, "constant claimed input state");
}

} // namespace nts::test
