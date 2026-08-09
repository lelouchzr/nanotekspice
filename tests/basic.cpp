#include "basic/BasicSupport.hpp"
#include <exception>
#include <iostream>

int main() {
    try {
        nts::test::factoryTests();
        nts::test::basicTruthTests();
        nts::test::sourceTests();
        nts::test::connectionTests();
        nts::test::phaseAndGuardTests();
        std::cout << nts::test::checks << " basic component checks passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "basic component test failed after " << nts::test::checks
                  << " checks: " << error.what() << '\n';
        return 1;
    }
}
