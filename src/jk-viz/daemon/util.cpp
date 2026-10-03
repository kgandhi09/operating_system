#include "util.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

namespace jkv {

std::string readFile(const std::string &path, size_t max)
{
    std::string s;
    int fd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0)
        return s;
    char buf[8192];
    while (s.size() < max) {
        ssize_t n = read(fd, buf, sizeof buf);
        if (n <= 0)
            break;
        s.append(buf, size_t(n));
    }
    close(fd);
    return s;
}

std::string readLine(const std::string &path)
{
    std::string s = readFile(path, 4096);
    size_t nl = s.find('\n');
    if (nl != std::string::npos)
        s.resize(nl);
    return s;
}

std::string readLink(const std::string &path)
{
    char buf[4096];
    ssize_t n = readlink(path.c_str(), buf, sizeof buf - 1);
    if (n < 0)
        return {};
    return std::string(buf, size_t(n));
}

bool exists(const std::string &path)
{
    return access(path.c_str(), F_OK) == 0;
}

std::vector<std::string> listDir(const std::string &path)
{
    std::vector<std::string> v;
    DIR *d = opendir(path.c_str());
    if (!d)
        return v;
    while (dirent *e = readdir(d)) {
        if (e->d_name[0] == '.' && (!e->d_name[1] || (e->d_name[1] == '.' && !e->d_name[2])))
            continue;
        v.emplace_back(e->d_name);
    }
    closedir(d);
    return v;
}

std::vector<std::string_view> split(std::string_view s, char sep)
{
    std::vector<std::string_view> v;
    size_t i = 0;
    while (i <= s.size()) {
        if (sep == ' ') {
            while (i < s.size() && (s[i] == ' ' || s[i] == '\t'))
                i++;
            if (i >= s.size())
                break;
        }
        size_t j = i;
        while (j < s.size() && s[j] != sep && !(sep == ' ' && s[j] == '\t'))
            j++;
        v.push_back(s.substr(i, j - i));
        i = j + 1;
    }
    return v;
}

std::string trim(std::string_view s)
{
    size_t a = 0, b = s.size();
    while (a < b && isspace((unsigned char)s[a]))
        a++;
    while (b > a && isspace((unsigned char)s[b - 1]))
        b--;
    return std::string(s.substr(a, b - a));
}

bool startsWith(std::string_view s, std::string_view p)
{
    return s.substr(0, p.size()) == p;
}

uint64_t toU64(std::string_view s, int base)
{
    uint64_t v = 0;
    for (char c : s) {
        int d;
        if (c >= '0' && c <= '9')
            d = c - '0';
        else if (base == 16 && c >= 'a' && c <= 'f')
            d = c - 'a' + 10;
        else if (base == 16 && c >= 'A' && c <= 'F')
            d = c - 'A' + 10;
        else if (base == 16 && (c == 'x' || c == 'X') && v == 0)
            continue;
        else
            break;
        if (d >= base)
            break;
        v = v * uint64_t(base) + uint64_t(d);
    }
    return v;
}

uint64_t fieldValue(std::string_view text, std::string_view key)
{
    size_t pos = 0;
    while (pos < text.size()) {
        size_t nl = text.find('\n', pos);
        if (nl == std::string_view::npos)
            nl = text.size();
        std::string_view line = text.substr(pos, nl - pos);
        if (line.size() > key.size() && line.substr(0, key.size()) == key && line[key.size()] == ':') {
            std::string_view rest = line.substr(key.size() + 1);
            size_t i = 0;
            while (i < rest.size() && (rest[i] == ' ' || rest[i] == '\t'))
                i++;
            return toU64(rest.substr(i));
        }
        pos = nl + 1;
    }
    return 0;
}

std::string baseName(const std::string &path)
{
    size_t s = path.rfind('/');
    return s == std::string::npos ? path : path.substr(s + 1);
}

uint64_t nowMs()
{
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return uint64_t(ts.tv_sec) * 1000 + uint64_t(ts.tv_nsec) / 1000000;
}

uint64_t wallMs()
{
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return uint64_t(ts.tv_sec) * 1000 + uint64_t(ts.tv_nsec) / 1000000;
}

Json &Json::val(double d)
{
    sep();
    if (!std::isfinite(d))
        d = 0;
    char buf[32];
    // Enough digits for shares and rates, without noise.
    snprintf(buf, sizeof buf, "%.6g", d);
    out += buf;
    return *this;
}

void Json::str(std::string_view s)
{
    out += '"';
    const auto *p = reinterpret_cast<const unsigned char *>(s.data());
    size_t n = s.size();
    for (size_t i = 0; i < n; i++) {
        unsigned char c = p[i];
        if (c == '"' || c == '\\') {
            out += '\\';
            out += char(c);
        } else if (c < 0x20) {
            char buf[8];
            snprintf(buf, sizeof buf, "\\u%04x", c);
            out += buf;
        } else if (c < 0x80) {
            out += char(c);
        } else {
            // A valid UTF-8 sequence is copied; anything else becomes U+FFFD.
            int len = (c >= 0xc2 && c <= 0xdf) ? 2 : (c >= 0xe0 && c <= 0xef) ? 3 : (c >= 0xf0 && c <= 0xf4) ? 4 : 0;
            bool ok = len && i + size_t(len) <= n;
            for (int k = 1; ok && k < len; k++)
                ok = (p[i + size_t(k)] & 0xc0) == 0x80;
            if (ok) {
                out.append(reinterpret_cast<const char *>(p + i), size_t(len));
                i += size_t(len) - 1;
            } else {
                out += "\xef\xbf\xbd";
            }
        }
    }
    out += '"';
}

} // namespace jkv
