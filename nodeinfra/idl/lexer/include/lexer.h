#pragma once

// lexer architecture
// - stateful
// - streaming
// - single-pass
// - deterministic
// - recursive-descent-parser-friendly

#include <cstdint>
#include <string>

#include <token.h>

namespace jk_ridl {

class Lexer {

public:
  Lexer(std::string source, std::string file);

  [[nodiscard]]
  Token NextToken();

private:
  SourceLocation CurrentLocation() const;
  bool IsAtEnd() const;
  char Peek() const;
  char PeekNext() const;
  char Advance();
  Token MakeToken(TokenType type, std::string lexeme, SourceRange range);

  // helpers
  bool IsWhitespace(char c) const;
  bool IsLetter(char c) const;
  bool IsDigit(char c) const;
  bool IsIdentifierStart(char c) const;
  bool IsIdentifierBody(char c) const;
  TokenType GetIdentifierType(const std::string &lexeme) const;

private:
  std::string source_;
  std::string file_;
  std::size_t cursor_ = 0;

  uint32_t line_ = 1;
  uint32_t column_ = 1;
};

} // namespace jk_ridl
