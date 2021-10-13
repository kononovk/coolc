#include <parser/expressions.hpp>
#include <parser/parser.hpp>
#include <token/token.hpp>

#include <cassert>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace coolc {
using namespace expr;

Parser::Parser(const std::vector<coolc::Token>& tokens, std::string filename)
    : next_{tokens.cbegin()}, filename_(std::move(filename)) {
}

absl::StatusOr<expr::Program> Parser::ParseProgram() {
  expr::Program res;
  res.line_number = next_->line;
  while (next_->type == Token::Type::Class) {
    auto class_ = ParseClass();
    if (class_.ok()) {
      res.classes.push_back(std::move(*class_));
      AssertParser(next_->type == Token::Type::Semicolon);
      next_++;
    } else {
      return {std::move(class_).status()};
    }
  }
  AssertParser(res.classes.size() > 0);
  return absl::StatusOr<expr::Program>{std::move(res)};
}

absl::StatusOr<expr::Class> Parser::ParseClass() {
  Class res;
  res.filename = filename_;
  res.line_number = next_->line;
  AssertParser(next_->type == Token::Type::Class);
  next_++;
  AssertParser(next_->type == Token::Type::TypeID);
  res.type = *next_->lexeme;
  next_++;
  if (next_->type == Token::Type::Inherits) {
    next_++;
    AssertParser(next_->type == Token::Type::TypeID);
    res.inherits_type = *next_->lexeme;
    next_++;
  } else {
    res.inherits_type = "Object";
  }
  AssertParser(next_->type == Token::Type::LBrace);
  next_++;

  while (true) {
    auto feature_ = ParseFeature();
    if (feature_.ok()) {
      res.features.push_back(*feature_);
      AssertParser(next_->type == Token::Type::Semicolon);
      next_++;
    } else {
      break;
    }
  }

  AssertParser(next_->type == Token::Type::RBrace);
  next_++;
  return res;
}

absl::StatusOr<expr::Feature> Parser::ParseFeature() {
  Feature res;
  res.line_number = next_->line;
  if (next_->type != Token::Type::ObjectID) {
    return absl::StatusOr<expr::Feature>{};
  }
  if (std::next(next_)->type == Token::Type::LParen) {
    // TODO: add error processing
    res.feature = *ParseMethodFeature();
    return res;
  } else if (std::next(next_)->type == Token::Type::Colon) {
    // TODO: add error processing
    res.feature = *ParseAttributeFeature();
    return res;
  }
  AssertParser(false);
}

absl::StatusOr<expr::Attribute> Parser::ParseAttributeFeature() {
  // method feature
  Attribute res;
  AssertParser(next_->type == Token::Type::ObjectID);
  res.object_id = *next_->lexeme;
  next_++;
  AssertParser(next_->type == Token::Type::Colon);
  next_++;
  AssertParser(next_->type == Token::Type::TypeID);
  res.type_id = *next_->lexeme;
  next_++;
  if (next_->type == Token::Type::Assign) {
    next_++;
    res.expr = std::make_shared<Expression>(*ParseExpression());
  } else {
    res.expr = std::make_shared<Expression>(Expression{});
  }
  return absl::StatusOr<expr::Attribute>{res};
}

absl::StatusOr<expr::Method> Parser::ParseMethodFeature() {
  // attribute feature
  Method res;
  AssertParser(next_->type == Token::Type::ObjectID);
  res.object_id = *next_->lexeme;
  next_++;
  AssertParser(next_->type == Token::Type::LParen);
  next_++;

  while (true) {
    auto formal = ParseFormal();
    if (formal.ok()) {
      res.formals.push_back(*formal);
      if (next_->type == Token::Type::RParen) {
        next_++;
        break;
      }
      AssertParser(next_->type == Token::Type::Comma);
      next_++;
    } else {
      AssertParser(next_->type == Token::Type::RParen);
      next_++;
      break;
    }
  }
  AssertParser(next_->type == Token::Type::Colon);
  next_++;
  AssertParser(next_->type == Token::Type::TypeID);
  res.type_id = *next_->lexeme;
  next_++;
  AssertParser(next_->type == Token::Type::LBrace);
  next_++;
  res.expr = std::make_shared<Expression>(*ParseExpression());
  AssertParser(next_->type == Token::Type::RBrace);
  next_++;
  return absl::StatusOr<expr::Method>{res};
}

absl::StatusOr<expr::Formal> Parser::ParseFormal() {
  Formal res;
  if (next_->type != Token::Type::ObjectID) {
    return absl::StatusOr<expr::Formal>{};
  }
  res.line_number = next_->line;
  res.object_id = *next_->lexeme;
  next_++;
  if (next_->type != Token::Type::Colon) {
    AssertParser(false);
  }
  next_++;
  if (next_->type != Token::Type::TypeID) {
    AssertParser(false);
  }
  res.type_id = *next_->lexeme;
  next_++;
  return absl::StatusOr<expr::Formal>{res};
}

