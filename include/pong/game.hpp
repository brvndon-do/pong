#pragma once

#include <cstdint>
#include <optional>
#include <pong/entities.hpp>
#include <pong/input.hpp>
#include <pong/paddle.hpp>
#include <type_traits>

namespace pong::game {
enum class GameState : std::uint8_t { Serving, Playing, GameOver };

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

[[nodiscard]] GameContext update(GameContext context,
                                 input::InputState inputState, float dt,
                                 float servingAngle);

static_assert(sizeof(GameContext) == 64);
static_assert(std::is_trivially_copyable_v<GameContext>);

} // namespace pong::game