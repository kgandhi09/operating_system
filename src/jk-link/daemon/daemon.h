// jk-linkd's state, shared by its parts:
//   discovery.cpp  the beacons (UDP 7710, multicast and unicast)
//   links.cpp      TLS connections on TCP 7710: links between paired
//                  machines, pairing, probes of addresses added by hand
//   control.cpp    the local control socket (jk-link, jk-link gui)
//   main.cpp       start-up, privileges, the main loop
//
// The main loop owns the sockets; each TLS connection runs in a thread of
// its own. Everything below is guarded by Daemon::mu.
#pragma once

#include "../lib/beacon.h"
#include "../lib/identity.h"
#include "../lib/json.h"
#include "../lib/net.h"
#include "../lib/store.h"

#include <atomic>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

namespace jkl {

int64_t nowMs();     // monotonic
int64_t nowUs();     // monotonic, for round-trip times
int64_t unixTime();  // wall clock, seconds

struct SeenAddress {
    Address addr;      // with the peer's TCP port
    int64_t seen = 0;  // nowMs
};

struct Neighbor {
    PublicKey key;
    double rtt = -1;
};

// A machine this one knows of: seen by its beacons, reached at an address
// added by hand, or paired.
struct Peer {
    PublicKey key{};
    std::string name, arch, version;
    uint16_t port = kPort;
    std::vector<SeenAddress> addrs;   // most recent first
    int64_t lastSeen = 0;             // nowMs of its last beacon or link traffic
    uint64_t lastSeq = 0;
    bool left = false;                // said goodbye (Leave) since
    int64_t nextConnect = 0;          // nowMs: when to try a link again
    std::vector<Neighbor> neighbors;  // its own links (paired peers tell)
    int64_t neighborsAt = 0;
};

// One TLS connection. Its thread owns the socket and the TLS state; others
// queue lines in `out` and set `stop`.
struct Link {
    enum Purpose { Mesh, Pair, Probe, Incoming };
    Purpose purpose = Incoming;
    bool outgoing = false;
    Address addr;
    PublicKey expected{};   // outgoing to a known key: it must prove this one
    bool expectKey = false;
    PublicKey key{};        // the key it proved
    bool haveKey = false;
    bool up = false;        // TLS done and hello received
    bool trusted = false;   // a link between paired machines
    std::deque<std::string> out;
    std::atomic<bool> stop{false};
    int64_t started = 0, since = 0, lastRx = 0, lastPing = 0, lastLinks = 0;
    double rtt = -1;
};

struct Pairing {
    std::string code, name;
    int confirmed = 0;      // 1 yes, -1 no: the user here compared the codes
    bool accepted = false;  // the other machine's user accepted
    bool failed = false;
    std::string state;      // connecting, waiting, paired, rejected, failed, cancelled
    int64_t started = 0;
    std::shared_ptr<Link> link;
};

struct Request {
    std::string code, name;
    int64_t at = 0;
    std::shared_ptr<Link> link;
};

struct Event {
    int64_t time;
    std::string level, text;
};

class Daemon {
public:
    std::mutex mu;

    Identity id;
    Store store;
    std::string stateDir, runDir, nameOverride;
    std::string adminGroup = "wheel";   // who may change things, besides root
    uint16_t port = kPort;
    SSL_CTX *tlsServer = nullptr, *tlsClient = nullptr;
    std::atomic<bool> quit{false};

    std::map<PublicKey, Peer> peers;
    std::vector<std::shared_ptr<Link>> links;
    std::map<PublicKey, Pairing> pairings;
    std::map<PublicKey, Request> requests;
    std::deque<Event> events;

    // discovery.cpp
    int udp4 = -1, udp6 = -1;
    std::set<int> joined4, joined6;   // interface indexes in the groups
    std::vector<std::string> ownAddresses;
    int64_t nextBeacon = 0, nextScan = 0, lastQueryAnswer = 0;
    bool openDiscovery();
    void scanInterfaces();
    void sendBeacons(Beacon::Type type);
    void sendBeaconTo(const Address &a, Beacon::Type type);
    void receiveBeacon(int fd);

    // links.cpp
    int tcp = -1;
    bool openListener();
    void acceptConnection();
    void startLink(Link::Purpose purpose, const Address &a, const PublicKey *expected);
    void manageLinks();
    void runLink(std::shared_ptr<Link> link, int fd);
    void handleMessage(Link &link, const Json &msg);
    void sendTo(Link &link, const Json &msg);
    std::shared_ptr<Link> meshLink(const PublicKey &key);
    void completePairing(const PublicKey &key);
    std::map<PublicKey, size_t> connectTurn;     // which address to try next
    std::map<std::string, int64_t> nextProbe;    // addresses added by hand

    // control.cpp
    int control = -1;
    struct Client {
        int fd;
        uid_t uid;
        std::string in, out;
    };
    std::vector<Client> clients;
    bool openControl();
    void acceptClient();
    bool serviceClient(Client &c, short revents);
    Json command(const Json &req, uid_t uid);
    Json status();

    // Helpers (main.cpp)
    std::string hostName() const;
    std::string arch() const;
    std::string osVersion() const;
    Beacon ownBeacon(Beacon::Type type);
    void event(const std::string &level, const std::string &text);
    std::string label(const PublicKey &key) const;   // "jk-tablet (jk-7f3a-91c2)"
    bool online(const Peer &p) const;
    Peer &peer(const PublicKey &key);
    void rememberAddress(Peer &p, const Address &a);
    Address bestAddress(const Peer &p) const;   // IPv4 seen lately first
    void addTrust(const PublicKey &key, const std::string &name);
    void removeTrust(const PublicKey &key);
    void saveCache();
};

} // namespace jkl
