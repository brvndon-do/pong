#include "pong/network_protocol.hpp"
#include <arpa/inet.h>
#include <array>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <expected>
#include <format>
#include <netinet/in.h>
#include <optional>
#include <pong/network.hpp>
#include <span>
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

std::expected<void, std::string>
Connection::send(std::span<const std::byte> data) {
    ssize_t total{0}, written{0}, bytesLeft{static_cast<ssize_t>(data.size())};

    /*
        another cool way to write this is:
            while (!data.empty())
                send data.data() and data.size()
                if successful write
                    data = data.subspan(written)

        subspan(n) returns a view that skips the first n bytes

        i'm choosing to leave it since that's how i learned it from beej's
       networking guide
    */

    while (bytesLeft > 0) {
        written = ::send(s_.fd(), data.data() + total, bytesLeft, MSG_NOSIGNAL);
        if (written == -1)
            return std::unexpected{std::format(
                "socket: error during send; err: {}", std::strerror(errno))};

        total += written;
        bytesLeft -= written;
    }

    return {};
}

std::expected<void, std::string> Connection::receive() {
    std::array<std::byte, 256> buf{};
    ssize_t bytesReceived{::recv(s_.fd(), buf.data(), buf.size(), 0)};

    // peer closed connection
    if (bytesReceived == 0)
        return std::unexpected{"socket: connection closed"};

    if (bytesReceived == -1) {
        // nothing to read
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return {};

        return std::unexpected{std::format("socket: unexpected error; err: {}",
                                           std::strerror(errno))};
    }

    // bytesReceived > 0
    receiveBuffer_.insert(receiveBuffer_.end(), buf.begin(),
                          buf.begin() + bytesReceived);

    return {};
}

std::expected<std::optional<protocol::Message>, std::string>
Connection::nextMessage() {
    if (receiveBuffer_.empty())
        return std::nullopt;

    protocol::MessageType messageType{
        static_cast<protocol::MessageType>(receiveBuffer_[0])};

    auto payloadSize{protocol::mapPayloadSz(messageType)};

    if (!payloadSize.has_value())
        return std::unexpected{"payload: unknown tag"};

    const size_t totalPayloadSize{*payloadSize + protocol::header_sz};

    if (receiveBuffer_.size() < totalPayloadSize)
        return std::nullopt; // bytes stay in buffer until more arrive

    // extract payload from buffer (exclude header byte)
    std::span payloadSpan{
        std::span{receiveBuffer_}.subspan(protocol::header_sz, *payloadSize)};
    protocol::Message decodedPayload{};

    switch (messageType) {
    case protocol::MessageType::Greet: {
        if (auto decoded{protocol::decodeGreetPayload(payloadSpan)};
            decoded.has_value())
            decodedPayload = *decoded;
        else
            return std::unexpected{"payload: decoding greet failed"};

        break;
    }
    case protocol::MessageType::Input: {
        if (auto decoded{protocol::decodeInputPayload(payloadSpan)};
            decoded.has_value())
            decodedPayload = *decoded;
        else
            return std::unexpected{"payload: decoding input failed"};

        break;
    }
    case protocol::MessageType::Snapshot: {
        if (auto decoded{protocol::decodeSnapshotPayload(payloadSpan)};
            decoded.has_value())
            decodedPayload = *decoded;
        else
            return std::unexpected{"payload: decoding snapshot failed"};

        break;
    }
    }

    // remove message from front of buffer
    receiveBuffer_.erase(receiveBuffer_.begin(),
                         receiveBuffer_.begin() + totalPayloadSize);

    return decodedPayload;
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