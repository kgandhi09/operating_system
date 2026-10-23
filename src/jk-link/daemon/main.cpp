// jk-linkd: this machine's identity on the network, how it finds other
// jk_os machines and which of them it trusts. Started by
// /etc/init.d/S45jk-link as root; once its sockets are open it runs as the
// user jk-link.
//
//   jk-linkd [--state-dir DIR] [--run-dir DIR] [--port N] [--name NAME]
//            [--user USER] [--admin-group GROUP]
//
//   --state-dir  the key, the paired machines and the settings
//                (/data/etc/jk-link on an installed system, else
//                /var/lib/jk-link)
//   --run-dir    the control socket (/run/jk-link)
//   --port       TCP port for links (7710); beacons always use UDP 7710
//   --name       the name it announces (default: the host name)
//   --user       who it runs as when started as root (jk-link)
//   --admin-group  who may pair and change settings, besides root (wheel)
#include "daemon.h"

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <grp.h>
#include <netinet/in.h>
#include <poll.h>
#include <pwd.h>
#include <random>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>

namespace jkl {

int64_t nowMs()
{
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return int64_t(ts.tv_sec) * 1000 + ts.tv_nsec / 1000000;
}

int64_t nowUs()
{
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return int64_t(ts.tv_sec) * 1000000 + ts.tv_nsec / 1000;
}

int64_t unixTime()
{
    return int64_t(std::time(nullptr));
}

std::string Daemon::hostName() const
{
    if (!nameOverride.empty())
        return nameOverride;
    char buf[256] = "";
    gethostname(buf, sizeof buf - 1);
    return buf;
}

std::string Daemon::arch() const
{
    utsname u;
    return uname(&u) == 0 ? u.machine : "";
}

std::string Daemon::osVersion() const
{
    static std::string v = [] {
        std::ifstream f("/etc/os-release");
        std::string l, name, ver;
        while (std::getline(f, l)) {
            auto val = [&](const char *k) {
                std::string s = l.substr(std::strlen(k));
                if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
                    s = s.substr(1, s.size() - 2);
                return s;
            };
            if (l.rfind("ID=", 0) == 0) name = val("ID=");
            else if (l.rfind("VERSION_ID=", 0) == 0) ver = val("VERSION_ID=");
        }
        return name.empty() ? ver : ver.empty() ? name : name + " " + ver;
    }();
    return v;
}

Beacon Daemon::ownBeacon(Beacon::Type type)
{
    static uint64_t lastSeq = 0;
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t seq = uint64_t(ts.tv_sec) * 1000 + uint64_t(ts.tv_nsec / 1000000);
    lastSeq = seq = std::max(seq, lastSeq + 1);
    Beacon b;
    b.type = type;
    b.port = port;
    b.seq = seq;
    b.name = hostName();
    b.arch = arch();
    b.version = osVersion();
    return b;
}

void Daemon::event(const std::string &level, const std::string &text)
{
    events.push_back({unixTime(), level, text});
    while (events.size() > 200)
        events.pop_front();
    char when[32];
    time_t t = time(nullptr);
    strftime(when, sizeof when, "%F %T", localtime(&t));
    std::fprintf(stderr, "%s %s: %s\n", when, level.c_str(), text.c_str());
}

std::string Daemon::label(const PublicKey &key) const
{
    std::string name;
    if (auto it = peers.find(key); it != peers.end())
        name = it->second.name;
    if (name.empty())
        if (auto it = store.trust.find(key); it != store.trust.end())
            name = it->second.name;
    return name.empty() ? machineId(key) : name + " (" + machineId(key) + ")";
}

bool Daemon::online(const Peer &p) const
{
    for (auto &l : links)
        if (l->haveKey && l->key == p.key && l->up && l->trusted && !l->stop)
            return true;
    return p.lastSeen && !p.left && nowMs() - p.lastSeen < 20000;
}

Peer &Daemon::peer(const PublicKey &key)
{
    Peer &p = peers[key];
    p.key = key;
    return p;
}

void Daemon::rememberAddress(Peer &p, const Address &a)
{
    int64_t now = nowMs();
    auto it = std::find_if(p.addrs.begin(), p.addrs.end(), [&](auto &s) { return s.addr == a; });
    if (it != p.addrs.end())
        p.addrs.erase(it);
    p.addrs.insert(p.addrs.begin(), {a, now});
    if (p.addrs.size() > 6)
        p.addrs.resize(6);
}

Address Daemon::bestAddress(const Peer &p) const
{
    int64_t now = nowMs();
    for (auto &s : p.addrs)
        if (s.addr.family() == AF_INET && now - s.seen < 30000)
            return s.addr;
    return p.addrs.front().addr;
}

void Daemon::addTrust(const PublicKey &key, const std::string &name)
{
    store.trust[key] = {name, unixTime()};
    if (!store.saveTrust())
        event("error", "cannot save " + store.dir + "/trust");
    saveCache();
}

void Daemon::removeTrust(const PublicKey &key)
{
    store.trust.erase(key);
    store.cache.erase(key);
    if (!store.saveTrust())
        event("error", "cannot save " + store.dir + "/trust");
    store.saveCache();
}

void Daemon::saveCache()
{
    bool changed = false;
    for (auto &[key, t] : store.trust) {
        auto it = peers.find(key);
        if (it == peers.end() || it->second.addrs.empty())
            continue;
        std::vector<std::string> a;
        for (auto &s : it->second.addrs)
            if (a.size() < 4)
                a.push_back(s.addr.str());
        if (store.cache[key] != a) {
            store.cache[key] = a;
            changed = true;
        }
    }
    if (changed)
        store.saveCache();
}

} // namespace jkl

