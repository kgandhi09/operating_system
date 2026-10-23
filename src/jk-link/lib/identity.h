// A machine's jk-link identity: its Ed25519 key pair, made on the first
// start and kept in the state directory (identity.key, 0600). The public key
// is the identity; everything else (name, addresses) is only a label.
//
//   machine ID   jk-xxxx-xxxx: the first 4 bytes of SHA-256(public key)
//   fingerprint  the whole SHA-256(public key), hex in groups of 4
#pragma once

#include <array>
#include <cstdint>
#include <string>

typedef struct evp_pkey_st EVP_PKEY;
typedef struct ssl_ctx_st SSL_CTX;
typedef struct ssl_st SSL;

namespace jkl {

using PublicKey = std::array<uint8_t, 32>;
using Signature = std::array<uint8_t, 64>;

std::string toHex(const uint8_t *data, size_t n);
bool fromHex(const std::string &hex, PublicKey &key);
std::string machineId(const PublicKey &key);
std::string fingerprint(const PublicKey &key);
bool verify(const PublicKey &key, const uint8_t *msg, size_t n, const Signature &sig);

// The pairing code both machines show: 6 digits from SHA-256 over both
// public keys (in byte order, so it is the same on both sides), "482 913".
std::string pairingCode(const PublicKey &a, const PublicKey &b);

class Identity {
public:
    Identity() = default;
    ~Identity();
    Identity(const Identity &) = delete;
    Identity &operator=(const Identity &) = delete;

    // Loads <dir>/identity.key, or makes and saves a new key. False and a
    // message in error() if neither works.
    bool loadOrCreate(const std::string &dir);
    const std::string &error() const { return error_; }
    bool created() const { return created_; }

    const PublicKey &publicKey() const { return pub_; }
    std::string id() const { return machineId(pub_); }
    Signature sign(const uint8_t *msg, size_t n) const;

    // A TLS 1.3 context with this key and a self-signed certificate for
    // it, asking the other side for its own. Certificates aren't checked
    // against anything: who the other side is comes from its public key
    // (peerKey), which the caller compares with the one it expects.
    SSL_CTX *tlsContext(bool server);

private:
    EVP_PKEY *key_ = nullptr;
    PublicKey pub_{};
    std::string error_;
    bool created_ = false;
};

// The public key the other side of an established TLS connection proved.
bool peerKey(SSL *ssl, PublicKey &key);

} // namespace jkl
