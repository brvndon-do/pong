#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <pong/config.hpp>
#include <span>
#include <variant>

namespace pong::network::protocol {
enum class MessageType : std::uint8_t { Greet, Input, Snapshot };

inline constexpr std::size_t header_sz{1};
inline constexpr std::size_t greet_payload_sz{1 + config::max_player_name};
inline constexpr std::size_t name_offset{1};
inline constexpr std::size_t input_payload_sz{3};
inline constexpr std::size_t snapshot_payload_sz{19};

struct GreetPayload {
    std::uint8_t nameLength{};
    std::array<char, config::max_player_name> playerName{};
};

struct InputPayload {
    std::int8_t direction{};
    std::uint8_t serve{};
    std::uint8_t restart{};
};

struct SnapshotPayload {
    float player1PaddleY{};
    float player2PaddleY{};
    float ballX{};
    float ballY{};
    std::uint8_t gameState{};
    std::uint8_t player1Score{};
    std::uint8_t player2Score{};
};

using Message = std::variant<GreetPayload, InputPayload, SnapshotPayload>;

[[nodiscard]] constexpr std::optional<std::size_t>
mapPayloadSz(MessageType messageType) {
    switch (messageType) {
    case MessageType::Greet:
        return greet_payload_sz;
    case MessageType::Input:
        return input_payload_sz;
    case MessageType::Snapshot:
        return snapshot_payload_sz;
    }

    return std::nullopt;
}
[[nodiscard]] std::array<std::byte, header_sz + greet_payload_sz>
encodeGreetPayload(GreetPayload payload);

[[nodiscard]] std::optional<GreetPayload>
decodeGreetPayload(std::span<const std::byte> data);

[[nodiscard]] std::array<std::byte, header_sz + input_payload_sz>
encodeInputPayload(InputPayload payload);

[[nodiscard]] std::optional<InputPayload>
decodeInputPayload(std::span<const std::byte> data);

[[nodiscard]] std::array<std::byte, header_sz + snapshot_payload_sz>
encodeSnapshotPayload(SnapshotPayload payload);

[[nodiscard]] std::optional<SnapshotPayload>
decodeSnapshotPayload(std::span<const std::byte> data);

} // namespace pong::network::protocol