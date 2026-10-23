// A small JSON value: what jk-linkd, jk-link and the peers exchange (one
// object per line). Numbers are doubles, which hold every integer used here.
#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace jkl {

class Json {
public:
    using Array = std::vector<Json>;
    using Object = std::map<std::string, Json>;

    Json() = default;
    Json(std::nullptr_t) {}
    Json(bool b) : v_(b) {}
    Json(int n) : v_(double(n)) {}
    Json(long n) : v_(double(n)) {}
    Json(long long n) : v_(double(n)) {}
    Json(unsigned n) : v_(double(n)) {}
    Json(unsigned long n) : v_(double(n)) {}
    Json(unsigned long long n) : v_(double(n)) {}
    Json(double d) : v_(d) {}
    Json(const char *s) : v_(std::string(s)) {}
    Json(std::string s) : v_(std::move(s)) {}
    Json(Array a) : v_(std::make_shared<Array>(std::move(a))) {}
    Json(Object o) : v_(std::make_shared<Object>(std::move(o))) {}

    bool isNull() const { return v_.index() == 0; }
    bool isBool() const { return v_.index() == 1; }
    bool isNumber() const { return v_.index() == 2; }
    bool isString() const { return v_.index() == 3; }
    bool isArray() const { return v_.index() == 4; }
    bool isObject() const { return v_.index() == 5; }

    bool toBool(bool def = false) const { return isBool() ? std::get<1>(v_) : def; }
    double toNumber(double def = 0) const { return isNumber() ? std::get<2>(v_) : def; }
    std::string toString(const std::string &def = {}) const { return isString() ? std::get<3>(v_) : def; }
    const Array &array() const;
    const Object &object() const;

    // obj["key"]: the member, or null; set() adds or replaces one.
    const Json &operator[](const std::string &key) const;
    Json &set(const std::string &key, Json value);
    // push(): appends to an array (a null becomes an empty array first).
    Json &push(Json value);

    std::string dump() const;
    static std::optional<Json> parse(const std::string &text);

private:
    std::variant<std::monostate, bool, double, std::string, std::shared_ptr<Array>,
                 std::shared_ptr<Object>> v_;
};

} // namespace jkl
