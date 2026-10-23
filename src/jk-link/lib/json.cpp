#include "json.h"

#include <cctype>
#include <cmath>
#include <cstdio>

namespace jkl {

namespace {

const Json kNull;
const Json::Array kEmptyArray;
const Json::Object kEmptyObject;

void dumpString(const std::string &s, std::string &out)
{
    out += '"';
    for (unsigned char c : s) {
        switch (c) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (c < 0x20) {
                char buf[8];
                std::snprintf(buf, sizeof buf, "\\u%04x", c);
                out += buf;
            } else {
                out += char(c);
            }
        }
    }
    out += '"';
}

struct Parser {
    const std::string &s;
    size_t i = 0;
    int depth = 0;

    void ws()
    {
        while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r'))
            ++i;
    }
    bool lit(const char *w)
    {
        size_t n = std::char_traits<char>::length(w);
        if (s.compare(i, n, w) != 0)
            return false;
        i += n;
        return true;
    }
    static void utf8(unsigned cp, std::string &out)
    {
        if (cp < 0x80) {
            out += char(cp);
        } else if (cp < 0x800) {
            out += char(0xC0 | (cp >> 6));
            out += char(0x80 | (cp & 0x3F));
        } else if (cp < 0x10000) {
            out += char(0xE0 | (cp >> 12));
            out += char(0x80 | ((cp >> 6) & 0x3F));
            out += char(0x80 | (cp & 0x3F));
        } else {
            out += char(0xF0 | (cp >> 18));
            out += char(0x80 | ((cp >> 12) & 0x3F));
            out += char(0x80 | ((cp >> 6) & 0x3F));
            out += char(0x80 | (cp & 0x3F));
        }
    }
    bool hex4(unsigned &v)
    {
        if (i + 4 > s.size())
            return false;
        v = 0;
        for (int k = 0; k < 4; ++k) {
            char c = s[i++];
            v <<= 4;
            if (c >= '0' && c <= '9') v |= unsigned(c - '0');
            else if (c >= 'a' && c <= 'f') v |= unsigned(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') v |= unsigned(c - 'A' + 10);
            else return false;
        }
        return true;
    }
    bool str(std::string &out)
    {
        if (i >= s.size() || s[i] != '"')
            return false;
        ++i;
        while (i < s.size()) {
            char c = s[i++];
            if (c == '"')
                return true;
            if (c != '\\') {
                out += c;
                continue;
            }
            if (i >= s.size())
                return false;
            char e = s[i++];
            switch (e) {
            case '"': out += '"'; break;
            case '\\': out += '\\'; break;
            case '/': out += '/'; break;
            case 'b': out += '\b'; break;
            case 'f': out += '\f'; break;
            case 'n': out += '\n'; break;
            case 'r': out += '\r'; break;
            case 't': out += '\t'; break;
            case 'u': {
                unsigned cp;
                if (!hex4(cp))
                    return false;
                if (cp >= 0xD800 && cp < 0xDC00 && lit("\\u")) {
                    unsigned lo;
                    if (!hex4(lo))
                        return false;
                    cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                }
                utf8(cp, out);
                break;
            }
            default:
                return false;
            }
        }
        return false;
    }
    std::optional<Json> value()
    {
        if (++depth > 64)
            return std::nullopt;
        ws();
        std::optional<Json> r;
        if (i >= s.size()) {
            r = std::nullopt;
        } else if (s[i] == '{') {
            ++i;
            Json::Object o;
            ws();
            if (i < s.size() && s[i] == '}') {
                ++i;
                r = Json(std::move(o));
            } else {
                for (;;) {
                    ws();
                    std::string k;
                    if (!str(k)) break;
                    ws();
                    if (i >= s.size() || s[i] != ':') break;
                    ++i;
                    auto v = value();
                    if (!v) break;
                    o[k] = std::move(*v);
                    ws();
                    if (i < s.size() && s[i] == ',') { ++i; continue; }
                    if (i < s.size() && s[i] == '}') { ++i; r = Json(std::move(o)); }
                    break;
                }
            }
        } else if (s[i] == '[') {
            ++i;
            Json::Array a;
            ws();
            if (i < s.size() && s[i] == ']') {
                ++i;
                r = Json(std::move(a));
            } else {
                for (;;) {
                    auto v = value();
                    if (!v) break;
                    a.push_back(std::move(*v));
                    ws();
                    if (i < s.size() && s[i] == ',') { ++i; continue; }
                    if (i < s.size() && s[i] == ']') { ++i; r = Json(std::move(a)); }
                    break;
                }
            }
        } else if (s[i] == '"') {
            std::string v;
            if (str(v))
                r = Json(std::move(v));
        } else if (lit("true")) {
            r = Json(true);
        } else if (lit("false")) {
            r = Json(false);
        } else if (lit("null")) {
            r = Json();
        } else {
            size_t start = i;
            if (s[i] == '-') ++i;
            while (i < s.size() && (std::isdigit((unsigned char)s[i]) || s[i] == '.' || s[i] == 'e'
                                    || s[i] == 'E' || s[i] == '+' || s[i] == '-'))
                ++i;
            if (i > start) {
                try {
                    size_t used = 0;
                    double d = std::stod(s.substr(start, i - start), &used);
                    if (used == i - start)
                        r = Json(d);
                } catch (...) {
                }
            }
        }
        --depth;
        return r;
    }
};

} // namespace

const Json::Array &Json::array() const
{
    return isArray() ? *std::get<4>(v_) : kEmptyArray;
}

const Json::Object &Json::object() const
{
    return isObject() ? *std::get<5>(v_) : kEmptyObject;
}

const Json &Json::operator[](const std::string &key) const
{
    if (!isObject())
        return kNull;
    auto &o = *std::get<5>(v_);
    auto it = o.find(key);
    return it == o.end() ? kNull : it->second;
}

Json &Json::set(const std::string &key, Json value)
{
    if (!isObject())
        v_ = std::make_shared<Object>();
    (*std::get<5>(v_))[key] = std::move(value);
    return *this;
}

Json &Json::push(Json value)
{
    if (!isArray())
        v_ = std::make_shared<Array>();
    std::get<4>(v_)->push_back(std::move(value));
    return *this;
}

std::string Json::dump() const
{
    std::string out;
    switch (v_.index()) {
    case 0: out = "null"; break;
    case 1: out = std::get<1>(v_) ? "true" : "false"; break;
    case 2: {
        double d = std::get<2>(v_);
        char buf[32];
        if (!std::isfinite(d))
            out = "null";
        else if (d == std::floor(d) && std::fabs(d) < 1e15)
            out = (std::snprintf(buf, sizeof buf, "%.0f", d), buf);
        else
            out = (std::snprintf(buf, sizeof buf, "%.6g", d), buf);
        break;
    }
    case 3: dumpString(std::get<3>(v_), out); break;
    case 4: {
        out += '[';
        bool first = true;
        for (auto &v : *std::get<4>(v_)) {
            if (!first) out += ',';
            first = false;
            out += v.dump();
        }
        out += ']';
        break;
    }
    case 5: {
        out += '{';
        bool first = true;
        for (auto &[k, v] : *std::get<5>(v_)) {
            if (!first) out += ',';
            first = false;
            dumpString(k, out);
            out += ':';
            out += v.dump();
        }
        out += '}';
        break;
    }
    }
    return out;
}

std::optional<Json> Json::parse(const std::string &text)
{
    Parser p{text};
    auto v = p.value();
    if (!v)
        return std::nullopt;
    p.ws();
    if (p.i != text.size())
        return std::nullopt;
    return v;
}

} // namespace jkl
