#pragma once

#include "util/type_traits.hpp"

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace coolc {

struct Expression;
struct Program;

/// used only for end-to-end parser tests
void PrintProgram(const Program& p, int offset = 0);

struct LineNumbered {
  size_t line_number{0};
};

/**
 * Program parts:
 */
struct TypedId : LineNumbered {
  std::string type_id;
  std::string object_id;
};

/// Formal = Id : Type
struct Formal : TypedId {};

/// Method = Id ([Formal]*) : Type { expr }
struct Method : TypedId {
  std::vector<Formal> formals;
  std::shared_ptr<Expression> expr;
};

/// Attribute = Id : Type [ <- expr ]
struct Attribute : TypedId {
  std::shared_ptr<Expression> expr;
};

/// Feature = Method | Attribute
struct Feature {
  std::variant<Method, Attribute> feature;
};

/// Class = class Type [ inherits Type ] { [Feature]* }
struct Class : LineNumbered {
  std::string type;
  std::string inherits_type;
  std::vector<Feature> features;
  std::string filename;
};

/// Program = [Class]+
struct Program : LineNumbered {
  std::vector<Class> classes;
};

/**
 * Expressions classes
 */

struct Empty : LineNumbered {};

/// Unary expressions
struct UnaryExpressionBase : LineNumbered {
  std::shared_ptr<Expression> arg;
};

template <typename T>
concept UnaryExpressionT = std::is_base_of_v<UnaryExpressionBase, T>;

struct Inversion : UnaryExpressionBase {};
struct IsVoid : UnaryExpressionBase {};
struct Not : UnaryExpressionBase {};

/// Binary expressions
struct BinaryExpressionBase : LineNumbered {
  std::shared_ptr<Expression> lhs;
  std::shared_ptr<Expression> rhs;
};

template <typename T>
concept BinaryExpressionT = std::is_base_of_v<BinaryExpressionBase, T>;

struct Plus : BinaryExpressionBase {};
struct Sub : BinaryExpressionBase {};
struct Mul : BinaryExpressionBase {};
struct Div : BinaryExpressionBase {};
struct Less : BinaryExpressionBase {};
struct LessEq : BinaryExpressionBase {};
struct Equal : BinaryExpressionBase {};

/// Literals
struct Int : LineNumbered {
  int32_t value;
};

struct String : LineNumbered {
  std::string value;
};

struct Bool : LineNumbered {
  bool value;
};

template <typename T>
concept IsBasicT = util::IsOneOfV<T, Bool, String, Int>;

/// Other
struct If : LineNumbered {
  std::shared_ptr<Expression> condition;
  std::shared_ptr<Expression> then_expr;
  std::shared_ptr<Expression> else_expr;
};

struct While : LineNumbered {
  std::shared_ptr<Expression> condition;
  std::shared_ptr<Expression> loop_body;
};

struct Id : LineNumbered {
  std::string name;
};

struct Assign : LineNumbered {
  std::string identifier;
  std::shared_ptr<Expression> rhs;
};

struct New : LineNumbered {
  std::string type;
};

struct Dispatch : LineNumbered {
  std::shared_ptr<Expression> expr;
  std::optional<std::string> type_id;
  std::shared_ptr<Id> object_id;
  std::vector<std::shared_ptr<Expression>> parameters;
};

struct Let : LineNumbered {
  std::shared_ptr<Expression> expr;
  std::vector<Attribute> attrs;
};

struct Case : LineNumbered {
  std::shared_ptr<Expression> expr;
  std::vector<Attribute> cases;
};

struct Block : LineNumbered {
  std::vector<std::shared_ptr<Expression>> expr;
};

template <typename T>
concept ExpressionT = BinaryExpressionT<T> || UnaryExpressionT<T> ||
                      util::IsOneOfV<T,
                                     Empty,              // Empty expression (default value)
                                     Int, String, Bool,  // Type constants
                                     Id, New, Dispatch, Assign, If, While, Case, Let, Block>;

template <typename T>
concept Arithmetic = util::IsOneOfV<T, Plus, Mul, Div, Sub>;

template <typename T>
concept Comparison = util::IsOneOfV<T, Less, LessEq>;

/**
 * Main Expression class
 */
struct Expression {
  std::variant<Empty,                                     // Empty expression (default value)
               Inversion, IsVoid, Not,                    // Unary expressions
               Plus, Mul, Div, Sub, Less, LessEq, Equal,  // Binary expressions
               Int, String, Bool,                         // Type constants
               Id, New, Dispatch, Assign, If, While, Case, Let, Block>
      data_{Empty{}};

  std::string type{"_no_type"};

  template <typename T>
  requires ExpressionT<std::remove_cvref_t<T>> Expression(T&& data) : data_{std::forward<T>(data)} {
  }

  template <ExpressionT T>
  constexpr bool Is() const noexcept {
    return std::holds_alternative<T>(data_);
  }

  template <ExpressionT T>
  constexpr T* As() noexcept {
    return std::get_if<T>(&data_);
  }

  template <ExpressionT T>
  constexpr const T* As() const noexcept {
    return std::get_if<T>(&data_);
  }
};

}  // namespace coolc
