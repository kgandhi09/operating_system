#pragma once

#include <cstdint>

namespace jk_ridl {

enum class TokenType : uint16_t {

  // Special
  EndOfFile,
  Unknown,

  // Keywords
  KwInclude,
  KwNamespace,
  KwMessage,
  KwArray,
  KwVector,
  KwString,

  // Primitive Types
  KwBool,
  KwU8,
  KwU16,
  KwU32,
  KwU64,
  KwI8,
  KwI16,
  KwI32,
  KwI64,
  KwF32,
  KwF64,

  // Literals
  Identifier,
  IntegerLiteral,
  StringLiteral,

  // Trivia
  Whitespace,
  SingleLineComment,
  MultiLineComment,

  // Symbols / Punctuation
  LBrace,    // {
  RBrace,    // }
  LAngle,    // <
  RAngle,    // >
  Semicolon, // ;
  Comma,     // ,
  Dot        // .

};

} // namespace jk_ridl
