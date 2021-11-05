#pragma once
#include "ast/expression.hpp"
#include "semant/error.hpp"

#include <array>
#include <cassert>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define CHECK_NULLOPT(flag) \
  {                         \
    if (!(flag))            \
      return {};            \
  }

#define CHECK_ERROR(flag) \
  {                       \
    if (!(flag))          \
      return false;       \
  }

namespace coolc {

class InheritanceGraph {
 public:
  constexpr static std::array fundamentals{"String", "IO", "Int", "Bool", "Object"};

  InheritanceGraph();
  void Reserve(std::size_t size);
  bool InsertClass(const Class& cl);

  bool IsBasic(std::string_view class_name);

  bool FillAndCheck(const Program& p);

  // pre-condition: all base classes must be in classes_graph as keys
  // true if correct
  bool CheckAcyclic() const;

  bool HasMain() const;

  bool HasClass(std::string class_name) const {
    return _classes_graph.contains(class_name);
  }

  void CalculateDepth(const std::string& class_name);

  bool CheckAncessorDefined(const Class& cl) const;

  std::string GetAncessor(std::string class_name) const {
    return _classes_graph.at(class_name);
  }

  bool IsAncessor(std::string_view base, std::string_view derived) const {
    if (derived == "SELF_TYPE") {
      return true;
    }
    while (derived != "Object") {
      if (derived == base) {
        return true;
      }
      derived = _classes_graph.at(std::string(derived));
    }
    return base == derived;
  }

  std::string GetLca(std::string_view left, std::string_view right) const;

 private:
  std::unordered_map<std::string_view, std::size_t> _height;
  std::unordered_map<std::string, std::string> _classes_graph;
};

}  // namespace coolc
