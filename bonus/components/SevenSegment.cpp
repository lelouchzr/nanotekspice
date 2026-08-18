#include "SevenSegment.hpp"
#include <algorithm>
#include <array>

namespace nts::bonus {

SevenSegment::SevenSegment() : Component("7seg", 4) {
    for (std::size_t pin = 1; pin <= 4; ++pin)
        definePin(pin, PinDirection::Input);
}

Drive SevenSegment::evaluate(std::size_t) {
    return Drive::HighImpedance;
}

void SevenSegment::onPrepare(std::size_t) {
    for (std::size_t bit = 0; bit < bits_.size(); ++bit)
        pending_[bit] = read(bit + 1);
}

void SevenSegment::onSimulate(std::size_t) {
    bits_ = pending_;
}

std::optional<DisplayFrame> SevenSegment::visualization() const {
    constexpr std::array<unsigned, 16> masks{0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07,
                                             0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71};
    constexpr std::array<std::array<std::size_t, 4>, 7> rectangles{{
        {12, 4, 40, 7},
        {52, 12, 7, 36},
        {52, 55, 7, 36},
        {12, 92, 40, 7},
        {5, 55, 7, 36},
        {5, 12, 7, 36},
        {12, 48, 40, 7},
    }};
    DisplayFrame frame;
    frame.kind = DisplayKind::SevenSegment;
    frame.width = 64;
    frame.height = 104;
    frame.pixels.assign(frame.width * frame.height, 0x0b101a);
    const bool known =
        std::none_of(bits_.begin(), bits_.end(), [](Tristate value) { return value == Undefined; });
    frame.known.assign(frame.pixels.size(), known);
    unsigned number = 0;
    for (std::size_t bit = 0; bit < bits_.size(); ++bit)
        if (bits_[bit] == True)
            number |= 1U << bit;
    frame.text = known ? std::string(1, "0123456789ABCDEF"[number]) : "U";
    for (std::size_t segment = 0; segment < rectangles.size(); ++segment) {
        const auto &box = rectangles[segment];
        const bool lit = known && (masks[number] & (1U << segment));
        for (std::size_t y = box[1]; y < box[1] + box[3]; ++y)
            for (std::size_t x = box[0]; x < box[0] + box[2]; ++x)
                frame.pixels[y * frame.width + x] = lit ? 0xf3a85c : 0x28303e;
    }
    return frame;
}

} // namespace nts::bonus
