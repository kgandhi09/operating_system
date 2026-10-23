// The control socket, /run/jk-link/socket (lib/ipc.h): requests from
// jk-link and jk-link gui. Anyone may read the status; changing anything
// takes root or group wheel (the administrators, as for sudo), checked from
// the connecting process's credentials.
//
//   status                          this machine, the others, pairings,
//                                   requests, recent events
//   pair        {peer}              start pairing; answers the code
//   confirm     {peer, yes}         the codes match here (or not)
//   accept      {peer}              accept a pairing request
//   reject      {peer}              reject one
//   unpair      {peer}              forget a paired machine (both sides)
//   add         {address}           a machine to reach without multicast
//   remove      {address}
//   visibility  {mode}              open, paired or off
//
// {peer}: a machine ID (or the start of one), a name, or an address.
#include "daemon.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <poll.h>
#include <grp.h>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

namespace jkl {

namespace {

bool isAdmin(uid_t uid, const std::string &adminGroup)
{
    if (uid == 0 || uid == geteuid())
        return true;
    passwd *pw = getpwuid(uid);
    group *wheel = getgrnam(adminGroup.c_str());
    if (!pw || !wheel)
        return false;
    gid_t gids[256];
    int n = 256;
    if (getgrouplist(pw->pw_name, pw->pw_gid, gids, &n) < 0)
        return false;
    for (int i = 0; i < n; ++i)
        if (gids[i] == wheel->gr_gid)
            return true;
    return false;
}

Json fail(const std::string &why)
{
    Json r;
    r.set("ok", false).set("error", why);
    return r;
}

Json okay()
{
    Json r;
    r.set("ok", true);
    return r;
}

std::string hexKey(const PublicKey &k)
{
    return toHex(k.data(), k.size());
}

} // namespace

bool Daemon::openControl()
{
    std::string path = runDir + "/socket";
    sockaddr_un sun{};
    sun.sun_family = AF_UNIX;
    if (path.size() >= sizeof sun.sun_path) {
        event("error", "socket path too long: " + path);
        return false;
    }
    std::memcpy(sun.sun_path, path.c_str(), path.size() + 1);
    ::unlink(path.c_str());
    control = ::socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (control < 0 || ::bind(control, reinterpret_cast<sockaddr *>(&sun), sizeof sun) != 0
        || ::listen(control, 16) != 0) {
        event("error", "cannot open " + path + ": " + std::strerror(errno));
        return false;
    }
    // Everyone may connect; what they may do is checked per request.
    ::chmod(path.c_str(), 0666);
    return true;
}

void Daemon::acceptClient()
{
    for (;;) {
        int fd = ::accept4(control, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (fd < 0)
            return;
        ucred cred{};
        socklen_t len = sizeof cred;
        if (clients.size() >= 32 || getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &cred, &len) != 0) {
            ::close(fd);
            continue;
        }
        clients.push_back({fd, cred.uid, {}, {}});
    }
}

// False when the client is gone.
bool Daemon::serviceClient(Client &c, short revents)
{
    if (revents & POLLIN) {
        char buf[16384];
        for (;;) {
            ssize_t n = ::read(c.fd, buf, sizeof buf);
            if (n > 0) {
                c.in.append(buf, size_t(n));
                if (c.in.size() > 1 << 20)
                    return false;
                continue;
            }
            if (n == 0)
                return false;
            if (errno != EAGAIN && errno != EINTR)
                return false;
            break;
        }
        size_t nl;
        while ((nl = c.in.find('\n')) != std::string::npos) {
            std::string line = c.in.substr(0, nl);
            c.in.erase(0, nl + 1);
            auto req = Json::parse(line);
            Json ans = req && req->isObject() ? command(*req, c.uid) : fail("not a JSON request");
            c.out += ans.dump() + "\n";
        }
    }
    if (revents & (POLLHUP | POLLERR))
        return false;
    while (!c.out.empty()) {
        ssize_t n = ::write(c.fd, c.out.data(), c.out.size());
        if (n > 0) {
            c.out.erase(0, size_t(n));
            continue;
        }
        if (n < 0 && (errno == EAGAIN || errno == EINTR))
            break;
        return false;
    }
    return true;
}

Json Daemon::command(const Json &req, uid_t uid)
{
    const std::string cmd = req["cmd"].toString();
    if (cmd == "status")
        return status();

    static const std::set<std::string> known = {"pair", "confirm", "accept", "reject", "unpair",
                                                "add", "remove", "visibility"};
    if (!known.count(cmd))
        return fail("unknown command: " + cmd);
    if (!isAdmin(uid, adminGroup))
        return fail("only root and administrators (group " + adminGroup + ") may change jk-link");

    if (cmd == "visibility") {
        Visibility v;
        if (!parseVisibility(req["mode"].toString(), v))
            return fail("visibility is open, paired or off");
        if (v == store.visibility)
            return okay();
        if (v == Visibility::Off)
            sendBeacons(Beacon::Leave);
        store.visibility = v;
        store.saveConfig();
        event("info", std::string("visibility ") + visibilityName(v));
        nextBeacon = 0;
        return okay();
    }
    if (cmd == "add" || cmd == "remove") {
        std::string text = req["address"].toString();
        auto a = Address::parse(text, kPort);
        if (!a)
            return fail("not an IP address (and port): " + text);
        std::string s = a->str();
        auto it = std::find(store.manual.begin(), store.manual.end(), s);
        if (cmd == "add") {
            if (it == store.manual.end())
                store.manual.push_back(s);
            nextProbe[s] = 0;
            event("info", "added " + s);
        } else {
            if (it == store.manual.end())
                return fail(s + " was not added");
            store.manual.erase(it);
            event("info", "removed " + s);
        }
        store.saveConfig();
        Json r = okay();
        r.set("address", s);
        return r;
    }

    // The rest name a machine.
    std::string what = req["peer"].toString();
    if (what.empty())
        return fail("which machine?");
    std::vector<PublicKey> match;
    auto add = [&](const PublicKey &k) {
        if (std::find(match.begin(), match.end(), k) == match.end())
            match.push_back(k);
    };
    std::set<PublicKey> all;
    for (auto &[k, p] : peers) all.insert(k);
    for (auto &[k, t] : store.trust) all.insert(k);
    for (auto &[k, r] : requests) all.insert(k);
    for (auto &k : all) {
        std::string mid = machineId(k), hex = hexKey(k);
        auto pit = peers.find(k);
        std::string name = pit != peers.end() ? pit->second.name : "";
        if (name.empty() && store.trust.count(k))
            name = store.trust[k].name;
        if (mid == what || hex == what || name == what)
            add(k);
        else if (what.size() >= 4 && (mid.rfind(what, 0) == 0 || mid.rfind("jk-" + what, 0) == 0))
            add(k);   // the start of an ID: "jk-7f3a", "7f3a"
        else if (pit != peers.end())
            for (auto &s : pit->second.addrs)
                if (s.addr.host() == what || s.addr.str() == what)
                    add(k);
    }
    if (match.empty())
        return fail("no machine called " + what + " (jk-link peers lists them)");
    if (match.size() > 1) {
        std::string ids;
        for (auto &k : match)
            ids += " " + machineId(k);
        return fail(what + " is more than one machine:" + ids + "; name one by its ID");
    }
    const PublicKey key = match.front();

    if (cmd == "pair") {
        if (store.trust.count(key))
            return fail(label(key) + " is already paired");
        auto it = pairings.find(key);
        if (it != pairings.end() && (it->second.state == "connecting" || it->second.state == "waiting"))
            return fail("already pairing with " + label(key));
        Peer &p = peer(key);
        if (p.addrs.empty())
            return fail("no address known for " + label(key));
        Address to = bestAddress(p);
        Pairing pr;
        pr.code = pairingCode(id.publicKey(), key);
        pr.name = p.name;
        pr.state = "connecting";
        pr.started = nowMs();
        pairings[key] = pr;
        startLink(Link::Pair, to, &key);
        pairings[key].link = links.back();
        event("info", "pairing with " + label(key) + ", code " + pr.code);
        Json r = okay();
        r.set("id", machineId(key)).set("name", p.name).set("code", pr.code);
        return r;
    }
    if (cmd == "confirm") {
        auto it = pairings.find(key);
        if (it == pairings.end() || it->second.failed || it->second.state == "paired")
            return fail("not pairing with " + label(key));
        if (!req["yes"].toBool()) {
            it->second.confirmed = -1;
            it->second.state = "cancelled";
            it->second.failed = true;
            if (auto l = it->second.link) {
                Json c;
                c.set("t", "pair-cancel");
                sendTo(*l, c);
                l->stop = true;
            }
            event("warn", "pairing with " + label(key) + " cancelled: the codes differ");
            return okay();
        }
        it->second.confirmed = 1;
        if (it->second.accepted)
            completePairing(key);
        return okay();
    }
    if (cmd == "accept" || cmd == "reject") {
        auto it = requests.find(key);
        if (it == requests.end())
            return fail("no pairing request from " + label(key));
        auto l = it->second.link;
        if (!l || l->stop)
            return fail(label(key) + " went away");
        Json m;
        m.set("t", cmd == "accept" ? "pair-accept" : "pair-reject");
        sendTo(*l, m);
        if (cmd == "accept") {
            addTrust(key, it->second.name);
            if (auto other = meshLink(key); other && other != l)
                other->stop = true;
            l->trusted = true;
            l->purpose = Link::Mesh;
            event("info", "paired with " + label(key));
            event("info", "link to " + label(key) + " up (" + l->addr.host() + ")");
        } else {
            l->stop = true;
            event("info", "rejected pairing from " + label(key));
        }
        requests.erase(it);
        return okay();
    }
    if (cmd == "unpair") {
        if (!store.trust.count(key))
            return fail(label(key) + " is not paired");
        for (auto &l : links)
            if (l->haveKey && l->key == key) {
                if (l->trusted) {
                    Json m;
                    m.set("t", "unpair");
                    sendTo(*l, m);
                }
                l->trusted = false;
                l->stop = true;
            }
        event("info", "unpaired " + label(key));
        removeTrust(key);
        return okay();
    }
    return fail("unknown command: " + cmd);
}

Json Daemon::status()
{
    int64_t now = nowMs();
    Json self;
    self.set("id", id.id())
        .set("key", hexKey(id.publicKey()))
        .set("fingerprint", fingerprint(id.publicKey()))
        .set("name", hostName())
        .set("arch", arch())
        .set("version", osVersion())
        .set("port", port)
        .set("visibility", visibilityName(store.visibility));
    Json addrs = Json(Json::Array{});
    for (auto &a : ownAddresses)
        addrs.push(a);
    self.set("addresses", addrs);
    Json links_ = Json(Json::Array{});
    for (auto &l : links)
        if (l->up && l->trusted) {
            Json o;
            o.set("key", hexKey(l->key)).set("rtt", l->rtt);
            links_.push(o);
        }
    self.set("links", links_);

    std::set<PublicKey> keys;
    for (auto &[k, p] : peers) keys.insert(k);
    for (auto &[k, t] : store.trust) keys.insert(k);
    Json list = Json(Json::Array{});
    for (auto &k : keys) {
        Peer &p = peer(k);
        auto tr = store.trust.find(k);
        bool paired = tr != store.trust.end();
        std::shared_ptr<Link> ml = meshLink(k);
        bool connecting = false;
        for (auto &l : links)
            if (!l->up && !l->stop && ((l->haveKey && l->key == k) || (l->expectKey && l->expected == k)))
                connecting = true;
        Json o;
        o.set("id", machineId(k))
            .set("key", hexKey(k))
            .set("fingerprint", fingerprint(k))
            .set("name", p.name.empty() && paired ? tr->second.name : p.name)
            .set("arch", p.arch)
            .set("version", p.version)
            .set("online", online(p))
            .set("paired", paired)
            .set("link", ml ? "up" : connecting ? "connecting" : "down")
            .set("rtt", ml ? Json(ml->rtt) : Json())
            .set("link_since", ml ? Json(double(now - ml->since) / 1000) : Json())
            .set("last_seen", p.lastSeen ? Json(double(now - p.lastSeen) / 1000) : Json());
        if (paired) {
            o.set("paired_name", tr->second.name).set("paired_since", double(tr->second.since));
            o.set("renamed", !p.name.empty() && !tr->second.name.empty() && p.name != tr->second.name);
        }
        Json pa = Json(Json::Array{});
        for (int v4 = 1; v4 >= 0; --v4)   // IPv4 first: the one people type
            for (auto &s : p.addrs)
                if ((s.addr.family() == AF_INET) == bool(v4))
                    pa.push(s.addr.str());
        o.set("addresses", pa);
        Json nb = Json(Json::Array{});
        if (now - p.neighborsAt < 15000)
            for (auto &n : p.neighbors) {
                Json x;
                x.set("key", hexKey(n.key)).set("id", machineId(n.key)).set("rtt", n.rtt);
                nb.push(x);
            }
        o.set("neighbors", nb);
        list.push(o);
    }

    Json prs = Json(Json::Array{});
    for (auto &[k, pr] : pairings) {
        Json o;
        o.set("id", machineId(k)).set("key", hexKey(k)).set("name", pr.name).set("code", pr.code)
            .set("state", pr.state).set("confirmed", pr.confirmed).set("accepted", pr.accepted)
            .set("age", double(now - pr.started) / 1000);
        prs.push(o);
    }
    Json rqs = Json(Json::Array{});
    for (auto &[k, r] : requests) {
        Json o;
        o.set("id", machineId(k)).set("key", hexKey(k)).set("name", r.name).set("code", r.code)
            .set("age", double(now - r.at) / 1000);
        rqs.push(o);
    }
    Json evs = Json(Json::Array{});
    for (auto &e : events) {
        Json o;
        o.set("time", double(e.time)).set("level", e.level).set("text", e.text);
        evs.push(o);
    }
    Json manual = Json(Json::Array{});
    for (auto &m : store.manual)
        manual.push(m);

    Json r = okay();
    r.set("self", self).set("peers", list).set("pairings", prs).set("requests", rqs)
        .set("events", evs).set("manual", manual);
    return r;
}

} // namespace jkl
