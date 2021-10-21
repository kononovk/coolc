#include "parser/parser.hpp"

#include "ast/expression.hpp"
#include "token/token.hpp"

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace coolc {

Parser::Parser(const std::vector<Token>& tokens, std::string filename)
    : filename_(std::move(filename)), next_{tokens.cbegin()} {
}

Program Parser::ParseProgram() {
  Program res;
  res.line_number = next_->line;
  while (next_->type == Token::Type::Class) {
    res.classes.emplace_back(ParseClass());
    AssertMatch(Token::Type::Semicolon);
  }
  Assert(res.classes.size() > 0 && next_->type == Token::Type::Unknown);
  return res;
}

Class Parser::ParseClass() {
  Assert(next_->type == Token::Type::Class);
  Class res{.filename = filename_};
  res.line_number = next_->line;
  next_++;
  Assert(next_->type == Token::Type::TypeID);
  res.type = *next_->lexeme;
  next_++;
  if (Match(Token::Type::Inherits)) {
    Assert(next_->type == Token::Type::TypeID);
    res.inherits_type = *next_->lexeme;
    next_++;
  } else {
    res.inherits_type = "Object";
  }
  AssertMatch(Token::Type::LBrace);

  for (auto feature_ = ParseFeature(); feature_; feature_ = ParseFeature()) {
    res.features.push_back(*feature_);
    AssertMatch(Token::Type::Semicolon);
  }

  AssertMatch(Token::Type::RBrace);
  return res;
}

std::optional<Feature> Parser::ParseFeature() {
  Feature res;
  if (next_->type != Token::Type::ObjectID) {
    return {};
  }
  if (std::next(next_)->type == Token::Type::LParen) {
    res.feature = ParseMethodFeature();
    return {std::move(res)};
  } else if (std::next(next_)->type == Token::Type::Colon) {
    res.feature = ParseAttributeFeature();
    return {std::move(res)};
  }
  Assert(false);
  return {};  // unreachable code, just to avoid warnings
}

Attribute Parser::ParseAttributeFeature() {
  Assert(next_->type == Token::Type::ObjectID);
  Attribute res;
  res.line_number = next_->line;
  res.object_id = *next_->lexeme;
  next_++;
  AssertMatch(Token::Type::Colon);
  Assert(next_->type == Token::Type::TypeID);
  res.type_id = *next_->lexeme;
  next_++;
  if (Match(Token::Type::Assign)) {
    res.expr = std::make_shared<Expression>(ParseExpression());
  } else {
    res.expr = std::make_shared<Expression>(Empty{});
  }
  return res;
}

Method Parser::ParseMethodFeature() {
  // attribute feature
  Method res;
  Assert(next_->type == Token::Type::ObjectID);
  res.line_number = next_->line;
  res.object_id = *next_->lexeme;
  next_++;
  AssertMatch(Token::Type::LParen);

  while (true) {
    auto formal = ParseFormal();
    if (formal) {
      res.formals.push_back(*formal);
      if (next_->type == Token::Type::RParen) {
        next_++;
        break;
      }
      Assert(next_->type == Token::Type::Comma);
      next_++;
    } else {
      AssertMatch(Token::Type::RParen);
      break;
    }
  }
  AssertMatch(Token::Type::Colon);
  Assert(next_->type == Token::Type::TypeID);
  res.type_id = *next_->lexeme;
  next_++;
  AssertMatch(Token::Type::LBrace);
  res.expr = std::make_shared<Expression>(ParseExpression());
  AssertMatch(Token::Type::RBrace);
  return res;
}

std::optional<Formal> Parser::ParseFormal() {
  if (next_->type != Token::Type::ObjectID) {
    return {};
  }
  Formal res;
  res.object_id = *next_->lexeme;
  res.line_number = next_->line;
  next_++;
  AssertMatch(Token::Type::Colon);
  Assert(next_->type == Token::Type::TypeID);
  res.type_id = *next_->lexeme;
  next_++;
  return {std::move(res)};
}

/// Sorted by priority from lowest to highest
/// Id <- expr
Expression Parser::ParseAssign() {
  if (next_->type != Token::Type::ObjectID) {
    return ParseNot();
  }
  Assign res;
  res.line_number = next_->line;
  res.identifier = *next_->lexeme;
  if (std::next(next_)->type != Token::Type::Assign) {
    return ParseNot();
  }
  next_ += 2;
  auto expr = ParseAssign();
  res.rhs = std::make_shared<Expression>(expr);
  return {std::move(res)};
}

/// Not expr
Expression Parser::ParseNot() {
  if (next_->type != Token::Type::Not) {
    return ParseComparisons();
  }
  auto line = next_->line;
  next_++;
  return {Not{line, std::make_shared<Expression>(ParseNot())}};  // TODO: check and new tests
}

/// expr (<|<=|=) expr
Expression Parser::ParseComparisons() {
  auto term = ParseAddSub();
  auto line = next_->line;

  if (Match(Token::Type::Less)) {
    return {Less{line, std::make_shared<Expression>(std::move(term)), std::make_shared<Expression>(ParseAddSub())}};
  }
  if (Match(Token::Type::Equals)) {
    return {Equal{line, std::make_shared<Expression>(std::move(term)), std::make_shared<Expression>(ParseAddSub())}};
  }
  if (Match(Token::Type::Leq)) {
    return {LessEq{line, std::make_shared<Expression>(std::move(term)), std::make_shared<Expression>(ParseAddSub())}};
  }
  return term;
}

/// expr (+|-) expr
Expression Parser::ParseAddSub() {
  auto term = ParseMulDiv();
  while (next_->type == Token::Type::Plus || next_->type == Token::Type::Minus) {
    auto line = next_->line;
    auto type = next_->type;
    ++next_;
    auto next_term = ParseMulDiv();
    auto lhs = std::make_shared<Expression>(std::move(term));
    auto rhs = std::make_shared<Expression>(std::move(next_term));

    if (type == Token::Type::Plus) {
      term = {Plus{line, std::move(lhs), std::move(rhs)}};
    } else {
      term = {Sub{line, std::move(lhs), std::move(rhs)}};
    }
  }
  return term;
}

/// expr (*|/) expr
Expression Parser::ParseMulDiv() {
  auto term = ParseIsVoidOrInversion();
  while (next_->type == Token::Type::Mul || next_->type == Token::Type::Slash) {
    auto line = next_->line;
    auto type = next_->type;
    ++next_;
    auto next_term = ParseIsVoidOrInversion();
    auto lhs = std::make_shared<Expression>(std::move(term));
    auto rhs = std::make_shared<Expression>(std::move(next_term));

    if (type == Token::Type::Mul) {
      term = {Mul{line, std::move(lhs), std::move(rhs)}};
    } else {
      term = {Div{line, std::move(lhs), std::move(rhs)}};
    }
  }
  return term;
}

/// (~|isvoid) expr
Expression Parser::ParseIsVoidOrInversion() {
  auto line = next_->line;
  if (Match(Token::Type::Isvoid)) {
    return {IsVoid{line, std::make_shared<Expression>(ParseIsVoidOrInversion())}};
  } else if (Match(Token::Type::Tilde)) {
    return {Inversion{line, std::make_shared<Expression>(ParseIsVoidOrInversion())}};
  }
  return ParseDispatch();
}

/// expr[@Type].Id([expr]*)
Expression Parser::ParseDispatch() {
  Dispatch res;

  auto parse_simple = [&] {
    res.object_id = std::make_shared<Id>(Id{next_->line, *next_->lexeme});
    next_++;
    res.parameters = GetParameterList();
  };
  res.line_number = next_->line;

  if (next_->type == Token::Type::ObjectID && std::next(next_)->type == Token::Type::LParen) {
    res.expr = std::make_shared<Expression>(Id{next_->line, "self"});
    parse_simple();
  } else {
    auto a = ParseAtom();
    if (Match(Token::Type::At)) {
      Assert(next_->type == Token::Type::TypeID);
      res.type_id = *next_->lexeme;
      next_++;
      Assert(next_->type == Token::Type::Dot);
    }
    if (Match(Token::Type::Dot)) {
      res.expr = std::make_shared<Expression>(std::move(a));
      parse_simple();
    } else {
      return a;
    }
  }
  while (next_->type == Token::Type::Dot) {
    res.expr = std::make_shared<Expression>(std::move(res));
    res.line_number = next_->line;
    next_++;
    parse_simple();
  }
  return {std::move(res)};
}

Expression Parser::ParseAtom() {
  auto line = next_->line;
  if (next_->type == Token::Type::Integer) {
    int32_t value = std::stoi(*(next_++)->lexeme);
    return {Int{line, value}};
  }
  if (next_->type == Token::Type::String) {
    return {String{line, *(next_++)->lexeme}};
  }
  if (next_->type == Token::Type::True || next_->type == Token::Type::False) {
    return {Bool{line, (next_++)->type == Token::Type::True}};
  }
  if (next_->type == Token::Type::If) {
    return ParseIf();
  }
  if (next_->type == Token::Type::While) {
    return ParseWhile();
  }
  if (next_->type == Token::Type::LBrace) {
    return ParseBlock();
  }
  if (next_->type == Token::Type::New) {
    return ParseNew();
  }
  if (next_->type == Token::Type::Case) {
    return ParseCase();
  }
  if (Match(Token::Type::LParen)) {
    auto exp = ParseExpression();
    Assert(next_->type == Token::Type::RParen);
    next_++;
    return exp;
  }
  if (next_->type == Token::Type::ObjectID) {
    Id res{line, *next_->lexeme};
    ++next_;
    return {std::move(res)};
  }
  if (next_->type == Token::Type::Let) {
    Let res;
    res.line_number = line;
    next_++;
    while (next_->type != Token::Type::In) {
      res.attrs.push_back(ParseAttributeFeature());
      if (next_->type == Token::Type::Comma) {
        next_++;
      } else if (next_->type != Token::Type::In) {
        Assert(false);
      }
    }
    Assert(res.attrs.size() > 0 && next_->type == Token::Type::In);
    next_++;
    res.expr = std::make_shared<Expression>(ParseExpression());
    return {std::move(res)};
  }
  Assert(false);
  // only for avoid warnings that function is noreturn
  return {Empty{}};
}

Expression Parser::ParseExpression() {
  return ParseAssign();
}

Expression Parser::ParseIf() {
  Assert(next_->type == Token::Type::If);
  If res;
  res.line_number = (next_++)->line;
  res.condition = std::make_shared<Expression>(ParseExpression());
  AssertMatch(Token::Type::Then);
  res.then_expr = std::make_shared<Expression>(ParseExpression());
  AssertMatch(Token::Type::Else);
  res.else_expr = std::make_shared<Expression>(ParseExpression());
  AssertMatch(Token::Type::Fi);
  return {std::move(res)};
}

Expression Parser::ParseWhile() {
  Assert(next_->type == Token::Type::While);
  While res;
  res.line_number = (next_++)->line;
  res.condition = std::make_shared<Expression>(ParseExpression());
  AssertMatch(Token::Type::Loop);
  res.loop_body = std::make_shared<Expression>(ParseExpression());
  AssertMatch(Token::Type::Pool);
  return {std::move(res)};
}

Expression Parser::ParseBlock() {
  Assert(next_->type == Token::Type::LBrace);
  Block res;
  res.line_number = (next_++)->line;
  while (next_->type != Token::Type::RBrace) {
    auto expr = ParseExpression();
    res.expr.push_back(std::make_shared<Expression>(std::move(expr)));
    AssertMatch(Token::Type::Semicolon);
  }
  Assert(res.expr.size() > 0);
  AssertMatch(Token::Type::RBrace);
  return {std::move(res)};
}

Expression Parser::ParseCase() {
  Assert(next_->type == Token::Type::Case);
  Case res;
  res.line_number = next_->line;
  next_++;
  res.expr = std::make_shared<Expression>(ParseExpression());
  AssertMatch(Token::Type::Of);

  while (next_->type != Token::Type::Esac) {
    Assert(next_->type == Token::Type::ObjectID);
    Attribute attr;
    attr.line_number = next_->line;
    attr.object_id = *next_->lexeme;
    next_++;
    AssertMatch(Token::Type::Colon);
    Assert(next_->type == Token::Type::TypeID);
    attr.type_id = *next_->lexeme;
    next_++;
    AssertMatch(Token::Type::Darrow);
    attr.expr = std::make_shared<Expression>(ParseExpression());
    AssertMatch(Token::Type::Semicolon);
    res.cases.push_back(std::move(attr));
  }

  Assert(res.cases.size() > 0);
  AssertMatch(Token::Type::Esac);
  return {std::move(res)};
}

Expression Parser::ParseNew() {
  Assert(next_->type == Token::Type::New);
  auto line = (next_++)->line;
  Assert(next_->type == Token::Type::TypeID);
  New res{line, *(next_++)->lexeme};
  return {std::move(res)};
}

std::vector<std::shared_ptr<Expression>> Parser::GetParameterList() {
  std::vector<std::shared_ptr<Expression>> parameters;
  if (Match(Token::Type::LParen)) {
    while (!Match(Token::Type::RParen)) {
      parameters.emplace_back(std::make_shared<Expression>(ParseExpression()));
      if (Match(Token::Type::Comma)) {
        Assert(next_->type != Token::Type::RParen);
      }
    }
  }
  return parameters;
}

bool Parser::Match(Token::Type type) {
  if (next_->type == type) {
    next_++;
    return true;
  }
  return false;
}

void Parser::Assert(bool condition) {
  if (!condition) {
    std::cerr << filename_ << ", line " << next_->line << std::endl
              << "Compilation halted due to lex and parse errors" << std::endl;
    std::exit(1);
  }
}

void Parser::AssertMatch(Token::Type type) {
  if (next_->type != type) {
    Assert(false);
  }
  next_++;
}

}  // namespace coolc
