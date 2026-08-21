#include "Support.hpp"
#include "nts/Parser.hpp"
#include <algorithm>

namespace nts::bonus::test {

void computerTests(const std::filesystem::path &examples) {
    auto circuit = Parser::load(examples / "computer.nts");
    circuit.component("cpu").loadProgram(examples / "triangular.subleq");
    circuit.setInput("clk", False);
    require(circuit.component("cpu").compute(4) == Undefined,
            "CPU output bus was defined before a program output");
    for (unsigned tick = 0; tick < 150; ++tick)
        circuit.simulate();
    require(circuit.component("cpu").compute(24) == True &&
                circuit.component("digit").visualization()->text == "7",
            "little computer did not calculate55 and drive the hexadecimal display");
    const auto matrix = circuit.component("screen").visualization();
    require(matrix && matrix->known[0], "little computer did not write its output bus into matrix");
    circuit.component("cpu").loadProgram(examples / "paint.subleq");
    circuit.setInput("clk", False);
    for (unsigned tick = 0; tick < 42000; ++tick)
        circuit.simulate();
    const auto screen = circuit.component("screen").visualization();
    require(circuit.component("cpu").compute(24) == True && screen &&
                std::count(screen->known.begin(), screen->known.end(), true) == 4096,
            "little computer failed CPU-to-matrix full framebuffer program");
    require(screen->pixels[0] == 0 && screen->pixels[255] == 0xffffffU &&
                screen->pixels[4095] == 0xffffffU,
            "little computer framebuffer output values are incorrect");
    auto keyboard = Parser::load(examples / "keyboard.nts");
    keyboard.component("cpu").loadProgram(examples / "echo.subleq");
    keyboard.setInput("clk", False);
    keyboard.setInput("strobe", False);
    for (unsigned bit = 0; bit < 8; ++bit)
        keyboard.setInput("key" + std::to_string(bit), (65U & (1U << bit)) ? True : False);
    keyboard.simulate();
    keyboard.simulate();
    require(keyboard.component("cpu").compute(25) == True, "CPU did not wait for keyboard bus");
    keyboard.setInput("strobe", True);
    for (unsigned tick = 0; tick < 10; ++tick)
        keyboard.simulate();
    require(keyboard.component("cpu").compute(24) == True &&
                keyboard.component("digit").visualization()->text == "1" &&
                keyboard.component("cpu").visualization()->text.find("OUTPUT=A") !=
                    std::string::npos,
            "keyboard strobe did not feed and resume the CPU through IComponent pins");
}

} // namespace nts::bonus::test
