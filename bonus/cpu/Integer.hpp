#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace nts::bonus {

class Integer {
  public:
    Integer();
    explicit Integer(std::int64_t value);
    explicit Integer(std::string_view value);
    [[nodiscard]] bool negative() const;
    [[nodiscard]] bool zero() const;
    [[nodiscard]] std::string string() const;
    [[nodiscard]] std::uint32_t lowBits(unsigned bits) const;
    Integer &operator+=(const Integer &other);
    Integer &operator-=(const Integer &other);
    [[nodiscard]] Integer operator-() const;
    [[nodiscard]] bool operator==(const Integer &other) const;
    [[nodiscard]] bool operator<(const Integer &other) const;
    [[nodiscard]] bool operator<=(const Integer &other) const;
    [[nodiscard]] Integer operator+(const Integer &other) const;
    [[nodiscard]] Integer operator-(const Integer &other) const;

  private:
    bool negative_ = false;
    std::string magnitude_ = "0";
    void normalize();
    [[nodiscard]] int compareMagnitude(const Integer &other) const;
    static std::string addMagnitude(std::string_view first, std::string_view second);
    static std::string subtractMagnitude(std::string_view first, std::string_view second);
};

} // namespace nts::bonus
