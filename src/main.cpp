#include <lexer/lexer.hpp>

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
    os << *token.lexeme;
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
    coolc::Lexer lexer(ReadAllFile(argv[i]));
    auto tokens = lexer.GetAllTokens();
    for (const auto& token : tokens) {
      std::cout << token;
    }
  }
  return 0;
}
