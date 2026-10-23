// What jk-linkd keeps in its state directory (/data/etc/jk-link on an
// installed system), besides identity.key:
//
//   trust     the paired machines: "<public key hex> <paired since> <name>"
//   link.conf settings: "visibility open|paired|off", "peer <address>" (added
//             by hand, for networks without multicast)
//   cache     where paired machines were last seen: "<key hex> <address>"
//
// Each is rewritten whole (a new file renamed over the old one).
#pragma once

#include "identity.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace jkl {

struct TrustEntry {
    std::string name;      // its name when it was paired
    int64_t since = 0;     // unix time
};

enum class Visibility { Open, Paired, Off };
const char *visibilityName(Visibility v);
bool parseVisibility(const std::string &s, Visibility &v);

struct Store {
    std::string dir;
    std::map<PublicKey, TrustEntry> trust;
    Visibility visibility = Visibility::Open;
    std::vector<std::string> manual;
    std::map<PublicKey, std::vector<std::string>> cache;

    void load();
    bool saveTrust() const;
    bool saveConfig() const;
    bool saveCache() const;
};

} // namespace jkl
