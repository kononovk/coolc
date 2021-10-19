#include <ast/expression.hpp>
#include <coolc/config.hpp>
#include <lexer/lexer.hpp>
#include <parser/parser.hpp>
#include <token/token.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace {

std::string ReadAllFile(std::string_view filename) {
  std::ifstream is(std::filesystem::path{filename});
  return std::string(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>());
}

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
  if (argc != 2) {
    std::cerr << "error: no input files" << std::endl;
    return 1;
  }

  for (int i = 1; i < argc; ++i) {
    coolc::Lexer lexer(ReadAllFile(argv[i]));
    auto tokens = lexer.Tokenize();
#ifdef COOLC_LEXER
    std::cout << "#name \"" << argv[i] << '\"' << std::endl;
    for (const auto& el : tokens) {
      std::cout << el;
    }
#endif  // COOLC_LEXER

#ifdef COOLC_PARSER
    auto program = coolc::Parser(tokens, argv[i]).ParseProgram();
    PrintProgram(program);
#endif  // COOLC_PARSER
  }

  return 0;
}
