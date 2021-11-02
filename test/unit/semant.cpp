#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semant/inheritance_graph.hpp"

#include <vector>

#include <gtest/gtest.h>

TEST(Exists, Simple) {
  coolc::Program p;
  p.classes = std::vector<coolc::Class>{{.type = "A", .inherits_type = "B"}, {.type = "B", .inherits_type = "A"}};
  coolc::InheritanceGraph g{p};
  EXPECT_TRUE(g.CheckCycle());
}

TEST(NotExists, Simple) {
  coolc::Program p;
  p.classes = std::vector<coolc::Class>{{.type = "A", .inherits_type = "B"}, {.type = "C", .inherits_type = "D"}};
  coolc::InheritanceGraph g{p};
  EXPECT_FALSE(g.CheckCycle());
}
