#include "Export.hpp"
#include "../GraphGui.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace nts::bonus {

std::string escapeXml(std::string_view value) {
    std::string result;
    for (char character : value) {
        switch (character) {
        case '&':
            result += "&amp;";
            break;
        case '<':
            result += "&lt;";
            break;
        case '>':
            result += "&gt;";
            break;
        case '\"':
            result += "&quot;";
            break;
        case '\'':
            result += "&apos;";
            break;
        case '\n':
            result += "&#10;";
            break;
        case '\t':
            result += "&#9;";
            break;
        default:
            if (static_cast<unsigned char>(character) >= 32 || character == '\n' ||
                character == '\t')
                result += character;
            break;
        }
    }
    return result;
}

std::string colorHex(std::uint32_t color) {
    std::ostringstream stream;
    stream << '#' << std::hex << std::setfill('0') << std::setw(6) << (color & 0xffffffU);
    return stream.str();
}

int nodeHeight(const GraphNode &node) {
    return node.display ? node.display->kind == DisplayKind::Processor ? 128 : 220 : 98;
}

GraphModel arrangeGraph(GraphModel graph) {
    const bool automatic = graph.nodes.size() > 1 &&
                           std::all_of(graph.nodes.begin(), graph.nodes.end(),
                                       [](const auto &node) { return node.x == 0 && node.y == 0; });
    for (std::size_t i = 0; i < graph.nodes.size(); ++i) {
        auto &node = graph.nodes[i];
        node.x =
            automatic ? 40 + static_cast<int>(i % 4) * 290 : std::clamp(node.x, -1000000, 1000000);
        node.y =
            automatic ? 60 + static_cast<int>(i / 4) * 270 : std::clamp(node.y, -1000000, 1000000);
    }
    return graph;
}

std::string graphSvg(const GraphModel &input) {
    const auto graph = arrangeGraph(input);
    int minimumX = 0, minimumY = 0, maximumX = 1000, maximumY = 700;
    for (const auto &node : graph.nodes) {
        minimumX = std::min(minimumX, node.x - 30);
        minimumY = std::min(minimumY, node.y - 30);
        maximumX = std::max(maximumX, node.x + 280);
        maximumY = std::max(maximumY, node.y + nodeHeight(node) + 30);
    }
    std::ostringstream svg;
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"" << minimumX << ' ' << minimumY
        << ' ' << maximumX - minimumX << ' ' << maximumY - minimumY
        << "\" role=\"img\" aria-label=\"NanoTekSpice circuit\">"
        << "<style>text{font:13px "
           "monospace;fill:#dce8ef}.node{cursor:pointer}.wire{fill:none;stroke:#677e90;stroke-"
           "width:2}</style>"
        << "<rect x=\"" << minimumX << "\" y=\"" << minimumY << "\" width=\"" << maximumX - minimumX
        << "\" height=\"" << maximumY - minimumY << "\" fill=\"#0a111c\"/>";
    for (const auto &edge : graph.edges) {
        const auto find = [&](const std::string &name) {
            return std::find_if(graph.nodes.begin(), graph.nodes.end(),
                                [&](const auto &node) { return node.name == name; });
        };
        const auto first = find(edge.fromNode), second = find(edge.toNode);
        if (first == graph.nodes.end() || second == graph.nodes.end())
            continue;
        const int x1 = first->x + 240, y1 = first->y + 49;
        const int x2 = second->x, y2 = second->y + 49;
        svg << "<path class=\"wire\" d=\"M" << x1 << ',' << y1 << " C" << x1 + 40 << ',' << y1
            << ' ' << x2 - 40 << ',' << y2 << ' ' << x2 << ',' << y2 << "\"><title>"
            << escapeXml(edge.fromNode) << ':' << edge.fromPin << " - " << escapeXml(edge.toNode)
            << ':' << edge.toPin << "</title></path>";
    }
    for (const auto &node : graph.nodes) {
        std::ostringstream detail;
        detail << node.name << " [" << node.type << "]";
        for (const auto &pin : node.pins)
            detail << "\nPIN " << pin.number << ' ' << pin.direction << " = "
                   << toString(pin.value);
        if (node.display)
            detail << '\n' << node.display->text;
        svg << "<g class=\"node\" data-name=\"" << escapeXml(node.name) << "\" data-detail=\""
            << escapeXml(detail.str()) << "\" transform=\"translate(" << node.x << ',' << node.y
            << ")\">";
        svg << "<rect width=\"240\" height=\"" << nodeHeight(node)
            << "\" rx=\"9\" fill=\"#142131\" stroke=\"#344b61\"/>";
        svg << "<text x=\"14\" y=\"24\">" << escapeXml(node.name) << "</text>";
        svg << "<text x=\"14\" y=\"46\" style=\"fill:#78d5ad\">" << escapeXml(node.type)
            << "</text>";
        std::string values;
        for (const auto &pin : node.pins)
            values += std::to_string(pin.number) + ':' + std::string(toString(pin.value)) + ' ';
        svg << "<text x=\"14\" y=\"75\">" << escapeXml(values.substr(0, 27)) << "</text>";
        if (node.display) {
            const auto &display = *node.display;
            if (display.width > 0 && display.height > 0 && display.width <= 1024 &&
                display.height <= 1024 && display.pixels.size() == display.width * display.height) {
                const double scale = std::min(200.0 / static_cast<double>(display.width),
                                              120.0 / static_cast<double>(display.height));
                for (std::size_t y = 0; y < display.height; ++y)
                    for (std::size_t x = 0; x < display.width; ++x) {
                        const auto index = y * display.width + x;
                        const bool known = display.known.empty() ||
                                           (index < display.known.size() && display.known[index]);
                        const auto color = known         ? display.pixels[index]
                                           : (x + y) % 2 ? 0x303c4cU
                                                         : 0x1d2735U;
                        svg << "<rect x=\"" << 14 + static_cast<double>(x) * scale << "\" y=\""
                            << 90 + static_cast<double>(y) * scale << "\" width=\"" << scale
                            << "\" height=\"" << scale << "\" fill=\"" << colorHex(color) << "\"/>";
                    }
            } else {
                svg << "<text x=\"14\" y=\"104\">" << escapeXml(display.text.substr(0, 27))
                    << "</text>";
            }
        }
        svg << "</g>";
    }
    svg << "</svg>";
    return svg.str();
}

