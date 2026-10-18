#include <optional>
#include <pong/ball.hpp>
#include <pong/config.hpp>
#include <pong/entities.hpp>
#include <pong/game.hpp>
#include <pong/input.hpp>
#include <pong/paddle.hpp>

namespace cfg = pong::config;

namespace pong::game {
namespace {
entities::Ball updateBall(entities::Ball ball, float dt,
                          entities::Paddle player1, entities::Paddle player2) {
    ball = ball::move(ball, dt);
    ball = ball::bounceWalls(ball, cfg::screen_height);
    ball = ball::bouncePaddle(ball, player1);
    ball = ball::bouncePaddle(ball, player2);

    return ball;
}
} // namespace

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

GameContext update(GameContext context, input::InputState inputState, float dt,
                   float servingAngle) {
    entities::Paddle& player1{context.player1};
    entities::Paddle& player2{context.player2};
    entities::Ball& ball{context.ball};
    GameState& state{context.state};
    const input::PlayerInput player1Input{inputState.player1};
    const input::PlayerInput player2Input{inputState.player2};

    // allow players to still position during any state
    player1 = paddle::move(player1, player1Input.direction, cfg::paddle_speed,
                           dt, cfg::screen_height);
    player2 = paddle::move(player2, player2Input.direction, cfg::paddle_speed,
                           dt, cfg::screen_height);

    switch (state) {
    case GameState::Serving: {
        if (player1Input.serve || player2Input.serve)
            state = GameState::Playing;

        break;
    }
    case GameState::Playing: {
        ball = updateBall(ball, dt, player1, player2);

        const std::optional<paddle::Side> scoringSide{
            getScoringSide(ball, cfg::screen_width)};
        if (scoringSide) {
            ball = ball::init(cfg::ball_radius, *scoringSide, servingAngle,
                              cfg::ball_speed, cfg::screen_width,
                              cfg::screen_height);

            switch (*scoringSide) {
            case paddle::Side::Left:
                ++context.player1Score;
                break;
            case paddle::Side::Right:
                ++context.player2Score;
                break;
            }
        }

        if (context.player1Score >= cfg::winning_score ||
            context.player2Score >= cfg::winning_score) {
            state = GameState::GameOver;
        }

        break;
    }
    case GameState::GameOver: {
        if (player1Input.restart || player2Input.restart) {
            // safe for the object references since this overwrites the
            // object in place
            context = init(servingAngle, GameState::Playing);
        }

        break;
    }
    }

    return context;
}
} // namespace pong::game