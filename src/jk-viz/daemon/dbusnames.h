// jk-vizd: the D-Bus names each process owns, on the system bus and on the
// users' session buses (the labels of jk-viz's D-Bus edges).
#pragma once
#include <map>
#include <set>
#include <string>
#include <vector>

namespace jkv {

struct Bus {
    std::string address;
    unsigned uid = 0;   // queried as this user (a session bus only lets its user in)
    bool operator<(const Bus &o) const { return uid != o.uid ? uid < o.uid : address < o.address; }
    bool operator==(const Bus &o) const = default;
};

class DbusNames {
public:
    explicit DbusNames(std::string self) : self_(std::move(self)) {}
    // The names by pid, asked again at most every few seconds.
    const std::map<int, std::vector<std::string>> &get(const std::set<Bus> &buses);

private:
    std::string self_;
    std::map<int, std::vector<std::string>> names_;
    unsigned long long next_ = 0;
    std::set<Bus> lastBuses_;
};

// jk-vizd --dbus-names <address>: prints "<pid> <name>" for each well-known name.
int dbusNamesMain(const char *address);

} // namespace jkv
