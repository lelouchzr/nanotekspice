#include "advanced/Support.hpp"
#include <exception>
#include <iostream>

int main() {
    std::string stage;
    try {
        advanced_test::Directory directory;
        stage = "combinational";
        advanced_test::combinational();
        stage = "flipflops";
        advanced_test::flipflops();
        stage = "counters";
        advanced_test::counters();
        stage = "shiftAndLatch";
        advanced_test::shiftAndLatch();
        stage = "memories";
        advanced_test::memories();
        stage = "logger";
        advanced_test::logger();
        std::cout << advanced_test::checks() << " advanced component checks passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "advanced tests (" << stage << "): " << error.what() << '\n';
        return 1;
    }
}
