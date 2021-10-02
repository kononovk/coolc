#include <lexer/lexer.hpp>
#include <token/token.hpp>

#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace coolc {

Lexer::Lexer(std::string source_code) : _sstream(std::move(source_code)) {
}

Token Lexer::NextToken() {
  SkipWs();
  if (_sstream.eof() || !_sstream.good()) {
    return {};
  }
  if (auto token = GetSpecial(); token) {
    return *token;
  }
  char curr = _sstream.peek();
  if (std::isdigit(curr)) {
    auto token = GetIntLiteral();
    return *token;
  }
  if (curr == '\"') {
    auto token = GetStringLiteral();
    return *token;  // may be error
  }
  std::string lexeme;
  _sstream >> lexeme;
  assert(!lexeme.empty());
  if (auto token = IsKeyword(lexeme); token) {
    return *token;
  } else {
    if (std::isupper(lexeme[0])) {
      std::regex rgx(R"([A-Z]\w*)");
      std::smatch match;
      if (std::regex_search(lexeme, match, rgx)) {
        lexeme = match[0];
        long pos = _sstream.tellg();
        long offset = match.suffix().str().size();
        _sstream.seekg(pos - offset);
        return {.type = Token::Type::TypeID, .lexeme = lexeme, .line = current_line};
      }
      assert(false);
    } else if (std::islower(lexeme[0])) {
      std::regex rgx(R"([a-z]\w*)");
      std::smatch match;
      if (std::regex_search(lexeme, match, rgx)) {
        lexeme = match[0];
        long pos = _sstream.tellg();
        long offset = match.suffix().str().size();
        _sstream.seekg(pos - offset);
        return {.type = Token::Type::ObjectID, .lexeme = lexeme, .line = current_line};
      }
      assert(false);
    }
  }
  // Keywords, TypeId, ObjectId
  return Token{.lexeme = "Unknown error", .line = current_line};
}

std::vector<Token> Lexer::GetAllTokens() {
  auto is_eof = [](const Token& token) {
    return !token.lexeme && token.type == Token::Type::Unknown;
  };
  std::vector<Token> result;
  for (auto token = NextToken(); !is_eof(token); token = NextToken()) {
    result.push_back(std::move(token));
  }
  return result;
}

void Lexer::SkipWs() {
  const char* spaces = "\t\n\f\r\v\b ";
  char c{};
  while (!_sstream.eof() && _sstream.good()) {
    _sstream.get(c);
    if (!std::strchr(spaces, c) || c == '\0') {
      _sstream.putback(c);
      break;
    }
    if (c == '\n') {
      current_line++;
    }
  }
}

