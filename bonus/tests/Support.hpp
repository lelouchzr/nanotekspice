#pragma once

#include "nts/Circuit.hpp"
#include <filesystem>
#include <functional>
#include <string>

namespace nts::bonus::test {

extern std::size_t checks;
void require(bool condition, const std::string &message);
void rejects(const std::function<void()> &operation, const std::string &message);
void addInputs(Circuit &circuit, const std::string &target, std::size_t count,
               std::size_t firstPin = 1);
void setBits(Circuit &circuit, const std::string &target, std::size_t count, std::uint32_t value,
             std::size_t firstPin = 1);
void integerTests();
void cpuTests(const std::filesystem::path &examples);
void displayTests();
void graphTests(const std::filesystem::path &examples);
void computerTests(const std::filesystem::path &examples);
void viewportTests();

} // namespace nts::bonus::test
