#include "net.h"

#include <algorithm>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>

namespace jkl {

uint16_t Address::port() const
{
    if (sa.ss_family == AF_INET)
        return ntohs(reinterpret_cast<const sockaddr_in &>(sa).sin_port);
    if (sa.ss_family == AF_INET6)
        return ntohs(reinterpret_cast<const sockaddr_in6 &>(sa).sin6_port);
    return 0;
}

void Address::setPort(uint16_t port)
{
    if (sa.ss_family == AF_INET)
        reinterpret_cast<sockaddr_in &>(sa).sin_port = htons(port);
    else if (sa.ss_family == AF_INET6)
        reinterpret_cast<sockaddr_in6 &>(sa).sin6_port = htons(port);
}

std::string Address::host() const
{
    char buf[INET6_ADDRSTRLEN + IF_NAMESIZE + 2] = "";
    if (sa.ss_family == AF_INET) {
        inet_ntop(AF_INET, &reinterpret_cast<const sockaddr_in &>(sa).sin_addr, buf, sizeof buf);
        return buf;
    }
    if (sa.ss_family == AF_INET6) {
        auto &s6 = reinterpret_cast<const sockaddr_in6 &>(sa);
        // An IPv4 address on a dual-stack socket shows as plain IPv4.
        if (IN6_IS_ADDR_V4MAPPED(&s6.sin6_addr)) {
            inet_ntop(AF_INET, &s6.sin6_addr.s6_addr[12], buf, sizeof buf);
            return buf;
        }
        inet_ntop(AF_INET6, &s6.sin6_addr, buf, sizeof buf);
        std::string h = buf;
        if (s6.sin6_scope_id) {
            char ifname[IF_NAMESIZE];
            if (if_indextoname(s6.sin6_scope_id, ifname))
                h += std::string("%") + ifname;
            else
                h += "%" + std::to_string(s6.sin6_scope_id);
        }
        return h;
    }
    return "?";
}

std::string Address::str() const
{
    std::string h = host();
    if (h.find(':') != std::string::npos)
        return "[" + h + "]:" + std::to_string(port());
    return h + ":" + std::to_string(port());
}

bool Address::sameHost(const Address &o) const
{
    return host() == o.host();
}

Address Address::from(const sockaddr *s, socklen_t len)
{
    Address a;
    // An IPv4-mapped IPv6 address (dual-stack accept) becomes plain IPv4.
    if (s->sa_family == AF_INET6) {
        auto *s6 = reinterpret_cast<const sockaddr_in6 *>(s);
        if (IN6_IS_ADDR_V4MAPPED(&s6->sin6_addr)) {
            sockaddr_in s4{};
            s4.sin_family = AF_INET;
            s4.sin_port = s6->sin6_port;
            std::memcpy(&s4.sin_addr, &s6->sin6_addr.s6_addr[12], 4);
            std::memcpy(&a.sa, &s4, sizeof s4);
            a.len = sizeof s4;
            return a;
        }
    }
    len = std::min<socklen_t>(len, sizeof a.sa);
    std::memcpy(&a.sa, s, len);
    a.len = len;
    return a;
}

std::optional<Address> Address::parse(const std::string &text, uint16_t defaultPort)
{
    std::string host = text, port;
    if (!host.empty() && host[0] == '[') {
        size_t close = host.find(']');
        if (close == std::string::npos)
            return std::nullopt;
        if (close + 1 < host.size()) {
            if (host[close + 1] != ':')
                return std::nullopt;
            port = host.substr(close + 2);
        }
        host = host.substr(1, close - 1);
    } else if (std::count(host.begin(), host.end(), ':') == 1) {
        size_t colon = host.find(':');
        port = host.substr(colon + 1);
        host = host.substr(0, colon);
    }
    unsigned long p = defaultPort;
    if (!port.empty()) {
        char *end = nullptr;
        p = std::strtoul(port.c_str(), &end, 10);
        if (*end || p == 0 || p > 65535)
            return std::nullopt;
    }
    addrinfo hints{}, *res = nullptr;
    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host.c_str(), nullptr, &hints, &res) != 0 || !res)
        return std::nullopt;
    Address a = from(res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
    a.setPort(uint16_t(p));
    return a;
}

} // namespace jkl
