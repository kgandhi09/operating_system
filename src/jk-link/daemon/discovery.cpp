// Discovery: the beacons on UDP 7710.
//
// visibility open    every 5 s a signed announce, multicast on every
//                    interface that has multicast (IPv4 239.255.77.10, IPv6
//                    ff02::7710), and unicast to the addresses added by
//                    hand; queries are answered.
// visibility paired  announces only unicast, to the paired machines' last
//                    known addresses and those added by hand: invisible to
//                    the rest of the network.
// visibility off     no beacons at all; paired machines can still connect
//                    to the addresses they know.
// Beacons from others are always listened to. A machine is offline after
// 20 s without a beacon or link traffic, or at once when it says Leave.
#include "daemon.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <unistd.h>

namespace jkl {

namespace {

sockaddr_in group4()
{
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_port = htons(kPort);
    inet_pton(AF_INET, kGroup4, &a.sin_addr);
    return a;
}

sockaddr_in6 group6(unsigned ifindex)
{
    sockaddr_in6 a{};
    a.sin6_family = AF_INET6;
    a.sin6_port = htons(kPort);
    a.sin6_scope_id = ifindex;
    inet_pton(AF_INET6, kGroup6, &a.sin6_addr);
    return a;
}

int udpSocket(int family)
{
    int fd = ::socket(family, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd < 0)
        return -1;
    int one = 1, ttl = 1;
    // Shared: more than one jk-linkd on a machine (tests) all hear the
    // multicast beacons.
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof one);
    if (family == AF_INET) {
        sockaddr_in a{};
        a.sin_family = AF_INET;
        a.sin_port = htons(kPort);
        setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof ttl);
        setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &one, sizeof one);
        if (::bind(fd, reinterpret_cast<sockaddr *>(&a), sizeof a) == 0)
            return fd;
    } else {
        sockaddr_in6 a{};
        a.sin6_family = AF_INET6;
        a.sin6_port = htons(kPort);
        setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &one, sizeof one);
        setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &ttl, sizeof ttl);
        setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &one, sizeof one);
        if (::bind(fd, reinterpret_cast<sockaddr *>(&a), sizeof a) == 0)
            return fd;
    }
    ::close(fd);
    return -1;
}

} // namespace

bool Daemon::openDiscovery()
{
    udp4 = udpSocket(AF_INET);
    udp6 = udpSocket(AF_INET6);
    if (udp4 < 0 && udp6 < 0) {
        event("error", std::string("cannot open UDP port 7710: ") + std::strerror(errno));
        return false;
    }
    scanInterfaces();
    return true;
}

// The interfaces that can carry the beacons (up, multicast, not loopback or
// point-to-point): joined to the groups. Called every 10 s, so a Wi-Fi that
// comes up later is used too; a new one sends a query at once.
void Daemon::scanInterfaces()
{
    ifaddrs *list = nullptr;
    if (getifaddrs(&list) != 0)
        return;
    std::set<int> want4, want6;
    std::vector<std::string> addrs;
    for (ifaddrs *i = list; i; i = i->ifa_next) {
        if (!i->ifa_addr || !(i->ifa_flags & IFF_UP) || (i->ifa_flags & IFF_LOOPBACK))
            continue;
        int family = i->ifa_addr->sa_family;
        if (family != AF_INET && family != AF_INET6)
            continue;
        Address a = Address::from(i->ifa_addr, family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6));
        addrs.push_back(a.host());
        if (!(i->ifa_flags & IFF_MULTICAST) || (i->ifa_flags & IFF_POINTOPOINT))
            continue;
        int idx = int(if_nametoindex(i->ifa_name));
        (family == AF_INET ? want4 : want6).insert(idx);
    }
    freeifaddrs(list);
    ownAddresses = addrs;

    bool added = false;
    for (int idx : want4) {
        if (udp4 < 0 || joined4.count(idx))
            continue;
        ip_mreqn m{};
        m.imr_multiaddr = group4().sin_addr;
        m.imr_ifindex = idx;
        if (setsockopt(udp4, IPPROTO_IP, IP_ADD_MEMBERSHIP, &m, sizeof m) == 0 || errno == EADDRINUSE) {
            joined4.insert(idx);
            added = true;
        }
    }
    for (int idx : want6) {
        if (udp6 < 0 || joined6.count(idx))
            continue;
        ipv6_mreq m{};
        m.ipv6mr_multiaddr = group6(unsigned(idx)).sin6_addr;
        m.ipv6mr_interface = unsigned(idx);
        if (setsockopt(udp6, IPPROTO_IPV6, IPV6_JOIN_GROUP, &m, sizeof m) == 0 || errno == EADDRINUSE) {
            joined6.insert(idx);
            added = true;
        }
    }
    // Interfaces that went away leave the groups by themselves.
    std::erase_if(joined4, [&](int idx) { return !want4.count(idx); });
    std::erase_if(joined6, [&](int idx) { return !want6.count(idx); });
    if (added && store.visibility == Visibility::Open)
        sendBeacons(Beacon::Query);
}

