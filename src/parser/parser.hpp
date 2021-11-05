#pragma once

#include "ast/expression.hpp"
#include "token/token.hpp"

#include <cassert>
#include <memory>
#include <variant>
#include <vector>

namespace coolc {

class Parser {
 public:
  explicit Parser(const std::vector<coolc::Token>& tokens, std::string filename);

  Program ParseProgram();

 private:
  Class ParseClass();
  std::optional<Feature> ParseFeature();
  Attribute ParseAttributeFeature();
  Method ParseMethodFeature();
  std::optional<Formal> ParseFormal();
  Expression ParseExpression();

  /// Sorted by priority from lowest to highest
  Expression ParseAssign();
  Expression ParseNot();
  Expression ParseComparisons();
  Expression ParseAddSub();
  Expression ParseMulDiv();
  Expression ParseIsVoidOrInversion();
  Expression ParseDispatch();
  Expression ParseAtom();

  Expression ParseIf();
  Expression ParseWhile();
  Expression ParseCase();
  Expression ParseBlock();
  Expression ParseNew();

  std::vector<std::shared_ptr<Expression>> GetParameterList();

  /// FillAndCheck condition. If false terminate program with error in stderr
  void Assert(bool condition);

  /// FillAndCheck current token type.
  /// If true move next_, otherwise terminate program with error in stderr
  void AssertMatch(Token::Type);

  /// FillAndCheck current token type.
  /// If true move next_, otherwise return false
  bool Match(Token::Type);

  std::string filename_;
  std::vector<coolc::Token>::const_iterator next_;
};

}  // namespace coolc
