#include "ast/expression.hpp"

#include <iostream>
#include <variant>

namespace coolc {
namespace {

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
    return "_leq";
  } else if constexpr (std::is_same_v<T, Equal>) {
    return "_eq";
  } else {
    std::terminate();
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
    std::terminate();
  }
}

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
  } else if (std::same_as<T, Empty>) {
    return "_no_expr";
  } else if (std::same_as<T, Block>) {
    return "_block";
  } else if (std::same_as<T, Let>) {
    return "_let";
  } else {
    std::terminate();
  }
}

std::string MakeOffsetString(int offset) {
  return std::string(offset, ' ');
}

void PrintExpression(const Expression& expression, int offset) {
  std::string offset_str = MakeOffsetString(offset);

  std::visit(util::overloaded{[&offset_str](const ExpressionT auto& expr) {
               std::cout << offset_str << '#' << expr.line_number << std::endl;
               if constexpr (std::same_as<std::decay_t<decltype(expr)>, Dispatch>) {
                 std::cout << offset_str << (expr.type_id ? "_static" : "") << "_dispatch" << std::endl;
               } else {
                 std::cout << offset_str << GetExpressionName<std::decay_t<decltype(expr)>>() << std::endl;
               }
             }},
             expression.data_);

  offset += 2;
  offset_str += "  ";

  auto visitor = util::overloaded{
      [offset](const BinaryExpressionT auto& expr) {
        PrintExpression(*expr.lhs, offset);
        PrintExpression(*expr.rhs, offset);
      },
      [offset](const UnaryExpressionT auto& expr) { PrintExpression(*expr.arg, offset); },
      [&offset_str](const Int& expr) { std::cout << offset_str << expr.value << std::endl; },
      [&offset_str](const String& expr) { std::cout << offset_str << '"' << expr.value << '"' << std::endl; },
      [&offset_str](const Bool& expr) { std::cout << offset_str << expr.value << std::endl; },
      [&offset_str](const Id& expr) { std::cout << offset_str << expr.name << std::endl; },
      [&offset_str](const New& expr) { std::cout << offset_str << expr.type << std::endl; },
      [](const Empty&) {},
      [offset](const If& expr) {
        PrintExpression(*expr.condition, offset);
        PrintExpression(*expr.then_expr, offset);
        PrintExpression(*expr.else_expr, offset);
      },
      [offset](const While& expr) {
        PrintExpression(*expr.condition, offset);
        PrintExpression(*expr.loop_body, offset);
      },
      [offset](const Block& expr) {
        for (const auto& el : expr.expr) {
          PrintExpression(*el, offset);
        }
      },
      [&offset_str](const Assign& expr) {
        std::cout << offset_str << expr.identifier << std::endl;
        PrintExpression(*expr.rhs, offset_str.size());
      },
      [offset](const Let& expr) mutable {
        const auto& args = expr.attrs;
        for (std::size_t i = 0; i < args.size(); ++i) {
          if (i != 0) {
            std::cout << std::string(offset - 2, ' ') << '#' << expr.attrs[i].line_number << std::endl;
            std::cout << std::string(offset - 2, ' ') << "_let" << std::endl;
          }
          std::cout << std::string(offset, ' ') << expr.attrs[i].object_id << std::endl;
          std::cout << std::string(offset, ' ') << expr.attrs[i].type_id << std::endl;
          if (args[i].expr) {
            PrintExpression(*args[i].expr, offset);
          }
          offset += 2;
        }
        PrintExpression(*expr.expr, offset - 2);
        for (std::size_t i = 1; i < args.size(); ++i) {
          offset -= 2;
          std::cout << std::string(offset - 2, ' ') << ": _no_type" << std::endl;
        }
      },
      [offset, &offset_str](const Case& expr) {
        PrintExpression(*expr.expr, offset);
        for (const auto& el : expr.cases) {
          std::cout << offset_str << '#' << el.line_number << std::endl
                    << offset_str << "_branch" << std::endl
                    << offset_str << "  " << el.object_id << std::endl
                    << offset_str << "  " << el.type_id << std::endl;
          PrintExpression(*el.expr, offset + 2);
        }
      },
      [offset, &offset_str](const Dispatch& expr) {
        PrintExpression(*expr.expr, offset);
        if (expr.type_id) {
          std::cout << offset_str << *expr.type_id << std::endl;
        }
        std::cout << offset_str << expr.object_id->name << std::endl << offset_str << '(' << std::endl;
        for (const auto& el : expr.parameters) {
          PrintExpression(*el, offset);
        }
        std::cout << offset_str << ')' << std::endl;
      },
      [](const auto&) { std::terminate(); }};

  std::visit(visitor, expression.data_);
  std::cout << std::string(offset - 2, ' ') << ": " << expression.type << std::endl;
}

void PrintFormal(const Formal& f, int offset) {
  std::string offset_str = MakeOffsetString(offset);
  std::cout << offset_str << '#' << f.line_number << std::endl << offset_str << "_formal" << std::endl;
  offset_str += "  ";
  std::cout << offset_str << f.object_id << std::endl << offset_str << f.type_id << std::endl;
}

void PrintFeature(const Feature& f, int offset) {
  auto feature = f.feature;
  auto offset_str = MakeOffsetString(offset);

  // clang-format off
  auto visitor = util::overloaded {
    [&offset_str](const Method& m) {
      std::cout << offset_str << '#' << m.line_number << std::endl
                << offset_str << "_method" << std::endl;
      offset_str += std::string(2, ' ');
      std::cout << offset_str << m.object_id << std::endl;
      for (const auto& el : m.formals) {
        PrintFormal(el, offset_str.size());
      }
      std::cout << offset_str << m.type_id << std::endl;
      PrintExpression(*m.expr, offset_str.size());
    },
    [&offset_str](const Attribute& a) {
      std::cout << offset_str << '#' << a.line_number << std::endl << offset_str << "_attr" << std::endl;
      offset_str += "  ";
      std::cout << offset_str << a.object_id << std::endl << offset_str << a.type_id << std::endl;
      PrintExpression(*a.expr, offset_str.size());
    }
  };
  // clang-format on
  std::visit(visitor, feature);
}

void PrintClass(const Class& c, int offset) {
  auto offset_str = MakeOffsetString(offset);
  std::cout << offset_str << '#' << c.line_number << std::endl << offset_str << "_class" << std::endl;
  offset += 2;
  offset_str += "  ";
  std::cout << offset_str << c.type << std::endl
            << offset_str << c.inherits_type << std::endl
            << offset_str << '"' << c.filename << '"' << std::endl
            << offset_str << "(" << std::endl;
  for (const auto& el : c.features) {
    PrintFeature(el, offset);
  }
  std::cout << offset_str << ")" << std::endl;
}

}  // namespace

void PrintProgram(const Program& p, int offset) {
  std::cout << '#' << p.line_number << std::endl;
  std::cout << std::string(offset, ' ') << "_program" << std::endl;
  for (const auto& class_ : p.classes) {
    PrintClass(class_, offset + 2);
  }
}

}  // namespace coolc
