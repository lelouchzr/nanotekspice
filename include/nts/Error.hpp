#pragma once
#include <stdexcept>
#include <string>

namespace nts {

class Error : public std::runtime_error {
  public:
    explicit Error(const std::string &message);
};

class PinError : public Error {
  public:
    explicit PinError(const std::string &message);
};

class LinkError : public Error {
  public:
    explicit LinkError(const std::string &message);
};

class ComponentError : public Error {
  public:
    explicit ComponentError(const std::string &message);
};

} // namespace nts
