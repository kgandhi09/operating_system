#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <parser.h>
#include <set>
#include <sstream>

namespace fs = std::filesystem;
using namespace jk_ridl;
namespace {
constexpr std::size_t max_payload = 65536;
constexpr const char *usage =
    "usage: jkbuf [-I directory] [--depfile output.d] -o output.h input.jkbuf\n";
std::string depfile_path(const fs::path &path) {
  std::string escaped;
  for (char c : fs::absolute(path).generic_string()) {
    if (c == '\n' || c == '\r')
      throw std::runtime_error("depfile paths cannot contain newlines");
    if (c == '$')
      escaped += '$';
    else if (c == ' ' || c == '\t' || c == '#' || c == '\\' || c == ':')
      escaped += '\\';
    escaped += c;
  }
  return escaped;
}
std::uint64_t hash(const std::string &s) {
  std::uint64_t h = 14695981039346656037ULL;
  for (unsigned char c : s) {
    h ^= c;
    h *= 1099511628211ULL;
  }
  return h;
}
std::string join(const std::string &a, const std::string &b) {
  return a.empty() ? b : a + "." + b;
}
std::string symbol(std::string name) {
  std::replace(name.begin(), name.end(), '.', '_');
  return "jkbuf_" + name;
}
struct Decl {
  MessageNode *node;
  std::string scope;
};
struct Repr {
  std::string name, validate, signature;
  std::size_t size, align;
};
class Compiler {
  std::vector<std::unique_ptr<SchemaNode>> schemas_;
  std::map<std::string, Decl> messages_;
  std::map<std::string, Repr> generated_;
  std::set<std::string> active_files_, loaded_, visiting_, symbols_;
  std::set<fs::path> inputs_;
  std::vector<fs::path> includes_;
  std::ostringstream body_, traits_;
  std::size_t bytes_ = 0;
  static std::size_t aligned(std::size_t n, std::size_t a) {
    return (n + a - 1) / a * a;
  }
  void collect(std::vector<std::unique_ptr<DeclarationNode>> &decls,
               const std::string &scope, const fs::path &file) {
    if (scope.size() > 512)
      throw std::runtime_error("namespace name too long");
    for (auto &d : decls) {
      if (auto n = dynamic_cast<NamespaceNode *>(d.get()))
        collect(n->declarations, join(scope, n->name), file);
      else if (auto inc = dynamic_cast<IncludeNode *>(d.get())) {
        fs::path resolved = file.parent_path() / inc->path;
        if (!fs::exists(resolved)) {
          for (auto &dir : includes_)
            if (fs::exists(dir / inc->path)) {
              resolved = dir / inc->path;
              break;
            }
        }
        if (!fs::exists(resolved))
          throw Error(inc->range.start, "include not found: " + inc->path);
        load(resolved, scope);
      } else if (auto m = dynamic_cast<MessageNode *>(d.get())) {
        auto full = join(scope, m->name);
        if (messages_.size() >= 1024)
          throw Error(m->range.start, "too many messages (limit 1024)");
        if (!messages_.emplace(full, Decl{m, scope}).second)
          throw Error(m->range.start, "duplicate message: " + full);
        if (symbol(full).starts_with("jkbuf_detail_"))
          throw Error(m->range.start,
                      "generated name prefix jkbuf_detail_ is reserved");
        for (const char *suffix : {"", "_init", "_valid", "_SCHEMA",
                                   "_size_check", "_DEFINED", "_TRAITS"}) {
          auto generated_name = symbol(full) + suffix;
          if (generated_name == "jkbuf_boolean_valid" ||
              !symbols_.insert(generated_name).second)
            throw Error(m->range.start, "generated C name collision: " + full);
        }
        std::set<std::string> fields;
        for (auto &f : m->fields)
          if (!fields.insert(f->name).second)
            throw Error(f->range.start, "duplicate field: " + f->name);
      }
    }
  }
  void load(const fs::path &input, const std::string &scope) {
    auto path = fs::canonical(input).string();
    inputs_.insert(path);
    if (active_files_.contains(path))
      throw std::runtime_error(path + ": cyclic include");
    if (active_files_.size() >= 64)
      throw std::runtime_error("include nesting exceeds 64 files");
    if (!loaded_.insert(path + "@" + scope).second)
      return;
    auto size = fs::file_size(path);
    if (size > 1024 * 1024 || (bytes_ += size) > 8 * 1024 * 1024)
      throw std::runtime_error(path + ": schema input size limit exceeded");
    std::ifstream in(path);
    if (!in)
      throw std::runtime_error("cannot read " + path);
    std::string text((std::istreambuf_iterator<char>(in)), {});
    active_files_.insert(path);
    auto schema = std::make_unique<SchemaNode>(Parser(text, path).Parse());
    auto *root = schema.get();
    schemas_.push_back(std::move(schema));
    collect(root->declarations, scope, path);
    active_files_.erase(path);
  }
  Repr type(TypeNode &t, const std::string &scope) {
    if (auto p = dynamic_cast<PrimitiveTypeNode *>(&t)) {
      static const char *names[] = {
          "uint8_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "int8_t",
          "int16_t", "int32_t", "int64_t",  "float",    "double"};
      static const std::size_t sizes[] = {1, 1, 2, 4, 8, 1, 2, 4, 8, 4, 8};
      auto i = static_cast<unsigned>(p->kind);
      return {names[i], i == 0 ? "jkbuf_boolean_valid" : "",
              "p" + std::to_string(i), sizes[i], sizes[i]};
    }
    if (auto n = dynamic_cast<NamedTypeNode *>(&t)) {
      std::string current = scope;
      for (;;) {
        auto full = join(current, n->name);
        if (messages_.contains(full))
          return message(full);
        if (current.empty())
          break;
        auto dot = current.rfind('.');
        current = dot == std::string::npos ? "" : current.substr(0, dot);
      }
      throw Error(t.range.start, "unknown type: " + n->name);
    }
    bool string = false, vector = false;
    std::size_t n;
    Repr element;
    if (auto a = dynamic_cast<ArrayTypeNode *>(&t)) {
      n = a->length;
      element = type(*a->element_type, scope);
    } else if (auto v = dynamic_cast<VectorTypeNode *>(&t)) {
      n = v->capacity;
      vector = true;
      element = type(*v->element_type, scope);
    } else if (auto s = dynamic_cast<StringTypeNode *>(&t)) {
      n = s->max_length;
      string = true;
      element = {"uint8_t", "", "bytes", 1, 1};
    } else
      throw Error(t.range.start, "unsupported type");
    bool bounded = string || vector;
    auto align =
        std::max(element.align, bounded ? std::size_t(4) : std::size_t(1));
    auto size = aligned(
        (bounded ? aligned(4, element.align) : 0) + element.size * n, align);
    if (size > max_payload)
      throw Error(t.range.start, "type exceeds 65536-byte local payload limit");
    auto signature = (string   ? "s"
                      : vector ? "v"
                               : "a") +
                     std::to_string(n) + "<" + element.signature + ">";
    auto name = "jkbuf_detail_" + std::to_string(hash(signature));
    body_ << "#ifndef " << name << "_DEFINED\n#define " << name << "_DEFINED\n";
    body_ << "typedef struct " << name << " {\n";
    if (bounded)
      body_ << "  uint32_t length;\n";
    body_ << "  " << element.name << " data[" << n << "];\n} " << name << ";\n";
    body_ << "static inline bool " << name << "_valid(const " << name
          << " *v) {\n  if (!v) return false;\n";
    if (bounded)
      body_ << "  if (v->length > " << n << ") return false;\n";
    if (!element.validate.empty())
      body_ << "  for (uint32_t i = 0; i < "
            << (bounded ? "v->length" : std::to_string(n))
            << "; ++i)\n    if (!" << element.validate
            << "(&v->data[i])) return false;\n";
    body_ << "  return true;\n}\n";
    body_ << "#endif\n";
    return {name, name + "_valid", signature, size, align};
  }
  Repr message(const std::string &full) {
    if (generated_.contains(full))
      return generated_.at(full);
    auto &d = messages_.at(full);
    if (!visiting_.insert(full).second)
      throw Error(d.node->range.start, "recursive message type: " + full);
    if (visiting_.size() > 64)
      throw Error(d.node->range.start, "type dependency depth exceeds 64");
    std::vector<std::pair<FieldNode *, Repr>> fields;
    std::size_t size = 0, align = 1;
    std::string canonical = "jkbuf-local-v1:" + full + "{";
    for (auto &f : d.node->fields) {
      auto r = type(*f->type, d.scope);
      size = aligned(size, r.align) + r.size;
      align = std::max(align, r.align);
      if (size > max_payload)
        throw Error(f->range.start,
                    "message exceeds 65536-byte local payload limit");
      canonical += f->name + ":" + r.signature + ";";
      fields.emplace_back(f.get(), r);
    }
    size = aligned(std::max(size, std::size_t(1)), align);
    if (size > max_payload)
      throw Error(d.node->range.start,
                  "message exceeds 65536-byte local payload limit");
    auto id = hash(canonical + "}");
    auto name = symbol(full);
    body_ << "\n#ifndef " << name << "_DEFINED\n#define " << name
          << "_DEFINED\n";
    body_ << "/* RIDL: " << full << " */\ntypedef struct " << name << " {\n";
    for (auto &[f, r] : fields)
      body_ << "  " << r.name << " f_" << f->name << ";\n";
    if (fields.empty())
      body_ << "  uint8_t jk_empty;\n";
    body_ << "} " << name << ";\n";
    body_ << "typedef char " << name << "_size_check[(sizeof(" << name
          << ") == " << size << ") ? 1 : -1];\n";
    body_ << "#define " << name << "_SCHEMA UINT64_C(" << id << ")\n";
    body_ << "static inline void " << name << "_init(" << name
          << " *v) { memset(v, 0, sizeof(*v)); }\n";
    body_ << "static inline bool " << name << "_valid(const " << name
          << " *v) {\n  if (!v) return false;\n";
    for (auto &[f, r] : fields)
      if (!r.validate.empty())
        body_ << "  if (!" << r.validate << "(&v->f_" << f->name
              << ")) return false;\n";
    body_ << "  return true;\n}\n";
    body_ << "#elif " << name << "_SCHEMA != UINT64_C(" << id
          << ")\n#error Conflicting_RIDL_schema_for_" << name << "\n#endif\n";
    traits_ << "#ifndef " << name << "_TRAITS\n#define " << name << "_TRAITS\n";
    traits_ << "template<> struct MessageTraits<::" << name << "> {\n"
            << "  static constexpr uint64_t schema = " << name << "_SCHEMA;\n"
            << "  static bool valid(const ::" << name << " &v) { return "
            << name << "_valid(&v); }\n};\n#endif\n";
    Repr r{name, name + "_valid", "m" + full + ":" + std::to_string(id), size,
           align};
    generated_[full] = r;
    visiting_.erase(full);
    return r;
  }

public:
  const std::set<fs::path> &inputs() const { return inputs_; }
  explicit Compiler(std::vector<fs::path> includes)
      : includes_(std::move(includes)) {}
  std::string compile(const fs::path &input) {
    load(input, "");
    for (auto &[name, d] : messages_)
      message(name);
    auto body = body_.str();
    auto suffix = std::to_string(hash(body));
    return "/* Generated by jkbuf. Local native layout; not a network wire "
           "format. */\n"
           "#ifndef JKBUF_GENERATED_" +
           suffix + "\n#define JKBUF_GENERATED_" + suffix +
           "\n"
           "#include <stdint.h>\n#include <stdbool.h>\n#include <string.h>\n"
           "#ifndef JKBUF_BOOLEAN_VALID_DEFINED\n#define "
           "JKBUF_BOOLEAN_VALID_DEFINED\n"
           "static inline bool jkbuf_boolean_valid(const uint8_t *v) { return "
           "*v <= 1; }\n#endif\n" +
           body +
           "\n#ifdef __cplusplus\n#include <jk/message.hpp>\nnamespace jk {\n" +
           traits_.str() + "}\n#endif\n#endif\n";
  }
};
} // namespace
int main(int argc, char **argv) {
  try {
    fs::path input, output, depfile;
    std::vector<fs::path> includes;
    for (int i = 1; i < argc; ++i) {
      std::string a = argv[i];
      if (a == "--help") {
        std::cout << usage;
        return 0;
      }
      if ((a == "-o" || a == "-I" || a == "--depfile") && i + 1 < argc) {
        if (a == "-o")
          output = argv[++i];
        else if (a == "--depfile")
          depfile = argv[++i];
        else
          includes.emplace_back(argv[++i]);
      } else if (!a.empty() && a[0] != '-' && input.empty())
        input = a;
      else
        throw std::runtime_error("invalid arguments (use --help)");
    }
    if (input.empty() || output.empty())
      throw std::runtime_error(usage);
    Compiler compiler(std::move(includes));
    auto result = compiler.compile(input);
    auto same_file = [](const fs::path &a, const fs::path &b) {
      return fs::weakly_canonical(a) == fs::weakly_canonical(b) ||
             (fs::exists(a) && fs::exists(b) && fs::equivalent(a, b));
    };
    for (const auto &source : compiler.inputs()) {
      if (same_file(source, output) ||
          (!depfile.empty() && same_file(source, depfile)))
        throw std::runtime_error("output must not overwrite input: " + source.string());
    }
    std::string dependencies;
    if (!depfile.empty()) {
      if (same_file(output, depfile))
        throw std::runtime_error("depfile must differ from output header");
      dependencies = depfile_path(output) + ":";
      for (const auto &source : compiler.inputs())
        dependencies += " " + depfile_path(source);
      dependencies += '\n';
      std::ofstream deps(depfile);
      if (!deps || !(deps << dependencies) || !deps.flush())
        throw std::runtime_error("cannot write " + depfile.string());
    }
    std::ofstream out(output);
    if (!out || !(out << result) || !out.flush())
      throw std::runtime_error("cannot write " + output.string());
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "jkbuf: " << e.what() << '\n';
    return 1;
  }
}
