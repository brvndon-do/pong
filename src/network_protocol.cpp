#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <pong/config.hpp>
#include <pong/game.hpp>
#include <pong/network_protocol.hpp>
#include <span>

namespace pong::network::protocol {
namespace {
void writeFloat(std::span<std::byte> data, std::size_t offset, float input) {
    std::uint32_t bits{std::bit_cast<std::uint32_t>(input)};

    data[offset + 0] = static_cast<std::byte>((bits >> 24) & 0xFF);
    data[offset + 1] = static_cast<std::byte>((bits >> 16) & 0xFF);
    data[offset + 2] = static_cast<std::byte>((bits >> 8) & 0xFF);
    data[offset + 3] = static_cast<std::byte>((bits >> 0) & 0xFF);
}

float readFloat(std::span<const std::byte> data, std::size_t offset) {
    std::uint32_t bits{0};
    bits |= static_cast<std::uint32_t>(data[offset + 0]) << 24;
    bits |= static_cast<std::uint32_t>(data[offset + 1]) << 16;
    bits |= static_cast<std::uint32_t>(data[offset + 2]) << 8;
    bits |= static_cast<std::uint32_t>(data[offset + 3]) << 0;

    return std::bit_cast<float>(bits);
}
} // namespace

std::array<std::byte, header_sz + greet_payload_sz>
encodeGreetPayload(GreetPayload payload) {
    std::array<std::byte, header_sz + greet_payload_sz> buffer{
        static_cast<std::byte>(MessageType::Greet),
        static_cast<std::byte>(payload.nameLength)};

    for (std::size_t i = 0; i < config::max_player_name; ++i) {
        buffer[header_sz + 1 + i] =
            static_cast<std::byte>(payload.playerName[i]);
    }

    return buffer;
}

std::optional<GreetPayload>
decodeGreetPayload(std::span<const std::byte> data) {
    if (data.size() != greet_payload_sz)
        return std::nullopt;

    std::uint8_t nameLength{static_cast<std::uint8_t>(data[0])};
    if (nameLength > config::max_player_name)
        return std::nullopt;

    std::array<char, config::max_player_name> playerName{};
    for (std::size_t i = 0; i < config::max_player_name; ++i) {
        playerName[i] = static_cast<char>(data[header_sz + i]);
    }

    return GreetPayload{.nameLength = nameLength, .playerName = playerName};
}

std::array<std::byte, header_sz + input_payload_sz>
encodeInputPayload(InputPayload payload) {
    return {static_cast<std::byte>(MessageType::Input),
            static_cast<std::byte>(payload.direction),
            static_cast<std::byte>(payload.serve),
            static_cast<std::byte>(payload.restart)};
}

std::optional<InputPayload>
decodeInputPayload(std::span<const std::byte> data) {
    if (data.size() != input_payload_sz)
        return std::nullopt;

    const std::int8_t direction{static_cast<std::int8_t>(data[0])};
    if (direction < -1 || direction > 1)
        return std::nullopt;

    const std::uint8_t serve{static_cast<std::uint8_t>(data[1])};
    if (serve > 1)
        return std::nullopt;

    const std::uint8_t restart{static_cast<std::uint8_t>(data[2])};
    if (restart > 1)
        return std::nullopt;

    return InputPayload{
        .direction = direction, .serve = serve, .restart = restart};
}

std::array<std::byte, header_sz + snapshot_payload_sz>
encodeSnapshotPayload(SnapshotPayload payload) {
    std::array<std::byte, header_sz + snapshot_payload_sz> buffer{
        static_cast<std::byte>(MessageType::Snapshot)};
    writeFloat(buffer, 0 + header_sz, payload.player1PaddleY);
    writeFloat(buffer, 4 + header_sz, payload.player2PaddleY);
    writeFloat(buffer, 8 + header_sz, payload.ballX);
    writeFloat(buffer, 12 + header_sz, payload.ballY);

    buffer[16 + header_sz] = static_cast<std::byte>(payload.gameState);
    buffer[17 + header_sz] = static_cast<std::byte>(payload.player1Score);
    buffer[18 + header_sz] = static_cast<std::byte>(payload.player2Score);

    return buffer;
}

std::optional<SnapshotPayload>
decodeSnapshotPayload(std::span<const std::byte> data) {
    if (data.size() != snapshot_payload_sz)
        return std::nullopt;

    const float player1PaddleY{readFloat(data, 0)};
    if (!std::isfinite(player1PaddleY))
        return std::nullopt;

    const float player2PaddleY{readFloat(data, 4)};
    if (!std::isfinite(player2PaddleY))
        return std::nullopt;

    const float ballX{readFloat(data, 8)};
    if (!std::isfinite(ballX))
        return std::nullopt;

    const float ballY{readFloat(data, 12)};
    if (!std::isfinite(ballY))
        return std::nullopt;

    const std::uint8_t gameState{static_cast<std::uint8_t>(data[16])};
    // NOTE: if more states added after GameOver, change this!
    if (gameState > static_cast<std::uint8_t>(game::GameState::GameOver))
        return std::nullopt;

    const std::uint8_t player1Score{static_cast<std::uint8_t>(data[17])};
    if (player1Score > config::winning_score)
        return std::nullopt;

    const std::uint8_t player2Score{static_cast<std::uint8_t>(data[18])};
    if (player2Score > config::winning_score)
        return std::nullopt;

    return SnapshotPayload{.player1PaddleY = player1PaddleY,
                           .player2PaddleY = player2PaddleY,
                           .ballX = ballX,
                           .ballY = ballY,
                           .gameState = gameState,
                           .player1Score = player1Score,
                           .player2Score = player2Score};
}
} // namespace pong::network::protocol