#include "CircuitGraph.hpp"
#include "GraphGui.hpp"
#include "Support.hpp"
#include "nts/Parser.hpp"
#include <fstream>
#include <sstream>
#include <unistd.h>

namespace nts::bonus::test {

void graphTests(const std::filesystem::path &examples) {
    auto circuit = Parser::load(examples / "seven_segment.nts");
    circuit.setInput("b0", True);
    circuit.setInput("b1", False);
    circuit.setInput("b2", True);
    circuit.setInput("b3", False);
    auto callbacks = circuitCallbacks(circuit);
    callbacks.step();
    auto graph = callbacks.captureGraph();
    require(graph.tick == 1 && graph.nodes.size() == 5 && graph.edges.size() == 4,
            "graph bridge did not preserve real components/links/tick");
    bool found = false;
    for (const auto &node : graph.nodes)
        if (node.name == "digit") {
            found = true;
            require(node.x == 420 && node.y == 120 && node.display && node.display->text == "5",
                    "graph lost layout or live graphical component");
        }
    require(found, "graph component disappeared");
    callbacks.setInput("b1", True);
    callbacks.step();
    require(circuit.component("digit").visualization()->text == "7",
            "GUI callback input did not affect live simulation");
    const auto svg = graphSvg(callbacks.captureGraph());
    require(svg.find("<svg") != std::string::npos && svg.find("digit") != std::string::npos &&
                svg.find("<path") != std::string::npos && svg.find("#f3a85c") != std::string::npos,
            "graph SVG is missing components, wires or rendered7seg pixels");
    graph.nodes.front().name = "<script>alert('x')</script>\"";
    graph.nodes.front().type = "<unsafe>";
    const auto escaped = graphSvg(graph);
    require(escaped.find("<script>alert") == std::string::npos &&
                escaped.find("&lt;script&gt;") != std::string::npos,
            "graph export did not escape external labels");
    const auto directory = std::filesystem::temp_directory_path() /
                           ("nanotekspice-bonus-exports-" + std::to_string(::getpid()));
    std::filesystem::create_directories(directory);
    const auto svgPath = directory / "circuit.svg", htmlPath = directory / "circuit.html";
    exportSvg(graph, svgPath);
    exportHtml(graph, htmlPath);
    std::ifstream file(htmlPath);
    std::ostringstream html;
    html << file.rdbuf();
    require(html.str().find("Recorded snapshot") != std::string::npos &&
                html.str().find("pointermove") != std::string::npos &&
                html.str().find("Filter components") != std::string::npos,
            "HTML inspector lacks explicit snapshot semantics or interaction");
    rejects([&] { exportSvg(graph, directory / "missing" / "file.svg"); },
            "graph export swallowed file errors");
    std::filesystem::remove(svgPath);
    std::filesystem::remove(htmlPath);
    std::filesystem::remove(directory);
}

} // namespace nts::bonus::test
