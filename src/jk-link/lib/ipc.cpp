#include "ipc.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace jkl {

std::string socketPath()
{
    const char *env = std::getenv("JK_LINK_SOCKET");
    return env && *env ? env : kSocketPath;
}

ControlClient::~ControlClient()
{
    if (fd_ >= 0)
        ::close(fd_);
}

bool ControlClient::connect(std::string *error)
{
    std::string path = socketPath();
    sockaddr_un sun{};
    sun.sun_family = AF_UNIX;
    if (path.size() >= sizeof sun.sun_path) {
        if (error) *error = "socket path too long: " + path;
        return false;
    }
    std::memcpy(sun.sun_path, path.c_str(), path.size() + 1);
    fd_ = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd_ < 0 || ::connect(fd_, reinterpret_cast<sockaddr *>(&sun), sizeof sun) != 0) {
        if (error) *error = "jk-linkd is not running (" + path + ": " + std::strerror(errno) + ")";
        return false;
    }
    return true;
}

std::string ControlClient::request(const std::string &line, std::string *error)
{
    std::string out = line + "\n";
    for (size_t off = 0; off < out.size();) {
        ssize_t n = ::write(fd_, out.data() + off, out.size() - off);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0) {
            if (error) *error = "jk-linkd closed the connection";
            return {};
        }
        off += size_t(n);
    }
    for (;;) {
        size_t nl = buf_.find('\n');
        if (nl != std::string::npos) {
            std::string ans = buf_.substr(0, nl);
            buf_.erase(0, nl + 1);
            return ans;
        }
        char tmp[65536];
        ssize_t n = ::read(fd_, tmp, sizeof tmp);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0) {
            if (error) *error = "jk-linkd closed the connection";
            return {};
        }
        buf_.append(tmp, size_t(n));
    }
}

} // namespace jkl
