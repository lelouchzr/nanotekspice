#include "nts/Parser.hpp"
#include "nts/Error.hpp"
#include "nts/Shell.hpp"
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>

namespace {
std::size_t checks = 0;

void require(bool value, const std::string &message) {
    ++checks;
    if (!value)
        throw std::runtime_error(message);
}

nts::Circuit parse(const std::string &text) {
    std::istringstream input(text);
    return nts::Parser::parse(input, "fixture");
}

void reject(const std::string &text) {
    bool rejected = false;
    try {
        auto circuit = parse(text);
        (void)circuit;
    } catch (const nts::Error &error) {
        rejected = std::string(error.what()).find("fixture") != std::string::npos;
    }
    require(rejected, "invalid circuit accepted or diagnostic lost source location");
}

void parsing() {
    const std::string source = "# Français comments may contain UTF-8\n.chipsets:\r\n"
                               "input in-A\t# pending source\noutput z\noutput A\n"
                               "and gate\ntrue one\n.links:\n"
                               "in-A : 1\tgate: 1\none:1 gate:2\n"
                               "gate:3 z:1\nz:1 A:1 # input-to-input fanout\n";
    auto circuit = parse(source);
    require(circuit.tick() == 0 && circuit.nodes().size() == 5 && circuit.links().size() == 4,
            "parser lost declarations or bidirectional links");
    circuit.setInput("in-A", nts::True);
    require(circuit.component("z").compute(1) == nts::Undefined,
            "pending input became visible before simulation");
    circuit.simulate();
    require(circuit.component("z").compute(1) == nts::True &&
                circuit.component("A").compute(1) == nts::True,
            "electrical net did not propagate through fanout");
    std::istringstream input;
    std::ostringstream output;
    nts::Shell shell(circuit, input, output);
    shell.display();
    const auto rendered = output.str();
    require(rendered.find("  A: 1\n  z: 1") != std::string::npos,
            "outputs are not sorted in ASCII order");
    require(rendered.starts_with("tick: 1\ninput(s):\n  in-A: 1\noutput(s):\n"),
            "display differs from subject format");
    auto unconnected = parse(".chipsets:\noutput a\n.links:");
    require(unconnected.component("a").compute(1) == nts::Undefined,
            "unconnected output must be undefined");
    auto noOutput = parse(".chipsets:\ntrue always\n.links:\n");
    noOutput.simulate();
    require(noOutput.component("always").compute(1) == nts::True,
            "circuit without outputs is not forbidden by the subject");
    for (const auto &broken :
         {std::string{}, std::string(".chipsets:\n.links:\n"),
          std::string(".links:\n.chipsets:\ninput a\n"),
          std::string("input a\n.chipsets:\n.links:\n"),
          std::string(".chipsets:\ninput a\ninput a\n.links:\n"),
          std::string(".chipsets:\nunknown a\n.links:\n"),
          std::string(".chipsets:\ninput a extra\n.links:\n"),
          std::string(".chipsets:\ninput a\n.links:\na:1 missing:1\n"),
          std::string(".chipsets:\ninput a\noutput o\n.links:\na:0 o:1\n"),
          std::string(".chipsets:\ninput a\noutput o\n.links:\na:-1 o:1\n"),
          std::string(".chipsets:\ninput a\noutput o\n.links:\na:2 o:1\n"),
          std::string(".chipsets:\ninput a\noutput o\n.links:\na:1o:1\n"),
          std::string(".chipsets:\ninput a\noutput o\n.links:\na:1 o:1 junk\n"),
          std::string(".chipsets:\ninput a\n.chipsets:\n.links:\n"),
          std::string(".chipsets:\ninput a\n.links:\n.links:\n"),
          std::string(".chipsets:\ninput a\n.links:\na:999999999999999999999999 o:1\n"),
          std::string(".chipsets:\ninput a\n.links:\n") + std::string(8193, 'x')})
        reject(broken);
    reject(std::string(".chipsets:\ninput a") + '\0' + "\n.links:\n");
    reject(std::string(4 * 1024 * 1024 + 1, '#'));
#ifdef NTS_BONUS
    auto graphics = parse(".chipsets:\ninput a\n.links:\n.graphic:\na -5 20\n");
    require(graphics.layout().at("a").x == -5, "graphic placement lost coordinates");
    reject(".chipsets:\ninput a\n.links:\n.graphic:\na 0 0\na 1 1\n");
    reject(".chipsets:\ninput a\n.links:\n.graphic:\na 1000001 0\n");
    reject(".chipsets:\ninput a\n.links:\n.graphic:\nmissing 0 0\n");
#else
    reject(".chipsets:\ninput a\n.links:\n.graphic:\na 0 0\n");
#endif
    std::mt19937 random(20261029);
    for (int trial = 0; trial < 4000; ++trial) {
        std::string garbage;
        const auto length = random() % 200;
        for (std::size_t character = 0; character < length; ++character)
            garbage += static_cast<char>(random() % 128);
        try {
            (void)parse(garbage);
        } catch (const nts::Error &) {
        }
        ++checks;
    }
}

void clockAndShell() {
    auto circuit = parse(".chipsets:\nclock cl\noutput out\n.links:\nout:1 cl:1\n");
    std::istringstream input;
    std::ostringstream output;
    nts::Shell shell(circuit, input, output);
    require(shell.execute("cl = 0"), "assignment stopped shell");
    require(circuit.component("cl").currentInput() == nts::Undefined,
            "clock assignment must stay pending at tick zero");
    circuit.simulate();
    require(circuit.component("out").compute(1) == nts::False, "clock tick1 must be assigned zero");
    circuit.simulate();
    require(circuit.component("out").compute(1) == nts::True, "clock tick2 must invert");
    for (int index = 0; index < 3; ++index)
        circuit.simulate();
    require(circuit.tick() == 5 && circuit.component("out").compute(1) == nts::False,
            "clock example tick5 differs from subject");
    require(!shell.execute("exit"), "exit must stop shell");
    bool bounded = false;
    try {
        (void)shell.execute(std::string(100000, ' '));
    } catch (const nts::Error &) {
        bounded = true;
    }
    require(bounded, "shell must bound raw whitespace before trimming");
    std::ostringstream brokenOutput;
    brokenOutput.setstate(std::ios::badbit);
    nts::Shell brokenShell(circuit, input, brokenOutput);
    bool writeFailed = false;
    try {
        brokenShell.display();
    } catch (const nts::Error &) {
        writeFailed = true;
    }
    require(writeFailed, "display must report stdout failure");
    for (const auto &command :
         {"cl=2", "out=1", "absent=0", "simulate junk", "cl==0", "=0", "cl="}) {
        bool rejected = false;
        try {
            (void)shell.execute(command);
        } catch (const nts::Error &) {
            rejected = true;
        }
        require(rejected, std::string("shell accepted invalid command: ") + command);
    }
}
} // namespace

int main() {
    try {
        parsing();
        clockAndShell();
        std::cout << checks << " parser/shell checks passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
