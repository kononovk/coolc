#pragma once

#include <util/type_traits.hpp>

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace coolc::expr {
namespace detail {

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

}  // namespace detail

struct LineNumbered {
  size_t line_number{0};
};

struct Expression;

struct LineNumberedAndTyped {
  size_t line_number{};
  std::optional<std::string> type_id;
};

struct UnaryExpressionBase : LineNumberedAndTyped {
  std::shared_ptr<Expression> arg;
};

struct BinaryExpressionBase : LineNumberedAndTyped {
  std::shared_ptr<Expression> lhs;
  std::shared_ptr<Expression> rhs;
};

template <typename T>
concept UnaryExpressionT = std::is_base_of_v<UnaryExpressionBase, T>;

template <typename T>
concept BinaryExpressionT = std::is_base_of_v<BinaryExpressionBase, T>;

struct Inversion : UnaryExpressionBase {};
struct IsVoid : UnaryExpressionBase {};
struct Not : UnaryExpressionBase {};

struct Plus : BinaryExpressionBase {};
struct Sub : BinaryExpressionBase {};
struct Mul : BinaryExpressionBase {};
struct Div : BinaryExpressionBase {};
struct Less : BinaryExpressionBase {};
struct LessEq : BinaryExpressionBase {};
struct Equal : BinaryExpressionBase {};
struct Empty {};

template <BinaryExpressionT T>
constexpr const char* GetBinaryExpressionName() {
  if constexpr (std::is_same_v<T, Plus>) {
    return "_plus";
  } else if constexpr (std::is_same_v<T, Sub>) {
    return "_sub";
  } else if constexpr (std::is_same_v<T, Mul>) {
    return "_mul";
  } else if constexpr (std::is_same_v<T, Div>) {
    return "_divide";
  } else if constexpr (std::is_same_v<T, Less>) {
    return "_lt";
  } else if constexpr (std::is_same_v<T, LessEq>) {
    return "_le";
  } else if constexpr (std::is_same_v<T, Equal>) {
    return "_eq";
  } else {
    assert(false);
  }
}

template <UnaryExpressionT T>
constexpr const char* GetUnaryExpressionName() {
  if constexpr (std::is_same_v<T, IsVoid>) {
    return "_isvoid";
  } else if constexpr (std::is_same_v<T, Inversion>) {
    return "_neg";
  } else if constexpr (std::is_same_v<T, Not>) {
    return "_comp";
  } else {
    assert(false);
  }
}

struct Int : LineNumbered {
  int32_t value;
};

struct If : LineNumberedAndTyped {
  std::shared_ptr<Expression> condition;
  std::shared_ptr<Expression> then_expr;
  std::shared_ptr<Expression> else_expr;
};

struct While : LineNumberedAndTyped {
  std::shared_ptr<Expression> condition;
  std::shared_ptr<Expression> loop_body;
};

struct Id : LineNumberedAndTyped {
  std::string name;
};

struct Assign : LineNumberedAndTyped {
  std::string identifier;
  std::shared_ptr<Expression> rhs;
};

struct String : LineNumberedAndTyped {
  std::string value;
};

struct Bool : LineNumberedAndTyped {
  bool value;
};

struct New : LineNumberedAndTyped {
  std::string type;
};

struct MethodCall : LineNumberedAndTyped {
  std::shared_ptr<Id> identifier;
  std::vector<std::shared_ptr<Expression>> expr;
};

struct Dispatch : LineNumbered {
  std::shared_ptr<Expression> expr;
  std::optional<std::string> type_id;
  std::shared_ptr<Id> object_id;
  std::vector<std::shared_ptr<Expression>> parameters;
};

struct FormalBase : LineNumberedAndTyped {
  std::string object_id;
};

struct Formal : FormalBase {
  void Print(int offset = 0) const {
    std::string offset_str = std::string(offset, ' ');
    std::cout << offset_str << "_formal" << std::endl;
    offset_str += "  ";
    std::cout << offset_str << object_id << std::endl << offset_str << *type_id << std::endl;
  }
};

struct Method : FormalBase {
  std::vector<Formal> formals;
  std::shared_ptr<Expression> expr;
};

struct Attribute : FormalBase {
  std::optional<std::shared_ptr<Expression>> expr;
};

struct Let : LineNumberedAndTyped {
  std::vector<Attribute> attrs;
  std::shared_ptr<Expression> expr;
};

struct Case : LineNumberedAndTyped {
  std::shared_ptr<Expression> expr;
  std::vector<Attribute> cases;
};

struct Block : LineNumbered {
  std::vector<std::shared_ptr<Expression>> expr;
};

template <typename T>
concept ExpressionT = IsOneOfV<T, Plus, Mul, Sub, Div, Less, LessEq, Equal, Inversion, Not, IsVoid, Int, Id, Assign, If,
                               Bool, New, String, While, Case>;  // TODO: add all expressions

template <ExpressionT T>
constexpr const char* GetExpressionName() {
  if constexpr (BinaryExpressionT<T>) {
    return GetBinaryExpressionName<T>();
  } else if constexpr (UnaryExpressionT<T>) {
    return GetUnaryExpressionName<T>();
  } else if (std::same_as<T, Id>) {
    return "_object";
  } else if (std::same_as<T, Int>) {
    return "_int";
  } else if (std::same_as<T, Assign>) {
    return "_assign";
  } else if (std::same_as<T, If>) {
    return "_cond";
  } else if (std::same_as<T, Bool>) {
    return "_bool";
  } else if (std::same_as<T, New>) {
    return "_new";
  } else if (std::same_as<T, String>) {
    return "_string";
  } else if (std::same_as<T, While>) {
    return "_loop";
  } else if (std::same_as<T, Case>) {
    return "_typcase";
  } else {
    assert(false);
  }
}

struct Expression {
  std::variant<Empty,                                     // Empty variant for errors
               Plus, Mul, Div, Sub, Less, LessEq, Equal,  // Binary expressions
               Int, String, Bool,                         // Type constants
               Id, New, MethodCall, Dispatch, Inversion, IsVoid, Not, Assign, If, While, Case, Let, Block>
      data_{Empty{}};

  template <ExpressionT T>
  bool Is() const {
    return std::holds_alternative<T>(data_);
  }

  template <ExpressionT T>
  T* As() {
    return std::get_if<T>(&data_);
  }

  template <ExpressionT T>
  const T* As() const {
    return std::get_if<T>(&data_);
  }

  void Print(int offset = 0) const {
    std::cout << std::string(offset, ' ');
    offset += 2;

    auto visitor =
        detail::overloaded{[offset](const BinaryExpressionT auto& expr) {
                             std::cout << "#" << expr.line_number << std::endl;
                             std::cout << std::string(offset - 2, ' ')
                                       << GetBinaryExpressionName<std::decay_t<decltype(expr)>>() << std::endl;
                             expr.lhs->Print(offset);
                             expr.rhs->Print(offset);
                             std::cout << std::string(offset - 2, ' ') << ": "
                                       << "_no_type" << std::endl;
                           },
                           [offset](const UnaryExpressionT auto& expr) {
                             std::cout << '#' << expr.line_number << std::endl;
                             std::cout << std::string(offset - 2, ' ')
                                       << GetUnaryExpressionName<std::decay_t<decltype(expr)>>() << std::endl;
                             expr.arg->Print(offset);
                             std::cout << std::string(offset - 2, ' ') << ": _no_type" << std::endl;
                           },
                           [offset](const Int& expr) {
                             std::cout << "#" << expr.line_number << std::endl;
                             std::cout << std::string(offset - 2, ' ') << GetExpressionName<Int>() << std::endl;
                             std::cout << std::string(offset, ' ') << expr.value << std::endl;
                             std::cout << std::string(offset - 2, ' ') << ": _no_type" << std::endl;
                           },
                           [offset](const String& expr) {
                             std::cout << "#" << expr.line_number << std::endl;
                             std::cout << std::string(offset - 2, ' ') << GetExpressionName<String>() << std::endl;
                             std::cout << std::string(offset, ' ') << '"' << expr.value << '"' << std::endl;
                             std::cout << std::string(offset - 2, ' ') << ": _no_type" << std::endl;
                           },
                           [offset](const Bool& expr) {
                             std::cout << "#" << expr.line_number << std::endl;
                             std::cout << std::string(offset - 2, ' ') << GetExpressionName<Bool>() << std::endl;
                             std::cout << std::string(offset, ' ') << expr.value << std::endl;
                             std::cout << std::string(offset - 2, ' ') << ": _no_type" << std::endl;
                           },
                           [offset](const If& expr) {
                             std::cout << "#" << expr.line_number << std::endl;
                             std::cout << std::string(offset - 2, ' ') << GetExpressionName<If>() << std::endl;
                             expr.condition->Print(offset);
                             expr.then_expr->Print(offset);
                             expr.else_expr->Print(offset);
                             std::cout << std::string(offset - 2, ' ') << ": _no_type" << std::endl;
                           },
                           [offset](const While& expr) {
                             std::cout << "#" << expr.line_number << std::endl;
                             std::cout << std::string(offset - 2, ' ') << GetExpressionName<While>() << std::endl;
                             expr.condition->Print(offset);
                             expr.loop_body->Print(offset);
                             std::cout << std::string(offset - 2, ' ') << ": _no_type" << std::endl;
                           },
                           [offset](const Block& expr) {
                             std::cout << "#" << expr.line_number << std::endl;
                             std::cout << std::string(offset - 2, ' ') << "_block" << std::endl;
                             for (const auto& el : expr.expr) {
                               // std::cout << std::string(offset - 2, ' ') << "Expression:" << std::endl;
                               el->Print(offset);
                             }
                             std::cout << std::string(offset - 2, ' ') << ": _no_type" << std::endl;
                           },
                           [offset](const Assign& expr) {
                             std::cout << "#" << expr.line_number << std::endl;
                             std::cout << std::string(offset - 2, ' ') << GetExpressionName<Assign>() << std::endl;
                             std::cout << std::string(offset, ' ') << expr.identifier << std::endl;
                             expr.rhs->Print(offset);
                             std::cout << std::string(offset - 2, ' ') << ": "
                                       << "_no_type" << std::endl;
                           },
                           [offset](const Id& expr) {
                             std::cout << '#' << expr.line_number << std::endl;  // TODO
                             std::cout << std::string(offset - 2, ' ') << GetExpressionName<Id>() << std::endl;
                             std::cout << std::string(offset, ' ') << expr.name << std::endl;
                             std::cout << std::string(offset - 2, ' ') << ": _no_type" << std::endl;
                           },
                           [offset](const MethodCall& expr) {
                             std::cout << "#" << expr.line_number << std::endl;
                             std::cout << std::string(offset - 2, ' ') << "_dispatch" << std::endl;
                             auto tmp_expr = Expression{Id{expr.identifier->line_number, {}, "self"}};
                             tmp_expr.Print(offset);
                             std::cout << std::string(offset, ' ') << expr.identifier->name << std::endl;
                             std::cout << std::string(offset, ' ') << '(' << std::endl;
                             for (const auto& el : expr.expr) {
                               el->Print(offset);
                             }
                             std::cout << std::string(offset, ' ') << ')' << std::endl;
                             std::cout << std::string(offset - 2, ' ') << ": _no_type" << std::endl;
                           },
                           [offset](const Let& expr) mutable {
                             const auto& args = expr.attrs;
                             for (std::size_t i = 0; i < args.size(); ++i) {
                               if (i == 0) {
                                 std::cout << '#' << expr.line_number << std::endl;
                               } else {
                                 std::cout << std::string(offset - 2, ' ') << '#' << expr.line_number << std::endl;
                               }
                               std::cout << std::string(offset - 2, ' ') << "_let" << std::endl;
                               std::cout << std::string(offset, ' ') << expr.attrs[i].object_id << std::endl;
                               std::cout << std::string(offset, ' ') << *expr.attrs[i].type_id << std::endl;
                               if (args[i].expr) {
                                 (*args[i].expr)->Print(offset);
                               } else {
                                 /// TODO!
                               }
                               offset += 2;
                             }
                             offset -= 2;
                             expr.expr->Print(offset);
                             for (std::size_t i = 0; i < args.size(); ++i) {
                               offset -= 2;
                               std::cout << std::string(offset, ' ') << ": _no_type" << std::endl;
                             }
                           },
                           [offset](const New& expr) {
                             std::cout << '#' << expr.line_number << std::endl;
                             std::cout << std::string(offset - 2, ' ') << GetExpressionName<New>() << std::endl;
                             std::cout << std::string(offset, ' ') << expr.type << std::endl;
                             std::cout << std::string(offset - 2, ' ') << ": _no_type" << std::endl;
                           },
                           [offset](const Case& expr) {
                             std::cout << '#' << expr.line_number << std::endl;
                             std::cout << std::string(offset - 2, ' ') << GetExpressionName<Case>() << std::endl;
                             expr.expr->Print(offset);
                             for (const auto& el : expr.cases) {
                               std::cout << std::string(offset, ' ') << '#' << el.line_number << std::endl;
                               std::cout << std::string(offset, ' ') << "_branch" << std::endl;
                               std::cout << std::string(offset + 2, ' ') << el.object_id << std::endl;
                               std::cout << std::string(offset + 2, ' ') << *el.type_id << std::endl;
                               (*el.expr)->Print(offset + 2);
                             }
                             std::cout << std::string(offset - 2, ' ') << ": _no_type" << std::endl;
                           },
                           [offset](const Empty& expr) {
                             std::cout << "#0" << std::endl;
                             std::cout << std::string(offset - 2, ' ') << "_no_expr" << std::endl;
                             std::cout << std::string(offset - 2, ' ') << ": _no_type" << std::endl;
                           },
                           [offset](const Dispatch& expr) {
                             std::cout << '#' << expr.line_number << std::endl;
                             if (expr.type_id) {
                               std::cout << std::string(offset - 2, ' ') << "_static_dispatch" << std::endl;
                             } else {
                               std::cout << std::string(offset - 2, ' ') << "_dispatch" << std::endl;
                             }
                             expr.expr->Print(offset);
                             if (expr.type_id) {
                               std::cout << std::string(offset, ' ') << *expr.type_id << std::endl;
                             }
                             std::cout << std::string(offset, ' ') << expr.object_id->name << std::endl;
                             std::cout << std::string(offset, ' ') << '(' << std::endl;
                             for (const auto& el : expr.parameters) {
                               el->Print(offset);
                             }
                             std::cout << std::string(offset, ' ') << ')' << std::endl;
                             std::cout << std::string(offset - 2, ' ') << ": "
                                       << "_no_type" << std::endl;
                           },
                           [](const auto& expr) {
                             assert(false);
                           }};

    std::visit(visitor, data_);
  }
};

struct Feature : LineNumbered {
  void Print(int offset = 0) const {
    auto offset_str = std::string(offset, ' ');
    if (std::holds_alternative<Method>(feature)) {
      auto* m_ptr = std::get_if<Method>(&feature);
      std::cout << offset_str << '#' << line_number << std::endl;
      std::cout << offset_str << "_method" << std::endl;
      offset += 2;
      offset_str += std::string(2, ' ');
      std::cout << offset_str << m_ptr->object_id << std::endl;
      for (const auto& el : m_ptr->formals) {
        std::cout << offset_str << '#' << el.line_number << std::endl;
        el.Print(offset);
      }
      std::cout << offset_str << *m_ptr->type_id << std::endl;
      m_ptr->expr->Print(offset);
    } else if (std::holds_alternative<Attribute>(feature)) {
      auto* a_ptr = std::get_if<Attribute>(&feature);
      std::cout << offset_str << '#' << line_number << std::endl;
      std::cout << offset_str << "_attr" << std::endl;
      offset_str += "  ";
      std::cout << offset_str << a_ptr->object_id << std::endl;
      std::cout << offset_str << *a_ptr->type_id << std::endl;
      (*a_ptr->expr)->Print(offset + 2);
    }
  }
  std::variant<Method, Attribute> feature;
};

struct Class : LineNumbered {
  void Print(int offset = 0) const {
    auto offset_str = std::string(offset, ' ');
    std::cout << offset_str << '#' << line_number << std::endl;
    std::cout << offset_str << "_class" << std::endl;
    offset += 2;
    offset_str += "  ";
    std::cout << offset_str << type << std::endl;
    std::cout << offset_str << inherits_type << std::endl;
    std::cout << offset_str << '"' << filename << '"' << std::endl;
    std::cout << offset_str << "(" << std::endl;
    for (const auto& el : features) {
      el.Print(offset);
    }
    std::cout << offset_str << ")" << std::endl;
  }
  std::string type;
  std::string inherits_type;
  std::vector<Feature> features;
  std::string filename;
};

struct Program : LineNumbered {
  std::vector<Class> classes;

  void Print(int offset = 0) const {
    std::cout << '#' << line_number << std::endl;
    std::cout << std::string(offset, ' ') << "_program" << std::endl;
    offset += 2;
    for (const auto& class_ : classes) {
      class_.Print(offset);
    }
  }
};

}  // namespace coolc::expr
