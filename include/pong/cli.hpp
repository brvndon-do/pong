#pragma once

#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <variant>

namespace pong::cli {

struct Endpoint {
    std::string address;
    std::uint16_t port{0};
};

struct HostSettings {
    Endpoint endpoint{};
    std::string playerName{"Player 1"};
};

struct ClientSettings {
    Endpoint endpoint{};
    std::string playerName{"Player 2"};
};

struct LocalSettings {
    std::string player1Name{"Player 1"};
    std::string player2Name{"Player 2"};
};

using Settings = std::variant<LocalSettings, HostSettings, ClientSettings>;

[[nodiscard]] std::expected<Settings, std::string>
parseArgs(std::span<const std::string_view> args);
} // namespace pong::cli