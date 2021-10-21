#include "ast/expression.hpp"
#include "lexer/lexer.hpp"
#include "token/token.hpp"
#include "util/util.hpp"

#include <filesystem>
#include <iostream>

namespace {

std::ostream& operator<<(std::ostream& os, const coolc::Token& token) {
  if (token.type == coolc::Token::Type::Unknown && !token.lexeme) {
    return os;
  }
  os << '#' << token.line << ' ' << coolc::Token::ToString(token.type);
  if (token.lexeme) {
    if (token.type == coolc::Token::Type::Unknown && !token.lexeme) {
      return os;
    }
    os << ' ';
    if (token.type == coolc::Token::Type::String || token.type == coolc::Token::Type::Unknown) {
      os << "\"";
    }
    os << *token.lexeme;
    if (token.type == coolc::Token::Type::String || token.type == coolc::Token::Type::Unknown) {
      os << "\"";
    }
  }
  os << std::endl;
  return os;
}

}  // namespace

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "error: no input files" << std::endl;
    return 1;
  }

  for (int i = 1; i < argc; ++i) {
    coolc::Lexer lexer(ReadAllFile(argv[i]));
    auto tokens = lexer.Tokenize();
    std::cout << "#name \"" << argv[i] << '\"' << std::endl;
    for (const auto& el : tokens) {
      std::cout << el;
    }
  }
  return 0;
}
