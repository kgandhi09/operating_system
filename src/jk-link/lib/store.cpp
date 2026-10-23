#include "store.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <unistd.h>

namespace jkl {

namespace {

std::vector<std::string> lines(const std::string &path)
{
    std::vector<std::string> out;
    std::ifstream f(path);
    std::string l;
    while (std::getline(f, l))
        if (!l.empty() && l[0] != '#')
            out.push_back(l);
    return out;
}

bool writeFile(const std::string &path, const std::string &text)
{
    std::string tmp = path + ".new";
    FILE *f = std::fopen(tmp.c_str(), "w");
    if (!f)
        return false;
    bool ok = std::fwrite(text.data(), 1, text.size(), f) == text.size() && std::fflush(f) == 0
              && fsync(fileno(f)) == 0;
    ok = std::fclose(f) == 0 && ok;
    if (!ok || std::rename(tmp.c_str(), path.c_str()) != 0) {
        std::remove(tmp.c_str());
        return false;
    }
    return true;
}

} // namespace

const char *visibilityName(Visibility v)
{
    switch (v) {
    case Visibility::Open: return "open";
    case Visibility::Paired: return "paired";
    case Visibility::Off: return "off";
    }
    return "open";
}

bool parseVisibility(const std::string &s, Visibility &v)
{
    if (s == "open") v = Visibility::Open;
    else if (s == "paired") v = Visibility::Paired;
    else if (s == "off") v = Visibility::Off;
    else return false;
    return true;
}

void Store::load()
{
    trust.clear();
    for (auto &l : lines(dir + "/trust")) {
        std::istringstream in(l);
        std::string hex, name;
        int64_t since = 0;
        PublicKey k;
        if (!(in >> hex >> since) || !fromHex(hex, k))
            continue;
        std::getline(in >> std::ws, name);
        trust[k] = {name, since};
    }
    manual.clear();
    for (auto &l : lines(dir + "/link.conf")) {
        std::istringstream in(l);
        std::string key, value;
        in >> key >> value;
        if (key == "visibility")
            parseVisibility(value, visibility);
        else if (key == "peer" && !value.empty())
            manual.push_back(value);
    }
    cache.clear();
    for (auto &l : lines(dir + "/cache")) {
        std::istringstream in(l);
        std::string hex, addr;
        PublicKey k;
        if (in >> hex >> addr && fromHex(hex, k))
            cache[k].push_back(addr);
    }
}

bool Store::saveTrust() const
{
    std::string s = "# jk-link: the paired machines (public key, paired since, name then)\n";
    for (auto &[k, t] : trust)
        s += toHex(k.data(), k.size()) + " " + std::to_string(t.since) + " " + t.name + "\n";
    return writeFile(dir + "/trust", s);
}

bool Store::saveConfig() const
{
    std::string s = "# jk-link settings (jk-link visibility, jk-link add/remove)\n";
    s += std::string("visibility ") + visibilityName(visibility) + "\n";
    for (auto &m : manual)
        s += "peer " + m + "\n";
    return writeFile(dir + "/link.conf", s);
}

bool Store::saveCache() const
{
    std::string s = "# jk-link: where paired machines were last seen\n";
    for (auto &[k, addrs] : cache)
        for (auto &a : addrs)
            s += toHex(k.data(), k.size()) + " " + a + "\n";
    return writeFile(dir + "/cache", s);
}

} // namespace jkl
