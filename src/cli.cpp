#include <cstddef>
#include <expected>
#include <format>
#include <pong/cli.hpp>
#include <string>
#include <string_view>

namespace pong::cli {
namespace {
std::expected<std::string_view, std::string>
nextValue(const std::span<const std::string_view> args, std::size_t& index,
          std::size_t maxTokenLength = 10) {
    if (index + 1 >= args.size())
        return std::unexpected{
            std::format("Missing argument: {}", args[index])};

    if (args[index + 1].empty())
        return std::unexpected{
            std::format("Argument {} cannot be empty", args[index])};

    if (args[index + 1].length() > maxTokenLength)
        return std::unexpected{std::format("Argument {} exceeds {} characters",
                                           args[index], maxTokenLength)};

    return args[++index];
}
} // namespace

std::expected<Settings, std::string>
parseArgs(std::span<const std::string_view> args) {
    Settings userSettings{};

    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "-p1") {
            if (const auto nextToken{nextValue(args, i)}; nextToken.has_value())
                userSettings.player1Name = *nextToken;
            else
                return std::unexpected{nextToken.error()};
        } else if (args[i] == "-p2") {
            if (const auto nextToken{nextValue(args, i)}; nextToken.has_value())
                userSettings.player2Name = *nextToken;
            else
                return std::unexpected{nextToken.error()};
        } else
            return std::unexpected{std::format("Unknown flag: {}", args[i])};
    }

    return userSettings;
}
} // namespace pong::cli