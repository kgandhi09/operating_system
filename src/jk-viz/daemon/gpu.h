// jk-vizd: GPU load per process. NVIDIA through NVML (libnvidia-ml, loaded
// when present); the other GPUs (Intel, AMD, ...) through the DRM clients'
// usage the kernel reports in /proc/<pid>/fdinfo.
#pragma once
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace jkv {

struct GpuProc {
    double busy = 0;    // share of this GPU, 0..1
    uint64_t vram = 0;
};

struct GpuDev {
    std::string pci;    // "0000:01:00.0"
    std::string name;
    double busy = 0;    // 0..1
    uint64_t vramTotal = 0, vramUsed = 0;
    double watts = 0;
    bool havePower = false;
    std::map<int, GpuProc> procs;
};

class GpuStats {
public:
    GpuStats();
    ~GpuStats();
    // One sample: begin(), drmFd() for each DRM file of each process, then finish().
    void begin();
    void drmFd(int pid, const std::string &fdinfo);
    std::vector<GpuDev> finish(double interval);

private:
    struct Nvml;
    Nvml *nv_ = nullptr;
    uint64_t nvRetry_ = 0;
    struct Client { int pid; std::string pdev; std::map<std::string, uint64_t> engines; uint64_t vram; };
    std::map<std::string, Client> clients_, prevClients_;   // by "<pdev>/<client id>"
    void nvmlSample(std::vector<GpuDev> &out, double interval);
};

} // namespace jkv
