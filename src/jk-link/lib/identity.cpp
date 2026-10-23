#include "identity.h"

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <fcntl.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <sys/stat.h>
#include <unistd.h>

namespace jkl {

std::string toHex(const uint8_t *data, size_t n)
{
    static const char digits[] = "0123456789abcdef";
    std::string s;
    s.reserve(n * 2);
    for (size_t i = 0; i < n; ++i) {
        s += digits[data[i] >> 4];
        s += digits[data[i] & 15];
    }
    return s;
}

bool fromHex(const std::string &hex, PublicKey &key)
{
    if (hex.size() != key.size() * 2)
        return false;
    auto nib = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    for (size_t i = 0; i < key.size(); ++i) {
        int h = nib(hex[2 * i]), l = nib(hex[2 * i + 1]);
        if (h < 0 || l < 0)
            return false;
        key[i] = uint8_t(h << 4 | l);
    }
    return true;
}

std::string machineId(const PublicKey &key)
{
    uint8_t d[SHA256_DIGEST_LENGTH];
    SHA256(key.data(), key.size(), d);
    std::string h = toHex(d, 4);
    return "jk-" + h.substr(0, 4) + "-" + h.substr(4, 4);
}

std::string fingerprint(const PublicKey &key)
{
    uint8_t d[SHA256_DIGEST_LENGTH];
    SHA256(key.data(), key.size(), d);
    std::string h = toHex(d, sizeof d), out;
    for (size_t i = 0; i < h.size(); i += 4) {
        if (i)
            out += ' ';
        out += h.substr(i, 4);
    }
    return out;
}

bool verify(const PublicKey &key, const uint8_t *msg, size_t n, const Signature &sig)
{
    EVP_PKEY *pk = EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, nullptr, key.data(), key.size());
    if (!pk)
        return false;
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    bool ok = ctx && EVP_DigestVerifyInit(ctx, nullptr, nullptr, nullptr, pk) == 1
              && EVP_DigestVerify(ctx, sig.data(), sig.size(), msg, n) == 1;
    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pk);
    return ok;
}

std::string pairingCode(const PublicKey &a, const PublicKey &b)
{
    const PublicKey &lo = a < b ? a : b, &hi = a < b ? b : a;
    uint8_t buf[64 + 16], d[SHA256_DIGEST_LENGTH];
    std::copy(lo.begin(), lo.end(), buf);
    std::copy(hi.begin(), hi.end(), buf + 32);
    std::copy_n("jk-link pairing", 16, buf + 64);
    SHA256(buf, sizeof buf, d);
    uint32_t v = uint32_t(d[0]) << 24 | uint32_t(d[1]) << 16 | uint32_t(d[2]) << 8 | d[3];
    char code[8];
    std::snprintf(code, sizeof code, "%06u", v % 1000000);
    return std::string(code, 3) + " " + std::string(code + 3, 3);
}

Identity::~Identity()
{
    EVP_PKEY_free(key_);
}

bool Identity::loadOrCreate(const std::string &dir)
{
    std::string path = dir + "/identity.key";
    if (FILE *f = std::fopen(path.c_str(), "r")) {
        key_ = PEM_read_PrivateKey(f, nullptr, nullptr, nullptr);
        std::fclose(f);
        if (!key_ || EVP_PKEY_get_id(key_) != EVP_PKEY_ED25519) {
            error_ = path + " is not an Ed25519 private key";
            return false;
        }
    } else {
        key_ = EVP_PKEY_Q_keygen(nullptr, nullptr, "ED25519");
        if (!key_) {
            error_ = "cannot make an Ed25519 key";
            return false;
        }
        std::string tmp = path + ".new";
        int fd = ::open(tmp.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
        FILE *out = fd >= 0 ? fdopen(fd, "w") : nullptr;
        bool ok = out && PEM_write_PrivateKey(out, key_, nullptr, nullptr, 0, nullptr, nullptr) == 1;
        if (out) {
            ok = ok && std::fflush(out) == 0 && fsync(fd) == 0;
            std::fclose(out);
        } else if (fd >= 0) {
            ::close(fd);
        }
        if (!ok || std::rename(tmp.c_str(), path.c_str()) != 0) {
            ::unlink(tmp.c_str());
            error_ = "cannot write " + path;
            return false;
        }
        created_ = true;
    }
    size_t n = pub_.size();
    if (EVP_PKEY_get_raw_public_key(key_, pub_.data(), &n) != 1 || n != pub_.size()) {
        error_ = "cannot read the public key";
        return false;
    }
    return true;
}

Signature Identity::sign(const uint8_t *msg, size_t n) const
{
    Signature sig{};
    size_t len = sig.size();
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx && EVP_DigestSignInit(ctx, nullptr, nullptr, nullptr, key_) == 1)
        EVP_DigestSign(ctx, sig.data(), &len, msg, n);
    EVP_MD_CTX_free(ctx);
    return sig;
}

SSL_CTX *Identity::tlsContext(bool server)
{
    SSL_CTX *ctx = SSL_CTX_new(server ? TLS_server_method() : TLS_client_method());
    if (!ctx)
        return nullptr;
    SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);
    SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION);
    // No session resumption: every connection proves its key again.
    SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_OFF);
    SSL_CTX_set_num_tickets(ctx, 0);

    X509 *cert = X509_new();
    X509_set_version(cert, 2);
    uint8_t serial[8];
    RAND_bytes(serial, sizeof serial);
    BIGNUM *bn = BN_bin2bn(serial, sizeof serial, nullptr);
    BN_to_ASN1_INTEGER(bn, X509_get_serialNumber(cert));
    BN_free(bn);
    X509_gmtime_adj(X509_getm_notBefore(cert), -86400);
    X509_gmtime_adj(X509_getm_notAfter(cert), 100L * 365 * 86400);
    X509_set_pubkey(cert, key_);
    X509_NAME *name = X509_get_subject_name(cert);
    std::string cn = id();
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_UTF8,
                               reinterpret_cast<const unsigned char *>(cn.c_str()), -1, -1, 0);
    X509_set_issuer_name(cert, name);
    bool ok = X509_sign(cert, key_, nullptr) > 0 && SSL_CTX_use_certificate(ctx, cert) == 1
              && SSL_CTX_use_PrivateKey(ctx, key_) == 1;
    X509_free(cert);
    if (!ok) {
        SSL_CTX_free(ctx);
        return nullptr;
    }
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                       [](int, X509_STORE_CTX *) { return 1; });
    return ctx;
}

bool peerKey(SSL *ssl, PublicKey &key)
{
    X509 *cert = SSL_get1_peer_certificate(ssl);
    if (!cert)
        return false;
    EVP_PKEY *pk = X509_get0_pubkey(cert);
    size_t n = key.size();
    bool ok = pk && EVP_PKEY_get_id(pk) == EVP_PKEY_ED25519
              && EVP_PKEY_get_raw_public_key(pk, key.data(), &n) == 1 && n == key.size();
    X509_free(cert);
    return ok;
}

} // namespace jkl
