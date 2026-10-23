// The discovery beacon: one UDP datagram (port 7710; multicast to
// 239.255.77.10 and ff02::7710, or unicast) saying "this key is here, its
// jk-linkd listens on TCP port N". Signed with the sender's key, so nobody
// can announce someone else's key at another address. A hint only: who a
// machine is gets proven again in the TLS handshake.
//
//   0   "JKL1"
//   4   u8   type: 1 announce, 2 query (an announce asking for answers),
//            3 leave (going away)
//   5   u8   flags (none yet)
//   6   u16  TCP port, little-endian
//   8   u64  sequence (the sender's clock in ms: newer replaces older)
//   16  32   Ed25519 public key
//   48  name, arch, version: each a u8 length and that many bytes (UTF-8)
//   ..  64   signature over everything before it
#pragma once

#include "identity.h"

#include <optional>
#include <string>
#include <vector>

namespace jkl {

constexpr uint16_t kPort = 7710;
constexpr const char *kGroup4 = "239.255.77.10";
constexpr const char *kGroup6 = "ff02::7710";

struct Beacon {
    enum Type : uint8_t { Announce = 1, Query = 2, Leave = 3 };
    Type type = Announce;
    uint8_t flags = 0;
    uint16_t port = kPort;
    uint64_t seq = 0;
    PublicKey key{};
    std::string name, arch, version;

    // The signed datagram (fields longer than their limit are cut).
    std::vector<uint8_t> encode(const Identity &id) const;
    // A beacon from a datagram, only if well formed and its signature holds.
    static std::optional<Beacon> decode(const uint8_t *data, size_t n);
};

} // namespace jkl
