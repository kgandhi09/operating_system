// jk-vizd: network bytes per local TCP/UDP port, from a packet socket that
// sees every packet of the physical interfaces (headers only: nothing of
// the contents is read or kept).
#pragma once
#include <atomic>
#include <cstdint>
#include <map>
#include <mutex>
#include <set>
#include <thread>

namespace jkv {

struct PortBytes { uint64_t rx = 0, tx = 0; };

class NetCapture {
public:
    ~NetCapture() { stop(); }
    bool start();
    void stop();
    bool running() const { return fd_ >= 0; }
    // The interfaces to count (the physical ones), by index.
    void setInterfaces(const std::set<int> &ifs);
    // What was counted since the last call: by (protocol << 16 | local port),
    // protocol 6 (TCP) or 17 (UDP); the rest (other protocols) under key 0.
    std::map<uint32_t, PortBytes> take(uint64_t &drops);

private:
    int fd_ = -1;
    std::atomic<bool> quit_{false};
    std::thread th_;
    std::mutex mu_;
    std::map<uint32_t, PortBytes> counts_;
    std::set<int> ifs_;
    void run();
};

} // namespace jkv
