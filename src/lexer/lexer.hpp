#pragma once

#include "token/token.hpp"

#include <cstdint>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace coolc {

class Lexer {
 public:
  explicit Lexer(std::string source_code);

  Token NextToken();

  std::vector<Token> Tokenize();

 private:
  void SkipWs();
  Token SkipComment();

  std::optional<Token> GetSpecial();
  std::optional<Token> GetKeyword(const std::string& keyword);
  std::optional<Token> GetIntLiteral();
  std::optional<Token> GetStringLiteral();
  std::optional<Token> CheckInvalid(char ch);

  Token MakeToken(Token::Type type);
  Token MakeToken(Token::Type type, std::string lexeme);

  uint32_t current_line = 1;
  std::stringstream _sstream;
};

}  // namespace coolc
