// jk-link: the jk_os machines this one sees and trusts (talks to jk-linkd).
//
//   jk-link                      this machine, the others, pending pairings
//   jk-link id                   this machine's identity
//   jk-link peers [--json]       the machines it knows
//   jk-link pair <machine>       pair with one: both show a code to compare
//   jk-link requests             pairing requests from other machines
//   jk-link accept <machine>     accept one (after comparing the code)
//   jk-link reject <machine>
//   jk-link unpair <machine>     forget a paired machine (it forgets this one)
//   jk-link add <ip[:port]>      a machine to reach without multicast
//   jk-link remove <ip[:port]>
//   jk-link visibility [open|paired|off]
//   jk-link events               what happened lately
//   jk-link gui                  the mesh window
//
// <machine>: its name, its ID (jk-7f3a-91c2, or its start) or its address.
#include "../lib/ipc.h"
#include "../lib/json.h"

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

using namespace jkl;

namespace {

bool tty = isatty(1);
const char *bold() { return tty ? "\033[1m" : ""; }
const char *dim() { return tty ? "\033[2m" : ""; }
const char *green() { return tty ? "\033[32m" : ""; }
const char *yellow() { return tty ? "\033[33m" : ""; }
const char *red() { return tty ? "\033[31m" : ""; }
const char *cyan() { return tty ? "\033[36m" : ""; }
const char *reset() { return tty ? "\033[0m" : ""; }

[[noreturn]] void die(const std::string &msg)
{
    std::fprintf(stderr, "jk-link: %s\n", msg.c_str());
    std::exit(1);
}

ControlClient client;

Json call(Json req)
{
    std::string err, ans = client.request(req.dump(), &err);
    if (ans.empty())
        die(err);
    auto r = Json::parse(ans);
    if (!r)
        die("jk-linkd sent something that isn't JSON");
    return *r;
}

Json callOk(Json req)
{
    Json r = call(std::move(req));
    if (!r["ok"].toBool())
        die(r["error"].toString("failed"));
    return r;
}

Json cmd(const char *name)
{
    Json j;
    j.set("cmd", name);
    return j;
}

std::string pad(std::string s, size_t w)
{
    // Display width: count UTF-8 lead bytes only.
    size_t n = 0;
    for (unsigned char c : s)
        n += (c & 0xC0) != 0x80;
    if (n < w)
        s += std::string(w - n, ' ');
    return s;
}

std::string ago(const Json &secs)
{
    if (!secs.isNumber())
        return "never";
    double s = secs.toNumber();
    char b[32];
    if (s < 60) std::snprintf(b, sizeof b, "%.0fs ago", s);
    else if (s < 3600) std::snprintf(b, sizeof b, "%.0fm ago", s / 60);
    else if (s < 86400) std::snprintf(b, sizeof b, "%.0fh ago", s / 3600);
    else std::snprintf(b, sizeof b, "%.0fd ago", s / 86400);
    return b;
}

std::string rttText(const Json &rtt)
{
    double r = rtt.toNumber(-1);
    if (r < 0)
        return "-";
    char b[32];
    std::snprintf(b, sizeof b, r < 1 ? "%.2f ms" : r < 10 ? "%.1f ms" : "%.0f ms", r);
    return b;
}

void printSelf(const Json &self)
{
    std::printf("%sThis machine%s  %s%s%s  %s%s%s\n", bold(), reset(), cyan(), self["name"].toString().c_str(),
                reset(), dim(), self["id"].toString().c_str(), reset());
    std::printf("  %-12s %s\n", "fingerprint", self["fingerprint"].toString().c_str());
    std::printf("  %-12s %s, %s\n", "system", self["version"].toString().c_str(), self["arch"].toString().c_str());
    std::string addrs;
    for (auto &a : self["addresses"].array())
        addrs += (addrs.empty() ? "" : ", ") + a.toString();
    std::printf("  %-12s %s (TCP %d)\n", "addresses", addrs.empty() ? "none" : addrs.c_str(),
                int(self["port"].toNumber()));
    std::printf("  %-12s %s\n", "visibility", self["visibility"].toString().c_str());
}

void printPeers(const Json &st)
{
    const auto &peers = st["peers"].array();
    if (peers.empty()) {
        std::printf("No other jk_os machines seen yet.\n");
        return;
    }
    std::printf("%s%s%s%s%s%s%s%s\n", bold(), pad("NAME", 18).c_str(), pad("ID", 14).c_str(),
                pad("ADDRESS", 26).c_str(), pad("STATE", 26).c_str(), pad("LINK", 12).c_str(), "RTT", reset());
    for (auto &p : peers) {
        std::string state, color = dim();
        bool online = p["online"].toBool(), paired = p["paired"].toBool();
        if (paired) {
            state = online ? "paired, online" : "paired, offline";
            color = online ? green() : yellow();
        } else {
            state = online ? "seen" : "gone";
            color = online ? "" : dim();
        }
        if (!online)
            state += ", " + ago(p["last_seen"]);
        auto addrs = p["addresses"].array();
        std::string addr = addrs.empty() ? "-" : addrs.front().toString();
        std::string link = p["link"].toString();
        std::string name = p["name"].toString("?");
        std::printf("%s%s%s%s%s%s%s%s\n", color.c_str(), pad(name, 18).c_str(), pad(p["id"].toString(), 14).c_str(),
                    pad(addr.size() > 25 ? addr.substr(0, 24) + "…" : addr, 26).c_str(), pad(state, 26).c_str(), pad(link, 12).c_str(),
                    rttText(p["rtt"]).c_str(), reset());
        if (p["renamed"].toBool())
            std::printf("  %s! paired as %s, now calls itself %s%s\n", yellow(),
                        p["paired_name"].toString().c_str(), name.c_str(), reset());
    }
}

void printPending(const Json &st)
{
    for (auto &r : st["requests"].array())
        std::printf("%sPairing request%s from %s (%s), code %s%s%s: jk-link accept %s\n", yellow(), reset(),
                    r["name"].toString().c_str(), r["id"].toString().c_str(), bold(), r["code"].toString().c_str(),
                    reset(), r["id"].toString().c_str());
    for (auto &p : st["pairings"].array())
        std::printf("Pairing with %s (%s), code %s: %s\n", p["name"].toString().c_str(),
                    p["id"].toString().c_str(), p["code"].toString().c_str(), p["state"].toString().c_str());
}

bool ask(const std::string &q)
{
    std::printf("%s [y/N] ", q.c_str());
    std::fflush(stdout);
    std::string a;
    if (!std::getline(std::cin, a))
        return false;
    return a == "y" || a == "Y" || a == "yes";
}

int pair(const std::string &who)
{
    Json req = cmd("pair");
    req.set("peer", who);
    Json r = callOk(req);
    std::string id = r["id"].toString(), name = r["name"].toString(id), code = r["code"].toString();
    std::printf("Pairing with %s%s%s (%s).\n\n    code  %s%s%s\n\n", bold(), name.c_str(), reset(), id.c_str(),
                bold(), code.c_str(), reset());
    std::printf("%s shows a pairing request with a code: accept it there\n"
                "(jk-link accept, or jk-link gui) if it is the same.\n\n", name.c_str());
    bool yes = ask("Does " + name + " show " + code + "?");
    Json c = cmd("confirm");
    c.set("peer", id).set("yes", yes);
    callOk(c);
    if (!yes) {
        std::printf("Cancelled: the codes differ, so something between the two machines may be\n"
                    "impersonating one of them. Nothing was paired.\n");
        return 1;
    }
    std::printf("Waiting for %s to accept", name.c_str());
    std::fflush(stdout);
    for (int i = 0; i < 1300; ++i) {
        Json st = callOk(cmd("status"));
        for (auto &p : st["pairings"].array()) {
            if (p["id"].toString() != id)
                continue;
            std::string s = p["state"].toString();
            if (s == "paired") {
                std::printf("\n%sPaired with %s.%s\n", green(), name.c_str(), reset());
                return 0;
            }
            if (s != "waiting" && s != "connecting") {
                std::printf("\n%sNot paired: %s.%s\n", red(), s.c_str(), reset());
                return 1;
            }
        }
        if (i % 10 == 9) {
            std::printf(".");
            std::fflush(stdout);
        }
        usleep(100000);
    }
    std::printf("\nNot paired: no answer.\n");
    return 1;
}

int accept(const std::string &who, bool yes)
{
    Json st = callOk(cmd("status"));
    Json found;
    for (auto &r : st["requests"].array())
        if (r["id"].toString() == who || r["name"].toString() == who || r["key"].toString() == who
            || r["id"].toString().rfind(who, 0) == 0 || r["id"].toString().rfind("jk-" + who, 0) == 0)
            found = r;
    if (found.isNull())
        die("no pairing request from " + who + " (jk-link requests lists them)");
    std::string name = found["name"].toString(), id = found["id"].toString(), code = found["code"].toString();
    if (yes) {
        std::printf("Pairing request from %s%s%s (%s).\n\n    code  %s%s%s\n\n", bold(), name.c_str(), reset(),
                    id.c_str(), bold(), code.c_str(), reset());
        if (!ask("Does " + name + " show the same code?")) {
            Json r = cmd("reject");
            r.set("peer", id);
            callOk(r);
            std::printf("Rejected.\n");
            return 1;
        }
    }
    Json r = cmd(yes ? "accept" : "reject");
    r.set("peer", id);
    callOk(r);
    std::printf(yes ? "%sPaired with %s.%s\n" : "%sRejected %s.%s\n", yes ? green() : "", name.c_str(), reset());
    return 0;
}

void usage()
{
    std::printf(
        "usage: jk-link [command]\n"
        "  (none)                    this machine, the others, pending pairings\n"
        "  id                        this machine's identity\n"
        "  peers [--json]            the machines it knows\n"
        "  pair <machine>            pair with one (both show a code to compare)\n"
        "  requests                  pairing requests from other machines\n"
        "  accept <machine>          accept one, after comparing its code\n"
        "  reject <machine>\n"
        "  unpair <machine>          forget a paired machine (it forgets this one too)\n"
        "  add <ip[:port]>           a machine to reach where multicast doesn't\n"
        "  remove <ip[:port]>\n"
        "  visibility [open|paired|off]\n"
        "  events                    what happened lately\n"
        "  gui                       the mesh window\n"
        "<machine>: its name, its ID (jk-7f3a-91c2, or its start) or its address.\n"
        "Pairing and changes need root or an administrator (group wheel).\n");
}

} // namespace

