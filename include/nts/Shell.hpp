#pragma once

#include "Circuit.hpp"
#include <istream>
#include <ostream>
#include <string>

namespace nts {

class Shell {
  public:
    Shell(Circuit &circuit, std::istream &input, std::ostream &output);
    void run();
    void display();
    [[nodiscard]] bool execute(const std::string &command);

  private:
    Circuit &circuit_;
    std::istream &input_;
    std::ostream &output_;
    void loop();
};

} // namespace nts
