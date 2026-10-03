// jk-vizd: what one snapshot holds, and the collector that takes them.
#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace jkv {

// The layers, top (the hardware) to bottom (the applications).
enum Layer { Silicon = 0, Firmware = 1, Kernel = 2, Services = 3, Sessions = 4, Apps = 5 };

// What a node consumes over the last interval. For hardware nodes (hw), the
// device's own load instead: its busy share, bytes per second, watts.
struct Res {
    double cpu = 0;     // share of all CPUs, 0..1 (hardware: the core's busy share)
    uint64_t mem = 0;   // bytes (processes: PSS)
    double gpu = 0;     // share of all GPUs, 0..1 (hardware: the GPU's busy share)
    uint64_t vram = 0;  // bytes
    double io = 0;      // disk bytes/s, read + written
    double net = 0;     // network bytes/s, received + sent
    double pwr = 0;     // watts (an estimate for processes)
};

struct Node {
    std::string id, kind, name, parent;
    int layer = 0;
    int pid = -1;
    bool hw = false;
    std::string user, cmd, state, info;
    int threads = 0, fds = 0;
    double irqs = 0;                    // interrupts/s (devices)
    Res r;
    std::vector<std::string> members;   // grouped kernel threads
};

struct Edge {
    std::string a, b, type, label;
    int count = 1;
};

struct Totals {
    int cpus = 0;
    uint64_t memTotal = 0;
    int gpus = 0;
    uint64_t vramTotal = 0;
    double io = 0, net = 0;             // bytes/s, all of it
    double cpuWatts = 0, gpuWatts = 0;
    bool havePower = false, haveNet = false;
    uint64_t netDrops = 0;
};

struct Snapshot {
    uint64_t t = 0;
    double interval = 0;
    std::string host;
    Totals totals;
    std::vector<Node> nodes;
    std::vector<Edge> edges;
};

std::string toJson(const Snapshot &s);

class NetCapture;
class GpuStats;
class DbusNames;

class Collector {
public:
    Collector();
    ~Collector();
    // Takes a sample. The first one only sets the baselines (rates need two).
    bool sample(Snapshot &out);
    // Packet capture for per-process network traffic (only while someone watches).
    void startCapture();
    void stopCapture();

private:
    struct ProcPrev { uint64_t start = 0, ticks = 0, io = 0; };
    std::unordered_map<int, ProcPrev> prevProc_;
    std::vector<uint64_t> prevCpu_;     // per CPU: busy, total (pairs); [0..1] all CPUs
    uint64_t prevTotal_ = 0, prevIdle_ = 0, prevIrq_ = 0;
    std::map<std::string, uint64_t> prevDisk_, prevNet_, prevRapl_;
    std::map<int, uint64_t> prevIrqCount_;
    uint64_t prevT_ = 0;
    bool primed_ = false;
    std::map<std::string, std::string> pciVendors_, pciDevices_;
    std::map<std::string, std::string> users_;
    std::string machineName_;
    std::unique_ptr<NetCapture> cap_;
    std::unique_ptr<GpuStats> gpu_;
    std::unique_ptr<DbusNames> dbus_;
    uint64_t lastUsersLoad_ = 0;

    void loadPciIds();
    std::string pciName(const std::string &vendor, const std::string &device);
    std::string userName(unsigned uid);
};

} // namespace jkv