using namespace jkl;

namespace {

volatile sig_atomic_t gotSignal = 0;

void usage()
{
    std::fprintf(stderr, "usage: jk-linkd [--state-dir DIR] [--run-dir DIR] [--port N] [--name NAME] [--user USER] [--admin-group GROUP]\n");
    std::exit(2);
}

bool isMountpoint(const char *path)
{
    struct stat a, b;
    return stat(path, &a) == 0 && stat("/", &b) == 0 && a.st_dev != b.st_dev;
}

bool makeDir(const std::string &path, mode_t mode)
{
    std::string p;
    for (size_t i = 1; i <= path.size(); ++i)
        if (i == path.size() || path[i] == '/') {
            p = path.substr(0, i);
            if (::mkdir(p.c_str(), i == path.size() ? mode : 0755) != 0 && errno != EEXIST)
                return false;
        }
    return true;
}

} // namespace

int main(int argc, char **argv)
{
    Daemon d;
    std::string user = "jk-link";
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto next = [&]() -> std::string {
            if (i + 1 >= argc)
                usage();
            return argv[++i];
        };
        if (a == "--state-dir") d.stateDir = next();
        else if (a == "--run-dir") d.runDir = next();
        else if (a == "--port") d.port = uint16_t(std::atoi(next().c_str()));
        else if (a == "--name") d.nameOverride = next();
        else if (a == "--user") user = next();
        else if (a == "--admin-group") d.adminGroup = next();
        else usage();
    }
    if (d.stateDir.empty())
        d.stateDir = isMountpoint("/data") ? "/data/etc/jk-link" : "/var/lib/jk-link";
    if (d.runDir.empty())
        d.runDir = "/run/jk-link";
    if (!d.port)
        usage();

    std::signal(SIGPIPE, SIG_IGN);
    std::signal(SIGTERM, [](int) { gotSignal = 1; });
    std::signal(SIGINT, [](int) { gotSignal = 1; });

    umask(077);
    if (!makeDir(d.stateDir, 0700) || !makeDir(d.runDir, 0755)) {
        std::fprintf(stderr, "jk-linkd: cannot make %s or %s: %s\n", d.stateDir.c_str(), d.runDir.c_str(),
                     std::strerror(errno));
        return 1;
    }
    umask(022);
    d.store.dir = d.stateDir;
    d.store.load();
    if (!d.id.loadOrCreate(d.stateDir)) {
        std::fprintf(stderr, "jk-linkd: %s\n", d.id.error().c_str());
        return 1;
    }
    d.tlsServer = d.id.tlsContext(true);
    d.tlsClient = d.id.tlsContext(false);
    if (!d.tlsServer || !d.tlsClient) {
        std::fprintf(stderr, "jk-linkd: cannot set up TLS\n");
        return 1;
    }

    std::unique_lock lock(d.mu);
    d.event("info", std::string(d.id.created() ? "new identity " : "identity ") + d.id.id() + " ("
                        + fingerprint(d.id.publicKey()) + ") in " + d.stateDir);
    if (!d.openListener() || !d.openControl())
        return 1;
    d.openDiscovery();

    // From here on as the user jk-link: the state and run directories
    // become its own.
    if (geteuid() == 0) {
        passwd *pw = getpwnam(user.c_str());
        if (!pw) {
            d.event("warn", "no user " + user + ": running as root");
        } else {
            for (auto f : {"", "/identity.key", "/trust", "/link.conf", "/cache"})
                if (::chown((d.stateDir + f).c_str(), pw->pw_uid, pw->pw_gid) != 0 && *f == 0)
                    d.event("warn", "cannot hand " + d.stateDir + " to " + user);
            if (::chown(d.runDir.c_str(), pw->pw_uid, pw->pw_gid) != 0
                || ::chown((d.runDir + "/socket").c_str(), pw->pw_uid, pw->pw_gid) != 0)
                d.event("warn", "cannot hand " + d.runDir + " to " + user);
            if (initgroups(pw->pw_name, pw->pw_gid) != 0 || setgid(pw->pw_gid) != 0
                || setuid(pw->pw_uid) != 0 || setuid(0) == 0) {
                std::fprintf(stderr, "jk-linkd: cannot become %s\n", user.c_str());
                return 1;
            }
        }
    }
    d.event("info", std::string("listening on TCP ") + std::to_string(d.port) + ", visibility "
                        + visibilityName(d.store.visibility));
    if (d.store.visibility != Visibility::Off)
        d.sendBeacons(Beacon::Query);

    std::mt19937 rng(std::random_device{}());
    int64_t nextManage = 0, nextCache = nowMs() + 60000;
    while (!gotSignal) {
        std::vector<pollfd> fds;
        fds.push_back({d.udp4, POLLIN, 0});
        fds.push_back({d.udp6, POLLIN, 0});
        fds.push_back({d.tcp, POLLIN, 0});
        fds.push_back({d.control, POLLIN, 0});
        for (auto &c : d.clients)
            fds.push_back({c.fd, short(POLLIN | (c.out.empty() ? 0 : POLLOUT)), 0});
        lock.unlock();
        ::poll(fds.data(), fds.size(), 250);
        lock.lock();

        if (fds[0].revents & POLLIN) d.receiveBeacon(d.udp4);
        if (fds[1].revents & POLLIN) d.receiveBeacon(d.udp6);
        if (fds[2].revents & POLLIN) d.acceptConnection();
        std::vector<Daemon::Client> keep;
        for (size_t i = 0; i < d.clients.size(); ++i) {
            if (d.serviceClient(d.clients[i], fds[4 + i].revents))
                keep.push_back(d.clients[i]);
            else
                ::close(d.clients[i].fd);
        }
        d.clients = keep;
        if (fds[3].revents & POLLIN) d.acceptClient();

        int64_t now = nowMs();
        if (now >= d.nextScan) {
            d.nextScan = now + 10000;
            d.scanInterfaces();
        }
        if (now >= d.nextBeacon) {
            d.nextBeacon = now + 5000 + std::uniform_int_distribution<int>(0, 1000)(rng);
            d.sendBeacons(Beacon::Announce);
        }
        if (now >= nextManage) {
            nextManage = now + 1000;
            d.manageLinks();
        }
        if (now >= nextCache) {
            nextCache = now + 60000;
            d.saveCache();
        }
    }

    d.event("info", "stopping");
    if (d.store.visibility != Visibility::Off)
        d.sendBeacons(Beacon::Leave);
    d.saveCache();
    d.quit = true;
    for (auto &l : d.links)
        l->stop = true;
    lock.unlock();
    for (int i = 0; i < 20; ++i) {
        {
            std::lock_guard g(d.mu);
            if (d.links.empty())
                break;
        }
        usleep(50000);
    }
    ::unlink((d.runDir + "/socket").c_str());
    return 0;
}
