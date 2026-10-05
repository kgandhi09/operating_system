#include "source_location.h"
#include <lexer.h>

namespace jk_ridl {

Lexer::Lexer(std::string source, std::string file)
    : source_(std::move(source)), file_(std::move(file)) {}

SourceLocation Lexer::CurrentLocation() const {
  return {
      .file = file_,
      .line = line_,
      .column = column_,
  };
}

bool Lexer::IsAtEnd() const { return this->cursor_ >= source_.size(); }

char Lexer::Peek() const {

  if (IsAtEnd()) {
    return '\0';
  }

  return source_[cursor_];
}

char Lexer::PeekNext() const {

  if (cursor_ + 1 >= source_.size()) {
    return '\0';
  }

  return source_[cursor_ + 1];
}

char Lexer::Advance() {

  if (IsAtEnd()) {
    return '\0';
  }

  char current = source_[cursor_];

  cursor_++;

  if (current == '\n') {
    line_++;
    column_ = 1;
  } else {
    column_++;
  }

  return current;
}

bool Lexer::IsWhitespace(char c) const {
  switch (c) {
  case ' ':
  case '\t':
  case '\r':
  case '\n':
    return true;
  default:
    return false;
  }
}

bool Lexer::IsLetter(char c) const {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Lexer::IsDigit(char c) const { return (c >= '0' && c <= '9'); }

bool Lexer::IsIdentifierStart(char c) const { return IsLetter(c); }

bool Lexer::IsIdentifierBody(char c) const {
  return IsLetter(c) || IsDigit(c) || c == '_';
}

TokenType Lexer::GetIdentifierType(const std::string &lexeme) const {

  // Keywords
  if (lexeme == "namespace") {
    return TokenType::KwNamespace;
  }
  if (lexeme == "message") {
    return TokenType::KwMessage;
  }
  if (lexeme == "array") {
    return TokenType::KwArray;
  }
  if (lexeme == "vector") {
    return TokenType::KwVector;
  }
  if (lexeme == "string") {
    return TokenType::KwString;
  }

  // Primitive Types
  if (lexeme == "bool") {
    return TokenType::KwBool;
  }
  if (lexeme == "u8") {
    return TokenType::KwU8;
  }
  if (lexeme == "u16") {
    return TokenType::KwU16;
  }
  if (lexeme == "u32") {
    return TokenType::KwU32;
  }
  if (lexeme == "u64") {
    return TokenType::KwU64;
  }
  if (lexeme == "i8") {
    return TokenType::KwI8;
  }
  if (lexeme == "i16") {
    return TokenType::KwI16;
  }
  if (lexeme == "i32") {
    return TokenType::KwI32;
  }
  if (lexeme == "i64") {
    return TokenType::KwI64;
  }
  if (lexeme == "f32") {
    return TokenType::KwF32;
  }
  if (lexeme == "f64") {
    return TokenType::KwF64;
  }

  return TokenType::Identifier;
}

Token Lexer::MakeToken(TokenType type, std::string lexeme, SourceRange range) {
  return Token{.type = type,
               .lexeme = std::move(lexeme),
               .range = {.start = range.start, .end = range.end}};
}

Token Lexer::NextToken() {

  const SourceLocation start = CurrentLocation();

  if (IsAtEnd()) {
    return MakeToken(TokenType::EndOfFile, "", {.start = start, .end = start});
  }

  const char current = Advance();

  // Whitespace
  if (IsWhitespace(current)) {
    std::string lexeme;
    lexeme += current;
    while (IsWhitespace(Peek())) {
      lexeme += Advance();
    }

    const SourceLocation end = CurrentLocation();

    return MakeToken(TokenType::Whitespace, std::move(lexeme),
                     {.start = start, .end = end});
  }

  // Identifier / Keyword
  if (IsIdentifierStart(current)) {
    std::string lexeme;
    lexeme += current;
    while (IsIdentifierBody(Peek())) {
      lexeme += Advance();
    }

    const TokenType type = GetIdentifierType(lexeme);

    const SourceLocation end = CurrentLocation();

    return MakeToken(type, std::move(lexeme), {.start = start, .end = end});
  }

  // Integer Literal
  if (IsDigit(current)) {
    std::string lexeme;
    lexeme += current;
    while (IsDigit(Peek())) {
      lexeme += Advance();
    }

    const SourceLocation end = CurrentLocation();

    return MakeToken(TokenType::IntegerLiteral, std::move(lexeme),
                     {.start = start, .end = end});
  }

  // String Literal
  if (current == '"') {
    std::string lexeme;
    while (!IsAtEnd() && Peek() != '"' && Peek() != '\n') {
      lexeme += Advance();
    }

    if (IsAtEnd() || Peek() != '"') {
      const SourceLocation end = CurrentLocation();
      return MakeToken(TokenType::Unknown, std::move(lexeme),
                       {.start = start, .end = end});
    }

    Advance(); // consume closing quote

    const SourceLocation end = CurrentLocation();

    return MakeToken(TokenType::StringLiteral, std::move(lexeme),
                     {.start = start, .end = end});
  }

  // Punctuation
  const SourceLocation end = CurrentLocation();

  SourceRange range{.start = start, .end = end};

  switch (current) {
  case '{':
    return MakeToken(TokenType::LBrace, "{", range);
  case '}':
    return MakeToken(TokenType::RBrace, "}", range);
  case '<':
    return MakeToken(TokenType::LAngle, "<", range);
  case '>':
    return MakeToken(TokenType::RAngle, ">", range);
  case ';':
    return MakeToken(TokenType::Semicolon, ";", range);
  case ',':
    return MakeToken(TokenType::Comma, ",", range);
  case '.':
    return MakeToken(TokenType::Dot, ".", range);
  default:
    return MakeToken(TokenType::Unknown, std::string(1, current), range);
  }
}

} // namespace jk_ridl
