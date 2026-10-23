// The local control socket of jk-linkd (/run/jk-link/socket): one JSON
// request per line, {"cmd": "...", ...}, answered by one JSON line,
// {"ok": true, ...} or {"ok": false, "error": "..."}. Anyone may ask
// (status); changing anything (pair, accept, unpair, add, visibility, ...)
// takes root or group wheel. JK_LINK_SOCKET overrides the path.
#pragma once

#include <string>

namespace jkl {

constexpr const char *kSocketPath = "/run/jk-link/socket";

std::string socketPath();
// A blocking client: connect, then request() a line and read the answer.
class ControlClient {
public:
    ~ControlClient();
    bool connect(std::string *error = nullptr);
    // The answer line, or empty (and an error) if the daemon went away.
    std::string request(const std::string &line, std::string *error = nullptr);

private:
    int fd_ = -1;
    std::string buf_;
};

} // namespace jkl
