#include "lexer/lexer.hpp"

#include "token/token.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace coolc {

namespace {

// just private helper to skip characters after null character in string
void SkipAfterNull(std::stringstream* ss) {
  while (!ss->eof() && ss->peek() != '\n' && ss->peek() != '"') {
    ss->get();
  }
  if (ss->peek() == '"') {
    ss->get();
  }
}

// pre-condition: lexeme.size() > 0
bool IsTypeId(std::string_view lexeme) {
  return std::isupper(lexeme[0]);
}

}  // namespace

Lexer::Lexer(std::string source_code) : _current_line{1}, _sstream{std::move(source_code)} {
}

Token Lexer::NextToken() {
  SkipWs();
  if (!_sstream.good()) {
    return {};
  }
  if (auto token = GetSpecial(); token) {
    return *token;
  }
  if (auto token = GetIntLiteral(); token) {
    return *token;
  }
  if (auto token = GetStringLiteral(); token) {
    return *token;  // may be error
  }
  std::string lexeme;
  _sstream >> lexeme;
  assert(!lexeme.empty());
  if (auto token = GetKeyword(lexeme); token) {
    return *token;
  }

  // ObjectID / TypeID
  if (!std::isalpha(lexeme[0])) {
    return Token{.lexeme = "Unknown error", .line = _current_line};
  }
  std::regex rgx;
  Token::Type token_type{};
  if (IsTypeId(lexeme)) {
    rgx = R"(^[A-Z]\w*)";
    token_type = Token::Type::TypeID;
  } else {
    rgx = R"(^[a-z]\w*)";
    token_type = Token::Type::ObjectID;
  }
  std::smatch match;
  std::regex_search(lexeme, match, rgx);
  lexeme = match[0];
  _sstream.seekg(-match.suffix().length(), std::ios_base::cur);
  return MakeToken(token_type, lexeme);
}

std::vector<Token> Lexer::Tokenize() {
  auto is_eof = [](const Token& token) { return !token.lexeme && token.type == Token::Type::Unknown; };
  std::vector<Token> result;
  for (auto token = NextToken(); !is_eof(token); token = NextToken()) {
    result.push_back(std::move(token));
  }
  result.push_back(MakeToken(Token::Type::Unknown));
  return result;
}

void Lexer::SkipWs() {
  while (_sstream.good() && std::isspace(_sstream.peek())) {
    if (_sstream.get() == '\n') {
      _current_line++;
    }
  }
}

/// pre-condition: string_stream is inside multiline comment
/// skip comment and get next token or return error
Token Lexer::SkipComment() {
  std::regex reg(R"((\(\*|\*\)))");
  std::cmatch match;
  int32_t comment_counter = 1;
  // TODO: why str is incorrect here ?
  // TODO: stupid compiler/STL in CI, change this code latter
  auto tmp_sstream = _sstream.str();
  auto tmp_sstream_offset = _sstream.tellg();
  while (std::regex_search(tmp_sstream.c_str() + tmp_sstream_offset, match, reg)) {
    auto prefix = match.prefix().str();
    long offset = prefix.size() + match[0].length();
    _current_line += std::count(prefix.begin(), prefix.end(), '\n');
    _sstream.seekg(_sstream.tellg() + offset);
    tmp_sstream_offset = _sstream.tellg();
    comment_counter += match[0].str() == "*)" ? -1 : 1;
    if (comment_counter == 0) {
      return NextToken();
    }
  }
  for (auto el = tmp_sstream.c_str() + _sstream.tellg(); *el != '\0'; ++el) {
    if (*el == '\n') {
      _current_line++;
    }
  }
  _sstream.setstate(std::stringstream::eofbit);
  return MakeToken(Token::Type::Unknown, "EOF in comment");
}

std::optional<Token> Lexer::GetSpecial() {
  char curr = _sstream.get();

  if (auto token = CheckInvalid(curr); token) {
    return *token;
  }

  if (curr == '(') {
    char next = _sstream.get();
    switch (next) {
      case '*':
        return SkipComment();
      default:
        _sstream.putback(next);
        return MakeToken(Token::Type::LParen);
    }
  }
  if (curr == '-') {
    char next = _sstream.get();
    switch (next) {
      case '-':
        _sstream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        _current_line++;
        return NextToken();
      default:
        _sstream.putback(next);
        return MakeToken(Token::Type::Minus);
    }
  }
  if (curr == '*') {
    char next = _sstream.get();
    switch (next) {
      case ')':
        return MakeToken(Token::Type::Unknown, "Unmatched *)");
      default:
        _sstream.putback(next);
        return MakeToken(Token::Type::Mul);
    }
  }
  if (curr == '<') {
    char next = _sstream.get();
    switch (next) {
      case '=':
        return MakeToken(Token::Type::Leq);
      case '-':
        return MakeToken(Token::Type::Assign);
      default:
        _sstream.putback(next);
        return MakeToken(Token::Type::Less);
    }
  }
  if (curr == '=') {
    char next = _sstream.get();
    switch (next) {
      case '>':
        return MakeToken(Token::Type::Darrow);
      default:
        _sstream.putback(next);
        return MakeToken(Token::Type::Equals);
    }
  }

  if (auto type = Token::FromString(std::string{curr}); type != Token::Type::Unknown) {
    return MakeToken(type);
  }
  _sstream.putback(curr);
  return {};
}

