// jk-vizd: one sample of the whole machine, from the silicon to the
// applications: the hardware (sysfs), firmware (DMI), the kernel and its
// threads, every process, what connects them (Unix sockets, D-Bus, pipes,
// TCP/UDP, devices), and what each of them consumes.
#include "dbusnames.h"
#include "gpu.h"
#include "model.h"
#include "netcap.h"
#include "util.h"

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <functional>
#include <tuple>
#include <cstring>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/sock_diag.h>
#include <linux/unix_diag.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

namespace jkv {

namespace {

constexpr unsigned PF_KTHREAD = 0x00200000;

struct Proc {
    int pid = 0, ppid = 0;
    unsigned uid = 0;
    std::string comm, cmd, state;
    uint64_t start = 0, ticks = 0, io = 0, pss = 0;
    bool kthread = false;
    bool loginLeader = false;  // leads a session on a terminal (a login shell, getty)
    bool leadsToSession = false;   // it, or something under it, is a session program
    int threads = 0, nfds = 0;
    double cpu = 0, ioRate = 0;
    int layer = -1;
    std::vector<uint64_t> sockets;
    std::vector<std::pair<uint64_t, int>> pipes;   // inode, access mode (O_RDONLY, ...)
    std::vector<std::string> devs;                 // /dev paths
};

// A kernel thread's group: "kworker/3:1H-events" -> "kworker", "irq/45-nvme0q1"
// -> "irq", "rcu_preempt" -> "rcu", "jbd2/nvme0n1p2-8" -> "jbd2".
std::string kthreadGroup(const std::string &comm)
{
    std::string g = comm.substr(0, comm.find('/'));
    while (!g.empty() && isdigit((unsigned char)g.back()))
        g.pop_back();
    while (!g.empty() && (g.back() == '_' || g.back() == '-'))
        g.pop_back();
    if (startsWith(g, "rcu"))
        return "rcu";
    if (startsWith(g, "kworker"))
        return "kworker";
    if (g.empty())
        return comm;
    return g;
}

// Processes that make up a login session (between the console and the
// applications): getty and login, the session bus, the compositor, the
// desktop shell and its helpers, the audio server.
const std::set<std::string> kSessionNames = {
    "getty", "agetty", "jk-getty", "login", "sshd-session", "su", "jk-session", "dbus-run-session",
    "dbus-daemon", "dbus-broker", "dbus-broker-launch", "startplasma-wayland", "startplasma-waylandsession",
    "plasma_session", "kwin_wayland", "kwin_wayland_wrapper", "Xwayland", "cage", "seatd", "pipewire",
    "pipewire-pulse", "wireplumber", "plasmashell", "ksmserver", "kded6", "kglobalacceld", "kactivitymanagerd",
    "xdg-desktop-portal", "xdg-desktop-portal-kde", "xdg-document-portal", "xdg-permission-store",
    "at-spi-bus-launcher", "at-spi2-registryd", "polkit-kde-authentication-agent-1", "org_kde_powerdevil",
    "kscreen_backend_launcher", "xembedsniproxy", "gmenudbusmenuproxy", "kaccess", "ksecretd", "kwalletd6",
    "baloo_file", "kiod6", "jk-audio", "gnome-shell", "gnome-session-binary", "gnome-session-service",
    "gdm-wayland-session", "gdm-session-worker", "systemd", "gsd-xsettings", "Xorg", "Xwayland", "sddm-helper",
    "kdeconnectd", "plasma-browser-integration-host", "DiscoverNotifier", "ibus-daemon", "jk-gui", "jk-dev",
};
const std::set<std::string> kShells = {"sh", "bash", "zsh", "fish", "dash", "ash", "busybox", "-bash", "-sh", "-zsh"};

std::string hexIp(std::string_view s, bool &loopback, bool &any)
{
    // /proc/net/tcp's addresses: hex, in the kernel's (little-endian) word order.
    loopback = false;
    any = true;
    for (char c : s)
        if (c != '0')
            any = false;
    if (s.size() == 8) {
        uint32_t a = uint32_t(toU64(s, 16));
        loopback = (a & 0xff) == 127;
    } else if (s.size() == 32) {
        loopback = s == "00000000000000000000000001000000"
            || (s.substr(0, 24) == "0000000000000000FFFF0000" && s.substr(30, 2) == "7F");
    }
    return std::string(s);
}

struct InetSock {
    unsigned proto;     // 6 or 17
    std::string laddr, raddr;
    unsigned lport, rport, state;
    uint64_t inode;
    bool lLoop, rLoop, lAny, rAny;
};

std::vector<InetSock> readInet()
{
    std::vector<InetSock> v;
    for (auto [file, proto] : {std::pair{"tcp", 6u}, {"tcp6", 6u}, {"udp", 17u}, {"udp6", 17u}}) {
        std::string text = readFile(std::string("/proc/net/") + file, 64 << 20);
        bool firstLine = true;
        for (auto line : split(text, '\n')) {
            if (firstLine) {
                firstLine = false;
                continue;
            }
            auto f = split(line, ' ');
            if (f.size() < 10)
                continue;
            InetSock s;
            s.proto = proto;
            auto l = split(f[1], ':'), r = split(f[2], ':');
            if (l.size() != 2 || r.size() != 2)
                continue;
            s.laddr = hexIp(l[0], s.lLoop, s.lAny);
            s.raddr = hexIp(r[0], s.rLoop, s.rAny);
            s.lport = unsigned(toU64(l[1], 16));
            s.rport = unsigned(toU64(r[1], 16));
            s.state = unsigned(toU64(f[3], 16));
            s.inode = toU64(f[9]);
            v.push_back(std::move(s));
        }
    }
    return v;
}

struct UnixSock { uint64_t peer = 0; std::string name; };

// Every Unix socket, with its peer and its name, through sock_diag (what `ss -x` shows).
std::unordered_map<uint64_t, UnixSock> readUnix()
{
    std::unordered_map<uint64_t, UnixSock> m;
    int fd = socket(AF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_SOCK_DIAG);
    if (fd < 0)
        return m;
    struct {
        nlmsghdr nlh;
        unix_diag_req r;
    } req{};
    req.nlh.nlmsg_len = sizeof req;
    req.nlh.nlmsg_type = SOCK_DIAG_BY_FAMILY;
    req.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    req.r.sdiag_family = AF_UNIX;
    req.r.udiag_states = ~0u;
    req.r.udiag_show = UDIAG_SHOW_PEER | UDIAG_SHOW_NAME;
    if (send(fd, &req, sizeof req, 0) < 0) {
        close(fd);
        return m;
    }
    alignas(nlmsghdr) char buf[32768];
    bool done = false;
    while (!done) {
        ssize_t n = recv(fd, buf, sizeof buf, 0);
        if (n <= 0)
            break;
        for (auto *h = reinterpret_cast<nlmsghdr *>(buf); NLMSG_OK(h, unsigned(n)); h = NLMSG_NEXT(h, n)) {
            if (h->nlmsg_type == NLMSG_DONE || h->nlmsg_type == NLMSG_ERROR) {
                done = true;
                break;
            }
            auto *u = static_cast<unix_diag_msg *>(NLMSG_DATA(h));
            UnixSock s;
            int len = int(h->nlmsg_len - NLMSG_LENGTH(sizeof *u));
            for (auto *a = reinterpret_cast<rtattr *>(u + 1); RTA_OK(a, len); a = RTA_NEXT(a, len)) {
                if (a->rta_type == UNIX_DIAG_PEER) {
                    s.peer = *static_cast<uint32_t *>(RTA_DATA(a));
                } else if (a->rta_type == UNIX_DIAG_NAME) {
                    const char *p = static_cast<const char *>(RTA_DATA(a));
                    size_t l = RTA_PAYLOAD(a);
                    if (l && p[0] == '\0')
                        s.name = "@" + std::string(p + 1, strnlen(p + 1, l - 1));
                    else
                        s.name = std::string(p, strnlen(p, l));
                }
            }
            m[u->udiag_ino] = std::move(s);
        }
    }
    close(fd);
    return m;
}

std::string fmtShare(double v)
{
    char b[32];
    snprintf(b, sizeof b, "%.1f%%", v * 100);
    return b;
}

} // namespace

// ---------------------------------------------------------------- setup
Collector::Collector()
{
    loadPciIds();
    std::string vendor = trim(readLine("/sys/class/dmi/id/sys_vendor"));
    std::string product = trim(readLine("/sys/class/dmi/id/product_name"));
    machineName_ = trim(vendor + " " + product);
    if (machineName_.empty())
        machineName_ = "This machine";
    gpu_ = std::make_unique<GpuStats>();
    std::string self = readLink("/proc/self/exe");
    dbus_ = std::make_unique<DbusNames>(self.empty() ? "/usr/sbin/jk-vizd" : self);
}

Collector::~Collector() = default;

void Collector::startCapture()
{
    if (!cap_)
        cap_ = std::make_unique<NetCapture>();
    cap_->start();
}

void Collector::stopCapture()
{
    if (cap_)
        cap_->stop();
}

void Collector::loadPciIds()
{
    // pci.ids: "vvvv  Vendor" lines, then "\tdddd  Device" lines for it.
    std::string text;
    for (const char *p : {"/usr/share/hwdata/pci.ids", "/usr/share/misc/pci.ids", "/usr/share/pci.ids"}) {
        text = readFile(p, 16 << 20);
        if (!text.empty())
            break;
    }
    std::string vendor;
    for (auto line : split(text, '\n')) {
        if (line.empty() || line[0] == '#')
            continue;
        if (line[0] == 'C' && line.size() > 1 && line[1] == ' ')
            break;      // the device classes: the end of the vendor list
        if (line[0] != '\t' && line.size() > 6) {
            vendor = std::string(line.substr(0, 4));
            pciVendors_[vendor] = trim(line.substr(4));
        } else if (line.size() > 7 && line[0] == '\t' && line[1] != '\t') {
            pciDevices_[vendor + std::string(line.substr(1, 4))] = trim(line.substr(5));
        }
    }
}

std::string Collector::pciName(const std::string &vendor, const std::string &device)
{
    auto d = pciDevices_.find(vendor + device);
    if (d != pciDevices_.end())
        return d->second;
    auto v = pciVendors_.find(vendor);
    return (v != pciVendors_.end() ? v->second : vendor) + " device " + device;
}

std::string Collector::userName(unsigned uid)
{
    uint64_t now = nowMs();
    if (now - lastUsersLoad_ > 60000 || users_.empty()) {
        lastUsersLoad_ = now;
        users_.clear();
        for (auto line : split(readFile("/etc/passwd"), '\n')) {
            auto f = split(line, ':');
            if (f.size() > 2)
                users_[std::string(f[2])] = std::string(f[0]);
        }
    }
    auto it = users_.find(std::to_string(uid));
    return it != users_.end() ? it->second : std::to_string(uid);
}

// ---------------------------------------------------------------- sample
bool Collector::sample(Snapshot &snap)
{
    uint64_t t = nowMs();
    double dt = primed_ ? (t - prevT_) / 1000.0 : 0;
    if (primed_ && dt < 0.05)
        return false;
    snap = Snapshot{};
    snap.t = wallMs();
    snap.interval = dt;
    snap.host = trim(readLine("/proc/sys/kernel/hostname"));
    Totals &tot = snap.totals;
    std::vector<Node> &nodes = snap.nodes;
    std::vector<Edge> &edges = snap.edges;
    std::unordered_map<std::string, size_t> index;
    auto add = [&](Node n) -> Node & {
        index[n.id] = nodes.size();
        nodes.push_back(std::move(n));
        return nodes.back();
    };
    auto node = [&](const std::string &id) -> Node * {
        auto it = index.find(id);
        return it == index.end() ? nullptr : &nodes[it->second];
    };

    // ------------------------------------------------ CPU time
    // /proc/stat: user nice system idle iowait irq softirq steal (guest is in user).
    std::string stat = readFile("/proc/stat");
    uint64_t total = 0, idle = 0, irq = 0;
    std::vector<std::pair<uint64_t, uint64_t>> cpuNow;     // per CPU: busy, total
    std::vector<int> cpuIds;
    for (auto line : split(stat, '\n')) {
        if (!startsWith(line, "cpu"))
            continue;
        auto f = split(line, ' ');
        if (f.size() < 9)
            continue;
        uint64_t v[8];
        for (int i = 0; i < 8; i++)
            v[i] = toU64(f[size_t(i) + 1]);
        uint64_t sum = v[0] + v[1] + v[2] + v[3] + v[4] + v[5] + v[6] + v[7];
        uint64_t idl = v[3] + v[4];
        if (f[0] == "cpu") {
            total = sum;
            idle = idl;
            irq = v[5] + v[6];
        } else {
            cpuIds.push_back(int(toU64(f[0].substr(3))));
            cpuNow.emplace_back(sum - idl, sum);
        }
    }
    uint64_t dTotal = total - prevTotal_;
    double cpuDen = (primed_ && dTotal) ? double(dTotal) : 0;
    tot.cpus = int(cpuIds.size());

    // ------------------------------------------------ memory
    std::string meminfo = readFile("/proc/meminfo");
    auto mi = [&](const char *k) { return fieldValue(meminfo, k) * 1024; };
    tot.memTotal = mi("MemTotal");

    // ------------------------------------------------ hardware (layer 0)
    Node machine;
    machine.id = "machine";
    machine.kind = "machine";
    machine.layer = Silicon;
    machine.name = machineName_;
    machine.hw = true;
    {
        std::string board = trim(readLine("/sys/class/dmi/id/board_vendor") + " " + readLine("/sys/class/dmi/id/board_name"));
        machine.info = board.empty() ? "" : "board: " + board;
    }
    add(machine);

    // sysfs path -> node: what devices (processes' device files, disks,
    // network interfaces, USB) hang off.
    std::vector<std::pair<std::string, std::string>> sysPaths;
    auto ownerOf = [&](std::string path) -> std::string {
        std::string best;
        size_t bestLen = 0;
        for (auto &[p, id] : sysPaths)
            if (p.size() > bestLen && startsWith(path, p) && (path.size() == p.size() || path[p.size()] == '/')) {
                best = id;
                bestLen = p.size();
            }
        return best;
    };
    auto realPath = [](const std::string &p) {
        char buf[PATH_MAX];
        return realpath(p.c_str(), buf) ? std::string(buf) : std::string();
    };

    // CPUs: packages and their logical CPUs (cpuinfo for names and microcode).
    std::string cpuinfo = readFile("/proc/cpuinfo");
    std::map<int, int> pkgOf;
    std::map<int, std::string> pkgModel, pkgUcode;
    {
        int cur = -1;
        std::string model, ucode;
        for (auto line : split(cpuinfo, '\n')) {
            size_t c = line.find(':');
            if (c == std::string_view::npos)
                continue;
            std::string k = trim(line.substr(0, c)), v = trim(line.substr(c + 1));
            if (k == "processor")
                cur = int(toU64(v));
            else if (k == "physical id" && cur >= 0)
                pkgOf[cur] = int(toU64(v));
            else if (k == "model name" && cur >= 0)
                pkgModel[pkgOf.count(cur) ? pkgOf[cur] : 0] = v, model = v;
            else if (k == "microcode")
                ucode = v;
        }
        // "physical id" comes after "model name": fill both per package again.
        for (auto &[cpu, pkg] : pkgOf) {
            if (!pkgModel.count(pkg))
                pkgModel[pkg] = model;
            pkgUcode[pkg] = ucode;
        }
        if (pkgModel.empty())
            pkgModel[0] = model.empty() ? "CPU" : model;
        if (pkgUcode.empty())
            pkgUcode[0] = ucode;
    }
    std::map<int, std::pair<double, int>> pkgBusy;
    for (auto &[pkg, model] : pkgModel) {
        Node n;
        n.id = "pkg" + std::to_string(pkg);
        n.kind = "cpu";
        n.layer = Silicon;
        n.parent = "machine";
        n.name = model;
        n.hw = true;
        add(n);
    }
    std::map<int, std::string> coreName;
    for (size_t i = 0; i < cpuIds.size(); i++) {
        int c = cpuIds[i];
        Node n;
        n.id = "cpu" + std::to_string(c);
        n.kind = "core";
        n.layer = Silicon;
        int pkg = pkgOf.count(c) ? pkgOf[c] : pkgModel.begin()->first;
        n.parent = "pkg" + std::to_string(pkg);
        n.name = "CPU " + std::to_string(c);
        std::string core = readLine("/sys/devices/system/cpu/cpu" + std::to_string(c) + "/topology/core_id");
        std::string freq = readLine("/sys/devices/system/cpu/cpu" + std::to_string(c) + "/cpufreq/scaling_cur_freq");
        n.info = "core " + core + (freq.empty() ? "" : ", " + std::to_string(toU64(freq) / 1000) + " MHz");
        n.hw = true;
        if (primed_ && prevCpu_.size() >= 2 * (i + 1)) {
            uint64_t db = cpuNow[i].first - prevCpu_[2 * i], dtot = cpuNow[i].second - prevCpu_[2 * i + 1];
            n.r.cpu = dtot ? double(db) / double(dtot) : 0;
        }
        pkgBusy[pkg].first += n.r.cpu;
        pkgBusy[pkg].second++;
        add(n);
    }
    for (auto &[pkg, b] : pkgBusy)
        if (auto *n = node("pkg" + std::to_string(pkg)))
            n->r.cpu = b.second ? b.first / b.second : 0;

    {
        Node n;
        n.id = "ram";
        n.kind = "ram";
        n.layer = Silicon;
        n.parent = "machine";
        n.name = "Memory";
        char b[64];
        snprintf(b, sizeof b, "%.1f GiB", tot.memTotal / 1073741824.0);
        n.info = b;
        n.hw = true;
        n.r.mem = tot.memTotal - mi("MemAvailable");
        add(n);
    }

    // IRQ counts (for the devices' interrupt rates).
    std::map<int, uint64_t> irqCount;
    for (auto line : split(readFile("/proc/interrupts"), '\n')) {
        auto f = split(line, ' ');
        if (f.size() < 2 || f[0].empty() || !isdigit((unsigned char)f[0][0]))
            continue;
        uint64_t s = 0;
        for (size_t i = 1; i < f.size() && i <= size_t(tot.cpus); i++)
            s += toU64(f[i]);
        irqCount[int(toU64(f[0]))] = s;
    }
    auto irqRate = [&](const std::vector<int> &irqs) {
        double r = 0;
        if (!primed_ || dt <= 0)
            return r;
        for (int q : irqs) {
            auto a = irqCount.find(q), b = prevIrqCount_.find(q);
            if (a != irqCount.end() && b != prevIrqCount_.end() && a->second >= b->second)
                r += double(a->second - b->second) / dt;
        }
        return r;
    };

    // Drivers (layer 2): one node each, with an edge from each device it drives.
    std::map<std::string, double> driverIrqs;
    auto addDriverEdge = [&](const std::string &dev, const std::string &drv) {
        if (drv.empty())
            return;
        if (!node("drv:" + drv)) {
            Node d;
            d.id = "drv:" + drv;
            d.kind = "driver";
            d.layer = Kernel;
            d.parent = "kernel";
            d.name = drv;
            std::string mod = readLink("/sys/bus/pci/drivers/" + drv + "/module");
            if (mod.empty())
                mod = readLink("/sys/bus/usb/drivers/" + drv + "/module");
            d.info = mod.empty() ? "driver (built in)" : "driver (module " + baseName(mod) + ")";
            add(d);
        }
        Edge e;
        e.a = dev;
        e.b = "drv:" + drv;
        e.type = "hw";
        edges.push_back(e);
    };

    // PCI devices (bridges left out: what they connect hangs off the machine).
    int gpuCount = 0;
    std::map<std::string, std::string> gpuNode;     // pci address -> node id
    std::vector<std::string> nvidiaGpus;
    for (auto &addr : listDir("/sys/bus/pci/devices")) {
        std::string dir = "/sys/bus/pci/devices/" + addr;
        uint64_t cls = toU64(readLine(dir + "/class"), 16);
        if ((cls >> 16) == 0x06)
            continue;
        std::string vendor = readLine(dir + "/vendor").substr(2), device = readLine(dir + "/device").substr(2);
        Node n;
        n.id = "pci:" + addr;
        n.layer = Silicon;
        n.parent = "machine";
        n.hw = true;
        n.name = pciName(vendor, device);
        auto vn = pciVendors_.find(vendor);
        n.info = (vn != pciVendors_.end() ? vn->second : vendor) + " · PCI " + addr;
        switch (cls >> 16) {
        case 0x03: n.kind = "gpu"; gpuCount++; gpuNode[addr] = n.id; if (vendor == "10de") nvidiaGpus.push_back(n.id); break;
        case 0x02: n.kind = "nic"; break;
        case 0x01: n.kind = "storage"; break;
        case 0x04: n.kind = "media"; break;
        case 0x0d: n.kind = "wireless"; break;
        case 0x0c: n.kind = ((cls >> 8) & 0xff) == 0x03 ? "usbhost" : "bus"; break;
        default: n.kind = "pci"; break;
        }
        if (n.kind == "gpu") {
            if (readLine(dir + "/boot_vga") == "1")
                n.info += " · boot display";
            uint64_t vt = toU64(readLine(dir + "/mem_info_vram_total"));
            if (vt) {
                tot.vramTotal += vt;
                n.r.vram = toU64(readLine(dir + "/mem_info_vram_used"));
            }
        }
        std::vector<int> irqs;
        for (auto &q : listDir(dir + "/msi_irqs"))
            irqs.push_back(int(toU64(q)));
        if (irqs.empty()) {
            int q = int(toU64(readLine(dir + "/irq")));
            if (q > 0)
                irqs.push_back(q);
        }
        n.irqs = irqRate(irqs);
        std::string drv = baseName(readLink(dir + "/driver"));
        if (drv == "pcieport" || drv == "shpchp")
            drv.clear();
        sysPaths.emplace_back(realPath(dir), n.id);
        add(n);
        addDriverEdge(n.id, drv);
        driverIrqs[drv] += n.irqs;
    }
    tot.gpus = gpuCount;

    // USB devices (not the root hubs: their devices hang off the controller).
    {
        std::vector<std::pair<std::string, std::string>> usb;     // real path, name
        for (auto &name : listDir("/sys/bus/usb/devices"))
            if (name.find(':') == std::string::npos && !startsWith(name, "usb"))
                usb.emplace_back(realPath("/sys/bus/usb/devices/" + name), name);
        std::sort(usb.begin(), usb.end(), [](auto &a, auto &b) { return a.first.size() < b.first.size(); });
        for (auto &[path, name] : usb) {
            Node n;
            n.id = "usb:" + name;
            n.kind = "usb";
            n.layer = Silicon;
            n.hw = true;
            n.parent = ownerOf(path);
            if (n.parent.empty())
                n.parent = "machine";
            std::string prod = trim(readLine(path + "/product")), manu = trim(readLine(path + "/manufacturer"));
            n.name = prod.empty() ? "USB " + readLine(path + "/idVendor") + ":" + readLine(path + "/idProduct") : prod;
            n.info = (manu.empty() ? std::string() : manu + " · ") + "USB " + name;
            if (readLine(path + "/bDeviceClass") == "09")
                n.kind = "usbhub";
            sysPaths.emplace_back(path, n.id);
            add(n);
            std::set<std::string> drvs;
            for (auto &iface : listDir(path))
                if (iface.find(':') != std::string::npos) {
                    std::string d = baseName(readLink(path + "/" + iface + "/driver"));
                    if (!d.empty() && d != "hub")
                        drvs.insert(d);
                }
            for (auto &d : drvs)
                addDriverEdge(n.id, d);
        }
    }

    // Disks (the whole disks with a device behind them; not loop, zram, ...).
    std::map<std::string, std::string> diskNode;
    for (auto &b : listDir("/sys/block")) {
        std::string dir = "/sys/block/" + b;
        if (!exists(dir + "/device") || startsWith(b, "loop") || startsWith(b, "zram") || startsWith(b, "ram"))
            continue;
        Node n;
        n.id = "disk:" + b;
        n.kind = "disk";
        n.layer = Silicon;
        n.hw = true;
        std::string rp = realPath(dir + "/device");
        n.parent = ownerOf(rp);
        if (n.parent.empty())
            n.parent = "machine";
        std::string model = trim(readLine(dir + "/device/model"));
        char sz[48];
        snprintf(sz, sizeof sz, "%.0f GB", toU64(readLine(dir + "/size")) * 512 / 1e9);
        n.name = b;
        n.info = (model.empty() ? std::string() : model + " · ") + sz;
        diskNode[b] = n.id;
        sysPaths.emplace_back(realPath(dir), n.id);
        add(n);
    }
    std::map<std::string, uint64_t> diskNow;
    for (auto line : split(readFile("/proc/diskstats"), '\n')) {
        auto f = split(line, ' ');
        if (f.size() < 10)
            continue;
        std::string name(f[2]);
        if (diskNode.count(name))
            diskNow[name] = (toU64(f[5]) + toU64(f[9])) * 512;
    }
    double diskTotal = 0;
    if (primed_ && dt > 0)
        for (auto &[name, bytes] : diskNow) {
            auto p = prevDisk_.find(name);
            if (p == prevDisk_.end() || bytes < p->second)
                continue;
            double rate = double(bytes - p->second) / dt;
            node(diskNode[name])->r.io = rate;
            diskTotal += rate;
        }

    // Network interfaces (the physical ones; virtual ones carry the same
    // traffic again).
    std::string defaultIf;
    for (auto line : split(readFile("/proc/net/route"), '\n')) {
        auto f = split(line, '\t');
        if (f.size() > 2 && f[1] == "00000000") {
            defaultIf = trim(f[0]);
            break;
        }
    }
    std::map<std::string, std::string> ifNode;
    std::set<int> physIfs;
    for (auto &ifn : listDir("/sys/class/net")) {
        std::string dir = "/sys/class/net/" + ifn;
        if (!exists(dir + "/device"))
            continue;
        Node n;
        n.id = "net:" + ifn;
        n.kind = "netif";
        n.layer = Silicon;
        n.hw = true;
        n.parent = ownerOf(realPath(dir + "/device"));
        if (n.parent.empty())
            n.parent = "machine";
        n.name = ifn;
        std::string oper = readLine(dir + "/operstate");
        n.info = (exists(dir + "/wireless") || exists(dir + "/phy80211") ? "Wi-Fi · " : "Ethernet · ") + oper;
        if (ifn == defaultIf)
            n.info += " · default route";
        physIfs.insert(int(toU64(readLine(dir + "/ifindex"))));
        ifNode[ifn] = n.id;
        add(n);
    }
    if (defaultIf.empty() || !ifNode.count(defaultIf))
        defaultIf = ifNode.empty() ? "" : ifNode.begin()->first;
    std::map<std::string, uint64_t> netNow;
    for (auto line : split(readFile("/proc/net/dev"), '\n')) {
        size_t c = line.find(':');
        if (c == std::string_view::npos)
            continue;
        std::string ifn = trim(line.substr(0, c));
        auto f = split(line.substr(c + 1), ' ');
        if (f.size() >= 9 && ifNode.count(ifn))
            netNow[ifn] = toU64(f[0]) + toU64(f[8]);
    }
    double netTotal = 0;
    if (primed_ && dt > 0)
        for (auto &[ifn, bytes] : netNow) {
            auto p = prevNet_.find(ifn);
            if (p == prevNet_.end() || bytes < p->second)
                continue;
            double rate = double(bytes - p->second) / dt;
            node(ifNode[ifn])->r.net = rate;
            netTotal += rate;
        }
    if (cap_ && cap_->running())
        cap_->setInterfaces(physIfs);

    // Energy (RAPL): the CPU packages' power.
    std::map<std::string, uint64_t> raplNow;
    double cpuWatts = 0;
    bool havePower = false;
    for (auto &z : listDir("/sys/class/powercap")) {
        if (!startsWith(z, "intel-rapl:") || std::count(z.begin(), z.end(), ':') != 1)
            continue;
        std::string dir = "/sys/class/powercap/" + z;
        std::string nm = readLine(dir + "/name");
        if (!startsWith(nm, "package"))
            continue;
        std::string e = readLine(dir + "/energy_uj");
        if (e.empty())
            continue;   // (root only)
        uint64_t uj = toU64(e);
        raplNow[z] = uj;
        havePower = true;
        auto p = prevRapl_.find(z);
        if (primed_ && dt > 0 && p != prevRapl_.end()) {
            uint64_t d = uj >= p->second ? uj - p->second : uj + toU64(readLine(dir + "/max_energy_range_uj")) - p->second;
            double w = double(d) / 1e6 / dt;
            cpuWatts += w;
            int pkg = int(toU64(nm.substr(nm.find('-') + 1)));
            if (auto *n = node("pkg" + std::to_string(pkg)))
                n->r.pwr = w;
        }
    }

    // ------------------------------------------------ firmware (layer 1)
    {
        Node fw;
        fw.id = "fw";
        fw.kind = "firmware";
        fw.layer = Firmware;
        fw.parent = "machine";
        fw.name = exists("/sys/firmware/efi") ? "UEFI firmware" : "BIOS";
        fw.info = trim(readLine("/sys/class/dmi/id/bios_vendor") + " " + readLine("/sys/class/dmi/id/bios_version")
                       + " (" + readLine("/sys/class/dmi/id/bios_date") + ")");
        add(fw);
        for (auto &[pkg, uc] : pkgUcode) {
            if (uc.empty())
                continue;
            Node n;
            n.id = "ucode" + std::to_string(pkg);
            n.kind = "firmware";
            n.layer = Firmware;
            n.parent = "pkg" + std::to_string(pkg);
            n.name = "CPU microcode";
            n.info = "revision " + uc;
            add(n);
        }
    }

    // ------------------------------------------------ kernel (layer 2)
    {
        Node k;
        k.id = "kernel";
        k.kind = "kernel";
        k.layer = Kernel;
        k.parent = "fw";
        k.name = "Linux " + trim(readLine("/proc/sys/kernel/osrelease"));
        k.info = "interrupts and softirqs: the kernel's own CPU time";
        add(k);
        Node m;
        m.kind = "kmem";
        m.layer = Kernel;
        m.parent = "kernel";
        m.id = "kmem:slab";
        m.name = "kernel objects (slab)";
        m.r.mem = mi("Slab");
        add(m);
        m.id = "kmem:pt";
        m.name = "page tables, kernel stacks";
        m.r.mem = mi("PageTables") + mi("SecPageTables") + mi("KernelStack");
        add(m);
        m.id = "kmem:cache";
        m.name = "page cache (unmapped files)";
        uint64_t cache = mi("Cached") + mi("Buffers"), mapped = mi("Mapped");
        m.r.mem = cache > mapped ? cache - mapped : 0;
        m.info = "file contents kept in memory; given back when programs need it";
        add(m);
    }
    // Interrupt rates on the drivers too.
    for (auto &[d, r] : driverIrqs)
        if (auto *n = node("drv:" + d))
            n->irqs = r;

    // ------------------------------------------------ processes
    std::vector<Proc> procs;
    std::unordered_map<int, size_t> byPid;
    std::unordered_map<uint64_t, std::vector<int>> sockOwner;
    std::unordered_map<uint64_t, std::vector<std::pair<int, int>>> pipeOwner;
    std::set<Bus> buses;
    gpu_->begin();
    for (auto &ent : listDir("/proc")) {
        if (!isdigit((unsigned char)ent[0]))
            continue;
        std::string dir = "/proc/" + ent;
        std::string st = readFile(dir + "/stat", 4096);
        size_t rp = st.rfind(')'), lp = st.find('(');
        if (rp == std::string::npos || lp == std::string::npos)
            continue;
        Proc p;
        p.pid = int(toU64(ent));
        p.comm = st.substr(lp + 1, rp - lp - 1);
        auto f = split(std::string_view(st).substr(rp + 2), ' ');
        if (f.size() < 20)
            continue;
        p.state = std::string(f[0]);
        p.ppid = int(toU64(f[1]));
        p.loginLeader = toU64(f[3]) == uint64_t(p.pid) && toU64(f[4]) != 0;
        p.kthread = (toU64(f[6]) & PF_KTHREAD) || p.pid == 2 || p.ppid == 2;
        p.ticks = toU64(f[11]) + toU64(f[12]);
        p.threads = int(toU64(f[17]));
        p.start = toU64(f[19]);
        struct stat sb;
        if (::stat(dir.c_str(), &sb) == 0)
            p.uid = sb.st_uid;
        auto prev = prevProc_.find(p.pid);
        bool samePrev = prev != prevProc_.end() && prev->second.start == p.start;
        // (A process started during the interval: all its time is in it.)
        if (cpuDen > 0)
            p.cpu = double(p.ticks - (samePrev ? std::min(prev->second.ticks, p.ticks) : 0)) / cpuDen;
        if (!p.kthread) {
            std::string cl = readFile(dir + "/cmdline", 2048);
            for (auto &c : cl)
                if (c == '\0')
                    c = ' ';
            p.cmd = trim(cl);
            // comm is cut at 15 characters: the program's full name from argv[0].
            if (p.comm.size() >= 15) {
                std::string arg0 = baseName(std::string(split(p.cmd, ' ').empty() ? "" : split(p.cmd, ' ')[0]));
                if (startsWith(arg0, p.comm))
                    p.comm = arg0;
            }
            p.pss = fieldValue(readFile(dir + "/smaps_rollup", 8192), "Pss") * 1024;
            std::string io = readFile(dir + "/io", 1024);
            uint64_t rb = fieldValue(io, "read_bytes"), wb = fieldValue(io, "write_bytes"), cw = fieldValue(io, "cancelled_write_bytes");
            p.io = rb + (wb > cw ? wb - cw : 0);
            if (primed_ && dt > 0 && samePrev && p.io >= prev->second.io)
                p.ioRate = double(p.io - prev->second.io) / dt;
            // Open files: sockets, pipes, devices.
            std::string fdDir = dir + "/fd/";
            for (auto &fd : listDir(fdDir)) {
                p.nfds++;
                std::string l = readLink(fdDir + fd);
                if (startsWith(l, "socket:[")) {
                    uint64_t ino = toU64(std::string_view(l).substr(8));
                    p.sockets.push_back(ino);
                    sockOwner[ino].push_back(p.pid);
                } else if (startsWith(l, "pipe:[")) {
                    uint64_t ino = toU64(std::string_view(l).substr(6));
                    // fdinfo's "flags:" (octal): which end of the pipe this is.
                    std::string fi = readFile(dir + "/fdinfo/" + fd, 1024);
                    size_t at = fi.find("flags:");
                    int mode = at == std::string::npos ? 2 : int(toU64(trim(std::string_view(fi).substr(at + 6, 24)), 8) & 3);
                    p.pipes.emplace_back(ino, mode);
                    pipeOwner[ino].emplace_back(p.pid, mode);
                } else if (startsWith(l, "/dev/")) {
                    if (startsWith(l, "/dev/dri/"))
                        gpu_->drmFd(p.pid, readFile(dir + "/fdinfo/" + fd, 8192));
                    if (startsWith(l, "/dev/pts") || startsWith(l, "/dev/tty") || startsWith(l, "/dev/shm")
                        || l == "/dev/null" || l == "/dev/zero" || l == "/dev/urandom" || l == "/dev/random"
                        || l == "/dev/ptmx" || l == "/dev/console" || l == "/dev/full" || startsWith(l, "/dev/char"))
                        continue;
                    if (std::find(p.devs.begin(), p.devs.end(), l) == p.devs.end())
                        p.devs.push_back(l);
                }
            }
            if (p.uid >= 1000 || p.comm == "dbus-daemon") {
                std::string env = readFile(dir + "/environ", 1 << 16);
                size_t at = env.find("DBUS_SESSION_BUS_ADDRESS=");
                if (at != std::string::npos && (at == 0 || env[at - 1] == '\0')) {
                    std::string addr = env.substr(at + 25, env.find('\0', at) - at - 25);
                    if (!addr.empty() && p.uid != 0)
                        buses.insert(Bus{addr, p.uid});
                }
            }
        }
        byPid[p.pid] = procs.size();
        procs.push_back(std::move(p));
    }
    // (Only now: an entry for each process, gone ones dropped.)
    std::unordered_map<int, ProcPrev> nextPrev;
    for (auto &p : procs)
        nextPrev[p.pid] = ProcPrev{p.start, p.ticks, p.io};

    // Layers of the processes: the kernel's threads in the kernel; init and
    // the services it starts; the login sessions; the applications. A child
    // is never above its parent.
    auto procName = [](const Proc &p) { return p.comm; };
    // Wrappers that start a session (scripts like jk-gui, dbus-run-session)
    // belong to it: whatever has a session program under it.
    for (auto &p : procs) {
        if (p.kthread || !kSessionNames.count(p.comm) || p.comm == "getty" || p.comm == "agetty")
            continue;
        for (Proc *q = &p; q && !q->leadsToSession; q = byPid.count(q->ppid) ? &procs[byPid[q->ppid]] : nullptr) {
            q->leadsToSession = true;
            if (q->pid <= 1)
                break;
        }
    }
    std::function<int(Proc &)> layerOf = [&](Proc &p) -> int {
        if (p.layer >= 0)
            return p.layer;
        p.layer = Apps;     // (cycles, in theory)
        int l;
        Proc *parent = byPid.count(p.ppid) ? &procs[byPid[p.ppid]] : nullptr;
        int pl = parent ? layerOf(*parent) : Services;
        if (p.kthread)
            l = Kernel;
        else if (p.pid == 1)
            l = Services;
        else {
            // In a session: started from a login (or as a user), or one of
            // the session's own programs.
            bool user = p.uid >= 1000;
            bool sessionProc = kSessionNames.count(procName(p)) > 0 || p.leadsToSession;
            // A login: the session leader on a terminal (getty, login, the
            // login shell), or a shell login or su started.
            bool loginShell = p.loginLeader
                || (kShells.count(procName(p)) && parent && parent->layer == Sessions
                    && (parent->comm == "login" || parent->comm == "su" || startsWith(parent->comm, "sshd")
                        || parent->comm == "jk-getty"));
            if (pl >= Sessions || user)
                l = (sessionProc || loginShell) && pl <= Sessions ? Sessions : Apps;
            else if (loginShell || p.comm == "getty" || p.comm == "agetty" || p.comm == "login" || p.comm == "jk-getty")
                l = Sessions;
            else
                l = Services;
            if (p.uid >= 1000 && l == Services)
                l = Apps;
        }
        p.layer = std::max(l, p.kthread ? Kernel : pl);
        return p.layer;
    };
    for (auto &p : procs)
        layerOf(p);

    // Kernel threads: kthreadd, and its threads in groups.
    std::map<std::string, std::vector<const Proc *>> ktGroups;
    for (auto &p : procs) {
        if (!p.kthread || p.pid == 2)
            continue;
        ktGroups[kthreadGroup(p.comm)].push_back(&p);
    }
    for (auto &p : procs) {
        if (p.kthread && p.pid != 2)
            continue;
        Node n;
        n.id = "p" + std::to_string(p.pid);
        n.kind = p.kthread ? "kthread" : "process";
        n.layer = p.layer;
        n.pid = p.pid;
        n.name = p.comm;
        n.user = userName(p.uid);
        n.cmd = p.cmd;
        n.state = p.state;
        n.threads = p.threads;
        n.fds = p.nfds;
        n.r.cpu = p.cpu;
        n.r.mem = p.pss;
        n.r.io = p.ioRate;
        if (p.pid == 1 || p.pid == 2 || p.ppid == 0)
            n.parent = "kernel";
        else
            n.parent = byPid.count(p.ppid) ? "p" + std::to_string(p.ppid) : "p1";
        add(n);
    }
    for (auto &[g, members] : ktGroups) {
        Node n;
        n.id = "kt:" + g;
        n.kind = "kthreads";
        n.layer = Kernel;
        n.parent = "p2";
        n.name = g;
        n.threads = int(members.size());
        std::vector<const Proc *> m = members;
        std::sort(m.begin(), m.end(), [](auto *a, auto *b) { return a->cpu != b->cpu ? a->cpu > b->cpu : a->comm < b->comm; });
        for (auto *p : m) {
            n.r.cpu += p->cpu;
            std::string s = p->comm + " (" + std::to_string(p->pid) + ")";
            if (p->cpu > 0)
                s += " " + fmtShare(p->cpu);
            n.members.push_back(s);
        }
        n.info = std::to_string(members.size()) + " kernel threads";
        add(n);
    }

    // ------------------------------------------------ GPUs
    std::vector<GpuDev> gpus = gpu_->finish(dt);
    std::vector<std::pair<std::string, std::string>> gpuEdges;
    double gpuWatts = 0, gpuProcSum = 0, gpuBusySum = 0;
    uint64_t vramProcSum = 0, vramUsedSum = 0;
    for (auto &g : gpus) {
        Node *gn = node(gpuNode.count(g.pci) ? gpuNode[g.pci] : "");
        if (gn) {
            gn->r.gpu = g.busy;
            if (g.vramTotal) {
                gn->r.vram = g.vramUsed;
                tot.vramTotal += g.vramTotal;
            }
            if (g.havePower)
                gn->r.pwr = g.watts;
            if (!g.name.empty() && gn->name.find("[") == std::string::npos)
                gn->info += " · " + g.name;
        }
        if (g.havePower) {
            gpuWatts += g.watts;
            havePower = true;
        }
        gpuBusySum += g.busy;
        vramUsedSum += g.vramTotal ? g.vramUsed : 0;
        for (auto &[pid, gp] : g.procs) {
            Node *pn = node("p" + std::to_string(pid));
            if (!pn)
                continue;
            double share = gpuCount ? gp.busy / gpuCount : 0;
            pn->r.gpu += share;
            pn->r.vram += gp.vram;
            pn->r.pwr += g.havePower ? g.watts * gp.busy : 0;
            gpuProcSum += share;
            vramProcSum += gp.vram;
            // An edge to the GPU it uses.
            if (gn)
                gpuEdges.emplace_back(pn->id, gn->id);
        }
    }

    // ------------------------------------------------ network per process
    std::vector<InetSock> inet = readInet();
    std::unordered_map<uint64_t, int> inodePid;
    for (auto &[ino, pids] : sockOwner)
        inodePid[ino] = pids.front();
    double netProcSum = 0;
    uint64_t drops = 0;
    if (cap_ && cap_->running() && primed_ && dt > 0) {
        tot.haveNet = true;
        std::map<uint32_t, int> portPid;
        for (auto &s : inet)
            if (s.inode && inodePid.count(s.inode)) {
                uint32_t key = s.proto << 16 | s.lport;
                if (!portPid.count(key))
                    portPid[key] = inodePid[s.inode];
            }
        for (auto &[key, b] : cap_->take(drops)) {
            auto it = portPid.find(key);
            if (it == portPid.end() || key == 0)
                continue;
            if (Node *pn = node("p" + std::to_string(it->second))) {
                double rate = double(b.rx + b.tx) / dt;
                pn->r.net += rate;
                netProcSum += rate;
            }
        }
    }
    tot.netDrops = drops;

    // ------------------------------------------------ totals and the rest
    // Everything adds up to 100% of each resource: what processes use, the
    // kernel, what's idle or free, and what couldn't be attributed.
    Node idleN;
    idleN.id = "idle";
    idleN.kind = "idle";
    idleN.layer = Kernel;
    idleN.parent = "kernel";
    idleN.name = "idle / free";
    idleN.info = "CPU idle time, free memory, idle GPUs, free video memory";
    Node other;
    other.id = "other";
    other.kind = "other";
    other.layer = Kernel;
    other.parent = "kernel";
    other.name = "unattributed";
    other.info = "processes that ended during the interval, kernel writeback, unmatched network traffic, "
                 "GPU and memory use no process accounts for";
    Node *kern = node("kernel");
    double procCpu = 0, procIo = 0;
    uint64_t procMem = 0;
    for (auto &n : nodes) {
        if (n.kind == "process" || n.kind == "kthread" || n.kind == "kthreads") {
            procCpu += n.r.cpu;
            procIo += n.r.io;
            procMem += n.r.mem;
        }
    }
    if (cpuDen > 0) {
        idleN.r.cpu = double(idle - prevIdle_) / cpuDen;
        kern->r.cpu = double(irq - prevIrq_) / cpuDen;
        other.r.cpu = std::max(0.0, 1.0 - idleN.r.cpu - kern->r.cpu - procCpu);
    }
    uint64_t kmem = node("kmem:slab")->r.mem + node("kmem:pt")->r.mem + node("kmem:cache")->r.mem;
    idleN.r.mem = mi("MemFree");
    other.r.mem = tot.memTotal > procMem + kmem + idleN.r.mem ? tot.memTotal - procMem - kmem - idleN.r.mem : 0;
    if (gpuCount) {
        idleN.r.gpu = std::max(0.0, (gpuCount - gpuBusySum) / gpuCount);
        other.r.gpu = std::max(0.0, gpuBusySum / gpuCount - gpuProcSum);
    }
    idleN.r.vram = tot.vramTotal > vramUsedSum ? tot.vramTotal - vramUsedSum : 0;
    other.r.vram = vramUsedSum > vramProcSum ? vramUsedSum - vramProcSum : 0;
    tot.io = std::max(diskTotal, procIo);
    other.r.io = std::max(0.0, diskTotal - procIo);
    tot.net = std::max(netTotal, netProcSum);
    other.r.net = tot.haveNet ? std::max(0.0, netTotal - netProcSum) : netTotal;
    // Power: each CPU package's energy shared out by CPU time (the idle
    // share to "idle"), each GPU's by its load. An estimate, said as much.
    tot.cpuWatts = cpuWatts;
    tot.gpuWatts = gpuWatts;
    tot.havePower = havePower;
    if (cpuWatts > 0) {
        for (auto &n : nodes)
            if (n.kind == "process" || n.kind == "kthread" || n.kind == "kthreads")
                n.r.pwr += cpuWatts * n.r.cpu;
        kern->r.pwr = cpuWatts * kern->r.cpu;
        idleN.r.pwr = cpuWatts * idleN.r.cpu;
        other.r.pwr = cpuWatts * other.r.cpu;
    }
    for (auto &g : gpus)
        if (g.havePower) {
            double procBusy = 0;
            for (auto &[pid, gp] : g.procs)
                if (node("p" + std::to_string(pid)))
                    procBusy += gp.busy;
            idleN.r.pwr += g.watts * std::max(0.0, 1 - g.busy);
            other.r.pwr += g.watts * std::max(0.0, g.busy - procBusy);
        }
    add(idleN);
    add(other);

    // ------------------------------------------------ edges between processes
    std::map<std::string, std::vector<std::string>> busNames;      // node id -> D-Bus names
    {
        std::set<Bus> all = buses;
        for (const char *sys : {"/run/dbus/system_bus_socket", "/var/run/dbus/system_bus_socket"})
            if (exists(sys)) {
                all.insert(Bus{std::string("unix:path=") + sys, 0});
                break;
            }
        for (auto &[pid, names] : dbus_->get(all))
            busNames["p" + std::to_string(pid)] = names;
    }
    auto isBus = [&](int pid) {
        auto it = byPid.find(pid);
        return it != byPid.end() && (procs[it->second].comm == "dbus-daemon" || startsWith(procs[it->second].comm, "dbus-broker"));
    };
    std::map<std::tuple<std::string, std::string, std::string>, Edge> merged;
    auto link = [&](const std::string &a, const std::string &b, const std::string &type, const std::string &label, bool directed) {
        if (a == b)
            return;
        std::string x = a, y = b;
        if (!directed && y < x)
            std::swap(x, y);
        auto &e = merged[{x, y, type}];
        if (e.type.empty()) {
            e.a = x;
            e.b = y;
            e.type = type;
            e.label = label;
            e.count = 0;
        } else if (!label.empty() && e.label.find(label) == std::string::npos && e.label.size() < 120) {
            e.label += e.label.empty() ? label : ", " + label;
        }
        e.count++;
    };
    // Unix sockets: each one's peer, and who holds the two.
    for (auto &[ino, us] : readUnix()) {
        if (!us.peer || ino > us.peer)
            continue;   // each connection once (from its lower inode)
        auto a = sockOwner.find(ino), b = sockOwner.find(us.peer);
        if (a == sockOwner.end() || b == sockOwner.end())
            continue;
        int pa = a->second.front(), pb = b->second.front();
        if (pa == pb)
            continue;
        std::string label = us.name;
        if (isBus(pa) || isBus(pb)) {
            int client = isBus(pa) ? pb : pa;
            auto bn = busNames.find("p" + std::to_string(client));
            std::string names;
            if (bn != busNames.end())
                for (size_t i = 0; i < bn->second.size() && i < 3; i++)
                    names += (i ? ", " : "") + bn->second[i] + (i == 2 && bn->second.size() > 3 ? ", …" : "");
            link("p" + std::to_string(pa), "p" + std::to_string(pb), "dbus", names, false);
        } else {
            link("p" + std::to_string(pa), "p" + std::to_string(pb), "socket", label, false);
        }
    }
    // Pipes: from the processes that write to the ones that read (not the
    // pipes half the system shares, like a make jobserver's).
    for (auto &[ino, holders] : pipeOwner) {
        std::set<int> pids;
        for (auto &h : holders)
            pids.insert(h.first);
        if (pids.size() < 2 || pids.size() > 16)
            continue;
        for (auto &w : holders) {
            if (w.second == 0)
                continue;   // O_RDONLY
            for (auto &r : holders)
                if (r.second != 1 && r.first != w.first)   // not O_WRONLY
                    link("p" + std::to_string(w.first), "p" + std::to_string(r.first), "pipe", "", true);
        }
    }
    // TCP/UDP: between local processes (loopback), and to the network.
    {
        std::map<std::tuple<std::string, unsigned, std::string, unsigned>, uint64_t> byEnds;
        for (auto &s : inet)
            if (s.proto == 6 && s.state == 1)
                byEnds[{s.laddr, s.lport, s.raddr, s.rport}] = s.inode;
        std::map<int, std::map<std::string, int>> netUse;   // pid -> "tcp", "listen :22", ...
        for (auto &s : inet) {
            if (!s.inode || !inodePid.count(s.inode))
                continue;
            int pid = inodePid[s.inode];
            if (s.proto == 6 && s.state == 1) {
                auto peer = byEnds.find({s.raddr, s.rport, s.laddr, s.lport});
                if (peer != byEnds.end() && inodePid.count(peer->second)) {
                    if (s.inode < peer->second)
                        link("p" + std::to_string(pid), "p" + std::to_string(inodePid[peer->second]), "tcp",
                             ":" + std::to_string(std::min(s.lport, s.rport)), false);
                } else if (!s.rLoop) {
                    netUse[pid]["tcp"]++;
                }
            } else if (s.proto == 6 && s.state == 10 && !s.lLoop) {
                netUse[pid]["listen tcp :" + std::to_string(s.lport)]++;
            } else if (s.proto == 17 && !s.lLoop) {
                netUse[pid]["udp :" + std::to_string(s.lport)]++;
            }
        }
        if (!defaultIf.empty())
            for (auto &[pid, uses] : netUse) {
                std::string label;
                for (auto &[what, n] : uses) {
                    if (!label.empty())
                        label += ", ";
                    label += what == "tcp" ? std::to_string(n) + " tcp" : what;
                }
                link("p" + std::to_string(pid), ifNode[defaultIf], "net", label, false);
            }
    }
    // Devices: the hardware behind the device files processes have open.
    for (auto &[a, b] : gpuEdges)
        link(a, b, "dev", "GPU", true);
    {
        std::map<std::string, std::string> devCache;
        for (auto &p : procs) {
            for (auto &d : p.devs) {
                std::string owner;
                auto c = devCache.find(d);
                if (c != devCache.end()) {
                    owner = c->second;
                } else {
                    struct stat sb;
                    if (startsWith(d, "/dev/nvidia")) {
                        owner = nvidiaGpus.empty() ? "" : nvidiaGpus.front();
                    } else if (::stat(d.c_str(), &sb) == 0 && (S_ISCHR(sb.st_mode) || S_ISBLK(sb.st_mode))) {
                        std::string sys = std::string(S_ISCHR(sb.st_mode) ? "/sys/dev/char/" : "/sys/dev/block/")
                            + std::to_string(major(sb.st_rdev)) + ":" + std::to_string(minor(sb.st_rdev));
                        owner = ownerOf(realPath(sys));
                    }
                    devCache[d] = owner;
                }
                if (!owner.empty() && owner != "machine")
                    link("p" + std::to_string(p.pid), owner, "dev", baseName(d), true);
            }
        }
    }
    for (auto &[k, e] : merged)
        edges.push_back(std::move(e));
    // D-Bus names on the processes too.
    for (auto &[id, names] : busNames)
        if (Node *n = node(id)) {
            std::string s;
            for (auto &nm : names)
                s += (s.empty() ? "" : ", ") + nm;
            n->info = "D-Bus: " + s;
        }

    // ------------------------------------------------ keep for next time
    prevProc_ = std::move(nextPrev);
    prevCpu_.clear();
    for (auto &c : cpuNow) {
        prevCpu_.push_back(c.first);
        prevCpu_.push_back(c.second);
    }
    prevTotal_ = total;
    prevIdle_ = idle;
    prevIrq_ = irq;
    prevDisk_ = diskNow;
    prevNet_ = netNow;
    prevRapl_ = raplNow;
    prevIrqCount_ = irqCount;
    prevT_ = t;
    bool ready = primed_;
    primed_ = true;
    return ready;
}

} // namespace jkv