std::optional<Token> Lexer::GetSpecial() {
  char curr = _sstream.get();
  const char* invalid_characters = "!#$%^&_>?`[]\\|";
  if (curr == '\000') {
    return MakeToken(Token::Type::Unknown, "\\000");
  } else if (curr == '\001') {
    return MakeToken(Token::Type::Unknown, "\\001");
  } else if (curr == '\002') {
    return MakeToken(Token::Type::Unknown, "\\002");
  } else if (curr == '\003') {
    return MakeToken(Token::Type::Unknown, "\\003");
  } else if (curr == '\004') {
    return MakeToken(Token::Type::Unknown, "\\004");
  }
  char to_print[] = {curr, '\0', '\0'};
  if (curr == '\\') {
    to_print[1] = '\\';
  }
  if (std::strchr(invalid_characters, curr) != nullptr) {
    return MakeToken(Token::Type::Unknown, to_print);
  }
  if (curr == '@') {
    return MakeSpecial(Token::Type::At);
  }
  if (curr == '{') {
    return MakeSpecial(Token::Type::LBrace);
  }
  if (curr == '}') {
    return MakeSpecial(Token::Type::RBrace);
  }
  if (curr == '(') {
    char next = _sstream.get();
    if (next != '*') {
      _sstream.putback(next);
      return MakeSpecial(Token::Type::LParen);
    }
    std::regex reg(R"((\(\*|\*\)))");
    std::cmatch match;
    int32_t comment_counter = 1;
    // todo: why str is incorrect here ?
    while (std::regex_search(_sstream.rdbuf()->view().data() + _sstream.tellg(), match, reg)) {
      long pos = _sstream.tellg();
      auto prefix = match.prefix().str();
      long offset = prefix.size() + match[0].str().size();
      current_line += std::count(prefix.begin(), prefix.end(), '\n');
      _sstream.seekg(pos + offset);

      auto match_str = match[0].str();
      if (match_str == "*)") {
        comment_counter--;
      } else {
        comment_counter++;
      }
      if (comment_counter == 0) {
        return NextToken();
      }
    }
    auto stream_buf = _sstream.rdbuf()->view();
    for (auto el = stream_buf.data() + _sstream.tellg(); *el != '\0'; ++el) {
      if (*el == '\n') {
        current_line++;
      }
    }
    _sstream.setstate(std::stringstream::eofbit);
    return MakeToken(Token::Type::Unknown, "EOF in comment");
  }
  if (curr == ')') {
    return MakeSpecial(Token::Type::RParen);
  }
  if (curr == ';') {
    return MakeSpecial(Token::Type::Semicolon);
  }
  if (curr == ':') {
    return MakeSpecial(Token::Type::Colon);
  }

  if (curr == '+') {
    return MakeSpecial(Token::Type::Plus);
  }
  if (curr == '-') {
    char next = _sstream.get();
    if (next == '-') {
      _sstream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      current_line++;
      return NextToken();
    } else {
      _sstream.putback(next);
      return MakeSpecial(Token::Type::Minus);
    }
  }
  if (curr == '*') {
    char next = _sstream.get();
    if (next == ')') {
      return MakeToken(Token::Type::Unknown, "Unmatched *)");
    }
    _sstream.putback(next);
    return MakeSpecial(Token::Type::Star);
  }

  if (curr == '/') {
    return MakeSpecial(Token::Type::Slash);
  }

  if (curr == '~') {
    return MakeSpecial(Token::Type::Tilde);
  }

  if (curr == '<') {
    char next = _sstream.get();
    if (next == '=') {
      return MakeSpecial(Token::Type::Leq);
    } else if (next == '-') {
      return MakeSpecial(Token::Type::Assign);
    } else {
      _sstream.putback(next);
      return MakeSpecial(Token::Type::Less);
    }
  }
  if (curr == '=') {
    char next = _sstream.get();
    if (next != '>') {
      _sstream.putback(next);
      return MakeSpecial(Token::Type::Equals);
    } else {
      return MakeSpecial(Token::Type::Darrow);
    }
  }
  if (curr == '.') {
    return MakeSpecial(Token::Type::Dot);
  }
  if (curr == ',') {
    return MakeSpecial(Token::Type::Comma);
  }
  _sstream.putback(curr);
  return {};
}

std::optional<Token> Lexer::GetIntLiteral() {
  std::string res;
  char curr = _sstream.get();
  while (std::isdigit(curr)) {
    res.push_back(curr);
    curr = _sstream.get();
  }
  if (!_sstream.eof() && _sstream.good()) {
    _sstream.putback(curr);
  }
  return MakeToken(Token::Type::Integer, std::move(res));
}

