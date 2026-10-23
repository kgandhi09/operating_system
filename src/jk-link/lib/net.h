// Addresses as jk-link shows and stores them: "192.168.1.42:7710",
// "[fe80::1%wlp4s0]:7710".
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <sys/socket.h>

namespace jkl {

struct Address {
    sockaddr_storage sa{};
    socklen_t len = 0;

    int family() const { return sa.ss_family; }
    uint16_t port() const;
    void setPort(uint16_t port);
    std::string host() const;   // without the port
    std::string str() const;    // with it
    bool sameHost(const Address &o) const;
    bool operator==(const Address &o) const { return str() == o.str(); }

    static Address from(const sockaddr *sa, socklen_t len);
    // "host", "host:port", "[v6]:port", "v6%if"; a missing port is defaultPort.
    static std::optional<Address> parse(const std::string &text, uint16_t defaultPort);
};

} // namespace jkl
