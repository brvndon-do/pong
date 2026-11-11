#include <algorithm>
#include <array>
#include <cstdint>
#include <expected>
#include <optional>
#include <pong/cli.hpp>
#include <pong/config.hpp>
#include <pong/network.hpp>
#include <pong/network_protocol.hpp>
#include <pong/session.hpp>
#include <string>
#include <string_view>
#include <variant>

namespace protocol = pong::network::protocol;

namespace pong::session {
namespace {
std::expected<std::string, std::string>
exchangeNames(network::Connection& connection, std::string_view playerName) {
    std::array<char, config::max_player_name> name{};
    std::string_view clampedName{playerName.substr(0, config::max_player_name)};
    std::ranges::copy(clampedName, name.begin());
    protocol::GreetPayload greetPayload{
        .nameLength = static_cast<std::uint8_t>(clampedName.length()),
        .playerName = name};
    auto encodedGreetPayload{protocol::encodeGreetPayload(greetPayload)};
    if (auto res{connection.send(encodedGreetPayload)}; !res.has_value())
        return std::unexpected{res.error()};

    while (true) {
        auto expectedMessage{connection.nextMessage()};
        if (!expectedMessage)
            return std::unexpected{expectedMessage.error()};

        if (*expectedMessage) { // has message; process and let it fall thru to
                                // receive() if empty
            auto message{expectedMessage->value()};
            if (const protocol::GreetPayload* payload{
                    std::get_if<protocol::GreetPayload>(&message)}) {
                std::string opponentName{
                    payload->nameLength > 0
                        ? std::string{payload->playerName.data(),
                                      payload->nameLength}
                        : "Opponent"};

                return opponentName;
            }
        }

        if (auto res{connection.receive()}; !res.has_value())
            return std::unexpected{res.error()};
    }
}

std::expected<Session, std::string>
createSessionFor(const cli::HostSettings& settings) {
    auto listener{network::Listener::createHost(settings.endpoint)};
    if (!listener.has_value())
        return std::unexpected{listener.error()};

    auto accepted{listener->accept()};
    if (!accepted.has_value())
        return std::unexpected{accepted.error()};

    auto opponentName{exchangeNames(*accepted, settings.playerName)};
    if (!opponentName.has_value())
        return std::unexpected{opponentName.error()};

    return Session{.player1Name = settings.playerName,
                   .player2Name = std::move(opponentName).value(),
                   .peer = std::move(accepted).value(),
                   .env = Environment::Host};
}

std::expected<Session, std::string>
createSessionFor(const cli::ClientSettings& settings) {
    auto connection{network::Connection::createClient(settings.endpoint)};
    if (!connection.has_value())
        return std::unexpected{connection.error()};

    auto opponentName{exchangeNames(*connection, settings.playerName)};
    if (!opponentName.has_value())
        return std::unexpected{opponentName.error()};

    return Session{.player1Name = std::move(opponentName).value(),
                   .player2Name = settings.playerName,
                   .peer = std::move(connection).value(),
                   .env = Environment::Client};
}

std::expected<Session, std::string>
createSessionFor(const cli::LocalSettings& settings) {
    return Session{.player1Name = settings.player1Name,
                   .player2Name = settings.player2Name,
                   .peer = std::nullopt,
                   .env = Environment::Local};
}

} // namespace

std::expected<Session, std::string>
createSession(const cli::Settings& settings) {
    return std::visit([](const auto& s) { return createSessionFor(s); },
                      settings);
}
} // namespace pong::session