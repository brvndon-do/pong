#pragma once

#include <optional>
#include <pong/entities.hpp>
#include <pong/paddle.hpp>

namespace pong::game {
enum class GameState { Serving, Playing, GameOver };

struct Scoreboard {
    int player1{};
    int player2{};
};

[[nodiscard]] std::optional<paddle::Side> getScoringSide(entities::Ball ball,
                                                         int screenWidth);
} // namespace pong::game