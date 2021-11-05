#include "semant/semant.hpp"

#include "ast/expression.hpp"
#include "semant/error.hpp"
#include "semant/inheritance_graph.hpp"

namespace coolc {

namespace {
bool CheckCaseNoIdenticalBranches(const Case& a) {
  std::unordered_set<std::string> case_branches{};
  for (const auto& el : a.cases) {
    if (!case_branches.insert(el.type_id).second) {
      std::cerr << "Case identical branches" << std::endl;
      return false;
    }
  }
  return true;
}
}  // namespace

Semant::Semant(Program&& p) : _p(std::move(p)), _ig{}, _ctx(_ig) {
}

bool Semant::CheckProgram() {
  CHECK_ERROR(_ig.FillAndCheck(_p))
  CHECK_ERROR(CheckClasses())
  return true;
}

const Program& Semant::GetProgram() const {
  return _p;
}

bool Semant::FillContent(const Class& cl) {
  ScopeGuard ctx_guard(&_ctx, cl);

  for (auto i : cl.features) {
    bool err =
        std::visit(util::Overloaded{[&](const Attribute& a) { return _ctx.AddAttribute(a.object_id, a.type_id); },
                                    [&](const Method& a) {
                                      std::vector<std::string> arg_types;
                                      for (const auto& el : a.formals) {
                                        arg_types.emplace_back(el.type_id);
                                      }
                                      return _ctx.AddMethod(a.object_id, a.type_id, std::move(arg_types));
                                    }},
                   i.feature);
    CHECK_ERROR(err)
  }
  return true;
}

bool Semant::CheckClasses() {
  for (auto& cl : _p.classes) {
    CHECK_ERROR(FillContent(cl))
  }

  for (auto& cl : _p.classes) {
    CHECK_ERROR(CheckClass(cl))
  }
  return true;
}

bool Semant::CheckClass(const Class& cl) {
  ScopeGuard new_scope(&_ctx, cl);
  for (const auto& el : cl.features) {
    CHECK_ERROR(CheckFeature(el))
  }
  return true;
}

bool Semant::CheckFeature(const Feature& f) {
  auto kek = std::visit(util::Overloaded{[this](const Method& m) { return CheckMethod(m); },
                                         [this](const Attribute& a) { return CheckAttribute(a); }},
                        f.feature);
  return kek;
}

bool Semant::CheckMethod(const Method& m) {
  ScopeGuard new_scope(&_ctx);
  for (const auto& f : m.formals) {
    if (f.type_id == "SELF_TYPE") {
      std::cerr << "Formal parameter " << f.object_id << " cannot have type SELF_TYPE." << std::endl;
      return false;
    }
    CHECK_ERROR(_ctx.AddObject(f.object_id, f.type_id))
  }
  auto type = CheckExpression(m.expr);
  CHECK_ERROR(type)
  if (m.type_id == "SELF_TYPE" && *type != "SELF_TYPE") {
    std::cerr << "Error here" << std::endl;
    return {};
  }
  auto static_type = (m.type_id == "SELF_TYPE" ? _ctx.current_class : m.type_id);
  auto real_type = (*type == "SELF_TYPE" ? _ctx.current_class : *type);
  if (!_ig.IsAncessor(static_type, real_type)) {
    std::cerr << "Inferred return type " << real_type << " of method " << m.object_id
              << " does not conform to declared return type " << m.type_id << "." << std::endl;
    return false;
  }
  return true;
}

bool Semant::CheckAttribute(const Attribute& a) {
  if (a.expr->Is<Empty>()) {
    return true;
  }
  auto type = CheckExpression(a.expr);
  CHECK_ERROR(type)
  if (*type == "SELF_TYPE") {
    *type = _ctx.current_class;
  }
  auto x = (bool)type;
  auto y = _ig.GetLca(*type, a.type_id) == a.type_id;
  if (!x || !y) {
    std::cout << "KEEEK" << std::endl;
  }
  return type && _ig.GetLca(*type, a.type_id) == a.type_id;
}

template <Arithmetic T>
MaybeType Semant::CheckArithmetic(const T& expr) {
  auto left = CheckExpression(expr.lhs);
  CHECK_NULLOPT(left && *left == "Int")
  auto right = CheckExpression(expr.rhs);
  CHECK_NULLOPT(right && *right == "Int")
  return "Int";
}

MaybeType Semant::CheckInversion(const Inversion& expr) {
  auto inv_type = CheckExpression(expr.arg);
  CHECK_NULLOPT(inv_type && *inv_type == "Int")
  return "Int";
}

MaybeType Semant::CheckIsVoid(const IsVoid& a) {
  CHECK_NULLOPT(CheckExpression(a.arg))
  return "Bool";
}

MaybeType Semant::CheckNot(const Not& a) {
  auto l_type = CheckExpression(a.arg);
  CHECK_NULLOPT(l_type && *l_type == "Bool")
  return "Bool";
}

template <Comparison T>
MaybeType Semant::CheckComparison(const T& a) {
  auto l_type = CheckExpression(a.lhs);
  CHECK_NULLOPT(l_type && *l_type == "Int")
  auto r_type = CheckExpression(a.rhs);
  CHECK_NULLOPT(r_type && *r_type == "Int")
  return "Bool";
}

MaybeType Semant::CheckBlock(const Block& a) {
  for (auto it = a.expr.cbegin(); it != std::prev(a.expr.cend()); ++it) {
    CHECK_NULLOPT(CheckExpression(*it))
  }
  return CheckExpression(*std::prev(a.expr.cend()));
}

MaybeType Semant::CheckIf(const If& a) {
  auto cond_type = CheckExpression(a.condition);
  CHECK_NULLOPT(cond_type && *cond_type == "Bool")
  auto then_type = CheckExpression(a.then_expr);
  CHECK_NULLOPT(then_type)
  auto else_type = CheckExpression(a.else_expr);
  CHECK_NULLOPT(else_type)
  if (*then_type == "SELF_TYPE") {
    *then_type = _ctx.current_class;
  }
  if (*else_type == "SELF_TYPE") {
    *else_type = _ctx.current_class;
  }
  return _ig.GetLca(std::move(*then_type), std::move(*else_type));
}

MaybeType Semant::CheckWhile(const While& a) {
  auto cond_type = CheckExpression(a.condition);
  CHECK_NULLOPT(cond_type && *cond_type == "Bool")
  CHECK_NULLOPT(CheckExpression(a.loop_body))
  return "Object";
}

MaybeType Semant::CheckId(const Id& a) {
  if (a.name == "self") {
    return "SELF_TYPE";
  }
  return _ctx.GetAttrObject(a.name);
}

MaybeType Semant::CheckEqual(const Equal& a) {
  auto lhs = CheckExpression(a.lhs);
  CHECK_NULLOPT(lhs)
  auto rhs = CheckExpression(a.rhs);
  CHECK_NULLOPT(rhs)
  if (*lhs == "Int" || *lhs == "String" || *lhs == "Bool" || *rhs == "Int" || *rhs == "String" || *rhs == "Bool") {
    CHECK_NULLOPT(*lhs == *rhs)
  }
  return "Bool";
}

MaybeType Semant::CheckLet(const Let& a) {
  static int counter = 0;
  if (counter == 0) {
    _ctx.Push();
  }

  for (const auto& el : a.attrs) {
    if (el.object_id == "self") {
      std::cerr << "'self' cannot be bound in a 'let' expression." << std::endl;
      return {};
    }
    auto lhs = el.type_id == "SELF_TYPE" ? _ctx.current_class : el.type_id;

    if (!el.expr->Is<Empty>()) {
      auto rhs = CheckExpression(el.expr);
      CHECK_NULLOPT(rhs)
      if (!_ig.IsAncessor(lhs, *rhs)) {
        std::cerr << "Inferred type " << *rhs << " of initialization of " << el.object_id
                  << " does not conform to identifiers declared type " << lhs << std::endl;
        return {};
      }
    }
    _ctx.AddObject(el.object_id, el.type_id);
  }
  counter++;
  auto res = CheckExpression(a.expr);
  counter--;
  if (counter == 0) {
    _ctx.Pop();
  }
  return res;
}

MaybeType Semant::CheckCase(const Case& a) {
  CHECK_NULLOPT(CheckExpression(a.expr))
  CHECK_NULLOPT(CheckCaseNoIdenticalBranches(a))
  std::vector<std::string> types;
  for (const auto& el : a.cases) {
    ScopeGuard new_scope(&_ctx);
    _ctx.AddObject(el.object_id, el.type_id);
    auto type = CheckExpression(el.expr);
    CHECK_NULLOPT(type)
    types.emplace_back(std::move(*type));
  }
  auto result = types[0];
  for (size_t i = 1; i < types.size(); i++) {
    result = _ig.GetLca(result, types[i]);
  }
  return result;
}

MaybeType Semant::CheckDispatch(const Dispatch& a) {
  auto dispatch_expr = CheckExpression(a.expr);
  CHECK_NULLOPT(dispatch_expr)

  std::vector<std::string> arg_types;
  for (const auto& arg : a.parameters) {
    auto arg_type = CheckExpression(arg);
    CHECK_NULLOPT(arg_type)
    if (arg_type == "SELF_TYPE") {
      arg_type = _ctx.current_class;
    }
    // CHECK_NULLOPT(*arg_type != "SELF_TYPE")
    arg_types.push_back(std::move(*arg_type));
  }

  std::string dispatch_type;
  if (a.type_id) {
    if (!_ig.IsAncessor(*a.type_id, (*dispatch_expr == "SELF_TYPE" ? _ctx.current_class : *dispatch_expr))) {
      std::cerr << "Expression type SELF_TYPE does not conform to declared static dispatch type C." << std::endl;
      // std::cout << "Lol" << std::endl;
      return {};
    }
    dispatch_type = *a.type_id;
  } else if (*dispatch_expr == "SELF_TYPE") {
    dispatch_type = _ctx.current_class;
  } else {
    dispatch_type = *dispatch_expr;
  }

  auto d = _ctx.GetMethod(dispatch_type, a.object_id->name);
  if (!d) {
    std::cerr << "Error message 3" << std::endl;
    return {};
  }

  if (arg_types.size() != d->args_types.size()) {
    // TODO: make error message
    std::cerr << "Error message 2" << std::endl;
    return {};
  }
  for (size_t i = 0; i < arg_types.size(); i++) {
    auto loc = (arg_types[i] == "SELF_TYPE" ? _ctx.current_class : arg_types[i]);
    if (!_ig.IsAncessor(d->args_types[i], loc)) {
      // TODO: make error message
      std::cerr << "Error message 1" << std::endl;
      return {};
    }
  }

  auto result = d->return_type == "SELF_TYPE" ? dispatch_type : d->return_type;
  if (a.expr->Is<Id>() && a.expr->As<Id>()->name == "self" && d->return_type == "SELF_TYPE") {
    return "SELF_TYPE";
  }
  return result;
}

MaybeType Semant::CheckAssignment(const Assign& a) {
  if (a.identifier == "self") {
    std::cerr << "Error Kek" << std::endl;
    return {};
  }
  auto lhs = _ctx.GetAttrObject(a.identifier);
  CHECK_NULLOPT(lhs)
  auto rhs = CheckExpression(a.rhs);
  CHECK_NULLOPT(rhs)
  CHECK_NULLOPT(_ig.IsAncessor(*lhs, *rhs));
  return *rhs;
}

MaybeType Semant::CheckNew(const New& a) {
  if (a.type == "SELF_TYPE") {
    return "SELF_TYPE";  // _ctx.current_class;
  }
  return a.type;
}

MaybeType Semant::CheckExpression(std::shared_ptr<Expression> expr) {
  // clang-format off
  auto type = std::visit(
      util::Overloaded{
          [](const Int&) -> MaybeType { return "Int"; },
          [](const String&) -> MaybeType { return "String"; },
          [](const Bool&) -> MaybeType { return "Bool"; },
          [this](const Arithmetic auto& a) -> MaybeType { return CheckArithmetic(a); },
          [this](const Inversion& a) -> MaybeType { return CheckInversion(a); },
          [this](const IsVoid& a) -> MaybeType { return CheckIsVoid(a); },
          [this](const Not& a) -> MaybeType { return CheckNot(a); },
          [this](const Comparison auto& a) -> MaybeType { return CheckComparison(a); },
          [this](const Block& a) -> MaybeType { return CheckBlock(a); },
          [this](const If& a) -> MaybeType { return CheckIf(a); },
          [this](const While& a) -> MaybeType { return CheckWhile(a); },
          [this](const Equal& a) -> MaybeType { return CheckEqual(a); },
          [this](const Id& a) -> MaybeType { return CheckId(a); },
          [this](const New& a) -> MaybeType { return CheckNew(a); },
          [this](const Assign& a) -> MaybeType { return CheckAssignment(a); },
          [this](const Dispatch& a) -> MaybeType { return CheckDispatch(a); },
          [this](const Case& a) -> MaybeType { return CheckCase(a); },
          [this](const Let& a) -> MaybeType { return CheckLet(a);; },
          [this](const Empty&) -> MaybeType { return "_no_type"; }},
      expr->data_);
  // clang-format on
  if (type && !_ig.HasClass(*type) && *type != "SELF_TYPE") {
    type.reset();
  }
  if (type) {
    expr->type = *type;
  }
  return type;
}

}  // namespace coolc
