#include <charconv>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <optional>
#include <pong/cli.hpp>
#include <string>
#include <string_view>
#include <utility>

namespace pong::cli {
namespace {
// TODO: move length check out of this function; for now, if maxTokenLength ==
// -1, do not validate
std::expected<std::string_view, std::string>
nextValue(const std::span<const std::string_view> args, std::size_t& index,
          std::size_t maxTokenLength = 10) {
    if (index + 1 >= args.size())
        return std::unexpected{
            std::format("Missing argument: {}", args[index])};

    if (args[index + 1].empty())
        return std::unexpected{
            std::format("Argument {} cannot be empty", args[index])};

    if (std::cmp_greater(maxTokenLength, -1) &&
        args[index + 1].length() > maxTokenLength)
        return std::unexpected{std::format("Argument {} exceeds {} characters",
                                           args[index], maxTokenLength)};

    return args[++index];
}

template <typename T>
concept NetworkSettings =
    std::same_as<T, HostSettings> || std::same_as<T, ClientSettings>;

template <NetworkSettings T>
std::expected<T, std::string>
parseNetwork(const std::span<const std::string_view> subArgs) {
    T networkSettings{};
    std::optional<std::string_view> parsedAddress;
    std::optional<std::uint16_t> parsedPort;

    for (std::size_t i = 0; i < subArgs.size(); ++i) {
        if (subArgs[i] == "-s") {
            if (parsedAddress.has_value())
                return std::unexpected{"Duplicated -s flag"};

            // TODO: -1 means no length validation; fix later
            if (const auto nextToken{nextValue(subArgs, i, -1)};
                nextToken.has_value())
                parsedAddress = *nextToken;
            else
                return std::unexpected{nextToken.error()};
        } else if (subArgs[i] == "-p") {
            if (parsedPort.has_value())
                return std::unexpected{"Duplicated -p flag"};

            if (const auto nextToken{nextValue(subArgs, i)};
                nextToken.has_value()) {
                std::uint16_t port{0};
                const auto [ptr, ec] = std::from_chars(
                    nextToken->data(), nextToken->data() + nextToken->size(),
                    port);

                if (ec == std::errc::invalid_argument)
                    return std::unexpected{
                        "Failed to parse port: not a number"};
                else if (ec == std::errc::result_out_of_range)
                    return std::unexpected{
                        "Failed to parse port: out of range"};
                else if (ptr != (nextToken->data() + nextToken->size()))
                    return std::unexpected{
                        "Failed to parse port: trailing junk"};
                else if (port == 0)
                    return std::unexpected{"Port 0 is not valid"};
                else
                    parsedPort = port;

            } else
                return std::unexpected{nextToken.error()};
        } else if (subArgs[i] == "-n") {
            if (const auto nextToken{nextValue(subArgs, i)};
                nextToken.has_value()) {
                networkSettings.playerName = *nextToken;
            } else
                return std::unexpected{nextToken.error()};
        } else
            return std::unexpected{std::format("Unknown flag: {}", subArgs[i])};
    }

    if (!parsedAddress || !parsedPort)
        return std::unexpected{"Both -s and -p flags are required"};

    networkSettings.endpoint.address = *parsedAddress;
    networkSettings.endpoint.port = *parsedPort;

    return networkSettings;
}

std::expected<LocalSettings, std::string>
parseLocal(const std::span<const std::string_view> args) {
    LocalSettings localSettings{};
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "-n1") {
            if (const auto nextToken{nextValue(args, i)}; nextToken.has_value())
                localSettings.player1Name = *nextToken;
            else
                return std::unexpected{nextToken.error()};
        } else if (args[i] == "-n2") {
            if (const auto nextToken{nextValue(args, i)}; nextToken.has_value())
                localSettings.player2Name = *nextToken;
            else
                return std::unexpected{nextToken.error()};
        } else
            return std::unexpected{std::format("Unknown flag: {}", args[i])};
    }

    return localSettings;
}
} // namespace

std::expected<Settings, std::string>
parseArgs(std::span<const std::string_view> args) {
    // network play
    if (!args.empty()) {
        const auto subArgs{args.subspan(1)};

        if (args[0] == "host")
            return parseNetwork<HostSettings>(subArgs);
        else if (args[0] == "client")
            return parseNetwork<ClientSettings>(subArgs);
    }

    // local play
    return parseLocal(args);
}
} // namespace pong::cli