void exportSvg(const GraphModel &graph, const std::filesystem::path &path) {
    std::ofstream output(path);
    if (!output || !(output << graphSvg(graph)) || !output.flush())
        throw std::runtime_error("cannot write circuit SVG: " + path.string());
}

void exportHtml(const GraphModel &graph, const std::filesystem::path &path) {
    std::ofstream output(path);
    if (!output)
        throw std::runtime_error("cannot write circuit HTML: " + path.string());
    output << "<!doctype html><html lang=\"en\"><meta charset=\"utf-8\"><title>NanoTekSpice "
              "inspector</title>"
           << "<style>body{margin:0;background:#0a111c;color:#dce8ef;font:14px "
              "monospace}header{padding:14px;display:flex;gap:20px;align-items:center}input,button{"
              "font:inherit;background:#142131;color:#dce8ef;border:1px solid "
              "#344b61;padding:7px}main{display:grid;grid-template-columns:1fr "
              "300px;height:88vh}#canvas{overflow:hidden;cursor:grab}svg{width:100%;height:100%;"
              "touch-action:none}pre{white-space:pre-wrap;overflow-wrap:anywhere;padding:15px;"
              "border-left:1px solid #344b61}.hidden{opacity:.12}</style>"
           << "<header><b>NanoTekSpice / tick " << graph.tick
           << "</b><input id=\"filter\" placeholder=\"Filter components\" aria-label=\"Filter "
              "components\"><button id=\"reset\">Reset view</button><span>Recorded snapshot. Live "
              "stepping: --gui</span></header>"
           << "<main><div id=\"canvas\">" << graphSvg(graph)
           << "</div><pre id=\"detail\">Select a component. Wheel zooms; drag pans.</pre></main>"
           << R"(<script>
const svg=document.querySelector('svg'), original=svg.getAttribute('viewBox');
let box=original.split(' ').map(Number), drag=null;
const apply=()=>svg.setAttribute('viewBox',box.join(' '));
document.querySelectorAll('.node').forEach(n=>n.addEventListener('click',()=>document.querySelector('#detail').textContent=n.dataset.detail));
document.querySelector('#filter').addEventListener('input',e=>document.querySelectorAll('.node').forEach(n=>n.classList.toggle('hidden',!n.dataset.name.toLowerCase().includes(e.target.value.toLowerCase()))));
document.querySelector('#reset').onclick=()=>{box=original.split(' ').map(Number);apply()};
svg.addEventListener('wheel',e=>{e.preventDefault();const r=svg.getBoundingClientRect(),f=e.deltaY>0?1.12:1/1.12,x=(e.clientX-r.left)/r.width,y=(e.clientY-r.top)/r.height;box[0]+=box[2]*x*(1-f);box[1]+=box[3]*y*(1-f);box[2]*=f;box[3]*=f;apply()},{passive:false});
svg.addEventListener('pointerdown',e=>{drag=[e.clientX,e.clientY,box[0],box[1]];svg.setPointerCapture(e.pointerId)});
svg.addEventListener('pointermove',e=>{if(!drag)return;const r=svg.getBoundingClientRect();box[0]=drag[2]-(e.clientX-drag[0])*box[2]/r.width;box[1]=drag[3]-(e.clientY-drag[1])*box[3]/r.height;apply()});
svg.addEventListener('pointerup',()=>drag=null);
</script></html>)";
    if (!output || !output.flush())
        throw std::runtime_error("cannot write circuit HTML: " + path.string());
}

} // namespace nts::bonus
