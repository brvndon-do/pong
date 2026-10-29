#pragma once

#include <cstdint>
#include <expected>
#include <string>

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

    [[nodiscard]] static std::expected<Socket, std::string>
    createHost(const Endpoint& endpoint);
    [[nodiscard]] static std::expected<Socket, std::string>
    createClient(const Endpoint& endpoint);

    [[nodiscard]] std::expected<Socket, std::string> accept();

    [[nodiscard]] int fd() const noexcept;
    [[nodiscard]] bool isValid() const noexcept;
};
} // namespace pong::network