#include "nts/Shell.hpp"
#include "nts/Error.hpp"
#include "nts/Text.hpp"
#include <chrono>
#include <csignal>
#include <iomanip>
#include <sstream>
#include <thread>

namespace nts {
namespace {

volatile std::sig_atomic_t interrupted = 0;

void stopLoop(int) {
    interrupted = 1;
}

std::string trim(const std::string &value) {
    const auto start = value.find_first_not_of(" \t\r");
    if (start == std::string::npos)
        return {};
    return value.substr(start, value.find_last_not_of(" \t\r") - start + 1);
}

} // namespace

Shell::Shell(Circuit &circuit, std::istream &input, std::ostream &output)
    : circuit_(circuit), input_(input), output_(output) {}

void Shell::display() {
    output_ << "tick: " << circuit_.tick() << "\ninput(s):\n";
    for (const auto &[name, node] : circuit_.nodes())
        if (node.component->isInputSource())
            output_ << "  " << name << ": " << toString(node.component->currentInput()) << '\n';
    output_ << "output(s):\n";
    for (const auto &[name, node] : circuit_.nodes())
        if (node.type == "output")
            output_ << "  " << name << ": " << toString(node.component->compute(1)) << '\n';
    output_.flush();
    if (!output_)
        throw Error("cannot write simulation output");
}

void Shell::loop() {
    interrupted = 0;
    const auto previous = std::signal(SIGINT, stopLoop);
    if (previous == SIG_ERR)
        throw Error("cannot install SIGINT handler");
    try {
        while (!interrupted) {
            circuit_.simulate();
            display();
            output_.flush();
            if (!output_)
                throw Error("cannot write simulation output");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } catch (...) {
        std::signal(SIGINT, previous);
        throw;
    }
    std::signal(SIGINT, previous);
}

bool Shell::execute(const std::string &text) {
    if (text.size() > 8192)
        throw Error("command exceeds 8192 bytes");
    const auto command = trim(text);
    if (command.empty())
        return true;
    if (command == "exit")
        return false;
    if (command == "display")
        display();
    else if (command == "simulate")
        circuit_.simulate();
    else if (command == "loop")
        loop();
#ifdef NTS_BONUS
    else if (command.starts_with("load ")) {
        std::istringstream tokens(command);
        std::string action, name, path, extra;
        if (!(tokens >> action >> name >> std::quoted(path)) || (tokens >> extra))
            throw Error("load expects component and program path");
        circuit_.component(name).loadProgram(path);
    }
#endif
    else {
        const auto equal = command.find('=');
        if (equal == std::string::npos || command.find('=', equal + 1) != std::string::npos)
            throw Error("unknown command '" + command + "'");
        const auto name = trim(command.substr(0, equal));
        const auto value = trim(command.substr(equal + 1));
        if (value != "0" && value != "1" && value != "U")
            throw Error("assignment value must be 0, 1 or U");
        circuit_.setInput(name, value == "0" ? False : value == "1" ? True : Undefined);
    }
    return true;
}

void Shell::run() {
    std::string command;
    while (true) {
        output_ << "> " << std::flush;
        if (!output_)
            throw Error("cannot write command prompt");
        if (!readBoundedLine(input_, command, 8192, "command input"))
            break;
        if (!execute(command))
            break;
    }
}

} // namespace nts
