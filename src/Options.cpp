#include "nts/Options.hpp"
#include "nts/Error.hpp"
#include <charconv>
#include <string_view>

namespace nts {

Options parseOptions(int argc, char **argv) {
    Options options;
    bool positional = false;
    for (int index = 1; index < argc; ++index) {
        const std::string value = argv[index];
        const auto argument = [&]() -> std::string {
            if (index + 1 >= argc)
                throw Error("missing argument after '" + value + "'");
            return argv[++index];
        };
        if (!positional && value == "--") {
            positional = true;
            continue;
        }
        if (!positional && (value == "--help" || value == "-h")) {
            if (argc != 2)
                throw Error("--help must be used alone");
            options.help = true;
            return options;
        }
#ifdef NTS_BONUS
        if (!positional && value == "--gui") {
            options.gui = true;
            continue;
        }
        if (!positional && value == "--export-svg") {
            options.svg = argument();
            continue;
        }
        if (!positional && value == "--export-html") {
            options.html = argument();
            continue;
        }
        if (!positional && value == "--set") {
            options.assignments.push_back(argument());
            continue;
        }
        if (!positional && value == "--program") {
            const auto component = argument();
            options.programs.emplace_back(component, argument());
            continue;
        }
        if (!positional && value == "--ticks") {
            const auto count = argument();
            std::size_t number = 0;
            const auto [end, error] =
                std::from_chars(count.data(), count.data() + count.size(), number);
            if (error != std::errc{} || end != count.data() + count.size() || number > 1000000)
                throw Error("--ticks expects an integer in 0..1000000");
            options.ticks = number;
            continue;
        }
#else
        (void)argument;
#endif
        if (!positional && value.starts_with('-'))
            throw Error("unknown option '" + value + "'");
        if (!options.circuit.empty())
            throw Error("exactly one circuit file is required");
        options.circuit = value;
    }
    if (options.circuit.empty())
        throw Error("usage: nanotekspice [options] circuit.nts");
    return options;
}

} // namespace nts
