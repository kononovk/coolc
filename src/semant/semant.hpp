#pragma once

#include "ast/expression.hpp"
#include "semant/error.hpp"
#include "semant/inheritance_graph.hpp"
#include "semant/scope.hpp"

#include <stack>

namespace coolc {

using MaybeType = std::optional<std::string>;

class Semant {
 public:
  Semant(Program&& p);

  bool CheckProgram();

  const Program& GetProgram() const;

  bool CheckClasses();
  bool FillContent(const Class& cl);
  bool CheckClass(const Class& cl);
  bool CheckFeature(const Feature& f);
  bool CheckMethod(const Method& m);
  bool CheckAttribute(const Attribute& a);

  MaybeType CheckInversion(const Inversion& expr);
  MaybeType CheckIsVoid(const IsVoid&);
  MaybeType CheckNot(const Not& a);
  MaybeType CheckBlock(const Block& a);
  MaybeType CheckIf(const If& a);
  MaybeType CheckWhile(const While& a);
  MaybeType CheckId(const Id& a);
  MaybeType CheckEqual(const Equal& a);
  MaybeType CheckAssignment(const Assign& a);
  MaybeType CheckLet(const Let& a);
  MaybeType CheckCase(const Case& a);
  MaybeType CheckDispatch(const Dispatch& a);
  MaybeType CheckNew(const New& a);

  template <Arithmetic T>
  MaybeType CheckArithmetic(const T& expr);

  template <Comparison T>
  MaybeType CheckComparison(const T& a);

  MaybeType CheckExpression(std::shared_ptr<Expression> expr);

 private:
  Program _p;
  InheritanceGraph _ig;
  Scope _ctx;
};

}  // namespace coolc
