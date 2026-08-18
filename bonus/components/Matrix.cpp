#include "Matrix.hpp"

namespace nts::bonus {

Matrix::Matrix() : Component("matrix", 20) {
    for (std::size_t pin = 1; pin <= 20; ++pin)
        definePin(pin, PinDirection::Input);
}

Drive Matrix::evaluate(std::size_t) {
    return Drive::HighImpedance;
}

void Matrix::onPrepare(std::size_t) {
    pending_.reset();
    std::size_t address = 0;
    for (std::size_t bit = 0; bit < 12; ++bit) {
        const auto value = read(9 + bit);
        if (value == Undefined)
            return;
        if (value == True)
            address |= std::size_t{1} << bit;
    }
    unsigned shade = 0;
    bool known = true;
    for (std::size_t bit = 0; bit < 8; ++bit) {
        const auto value = read(1 + bit);
        known = known && value != Undefined;
        if (value == True)
            shade |= 1U << bit;
    }
    const unsigned red = ((shade >> 5U) & 7U) * 255U / 7U;
    const unsigned green = ((shade >> 2U) & 7U) * 255U / 7U;
    const unsigned blue = (shade & 3U) * 85U;
    pending_ = MatrixWrite{address, (red << 16U) | (green << 8U) | blue, known};
}

void Matrix::onSimulate(std::size_t) {
    if (!pending_)
        return;
    pixels_[pending_->address] = pending_->color;
    known_[pending_->address] = pending_->known;
}

std::optional<DisplayFrame> Matrix::visualization() const {
    DisplayFrame frame;
    frame.kind = DisplayKind::PixelMatrix;
    frame.width = frame.height = 64;
    frame.pixels.assign(pixels_.begin(), pixels_.end());
    frame.known.assign(known_.begin(), known_.end());
    frame.text = "64x64 / RGB332 / address: X[5:0],Y[5:0]";
    return frame;
}

} // namespace nts::bonus
