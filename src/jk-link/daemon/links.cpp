// Links: TLS 1.3 on TCP 7710, both sides proving their Ed25519 key.
//
// Each connection opens with a hello (name, arch, version, port) from both
// sides, then carries JSON lines:
//   between paired machines (a mesh link, kept up while both are reachable):
//     ping/pong      every 3 s: liveness and round-trip time
//     links          every 5 s: this machine's own links, for the mesh map
//     unpair         "I no longer trust you" (both sides forget the pairing)
//   for pairing (any machine):
//     pair-request   asks the other machine's user to accept
//     pair-accept, pair-reject, pair-cancel
// Anything else from a machine that isn't paired is ignored. Two paired
// machines that connect to each other at the same time keep the link
// opened by the smaller key.
#include "daemon.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <poll.h>
#include <thread>
#include <unistd.h>

namespace jkl {

namespace {

constexpr size_t kMaxLine = 64 * 1024;
constexpr int64_t kPingEvery = 3000, kLinksEvery = 5000, kDeadAfter = 15000;
constexpr int64_t kPairWindow = 120000;
constexpr size_t kMaxUntrusted = 32;

bool writeAll(SSL *ssl, int fd, const std::string &data)
{
    size_t off = 0;
    while (off < data.size()) {
        size_t n = 0;
        int r = SSL_write_ex(ssl, data.data() + off, data.size() - off, &n);
        if (r == 1) {
            off += n;
            continue;
        }
        int err = SSL_get_error(ssl, r);
        if (err != SSL_ERROR_WANT_WRITE && err != SSL_ERROR_WANT_READ)
            return false;
        pollfd p{fd, short(err == SSL_ERROR_WANT_WRITE ? POLLOUT : POLLIN), 0};
        if (::poll(&p, 1, 5000) <= 0)
            return false;
    }
    return true;
}

PublicKey initiator(const Link &l, const PublicKey &own)
{
    return l.outgoing ? own : l.key;
}

} // namespace

bool Daemon::openListener()
{
    int fd = ::socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    int one = 1, zero = 0;
    if (fd >= 0) {
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
        setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &zero, sizeof zero);
        sockaddr_in6 a{};
        a.sin6_family = AF_INET6;
        a.sin6_port = htons(port);
        if (::bind(fd, reinterpret_cast<sockaddr *>(&a), sizeof a) != 0) {
            ::close(fd);
            fd = -1;
        }
    }
    if (fd < 0) {   // no IPv6 in this kernel
        fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
        if (fd < 0)
            return false;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
        sockaddr_in a{};
        a.sin_family = AF_INET;
        a.sin_port = htons(port);
        if (::bind(fd, reinterpret_cast<sockaddr *>(&a), sizeof a) != 0) {
            event("error", "cannot listen on TCP port " + std::to_string(port) + ": " + std::strerror(errno));
            ::close(fd);
            return false;
        }
    }
    if (::listen(fd, 16) != 0) {
        ::close(fd);
        return false;
    }
    tcp = fd;
    return true;
}

void Daemon::acceptConnection()
{
    for (;;) {
        sockaddr_storage from{};
        socklen_t len = sizeof from;
        int fd = ::accept4(tcp, reinterpret_cast<sockaddr *>(&from), &len, SOCK_CLOEXEC);
        if (fd < 0)
            return;
        size_t untrusted = 0;
        for (auto &l : links)
            untrusted += !l->trusted;
        if (untrusted >= kMaxUntrusted) {
            ::close(fd);
            continue;
        }
        auto link = std::make_shared<Link>();
        link->purpose = Link::Incoming;
        link->addr = Address::from(reinterpret_cast<sockaddr *>(&from), len);
        link->started = nowMs();
        links.push_back(link);
        std::thread(&Daemon::runLink, this, link, fd).detach();
    }
}

void Daemon::startLink(Link::Purpose purpose, const Address &a, const PublicKey *expected)
{
    auto link = std::make_shared<Link>();
    link->purpose = purpose;
    link->outgoing = true;
    link->addr = a;
    link->started = nowMs();
    if (expected) {
        link->expected = *expected;
        link->expectKey = true;
    }
    links.push_back(link);
    std::thread([this, link] {
        int fd = ::socket(link->addr.family(), SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
        bool ok = fd >= 0;
        if (ok && ::connect(fd, reinterpret_cast<const sockaddr *>(&link->addr.sa), link->addr.len) != 0) {
            ok = errno == EINPROGRESS;
            if (ok) {
                pollfd p{fd, POLLOUT, 0};
                int err = 0;
                socklen_t el = sizeof err;
                ok = ::poll(&p, 1, 5000) == 1 && getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &el) == 0
                     && err == 0;
            }
        }
        if (ok) {
            int flags = fcntl(fd, F_GETFL);
            fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
            runLink(link, fd);
            return;
        }
        if (fd >= 0)
            ::close(fd);
        std::lock_guard lock(mu);
        std::erase(links, link);
        if (link->purpose == Link::Pair && link->expectKey) {
            auto it = pairings.find(link->expected);
            if (it != pairings.end() && it->second.link == link && it->second.state == "connecting") {
                it->second.state = "failed";
                it->second.failed = true;
                event("warn", "cannot reach " + label(link->expected) + " at " + link->addr.str());
            }
        }
    }).detach();
}

