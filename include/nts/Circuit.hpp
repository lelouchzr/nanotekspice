#pragma once

#include "IComponent.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace nts {

struct Node {
    std::string type;
    std::unique_ptr<IComponent> component;
};

struct Link {
    std::string first;
    std::size_t firstPin = 0;
    std::string second;
    std::size_t secondPin = 0;
};

struct LayoutPosition {
    int x = 0;
    int y = 0;
};

class Circuit {
  public:
    void add(const std::string &type, const std::string &name);
    void connect(const Link &link);
    void setLayout(const std::string &name, LayoutPosition position);
    void setInput(const std::string &name, Tristate value);
    void simulate();
    [[nodiscard]] std::size_t tick() const noexcept;
    [[nodiscard]] IComponent &component(const std::string &name) const;
    [[nodiscard]] const std::map<std::string, Node, std::less<>> &nodes() const noexcept;
    [[nodiscard]] const std::vector<Link> &links() const noexcept;
    [[nodiscard]] const std::map<std::string, LayoutPosition, std::less<>> &layout() const noexcept;

  private:
    std::map<std::string, Node, std::less<>> nodes_;
    std::vector<Link> links_;
    std::map<std::string, LayoutPosition, std::less<>> layout_;
    std::size_t tick_ = 0;
};

} // namespace nts
