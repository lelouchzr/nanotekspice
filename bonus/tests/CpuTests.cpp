#include "Support.hpp"
#include "cpu/Subleq.hpp"
#include <sstream>

namespace nts::bonus::test {

void cpuTests(const std::filesystem::path &examples) {
    Subleq machine;
    machine.load(Program::load(examples / "triangular.subleq"));
    require(machine.run(1000) > 40 && machine.status() == CpuStatus::Halted,
            "CPU arithmetic loop did not halt");
    require(machine.output().size() == 1 && machine.output()[0].value == 55,
            "CPU failed sum 10+9+...+1");
    machine.load(Program::load(examples / "countdown.subleq"));
    machine.run(100);
    require(machine.output().size() == 5, "CPU countdown length");
    for (std::size_t i = 0; i < 5; ++i)
        require(machine.output()[i].value == 5 - i, "CPU conditional branch or loop is incorrect");
    machine.load(Program::load(examples / "hello.subleq"));
    machine.run(100);
    std::string hello;
    for (const auto &output : machine.output())
        hello.push_back(static_cast<char>(output.value));
    require(hello == "Hello\n", "CPU serial program did not execute");
    machine.load(Program::load(examples / "echo.subleq"));
    require(!machine.step() && machine.status() == CpuStatus::Waiting &&
                machine.instructions() == 0,
            "CPU input without data did not wait safely");
    machine.input(65);
    machine.run(20);
    require(machine.output().size() == 1 && machine.output()[0].value == 65,
            "CPU queued input did not resume");
    machine.load(Program::load(examples / "paint.subleq"));
    machine.run(25000);
    require(machine.status() == CpuStatus::Halted && machine.output().size() == 4096,
            "CPU framebuffer program did not paint all addresses");
    for (std::size_t i = 0; i < 4096; ++i)
        require(!machine.output()[i].serial && machine.output()[i].address == i &&
                    machine.output()[i].value == i % 256,
                "CPU framebuffer port address/color sequence");
    std::istringstream sparse(".entry code\n.org 10000000000000000000000000000000000000000000000\n"
                              "code: subleq source destination stop\nstop: .halt\n"
                              "source: .word 999999999999999999999999999999999999999999\n"
                              "destination: .word -1\n");
    const auto program = Program::parse(sparse, "sparse-test");
    machine.load(program);
    require(machine.step() && machine.pc() == program.entry + Integer(3),
            "CPU did not branch at an arbitrary precision address");
    require(machine.read(program.entry + Integer(7)).string() ==
                "-1000000000000000000000000000000000000000000",
            "CPU subtraction wrapped at a fixed word width");
    std::istringstream loop("subleq zero zero 0\nzero: .word 0\n");
    machine.load(Program::parse(loop, "loop"));
    require(machine.run(1234) == 1234 && machine.status() == CpuStatus::Ready,
            "CPU ignored execution budget on an infinite program");
    for (const std::string bad :
         {"", "x: .word 1\nx: .word 2", ".org -1\n.word 0", ".halt extra", ".entry missing\n.halt",
          "subleq 1 2", ".word 3\n.org 0\n.word 4"})
        rejects(
            [&] {
                std::istringstream input(bad);
                (void)Program::parse(input, "bad");
            },
            "CPU assembler accepted malformed program");
    Subleq limited(1);
    rejects([&] { limited.load(program); }, "CPU ignored physical memory budget");
    machine.reset();
    require(machine.instructions() == 0 && machine.output().empty(),
            "CPU reset did not restore image");
}

} // namespace nts::bonus::test
