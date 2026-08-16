#include "Integer.hpp"
#include <algorithm>
#include <stdexcept>

namespace nts::bonus {

Integer::Integer() = default;

Integer::Integer(std::int64_t value) : Integer(std::to_string(value)) {}

Integer::Integer(std::string_view value) {
    if (value.empty())
        throw std::invalid_argument("empty integer");
    if (value.front() == '-' || value.front() == '+') {
        negative_ = value.front() == '-';
        value.remove_prefix(1);
    }
    if (value.empty() || value.find_first_not_of("0123456789") != std::string_view::npos)
        throw std::invalid_argument("invalid integer literal");
    magnitude_ = value;
    normalize();
}

void Integer::normalize() {
    const auto first = magnitude_.find_first_not_of('0');
    magnitude_ = first == std::string::npos ? "0" : magnitude_.substr(first);
    if (magnitude_ == "0")
        negative_ = false;
}

bool Integer::negative() const {
    return negative_;
}

bool Integer::zero() const {
    return magnitude_ == "0";
}

std::string Integer::string() const {
    return negative_ ? "-" + magnitude_ : magnitude_;
}

int Integer::compareMagnitude(const Integer &other) const {
    if (magnitude_.size() != other.magnitude_.size())
        return magnitude_.size() < other.magnitude_.size() ? -1 : 1;
    return magnitude_ == other.magnitude_ ? 0 : magnitude_ < other.magnitude_ ? -1 : 1;
}

std::string Integer::addMagnitude(std::string_view first, std::string_view second) {
    std::string result;
    result.reserve(std::max(first.size(), second.size()) + 1);
    std::size_t a = first.size(), b = second.size();
    int carry = 0;
    while (a || b || carry) {
        const int value = (a ? first[--a] - '0' : 0) + (b ? second[--b] - '0' : 0) + carry;
        result.push_back(static_cast<char>('0' + value % 10));
        carry = value / 10;
    }
    std::reverse(result.begin(), result.end());
    return result;
}

std::string Integer::subtractMagnitude(std::string_view first, std::string_view second) {
    std::string result;
    result.reserve(first.size());
    std::size_t a = first.size(), b = second.size();
    int borrow = 0;
    while (a) {
        int value = first[--a] - '0' - borrow - (b ? second[--b] - '0' : 0);
        borrow = value < 0 ? 1 : 0;
        if (value < 0)
            value += 10;
        result.push_back(static_cast<char>('0' + value));
    }
    std::reverse(result.begin(), result.end());
    return result;
}

Integer &Integer::operator+=(const Integer &other) {
    if (negative_ == other.negative_)
        magnitude_ = addMagnitude(magnitude_, other.magnitude_);
    else {
        const int order = compareMagnitude(other);
        if (order >= 0)
            magnitude_ = subtractMagnitude(magnitude_, other.magnitude_);
        else {
            magnitude_ = subtractMagnitude(other.magnitude_, magnitude_);
            negative_ = other.negative_;
        }
    }
    normalize();
    return *this;
}

Integer &Integer::operator-=(const Integer &other) {
    return *this += -other;
}

Integer Integer::operator-() const {
    auto result = *this;
    if (!zero())
        result.negative_ = !result.negative_;
    return result;
}

Integer Integer::operator+(const Integer &other) const {
    auto result = *this;
    return result += other;
}

Integer Integer::operator-(const Integer &other) const {
    auto result = *this;
    return result -= other;
}

bool Integer::operator==(const Integer &other) const {
    return negative_ == other.negative_ && magnitude_ == other.magnitude_;
}

bool Integer::operator<(const Integer &other) const {
    if (negative_ != other.negative_)
        return negative_;
    return negative_ ? compareMagnitude(other) > 0 : compareMagnitude(other) < 0;
}

bool Integer::operator<=(const Integer &other) const {
    return *this < other || *this == other;
}

std::uint32_t Integer::lowBits(unsigned bits) const {
    if (bits == 0 || bits > 24)
        throw std::invalid_argument("integer lowBits expects between 1 and 24 bits");
    const std::uint32_t modulus = 1U << bits;
    std::uint32_t result = 0;
    for (char digit : magnitude_)
        result = (result * 10U + static_cast<std::uint32_t>(digit - '0')) % modulus;
    return negative_ && result ? modulus - result : result;
}

} // namespace nts::bonus
