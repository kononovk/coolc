#include "ast/expression.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semant/inheritance_graph.hpp"
#include "semant/semant.hpp"
#include "util/type_traits.hpp"
#include "util/util.hpp"

#include <iostream>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "error: no input files" << std::endl;
    return 1;
  }

  for (int i = 1; i < argc; ++i) {
    coolc::Lexer lexer(ReadAllFile(argv[i]));
    auto tokens = lexer.Tokenize();
    auto program = coolc::Parser(tokens, argv[i]).ParseProgram();

    coolc::Semant semantic_checker(std::move(program));

    if (!semantic_checker.CheckProgram()) {
      std::cerr << "Compilation halted due to static semantic errors." << std::endl;
      return 1;
    }
    coolc::PrintProgram(semantic_checker.GetProgram(), 0);
  }
  return 0;
}
