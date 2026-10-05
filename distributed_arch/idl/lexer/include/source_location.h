#pragma once

#include <cstdint>
#include <string>

namespace jk_ridl {

struct SourceLocation {
  std::string file;
  uint32_t line = 1;
  uint32_t column = 1;
};

struct SourceRange {
  SourceLocation start;
  SourceLocation end;
};

} // namespace jk_ridl
