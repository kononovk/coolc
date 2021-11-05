#pragma once

#include <cstdint>
#include <iostream>
#include <optional>
#include <string>

namespace coolc {

struct Error {
  std::optional<std::size_t> line;
  std::optional<std::string> filename;
  std::string error_message;
  Error(std::string error_msg) : error_message(std::move(error_msg)) {
  }
  Error(std::size_t l, std::string f, std::string e) : line(l), filename(std::move(f)), error_message(std::move(e)) {
  }

  friend std::ostream& operator<<(std::ostream& os, const Error& err) {
    if (err.filename) {
      os << *err.filename << ":";
    }
    if (err.line) {
      os << *err.line << ": ";
    }
    os << err.error_message << std::endl;
    return os;
  }
};

inline Error MakeError(const Class& cl, std::string message) {
  return {cl.line_number, cl.filename, std::move(message)};
}

using Ok = std::monostate;
using ErrorOrNull = std::optional<Error>;

}  // namespace coolc
