#include "Support.hpp"
#include <algorithm>

namespace nts::bonus::test {

void displayTests() {
    Circuit digit;
    digit.add("7seg", "hex");
    addInputs(digit, "hex", 4);
    auto view = digit.component("hex").visualization();
    require(view && view->text == "U", "7seg invented an initial defined value");
    for (unsigned value = 0; value < 16; ++value) {
        setBits(digit, "hex", 4, value);
        digit.simulate();
        view = digit.component("hex").visualization();
        require(view && view->text == std::string(1, "0123456789ABCDEF"[value]),
                "7seg hexadecimal pin order is incorrect");
        require(view->pixels.size() == view->width * view->height &&
                    std::count(view->pixels.begin(), view->pixels.end(), 0xf3a85cU) > 0,
                "7seg has no illuminated segments");
    }
    digit.setInput("hex_bit2", Undefined);
    digit.simulate();
    require(digit.component("hex").visualization()->text == "U", "7seg coerced unknown input");
    rejects([&] { (void)digit.component("hex").compute(5); }, "7seg accepted pin5");
    rejects([&] { digit.component("hex").loadProgram("unused"); }, "7seg accepted CPU program");
    Circuit matrix;
    matrix.add("matrix", "screen");
    addInputs(matrix, "screen", 20);
    auto pixels = matrix.component("screen").visualization();
    require(pixels && pixels->width == 64 && pixels->height == 64 &&
                std::count(pixels->known.begin(), pixels->known.end(), true) == 0,
            "matrix dimensions or unwritten state are wrong");
    for (unsigned address = 0; address < 4096; ++address) {
        setBits(matrix, "screen", 8, address % 256);
        setBits(matrix, "screen", 12, address, 9);
        matrix.simulate();
    }
    pixels = matrix.component("screen").visualization();
    require(std::count(pixels->known.begin(), pixels->known.end(), true) == 4096,
            "matrix failed to address all64x64 pixels");
    for (std::size_t address = 0; address < 4096; ++address) {
        const auto value = static_cast<unsigned>(address % 256);
        const auto expected = ((((value >> 5U) & 7U) * 255U / 7U) << 16U) |
                              ((((value >> 2U) & 7U) * 255U / 7U) << 8U) | ((value & 3U) * 85U);
        require(pixels->pixels[address] == expected, "matrix color shade or address mapping");
    }
    const auto before = pixels->pixels;
    matrix.setInput("screen_bit9", Undefined);
    setBits(matrix, "screen", 8, 0);
    matrix.simulate();
    require(matrix.component("screen").visualization()->pixels == before,
            "matrix wrote a guessed unknown address");
    setBits(matrix, "screen", 12, 4095, 9);
    matrix.setInput("screen_bit1", Undefined);
    matrix.simulate();
    require(!matrix.component("screen").visualization()->known[4095],
            "matrix coerced an undefined color into a known pixel");
    rejects([&] { (void)matrix.component("screen").compute(21); }, "matrix accepted pin21");
}

} // namespace nts::bonus::test
