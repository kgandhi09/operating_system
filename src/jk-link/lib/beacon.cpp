#include "beacon.h"

#include <cstring>

namespace jkl {

namespace {

void putString(std::vector<uint8_t> &out, const std::string &s, size_t max)
{
    size_t n = std::min(s.size(), max);
    out.push_back(uint8_t(n));
    out.insert(out.end(), s.begin(), s.begin() + long(n));
}

bool getString(const uint8_t *&p, const uint8_t *end, std::string &s)
{
    if (p >= end)
        return false;
    size_t n = *p++;
    if (size_t(end - p) < n)
        return false;
    s.assign(reinterpret_cast<const char *>(p), n);
    p += n;
    return true;
}

} // namespace

std::vector<uint8_t> Beacon::encode(const Identity &id) const
{
    std::vector<uint8_t> out = {'J', 'K', 'L', '1', type, flags, uint8_t(port), uint8_t(port >> 8)};
    for (int i = 0; i < 8; ++i)
        out.push_back(uint8_t(seq >> (8 * i)));
    const PublicKey &k = id.publicKey();
    out.insert(out.end(), k.begin(), k.end());
    putString(out, name, 63);
    putString(out, arch, 15);
    putString(out, version, 31);
    Signature sig = id.sign(out.data(), out.size());
    out.insert(out.end(), sig.begin(), sig.end());
    return out;
}

std::optional<Beacon> Beacon::decode(const uint8_t *data, size_t n)
{
    if (n < 48 + 3 + 64 || n > 512 || std::memcmp(data, "JKL1", 4) != 0)
        return std::nullopt;
    Beacon b;
    if (data[4] < Announce || data[4] > Leave)
        return std::nullopt;
    b.type = Type(data[4]);
    b.flags = data[5];
    b.port = uint16_t(data[6] | data[7] << 8);
    for (int i = 0; i < 8; ++i)
        b.seq |= uint64_t(data[8 + i]) << (8 * i);
    std::memcpy(b.key.data(), data + 16, 32);
    const uint8_t *p = data + 48, *end = data + n - 64;
    if (!getString(p, end, b.name) || !getString(p, end, b.arch) || !getString(p, end, b.version)
        || p != end)
        return std::nullopt;
    Signature sig;
    std::memcpy(sig.data(), end, 64);
    if (!verify(b.key, data, n - 64, sig))
        return std::nullopt;
    return b;
}

} // namespace jkl
