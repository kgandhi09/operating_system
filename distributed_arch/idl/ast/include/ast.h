#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <source_location.h>
namespace jk_ridl {

// Base AST Nodes
struct AstNode {
  SourceRange range;
  virtual ~AstNode() = default;
};

// Base Type Node
struct TypeNode : public AstNode {
  ~TypeNode() override = default;
};

// Primitive Type Kinds
enum class PrimitiveTypeKind {
  Bool,
  U8,
  U16,
  U32,
  U64,
  I8,
  I16,
  I32,
  I64,
  F32,
  F64,
};

// Primitive Type Node
struct PrimitiveTypeNode : public TypeNode {
  explicit PrimitiveTypeNode(PrimitiveTypeKind kind) : kind(kind) {}

  PrimitiveTypeKind kind;
};

// Named Type Reference
struct NamedTypeNode : public TypeNode {
  explicit NamedTypeNode(std::string name) : name(std::move(name)) {}

  std::string name;
};

// Fixed Array Type
struct ArrayTypeNode : public TypeNode {
  ArrayTypeNode(std::unique_ptr<TypeNode> element_type, std::uint64_t length)
      : element_type(std::move(element_type)), length(length) {}

  std::unique_ptr<TypeNode> element_type;
  std::uint64_t length;
};

// Bounded Vector Type
struct VectorTypeNode : public TypeNode {
  VectorTypeNode(std::unique_ptr<TypeNode> element_type, std::uint64_t capacity)
      : element_type(std::move(element_type)), capacity(capacity) {}

  std::unique_ptr<TypeNode> element_type;
  std::uint64_t capacity;
};

// Bounded String Type
struct StringTypeNode : public TypeNode {
  explicit StringTypeNode(std::uint64_t max_length) : max_length(max_length) {}

  std::uint64_t max_length;
};

// Message Field
struct FieldNode : public AstNode {
  FieldNode(std::string name, std::unique_ptr<TypeNode> type)
      : name(std::move(name)), type(std::move(type)) {}

  std::string name;
  std::unique_ptr<TypeNode> type;
};

struct DeclarationNode : public AstNode {
  virtual ~DeclarationNode() = default;
};

// Include Declaration
struct IncludeNode : public DeclarationNode {
  explicit IncludeNode(std::string path) : path(std::move(path)) {}

  std::string path;
};

// Message Declaration
struct MessageNode : public DeclarationNode {
  explicit MessageNode(std::string name) : name(std::move(name)) {}

  std::string name;
  std::vector<std::unique_ptr<FieldNode>> fields;
};

// Namespace Declaration
struct NamespaceNode : public DeclarationNode {
  explicit NamespaceNode(std::string name) : name(std::move(name)) {}

  std::string name;
  std::vector<std::unique_ptr<DeclarationNode>> declarations;
};

// Schema Root
struct SchemaNode : public AstNode {
  std::vector<std::unique_ptr<DeclarationNode>> declarations;
};

} // namespace jk_ridl