int main(int argc, char **argv)
{
    std::vector<std::string> args;
    bool json = false;
    for (int i = 1; i < argc; ++i)
        (std::strcmp(argv[i], "--json") == 0 ? json = true : (args.push_back(argv[i]), false));
    std::string c = args.empty() ? "status" : args[0];
    if (c == "-h" || c == "--help" || c == "help") {
        usage();
        return 0;
    }
    if (c == "gui") {
        const char *gui = std::getenv("JK_LINK_GUI");
        std::string path = gui && *gui ? gui : "/usr/lib/jk_os/jk-link-gui";
        std::vector<char *> av = {const_cast<char *>(path.c_str())};
        for (size_t i = 1; i < args.size(); ++i)
            av.push_back(const_cast<char *>(args[i].c_str()));
        av.push_back(nullptr);
        execv(path.c_str(), av.data());
        die("cannot start " + path + ": " + std::strerror(errno));
    }
    std::string err;
    if (!client.connect(&err))
        die(err);
    auto arg = [&](size_t i, const char *what) {
        if (args.size() <= i)
            die(std::string("which ") + what + "? (jk-link help)");
        return args[i];
    };

    if (c == "status") {
        Json st = callOk(cmd("status"));
        if (json) {
            std::printf("%s\n", st.dump().c_str());
            return 0;
        }
        printSelf(st["self"]);
        std::printf("\n");
        printPeers(st);
        if (!st["requests"].array().empty() || !st["pairings"].array().empty()) {
            std::printf("\n");
            printPending(st);
        }
    } else if (c == "id") {
        Json st = callOk(cmd("status"));
        if (json)
            std::printf("%s\n", st["self"].dump().c_str());
        else
            printSelf(st["self"]);
    } else if (c == "peers") {
        Json st = callOk(cmd("status"));
        if (json)
            std::printf("%s\n", st["peers"].dump().c_str());
        else
            printPeers(st);
    } else if (c == "requests") {
        Json st = callOk(cmd("status"));
        if (st["requests"].array().empty() && st["pairings"].array().empty())
            std::printf("No pairing requests.\n");
        printPending(st);
    } else if (c == "pair") {
        return pair(arg(1, "machine"));
    } else if (c == "accept" || c == "reject") {
        return accept(arg(1, "machine"), c == "accept");
    } else if (c == "unpair") {
        Json r = cmd("unpair");
        r.set("peer", arg(1, "machine"));
        callOk(r);
        std::printf("Unpaired %s.\n", args[1].c_str());
    } else if (c == "add" || c == "remove") {
        Json r = cmd(c.c_str());
        r.set("address", arg(1, "address"));
        Json a = callOk(r);
        std::printf("%s %s.\n", c == "add" ? "Added" : "Removed", a["address"].toString().c_str());
    } else if (c == "visibility") {
        if (args.size() < 2) {
            std::printf("%s\n", callOk(cmd("status"))["self"]["visibility"].toString().c_str());
            return 0;
        }
        Json r = cmd("visibility");
        r.set("mode", args[1]);
        callOk(r);
        std::printf("Visibility %s.\n", args[1].c_str());
    } else if (c == "events") {
        Json st = callOk(cmd("status"));
        for (auto &e : st["events"].array()) {
            time_t t = time_t(e["time"].toNumber());
            char when[32];
            strftime(when, sizeof when, "%F %T", localtime(&t));
            std::string lvl = e["level"].toString();
            const char *col = lvl == "warn" || lvl == "error" ? yellow() : lvl == "request" ? cyan() : "";
            std::printf("%s%s%s  %s%s%s\n", dim(), when, reset(), col, e["text"].toString().c_str(), reset());
        }
    } else {
        usage();
        return 2;
    }
    return 0;
}