void Daemon::sendBeaconTo(const Address &a, Beacon::Type type)
{
    std::vector<uint8_t> pkt = ownBeacon(type).encode(id);
    Address to = a;
    to.setPort(kPort);
    int fd = to.family() == AF_INET ? udp4 : udp6;
    if (fd >= 0)
        ::sendto(fd, pkt.data(), pkt.size(), 0, reinterpret_cast<const sockaddr *>(&to.sa), to.len);
}

void Daemon::sendBeacons(Beacon::Type type)
{
    if (store.visibility == Visibility::Off && type != Beacon::Leave)
        return;
    std::vector<uint8_t> pkt = ownBeacon(type).encode(id);
    if (store.visibility == Visibility::Open || type == Beacon::Leave) {
        for (int idx : joined4) {
            ip_mreqn m{};
            m.imr_ifindex = idx;
            setsockopt(udp4, IPPROTO_IP, IP_MULTICAST_IF, &m, sizeof m);
            sockaddr_in g = group4();
            ::sendto(udp4, pkt.data(), pkt.size(), 0, reinterpret_cast<sockaddr *>(&g), sizeof g);
        }
        for (int idx : joined6) {
            unsigned i = unsigned(idx);
            setsockopt(udp6, IPPROTO_IPV6, IPV6_MULTICAST_IF, &i, sizeof i);
            sockaddr_in6 g = group6(i);
            ::sendto(udp6, pkt.data(), pkt.size(), 0, reinterpret_cast<sockaddr *>(&g), sizeof g);
        }
    }
    // Unicast: the addresses added by hand, and (visibility paired, or a
    // goodbye) the paired machines where they were last seen.
    std::vector<Address> to;
    for (auto &m : store.manual)
        if (auto a = Address::parse(m, kPort))
            to.push_back(*a);
    if (store.visibility != Visibility::Open || type == Beacon::Leave) {
        for (auto &[key, t] : store.trust) {
            auto it = peers.find(key);
            if (it != peers.end() && !it->second.addrs.empty()) {
                to.push_back(it->second.addrs.front().addr);
            } else if (auto c = store.cache.find(key); c != store.cache.end()) {
                for (auto &s : c->second)
                    if (auto a = Address::parse(s, kPort))
                        to.push_back(*a);
            }
        }
    }
    for (auto &a : to) {
        Address u = a;
        u.setPort(kPort);
        int fd = u.family() == AF_INET ? udp4 : udp6;
        if (fd >= 0)
            ::sendto(fd, pkt.data(), pkt.size(), 0, reinterpret_cast<sockaddr *>(&u.sa), u.len);
    }
}

void Daemon::receiveBeacon(int fd)
{
    for (;;) {
        uint8_t buf[1024];
        sockaddr_storage from{};
        socklen_t fromLen = sizeof from;
        ssize_t n = ::recvfrom(fd, buf, sizeof buf, 0, reinterpret_cast<sockaddr *>(&from), &fromLen);
        if (n < 0)
            return;
        auto b = Beacon::decode(buf, size_t(n));
        if (!b || b->key == id.publicKey())
            continue;
        Address src = Address::from(reinterpret_cast<sockaddr *>(&from), fromLen);
        bool isNew = !peers.count(b->key);
        Peer &p = peer(b->key);
        // The same beacon over IPv4 and IPv6: one more address. An older one
        // (replayed, or delayed behind a newer one) changes nothing; a
        // sender whose clock went back an hour or more is new.
        if (b->seq == p.lastSeq) {
            src.setPort(b->port);
            rememberAddress(p, src);
            continue;
        }
        if (b->seq < p.lastSeq && p.lastSeq - b->seq < 3600 * 1000ull)
            continue;
        p.lastSeq = b->seq;
        p.name = b->name;
        p.arch = b->arch;
        p.version = b->version;
        p.port = b->port;
        src.setPort(b->port);
        rememberAddress(p, src);
        if (b->type == Beacon::Leave) {
            p.left = true;
            p.lastSeen = 0;
            event("info", label(b->key) + " left the network");
            continue;
        }
        bool wasOnline = online(p) && !p.left;
        p.left = false;
        p.lastSeen = nowMs();
        if (isNew)
            event("info", "discovered " + label(b->key) + " at " + src.host());
        else if (!wasOnline)
            event("info", label(b->key) + " is back at " + src.host());
        if (b->type == Beacon::Query) {
            bool paired = store.trust.count(b->key);
            if (store.visibility == Visibility::Open
                || (store.visibility == Visibility::Paired && paired)) {
                sendBeaconTo(src, Beacon::Announce);
                if (store.visibility == Visibility::Open && nowMs() - lastQueryAnswer > 1000) {
                    lastQueryAnswer = nowMs();
                    sendBeacons(Beacon::Announce);
                }
            }
        }
    }
}

} // namespace jkl
