#include "Program.hpp"
#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>

namespace nts::bonus {

ProgramData Program::load(const std::filesystem::path &path) {
    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("cannot open SUBLEQ program: " + path.string());
    return parse(file, path.string());
}

Integer Program::resolve(const std::string &token, const std::map<std::string, Integer> &labels) {
    const auto found = labels.find(token);
    if (found != labels.end())
        return found->second;
    return Integer(token);
}

ProgramData Program::parse(std::istream &input, const std::string &source) {
    std::map<std::string, Integer> labels;
    std::vector<ProgramWord> words;
    std::set<Integer> addresses;
    Integer address;
    std::string entry = "0", line;
    std::size_t number = 0, bytes = 0;
    while (std::getline(input, line)) {
        ++number;
        bytes += line.size();
        if (line.size() > 65536 || bytes > 4 * 1024 * 1024)
            throw std::runtime_error("SUBLEQ program exceeds the 4 MiB input budget");
        line.resize(line.find_first_of("#;") == std::string::npos ? line.size()
                                                                  : line.find_first_of("#;"));
        std::istringstream fields(line);
        std::string token;
        if (!(fields >> token))
            continue;
        try {
            if (token.ends_with(':')) {
                token.pop_back();
                if (token.empty() ||
                    token.find_first_not_of(
                        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.") !=
                        std::string::npos ||
                    (token.front() >= '0' && token.front() <= '9') ||
                    !labels.emplace(token, address).second)
                    throw std::invalid_argument("invalid or duplicate label");
                if (!(fields >> token))
                    continue;
            }
            if (token == ".org" || token == ".entry") {
                std::string value, extra;
                if (!(fields >> value) || fields >> extra)
                    throw std::invalid_argument("directive expects one argument");
                if (token == ".entry")
                    entry = value;
                else {
                    address = Integer(value);
                    if (address.negative())
                        throw std::invalid_argument("negative .org address");
                }
                continue;
            }
            std::vector<std::string> values;
            const bool instruction = token == "subleq";
            if (token == ".halt") {
                values = {"0", "0", "-1"};
                std::string extra;
                if (fields >> extra)
                    throw std::invalid_argument(".halt takes no operands");
            } else if (token != ".word" && !instruction)
                values.push_back(token);
            while (fields >> token)
                values.push_back(token);
            if (values.empty() || (instruction && values.size() != 3))
                throw std::invalid_argument("subleq expects three operands; data needs words");
            for (const auto &value : values) {
                if (words.size() >= 1000000 || !addresses.insert(address).second)
                    throw std::invalid_argument("too many cells or overlapping .org sections");
                words.push_back({address, value, number});
                address += Integer(1);
            }
        } catch (const std::exception &error) {
            throw std::runtime_error(source + ":" + std::to_string(number) + ": " + error.what());
        }
    }
    if (input.bad())
        throw std::runtime_error("cannot read SUBLEQ program: " + source);
    if (words.empty())
        throw std::runtime_error("SUBLEQ program contains no words");
    ProgramData result;
    for (const auto &word : words) {
        try {
            result.memory.emplace(word.address, resolve(word.token, labels));
        } catch (const std::exception &error) {
            throw std::runtime_error(source + ":" + std::to_string(word.line) + ": " +
                                     error.what());
        }
    }
    result.entry = resolve(entry, labels);
    if (result.entry.negative())
        throw std::runtime_error("SUBLEQ entry must be nonnegative");
    return result;
}

} // namespace nts::bonus
