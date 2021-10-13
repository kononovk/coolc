#pragma once

#include <parser/expressions.hpp>
#include <token/token.hpp>

#include <cassert>
#include <memory>
#include <variant>
#include <vector>

#include <absl/status/statusor.h>

namespace coolc {

/*
 * E -> T + E / T - E
 * T -> A * T | A / T
 * A -> int/string/bool
 */
class Parser {
 public:
  explicit Parser(const std::vector<coolc::Token>& tokens, std::string filename);

  absl::StatusOr<expr::Program> ParseProgram();

 private:
  absl::StatusOr<expr::Class> ParseClass();
  absl::StatusOr<expr::Feature> ParseFeature();
  absl::StatusOr<expr::Attribute> ParseAttributeFeature();
  absl::StatusOr<expr::Method> ParseMethodFeature();
  absl::StatusOr<expr::Formal> ParseFormal();
  absl::StatusOr<expr::Expression> ParseExpression();

  /// Sorted by priority from lowest to highest
  absl::StatusOr<expr::Expression> ParseAssign();
  absl::StatusOr<expr::Expression> ParseNot();
  absl::StatusOr<expr::Expression> ParseComparisons();
  absl::StatusOr<expr::Expression> ParseAddSub();
  absl::StatusOr<expr::Expression> ParseMulDiv();
  absl::StatusOr<expr::Expression> ParseIsVoidOrInversion();
  absl::StatusOr<expr::Expression> ParseDispatch();
  absl::StatusOr<expr::Expression> ParseAtom();

  absl::StatusOr<expr::Expression> ParseIf();
  absl::StatusOr<expr::Expression> ParseWhile();
  absl::StatusOr<expr::Expression> ParseCase();
  absl::StatusOr<expr::Expression> ParseBlock();
  absl::StatusOr<expr::Expression> ParseCall();
  absl::StatusOr<expr::Expression> ParseNew();

  void AssertParser(bool condition);

 private:
  std::string filename_;
  std::vector<coolc::Token>::const_iterator next_;
};

}  // namespace coolc
