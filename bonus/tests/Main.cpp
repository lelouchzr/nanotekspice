#include "Support.hpp"
#include <exception>
#include <iostream>

int main(int argc, char **argv) {
    try {
        const auto examples =
            argc > 1 ? std::filesystem::path(argv[1]) / "bonus/examples"
                     : std::filesystem::path(__FILE__).parent_path().parent_path() / "examples";
        nts::bonus::test::integerTests();
        nts::bonus::test::cpuTests(examples);
        nts::bonus::test::displayTests();
        nts::bonus::test::graphTests(examples);
        nts::bonus::test::computerTests(examples);
        std::cout << nts::bonus::test::checks << " bonus checks passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "bonus tests: " << error.what() << '\n';
        return 1;
    }
}
