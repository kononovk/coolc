#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

namespace coolc {

struct Token {
  enum Type : unsigned {
    // Default token type, represents Error, if optional<string> lexeme != nullopt
    Unknown = 0,
    // Identifiers
    TypeID,
    ObjectID,
    // Elemental Types
    Integer,
    String,
    // Keywords
    Class,
    If,
    Else,
    Then,
    Fi,
    In,
    Inherits,
    Isvoid,
    Let,
    Loop,
    Pool,
    True,
    False,
    While,
    Case,
    Esac,
    New,
    Of,
    Not,
    // Special Notation
    LBrace,     // {
    RBrace,     // }
    LParen,     // (
    RParen,     // )
    Semicolon,  // ;
    Colon,      // :
    Plus,       // +
    Minus,      // -
    Star,       // *
    Slash,      // /
    Tilde,      // ~
    Less,       // <
    Leq,        // <=
    Assign,     // <-
    Equals,     // =
    Darrow,     // =>
    Dot,        // .
    Comma,      // ,
    At,         // @
    // number of elements in enum
    Size,
  };

  // Only some tokens.
  static Type FromString(const std::string& str) {
    static std::unordered_map<std::string, Type> from_str{
        {"class", Type::Class},   {"in", Type::In},     {"loop", Type::Loop},
        {"pool", Type::Pool},     {"if", Type::If},     {"true", Type::True},
        {"false", Type::False},   {"else", Type::Else}, {"inherits", Type::Inherits},
        {"while", Type::While},   {"case", Type::Case}, {"fi", Type::Fi},
        {"isvoid", Type::Isvoid}, {"esac", Type::Esac}, {"new", Type::New},
        {"of", Type::Of},         {"not", Type::Not},   {"then", Type::Then},
        {"let", Type::Let},       {"{", Type::LBrace},  {"}", Type::RBrace},
        {"(", Type::LParen},      {")", Type::RParen},  {";", Type::Semicolon},
        {":", Type::Colon},       {"+", Type::Plus},    {"-", Type::Minus},
        {"*", Type::Star},        {"/", Type::Slash},   {"~", Type::Tilde},
        {"<", Type::Less},        {"=", Type::Equals},  {".", Type::Dot},
        {",", Type::Comma},       {"@", Type::At},
    };
    auto it = from_str.find(str);
    return it == from_str.end() ? Type::Unknown : it->second;
  }

  static const char* ToString(Type token_type) noexcept {
    constexpr static std::array to_str{"ERROR",
                                       // Identifiers
                                       "TYPEID", "OBJECTID",
                                       // Elemental Types
                                       "INT_CONST", "STR_CONST",
                                       // Keywords
                                       "CLASS", "IF", "ELSE", "THEN", "FI", "IN", "INHERITS", "ISVOID", "LET", "LOOP",
                                       "POOL", "BOOL_CONST true", "BOOL_CONST false", "WHILE", "CASE", "ESAC", "NEW",
                                       "OF", "NOT",
                                       // Special Notation
                                       "'{'", "'}'", "'('", "')'", "';'", "':'", "'+'", "'-'", "'*'", "'/'", "'~'",
                                       "'<'", "LE", "ASSIGN", "'='", "DARROW", "'.'", "','", "'@'"};

    return to_str[static_cast<unsigned>(token_type)];
  }

  Type type{Type::Unknown};
  std::optional<std::string> lexeme{};
  std::uint32_t line{0};
};

}  // namespace coolc
