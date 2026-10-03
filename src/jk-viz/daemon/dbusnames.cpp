#include "dbusnames.h"
#include "util.h"

#include <cstdio>
#include <cstring>
#include <dbus/dbus.h>
#include <fcntl.h>
#include <grp.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

namespace jkv {

namespace {
// Runs `jk-vizd --dbus-names <address>` as <uid>, and reads what it prints.
// A separate process: a session bus accepts only its own user, and libdbus
// is better kept out of the daemon (and out of a forked copy of it).
std::string queryBus(const std::string &self, const Bus &bus)
{
    int p[2];
    if (pipe2(p, O_CLOEXEC) < 0)
        return {};
    pid_t pid = fork();
    if (pid < 0) {
        close(p[0]);
        close(p[1]);
        return {};
    }
    if (pid == 0) {
        dup2(p[1], 1);
        int null = open("/dev/null", O_RDWR);
        if (null >= 0)
            dup2(null, 2);
        if (bus.uid != 0) {
            if (setgroups(0, nullptr) < 0 || setgid(bus.uid) < 0 || setuid(bus.uid) < 0)
                _exit(1);
        }
        execl(self.c_str(), "jk-vizd", "--dbus-names", bus.address.c_str(), (char *)nullptr);
        _exit(1);
    }
    close(p[1]);
    std::string out;
    uint64_t end = nowMs() + 3000;
    char buf[4096];
    for (;;) {
        uint64_t now = nowMs();
        if (now >= end) {
            kill(pid, SIGKILL);
            break;
        }
        pollfd pf{p[0], POLLIN, 0};
        if (poll(&pf, 1, int(end - now)) <= 0)
            continue;
        ssize_t n = read(p[0], buf, sizeof buf);
        if (n <= 0)
            break;
        out.append(buf, size_t(n));
    }
    close(p[0]);
    waitpid(pid, nullptr, 0);
    return out;
}
} // namespace

const std::map<int, std::vector<std::string>> &DbusNames::get(const std::set<Bus> &buses)
{
    uint64_t now = nowMs();
    if (now < next_ && buses == lastBuses_)
        return names_;
    next_ = now + 5000;
    lastBuses_ = buses;
    names_.clear();
    for (auto &b : buses) {
        std::string out = queryBus(self_, b);
        for (auto line : split(out, '\n')) {
            size_t sp = line.find(' ');
            if (sp == std::string_view::npos)
                continue;
            int pid = int(toU64(line.substr(0, sp)));
            if (pid > 0)
                names_[pid].emplace_back(line.substr(sp + 1));
        }
    }
    return names_;
}

int dbusNamesMain(const char *address)
{
    alarm(3);
    DBusError err;
    dbus_error_init(&err);
    DBusConnection *c = dbus_connection_open_private(address, &err);
    if (!c || !dbus_bus_register(c, &err))
        return 1;
    auto call = [&](const char *method, const char *arg) -> DBusMessage * {
        DBusMessage *m = dbus_message_new_method_call("org.freedesktop.DBus", "/org/freedesktop/DBus",
                                                      "org.freedesktop.DBus", method);
        if (arg)
            dbus_message_append_args(m, DBUS_TYPE_STRING, &arg, DBUS_TYPE_INVALID);
        DBusError e;
        dbus_error_init(&e);
        DBusMessage *r = dbus_connection_send_with_reply_and_block(c, m, 1000, &e);
        dbus_message_unref(m);
        dbus_error_free(&e);
        return r;
    };
    DBusMessage *r = call("ListNames", nullptr);
    if (!r)
        return 1;
    std::vector<std::string> names;
    DBusMessageIter it, arr;
    if (dbus_message_iter_init(r, &it) && dbus_message_iter_get_arg_type(&it) == DBUS_TYPE_ARRAY) {
        dbus_message_iter_recurse(&it, &arr);
        while (dbus_message_iter_get_arg_type(&arr) == DBUS_TYPE_STRING) {
            const char *s;
            dbus_message_iter_get_basic(&arr, &s);
            if (s[0] != ':' && strcmp(s, "org.freedesktop.DBus") != 0)
                names.emplace_back(s);
            dbus_message_iter_next(&arr);
        }
    }
    dbus_message_unref(r);
    for (auto &n : names) {
        DBusMessage *p = call("GetConnectionUnixProcessID", n.c_str());
        if (!p)
            continue;
        dbus_uint32_t pid = 0;
        if (dbus_message_get_args(p, nullptr, DBUS_TYPE_UINT32, &pid, DBUS_TYPE_INVALID))
            printf("%u %s\n", pid, n.c_str());
        dbus_message_unref(p);
    }
    fflush(stdout);
    dbus_connection_close(c);
    dbus_connection_unref(c);
    return 0;
}

} // namespace jkv
