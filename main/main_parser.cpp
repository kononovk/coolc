#include "ast/expression.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "util/util.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_set>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "error: no input files" << std::endl;
    return 1;
  }

  for (int i = 1; i < argc; ++i) {
    coolc::Lexer lexer(ReadAllFile(argv[i]));
    auto tokens = lexer.Tokenize();
    auto program = coolc::Parser(tokens, argv[i]).ParseProgram();
    PrintProgram(program);
  }
  return 0;
}