std::shared_ptr<Link> Daemon::meshLink(const PublicKey &key)
{
    for (auto &l : links)
        if (l->haveKey && l->key == key && l->up && l->trusted && !l->stop)
            return l;
    return nullptr;
}

void Daemon::sendTo(Link &link, const Json &msg)
{
    link.out.push_back(msg.dump() + "\n");
}

void Daemon::runLink(std::shared_ptr<Link> link, int fd)
{
    timeval tv{5, 0};
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof tv);
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);

    SSL *ssl = SSL_new(link->outgoing ? tlsClient : tlsServer);
    bool ok = ssl && SSL_set_fd(ssl, fd) == 1
              && (link->outgoing ? SSL_connect(ssl) : SSL_accept(ssl)) == 1;
    PublicKey key{};
    ok = ok && peerKey(ssl, key);
    {
        std::lock_guard lock(mu);
        if (!ok) {
            // TLS failed: not a jk-linkd, or not one that will talk to us.
        } else if (key == id.publicKey()) {
            ok = false;   // our own address (an address added by hand)
        } else if (link->expectKey && key != link->expected) {
            ok = false;
            event("warn", link->addr.str() + " answered with another key (" + machineId(key)
                              + ") than " + label(link->expected) + "'s");
        } else {
            link->key = key;
            link->haveKey = true;
            link->lastRx = nowMs();
            Json hello;
            hello.set("t", "hello").set("name", hostName()).set("arch", arch())
                .set("version", osVersion()).set("port", port);
            sendTo(*link, hello);
        }
    }

    std::string in;
    if (ok) {
        fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
        while (!link->stop && !quit) {
            pollfd p{fd, POLLIN, 0};
            // Data TLS has already decrypted doesn't wake poll.
            if (SSL_pending(ssl) == 0)
                ::poll(&p, 1, 200);
            bool closed = false, overflow = false;
            for (;;) {
                char buf[16384];
                size_t n = 0;
                int r = SSL_read_ex(ssl, buf, sizeof buf, &n);
                if (r == 1) {
                    in.append(buf, n);
                    if (in.size() > 4 * kMaxLine)
                        overflow = closed = true;
                    continue;
                }
                int err = SSL_get_error(ssl, r);
                if (err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE)
                    closed = true;
                break;
            }
            std::vector<std::string> out;
            {
                std::lock_guard lock(mu);
                size_t nl;
                // (Lines that came just before the other side closed count.)
                while (!overflow && (nl = in.find('\n')) != std::string::npos) {
                    std::string line = in.substr(0, nl);
                    in.erase(0, nl + 1);
                    if (line.size() > kMaxLine) {
                        overflow = closed = true;
                        break;
                    }
                    link->lastRx = nowMs();
                    if (auto msg = Json::parse(line); msg && msg->isObject())
                        handleMessage(*link, *msg);
                }
                int64_t now = nowMs();
                if (link->up && link->trusted) {
                    if (auto it = peers.find(link->key); it != peers.end())
                        it->second.lastSeen = std::max(it->second.lastSeen, link->lastRx);
                    if (now - link->lastPing >= kPingEvery) {
                        link->lastPing = now;
                        Json ping;
                        ping.set("t", "ping").set("ts", double(nowUs()));
                        sendTo(*link, ping);
                    }
                    if (now - link->lastLinks >= kLinksEvery) {
                        link->lastLinks = now;
                        Json msg, list;
                        msg.set("t", "links");
                        for (auto &l : links)
                            if (l->up && l->trusted && l.get() != link.get()) {
                                Json n;
                                n.set("key", toHex(l->key.data(), l->key.size())).set("rtt", l->rtt);
                                list.push(n);
                            }
                        msg.set("links", list.isNull() ? Json(Json::Array{}) : list);
                        sendTo(*link, msg);
                    }
                    if (now - link->lastRx > kDeadAfter)
                        closed = true;
                } else if (link->up && link->purpose == Link::Probe) {
                    link->stop = true;   // knows who is there now
                } else if (now - link->started > kPairWindow + 10000) {
                    closed = true;       // a connection nothing came of
                }
                out.assign(link->out.begin(), link->out.end());
                link->out.clear();
            }
            for (auto &s : out)
                if (!writeAll(ssl, fd, s)) {
                    closed = true;
                    break;
                }
            if (closed)
                break;
        }
        // Lines queued at the end (an unpair, a reject) still go out.
        std::vector<std::string> rest;
        {
            std::lock_guard lock(mu);
            rest.assign(link->out.begin(), link->out.end());
            link->out.clear();
        }
        for (auto &s : rest)
            writeAll(ssl, fd, s);
        SSL_shutdown(ssl);
    }
    SSL_free(ssl);
    ERR_clear_error();
    ::close(fd);

    std::lock_guard lock(mu);
    std::erase(links, link);
    if (link->up && link->trusted && !quit)
        event("info", "link to " + label(link->key) + " down");
    if (link->haveKey) {
        if (auto it = pairings.find(link->key); it != pairings.end() && it->second.link == link
            && (it->second.state == "connecting" || it->second.state == "waiting")) {
            it->second.state = "failed";
            it->second.failed = true;
            event("warn", "pairing with " + label(link->key) + " failed: the connection closed");
        }
        if (auto it = requests.find(link->key); it != requests.end() && it->second.link == link)
            requests.erase(it);
    }
}

