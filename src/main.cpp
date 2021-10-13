#include <lexer/lexer.hpp>
#include <parser/expressions.hpp>
#include <parser/parser.hpp>
#include <token/token.hpp>

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

std::string ReadAllFile(std::string_view filename) {
  std::ifstream is(std::filesystem::path{filename});
  is.seekg(0, std::ios::end);
  size_t size = is.tellg();
  std::string buffer(size, ' ');
  is.seekg(0);
  is.read(&buffer[0], size);
  return buffer;
}

std::ostream& operator<<(std::ostream& os, const coolc::Token& token) {
  os << '#' << token.line << ' ' << coolc::Token::ToString(token.type);
  if (token.lexeme) {
    if (token.type == coolc::Token::Type::Unknown && !token.lexeme) {
      return os;
    }
    os << ' ';
    if (token.type == coolc::Token::Type::String || token.type == coolc::Token::Unknown) {
      os << "\"";
    }
    os << *token.lexeme;
    if (token.type == coolc::Token::Type::String || token.type == coolc::Token::Unknown) {
      os << "\"";
    }
  }
  os << std::endl;
  return os;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "error: no input files" << std::endl;
    return 1;
  }

  for (int i = 1; i < argc; ++i) {
    // std::cout << "File Content:" << std::endl;
    std::string s = ReadAllFile(argv[i]);
    // std::cout << s << std::endl;
    // std::cout << "Tokens:" << std::endl;
    // std::cout << "#name \"" << argv[i] << '\"' << std::endl;
    coolc::Lexer lexer(std::move(s));
    auto tokens = lexer.Tokenize();
    // for (const auto& el : tokens) {
    //   std::cout << el;
    // }
    // std::cout << std::endl;
    // std::cout << "Parse tree: " << std::endl;
    auto program = *coolc::Parser(tokens, argv[i]).ParseProgram();
    program.Print();
  }

  return 0;
}
