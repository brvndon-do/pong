#pragma once

#include <cstdint>
#include <expected>
#include <pong/network_protocol.hpp>
#include <span>
#include <string>
#include <vector>

namespace pong::network {
struct Endpoint {
    std::string address;
    std::uint16_t port{0};
};

class Socket {
private:
    int fd_{-1};

public:
    Socket() = default;
    explicit Socket(int fd);

    // copy
    Socket(const Socket& other) = delete;
    Socket& operator=(const Socket& other) = delete;

    // move
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    ~Socket();

    [[nodiscard]] int fd() const noexcept;
    [[nodiscard]] bool isValid() const noexcept;
};

class Connection {
    friend class Listener;

private:
    Socket s_{};
    std::vector<std::byte> receiveBuffer_;

    explicit Connection(Socket s);

public:
    [[nodiscard]] static std::expected<Connection, std::string>
    createClient(const Endpoint& endpoint);

    [[nodiscard]] std::expected<void, std::string>
    send(std::span<const std::byte> data);
    [[nodiscard]] std::expected<void, std::string> receive();
    [[nodiscard]] std::expected<std::optional<protocol::Message>, std::string>
    nextMessage();
};

class Listener {
private:
    Socket s_{};

    explicit Listener(Socket s);

public:
    [[nodiscard]] static std::expected<Listener, std::string>
    createHost(const Endpoint& endpoint);

    [[nodiscard]] std::expected<Connection, std::string> accept();
};
} // namespace pong::network