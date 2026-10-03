// jk-vizd: jk-viz's collector. Runs as root (from /etc/init.d/S50jk-vizd):
// only root sees every process's files, sockets, I/O, the packet counts and
// the energy counters. It samples the machine every second while a jk-viz is
// connected, and sends each sample, as one line of JSON, to every jk-viz on
// /run/jk-viz.sock (for root and group wheel, the administrators). With
// nobody connected it does nothing.
//
//   jk-vizd                  serve (in the foreground)
//   jk-vizd --dump           print one sample and exit
//   jk-vizd --interval <ms>  the sampling interval (default 1000)
//
// JK_VIZ_SOCKET overrides the socket's path.
#include "dbusnames.h"
#include "model.h"
#include "util.h"

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <grp.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

using namespace jkv;

namespace {

volatile sig_atomic_t quit = 0;

struct Client {
    int fd;
    std::string out;
};

int listenOn(const std::string &path)
{
    int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
    if (fd < 0)
        return -1;
    sockaddr_un a{};
    a.sun_family = AF_UNIX;
    if (path.size() >= sizeof a.sun_path)
        return -1;
    strcpy(a.sun_path, path.c_str());
    unlink(path.c_str());
    // Created with no access, then opened to root and group wheel.
    mode_t old = umask(0177);
    int rc = bind(fd, (sockaddr *)&a, sizeof a);
    umask(old);
    if (rc < 0 || listen(fd, 16) < 0) {
        close(fd);
        return -1;
    }
    if (geteuid() == 0) {
        group *g = getgrnam("wheel");
        if (g && chown(path.c_str(), 0, g->gr_gid) == 0)
            chmod(path.c_str(), 0660);
    } else {
        chmod(path.c_str(), 0600);
    }
    return fd;
}

int serve(const std::string &path, int intervalMs)
{
    int lfd = listenOn(path);
    if (lfd < 0) {
        fprintf(stderr, "jk-vizd: cannot listen on %s: %s\n", path.c_str(), strerror(errno));
        return 1;
    }
    fprintf(stderr, "jk-vizd: listening on %s\n", path.c_str());
    Collector col;
    std::vector<Client> clients;
    uint64_t next = 0;
    while (!quit) {
        std::vector<pollfd> pfds;
        pfds.push_back({lfd, POLLIN, 0});
        for (auto &c : clients)
            pfds.push_back({c.fd, short(POLLIN | (c.out.empty() ? 0 : POLLOUT)), 0});
        int timeout = -1;
        if (!clients.empty()) {
            uint64_t now = nowMs();
            timeout = next > now ? int(next - now) : 0;
        }
        if (poll(pfds.data(), pfds.size(), timeout) < 0 && errno != EINTR)
            break;
        // Clients: gone ones out, pending output sent.
        for (size_t i = clients.size(); i-- > 0;) {
            short re = pfds[i + 1].revents;
            Client &c = clients[i];
            bool drop = re & (POLLHUP | POLLERR | POLLNVAL);
            if (!drop && (re & POLLIN)) {
                char buf[256];
                ssize_t n = read(c.fd, buf, sizeof buf);
                drop = n == 0 || (n < 0 && errno != EAGAIN && errno != EINTR);
            }
            if (!drop && (re & POLLOUT) && !c.out.empty()) {
                ssize_t n = write(c.fd, c.out.data(), c.out.size());
                if (n > 0)
                    c.out.erase(0, size_t(n));
                else if (n < 0 && errno != EAGAIN && errno != EINTR)
                    drop = true;
            }
            if (drop) {
                close(c.fd);
                clients.erase(clients.begin() + long(i));
            }
        }
        if (pfds[0].revents & POLLIN) {
            int fd = accept4(lfd, nullptr, nullptr, SOCK_CLOEXEC | SOCK_NONBLOCK);
            if (fd >= 0) {
                if (clients.empty()) {
                    // The first one after a pause: new baselines, so the
                    // first sample covers one interval, not the pause.
                    col.startCapture();
                    Snapshot s;
                    col.sample(s);
                    next = nowMs() + uint64_t(intervalMs);
                }
                clients.push_back({fd, {}});
            }
        }
        if (clients.empty()) {
            col.stopCapture();
            continue;
        }
        uint64_t now = nowMs();
        if (now < next)
            continue;
        next = std::max(next + uint64_t(intervalMs), now + uint64_t(intervalMs) / 2);
        Snapshot s;
        if (!col.sample(s))
            continue;
        std::string line = toJson(s);
        for (auto &c : clients) {
            // A client that stopped reading gets no more (and is dropped
            // once its backlog is too large).
            if (c.out.size() > (16u << 20)) {
                shutdown(c.fd, SHUT_RDWR);
                continue;
            }
            c.out += line;
            ssize_t n = write(c.fd, c.out.data(), c.out.size());
            if (n > 0)
                c.out.erase(0, size_t(n));
        }
    }
    for (auto &c : clients)
        close(c.fd);
    close(lfd);
    unlink(path.c_str());
    return 0;
}

} // namespace

int main(int argc, char **argv)
{
    std::string path = getenv("JK_VIZ_SOCKET") ? getenv("JK_VIZ_SOCKET") : "/run/jk-viz.sock";
    int interval = 1000;
    bool dump = false;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--dbus-names" && i + 1 < argc)
            return dbusNamesMain(argv[i + 1]);
        if (a == "--dump")
            dump = true;
        else if (a == "--interval" && i + 1 < argc)
            interval = std::max(200, atoi(argv[++i]));
        else if (a == "--socket" && i + 1 < argc)
            path = argv[++i];
        else {
            fprintf(stderr, "usage: jk-vizd [--dump] [--interval <ms>] [--socket <path>]\n");
            return a == "-h" || a == "--help" ? 0 : 2;
        }
    }
    signal(SIGPIPE, SIG_IGN);
    if (dump) {
        Collector col;
        col.startCapture();
        Snapshot s;
        col.sample(s);
        usleep(useconds_t(interval) * 1000);
        col.sample(s);
        col.stopCapture();
        fputs(toJson(s).c_str(), stdout);
        return 0;
    }
    struct sigaction sa{};
    sa.sa_handler = [](int) { quit = 1; };
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGINT, &sa, nullptr);
    return serve(path, interval);
}
