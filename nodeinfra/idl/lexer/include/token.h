#pragma once

#include <string>

#include "source_location.h"
#include "token_type.h"

namespace jk_ridl {

struct Token {
  TokenType type = TokenType::Unknown;
  std::string lexeme;
  SourceRange range;
};

} // namespace jk_ridl
