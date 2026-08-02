#include "nts/Error.hpp"

namespace nts {

Error::Error(const std::string &message) : std::runtime_error(message) {}

PinError::PinError(const std::string &message) : Error(message) {}

LinkError::LinkError(const std::string &message) : Error(message) {}

ComponentError::ComponentError(const std::string &message) : Error(message) {}

} // namespace nts
