#include "./lexer/lexer.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>

namespace {

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
    os << ' ';
    if (token.type == coolc::Token::Type::String || token.type == coolc::Token::Unknown) {
      os << "\"";
    }
    if (token.type == coolc::Token::Type::String) {
      auto& to_print = *token.lexeme;
      for (auto el : to_print) {
        if (el == '\n') {
          std::cout << "\\n";
        } else if (el == '\t') {
          std::cout << "\\t";
        } else if (el == '\r') {
          std::cout << "\\015";
        } else if (el == '\b') {
          std::cout << "\\b";
        } else if (el == '\f') {
          std::cout << "\\f";
        } else if (el == '\x1B') {
          std::cout << "\\e";
        } else if (el == '\x12') {
          std::cout << "\\022";
        } else if (el == '\x0b') {
          std::cout << "\\013";
        } else {
          std::cout << el;
        }
      }
    } else {
      os << *token.lexeme;
    }
    if (token.type == coolc::Token::Type::String || token.type == coolc::Token::Unknown) {
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
    std::cout << "#name \"" << argv[i] << '\"' << std::endl;
    // std::cout << "DEBUG INFO: " << ReadAllFile(argv[i]) << std::endl;
    coolc::Lexer lexer(ReadAllFile(argv[i]));
    auto tokens = lexer.GetAll();
    for (const auto& token : tokens) {
      std::cout << token;
    }
  }
  return 0;
}
