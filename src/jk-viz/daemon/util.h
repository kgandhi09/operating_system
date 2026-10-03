// jk-vizd: small helpers (files, strings, JSON output).
#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace jkv {

// The whole file, or "" when it can't be read (procfs files vanish all the time).
std::string readFile(const std::string &path, size_t max = 1 << 20);
// The first line, without the newline.
std::string readLine(const std::string &path);
std::string readLink(const std::string &path);
bool exists(const std::string &path);
std::vector<std::string> listDir(const std::string &path);
std::vector<std::string_view> split(std::string_view s, char sep);
std::string trim(std::string_view s);
bool startsWith(std::string_view s, std::string_view p);
uint64_t toU64(std::string_view s, int base = 10);
// "key:   value kB" lines (/proc/meminfo, status, io, smaps_rollup): the value of key.
uint64_t fieldValue(std::string_view text, std::string_view key);
std::string baseName(const std::string &path);
uint64_t nowMs();       // CLOCK_MONOTONIC
uint64_t wallMs();      // CLOCK_REALTIME

// A JSON writer: appends to a string, escapes, and replaces invalid UTF-8.
class Json {
public:
    std::string out;
    Json &beginObj() { sep(); out += '{'; first_ = true; return *this; }
    Json &endObj() { out += '}'; first_ = false; return *this; }
    Json &beginArr() { sep(); out += '['; first_ = true; return *this; }
    Json &endArr() { out += ']'; first_ = false; return *this; }
    Json &key(std::string_view k) { sep(); str(k); out += ':'; first_ = true; return *this; }
    Json &val(std::string_view s) { sep(); str(s); return *this; }
    Json &val(const char *s) { return val(std::string_view(s)); }
    Json &val(const std::string &s) { return val(std::string_view(s)); }
    Json &val(double d);
    Json &val(int64_t i) { sep(); out += std::to_string(i); return *this; }
    Json &val(uint64_t i) { sep(); out += std::to_string(i); return *this; }
    Json &val(int i) { return val(int64_t(i)); }
    Json &val(bool b) { sep(); out += b ? "true" : "false"; return *this; }
    template <class T> Json &kv(std::string_view k, const T &v) { key(k); return val(v); }

private:
    bool first_ = true;
    void sep() { if (!first_) out += ','; first_ = false; }
    void str(std::string_view s);
};

} // namespace jkv
