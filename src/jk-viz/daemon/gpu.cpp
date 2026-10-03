#include "gpu.h"
#include "util.h"

#include <algorithm>
#include <cctype>
#include <dlfcn.h>

namespace jkv {

// ---------------------------------------------------------------- NVML
// The few NVML types used, as nvml.h declares them (the library is loaded at
// run time: jk_os builds without NVIDIA's headers, and runs without NVIDIA).
namespace {
using nvmlDevice_t = void *;
struct nvmlUtilization_t { unsigned int gpu, memory; };
struct nvmlMemory_t { unsigned long long total, free, used; };
struct nvmlProcessInfo_t { unsigned int pid; unsigned long long usedGpuMemory; unsigned int gpuInstanceId, computeInstanceId; };
struct nvmlProcessUtilizationSample_t { unsigned int pid; unsigned long long timeStamp; unsigned int smUtil, memUtil, encUtil, decUtil; };
struct nvmlPciInfo_t {
    char busIdLegacy[16];
    unsigned int domain, bus, device, pciDeviceId, pciSubSystemId;
    char busId[32];
};
constexpr int NVML_SUCCESS = 0;
constexpr unsigned long long NVML_VALUE_NOT_AVAILABLE = ~0ULL;
} // namespace

struct GpuStats::Nvml {
    void *lib = nullptr;
    int (*init)() = nullptr;
    int (*shutdown)() = nullptr;
    int (*count)(unsigned int *) = nullptr;
    int (*handle)(unsigned int, nvmlDevice_t *) = nullptr;
    int (*name)(nvmlDevice_t, char *, unsigned int) = nullptr;
    int (*util)(nvmlDevice_t, nvmlUtilization_t *) = nullptr;
    int (*mem)(nvmlDevice_t, nvmlMemory_t *) = nullptr;
    int (*power)(nvmlDevice_t, unsigned int *) = nullptr;
    int (*procUtil)(nvmlDevice_t, nvmlProcessUtilizationSample_t *, unsigned int *, unsigned long long) = nullptr;
    int (*compute)(nvmlDevice_t, unsigned int *, nvmlProcessInfo_t *) = nullptr;
    int (*graphics)(nvmlDevice_t, unsigned int *, nvmlProcessInfo_t *) = nullptr;
    int (*pci)(nvmlDevice_t, nvmlPciInfo_t *) = nullptr;
    std::map<unsigned, unsigned long long> lastSeen;