void Daemon::handleMessage(Link &link, const Json &msg)
{
    const std::string t = msg["t"].toString();
    const PublicKey &key = link.key;
    bool trusted = store.trust.count(key);
    Peer &p = peer(key);
    int64_t now = nowMs();

    if (t == "hello") {
        p.name = msg["name"].toString().substr(0, 63);
        p.arch = msg["arch"].toString().substr(0, 15);
        p.version = msg["version"].toString().substr(0, 31);
        double port = msg["port"].toNumber(kPort);
        if (port >= 1 && port <= 65535)
            p.port = uint16_t(port);
        Address a = link.addr;
        a.setPort(p.port);
        rememberAddress(p, a);
        p.lastSeen = now;
        p.left = false;
        link.up = true;
        link.since = now;
        if (trusted && link.purpose != Link::Probe) {
            // One link per pair of machines: the one the smaller key opened.
            if (auto other = meshLink(key)) {
                PublicKey mine = initiator(link, id.publicKey()), theirs = initiator(*other, id.publicKey());
                if (mine < theirs)
                    other->stop = true;
                else {
                    link.stop = true;
                    return;
                }
            }
            link.trusted = true;
            event("info", "link to " + label(key) + " up (" + a.host() + ")");
            auto &t = store.trust[key];
            if (!t.name.empty() && t.name != p.name)
                event("warn", "paired machine " + t.name + " now calls itself " + p.name);
        } else if (trusted && link.purpose == Link::Probe && !meshLink(key)) {
            p.nextConnect = 0;   // an address added by hand: link to it now
        }
        if (link.purpose == Link::Pair) {
            auto it = pairings.find(key);
            if (it != pairings.end() && it->second.state == "connecting") {
                it->second.state = "waiting";
                Json req;
                req.set("t", "pair-request");
                sendTo(link, req);
            }
        }
        return;
    }
    if (!link.up)
        return;

    if (t == "ping" && link.trusted) {
        Json pong;
        pong.set("t", "pong").set("ts", msg["ts"]);
        sendTo(link, pong);
    } else if (t == "pong" && link.trusted) {
        double ts = msg["ts"].toNumber(-1), us = double(nowUs());
        if (ts > 0 && ts <= us)
            link.rtt = (us - ts) / 1000;
    } else if (t == "links" && link.trusted) {
        p.neighbors.clear();
        for (auto &n : msg["links"].array()) {
            Neighbor nb;
            if (fromHex(n["key"].toString(), nb.key)) {
                nb.rtt = n["rtt"].toNumber(-1);
                p.neighbors.push_back(nb);
            }
            if (p.neighbors.size() >= 256)
                break;
        }
        p.neighborsAt = now;
    } else if (t == "pair-request") {
        if (trusted) {
            // Already paired here (it lost the pairing): say yes again.
            Json acc;
            acc.set("t", "pair-accept");
            sendTo(link, acc);
            return;
        }
        if (requests.count(key) || requests.size() >= 16)
            return;
        std::shared_ptr<Link> self;
        for (auto &l : links)
            if (l.get() == &link)
                self = l;
        requests[key] = {pairingCode(id.publicKey(), key), p.name, now, self};
        event("request", "pairing request from " + label(key) + ", code " + requests[key].code);
    } else if (t == "pair-accept") {
        auto it = pairings.find(key);
        if (it == pairings.end() || it->second.link.get() != &link || it->second.failed)
            return;
        it->second.accepted = true;
        if (it->second.confirmed == 1)
            completePairing(key);
        else
            event("info", label(key) + " accepted the pairing: confirm the code here");
    } else if (t == "pair-reject") {
        auto it = pairings.find(key);
        if (it == pairings.end() || it->second.link.get() != &link)
            return;
        it->second.state = "rejected";
        it->second.failed = true;
        event("warn", label(key) + " rejected the pairing");
        link.stop = true;
    } else if (t == "pair-cancel") {
        requests.erase(key);
        // Accepted here, then the other side found the codes differed.
        if (trusted && unixTime() - store.trust[key].since < 300) {
            removeTrust(key);
            link.trusted = false;
        }
        event("warn", label(key) + " cancelled the pairing");
        link.stop = true;
    } else if (t == "unpair" && link.trusted) {
        removeTrust(key);
        event("warn", label(key) + " unpaired this machine");
        link.trusted = false;
        link.stop = true;
    }
}