/// Sorted by priority from lowest to highest
absl::StatusOr<expr::Expression> Parser::ParseAssign() {
  if (next_->type != Token::Type::ObjectID) {
    return ParseNot();
  }
  Assign res;
  res.line_number = next_->line;
  res.identifier = *next_->lexeme;
  if (std::next(next_)->type != Token::Type::Assign) {
    return ParseNot();
  }
  next_++;
  next_++;
  auto expr = ParseAssign();
  if (!expr.ok()) {
    return std::move(expr).status();
  }
  res.rhs = std::make_shared<Expression>(*expr);
  return Expression{res};
}

absl::StatusOr<expr::Expression> Parser::ParseNot() {
  if (next_->type != Token::Type::Not) {
    return ParseComparisons();
  }
  next_++;
  return Expression{Not{next_->line, {}, std::make_shared<Expression>(*ParseComparisons())}};
}

absl::StatusOr<expr::Expression> Parser::ParseComparisons() {
  auto term = ParseAddSub().value();
  auto line = next_->line;
  if (next_->type == Token::Type::Less) {
    next_++;
    return Expression{Less{line, {}, std::make_shared<Expression>(term), std::make_shared<Expression>(*ParseAddSub())}};
  } else if (next_->type == Token::Type::Leq) {
    next_++;
    return Expression{
        LessEq{line, {}, std::make_shared<Expression>(term), std::make_shared<Expression>(*ParseAddSub())}};
  } else if (next_->type == Token::Type::Equals) {
    next_++;
    return Expression{
        Equal{line, {}, std::make_shared<Expression>(term), std::make_shared<Expression>(*ParseAddSub())}};
  }
  return term;
  // assert(false);
}

absl::StatusOr<expr::Expression> Parser::ParseAddSub() {
  auto term = ParseMulDiv();
  while (next_->type == coolc::Token::Type::Plus || next_->type == coolc::Token::Type::Minus) {
    auto line = next_->line;
    if (next_->type == coolc::Token::Type::Plus) {
      ++next_;
      auto next_term = ParseMulDiv();
      auto expr = expr::Expression{Plus{line,
                                        {},
                                        std::make_shared<Expression>(std::move(*term)),
                                        std::make_shared<Expression>(std::move(*next_term))}};
      term = std::move(expr);
    } else if (next_->type == coolc::Token::Type::Minus) {
      ++next_;
      auto next_term = ParseMulDiv();
      auto expr = expr::Expression{Sub{line,
                                       {},
                                       std::make_shared<Expression>(std::move(*term)),
                                       std::make_shared<Expression>(std::move(*next_term))}};
      term = std::move(expr);
    }
  }
  return term;
}

absl::StatusOr<expr::Expression> Parser::ParseMulDiv() {
  auto atom = ParseIsVoidOrInversion();
  while (next_->type == Token::Type::Mul || next_->type == Token::Type::Slash) {
    auto line = next_->line;
    if (next_->type == coolc::Token::Type::Mul) {
      ++next_;
      auto next_term = ParseIsVoidOrInversion();
      auto expr = expr::Expression{
          Mul{line, {}, std::make_shared<Expression>(*atom), std::make_shared<Expression>(*next_term)}};
      atom = std::move(expr);
    } else if (next_->type == coolc::Token::Type::Slash) {
      ++next_;
      auto next_term = ParseIsVoidOrInversion();
      auto expr = expr::Expression{
          Div{line, {}, std::make_shared<Expression>(*atom), std::make_shared<Expression>(*next_term)}};
      atom = std::move(expr);
    }
  }
  return atom;
}

absl::StatusOr<expr::Expression> Parser::ParseIsVoidOrInversion() {
  auto line = next_->line;
  if (next_->type == Token::Type::Isvoid) {
    next_++;
    return expr::Expression{IsVoid{line, {}, std::make_shared<Expression>(*ParseIsVoidOrInversion())}};
  } else if (next_->type == Token::Type::Tilde) {
    next_++;
    return expr::Expression{Inversion{line, {}, std::make_shared<Expression>(*ParseIsVoidOrInversion())}};
  }
  auto atom = ParseDispatch();
  return atom;
}

