#include <algorithm>
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <pong/config.hpp>
#include <pong/network_protocol.hpp>
#include <span>
#include <string_view>

namespace protocol = pong::network::protocol;
namespace config = pong::config;

TEST_CASE("network::protocol greet encode/decode", "[protocol]") {
    std::array<char, config::max_player_name> name{};

    SECTION("greet encodes/decodes correctly") {
        std::ranges::copy(std::string_view{"brandondo1"}, name.begin());

        protocol::GreetPayload payload{.nameLength = 10, .playerName = name};

        auto encodedPayload{protocol::encodeGreetPayload(payload)};
        std::span encodedPayloadSpan{encodedPayload};
        auto decodedPayload{protocol::decodeGreetPayload(
            encodedPayloadSpan.subspan(protocol::header_sz))};

        REQUIRE(decodedPayload.has_value());
        REQUIRE(decodedPayload->nameLength == payload.nameLength);
        REQUIRE(decodedPayload->playerName == payload.playerName);
    }

    SECTION("greet encodes name with more than 10 characters and fails") {
        std::ranges::copy(std::string_view{"brandondo10"}, name.begin());

        protocol::GreetPayload payload{.nameLength = 11, .playerName = name};

        auto encodedPayload{protocol::encodeGreetPayload(payload)};
        std::span encodedPayloadSpan{encodedPayload};
        auto decodedPayload{protocol::decodeGreetPayload(
            encodedPayloadSpan.subspan(protocol::header_sz))};

        REQUIRE_FALSE(decodedPayload.has_value());
    }

    SECTION("greet decodes a span of incorrect length and fails") {
        std::ranges::copy(std::string_view{"brandon"}, name.begin());

        protocol::GreetPayload payload{.nameLength = 7, .playerName = name};

        auto encodedPayload{protocol::encodeGreetPayload(payload)};
        std::span encodedPayloadSpan{encodedPayload};
        auto decodedPayload{protocol::decodeGreetPayload(
            encodedPayloadSpan.subspan(2))}; // incorrect size

        REQUIRE_FALSE(decodedPayload.has_value());
    }
}

TEST_CASE("network::protocol input encode/decode", "[protocol]") {
    protocol::InputPayload payload{.direction = 1, .serve = 1, .restart = 0};

    SECTION("input encodes/decodes correctly") {
        auto encodedPayload{protocol::encodeInputPayload(payload)};
        std::span encodedPayloadSpan{encodedPayload};
        auto decodedPayload{protocol::decodeInputPayload(
            encodedPayloadSpan.subspan(protocol::header_sz))};

        REQUIRE(decodedPayload.has_value());
        REQUIRE(decodedPayload->direction == payload.direction);
        REQUIRE(decodedPayload->serve == payload.serve);
        REQUIRE(decodedPayload->restart == payload.restart);
    }

    SECTION("input encodes invalid direction and decode fails") {
        payload.direction = -2; // invalid range
        auto encodedPayload{protocol::encodeInputPayload(payload)};
        std::span encodedPayloadSpan{encodedPayload};
        auto decodedPayload{protocol::decodeInputPayload(
            encodedPayloadSpan.subspan(protocol::header_sz))};

        REQUIRE_FALSE(decodedPayload.has_value());
    }

    SECTION("input encodes invalid serve and decode fails") {
        payload.serve = 2; // invalid range
        auto encodedPayload{protocol::encodeInputPayload(payload)};
        std::span encodedPayloadSpan{encodedPayload};
        auto decodedPayload{protocol::decodeInputPayload(
            encodedPayloadSpan.subspan(protocol::header_sz))};

        REQUIRE_FALSE(decodedPayload.has_value());
    }

    SECTION("input encodes invalid restart and decode fails") {
        payload.restart = 2; // invalid range
        auto encodedPayload{protocol::encodeInputPayload(payload)};
        std::span encodedPayloadSpan{encodedPayload};
        auto decodedPayload{protocol::decodeInputPayload(
            encodedPayloadSpan.subspan(protocol::header_sz))};

        REQUIRE_FALSE(decodedPayload.has_value());
    }
}

TEST_CASE("network::protocol snapshot encode/decode", "[protocol]") {
    protocol::SnapshotPayload payload{.player1PaddleY = 1.0f,
                                      .player2PaddleY = 2.0f,
                                      .ballX = 1.0f,
                                      .ballY = 1.0f,
                                      .gameState = 1,
                                      .player1Score = 2,
                                      .player2Score = 3};

    SECTION("snapshot encodes/decodes correctly") {
        auto encodedPayload{protocol::encodeSnapshotPayload(payload)};
        std::span encodedPayloadSpan{encodedPayload};
        auto decodedPayload{protocol::decodeSnapshotPayload(
            encodedPayloadSpan.subspan(protocol::header_sz))};

        REQUIRE(decodedPayload.has_value());
        REQUIRE(decodedPayload->player1PaddleY == payload.player1PaddleY);
        REQUIRE(decodedPayload->player2PaddleY == payload.player2PaddleY);
        REQUIRE(decodedPayload->ballX == payload.ballX);
        REQUIRE(decodedPayload->ballY == payload.ballY);
        REQUIRE(decodedPayload->gameState == payload.gameState);
        REQUIRE(decodedPayload->player1Score == payload.player1Score);
        REQUIRE(decodedPayload->player2Score == payload.player2Score);
    }

    SECTION("snapshot encodes invalid gamestate and decode fails") {
        payload.gameState = 3; // invalid range

        auto encodedPayload{protocol::encodeSnapshotPayload(payload)};
        std::span encodedPayloadSpan{encodedPayload};
        auto decodedPayload{protocol::decodeSnapshotPayload(
            encodedPayloadSpan.subspan(protocol::header_sz))};

        REQUIRE_FALSE(decodedPayload.has_value());
    }

    SECTION("snapshot encodes invalid score and decode fails") {
        payload.player1Score = 6; // invalid range

        auto encodedPayload{protocol::encodeSnapshotPayload(payload)};
        std::span encodedPayloadSpan{encodedPayload};
        auto decodedPayload{protocol::decodeSnapshotPayload(
            encodedPayloadSpan.subspan(protocol::header_sz))};

        REQUIRE_FALSE(decodedPayload.has_value());
    }
}