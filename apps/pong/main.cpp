#include <cstdlib>
#include <expected>
#include <format>
#include <iostream>
#include <optional>
#include <pong/ball.hpp>
#include <pong/cli.hpp>
#include <pong/config.hpp>
#include <pong/entities.hpp>
#include <pong/game.hpp>
#include <pong/paddle.hpp>
#include <random>
#include <raylib.h>
#include <string>
#include <string_view>
#include <vector>

namespace ball = pong::ball;
namespace cfg = pong::config;
namespace cli = pong::cli;
namespace entities = pong::entities;
namespace game = pong::game;
namespace paddle = pong::paddle;

namespace {
void drawCenterLine() {
    for (float y = 0; y < cfg::screen_height;
         y += (cfg::dash_gap + cfg::dash_length)) {
        DrawRectangleRec({.x = (static_cast<float>(cfg::screen_width) / 2.0f) -
                               (cfg::dash_width / 2.0f),
                          .y = y,
                          .width = cfg::dash_width,
                          .height = cfg::dash_length},
                         cfg::dash_color);
    }
}

int getDirection(KeyboardKey up, KeyboardKey down) {
    return IsKeyDown(down) - IsKeyDown(up);
}

entities::Ball updateBall(entities::Ball ball, float dt,
                          entities::Paddle player1, entities::Paddle player2) {
    ball = ball::move(ball, dt);
    ball = ball::bounceWalls(ball, cfg::screen_height);
    ball = ball::bouncePaddle(ball, player1);
    ball = ball::bouncePaddle(ball, player2);

    return ball;
}

int getCenterTextX(const char* msg, int fontSize, int screenWidth) {
    return (screenWidth / 2) - MeasureText(msg, fontSize) / 2;
}
} // namespace

int main(int argc, char* argv[]) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-cfg::ball_max_serve_angle_rad,
                                               cfg::ball_max_serve_angle_rad);

    const auto userSettings{
        cli::parseArgs(std::vector<std::string_view>(argv + 1, argv + argc))};

    if (!userSettings.has_value()) {
        std::cerr << userSettings.error() << '\n';
        return EXIT_FAILURE;
    }

    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(cfg::screen_width, cfg::screen_height, cfg::window_title);
    SetTargetFPS(cfg::fps);

    game::GameContext gameContext{
        game::init(dist(gen), game::GameState::Serving)};
    entities::Paddle& player1{gameContext.player1};
    entities::Paddle& player2{gameContext.player2};
    entities::Ball& ball{gameContext.ball};
    game::GameState& state{gameContext.state};

    while (!WindowShouldClose()) {
        const float dt{GetFrameTime()};
        const int player1Dir{getDirection(KEY_W, KEY_S)};
        const int player2Dir{getDirection(KEY_UP, KEY_DOWN)};

        // allow players to still position during any state
        player1 = paddle::move(player1, player1Dir, cfg::paddle_speed, dt,
                               cfg::screen_height);
        player2 = paddle::move(player2, player2Dir, cfg::paddle_speed, dt,
                               cfg::screen_height);

        // TODO: experimental code design? might need to refactor.
        switch (state) {
        case game::GameState::Serving: {
            if (IsKeyPressed(KEY_SPACE))
                state = game::GameState::Playing;

            break;
        }
        case game::GameState::Playing: {
            ball = updateBall(ball, dt, player1, player2);

            const std::optional<paddle::Side> scoringSide{
                game::getScoringSide(ball, cfg::screen_width)};
            if (scoringSide) {
                ball = ball::init(cfg::ball_radius, *scoringSide, dist(gen),
                                  cfg::ball_speed, cfg::screen_width,
                                  cfg::screen_height);

                switch (*scoringSide) {
                case paddle::Side::Left:
                    ++gameContext.player1Score;
                    break;
                case paddle::Side::Right:
                    ++gameContext.player2Score;
                    break;
                }
            }

            if (gameContext.player1Score >= cfg::winning_score ||
                gameContext.player2Score >= cfg::winning_score) {
                state = game::GameState::GameOver;
            }

            break;
        }
        case game::GameState::GameOver: {
            if (IsKeyPressed(KEY_R)) {
                // safe for the object references since this overwrites the
                // object in place
                gameContext = game::init(dist(gen), game::GameState::Playing);
            }

            break;
        }
        }

        // draw
        BeginDrawing();

        ClearBackground(cfg::stage_background);

        drawCenterLine();

        // draw entities
        DrawRectangleRec(player1.rect, cfg::paddle_color);
        DrawRectangleRec(player2.rect, cfg::paddle_color);

        switch (state) {
        case game::GameState::Playing:
            // draw only when state is playing
            DrawCircleV(ball.pos, ball.radius, cfg::ball_color);
            break;
        case game::GameState::Serving:
            DrawText(cfg::start_msg,
                     getCenterTextX(cfg::start_msg, cfg::text_font_size,
                                    cfg::screen_width),
                     (cfg::screen_height / 2), cfg::text_font_size,
                     cfg::text_color);
            break;
        case game::GameState::GameOver: {
            const std::string winningPlayer =
                gameContext.player1Score >= cfg::winning_score
                    ? userSettings->player1Name
                    : userSettings->player2Name;
            const std::string winnerText =
                std::format("{} wins! {}", winningPlayer, cfg::gameover_msg);

            DrawText(winnerText.c_str(),
                     getCenterTextX(winnerText.c_str(), cfg::text_font_size,
                                    cfg::screen_width),
                     cfg::screen_height / 2, cfg::text_font_size,
                     cfg::text_color);
            break;
        }
        }

        // draw scoreboard
        const std::string player1Score{std::format(
            "{}: {}", userSettings->player1Name, gameContext.player1Score)};
        const std::string player2Score{std::format(
            "{}: {}", userSettings->player2Name, gameContext.player2Score)};

        DrawText(player1Score.c_str(), cfg::text_font_gap, cfg::text_font_gap,
                 cfg::text_font_size, cfg::text_color);
        DrawText(player2Score.c_str(),
                 cfg::screen_width -
                     MeasureText(player2Score.c_str(), cfg::text_font_size) -
                     cfg::text_font_gap,
                 cfg::text_font_gap, cfg::text_font_size, cfg::text_color);

        EndDrawing();
    }

    CloseWindow();
}