absl::StatusOr<expr::Expression> Parser::ParseDispatch() {
  Dispatch t;
  t.line_number = next_->line;
  Id self;
  self.line_number = next_->line;
  self.name = "self";
  t.expr = std::make_shared<Expression>(Expression{self});
  if (next_->type == Token::Type::ObjectID && std::next(next_)->type == Token::Type::LParen) {
    Id object;
    object.line_number = next_->line;
    object.name = *next_->lexeme;
    next_++;
    t.object_id = std::make_shared<Id>(object);
    next_++;
    while (true) {
      if (next_->type == Token::Type::RParen) {
        next_++;
        break;
      }
      auto expr = *ParseExpression();
      t.parameters.push_back(std::make_shared<Expression>(expr));

      if (next_->type == Token::Type::Comma) {
        next_++;
        AssertParser(next_->type != Token::Type::RParen);
      }
    }
  } else {
    auto a = ParseAtom();
    if (next_->type == Token::Type::At) {
      next_++;
      AssertParser(next_->type == Token::Type::TypeID);
      t.type_id = *next_->lexeme;
      next_++;
      AssertParser(next_->type == Token::Type::Dot);
    }
    if (next_->type == Token::Type::Dot) {
      next_++;
      t.expr = std::make_shared<Expression>(*a);
      Id object;
      object.line_number = next_->line;
      object.name = *next_->lexeme;
      next_++;
      t.object_id = std::make_shared<Id>(object);
      if (next_->type == Token::Type::LParen) {
        next_++;
        while (true) {
          if (next_->type == Token::Type::RParen) {
            next_++;
            break;
          }
          auto expr = *ParseExpression();
          t.parameters.push_back(std::make_shared<Expression>(expr));
          if (next_->type == Token::Type::Comma) {
            next_++;
            AssertParser(next_->type != Token::Type::RParen);
          }
        }
      }
    } else
      return a;
  }
  while (next_->type == Token::Type::Dot) {
    Dispatch t_next;
    t_next.line_number = next_->line;
    next_++;
    t_next.expr = std::make_shared<Expression>(Expression{t});
    auto object = ParseAtom();
    t_next.object_id = std::make_shared<Id>(std::get<Id>(object->data_));
    AssertParser(next_->type == Token::Type::LParen);
    next_++;
    while (true) {
      if (next_->type == Token::Type::RParen) {
        next_++;
        break;
      }
      auto expr = ParseExpression();
      t_next.parameters.push_back(std::make_shared<Expression>(*expr));
      if (next_->type == Token::Type::Comma) {
        next_++;
        AssertParser(next_->type == Token::Type::RParen);
      }
    }
    t = std::move(t_next);
  }
  return Expression{t};
}

absl::StatusOr<expr::Expression> Parser::ParseAtom() {
  if (next_->type == coolc::Token::Type::Integer) {
    int32_t value = std::stoi(*next_->lexeme);
    size_t line = next_->line;
    ++next_;
    return Expression{Int{line, value}};
  } else if (next_->type == coolc::Token::Type::String) {
    return Expression{String{next_->line, {}, *(next_++)->lexeme}};
  } else if (next_->type == coolc::Token::Type::True) {
    auto line = next_->line;
    ++next_;
    return Expression{Bool{line, {}, true}};
  } else if (next_->type == coolc::Token::Type::False) {
    auto line = next_->line;
    ++next_;
    return Expression{Bool{line, {}, false}};
  } else if (next_->type == coolc::Token::Type::LParen) {
    ++next_;
    auto exp = ParseExpression();
    AssertParser(next_->type == coolc::Token::Type::RParen);
    next_++;
    return *exp;
  } else if (next_->type == coolc::Token::Type::ObjectID) {
    Id res{next_->line, {}, *next_->lexeme};
    ++next_;
    return Expression{Id{res}};
  } else if (next_->type == coolc::Token::Type::Let) {
    Let result;
    result.line_number = next_->line;
    next_++;
    while (next_->type != Token::Type::In) {
      result.attrs.push_back(*ParseAttributeFeature());
      if (next_->type == Token::Type::Comma) {
        next_++;
      } else if (next_->type != Token::Type::In) {
        AssertParser(false);
      }
    }
    AssertParser(result.attrs.size() > 0);
    AssertParser(next_->type == Token::Type::In);
    next_++;
    result.expr = std::make_shared<Expression>(*ParseExpression());
    return Expression{Let{std::move(result)}};
  }
  // TODO: is it really correct ?
  // return absl::OkStatus();
  AssertParser(false);
}

/* - + / *
 * 123 * 5 + 4
 * */

absl::StatusOr<expr::Expression> Parser::ParseExpression() {
  switch (next_->type) {
    case Token::Type::Let:
      return ParseAtom();
    case Token::Type::If:
      return ParseIf();
    case Token::Type::While:
      return ParseWhile();
    case Token::Type::LBrace:
      return ParseBlock();
    case Token::Type::Case:
      return ParseCase();
    case Token::Type::New:
      return ParseNew();
    default:
      /// just skip it
      break;
  }
  return ParseAssign();
}