    bool load()
    {
        lib = dlopen("libnvidia-ml.so.1", RTLD_NOW | RTLD_LOCAL);
        if (!lib)
            return false;
        auto sym = [&](auto &fn, const char *n) { fn = reinterpret_cast<std::remove_reference_t<decltype(fn)>>(dlsym(lib, n)); return fn != nullptr; };
        bool ok = sym(init, "nvmlInit_v2") && sym(shutdown, "nvmlShutdown") && sym(count, "nvmlDeviceGetCount_v2")
            && sym(handle, "nvmlDeviceGetHandleByIndex_v2") && sym(name, "nvmlDeviceGetName")
            && sym(util, "nvmlDeviceGetUtilizationRates") && sym(mem, "nvmlDeviceGetMemoryInfo")
            && sym(pci, "nvmlDeviceGetPciInfo_v3");
        sym(power, "nvmlDeviceGetPowerUsage");
        sym(procUtil, "nvmlDeviceGetProcessUtilization");
        sym(compute, "nvmlDeviceGetComputeRunningProcesses_v3");
        sym(graphics, "nvmlDeviceGetGraphicsRunningProcesses_v3");
        if (!ok || init() != NVML_SUCCESS) {
            dlclose(lib);
            lib = nullptr;
            return false;
        }
        return true;
    }
    ~Nvml()
    {
        if (lib) {
            shutdown();
            dlclose(lib);
        }
    }
};

GpuStats::GpuStats() = default;
GpuStats::~GpuStats() { delete nv_; }

void GpuStats::nvmlSample(std::vector<GpuDev> &out, double interval)
{
    if (!nv_) {
        // The driver may come later (its modules load after jk-vizd starts):
        // try again every 30 s.
        uint64_t now = nowMs();
        if (now < nvRetry_)
            return;
        nvRetry_ = now + 30000;
        nv_ = new Nvml;
        if (!nv_->load()) {
            delete nv_;
            nv_ = nullptr;
            return;
        }
    }
    unsigned int n = 0;
    if (nv_->count(&n) != NVML_SUCCESS)
        return;
    for (unsigned i = 0; i < n; i++) {
        nvmlDevice_t d;
        if (nv_->handle(i, &d) != NVML_SUCCESS)
            continue;
        GpuDev g;
        nvmlPciInfo_t pi{};
        if (nv_->pci(d, &pi) == NVML_SUCCESS) {
            // "00000000:01:00.0" -> sysfs's "0000:01:00.0"
            std::string b = pi.busId;
            for (auto &c : b)
                c = char(tolower((unsigned char)c));
            g.pci = b.size() > 12 ? b.substr(b.size() - 12) : b;
        }
        char nm[96] = {};
        if (nv_->name(d, nm, sizeof nm) == NVML_SUCCESS)
            g.name = nm;
        nvmlUtilization_t u{};
        if (nv_->util(d, &u) == NVML_SUCCESS)
            g.busy = u.gpu / 100.0;
        nvmlMemory_t m{};
        if (nv_->mem(d, &m) == NVML_SUCCESS) {
            g.vramTotal = m.total;
            g.vramUsed = m.used;
        }
        unsigned int mw = 0;
        if (nv_->power && nv_->power(d, &mw) == NVML_SUCCESS) {
            g.watts = mw / 1000.0;
            g.havePower = true;
        }
        // Memory: the processes using the GPU (graphics and compute).
        for (auto fn : {nv_->graphics, nv_->compute}) {
            if (!fn)
                continue;
            std::vector<nvmlProcessInfo_t> infos(512);
            unsigned int cnt = unsigned(infos.size());
            if (fn(d, &cnt, infos.data()) != NVML_SUCCESS)
                continue;
            for (unsigned k = 0; k < cnt && k < infos.size(); k++) {
                auto &p = g.procs[int(infos[k].pid)];
                if (infos[k].usedGpuMemory != NVML_VALUE_NOT_AVAILABLE)
                    p.vram = std::max<uint64_t>(p.vram, infos[k].usedGpuMemory);
            }
        }
        // Load: the per-process samples since the last call (averaged).
        if (nv_->procUtil) {
            unsigned long long since = nv_->lastSeen.count(i) ? nv_->lastSeen[i]
                : (unsigned long long)(wallMs() * 1000 - uint64_t(interval * 1e6));
            unsigned int cnt = 0;
            nv_->procUtil(d, nullptr, &cnt, since);
            if (cnt) {
                std::vector<nvmlProcessUtilizationSample_t> s(cnt + 16);
                cnt = unsigned(s.size());
                if (nv_->procUtil(d, s.data(), &cnt, since) == NVML_SUCCESS) {
                    std::map<int, std::pair<double, int>> acc;
                    for (unsigned k = 0; k < cnt; k++) {
                        auto &a = acc[int(s[k].pid)];
                        a.first += s[k].smUtil;
                        a.second++;
                        nv_->lastSeen[i] = std::max(nv_->lastSeen[i], s[k].timeStamp);
                    }
                    for (auto &[pid, a] : acc)
                        g.procs[pid].busy = std::min(1.0, a.first / a.second / 100.0);
                }
            }
        }
        out.push_back(std::move(g));
    }
}

// ---------------------------------------------------------------- DRM fdinfo
void GpuStats::begin()
{
    clients_.clear();
}

void GpuStats::drmFd(int pid, const std::string &fdinfo)
{
    std::string pdev, id;
    Client c{pid, {}, {}, 0};
    for (auto line : split(fdinfo, '\n')) {
        size_t colon = line.find(':');
        if (colon == std::string_view::npos)
            continue;
        std::string_view k = line.substr(0, colon);
        std::string v = trim(line.substr(colon + 1));
        if (k == "drm-pdev")
            pdev = v;
        else if (k == "drm-client-id")
            id = v;
        else if (startsWith(k, "drm-engine-") && !startsWith(k, "drm-engine-capacity-"))
            c.engines[std::string(k.substr(11))] = toU64(v);
        else if (k == "drm-total-vram" || k == "drm-memory-vram") {
            uint64_t n = toU64(v);
            if (v.find("KiB") != std::string::npos)
                n <<= 10;
            else if (v.find("MiB") != std::string::npos)
                n <<= 20;
            else if (v.find("GiB") != std::string::npos)
                n <<= 30;
            c.vram = std::max(c.vram, n);
        }
    }
    if (pdev.empty() || id.empty() || c.engines.empty())
        return;
    c.pdev = pdev;
    // A client's file can be shared (passed between processes, or dup'ed):
    // counted once, for the first process seen with it.
    clients_.emplace(pdev + "/" + id, std::move(c));
}

std::vector<GpuDev> GpuStats::finish(double interval)
{
    std::vector<GpuDev> out;
    nvmlSample(out, interval);
    std::set<std::string> nvidia;
    for (auto &g : out)
        nvidia.insert(g.pci);
    std::map<std::string, std::map<std::string, double>> engineSum;    // pdev -> engine -> busy
    std::map<std::string, GpuDev> drm;
    for (auto &[key, c] : clients_) {
        if (nvidia.count(c.pdev))
            continue;
        auto &g = drm[c.pdev];
        g.pci = c.pdev;
        auto &p = g.procs[c.pid];
        p.vram += c.vram;
        g.vramUsed += c.vram;
        auto prev = prevClients_.find(key);
        if (prev == prevClients_.end() || interval <= 0)
            continue;
        double best = 0;
        for (auto &[eng, ns] : c.engines) {
            auto pe = prev->second.engines.find(eng);
            if (pe == prev->second.engines.end() || ns < pe->second)
                continue;
            double b = double(ns - pe->second) / (interval * 1e9);
            engineSum[c.pdev][eng] += b;
            best = std::max(best, b);
        }
        p.busy = std::min(1.0, p.busy + best);
    }
    for (auto &[pdev, g] : drm) {
        for (auto &[eng, b] : engineSum[pdev])
            g.busy = std::max(g.busy, std::min(1.0, b));
        out.push_back(std::move(g));
    }
    prevClients_ = std::move(clients_);
    clients_.clear();
    return out;
}

} // namespace jkv