std::optional<Token> Lexer::GetStringLiteral() {
  std::string buffer = "";
  char c;
  _sstream.get(c);
  size_t counter = 0;
  while (true) {
    if (_sstream.peek() == '\0') {
      while (_sstream.peek() != -1 && _sstream.peek() != '\n' && _sstream.peek() != '"')
        _sstream.get(c);
      if (_sstream.peek() == '"')
        _sstream.get(c);
      return MakeToken(Token::Type::Unknown, "String contains null character.");
    }
    if (_sstream.eof()) {
      return MakeToken(Token::Type::Unknown, "EOF in string constant");
    }
    if (_sstream.peek() == '\n') {
      _sstream.get(c);
      current_line++;
      return MakeToken(Token::Type::Unknown, "Unterminated string constant");
    }
    if (_sstream.peek() == '\\') {
      _sstream.get(c);
      switch (_sstream.peek()) {
        case '\0':
          while (_sstream.peek() != -1 && _sstream.peek() != '\n' && _sstream.peek() != '"')
            _sstream.get(c);
          if (_sstream.peek() == '"')
            _sstream.get(c);
          return MakeToken(Token::Type::Unknown, "String contains escaped null character.");
        case -1:
          return MakeToken(Token::Type::Unknown, "EOF in string constant");
        case '\\':
          buffer.append("\\\\");
          counter++;
          break;
        case '"':
          buffer.append("\\\"");
          counter++;
          break;
        case 'b':
          buffer.append("\\b");
          counter++;
          break;
        case 't':
          buffer.append("\\t");
          counter++;
          break;
        case 'n':
          buffer.append("\\n");
          counter++;
          break;
        case 'f':
          buffer.append("\\f");
          counter++;
          break;
        case '\n':
          buffer.append("\\n");
          current_line++;
          counter++;
          break;
        case '\t':
          buffer.append("\\t");
          counter++;
          break;
        case '\b':
          buffer.append("\\b");
          counter++;
          break;
        case '\f':
          buffer.append("\\f");
          counter++;
          break;
        case '\r':
          buffer.append("\\015");
          counter++;
          break;
        case '\033':
          buffer.append("\\033");
          counter++;
          break;
        default:
          buffer += _sstream.peek();
          counter++;
      }
      _sstream.get(c);
      continue;
    }

    if (_sstream.peek() == '"') {
      _sstream.get(c);
      if (counter > 1024) {
        return MakeToken(Token::Type::Unknown, "String constant too long");
      }
      return MakeToken(Token::Type::String, buffer);
    }

    if (_sstream.peek() == '\t') {
      _sstream.get(c);
      buffer.append("\\t");
      counter++;
      continue;
    }

    if (_sstream.peek() == '\b') {
      _sstream.get(c);
      buffer.append("\\b");
      counter++;
      continue;
    }

    if (_sstream.peek() == '\f') {
      _sstream.get(c);
      buffer.append("\\f");
      counter++;
      continue;
    }

    if (_sstream.peek() == '\r') {
      _sstream.get(c);
      buffer.append("\\015");
      counter++;
      continue;
    }

    if (_sstream.peek() == '\033') {
      _sstream.get(c);
      buffer.append("\\033");
      counter++;
      continue;
    }

    buffer += _sstream.peek();
    counter++;
    _sstream.get(c);
  }
}

std::optional<Token> Lexer::IsKeyword(const std::string& keyword) {
  std::regex reg(
      R"((class|if|else|fi|in|inherits|isvoid|loop|pool|true|false|while|case|esac|new|of|not|then|let)(\W)*)",
      std::regex_constants::icase);

  std::smatch match;
  if (!std::regex_match(keyword, match, reg) || match.prefix().str().size() != 0) {
    return {};
  }

  std::string little_str = match[1].str();
  for (auto& el : little_str) {
    if (std::isupper(el)) {
      el = std::tolower(el);
    }
  }
  long pos = _sstream.tellg();
  long offset = match[2].str().size();
  _sstream.seekg(pos - offset);

  if (little_str == "class") {
    return MakeSpecial(Token::Type::Class);
  }
  if (little_str == "if") {
    return MakeSpecial(Token::Type::If);
  }
  if (little_str == "else") {
    return MakeSpecial(Token::Type::Else);
  }
  if (little_str == "fi") {
    return MakeSpecial(Token::Type::Fi);
  }
  if (little_str == "in") {
    return MakeSpecial(Token::Type::In);
  }
  if (little_str == "inherits") {
    return MakeSpecial(Token::Type::Inherits);
  }
  if (little_str == "isvoid") {
    return MakeSpecial(Token::Type::Isvoid);
  }
  if (little_str == "let") {
    return MakeSpecial(Token::Type::Let);
  }
  if (little_str == "loop") {
    return MakeSpecial(Token::Type::Loop);
  }
  if (little_str == "pool") {
    return MakeSpecial(Token::Type::Pool);
  }
  if (little_str == "true" && islower(keyword[0])) {
    return MakeSpecial(Token::Type::True);
  }
  if (little_str == "false" && islower(keyword[0])) {
    return MakeSpecial(Token::Type::False);
  }
  if (little_str == "while") {
    return MakeSpecial(Token::Type::While);
  }
  if (little_str == "case") {
    return MakeSpecial(Token::Type::Case);
  }
  if (little_str == "esac") {
    return MakeSpecial(Token::Type::Esac);
  }
  if (little_str == "new") {
    return MakeSpecial(Token::Type::New);
  }
  if (little_str == "of") {
    return MakeSpecial(Token::Type::Of);
  }
  if (little_str == "not") {
    return MakeSpecial(Token::Type::Not);
  }
  if (little_str == "then") {
    return MakeSpecial(Token::Type::Then);
  }
  _sstream.seekg(pos);
  return {};
}

Token Lexer::MakeSpecial(Token::Type type) {
  return {.type = type, .line = current_line};
}
Token Lexer::MakeToken(Token::Type type, std::string lexeme) {
  return {.type = type, .lexeme = lexeme, .line = current_line};
}

}  // namespace coolc