void Daemon::completePairing(const PublicKey &key)
{
    auto &pr = pairings[key];
    addTrust(key, pr.name.empty() ? peer(key).name : pr.name);
    pr.state = "paired";
    event("info", "paired with " + label(key));
    if (auto l = pr.link; l && l->up && !l->stop) {
        if (auto other = meshLink(key); other && other != l)
            other->stop = true;
        l->trusted = true;
        l->purpose = Link::Mesh;
        event("info", "link to " + label(key) + " up (" + l->addr.host() + ")");
    }
}

// Every 2 s from the main loop: links to the paired machines that can be
// reached, probes of the addresses added by hand, and expired pairings.
void Daemon::manageLinks()
{
    int64_t now = nowMs();
    auto busy = [&](const PublicKey &k) {
        for (auto &l : links)
            if (!l->stop && ((l->haveKey && l->key == k) || (l->expectKey && l->expected == k)))
                return true;
        return false;
    };
    for (auto &[key, trust] : store.trust) {
        if (busy(key))
            continue;
        Peer &p = peer(key);
        if (now < p.nextConnect)
            continue;
        bool seen = p.lastSeen && now - p.lastSeen < 20000 && !p.left;
        std::vector<Address> to;
        if (!p.addrs.empty())
            to.push_back(bestAddress(p));
        for (auto &s : p.addrs)
            if (!(s.addr == to.front()))
                to.push_back(s.addr);
        if (to.empty())
            if (auto c = store.cache.find(key); c != store.cache.end())
                for (auto &s : c->second)
                    if (auto a = Address::parse(s, kPort))
                        to.push_back(*a);
        if (to.empty())
            continue;
        // Its beacons are coming: try every 5 s, cycling its addresses;
        // otherwise a known address every 30 s (it may have visibility off).
        Address a = to[connectTurn[key]++ % to.size()];
        p.nextConnect = now + (seen ? 5000 : 30000);
        startLink(Link::Mesh, a, &key);
    }
    for (auto &m : store.manual) {
        auto a = Address::parse(m, kPort);
        if (!a || now < nextProbe[m])
            continue;
        bool linked = false;
        for (auto &l : links)
            if (l->addr.sameHost(*a) && !l->stop)
                linked = true;
        nextProbe[m] = now + 15000;
        if (!linked)
            startLink(Link::Probe, *a, nullptr);
    }
    for (auto &[key, pr] : pairings) {
        if ((pr.state == "connecting" || pr.state == "waiting") && now - pr.started > kPairWindow) {
            pr.state = "failed";
            pr.failed = true;
            event("warn", "pairing with " + label(key) + " timed out");
            if (pr.link)
                pr.link->stop = true;
        }
    }
    std::erase_if(pairings, [&](auto &kv) {
        auto &pr = kv.second;
        return pr.state != "connecting" && pr.state != "waiting" && now - pr.started > kPairWindow + 60000;
    });
    std::erase_if(requests, [&](auto &kv) {
        if (now - kv.second.at <= kPairWindow)
            return false;
        if (kv.second.link)
            kv.second.link->stop = true;
        return true;
    });
    // Machines that went away and aren't paired are forgotten after 10 min.
    std::erase_if(peers, [&](auto &kv) {
        return !store.trust.count(kv.first) && !busy(kv.first) && !requests.count(kv.first)
               && !pairings.count(kv.first) && now - kv.second.lastSeen > 600000;
    });
}

} // namespace jkl