absl::StatusOr<expr::Expression> Parser::ParseIf() {
  AssertParser(next_->type == Token::Type::If);
  If res;
  res.line_number = next_->line;
  next_++;
  res.condition = std::make_shared<Expression>(*ParseExpression());
  AssertParser(next_->type == Token::Type::Then);
  next_++;
  res.then_expr = std::make_shared<Expression>(*ParseExpression());
  AssertParser(next_->type == Token::Type::Else);
  next_++;
  res.else_expr = std::make_shared<Expression>(*ParseExpression());
  AssertParser(next_->type == Token::Type::Fi);
  next_++;
  return absl::StatusOr<expr::Expression>(Expression{res});
}

absl::StatusOr<expr::Expression> Parser::ParseWhile() {
  AssertParser(next_->type == Token::Type::While);
  While res;
  res.line_number = next_->line;
  next_++;
  res.condition = std::make_shared<Expression>(*ParseExpression());
  AssertParser(next_->type == Token::Type::Loop);
  next_++;
  res.loop_body = std::make_shared<Expression>(*ParseExpression());
  AssertParser(next_->type == Token::Type::Pool);
  next_++;
  return absl::StatusOr<expr::Expression>(Expression{res});
}

absl::StatusOr<expr::Expression> Parser::ParseCase() {
  AssertParser(next_->type == Token::Type::Case);
  Case res;
  res.line_number = next_->line;
  next_++;
  res.expr = std::make_shared<Expression>(*ParseExpression());
  AssertParser(next_->type == Token::Type::Of);
  next_++;

  while (next_->type != Token::Type::Esac) {
    Attribute attr;
    AssertParser(next_->type == Token::Type::ObjectID);
    attr.line_number = next_->line;
    attr.object_id = *next_->lexeme;
    next_++;
    AssertParser(next_->type == Token::Type::Colon);
    next_++;
    AssertParser(next_->type == Token::Type::TypeID);
    attr.type_id = *next_->lexeme;
    next_++;
    AssertParser(next_->type == Token::Type::Darrow);
    next_++;
    attr.expr = std::make_shared<Expression>(*ParseExpression());
    AssertParser(next_->type == Token::Type::Semicolon);
    next_++;
    res.cases.push_back(std::move(attr));
  }

  /////
  AssertParser(res.cases.size() > 0);
  AssertParser(next_->type == Token::Type::Esac);
  next_++;
  return {Expression{res}};
}

void Parser::AssertParser(bool condition) {
  if (!condition) {
    std::cerr << filename_ << ", line " << next_->line << std::endl
              << "Compilation halted due to lex and parse errors" << std::endl;
    std::terminate();
  }
}

absl::StatusOr<expr::Expression> Parser::ParseBlock() {
  AssertParser(next_->type == Token::Type::LBrace);
  Block res;
  res.line_number = next_->line;
  next_++;
  while (next_->type != Token::Type::RBrace) {
    auto expr = *ParseExpression();
    res.expr.push_back(std::make_shared<Expression>(std::move(expr)));
    AssertParser(next_->type == Token::Type::Semicolon);
    next_++;
  }
  AssertParser(res.expr.size() > 0);
  AssertParser(next_->type == Token::Type::RBrace);
  next_++;
  return absl::StatusOr<expr::Expression>{Expression{res}};
}

absl::StatusOr<expr::Expression> Parser::ParseCall() {
  AssertParser(next_->type == Token::Type::ObjectID && std::next(next_)->type == Token::Type::LParen);
  MethodCall res;
  res.line_number = next_->line;
  res.identifier = std::make_shared<Id>(Id{next_->line, {}, *next_->lexeme});
  next_++;
  next_++;
  while (next_->type != Token::Type::RParen) {
    auto expr = *ParseExpression();
    if (next_->type == Token::Type::Comma) {
      next_++;
      AssertParser(next_->type != Token::Type::RParen);
    }
    res.expr.push_back(std::make_shared<Expression>(std::move(expr)));
  }
  AssertParser(next_->type == Token::Type::RParen);
  next_++;
  return absl::StatusOr<expr::Expression>{Expression{res}};
}

absl::StatusOr<expr::Expression> Parser::ParseNew() {
  AssertParser(next_->type == Token::Type::New);
  next_++;
  AssertParser(next_->type == Token::Type::TypeID);
  New res{next_->line, {}, *next_->lexeme};
  next_++;
  return {Expression{std::move(res)}};
}

}  // namespace coolc
