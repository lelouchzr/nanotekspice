#include "nts/Error.hpp"
#include "nts/Options.hpp"
#include "nts/Parser.hpp"
#include "nts/Shell.hpp"
#ifdef NTS_BONUS
#include "CircuitGraph.hpp"
#include "GraphGui.hpp"
#endif
#include <csignal>
#include <iostream>

int main(int argc, char **argv) {
    try {
#ifdef SIGPIPE
        std::signal(SIGPIPE, SIG_IGN);
#endif
        // Reporting stdout failures must not flush the same failed stream again.
        std::cerr.tie(nullptr);
        std::cout.exceptions(std::ios::badbit | std::ios::failbit);
        const auto options = nts::parseOptions(argc, argv);
        if (options.help) {
            std::cout << "USAGE: nanotekspice [options] circuit.nts\n"
                         "Commands: display, simulate, input=0|1|U, loop (Ctrl+C), exit\n";
#ifdef NTS_BONUS
            std::cout << "--gui | --ticks N | --set name=value | --program cpu file\n"
                         "--export-svg file | --export-html file\n"
                         "Bonus shell command: load cpu \"program.subleq\"\n";
#endif
            std::cout.flush();
            return 0;
        }
        auto circuit = nts::Parser::load(options.circuit);
        nts::Shell shell(circuit, std::cin, std::cout);
#ifdef NTS_BONUS
        for (const auto &[name, path] : options.programs)
            circuit.component(name).loadProgram(path);
        for (const auto &assignment : options.assignments) {
            if (assignment.find('=') == std::string::npos)
                throw nts::Error("--set expects name=value");
            (void)shell.execute(assignment);
        }
        if (options.ticks)
            for (std::size_t tick = 0; tick < *options.ticks; ++tick)
                circuit.simulate();
        if (options.svg)
            nts::bonus::exportSvg(nts::bonus::captureCircuit(circuit), *options.svg);
        if (options.html)
            nts::bonus::exportHtml(nts::bonus::captureCircuit(circuit), *options.html);
        if (options.gui)
            nts::bonus::runGui(nts::bonus::circuitCallbacks(circuit));
        else if (options.ticks || options.svg || options.html)
            shell.display();
        else
#endif
            shell.run();
        return 0;
    } catch (const std::exception &error) {
        std::cout.exceptions(std::ios::goodbit);
        std::cerr << "nanotekspice: " << error.what() << '\n';
        return 84;
    } catch (...) {
        std::cout.exceptions(std::ios::goodbit);
        std::cerr << "nanotekspice: unexpected failure\n";
        return 84;
    }
}
