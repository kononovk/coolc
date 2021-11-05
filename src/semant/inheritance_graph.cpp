#include "semant/inheritance_graph.hpp"

#include "ast/expression.hpp"

#include <cassert>
#include <deque>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace coolc {

InheritanceGraph::InheritanceGraph() {
  _classes_graph = {
      {"Object", "Object"}, {"Int", "Object"}, {"String", "Object"}, {"Bool", "Object"}, {"IO", "Object"}};
}

void InheritanceGraph::Reserve(std::size_t size) {
  _classes_graph.reserve(size);
}

bool InheritanceGraph::InsertClass(const Class& cl) {
  static constexpr std::array fund = {"Int", "String", "Bool"};

  if (cl.type == "SELF_TYPE") {
    std::cerr << MakeError(cl, "Redefinition of basic class SELF_TYPE.");
    return false;
  }

  if (IsBasic(cl.type)) {
    std::cerr << MakeError(cl, "Redefinition of basic class " + cl.type + ".");
    return false;
  }

  if (std::find(fund.begin(), fund.end(), cl.inherits_type) != fund.end()) {
    std::cerr << MakeError(cl, "Class " + cl.type + " cannot inherit class " + cl.inherits_type);
    return false;
  }

  if (_classes_graph.contains(cl.type)) {
    std::cerr << MakeError(cl, "Class " + cl.type + " was previously defined.");
    return false;
  }
  _classes_graph[cl.type] = cl.inherits_type;
  return true;
}

bool InheritanceGraph::CheckAncessorDefined(const Class& cl) const {
  if (!_classes_graph.contains(cl.inherits_type)) {
    std::cerr << MakeError(cl, "Class " + cl.type + " inherits from undefined class " + cl.inherits_type + ".");
    return false;
  }
  return true;
}

// pre-condition: all base classes must be in classes graph as keys
bool InheritanceGraph::CheckAcyclic() const {
  std::unordered_map<std::string_view, bool> visited(_classes_graph.size());
  std::function<bool(std::string_view)> dfs;

  // returns true if connectivity component is acyclic
  dfs = [&](std::string_view v) {
    if (visited[v]) {
      return false;
    }
    visited[v] = true;
    bool is_acyclic = true;
    if (_classes_graph.at(std::string{v}) != "Object") {
      is_acyclic = dfs(_classes_graph.at(std::string{v}));
    }
    visited[v] = false;
    return is_acyclic;
  };

  bool is_acyclic = true;
  for (const auto& el : _classes_graph) {
    if (!dfs(el.first)) {
      std::cerr << Error{"Class " + el.first + ", or an ancestor of " + el.first +
                         ", is involved in an inheritance cycle."};
      is_acyclic = false;
    }
  }
  return is_acyclic;
}

bool InheritanceGraph::HasMain() const {
  return _classes_graph.contains("Main") || (std::cerr << Error{"Class Main is not defined."}, false);
}

bool InheritanceGraph::IsBasic(std::string_view class_name) {
  return std::find(fundamentals.begin(), fundamentals.end(), class_name) != fundamentals.end();
}

void InheritanceGraph::CalculateDepth(const std::string& class_name) {
  if (class_name == "Object") {
    _height[class_name] = 0;
  }
  const auto& base = _classes_graph[class_name];

  if (!_height.contains(base)) {
    CalculateDepth(base);
  }
  _height[class_name] = _height[base] + 1;
}

// pre-condition: CheckAndFill Method must be called
std::string InheritanceGraph::GetLca(std::string_view left, std::string_view right) const {
  int left_h = _height.at(left);
  std::vector<std::string_view> left_path(left_h + 1);
  for (; left_h >= 0; left_h--) {
    left_path[left_h] = left;
    left = _classes_graph.at(std::string{left});
  }

  int right_h = _height.at(right);
  std::vector<std::string_view> right_path(right_h + 1);
  for (; right_h >= 0; right_h--) {
    right_path[right_h] = right;
    right = _classes_graph.at(std::string{right});
  }

  std::size_t i = 0;
  for (; i < std::min(left_path.size(), right_path.size()); i++) {
    if (left_path[i] != right_path[i]) {
      break;
    }
  }
  return std::string{left_path[i - 1]};
}

bool InheritanceGraph::FillAndCheck(const Program& p) {
  bool correct = true;

  for (const auto& cl : p.classes) {
    correct &= InsertClass(cl);
  }

  for (const auto& cl : p.classes) {
    correct &= CheckAncessorDefined(cl);
  }

  CHECK_ERROR(correct && CheckAcyclic() && HasMain())

  for (const auto& el : _classes_graph) {
    if (!_height.contains(el.first)) {
      CalculateDepth(el.first);
    }
  }

  return true;
}

}  // namespace coolc
