#include <pong/ball.hpp>
#include <pong/config.hpp>
#include <pong/entities.hpp>
#include <pong/game.hpp>
#include <pong/paddle.hpp>

namespace cfg = pong::config;

namespace pong::game {
GameContext init(float initialBallAngle, GameState startingState) {
    entities::Paddle player1{paddle::init(paddle::Side::Left, cfg::paddle_width,
                                          cfg::paddle_length, cfg::screen_width,
                                          cfg::screen_height)};

    entities::Paddle player2{
        paddle::init(paddle::Side::Right, cfg::paddle_width, cfg::paddle_length,
                     cfg::screen_width, cfg::screen_height)};

    entities::Ball ball{ball::init(cfg::ball_radius, paddle::Side::Left,
                                   initialBallAngle, cfg::ball_speed,
                                   cfg::screen_width, cfg::screen_height)};

    return {.player1 = player1,
            .player2 = player2,
            .ball = ball,
            .state = startingState,
            .player1Score = 0,
            .player2Score = 0};
}

std::optional<paddle::Side> getScoringSide(entities::Ball ball,
                                           int screenWidth) {
    if (ball.pos.x + ball.radius < 0)
        return paddle::Side::Right;
    else if (ball.pos.x - ball.radius > screenWidth)
        return paddle::Side::Left;

    return std::nullopt;
}
} // namespace pong::game