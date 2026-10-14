#pragma once

#include <expected>
#include <span>
#include <string>
#include <string_view>

namespace pong::cli {
struct Settings {
    std::string player1Name{"Player 1"};
    std::string player2Name{"Player 2"};
};

[[nodiscard]] std::expected<Settings, std::string>
parseArgs(std::span<const std::string_view> args);
} // namespace pong::cli