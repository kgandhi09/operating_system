#include <charconv>
#include <parser.h>

namespace jk_ridl {
Error::Error(const SourceLocation &at, const std::string &message)
    : std::runtime_error(at.file + ":" + std::to_string(at.line) + ":" +
                         std::to_string(at.column) + ": " + message) {}
Parser::Parser(std::string source, std::string file)
    : lexer_(std::move(source), std::move(file)) {
  Next();
}
void Parser::Next() {
  do {
    token_ = lexer_.NextToken();
  } while (token_.type == TokenType::Whitespace ||
           token_.type == TokenType::SingleLineComment ||
           token_.type == TokenType::MultiLineComment);
  if (token_.type == TokenType::Unknown)
    throw Error(token_.range.start,
                "invalid token or unterminated literal/comment");
}
Token Parser::Take(TokenType type, const char *expected) {
  if (token_.type != type)
    throw Error(token_.range.start, std::string("expected ") + expected);
  Token result = token_;
  Next();
  return result;
}
std::string Parser::Name() {
  std::string result = Take(TokenType::Identifier, "identifier").lexeme;
  while (token_.type == TokenType::Dot) {
    Next();
    result += "." + Take(TokenType::Identifier, "identifier after '.'").lexeme;
  }
  return result;
}
std::uint64_t Parser::Bound() {
  auto t = Take(TokenType::IntegerLiteral, "positive decimal bound");
  std::uint64_t n = 0;
  auto r =
      std::from_chars(t.lexeme.data(), t.lexeme.data() + t.lexeme.size(), n);
  if (r.ec != std::errc() || n == 0 || n > 65536)
    throw Error(t.range.start, "bound must be between 1 and 65536");
  return n;
}
std::unique_ptr<TypeNode> Parser::Type() {
  if (++depth_ > 64)
    throw Error(token_.range.start, "nesting exceeds 64 levels");
  auto start = token_.range.start;
  auto kind = token_.type;
  std::unique_ptr<TypeNode> result;
  if (kind >= TokenType::KwBool && kind <= TokenType::KwF64) {
    result = std::make_unique<PrimitiveTypeNode>(static_cast<PrimitiveTypeKind>(
        static_cast<int>(kind) - static_cast<int>(TokenType::KwBool)));
    Next();
  } else if (kind == TokenType::Identifier) {
    result = std::make_unique<NamedTypeNode>(Name());
  } else if (kind == TokenType::KwString) {
    Next();
    Take(TokenType::LAngle, "'<'");
    result = std::make_unique<StringTypeNode>(Bound());
    Take(TokenType::RAngle, "'>'");
  } else if (kind == TokenType::KwArray || kind == TokenType::KwVector) {
    Next();
    Take(TokenType::LAngle, "'<'");
    auto element = Type();
    Take(TokenType::Comma, "','");
    auto n = Bound();
    Take(TokenType::RAngle, "'>'");
    if (kind == TokenType::KwArray)
      result = std::make_unique<ArrayTypeNode>(std::move(element), n);
    else
      result = std::make_unique<VectorTypeNode>(std::move(element), n);
  } else
    throw Error(start, "expected field type");
  result->range = {start, token_.range.start};
  --depth_;
  return result;
}
std::unique_ptr<DeclarationNode> Parser::Declaration() {
  if (++depth_ > 64)
    throw Error(token_.range.start, "nesting exceeds 64 levels");
  auto start = token_.range.start;
  std::unique_ptr<DeclarationNode> result;
  if (token_.type == TokenType::KwInclude) {
    Next();
    auto path = Take(TokenType::StringLiteral, "include path").lexeme;
    if (path.empty())
      throw Error(start, "include path cannot be empty");
    result = std::make_unique<IncludeNode>(std::move(path));
  } else if (token_.type == TokenType::KwNamespace) {
    Next();
    auto node = std::make_unique<NamespaceNode>(Name());
    Take(TokenType::LBrace, "'{'");
    while (token_.type != TokenType::RBrace)
      node->declarations.push_back(Declaration());
    Take(TokenType::RBrace, "'}'");
    result = std::move(node);
  } else if (token_.type == TokenType::KwMessage) {
    Next();
    auto node = std::make_unique<MessageNode>(
        Take(TokenType::Identifier, "message name").lexeme);
    Take(TokenType::LBrace, "'{'");
    while (token_.type != TokenType::RBrace) {
      auto field_start = token_.range.start;
      auto type = Type();
      auto name = Take(TokenType::Identifier, "field name").lexeme;
      auto field =
          std::make_unique<FieldNode>(std::move(name), std::move(type));
      auto end = Take(TokenType::Semicolon, "';'").range.end;
      field->range = {field_start, end};
      node->fields.push_back(std::move(field));
    }
    Take(TokenType::RBrace, "'}'");
    result = std::move(node);
  } else
    throw Error(start, "expected include, namespace, or message declaration");
  result->range = {start, token_.range.start};
  --depth_;
  return result;
}
SchemaNode Parser::Parse() {
  SchemaNode schema;
  schema.range.start = token_.range.start;
  while (token_.type != TokenType::EndOfFile)
    schema.declarations.push_back(Declaration());
  schema.range.end = token_.range.end;
  return schema;
}
} // namespace jk_ridl
