#include "Workload.hpp"
#include <charconv>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>

namespace {
std::size_t number(const std::string &text) {
    std::size_t result = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), result);
    if (error != std::errc{} || end != text.data() + text.size() || result == 0)
        throw std::invalid_argument("expected a positive integer");
    return result;
}
} // namespace

int main(int argc, char **argv) {
    try {
        if (argc != 4 && argc != 5)
            throw std::invalid_argument(
                "nts_benchmark fanout|cascade|counters size iterations [profile-seconds]");
        const auto size = number(argv[2]), iterations = number(argv[3]);
        nts::benchmark::Workload workload(argv[1], size);
        const auto start = std::chrono::steady_clock::now();
        const double minimum = argc == 5 ? static_cast<double>(number(argv[4])) : 0;
        std::uint64_t digest = 0;
        std::size_t rounds = 0;
        do {
            digest ^= workload.run(iterations);
            ++rounds;
        } while (std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count() <
                 minimum);
        const double elapsed =
            std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        std::cout << "{\"case\":\"" << argv[1] << "\",\"size\":" << size
                  << ",\"iterations\":" << iterations << ",\"rounds\":" << rounds
                  << ",\"seconds\":" << elapsed << ",\"digest\":" << digest
                  << ",\"pid\":" << getpid() << "}\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 84;
    }
}
