#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <expected>
#include <format>
#include <netinet/in.h>
#include <pong/network.hpp>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

namespace pong::network {
namespace {
[[nodiscard]] std::expected<Socket, std::string> createSocket() {
    const int sfd{::socket(AF_INET, SOCK_STREAM, 0)};
    if (sfd == -1)
        return std::unexpected{
            std::format("socket: failed; err: {}", std::strerror(errno))};

    return Socket{sfd};
}

[[nodiscard]] std::expected<sockaddr_in, std::string>
createSocketAddr(const Endpoint& endpoint) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(endpoint.port);

    int inetrc{::inet_pton(AF_INET, endpoint.address.c_str(), &addr.sin_addr)};

    // invalid addr
    if (inetrc == 0)
        return std::unexpected{
            std::format("socket: failed to parse addr: {}", endpoint.address)};

    // unsupported addr family
    if (inetrc == -1)
        return std::unexpected{"socket: unsupported addr family"};

    return addr;
}
} // namespace

// Socket

// explicit
Socket::Socket(int fd) : fd_{fd} {}

Socket::Socket(Socket&& other) noexcept : fd_{std::exchange(other.fd_, -1)} {}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        if (fd_ != -1)
            ::close(fd_);

        fd_ = std::exchange(other.fd_, -1);
    }

    return *this;
}

Socket::~Socket() {
    if (fd_ != -1)
        ::close(fd_);
}

int Socket::fd() const noexcept { return fd_; }

bool Socket::isValid() const noexcept { return fd_ != -1; }

// Connection

Connection::Connection(Socket s) : s_{std::move(s)} {}

std::expected<Connection, std::string>
Connection::createClient(const Endpoint& endpoint) {
    auto s{createSocket()};
    if (!s.has_value())
        return std::unexpected{s.error()};

    auto addr{createSocketAddr(endpoint)};
    if (!addr.has_value())
        return std::unexpected{addr.error()};

    int connectrc{::connect(s->fd(),
                            reinterpret_cast<const sockaddr*>(&addr.value()),
                            sizeof(sockaddr_in))};

    if (connectrc == -1)
        return std::unexpected{std::format("socket: connect failed; err: {}",
                                           std::strerror(errno))};

    return Connection{std::move(s).value()};
}

// Listener

Listener::Listener(Socket s) : s_{std::move(s)} {}

std::expected<Listener, std::string>
Listener::createHost(const Endpoint& endpoint) {
    const int optval{1};

    auto s{createSocket()};
    if (!s.has_value())
        return std::unexpected{s.error()};

    int ssrc{::setsockopt(s->fd(), SOL_SOCKET, SO_REUSEADDR, &optval,
                          sizeof(optval))};
    if (ssrc == -1)
        return std::unexpected{
            std::format("socket: setsockopt(SO_REUSEADDR) failed; err: {}",
                        std::strerror(errno))};

    auto addr{createSocketAddr(endpoint)};
    if (!addr.has_value())
        return std::unexpected{addr.error()};

    int bindrc{::bind(s->fd(), reinterpret_cast<const sockaddr*>(&addr.value()),
                      sizeof(sockaddr_in))};
    if (bindrc == -1)
        return std::unexpected{
            std::format("socket: bind failed; err: {}", std::strerror(errno))};

    // always 1 client; n = 1
    int listenrc{::listen(s->fd(), 1)};
    if (listenrc == -1)
        return std::unexpected{std::format("socket: listen failed; err: {}",
                                           std::strerror(errno))};

    return Listener{std::move(s).value()};
}

std::expected<Connection, std::string> Listener::accept() {
    int sfd{::accept(s_.fd(), nullptr, nullptr)};
    if (sfd == -1)
        return std::unexpected{std::format("socket: accept failed; err: {}",
                                           std::strerror(errno))};

    return Connection{Socket{sfd}};
}

} // namespace pong::network