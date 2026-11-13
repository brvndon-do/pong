#pragma once

#include <cstdint>
#include <expected>
#include <optional>
#include <pong/cli.hpp>
#include <pong/network.hpp>
#include <string>

namespace pong::session {
enum class Environment : std::uint8_t { Local, Host, Client };

struct Session {
    std::string player1Name;
    std::string player2Name;
    std::optional<network::Connection> peer;
    Environment env{};
};

[[nodiscard]] std::expected<Session, std::string>
createSession(const cli::Settings& settings);
} // namespace pong::session