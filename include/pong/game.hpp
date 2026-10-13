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
    int player1Score{};
    int player2Score{};
};

[[nodiscard]] GameContext init(float initialBallAngle);

[[nodiscard]] std::optional<paddle::Side> getScoringSide(entities::Ball ball,
                                                         int screenWidth);

static_assert(sizeof(GameContext) == 60);
} // namespace pong::game