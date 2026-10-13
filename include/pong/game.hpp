#pragma once

#include <optional>
#include <pong/entities.hpp>
#include <pong/paddle.hpp>

namespace pong::game {
enum class GameState { Serving, Playing, GameOver };

struct GameContext {
    entities::Paddle player1{};
    entities::Paddle player2{};
    entities::Ball ball{};
    GameState state{GameState::Serving};
    int player1Score{};
    int player2Score{};
};

[[nodiscard]] GameContext init(float initialBallAngle, GameState startingState);

[[nodiscard]] std::optional<paddle::Side> getScoringSide(entities::Ball ball,
                                                         int screenWidth);

static_assert(sizeof(GameContext) == 64);
} // namespace pong::game