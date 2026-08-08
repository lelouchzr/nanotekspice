#include "nts/Text.hpp"
#include "nts/Error.hpp"

namespace nts {

bool readBoundedLine(std::istream &input, std::string &line, std::size_t limit,
                     const std::string &source) {
    line.clear();
    char character = 0;
    while (input.get(character)) {
        if (character == '\n')
            return true;
        if (line.size() == limit)
            throw Error(source + ": line exceeds " + std::to_string(limit) + " bytes");
        line.push_back(character);
    }
    if (input.bad() || (!input.eof() && input.fail()))
        throw Error(source + ": read failure");
    return !line.empty();
}

} // namespace nts
