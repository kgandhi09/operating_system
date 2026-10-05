#pragma once
#include <ast.h>
#include <lexer.h>
#include <stdexcept>

namespace jk_ridl {
class Error : public std::runtime_error {
public:
  Error(const SourceLocation &at, const std::string &message);
};
class Parser {
public:
  Parser(std::string source, std::string file);
  SchemaNode Parse();

private:
  Lexer lexer_;
  Token token_;
  unsigned depth_ = 0;
  void Next();
  Token Take(TokenType type, const char *expected);
  std::string Name();
  std::uint64_t Bound();
  std::unique_ptr<TypeNode> Type();
  std::unique_ptr<DeclarationNode> Declaration();
};
} // namespace jk_ridl