std::optional<Token> Lexer::GetIntLiteral() {
  if (!std::isdigit(_sstream.peek())) {
    return {};
  }
  std::string res;
  char curr = _sstream.get();
  while (std::isdigit(curr)) {
    res.push_back(curr);
    curr = _sstream.get();
  }
  if (_sstream.good()) {
    _sstream.putback(curr);
  }
  return MakeToken(Token::Type::Integer, std::move(res));
}

std::optional<Token> Lexer::GetStringLiteral() {
  static const std::unordered_map<char, const char*> escaped_sequences = {
      {'\t', "\\t"},  {'\b', "\\b"}, {'\r', "\\015"},   {'\f', "\\f"},     {'\033', "\\033"},
      {'\\', "\\\\"}, {'\n', "\\n"}, {'\x0b', "\\013"}, {'\x12', "\\022"}, {'\x1B', "\\e"}};
  static const std::unordered_map<char, const char*> char_to_escape = {
      {'t', "\\t"}, {'n', "\\n"}, {'b', "\\b"}, {'f', "\\f"}, {'\033', "\\033"}, {'\\', "\\\\"}, {'"', "\\\""}};

  if (_sstream.peek() != '\"') {
    return {};
  }
  std::string buffer{};
  char c = _sstream.get();
  size_t buf_len = 0;
  while (!_sstream.eof()) {
    char n = _sstream.peek();
    if (n == '\0') {
      SkipAfterNull(&_sstream);
      return MakeToken(Token::Type::Unknown, "String contains null character.");
    }
    if (n == '\n') {
      _sstream.get(c);
      _current_line++;
      return MakeToken(Token::Type::Unknown, "Unterminated string constant");
    }
    if (n == '"') {
      _sstream.get(c);
      if (buf_len > 1024) {
        return MakeToken(Token::Type::Unknown, "String constant too long");
      }
      return MakeToken(Token::Type::String, buffer);
    }
    if (n == '\\') {
      _sstream.get(c);
      char next = _sstream.peek();
      if (next == '\0') {
        SkipAfterNull(&_sstream);
        return MakeToken(Token::Type::Unknown, "String contains escaped null character.");
      } else if (next == std::char_traits<char>::eof()) {
        return MakeToken(Token::Type::Unknown, "EOF in string constant");
      } else if (char_to_escape.contains(next)) {
        buffer.append(char_to_escape.at(next));
      } else if (escaped_sequences.contains(next)) {
        buffer.append(escaped_sequences.at(next));
        if (next == '\n') {
          _current_line++;
        }
      } else {
        buffer.push_back(next);
      }
      _sstream.get(c);
    } else if (escaped_sequences.contains(n)) {
      _sstream.get(c);
      buffer.append(escaped_sequences.at(c));
    } else {
      buffer += _sstream.peek();
      _sstream.get(c);
    }
    buf_len++;
  }
  return MakeToken(Token::Type::Unknown, "EOF in string constant");
}

std::optional<Token> Lexer::GetKeyword(const std::string& keyword) {
  std::regex reg(
      R"(^(class|if|else|fi|inherits|in|isvoid|loop|pool|true|false|while|case|esac|new|of|not|then|let)(\W+.*)*)",
      std::regex_constants::icase);

  std::smatch match;
  if (!std::regex_match(keyword, match, reg) || match.prefix().length() > 0) {
    return {};
  }

  // TODO: rewrite to std::ranges::views::transform
  std::string little_str = match[1].str();
  std::for_each(little_str.begin(), little_str.end(), [](char& el) { el = std::tolower(el); });

  long pos = _sstream.tellg();
  // TODO: rewrite this code, this is very ugly
  if (pos == -1) {
    std::stringstream new_sstr{std::string{keyword.begin() + match[1].length(), keyword.end()}};
    _sstream.swap(new_sstr);
  } else {
    _sstream.seekg(pos - (keyword.size() - match[1].length()));
  }

  auto token_type = Token::FromString(little_str);
  if (token_type == Token::Type::Unknown || (token_type == Token::Type::True && !std::islower(keyword[0])) ||
      (token_type == Token::Type::False && !std::islower(keyword[0]))) {
    _sstream.seekg(pos);
    return {};
  }
  return MakeToken(token_type);
}

std::optional<Token> Lexer::CheckInvalid(char ch) {
  if (int i = static_cast<int>(ch); i >= 0 && i <= 4) {
    return MakeToken(Token::Type::Unknown, "\\00" + std::to_string(i));
  }

  constexpr const char* invalid_characters = "!#$%^&_>?`[]\\|";
  if (std::strchr(invalid_characters, ch)) {
    std::string to_print{ch};
    if (ch == '\\') {
      to_print.push_back('\\');
    }
    return MakeToken(Token::Type::Unknown, to_print);
  }
  return {};
}

Token Lexer::MakeToken(Token::Type type) {
  return {.type = type, .line = _current_line};
}

Token Lexer::MakeToken(Token::Type type, std::string lexeme) {
  return {.type = type, .lexeme = std::move(lexeme), .line = _current_line};
}

}  // namespace coolc
