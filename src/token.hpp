#pragma once

#include <cassert>
#include <cstdint>
#include <optional>
#include <string>

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
    LBrace,     // '{'
    RBrace,     // '}'
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
    At,
    // number of elements in enum
    Size,
  };

  static constexpr const char* ToString(Type token_type) noexcept {
    switch (token_type) {
      // Default token type, represents Error, if optional<string> lexeme != nullopt
      case Type::Unknown:
        return "ERROR";
      // Identifiers
      case Type::TypeID:
        return "TYPEID";
      case Type::ObjectID:
        return "OBJECTID";
      // Elemental Types
      case Type::Integer:
        return "INT_CONST";
      case Type::String:
        return "STR_CONST";
      // Keywords
      case Type::Class:
        return "CLASS";
      case Type::If:
        return "IF";
      case Type::Else:
        return "ELSE";
      case Type::Then:
        return "THEN";
      case Type::Fi:
        return "FI";
      case Type::In:
        return "IN";
      case Type::Inherits:
        return "INHERITS";
      case Type::Isvoid:
        return "ISVOID";
      case Type::Let:
        return "LET";
      case Type::Loop:
        return "LOOP";
      case Type::Pool:
        return "POOL";
      case Type::True:
        return "BOOL_CONST true";
      case Type::False:
        return "BOOL_CONST false";
      case Type::While:
        return "WHILE";
      case Type::Case:
        return "CASE";
      case Type::Esac:
        return "ESAC";
      case Type::New:
        return "NEW";
      case Type::Of:
        return "OF";
      case Type::Not:
        return "NOT";
      // Special Notation
      case Type::LBrace:
        return "'{'";
      case Type::RBrace:
        return "'}'";
      case Type::LParen:
        return "'('";
      case Type::RParen:
        return "')'";
      case Type::Semicolon:
        return "';'";
      case Type::Colon:
        return "':'";
      case Type::Plus:
        return "'+'";
      case Type::Minus:
        return "'-'";
      case Type::Star:
        return "'*'";
      case Type::Slash:
        return "'/'";
      case Type::Tilde:
        return "'~'";
      case Type::Less:
        return "'<'";
      case Type::Leq:
        return "LE";
      case Type::Equals:
        return "'='";
      case Type::Assign:
        return "ASSIGN";
      case Type::Darrow:
        return "DARROW";
      case Type::Dot:
        return "'.'";
      case Type::Comma:
        return "','";
      case Type::At:
        return "'@'";
      case Type::Size:
        assert(false);
    }
    assert(false);
  }

  Type type{Type::Unknown};
  std::optional<std::string> lexeme{};
  std::uint32_t line{0};
};

}  // namespace coolc
