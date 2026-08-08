#include "nts/Parser.hpp"
#include "nts/Error.hpp"
#include "nts/Text.hpp"
#include <charconv>
#include <fstream>
#include <sstream>
#include <string_view>

namespace nts {
namespace {

constexpr std::size_t MaxFileBytes = 4 * 1024 * 1024;

std::string trim(std::string value) {
    const auto begin = value.find_first_not_of(" \t\r");
    if (begin == std::string::npos)
        return {};
    return value.substr(begin, value.find_last_not_of(" \t\r") - begin + 1);
}

void identifier(std::string_view value) {
    if (value.empty() || value.size() > 128)
        throw Error("identifier must contain 1..128 characters");
    for (const char raw : value) {
        const auto character = static_cast<unsigned char>(raw);
        if (!((character >= 'A' && character <= 'Z') || (character >= 'a' && character <= 'z') ||
              (character >= '0' && character <= '9') || character == '_' || character == '-' ||
              character == '.'))
            throw Error("invalid identifier '" + std::string(value) + "'");
    }
}

std::pair<std::string, std::size_t> endpoint(const std::string &line, std::size_t &position) {
    const auto colon = line.find(':', position);
    if (colon == std::string::npos)
        throw Error("link endpoint must use name:pin");
    const auto name = trim(line.substr(position, colon - position));
    identifier(name);
    const auto start = line.find_first_not_of(" \t", colon + 1);
    if (start == std::string::npos)
        throw Error("missing pin number");
    position = start;
    while (position < line.size() && line[position] >= '0' && line[position] <= '9')
        ++position;
    std::size_t pin = 0;
    const auto [end, error] = std::from_chars(line.data() + start, line.data() + position, pin);
    if (error != std::errc{} || end != line.data() + position || pin == 0 ||
        (position < line.size() && line[position] != ' ' && line[position] != '\t'))
        throw Error("pin must be a positive integer followed by whitespace or end of line");
    return {name, pin};
}

int coordinate(const std::string &value) {
    int result = 0;
    const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), result);
    if (error != std::errc{} || end != value.data() + value.size() || result < -1000000 ||
        result > 1000000)
        throw Error("graphic coordinate must be an integer in -1000000..1000000");
    return result;
}

} // namespace

Circuit Parser::load(const std::filesystem::path &path) {
    std::error_code error;
    if (!std::filesystem::is_regular_file(path, error))
        throw Error("circuit is not a readable regular file: " + path.string());
    const auto size = std::filesystem::file_size(path, error);
    if (error || size > MaxFileBytes)
        throw Error("cannot read circuit or file exceeds 4 MiB: " + path.string());
    std::ifstream input(path, std::ios::binary);
    if (!input)
        throw Error("cannot open circuit: " + path.string());
    return parse(input, path.string());
}

Circuit Parser::parse(std::istream &stream, const std::string &source) {
    Circuit circuit;
    int section = 0;
    bool chipsets = false, links = false, graphics = false;
    std::size_t lineNumber = 0, bytes = 0;
    std::string line;
    while (readBoundedLine(stream, line, 8192, source + ':' + std::to_string(lineNumber + 1))) {
        ++lineNumber;
        try {
            bytes += line.size() + 1;
            if (bytes > MaxFileBytes || line.size() > 8192)
                throw Error("circuit exceeds file or line length limit");
            line = line.substr(0, line.find('#'));
            for (const char raw : line) {
                const auto character = static_cast<unsigned char>(raw);
                if ((character < 32 && character != '\t' && character != '\r') || character > 126)
                    throw Error("unexpected non-ASCII or control character");
            }
            line = trim(line);
            if (line.empty())
                continue;
            if (line == ".chipsets:") {
                if (chipsets || links || graphics)
                    throw Error(".chipsets: must occur once before .links:");
                chipsets = true;
                section = 1;
                continue;
            }
            if (line == ".links:") {
                if (!chipsets || links || graphics)
                    throw Error(".links: must occur once after .chipsets:");
                links = true;
                section = 2;
                continue;
            }
            if (line == ".graphic:") {
#ifdef NTS_BONUS
                if (!links || graphics)
                    throw Error(".graphic: must occur once after .links:");
                graphics = true;
                section = 3;
                continue;
#else
                throw Error(".graphic: requires a build with bonuses enabled");
#endif
            }
            if (section == 2) {
                std::size_t position = 0;
                const auto a = endpoint(line, position), b = endpoint(line, position);
                if (!trim(line.substr(position)).empty())
                    throw Error("link has trailing fields");
                circuit.connect({a.first, a.second, b.first, b.second});
                continue;
            }
            std::istringstream tokens(line);
            std::string first, second, third, extra;
            if (!(tokens >> first >> second))
                throw Error("expected two fields");
            if (section == 1) {
                if (tokens >> extra)
                    throw Error("component declaration has trailing fields");
                identifier(first);
                identifier(second);
                circuit.add(first, second);
            } else if (section == 3) {
                if (!(tokens >> third) || (tokens >> extra))
                    throw Error("graphic position must use name x y");
                circuit.setLayout(first, {coordinate(second), coordinate(third)});
            } else
                throw Error("instruction outside a known section");
        } catch (const Error &error) {
            throw Error(source + ':' + std::to_string(lineNumber) + ": " + error.what());
        }
    }
    if (stream.bad())
        throw Error(source + ": read failure");
    if (!chipsets || !links || circuit.nodes().empty())
        throw Error(source + ": expected .chipsets:, at least one component, and .links:");
    return circuit;
}

} // namespace nts
