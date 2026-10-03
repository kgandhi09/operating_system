#include "netcap.h"

#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace jkv {

bool NetCapture::start()
{
    if (fd_ >= 0)
        return true;
    // SOCK_DGRAM: the packets without their link-layer header, from the IP header on.
    fd_ = socket(AF_PACKET, SOCK_DGRAM | SOCK_CLOEXEC, htons(ETH_P_ALL));
    if (fd_ < 0)
        return false;
    int rcv = 8 << 20;
    setsockopt(fd_, SOL_SOCKET, SO_RCVBUF, &rcv, sizeof rcv);
    quit_ = false;
    th_ = std::thread([this] { run(); });
    return true;
}

void NetCapture::stop()
{
    if (fd_ < 0)
        return;
    quit_ = true;
    th_.join();
    close(fd_);
    fd_ = -1;
    std::lock_guard<std::mutex> l(mu_);
    counts_.clear();
}

void NetCapture::setInterfaces(const std::set<int> &ifs)
{
    std::lock_guard<std::mutex> l(mu_);
    ifs_ = ifs;
}

std::map<uint32_t, PortBytes> NetCapture::take(uint64_t &drops)
{
    drops = 0;
    if (fd_ >= 0) {
        tpacket_stats st{};
        socklen_t len = sizeof st;
        if (getsockopt(fd_, SOL_PACKET, PACKET_STATISTICS, &st, &len) == 0)   // (and resets them)
            drops = st.tp_drops;
    }
    std::lock_guard<std::mutex> l(mu_);
    std::map<uint32_t, PortBytes> m;
    m.swap(counts_);
    return m;
}

void NetCapture::run()
{
    unsigned char buf[128];     // enough for the IP and TCP/UDP headers
    std::set<int> ifs;
    int n = 0;
    while (!quit_) {
        pollfd p{fd_, POLLIN, 0};
        if (poll(&p, 1, 200) <= 0)
            continue;
        // Everything queued, then wait again.
        for (int burst = 0; burst < 4096 && !quit_; burst++) {
            sockaddr_ll sll{};
            socklen_t sl = sizeof sll;
            // MSG_TRUNC: the packet's real length, though only its start is copied.
            ssize_t len = recvfrom(fd_, buf, sizeof buf, MSG_TRUNC | MSG_DONTWAIT, (sockaddr *)&sll, &sl);
            if (len <= 0)
                break;
            if ((n++ & 255) == 0) {
                std::lock_guard<std::mutex> l(mu_);
                ifs = ifs_;
            }
            if (!ifs.count(sll.sll_ifindex))
                continue;
            bool out = sll.sll_pkttype == PACKET_OUTGOING;
            size_t got = size_t(len) < sizeof buf ? size_t(len) : sizeof buf;
            unsigned proto = 0;
            size_t l4 = 0;
            uint16_t eth = ntohs(sll.sll_protocol);
            if (eth == ETH_P_IP && got >= 20 && (buf[0] >> 4) == 4) {
                proto = buf[9];
                l4 = size_t(buf[0] & 15) * 4;
                // Only the first fragment carries the ports.
                if ((((buf[6] & 0x1f) << 8) | buf[7]) != 0)
                    proto = 0;
            } else if (eth == ETH_P_IPV6 && got >= 40) {
                proto = buf[6];
                l4 = 40;
            }
            uint32_t key = 0;
            if ((proto == 6 || proto == 17) && got >= l4 + 4) {
                uint16_t sport = uint16_t(buf[l4] << 8 | buf[l4 + 1]);
                uint16_t dport = uint16_t(buf[l4 + 2] << 8 | buf[l4 + 3]);
                key = proto << 16 | (out ? sport : dport);
            }
            std::lock_guard<std::mutex> l(mu_);
            auto &c = counts_[key];
            (out ? c.tx : c.rx) += uint64_t(len);
        }
    }
}

} // namespace jkv
